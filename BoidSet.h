#pragma once
#include "Boid.h"
#include <vector>
#include <iostream>



class BoidSet
{
  
	const static int NumBoids = 500;
	
	public:
	
	

	std::vector<std::vector<std::vector<Boid>>> boidList;

	BoidSet();

	void Initialise(ResourceCache *pRes, Scene *pScene, float x, float z, float cellSize);
	void Update(float tm, float xGrid, float zGrid, float cellSize);
};