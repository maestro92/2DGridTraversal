#include "main.h"


#include <stdio.h>
#include <string.h>

#include "global.h"

#define RENDER_DEBUG_FLAG 0



float CAMERA_POS_DELTA = 1;
float CAMERA_ZOOM_DELTA = 1;

#define PI 3.14159265

// the server simluates the game in descirete time steps called ticks


// http://stackoverflow.com/questions/4845410/error-lnk2019-unresolved-external-symbol-main-referenced-in-function-tmainc
#undef main


using namespace std;
//using namespace std::placeholders;
// https://www.youtube.com/watch?v=tlXM8qDOS3U
// Screen dimension constants


// frame rate
// https://sites.google.com/site/sdlgamer/intemediate/lesson-7
// FPS is 50
// So Interval is 1000/50 which is 20ms
// meaning my while loop runs 50 frames per second
// which is every other 20 ms
const int FRAMES_PER_SECOND = 60;
const int INTERVAL = 1000 / FRAMES_PER_SECOND;

//display surface
SDL_Surface* pDisplaySurface = NULL;
//event structure
SDL_Event event;


const float GAME_SPEED = 1.0f;
const float _FIXED_UPDATE_TIME_s = 0.01667f;
const float FIXED_UPDATE_TIME_s = _FIXED_UPDATE_TIME_s / GAME_SPEED;
const float FIXED_UPDATE_TIME_ms = FIXED_UPDATE_TIME_s * 1000;

const float MOUSE_DIST_THRESHOLD = 0.05;


const int SV_FRAMES_PER_SECOND = 20;
const float SV_FIXED_UPATE_TIME_s = 1 / SV_FRAMES_PER_SECOND;
const long long SV_FIXED_UPATE_TIME_ms = 1000 / SV_FRAMES_PER_SECOND;

const long long CLIENT_INTERP_DELAY_ms = SV_FIXED_UPATE_TIME_ms * 2;

// 15 ms, 66.6 ticks per sec are simulated
const int SERVER_SIMLUATION_FRAMES_PER_SECOND = 66;
const int SERVER_SIMLUATION_TIME_STEP = 1000 / SERVER_SIMLUATION_FRAMES_PER_SECOND;

// 50 ms, 20 snapshots per second
const int SERVER_SNAPSHOT_PER_SECOND = 20;
const int SERVER_SNAPSHOT_TIME_STEP = 1000 / SERVER_SNAPSHOT_PER_SECOND;

// But instead of sending a new packet to the server for each user command, the client sends command packets at a certain rate of packets per second (usually 30).
// This means two or more user commands are transmitted within the same packet.
const int CLIENT_INPUT_SENT_PER_SECOND = 33;
const int CLIENT_INPUT_SENT_TIME_STEP = 1000 / SERVER_SNAPSHOT_PER_SECOND;


const float SPAWN_POSITION_UNIT_OFFSET = 40.0f;

const int INVALID_OBJECT = 0x7FFFFFFF;

// link
// http://lodev.org/cgtutor/raycasting.html

DDARaycasting::~DDARaycasting()
{

}


void DDARaycasting::init()
{
	frameNum = 0;

	global.modelMgr = new ModelManager();
	global.modelMgr->init();

	// renderMgr has to init after the lightMgr
	global.rendererMgr = new RendererManager();
	global.rendererMgr->init(utl::SCREEN_WIDTH, utl::SCREEN_HEIGHT);


	isRunning = true;


	latencyOptions = { 0, 20, 50, 100, 200 };	// millisecond
	curLatencyOption = latencyOptions.size()-1;
	latency = latencyOptions[curLatencyOption] / 2;

	curLatencyOption = 0;

	containedFlag = false;

	timeProfilerIndex = 0;
	fpsProfilerIndex = 0;



	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// initGUI depends on the m_defaultPlayerID, so initNetworkLobby needs to run first
	initGUI();

	for (int i = 0; i < FPS_PROFILER_BUFFER; i++)
	{
		fpsProfiler[i] = 0;
	}


	//Initialize clear color
	glClearColor(0.0f, 0.5f, 0.0f, 1.0f);

	m_pipeline.setMatrixMode(PROJECTION_MATRIX);
	m_pipeline.loadIdentity();
//	m_pipeline.perspective(90, utl::SCREEN_WIDTH / utl::SCREEN_HEIGHT, utl::Z_NEAR, utl::Z_FAR);

	m_cameraZoom = 8;
	m_cameraCenter = glm::vec2(5, 5);

	updateCamera();

	ModelManager::enableVertexAttribArrays();

	glCullFace(GL_BACK);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	SDL_WM_SetCaption("MatchThree", NULL);


	loadData = false;

	float scale = 100.0;
	o_worldAxis.setScale(scale);
	o_worldAxis.setModel(global.modelMgr->get(ModelEnum::xyzAxis));


	if (loadData)
	{
		map.saveLatest = false;
		map.load("board1.txt");
		mapView.init(&map);
		debugDrawing();
	}
	else
	{
		map.saveLatest = true;
		map.init(10, 12);
		mapView.init(&map);
		debugDrawing();

	}
}

GLuint tempTexture;








void DDARaycasting::debugDrawing()
{
	m_gui.removeDebugLabels();
	float size = 15;
	//	float size = m_cameraZoom * 0.8;
	//	float size = 300 / m_cameraZoom;

	for (int y = 0; y < map.getHeight(); y++)
	{
		for (int x = 0; x < map.getWidth(); x++)
		{
			glm::vec3 pos = mapView.getWorldObject(x, y).m_position;
			glm::vec3 screenPos = worldToScreen(glm::vec3(pos.x, pos.y, 0));
			glm::vec3 labelPos = screenToUISpace(glm::vec2(screenPos.x, screenPos.y));

			string s = utl::intToStr(x) + " " + utl::intToStr(y);

			Label* coordLabel = new Label(s, labelPos.x + 20, labelPos.y, 0, 0, COLOR_WHITE);
			coordLabel->setFont(size, COLOR_BLACK);
			m_gui.addDebugLabel(coordLabel);
		}
	}
}




void DDARaycasting::initObjects()
{


}




/*
The client creates user commands from sampling input devices with the same tick rate that the server is running 
with. 

Instead of sending a new packet to the server for each user command, the client sends command packets at a certain rate of packets per second (usually 30).
This means two or more user commands are transmitted within the same packet. The Clients can increase the command rate with c1-cmdrate. This will increase
responsiveness but requires more outgoing bandwidth, too. 

https://developer.valvesoftware.com/wiki/Source_Multiplayer_Networking
*/
void DDARaycasting::clientFrame(long long dt)
{
	render();	
}



void DDARaycasting::GetTimeProfilerAverages()
{
	
	long long total = 0;

	for (int i = 0; i < TIME_PROFILER_BUFFER; i++)
	{
		total += timeProfiler[i];
	}

	cout << "average is " << total / TIME_PROFILER_BUFFER << endl;
	
	/*
	uint64 total = 0;

	for (int i = 0; i < TIME_PROFILER_BUFFER; i++)
	{
		total += timeProfiler[i];
	}

	cout << "average is " << total / TIME_PROFILER_BUFFER << endl;
	*/
	
	//	utl::debug()
}


void DDARaycasting::start()
{
	cout << "Start" << endl;

	long long dt = 0;
	long long oldTime = utl::getCurrentTime_ms(); 
	long long newTime = 0;
	
	Uint32 time0 = 0;
	Uint32 time1 = 0;
	
	while (isRunning)
	{
		// cout << "frame" << endl;
		time0 = SDL_GetTicks();

		newTime = utl::getCurrentTime_ms();

		dt = newTime - oldTime;
		// cout << "update" << endl;
		update();



		clientFrame(dt);


		oldTime = newTime;
		
		time1 = SDL_GetTicks();
		
		
		// cout << fpsProfilerIndex << endl;
		if (fpsProfilerIndex == FPS_PROFILER_BUFFER)
		{
			fpsProfilerIndex = 0;
		}
		fpsProfiler[fpsProfilerIndex] = (int)(time1 - time0);
		++fpsProfilerIndex;
		
		int fps = getAverageFPS();
		// cout << fps << endl;
		
		/*
		++fpsProfilerIndex;
		if (fpsProfilerIndex > 1000)
		{
			fpsProfilerIndex = 0;
		}
		*/
		
		// fpsProfilerIndex = 1;
		m_gui.setFPS(fps);
	}
}


int DDARaycasting::getAverageFPS()
{
	float averageFrameTime = 0;
	for (int i = 0; i < FPS_PROFILER_BUFFER; i++)
	{
		averageFrameTime += fpsProfiler[i];
	}

	if (averageFrameTime == 0)
	{
		return 0;
	}
	else
	{
		averageFrameTime = averageFrameTime / FPS_PROFILER_BUFFER;

		int fps = 1000 / averageFrameTime;

	//	cout << averageFrameTime << " " << fps << endl;
		return fps;
	}
}


void DDARaycasting::updateCamera()
{
	m_pipeline.ortho(m_cameraCenter.x - m_cameraZoom, 
					m_cameraCenter.x + m_cameraZoom,
					m_cameraCenter.y - m_cameraZoom,
					m_cameraCenter.y + m_cameraZoom, utl::Z_NEAR, utl::Z_FAR);

//	debugDrawing(curDrawing);
}



// VBO with dynamically changing number of points
// https://www.opengl.org/discussion_boards/showthread.php/178828-VBO-with-dynamically-changing-number-of-points-%21%21
// will need to store it both CPU and GPU
// need it on CPU to process enclosed data
// need it on GPU for rendering
// thickness is inversely proportional


// method1: load your sprites, then render them as textured quad.
// method2: glBufferData Way





glm::vec3 DDARaycasting::screenToWorldPoint(glm::vec2 screenPoint)
{
	glm::vec4 viewPort = glm::vec4(0, 0, utl::SCREEN_WIDTH, utl::SCREEN_HEIGHT);
	glm::vec3 temp = glm::vec3(screenPoint.x, screenPoint.y, 0);

	glm::vec3 worldPoint = glm::unProject(temp, (m_pipeline.getModelViewMatrix()), m_pipeline.getProjectionMatrix(), viewPort);
	return worldPoint;
}


glm::vec3 DDARaycasting::screenToUISpace(glm::vec2 screenPoint)
{
	glm::vec4 viewPort = glm::vec4(0, 0, utl::SCREEN_WIDTH, utl::SCREEN_HEIGHT);
	glm::vec3 temp = glm::vec3(screenPoint.x, screenPoint.y, 0);

	glm::vec3 worldPoint = glm::unProject(temp, (m_gui.getPipeline().getModelViewMatrix()), m_gui.getPipeline().getProjectionMatrix(), viewPort);
	
	return worldPoint;
}


glm::vec3 DDARaycasting::worldToScreen(glm::vec3 pos)
{
	glm::vec4 viewPort = glm::vec4(0, 0, utl::SCREEN_WIDTH, utl::SCREEN_HEIGHT);
//	glm::vec3 screenPos = glm::project(pos, glm::inverse(m_pipeline.getModelViewMatrix()), m_pipeline.getProjectionMatrix(), viewPort);
	glm::vec3 screenPos = glm::project(pos, m_pipeline.getModelViewMatrix(), m_pipeline.getProjectionMatrix(), viewPort);
	return screenPos;
}




void DDARaycasting::update()
{
	int mx, my;
	SDL_GetMouseState(&mx, &my);

	// need this for GUI
	m_mouseState.m_pos = glm::vec2(mx, utl::SCREEN_HEIGHT - my);


	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{


			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
					case SDLK_ESCAPE:
						isRunning = false;
						break;

					case SDLK_q:
						m_cameraZoom -= CAMERA_ZOOM_DELTA;
						updateCamera();

						break;

					case SDLK_v:
				//		map.findMatches();
						break;

					case SDLK_t:
					//	gameBoard.findMatches_BareBears();
						break;

					case SDLK_y:

						break;


					case SDLK_w:
						m_cameraZoom -= CAMERA_ZOOM_DELTA;
						updateCamera();
						break;

					case SDLK_e:
						/*
						if (startedCurrentLine)
						{
							onMouseBtnHold();
						}
						*/

						break;
					case SDLK_x:
						break;
					case SDLK_z:
						utl::debug("In here");
						inDrawingMode = !inDrawingMode;

						if (inDrawingMode)
						{
							resetDrawingMode();
						}
						else
						{

						}

						m_gui.setDrawingModeFlag(inDrawingMode);
						break;

					case SDLK_UP:
					//	m_cameraCenter.y += CAMERA_POS_DELTA;
						updateCamera();
						break;
					case SDLK_DOWN:
					//	m_cameraCenter.y -= CAMERA_POS_DELTA;
						updateCamera();
						break;
					case SDLK_LEFT:
					//	m_cameraCenter.x -= CAMERA_POS_DELTA;
						updateCamera();
						break;
					case SDLK_RIGHT:
					//	m_cameraCenter.x += CAMERA_POS_DELTA;
						updateCamera();
						break;

					default:
						break;
				}
				break;
			
			case SDL_MOUSEBUTTONDOWN:
				switch (event.button.button)
				{
					case SDL_BUTTON_LEFT:
						onMouseBtnDown();
						break;
					case SDL_BUTTON_WHEELUP:
						// m_cameraZoom -= CAMERA_ZOOM_DELTA;
						updateCamera();
						break;
					case SDL_BUTTON_WHEELDOWN:
						// m_cameraZoom += CAMERA_ZOOM_DELTA;
						updateCamera();
						break;
				}

				break;

			case SDL_MOUSEBUTTONUP:
				switch (event.button.button)
				{
					case SDL_BUTTON_LEFT:
						onMouseBtnUp();
						break;
				}



				break;
		}
	}



	if (inDrawingMode && hasDrawnSource && hasDrawnEnd == false)
	{
		onMouseBtnHold();
	}
}

void DDARaycasting::resetDrawingMode()
{
	hasDrawnSource = false;
	hasDrawnEnd = false;
	mapView.resetHighlight();
}





void DDARaycasting::onMouseBtnUp()
{
	int tmpx, tmpy;
	SDL_GetMouseState(&tmpx, &tmpy);
	tmpy = utl::SCREEN_HEIGHT - tmpy;

	if (inDrawingMode)
	{
		glm::vec2 screenPoint = glm::vec2(tmpx, tmpy);

		glm::vec3 worldPoint = screenToWorldPoint(screenPoint);
		glm::vec2 tempWorldPoint = glm::vec2(worldPoint.x, worldPoint.y);

		if (hasDrawnSource == false)
		{
			source = tempWorldPoint;
			hasDrawnSource = true;
			currentRay = constructLine(source, source, 0.05);
		}
		else
		{
			hasDrawnEnd = true;
			end = tempWorldPoint;
			UpdatingCurrentRayNewEndPoint(end);

			glm::vec2 dir = end - source;

			Raycaster raycaster(source, dir, end, &map);
//			raycaster.traverse_edgeExclusive();
			raycaster.traverse_edgeInclusive();

			raycaster.printTraversal();

			glm::vec2 endPoint = raycaster.traversal[raycaster.traversal.size() - 1];
			traversalRay = constructLine(source, map.getCellCenter(endPoint), 0.05);

			mapView.createMeshForGridCellsHighlight(raycaster.traversal);
		}
	}
}


void DDARaycasting::onMouseBtnHold()
{
	int tmpx, tmpy;
	SDL_GetMouseState(&tmpx, &tmpy);
	tmpy = utl::SCREEN_HEIGHT - tmpy;

	glm::vec2 screenPoint = glm::vec2(tmpx, tmpy);
	glm::vec3 worldPoint = screenToWorldPoint(screenPoint);
	glm::vec2 tempWorldPoint = glm::vec2(worldPoint.x, worldPoint.y);

	UpdatingCurrentRayNewEndPoint(tempWorldPoint);
}


void DDARaycasting::onMouseBtnDown()
{
	if (inDrawingMode == true)
	{
		if (hasDrawnSource == true && hasDrawnEnd == true)
		{
			resetDrawingMode();
		}
	}

}


void DDARaycasting::UpdatingCurrentRayNewEndPoint(glm::vec2 end)
{
	glm::vec2 p0 = source;
	glm::vec2 p1 = end;

	glm::vec2 diffVector = p1 - p0;
	glm::vec2 centerPoint = p0 + glm::vec2(diffVector.x / 2.0, diffVector.y / 2.0);

	currentRay->setPosition(glm::vec3(centerPoint.x, centerPoint.y, 0));

	float angle = atan2(diffVector.y, diffVector.x) * 180 / PI;

	float length = glm::distance(p0, p1);

	glm::vec3 scale(length, currentRay->getScale().y, 1);

	currentRay->setRotation(glm::rotate(angle, 0.0f, 0.0f, 1.0f));

	currentRay->setScale(scale);
}




WorldObject* DDARaycasting::constructLine(glm::vec2 p0, glm::vec2 p1, float width) const
{
	WorldObject* obj = new WorldObject();
	obj->setModel(global.modelMgr->get(ModelEnum::centeredQuad));

	glm::vec2 diffVector = p1 - p0;
	glm::vec2 centerPoint = p0 + glm::vec2(diffVector.x / 2.0, diffVector.y / 2.0);

	obj->setPosition(glm::vec3(centerPoint.x, centerPoint.y, 0));

	float angle = atan2(diffVector.y, diffVector.x) * 180 / PI;

	float length = glm::distance(p0, p1);

	glm::vec3 scale(length, width, 1);

	obj->setRotation(glm::rotate(angle, 0.0f, 0.0f, 1.0f));

	obj->setScale(scale);

	return obj;
}





/*
fixing the first and end point,

combine points that can do a line fit

*/


void DDARaycasting::render()
{
	// *******************************************************
	// ************* Rendering *******************************
	// *******************************************************

	m_pipeline.setMatrixMode(MODEL_MATRIX);
	glBindFramebuffer(GL_FRAMEBUFFER, RENDER_TO_SCREEN);
	glViewport(0, 0, utl::SCREEN_WIDTH, utl::SCREEN_HEIGHT);


	m_pipeline.setMatrixMode(VIEW_MATRIX);
	m_pipeline.loadIdentity();

	m_pipeline.translate(0.0f, 0.0f, 5.0f);

	m_pipeline.setMatrixMode(MODEL_MATRIX);
	//	m_pipeline.loadIdentity();

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);


	glCullFace(GL_BACK);
	glClear(GL_DEPTH_BUFFER_BIT);


	if (frameNum == 0)
	{
		map.debug();
	}

	mapView.render(m_pipeline);



	// Rendering wireframes
	p_renderer = &global.rendererMgr->r_fullVertexColor;
	p_renderer->enableShader();
	o_worldAxis.renderGroup(m_pipeline, p_renderer);
	p_renderer->disableShader();


	if (shouldRenderCurrentRay())
	{
		p_renderer = &global.rendererMgr->r_fullColor;
		p_renderer->enableShader();
		p_renderer->setData(R_FULL_COLOR::u_color, COLOR_RED);
		currentRay->renderGroup(m_pipeline, p_renderer);

		/*
		if (traversalRay != NULL)
		{
			p_renderer->setData(R_FULL_COLOR::u_color, COLOR_BLUE);
			traversalRay->renderGroup(m_pipeline, p_renderer);
		}
		*/
		p_renderer->disableShader();
	}

	long long timeNowMillis = getCurrentTimeMillis();

	int deltaTimeMillis = (unsigned int)(timeNowMillis - m_currentTimeMillis);
	m_currentTimeMillis = timeNowMillis;



	int fps = getAverageFPS();
	m_gui.setFPS(fps);

	m_gui.initGUIRenderingSetup();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_BLEND);


	m_gui.updateAndRender(m_mouseState);

	SDL_GL_SwapBuffers();
	frameNum++;
}


bool DDARaycasting::shouldRenderCurrentRay()
{
	return hasDrawnSource;
}


long long DDARaycasting::getCurrentTimeMillis()
{
#ifdef WIN32
	return GetTickCount();
#else
	timeval t;
	gettimeofday(&t, NULL);

	long long ret = t.tv_sec * 1000 + t.tv_usec / 1000;
	return ret;
#endif
}



#define MAX_CLIENTS 10
#define SERVER_PORT 60000







int main(int argc, char *argv[])
{
	utl::debug("Game Starting"); 
	utl::initSDL(utl::SCREEN_WIDTH, utl::SCREEN_HEIGHT, pDisplaySurface);
	utl::initGLEW();

	DDARaycasting Martin;


	Martin.init();
	Martin.start();

	utl::exitSDL(pDisplaySurface);
	//normal termination

	while (1)
	{

	}

	cout << "Terminating normally." << endl;
	return EXIT_SUCCESS;
}


int DDARaycasting::endWithError(char* msg, int error)
{
	//Display error message in console
	cout << msg << "\n";
	system("PAUSE");
	return error;
}


// http://kcat.strangesoft.net/mpstream.c



void DDARaycasting::initGUI()
{
	// run this before m_gui.init, so the textEngine is initialized
	// need to comeback and re-organize the gui the minimize dependencies
	Control::init("", 25, utl::SCREEN_WIDTH, utl::SCREEN_HEIGHT);
	m_gui.init(utl::SCREEN_WIDTH, utl::SCREEN_HEIGHT);
}


void DDARaycasting::renderGUI()
{

	m_gui.initGUIRenderingSetup();
	/// http://sdl.beuc.net/sdl.wiki/SDL_Average_FPS_Measurement
	//	unsigned int getTicks = SDL_GetTicks();

	//	static string final_str = "(" + utl::floatToStr(m_mouseState.m_pos.x) + ", " + utl::floatToStr(m_mouseState.m_pos.y) + ")";
	m_gui.updateAndRender(m_mouseState);

	// healthbar and text


}





