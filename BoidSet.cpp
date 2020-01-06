#include "BoidSet.h"

BoidSet::BoidSet()
{

}


void BoidSet::Initialise(ResourceCache *pRes, Scene *pScene , float x, float z,float cellSize)
{/*
	for (auto &boid : boidList)
	{
		boid.Initialise(pRes,pScene);
	}*/

	Boid initialBoidList[NumBoids];

	float xnum = x / cellSize;
	float znum = z / cellSize;



	//boidList.resize(xnum,std::vector<std::vector<Boid>(Boid())>(znum));
	boidList.resize(xnum);
	for (int i = 0; i<xnum; i++)
	{
		boidList[i].resize(znum);
		for (int j = 0; j<znum; j++)
		{
			//tube[i][j].resize(15, value);
		}
	}


	for (int i = 0; i < NumBoids; i++)
	{
		initialBoidList[i].Initialise(pRes, pScene,x,z,cellSize);

		//update position in grid vector/

		Vector3 pos = initialBoidList[i].pNode->GetPosition();

		float xPos = pos.x_, zPos = pos.z_;

		int vX = xPos / cellSize;
		int vZ = zPos / cellSize;

		boidList[vX][vZ].push_back(initialBoidList[i]);


	}


	//initialBoidList = NULL;

}

void BoidSet::Update(float deltaTime , float xGrid, float zGrid, float cellSize)
{

	const float xnum = xGrid / cellSize;
	const float znum = zGrid / cellSize;


//old method

	//for (int i = 0; i < NumBoids; i++)
	//{

	//	initialBoidList[i].ComputeForce(&initialBoidList[0], NumBoids);

	//}
	//
	//
	//for (auto &boid : initialBoidList)
	//{


	//	boid.Update(deltaTime);

	//}


//new

	
//calculate the forces
	for (int x = 0; x < xnum; x++)
	{


		for (int z = 0; z <znum; z++)
		{


			for (auto &boid : boidList[x][z])
			{

				//boid.ComputeForce(boidList[x][z], boidList[x][z].size());


				for (int i = -1; i < 2; i++)
				{
					for (int j = -1; j < 2; j++)
					{

						int xInd = x + i;
						int zInd = z + i;

						
						if (xInd >= xnum || xInd < 0)						
							continue;
						if (zInd >= znum || zInd < 0)
							continue;
						

						boid.ComputeForce(boidList[xInd][zInd], boidList[xInd][zInd].size());

					}

				}

				boid.AvgForce();



				
			}

		}


	}

	

	//update the objects (pos ect)
	for (int x = 0; x < xnum; x++)
	{


		for (int z =0; z <znum; z++)
		{


			for (auto &boid : boidList[x][z])
			{


				boid.Update(deltaTime);

			}

		}


	}

	

	//re update the vector as a performance saving thing this does nto have to be done everyfram and could later be done every x time

	for (int x = 0; x < xnum; x++)
	{


		for (int z = 0; z <znum; z++)
		{


			std::vector<int> toErase;
			int ie = 0;
	
	

			//std::cout << "BoidList: X: " << x <<"Z: "<< z << std::endl;

			for (auto &boid : boidList[x][z])
			{
		//		std::cout << "Starting Check: " << ie << std::endl;
				//update position in grid vector/

				Vector3 pos = boid.pNode->GetPosition();
				float xPos = pos.x_, zPos = pos.z_;

				int vX = xPos / cellSize;
				int vZ = zPos / cellSize;

				if (x != vX && z != vZ)
				{
					if (vX >= xnum || vX < 0)
					{
						ie++;
						continue;
					}
					if (vZ >= znum || vZ < 0)
					{
						ie++;
						continue;
					}

					boidList[vX][vZ].push_back(boid);				
					toErase.push_back(ie);

				//	std::cout << "Boid moved" << std::endl;

					//boidList[vX][vZ].erase(boid);
				}
				ie++;
				
			}

			int size = toErase.size();

			for (int i = toErase.size(); i > 0; i--)
			{
				int ind = toErase[i - 1];

				boidList[x][z].erase(boidList[x][z].begin() + ind);
			//	std::cout << "Boid Erased" << std::endl;
			}

		}


	}


}