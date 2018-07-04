#ifndef MAP_VIEW_H_
#define MAP_VIEW_H_

#include "utility_sdl.h"
#include "map.h"
#include "world_object.h"
#include "model_manager.h"
#include "renderer.h"
#include "renderer_manager.h"

class MapView
{
	public:
		void init(Map* map);
		void initGridLines();
		void render(Pipeline& p);
		void reset();
		
		void createMeshForGridCellsHighlight(vector<glm::vec2> traversal);
		void renderHighlight(Pipeline& p);
		void resetHighlight();
		WorldObject& getWorldObject(int x, int y);

	private:

		Map * m_map;

		Renderer*					p_renderer;
		vector< vector<WorldObject> > gridCells;
		
		Model* gridLineModels;			
		WorldObject gridLines;

		WorldObject highlight;
		Model* highlightModel;


		/*
		vector<WorldObject> matchIndicators;
		vector<WorldObject> potMatchIndicators;

		GLuint blueGemTexture;
		GLuint redGemTexture;
		GLuint orangeGemTexture;
		GLuint purpleGemTexture;
		GLuint greenGemTexture;
		*/
};


#endif
