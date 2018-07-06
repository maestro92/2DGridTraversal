#include "map.h"


void Map::init(int w, int h)
{
	m_cellSize = 1;

	for (int y = 0; y < h; y++)
	{
		vector<Cell> row(w);
		gridmap.push_back(row);
	}

	
	srand(time(NULL));
	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			gridmap[y][x] = static_cast<Map::Cell>((utl::randInt(0, (int)(Cell::NUM_TYPES))));
		}
	}

	gridmap[4][4] = Map::Cell::Empty;
	gridmap[4][5] = Map::Cell::Wall;
	gridmap[5][4] = Map::Cell::Wall;

	/*
	// testing along Edge Y axis
	gridmap[0][0] = Map::Cell::Wall;
	gridmap[1][0] = Map::Cell::Wall;
	gridmap[2][0] = Map::Cell::Wall;
	gridmap[3][0] = Map::Cell::Wall;
	gridmap[4][0] = Map::Cell::Wall;

	gridmap[1][1] = Map::Cell::Empty;
	gridmap[2][1] = Map::Cell::Empty;
	gridmap[3][1] = Map::Cell::Empty;
	gridmap[4][1] = Map::Cell::Empty;
	*/

	if (saveLatest)
	{
		save();
	}

}


bool Map::IsValidRange(glm::vec2 coord)
{
	if (coord.x < 0)	return false;
	if (coord.y < 0)	return false;
	if (coord.x >= getWidth())	return false;
	if (coord.y >= getHeight())	return false;
	return true;
}



int Map::getWidth()
{
	return gridmap[0].size();
}

int Map::getHeight()
{
	return gridmap.size();
}

Map::Cell Map::getCell(int x, int y)
{
	return gridmap[y][x];
}

Map::Cell Map::getCell(glm::vec2 coord)
{
	return gridmap[coord.y][coord.x];
}


void Map::debug()
{
	for (int y = 0; y < getHeight(); y++)
	{
		string s = "";
		for (int x = 0; x < getWidth(); x++)
		{
			s += utl::intToStr((int)(gridmap[y][x]));
		}
		cout << s << endl;
	}
	cout << endl;
}


void Map::resetFlags()
{

}


void Map::reset()
{

}


Object Map::serializeCell(Map::Cell gem)
{
	Object pointObj;

	pointObj.push_back(Pair("cell", (int)gem));

	return pointObj;
}


Map::Cell Map::deserializeCell(const mObject& obj)
{
	int intCell = utl::findValue(obj, "cell").get_int();
	return (Map::Cell)intCell;
}



float Map::getCellSize()
{
	return m_cellSize;
}


// http://www.cplusplus.com/forum/beginner/60827/
// c++ truncates, aka rounds down
glm::vec2 Map::worldPos2GridCoord(glm::vec2 pos)
{
	return glm::vec2(static_cast<int>(pos.x), static_cast<int>(pos.y));
}

glm::vec2 Map::gridCoord2WorldPos(glm::vec2 gridCoord)
{
	return gridCoord;
}

glm::vec2 Map::getCellMinCorner(glm::vec2 gridCoord)
{
	return gridCoord;
}

glm::vec2 Map::getCellMaxCorner(glm::vec2 gridCoord)
{
	glm::vec2 pos = gridCoord;
	pos.x += m_cellSize;
	pos.y += m_cellSize;
	return pos;
}

glm::vec2 Map::getCellCenter(glm::vec2 gridCoord)
{
	glm::vec2 pos = gridCoord;
	pos.x += m_cellSize / 2;
	pos.y += m_cellSize / 2;
	return pos;
}



void Map::save()
{
	ofstream myfile;
	myfile.open("data.txt");
	cout << "saving data " << endl;


	Object boardObj;
	boardObj.push_back(Pair("h", (int)gridmap.size()));
	boardObj.push_back(Pair("w", (int)gridmap[0].size()));

	Array array;
	for (int y = 0; y < gridmap.size(); y++)
	{
		Array row;
		for (int x = 0; x < gridmap[0].size(); x++)
		{
			Object vObj = serializeCell(gridmap[y][x]);
			row.push_back(vObj);
		}

		array.push_back(row);
	}

	boardObj.push_back(Pair("gridmap", array));
	write(boardObj, myfile, pretty_print);
	myfile.close();
}




void Map::load(char* filename)
{
	mValue content = utl::readJsonFileToMap(filename);

	const mObject& obj = content.get_obj();

	int w = utl::findValue(obj, "w").get_int();
	int h = utl::findValue(obj, "h").get_int();

	cout << w << " " << h << endl;

	vector<vector<Cell>> newMap;

	for (int y = 0; y < h; y++)
	{
		vector<Cell> row(w);
		newMap.push_back(row);
	}

	const mArray& boardArray = utl::findValue(obj, "gridmap").get_array();
	for (int y = 0; y < h; y++)
	{
		const mArray& row = boardArray[y].get_array();
		vector<Cell> gemRow(w);
		for (int x = 0; x < w; x++)
		{
			const mObject obj = row[x].get_obj();
			Cell gem = deserializeCell(obj);
			newMap[y][x] = gem;
		}
	}

	/*
	cout << "printing newBoard" << endl;
	for (int y = 0; y < h; y++)
	{
		string s = "";
		for (int x = 0; x < w; x++)
		{
			s += utl::intToStr((int)(newBoard[y][x]));
		}
		cout << s << endl;
	}
	cout << "end newBoard" << endl;
	*/
	gridmap = newMap;

//	debug();
}


void Map::debugVisitedFlags()
{

}


