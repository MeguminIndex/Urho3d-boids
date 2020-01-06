//
// Copyright (c) 2008-2016 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>

#include "Character.h"
#include "CharacterDemo.h"
#include "Touch.h"

#include <Urho3D/DebugNew.h>

//UI

static const StringHash E_CLIENTCUSTOMEVENTBYOLIVIER("ClientCustomEventByOlivier");

// Custom remote event we use to tell the client which object they control
static const StringHash E_CLIENTOBJECTAUTHORITY("ClientObjectAuthority");
// Identifier for the node ID parameter in the event data
static const StringHash PLAYER_ID("IDENTITY");
// Custom event on server, client has pressed button that it wants to start game
static const StringHash E_CLIENTISREADY("ClientReadyToStart");



URHO3D_DEFINE_APPLICATION_MAIN(CharacterDemo)

CharacterDemo::CharacterDemo(Context* context) :
    Sample(context),
    firstPerson_(false)
{
	//TUTORIAL: TODO


}

CharacterDemo::~CharacterDemo()
{
}

void CharacterDemo::Start()
{
	OpenConsoleWindow();

    // Execute base class startup
    Sample::Start();
    if (touchEnabled_)
        touch_ = new Touch(context_, TOUCH_SENSITIVITY);



	//TUTORIAL: TODO
	CreateScene();

	//subscibe this class relitive events
	SubscribeToEvents();
	

	//set the mouse mdoe to use
	Sample::InitMouseMode(MM_RELATIVE);

	CreateMainMenu();
}

Controls CharacterDemo::FromClientToServerControls()
{
	Input* input = GetSubsystem<Input>();
	Controls controls;
	//Check which button has been pressed, keep track
	controls.Set(CTRL_FORWARD, input->GetKeyDown(KEY_W));
	controls.Set(CTRL_BACK, input->GetKeyDown(KEY_S));
	controls.Set(CTRL_LEFT, input->GetKeyDown(KEY_A));
	controls.Set(CTRL_RIGHT, input->GetKeyDown(KEY_D));
	controls.Set(1024, input->GetKeyDown(KEY_E));

	// mouse yaw to server
	controls.yaw_ = yaw_;
	return controls;

}

void CharacterDemo::ProcessClientControls()
{
	Network* network = GetSubsystem<Network>();
	const Vector<SharedPtr<Connection> >& connections = network->GetClientConnections();
	//go through every client connected
	for (unsigned i = 0; i < connections.Size(); ++i)
	{
		Connection* connection = connections[i];
		Node* ballNode = serverObjects_[connection];
		// Client has no item connected
		if (!ballNode) continue;

		//get client controls sent by client
		const Controls& controls = connection->GetControls();

		RigidBody* body = ballNode->GetComponent<RigidBody>();
		// Torque is relative to the forward vector
		Quaternion rotation(0.0f, controls.yaw_, 0.0f);
		const float MOVE_TORQUE = 5.0f;
		if (controls.buttons_ & CTRL_FORWARD)
			body->ApplyTorque(rotation * Vector3::RIGHT * MOVE_TORQUE);
		if (controls.buttons_ & CTRL_BACK)
			body->ApplyTorque(rotation * Vector3::LEFT * MOVE_TORQUE);
		if (controls.buttons_ & CTRL_LEFT)
			body->ApplyTorque(rotation * Vector3::FORWARD * MOVE_TORQUE);
		if (controls.buttons_ & CTRL_RIGHT)
			body->ApplyTorque(rotation * Vector3::BACK * MOVE_TORQUE);


		
	}

}

void CharacterDemo::HandlePhysicsPreStep(StringHash eventType, VariantMap & eventData)
{
	Network* network = GetSubsystem<Network>();
	Connection* serverConnection = network->GetServerConnection();

	// Client: collect controls
	if (serverConnection)
	{
		//printf("fromtclientoserverc");
		serverConnection->SetPosition(cameraNode_->GetPosition()); // send camera position too
		serverConnection->SetControls(FromClientToServerControls()); // send controls to server
	
																	 //Then tell server to call custom event onto server machine
		//VariantMap remoteEventData;
		//remoteEventData["aValueRemoteValue"] = 0;
		//serverConnection->SendRemoteEvent(E_CLIENTCUSTOMEVENTBYOLIVIER, true, remoteEventData);

	
	}
	// Server: Read Controls, Apply them if needed
	else if (network->IsServerRunning())
	{
		
	//	std::cout << "Server runnign" << std::endl;
		ProcessClientControls(); // take data from clients, process it
	}

	
}

void CharacterDemo::HandleClientFinishedLoading(StringHash eventType, VariantMap & eventData)
{
	Log::WriteRaw("Client has finished laoding up the scene from server");

}

void CharacterDemo::HandleCustomEventByOlivier(StringHash eventType, VariantMap & eventData)
{
	int exampleValue = eventData["aValueRemoteValue"].GetUInt();
	printf("This is a custom event by Olivier - passed Value - %i \n", exampleValue);

}

void CharacterDemo::HandleConnect(StringHash eventType, VariantMap& eventData)
{
	//clear scene and prepair to recive from server


	Network* network = GetSubsystem<Network>();
	String address = serverAdressLineEdit_->GetText().Trimmed();
	if (address.Empty())
		address = "localhost";




	//specify a scne to use as client
	network->Connect(address, SERVER_PORT, scene_);


}

void CharacterDemo::CreateScene()
{
	gState = GameState();


	//TUTORIAL: TODO
	//si we cab acces resources
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	scene_ = new Scene(context_);
	//create scene subsystems
	scene_->CreateComponent<Octree>(LOCAL);
	scene_->CreateComponent<PhysicsWorld>(LOCAL);

	//create camera node and component
	cameraNode_ = new Node(context_);
	Camera* camera = cameraNode_->CreateComponent<Camera>(LOCAL);
	cameraNode_->SetPosition(Vector3(0.0f, 5.0f, 0.0f));
	camera->SetFarClip(300.0f);

	//configure viewport
	GetSubsystem<Renderer>()->SetViewport(0, new Viewport(context_, scene_, camera));

	//zone obj for lighting ambiant and fog
	Node* zoneNode = scene_->CreateChild("Zone");
	Zone* zone = zoneNode->CreateComponent<Zone>();
	zone->SetAmbientColor(Color(0.15f, 0.15f, 0.15f));
	zone->SetFogColor(Color(0.5f,0.5f,0.7f));
	zone->SetFogStart(100.0f);
	zone->SetFogEnd(300.0f);
	zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));


	//Create a directional light with cascade shadow mappin
	Node* lightNode = scene_->CreateChild("DirectionalLight");
	lightNode->SetDirection(Vector3(0.3f,-0.5f, 0.425f));
	Light* light = lightNode->CreateComponent<Light>();
	light->SetLightType(LIGHT_DIRECTIONAL);
	light->SetCastShadows(true);
	light->SetShadowBias(BiasParameters(0.00025f,0.5f));
	light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));
	light->SetSpecularIntensity(0.5f);


	//create floor
	Node* floorNode = scene_->CreateChild("Floor",LOCAL);
	float x, z;
	x = 500.0f, z =500.0f;

	//update the gamestae grids values
	gState.gridX = x;
	gState.gridZ = z;
	gState.cellSize = 20;//each cell is XxX

	floorNode->SetScale(Vector3(x, 1.0f, z));
	floorNode->SetPosition(Vector3(0.0f+(x/2), -0.5f, 0.0f +(z/2)));
	StaticModel* object = floorNode->CreateComponent<StaticModel>();
	object->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	object->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));

	RigidBody* body = floorNode->CreateComponent<RigidBody>();
	//use collision layer bit 2 to make world scenery this is what will raycast against to prevent camera from going inside geometry
	body->SetCollisionLayer(2);
	CollisionShape* shape = floorNode->CreateComponent<CollisionShape>();
	shape->SetBox(Vector3::ONE);


	//create shrooms 

	const unsigned NUM_MUSHROOMS = 60;

	for (unsigned i =0; i < NUM_MUSHROOMS; i++)
	{
		Node* objectNode = scene_->CreateChild("Mushroom");
		objectNode->SetPosition(Vector3(Random(x), 0.0f,Random(z)));
		objectNode->SetRotation(Quaternion(0.0f, Random(360.0f), 0.0f));
		objectNode->SetScale(2.0f + Random(5.0f));
		StaticModel* object = objectNode->CreateComponent<StaticModel>();
		object->SetModel(cache ->GetResource<Model>("Models/Mushroom.mdl"));
		object->SetMaterial(cache ->GetResource<Material>("Materials/Mushroom.xml"));
		object->SetCastShadows(true);
		RigidBody* body = objectNode->CreateComponent<RigidBody>();
		body->SetCollisionLayer(2);
		CollisionShape* shape = objectNode ->CreateComponent<CollisionShape>();
		shape->SetTriangleMesh(object->GetModel(), 0);
	}



	//const unsigned NUM_BOXES = 200;
	//for (unsigned i = 0; i < NUM_BOXES; ++i)
	//{
	//	float scale = Random(2.0f) + 0.5f;
	//	Node* objectNode = scene_->CreateChild("Box");
	//	objectNode->SetPosition(Vector3(Random(180.0f) - 90.0f,
	//		Random(10.0f) + 1.0f, Random(180.0f) - 90.0f));
	//	objectNode->SetRotation(Quaternion(Random(360.0f),
	//		Random(360.0f), Random(360.0f)));
	//	objectNode->SetScale(scale);
	//	StaticModel* object = objectNode->CreateComponent<StaticModel>();
	//	object->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
	//	object->SetMaterial(cache ->GetResource<Material>("Materials/Stone.xml"));
	//	object->SetCastShadows(true);
	//	RigidBody* body = objectNode->CreateComponent<RigidBody>();
	//	body->SetCollisionLayer(2);
	//	// Bigger boxes will be heavier and harder to move
	//	body->SetMass(scale * 2.0f);
	//	CollisionShape* shape = objectNode ->CreateComponent<CollisionShape>();
	//	shape->SetBox(Vector3::ONE);
	//}


	//adding boids

	boidSet.Initialise(cache,scene_,x,z, gState.cellSize);
	gMissile.Initialise(cache,scene_);

}

void CharacterDemo::CreateCharacter()
{
	//TUTORIAL: TODO
}

void CharacterDemo::CreateInstructions()
{
	//TUTORIAL: TODO
}

void CharacterDemo::SubscribeToEvents()
{
	//TUTORIAL: TODO

	//subscribe to update event
	SubscribeToEvent(E_UPDATE,URHO3D_HANDLER(CharacterDemo,HandleUpdate));
	SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(CharacterDemo, HandlePostUpdate));

	// Setting or applying controls
	SubscribeToEvent(E_PHYSICSPRESTEP, URHO3D_HANDLER(CharacterDemo, HandlePhysicsPreStep));

	
	SubscribeToEvent(E_CLIENTCUSTOMEVENTBYOLIVIER, URHO3D_HANDLER(CharacterDemo,
	HandleCustomEventByOlivier));
	GetSubsystem<Network>()->RegisterRemoteEvent(E_CLIENTCUSTOMEVENTBYOLIVIER);
		
	SubscribeToEvent(E_CLIENTISREADY, URHO3D_HANDLER(CharacterDemo, HandleClientToServerReadyToStart));
	GetSubsystem<Network>()->RegisterRemoteEvent(E_CLIENTISREADY);

	SubscribeToEvent(E_CLIENTOBJECTAUTHORITY, URHO3D_HANDLER(CharacterDemo, HandleServerToClientObjectID));
	GetSubsystem<Network>()->RegisterRemoteEvent(E_CLIENTOBJECTAUTHORITY);

}

void CharacterDemo::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	//TUTORIAL: TODO
	using namespace Update;

	//get the frame timestep
	float timeStep = eventData[P_TIMESTEP].GetFloat();



	//done move if ui has focus
	if (GetSubsystem<UI>()->GetFocusElement()) return;

	Input* input = GetSubsystem<Input>();

	//movement speed
	const float MOVE_SPEED = 20.0f;
	//mouse sensitivy
	const float MOUSE_SENSITIVITY = 0.1f;

	UI* ui = GetSubsystem<UI>();//stop camera move whiles UI is up
	if (!ui->GetCursor()->IsVisible())
	{

		//get mouse motion this frame and adjust camera node accordingly
		//clamp the pitch

		IntVector2 mouseMove = input->GetMouseMove();
		yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
		pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
		pitch_ = Clamp(pitch_, -90.0f, 90.0f);

		//create new orientation for camera node;

		cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));

		//read WSD keys (input) 

		/*if (input->GetKeyDown(KEY_W))
			cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED *
				timeStep);
		if (input->GetKeyDown(KEY_S))
			cameraNode_->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
		if (input->GetKeyDown(KEY_A))
			cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
		if (input->GetKeyDown(KEY_D))
			cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);*/

	}

	Network* network = GetSubsystem<Network>();
	if (network->IsServerRunning())
	{
		boidSet.Update(timeStep,gState.gridX,gState.gridZ,gState.cellSize);
		gMissile.Update(timeStep);
	}

	if (input->GetKeyDown(KEY_F) && gMissile.active == false)
	{
		gMissile.active = true;
		gMissile.pNode->SetEnabled(true);
		gMissile.mTimer = 10;
		gMissile.pRigidBody->SetPosition(cameraNode_->GetPosition());
		gMissile.pRigidBody->SetLinearVelocity(cameraNode_ ->GetDirection().Normalized()*40.0f);

	}

	if (input->GetKeyPress(KEY_M))
		menuVisible = !menuVisible;
}

void CharacterDemo::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{

	// menu visible & invisible
	UI* ui = GetSubsystem<UI>();
	Input* input = GetSubsystem<Input>();
	ui->GetCursor()->SetVisible(menuVisible);
	window_->SetVisible(menuVisible);


	// Only move the camera if we have a controllable object
	if (clientObjectID_)
	{
		Node* ballNode = this->scene_->GetNode(clientObjectID_);
		if (ballNode)
		{
			const float CAMERA_DISTANCE = 5.0f;
			cameraNode_->SetPosition(ballNode->GetPosition() + cameraNode_->GetRotation()
				* Vector3::BACK * CAMERA_DISTANCE);
		}
	}

}


void CharacterDemo::HandleClientStartGame(StringHash eventType, VariantMap& eventData)
{

	std::cout << "Client has pressed START GAME" << std::endl;
	if (clientObjectID_ == 0)//client is still observer as 0 is defualted value
	{
		Network* network = GetSubsystem<Network>();
		Connection* serverConnection = network->GetServerConnection();

		if (serverConnection)
		{
			VariantMap remoteEventData;
			remoteEventData[PLAYER_ID] = 0;
			serverConnection->SendRemoteEvent(E_CLIENTISREADY, true, remoteEventData);
		}


	}


}



void CharacterDemo::CreateMainMenu()
{
	InitMouseMode(MM_RELATIVE);

	ResourceCache* cache = GetSubsystem<ResourceCache>();
	UI* ui = GetSubsystem<UI>();
	UIElement* root = ui->GetRoot();
	XMLFile* uiStyle = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
	root->SetDefaultStyle(uiStyle);//set as defualt UI style


	/* Create a Cursor UI element.We want to be able to hide and show the main menu at will.
	When hidden, the mouse will control the camera, and when visible,
	the mouse will be able to interact with the GUI. */

	SharedPtr<Cursor> cursor(new Cursor(context_));
	cursor->SetStyleAuto(uiStyle);
	ui->SetCursor(cursor);
	// Create the Window and add it to the UI's root node
	window_ = new Window(context_);
	root->AddChild(window_);
	// Set Window size and layout settings
	window_->SetMinWidth(384);
	window_->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
	window_->SetAlignment(HA_CENTER, VA_CENTER);
	window_->SetName("MainMenu-Window");
	window_->SetStyleAuto();

	
	Font* font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");
	
	Button* connectButton_ = CreateButton("Connect",24,window_,cache,font);
	 serverAdressLineEdit_ = CreateLineEdit("localhost", 24, window_);
	Button* disconnectButton_ = CreateButton("Disconnect", 24, window_,cache, font);
	Button* startServerButton_ = CreateButton("Start Server", 24, window_, cache, font);
	Button* startClientGameButton_ = CreateButton("Client: Start Game",24,window_,cache,font);
	Button* QuitButton = CreateButton("QUIT", 24, window_, cache, font);
	

	CheckBox* checkbox = CreateCheckBox(24,window_);
	
	/*ToolTip* toolTip = checkbox->CreateChild<ToolTip>("ToolTip");
	toolTip->SetMinOffset(IntVector2(1, 1));
	BorderImage* textHolder = toolTip->CreateChild<BorderImage>("BorderImage");
	textHolder->SetStyle("ToolTipBorderImage");

	Text* toolTipText = textHolder->CreateChild<Text>("Text");
	toolTipText->SetStyle("ToolTipText");
	toolTipText->SetText("Tooool tip");
	*/

	//Slider* slider = window_->CreateChild<Slider>("Slider");
	//slider->SetMinHeight(24);
	//slider->SetStyleAuto();
	//window_->AddChild(slider);


	SubscribeToEvent(QuitButton,E_RELEASED,URHO3D_HANDLER(CharacterDemo,HandleQuit));
	SubscribeToEvent(startServerButton_, E_RELEASED, URHO3D_HANDLER(CharacterDemo, HandleStartServer));
	SubscribeToEvent(connectButton_, E_RELEASED, URHO3D_HANDLER(CharacterDemo, HandleConnect));
	SubscribeToEvent(startClientGameButton_, E_RELEASED, URHO3D_HANDLER(CharacterDemo, HandleClientStartGame));
	SubscribeToEvent(disconnectButton_, E_RELEASED, URHO3D_HANDLER(CharacterDemo, HandleDisconnect));


}

void CharacterDemo::HandleQuit(StringHash eventType, VariantMap& eventData)
{
	engine_->Exit();
}

void CharacterDemo::HandleDisconnect(StringHash eventType, VariantMap& eventData)
{
	std::cout << "HandleDisconnect has been pressed" << std::endl;

	Network* network = GetSubsystem<Network>();
	Connection* serverConnection = network->GetServerConnection();

	// Running as Client
	if (serverConnection)
	{
		serverConnection->Disconnect();
		scene_->Clear(true, false);
		clientObjectID_ = 0;
	}
	// Running as a server, stop it
	else if (network->IsServerRunning())
	{
		network->StopServer();
		scene_->Clear(true, false);
	}


}

void CharacterDemo::HandleStartServer(StringHash eventType, VariantMap & eventData)
{
	std::cout << "(HandleStartServer called) Server is started!" << std::endl;
	Network* network = GetSubsystem<Network>();
	network->StartServer(SERVER_PORT);
	// code to make your main menu disappear. Boolean value
	menuVisible = !menuVisible;

	//Server
	SubscribeToEvent(E_CLIENTSCENELOADED, URHO3D_HANDLER(CharacterDemo, HandleClientFinishedLoading));
	

	if (network->IsServerRunning())
	{
		SubscribeToEvent(E_CLIENTCONNECTED, URHO3D_HANDLER(CharacterDemo, HandleClientConnected));
		SubscribeToEvent(E_CLIENTDISCONNECTED, URHO3D_HANDLER(CharacterDemo, HandleClientDisconnected));
	}

}

void CharacterDemo::HandleClientConnected(StringHash eventType, VariantMap & eventData)
{
	std::cout << "Client connected!" << std::endl;

	using namespace ClientConnected;

	//when a client connects, assign to a scene
	Connection* newConnection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());
	newConnection->SetScene(scene_);//sets clients scene
									//send an event to the client that has just connected
	VariantMap remoteEventData;
	remoteEventData["aValueRemoteValue"] = 0;
	newConnection->SendRemoteEvent(E_CLIENTCUSTOMEVENTBYOLIVIER, true, remoteEventData);
	//or send to all clients:
	//network->BroadcastRemoteEvent(E_CLIENTCUSTOMEVENTBYOLIVIER, true, remoteEventData);

}

void CharacterDemo::HandleClientDisconnected(StringHash eventType, VariantMap & eventData)
{
	Log::WriteRaw("Client Disconnected!");

	using namespace ClientConnected;


}



Button* CharacterDemo::CreateButton(const String& text, int pHeight, Urho3D::Window* whichWindow, ResourceCache* cache, Font* font)
{

	//could pass in a font instead of loading font everytime since i may use same font for many buttons
	
	Button* button = whichWindow->CreateChild<Button>();
	button->SetMinHeight(pHeight);
	button->SetStyleAuto();

	Text* buttonText = button->CreateChild<Text>();
	buttonText->SetFont(font, 12);
	buttonText->SetAlignment(HA_CENTER, VA_BOTTOM);
	buttonText->SetText(text);
	whichWindow->AddChild(button);

	return button;
}
LineEdit* CharacterDemo::CreateLineEdit(const String& text, int pHeight, Urho3D::Window* whichWindow)
{
	LineEdit* lineEdit = whichWindow->CreateChild<LineEdit>();
	lineEdit->SetMinHeight(pHeight);
	lineEdit->SetAlignment(HA_CENTER, VA_CENTER);
	lineEdit->SetText(text);
	whichWindow->AddChild(lineEdit);
	lineEdit->SetStyleAuto();

	return lineEdit;
}
CheckBox* CharacterDemo::CreateCheckBox(int pHeight, Urho3D::Window* whichWindow) {

	CheckBox* checkbox = whichWindow->CreateChild<CheckBox>();
	checkbox->SetMinHeight(pHeight);
	checkbox->SetStyleAuto();
	whichWindow->AddChild(checkbox);

	return checkbox;
}

Node * CharacterDemo::CreateControllableObject()
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	// Create the scene node & visual representation. This will be a replicated object
	Node* ballNode = scene_->CreateChild("AClientBall");
	ballNode->SetPosition(Vector3(gState.gridX/2, 5.0f, gState.gridZ / 2));
	ballNode->SetScale(2.5f);
	StaticModel* ballObject = ballNode->CreateComponent<StaticModel>();
	ballObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
	ballObject->SetMaterial(cache->GetResource<Material>("Materials/StoneSmall.xml"));
	// Create the physics components
	RigidBody* body = ballNode->CreateComponent<RigidBody>();
	body->SetMass(1.0f);
	body->SetFriction(1.0f);
	// motion damping so that the ball can not accelerate limitlessly
	body->SetLinearDamping(0.25f);
	body->SetAngularDamping(0.25f);
	CollisionShape* shape = ballNode->CreateComponent<CollisionShape>();
	shape->SetSphere(1.0f);
	return ballNode;

}

void CharacterDemo::HandleServerToClientObjectID(StringHash eventType, VariantMap & eventData)
{
	clientObjectID_ = eventData[PLAYER_ID].GetUInt();
	printf("Client ID : %i \n", clientObjectID_);
}

void CharacterDemo::HandleClientToServerReadyToStart(StringHash eventType, VariantMap & eventData)
{
	std::cout << "Event sent by the clien and running on Server: Client is ready to start the game"
		<< std::endl;

	using namespace ClientConnected;

	Connection* newConnection = static_cast<Connection*>(eventData[P_CONNECTION].GetPtr());
	//create controllable object
	Node* newObject = CreateControllableObject();
	serverObjects_[newConnection] = newObject;


	//send objects node ID using a remote event
	VariantMap remoteEventData;
	remoteEventData[PLAYER_ID] = newObject->GetID();
	newConnection->SendRemoteEvent(E_CLIENTOBJECTAUTHORITY, true, remoteEventData);


}
