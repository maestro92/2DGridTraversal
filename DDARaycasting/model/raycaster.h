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

	Raycaster()
	{}

	Raycaster(glm::vec2 sourceIn, glm::vec2 dirIn, glm::vec2 endIn, Map* mapIn)
	{
		side = NONE;
		map = mapIn;
		cellSize = map->getCellSize();
		init(sourceIn, dirIn, endIn);
	}

	void init(glm::vec2 sourceIn, glm::vec2 dirIn, glm::vec2 endIn)
	{
		source = sourceIn;
		dir = dirIn;
		dir = glm::normalize(dir);
		end = endIn;

		dx = std::abs(cellSize / dir.x);
		dy = std::abs(cellSize / dir.y);

		curGridCoord = glm::vec2(static_cast<int>(source.x), static_cast<int>(source.y));

		// calculate step and initail rayLength parameters
		if (dir.x < 0)
		{
			stepX = -1;
			rayLengthToSideX = (source.x - curGridCoord.x) * dx;
		}
		else
		{
			stepX = 1;
			rayLengthToSideX = (curGridCoord.x + 1 - source.x) * dx;
		}

		if (dir.y < 0)
		{
			stepY = -1;
			rayLengthToSideY = (source.y - curGridCoord.y) * dy;
		}
		else
		{
			stepY = 1;
			rayLengthToSideY = (curGridCoord.y + 1 - source.y) * dy;
		}
	}


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
	void traverse_edgeInclusive()
	{
		glm::vec2 destGridCoord = glm::vec2(static_cast<int>(end.x), static_cast<int>(end.y));

		traversal.clear();
		traversal.push_back(curGridCoord);

		bool running = true;
		while (running == true)
		{
			// edge case, we wil check both for the edgeInclusive function
			if (rayLengthToSideX == rayLengthToSideY)
			{
				glm::vec2 tempCurGrid = curGridCoord;
				tempCurGrid.x += stepX;
				traversal.push_back(tempCurGrid);
				side = NORTH_SOUTH;
				if (checkTerminateCondition(tempCurGrid, destGridCoord))
				{
					running = false;
					break;
				}

				tempCurGrid = curGridCoord;
				tempCurGrid.y += stepY;
				traversal.push_back(tempCurGrid);
				side = EAST_WEST;
				if (checkTerminateCondition(tempCurGrid, destGridCoord))
				{
					running = false;
					break;
				}

				rayLengthToSideX += dx;
				curGridCoord.x += stepX;
				rayLengthToSideY += dy;
				curGridCoord.y += stepY;
				traversal.push_back(curGridCoord);
				side = NORTH_SOUTH;
				if (checkTerminateCondition(curGridCoord, destGridCoord))
				{
					running = false;
					break;
				}
			}
			else
			{
				if (rayLengthToSideX < rayLengthToSideY)
				{
					rayLengthToSideX += dx;
					curGridCoord.x += stepX;
					side = NORTH_SOUTH;
				}
				else
				{
					rayLengthToSideY += dy;
					curGridCoord.y += stepY;
					side = EAST_WEST;
				}

				traversal.push_back(curGridCoord);
				if (checkTerminateCondition(curGridCoord, destGridCoord))
				{
					running = false;
					break;
				}
			}
		}
	}



	void traverse_edgeExclusive()
	{
		glm::vec2 destGridCoord = glm::vec2(static_cast<int>(end.x), static_cast<int>(end.y));

		traversal.clear();
		traversal.push_back(curGridCoord);

		cout << curGridCoord.x << " " << curGridCoord.y << endl;

		bool running = true;
		while (running == true)
		{
			// edge case
			if (rayLengthToSideX == rayLengthToSideY)
			{
				cout << curGridCoord.x << " " << curGridCoord.y << endl;

				glm::vec2 tempCurGrid = curGridCoord;
				tempCurGrid.x += stepX;
				traversal.push_back(tempCurGrid);
				if (checkReachedDestination(tempCurGrid, destGridCoord))
				{
					running = false;
					break;
				}


				tempCurGrid = curGridCoord;
				tempCurGrid.y += stepY;
				traversal.push_back(tempCurGrid);
				if (checkReachedDestination(tempCurGrid, destGridCoord))
				{
					running = false;
					break;
				}

				rayLengthToSideX += dx;
				curGridCoord.x += stepX;
				rayLengthToSideY += dy;
				curGridCoord.y += stepY;
				traversal.push_back(curGridCoord);
				side = NORTH_SOUTH;
				if (checkTerminateCondition(curGridCoord, destGridCoord))
				{
					running = false;
					break;
				}
			}
			else
			{
				if (rayLengthToSideX < rayLengthToSideY)
				{
					rayLengthToSideX += dx;
					curGridCoord.x += stepX;
					side = NORTH_SOUTH;
				}
				else
				{
					rayLengthToSideY += dy;
					curGridCoord.y += stepY;
					side = EAST_WEST;
				}

				traversal.push_back(curGridCoord);
				if (checkTerminateCondition(curGridCoord, destGridCoord))
				{
					running = false;
					break;
				}
			}
		}
	}

	bool checkTerminateCondition(glm::vec2 testGridCoord, glm::vec2 destGridCoord)
	{
		return checkWall(testGridCoord) || checkReachedDestination(testGridCoord, destGridCoord);
	}

	bool checkWall(glm::vec2 testGridCoord)
	{
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

	/*
	// essentially the algorithm mentioned here
	// https://lodev.org/cgtutor/raycasting.html
	void raycast(glm::vec2 source, glm::vec2 dst)
	{
		glm::vec2 dir = dst - source;
		Raycaster raycaster(source, dir, dst, map->getCellSize());
		raycaster.traverse();

		raycaster.printTraversal();
	}
	*/



	/*

	// length of ray from current pos to the next x or y side
	float rayLengthToSideX;
	float rayLengthToSideY;

	// distance to next x or y
	float dx = std::abs(1 / dir.x);
	float dy = std::abs(1 / dir.y);

	// did we hit a wall
	int hitWall = 0;

	// which way along X axis we are stepping, 1 or -1. same thing with stepY
	int stepX;
	int stepY;

	// did we hit a North or South wall (hit a wall along x axis)
	// or did we hit a East or West wall (hit a wall along y axis)
	int side = -1;
	*/

};
