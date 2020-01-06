
#include "Boid.h"

float Boid::Range_FAttract = 70.0f;
float Boid::Range_FRepel = 10.0f;
float Boid::Range_FAlign = 40.0f;
float Boid::FAttract_Vmax = 20.0f;
float Boid::FAttract_Factor = 5.0f;
float Boid::FRepel_Factor = 150.0f;
float Boid::FAlign_Factor = 100.0f;

float maxSpeed = 5.0f;

Boid::Boid()
{

}

Boid::~Boid()
{

}

void Boid::Initialise(ResourceCache* pRes, Scene* pScene, float x, float z, float cellSize)
{
	pNode = pScene->CreateChild("Boid");
	pNode->SetPosition(Vector3(20 + Random(x/2), Random(600.0f), 20 + Random(z/2)));
	//pNode->SetRotation(Quaternion(Random(360.0f), Random(360.0f), Random(360.0f)));
	pNode->SetScale(2.0f + Random(5.0f));

	pObject = pNode->CreateComponent<StaticModel>();
	pObject->SetModel(pRes->GetResource<Model>("Models/Cone.mdl"));
	pObject->SetMaterial(pRes->GetResource<Material>("Materials/Stone.xml"));
	pObject->SetCastShadows(true);

	
	//CollisionShape* shape = pNode->CreateComponent<CollisionShape>();
	//shape->SetBox(Vector3::ONE);

	pRigidBody = pNode->CreateComponent<RigidBody>();
	pRigidBody->SetCollisionLayer(2);
	pRigidBody->SetMass(2); 
	pRigidBody->SetUseGravity(false);
	/*pRigidBody->SetPosition(Vector3(Random(180.0f) - 90.0f, 30.0f,
		Random(180.0f) - 90.0f));
*/
	pRigidBody->SetLinearVelocity(Vector3(Random(20.0f), Random(10.0f), Random(20.0f)));
	
	force = Vector3(0, 0, 0);
}
//origonal
void Boid::ComputeForce(Boid*pBoidList ,int NumBoids)
{
	Vector3 CoM; //center of mass, accumulated total
	Vector3 avgDir;
	Vector3 displacement;

	int n = 0; //count number of neighbours
	int dN = 0; //count number of alightment neighbours
	int rN = 0; //count number of replel neighbours;
	//set the force memeber variable to zero

	force = Vector3(0,0,0);

	//search neighbourhood
	for (int i =0; i <NumBoids; i++)
	{
		//the current boid?
		if (this == &pBoidList[i]) continue;

		//sep = vector position of this boid from current oid
		Vector3 sep = pRigidBody->GetPosition() - pBoidList[i].pRigidBody->GetPosition();

		float d = sep.Length(); //distance of boid
		if (d < Range_FAttract)
		{
			//with range, so is a neighbour
			CoM += pBoidList[i].pRigidBody->GetPosition();
			

			n++;
		}

		if (d < Range_FAlign)
		{
			avgDir += pBoidList[i].pNode->GetDirection();

				dN++;
		}


		if (d < Range_FRepel)
		{
			displacement -= sep;
			rN++;
		}
	}

	//Attractive force component
	if (n > 0)
	{
		//find average position = centre of mass
		CoM /= n;
		Vector3 dir = (CoM - pRigidBody->GetPosition()).Normalized();
		Vector3 vDesired = dir*FAttract_Vmax;
		force += (vDesired - pRigidBody->GetLinearVelocity())*FAttract_Factor;
	}

	//need to add the other 2 forced (alightment and somethign else??)
	//repel
	if (rN > 0)
	{
		//displacement /= rN;
		//Vector3 dis = (displacement + pRigidBody->GetPosition()).Normalized();
		//Vector3 disDesired = dis * FRepel_Factor;
		force += displacement * -FRepel_Factor;
	}


	//alighnment
	if (dN > 0)
	{
		avgDir /= dN;
		Vector3 dir = (avgDir - pRigidBody->GetPosition()).Normalized();
		Vector3 dirDesired = dir * FAlign_Factor;
		force += (dirDesired - pRigidBody->GetLinearVelocity());



	}

	

}


//new
void Boid::ComputeForce(std::vector<Boid> neighbour, int NumBoids)
{
	


				//set the force memeber variable to zero

	//force = Vector3(0, 0, 0);

	//search neighbourhood
	for (int i = 0; i <NumBoids; i++)
	{
		//the current boid?
		if (this == &neighbour[i]) continue;

		//sep = vector position of this boid from current boid
		Vector3 sep = pRigidBody->GetPosition() - neighbour[i].pRigidBody->GetPosition();

		float d = sep.Length(); //distance of boid
		if (d < Range_FAttract && d > 0)
		{
			//with range, so is a neighbour
			CoM += neighbour[i].pRigidBody->GetPosition();
			n++;
		}

		if (d < Range_FAlign && d > 0)
		{
			avgDir += neighbour[i].pRigidBody->GetAngularVelocity();

			dN++;
		}


		if (d < Range_FRepel && d > 0)
		{
			displacement += sep.Normalized()/d;
			rN++;
		}




		//if (d < Range_FAttract)
		//{
		//	//with range, so is a neighbour
		//	CoM += neighbour[i].pRigidBody->GetPosition();


		//	n++;
		//}

		//if (d < Range_FAlign)
		//{
		//	avgDir += neighbour[i].pNode->GetDirection();

		//	dN++;
		//}


		//if (d < Range_FRepel)
		//{
		//	displacement -= sep;
		//	rN++;
		//}
	}

	


}

void Boid::AvgForce()
{


	//Attractive force component
	if (n > 0)
	{
		//find average position = centre of mass
		CoM /= n;
		Vector3 dir = (CoM - pRigidBody->GetPosition()).Normalized();
		Vector3 vDesired = dir*FAttract_Vmax;
		force += (vDesired - pRigidBody->GetLinearVelocity())*FAttract_Factor;
	}


	//repel
	if (rN > 0)
	{
		displacement /= rN;
		//Vector3 dis = (displacement + pRigidBody->GetPosition()).Normalized();
		//Vector3 disDesired = dis * FRepel_Factor;
		force += (displacement.Normalized() * FRepel_Factor) - pRigidBody->GetLinearVelocity();
	}


	//alighnment
	if (dN > 0)
	{
		avgDir /= dN;
	//	Vector3 dir = (avgDir - pRigidBody->GetPosition()).Normalized();
	//	Vector3 dirDesired = dir * FAlign_Factor;
	//	force += (dirDesired - pRigidBody->GetLinearVelocity());

		force -= (avgDir.Normalized() * FAlign_Factor) - pRigidBody->GetAngularVelocity();


	}


	////Attractive force component
	//if (n > 0)
	//{
	//	//find average position = centre of mass
	//	CoM /= n;
	//	Vector3 dir = (CoM - pRigidBody->GetPosition()).Normalized();
	//	Vector3 vDesired = dir*FAttract_Vmax;
	//	force += (vDesired - pRigidBody->GetLinearVelocity())*FAttract_Factor;
	//}

	////need to add the other 2 forced (alightment and somethign else??)
	////repel
	//if (rN > 0)
	//{
	//	//displacement /= rN;
	//	//Vector3 dis = (displacement + pRigidBody->GetPosition()).Normalized();
	//	//Vector3 disDesired = dis * FRepel_Factor;
	//	force += displacement * -FRepel_Factor;
	//}


	////alighnment
	//if (dN > 0)
	//{
	//	avgDir /= dN;
	//	Vector3 dir = (avgDir - pRigidBody->GetPosition()).Normalized();
	//	Vector3 dirDesired = dir * FAlign_Factor;
	//	force += (dirDesired - pRigidBody->GetLinearVelocity());



	//}

}


void Boid::Update(float deltaTime)
{
	//if (direction != Vector3::ZERO)
	//	pNode->SetDirection(pNode->GetDirection() + direction);
		


	pRigidBody->ApplyForce(force);
	force = Vector3(0, 0, 0);

	CoM = Vector3(0, 0, 0); //center of mass, accumulated total
	avgDir = Vector3(0, 0, 0);
	displacement = Vector3(0, 0, 0);

	n = 0;
	dN = 0; 
	rN = 0; 

	//limit speed;
	Vector3 vel = pRigidBody->GetLinearVelocity();
	float d = vel.Length();
	if (d < 10.0f)
	{
		d = 10.0f;
		pRigidBody->SetLinearVelocity(vel.Normalized()*d);
	}
	else if (d > 50.0f)
	{
		d = 50.0f;
		pRigidBody->SetLinearVelocity(vel.Normalized()*d);
	}
	Vector3 vn = vel.Normalized();
	Vector3 cp = -vn.CrossProduct(Vector3(0.0f, 1.0f, 0.0f));
	float dp = cp.DotProduct(vn);
	pRigidBody->SetRotation(Quaternion(Acos(dp), cp));
	Vector3 p = pRigidBody->GetPosition();
	if (p.y_ < 10.0f)
	{
		p.y_ = 10.0f;
		pRigidBody->SetPosition(p);
	}
	else if (p.y_ > 50.0f)
	{
		p.y_ = 50.0f;
		pRigidBody->SetPosition(p);
	}


}
