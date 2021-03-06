#pragma once
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Input/Input.h>
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
namespace Urho3D
{
	class Node;
	class Scene;
	class RigidBody;
	class CollisionShape;
	class ResourceCache;
}
// All Urho3D classes reside in namespace Urho3D
using namespace Urho3D;


#include <vector>

class Boid
{
	

	

	static float Range_FAttract;
	static float Range_FRepel;
	static float Range_FAlign;
	static float FAttract_Factor;
	static float FRepel_Factor;
	static float FAlign_Factor;
	static float FAttract_Vmax;

public:
	Boid();
	~Boid();

	Vector3 force;
	
	Node* pNode;
	RigidBody* pRigidBody;
	CollisionShape* pCollisionShape;
	StaticModel* pObject;

	
	void Initialise(ResourceCache* pRes, Scene* pScene, float x, float z, float cellSize);
	void ComputeForce(Boid* neighbour,int NumBoids);
	void ComputeForce(std::vector<Boid> neighbour, int NumBoids);

	void AvgForce();

	void Update(float deltaTime);

	Vector3 CoM = Vector3(0, 0, 0); //center of mass, accumulated total
	Vector3 avgDir = Vector3(0, 0, 0);
	Vector3 displacement = Vector3(0, 0, 0);

	int n = 0; //count number of neighbours
	int dN = 0; //count number of alightment neighbours
	int rN = 0; //count number of replel neighbours;


};
