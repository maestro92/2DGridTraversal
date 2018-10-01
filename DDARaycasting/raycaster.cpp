#include "raycaster.h"


Raycaster::Raycaster()
{}

Raycaster::Raycaster(glm::vec2 sourceIn, glm::vec2 dirIn, glm::vec2 endIn, Map* mapIn)
{
	side = NONE;
	map = mapIn;
	cellSize = map->getCellSize();
	init(sourceIn, dirIn, endIn);
}

void Raycaster::init(glm::vec2 sourceIn, glm::vec2 dirIn, glm::vec2 endIn)
{
	source = sourceIn;
	dir = dirIn;
	dir = glm::normalize(dir);
	end = endIn;

	dx = std::abs(cellSize / dir.x);
	dy = std::abs(cellSize / dir.y);

	if (checkForCornerDiagnolCase(source, dir))
	{
		cout << "Corner_Diagnol" << endl;
		traversalCase = TraversalCase::Corner_Diagnol;
	}
	else if(checkForEdgeAlongX(source, dir))
	{
		cout << "Edge_Along_X" << endl;
		traversalCase = TraversalCase::Edge_Along_X;
	}
	else if (checkForEdgeAlongY(source, dir))
	{
		cout << "Edge_Along_Y" << endl;
		traversalCase = TraversalCase::Edge_Along_Y;
	}
	else
	{
		cout << "Regular" << endl;
		traversalCase = TraversalCase::Regular;
	}

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


bool Raycaster::checkForCornerDiagnolCase(glm::vec2 source, glm::vec2 dir)
{
	cout << dir.x << " " << dir.y << endl;
	// (0,0) -> (1,1) or (1,1) -> (0,0) 
	if (abs(dir.x) == abs(dir.y))
	{
		// (0,0) -> (1,1)
		bool flag0 = (dir.x > 0 && dir.y > 0);

		// (1,1) -> (0,0)
		bool flag1 = (dir.x < 0 && dir.y < 0);

		if (flag0 || flag1)
		{
			float localX = source.x - static_cast<int>(source.x);
			float localY = source.y - static_cast<int>(source.y);

			if (localX == localY)
			{
				return true;
			}
		}
	}

	// (0,1) -> (1,0) or (1,0) -> (0,1) 
	if (abs(dir.x) == abs(dir.y))
	{
		// (0,1) -> (1,0)
		bool flag0 = (dir.x < 0 && dir.y > 0);

		// (1,0) -> (0,1)
		bool flag1 = (dir.x > 0 && dir.y < 0);

		if (flag0 || flag1)
		{
			float localX = source.x - static_cast<int>(source.x);
			float localY = source.y - static_cast<int>(source.y);

			if (localX == 0 && localY == 0)
			{
				return true;
			}
			if (localX == 1 - localY)
			{
				return true;
			}
		}
	}
	return false;
}



bool Raycaster::checkForEdgeAlongX(glm::vec2 source, glm::vec2 dir)
{
	// check if source lies on along a NORTH_SOUTH edge of a cell
	// we check it by seeing if y is integer
	bool onHoriCellEdge = source.y == static_cast<int>(source.y);

	if (onHoriCellEdge == true)
	{
		if (dir.y == 0)
		{
			return true;
		}
	}

	return false;
}


bool Raycaster::checkForEdgeAlongY(glm::vec2 source, glm::vec2 dir)
{
	// check if source lies on along a NORTH_SOUTH edge of a cell
	// we check it by seeing if y is integer
	bool onVertCellEdge = source.x == static_cast<int>(source.x);

	if (onVertCellEdge == true)
	{
		if (dir.x == 0)
		{
			return true;
		}
	}

	return false;
}



void Raycaster::traverse()
{
	if (traversalCase == TraversalCase::Regular)
	{
		traverse_Regular();
	}
	else
	{
		// if you are at a exact corner, move a bit along the direction
		bool onCorner = source.x == static_cast<int>(source.x) && 
						source.y == static_cast<int>(source.y);

		if (onCorner)
		{
			cout << "on corner" << endl;

			source += 0.1f * dir;
			curGridCoord = glm::vec2(static_cast<int>(source.x), static_cast<int>(source.y));
		}
		else
		{
			cout << "not on corner" << endl;
		}

		if (traversalCase == TraversalCase::Edge_Along_X)
		{
			// if you are at a exact corner, move a bit along the direction
			traverse_EdgeAlongX();
		}
		else if (traversalCase == TraversalCase::Edge_Along_Y)
		{
			traverse_EdgeAlongY();
		}
		else if (traversalCase == TraversalCase::Corner_Diagnol)
		{
			traverse_CornerDiagnol();
		}
	}
}







void Raycaster::traverse_Regular()
{
	glm::vec2 destGridCoord = glm::vec2(static_cast<int>(end.x), static_cast<int>(end.y));

	traversal.clear();
	traversal.push_back(curGridCoord);

	bool running = true;
	while (running == true)
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

void Raycaster::traverse_EdgeAlongX()
{
	glm::vec2 destGridCoord = glm::vec2(static_cast<int>(end.x), static_cast<int>(end.y));

	traversal.clear();

	bool running = true;
	while (running == true)
	{
		glm::vec2 gc0 = curGridCoord;
		glm::vec2 gc1 = gc0;
		gc1.y -= 1;

		bool isWall0 = checkWall(gc0);
		bool isWall1 = checkWall(gc1);
		
		bool isEnd0 = checkReachedDestination(gc0, destGridCoord);
		bool isEnd1 = checkReachedDestination(gc1, destGridCoord);
		
		traversal.push_back(gc0);
		if (isEnd0) 
		{  
			break; 
		}

		traversal.push_back(gc1);
		if (isEnd1) 
		{  
			break; 
		}

		if (isWall0 && isWall1)
		{
			break;
		}

		curGridCoord.x += stepX;
	}
}

void Raycaster::traverse_EdgeAlongY()
{
	glm::vec2 destGridCoord = glm::vec2(static_cast<int>(end.x), static_cast<int>(end.y));

	traversal.clear();

	bool running = true;
	while (running == true)
	{
		glm::vec2 gc0 = curGridCoord;
		glm::vec2 gc1 = gc0;
		gc1.x -= 1;

		bool isWall0 = checkWall(gc0);
		bool isWall1 = checkWall(gc1);

		bool isEnd0 = checkReachedDestination(gc0, destGridCoord);
		bool isEnd1 = checkReachedDestination(gc1, destGridCoord);

		traversal.push_back(gc0);
		if (isEnd0)
		{
			break;
		}

		traversal.push_back(gc1);
		if (isEnd1)
		{
			break;
		}

		if (isWall0 && isWall1)
		{
			break;
		}

		curGridCoord.y += stepY;
	}
}

void Raycaster::traverse_CornerDiagnol()
{
	cout << curGridCoord.x << " " << curGridCoord.y << endl;

	glm::vec2 destGridCoord = glm::vec2(static_cast<int>(end.x), static_cast<int>(end.y));

	traversal.clear();
	traversal.push_back(curGridCoord);
	if (checkTerminateCondition(curGridCoord, destGridCoord))
	{
		return;
	}


	bool running = true;
	while (running == true)
	{
		glm::vec2 gc0 = curGridCoord;
		gc0.x += stepX;

		glm::vec2 gc1 = curGridCoord;
		gc1.y += stepY;

		glm::vec2 gc2 = curGridCoord;
		gc2.x += stepX;	gc2.y += stepY;

		bool isWall0 = checkWall(gc0);
		bool isWall1 = checkWall(gc1);
		bool isWall2 = checkWall(gc2);

		bool isEnd0 = checkReachedDestination(gc0, destGridCoord);
		bool isEnd1 = checkReachedDestination(gc1, destGridCoord);
		bool isEnd2 = checkReachedDestination(gc2, destGridCoord);

		traversal.push_back(gc0);
		if (isEnd0) 
		{ 
			break; 
		}

		traversal.push_back(gc1);
		if (isEnd1) 
		{  
			break; 
		}
		
		if (isWall0 && isWall1)
		{
			traversal.push_back(gc0);
			traversal.push_back(gc1);
			break;
		}

		traversal.push_back(gc2);
		if (isWall2 || isEnd2)
		{
			break;
		}
		
		curGridCoord = gc2;
	}
}