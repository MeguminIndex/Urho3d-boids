#include"Missile.h"


void Missile::Initialise(ResourceCache* pRes, Scene* pScene)
{
	pNode = pScene->CreateChild("Missile");
	pNode->SetPosition(Vector3(0,40.0f,0));
	pNode->SetRotation(Quaternion(0,0,0));
	pNode->SetScale(2.0f + Random(5.0f));

	pObject = pNode->CreateComponent<StaticModel>();
	pObject->SetModel(pRes->GetResource<Model>("Models/Cone.mdl"));
	pObject->SetMaterial(pRes->GetResource<Material>("Materials/Water.xml"));
	pObject->SetCastShadows(true);

	CollisionShape* shape = pNode->CreateComponent<CollisionShape>();
	shape->SetBox(Vector3::ONE);

	pRigidBody = pNode->CreateComponent<RigidBody>();
	pRigidBody->SetCollisionLayer(2);
	pRigidBody->SetMass(2);
	pRigidBody->SetUseGravity(false);
	/*pRigidBody->SetPosition(Vector3(Random(180.0f) - 90.0f, 30.0f,
	Random(180.0f) - 90.0f));
	*/
	//pRigidBody->SetLinearVelocity(Vector3(Random(20.0f), 0, Random(20.0f)));

	

	

	pNode->SetEnabled(false);


}


void::Missile::Update(float deltaTime)
{
	if (mTimer > 0 && active == true)
	{
		mTimer -= deltaTime;
	}
	else
	{
		active = false;
		pNode->SetEnabled(false);
	}



}

void Missile::OnCollisionStart(StringHash eventType, VariantMap & eventData)
{
	std::cout << "Collision started" << std::endl;
}
