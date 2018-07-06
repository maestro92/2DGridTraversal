#pragma once

#include "utility_math.h"
#include <vector>
#include <functional>
#include "utility_debug.h"
#include <iostream>
#include <fstream>
#include "map.h"
using namespace std;

struct Raycaster
{
	enum CellSide
	{
		NONE = -1,
		NORTH_SOUTH = 0,
		EAST_WEST
	};
	
	enum TraversalCase
	{
		Regular = 0,
		Edge_Along_X,	// (1,1) -> (5,1)
		Edge_Along_Y,	// (1,1) -> (1,5)
		Corner_Diagnol			// (0,0) -> (5,5)	
	};
	// my understanding is that 
/*
assume Diagnol case

	_ _ _ 
	W _ _ 
	_ W _

assume we start at left bottom corner (0,0),
if both (0,1) and (1,0) are wall, we block
if one of them is not wall, we let the ray through

same thing for EDGE_ALONG_X and EDGE_ALONG_Y case

assme we are going along y axis
we block in this case

	W W _
	W W _
	_ _ _

we let the ray through in the following two cases

	W _ _
	W _ _
	_ _ _

	_ W _
	_ W _
	_ _ _

*/

	TraversalCase traversalCase;

	glm::vec2 source;
	
	glm::vec2 curGridCoord;
	
	glm::vec2 dir;
	glm::vec2 end;

	Map* map;

	vector<glm::vec2> traversal;

	// length of ray from current pos to the next x or y side
	float rayLengthToSideX;
	float rayLengthToSideY;

	// length of ray to traverse a cell's size in x or y direction
	float dx;
	float dy;

	// did we hit a wall
	int hitWall;
	int reachedEnd;

	int cellSize;

	// which way along X axis we are stepping, 1 or -1. same thing with stepY
	int stepX;
	int stepY;

	// did we hit a North or South wall (hit a wall along x axis)
	// or did we hit a East or West wall (hit a wall along y axis)
	CellSide side;

	Raycaster();
	Raycaster(glm::vec2 sourceIn, glm::vec2 dirIn, glm::vec2 endIn, Map* mapIn);
	void init(glm::vec2 sourceIn, glm::vec2 dirIn, glm::vec2 endIn);
	bool checkForEdgeAlongX(glm::vec2 source, glm::vec2 dir);
	bool checkForEdgeAlongY(glm::vec2 source, glm::vec2 dir);
	bool checkForCornerDiagnolCase(glm::vec2 source, glm::vec2 dir);

	/*
	the idea of this traversal is that, we are always comparing rayLengthToSideX and rayLengthToySideY.
	one important thing is that, these two are always calculated from the source. source never changes.

	so when we go into, for example
			
			if (rayLengthToSideX < rayLengthToSideY)
			{
				rayLengthToSideX += dx;
				curGridCoord.x += stepX;
				side = NORTH_SOUTH;
			}
	
	we only just increment the rayLengthToSideX += dx. By doing this, we get the distance for rayLengthToSideX for our next sideX.
	and not do anything with rayLengthToSideY. rayLengthToSideY is still valid since it is the distance from source.

	you can think of it that we are not actually traversing. We are just calculating our traversal by calculating the distance to the nextX or nextY.
	*/
	void traverse();
	void traverse_Regular();
	void traverse_EdgeAlongX();
	void traverse_EdgeAlongY();
	void traverse_CornerDiagnol();

	bool checkTerminateCondition(glm::vec2 testGridCoord, glm::vec2 destGridCoord)
	{
		return checkWall(testGridCoord) || checkReachedDestination(testGridCoord, destGridCoord);
	}

	bool checkWall(glm::vec2 testGridCoord)
	{
		if (map->IsValidRange(testGridCoord) == false)
		{
			return true;
		}

		if (map->getCell(testGridCoord) == Map::Cell::Wall)
		{
			return true;
		}
		return false;
	}

	bool checkReachedDestination(glm::vec2 testGridCoord, glm::vec2 destGridCoord)
	{
		if (testGridCoord.x == destGridCoord.x && testGridCoord.y == destGridCoord.y)
		{
			return true;
		}
		return false;
	}


	void printTraversal()
	{
		cout << "Printing Traversals" << endl;
		for (int i = 0; i < traversal.size(); i++)
		{
			cout << traversal[i].x << " " << traversal[i].y << endl;
		}
		cout << endl << endl;
	}

	void reset()
	{
		traversal.clear();
	}
};
