#include "gui_manager.h"


void GUIManager::init(int screenWidth, int screenHeight)
{
	m_fpsLabel = NULL;
    m_GUIComponentsFlags = 0;

    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    // set m_GUIPipeline
    m_GUIPipeline.setMatrixMode(PROJECTION_MATRIX);
    m_GUIPipeline.loadIdentity();
    m_GUIPipeline.ortho(0, m_screenWidth, m_screenHeight, 0, -1, 1);

    m_GUIPipeline.setMatrixMode(MODEL_MATRIX);
    m_GUIPipeline.loadIdentity();

    m_textureQuad = QuadModel(1,1);

	
	char* filename = "gui/gui_renderer_data.json";

	Value vContent = utl::readJsonFileToVector(filename);
	const Array& vArray = vContent.get_array();

	string path = "gui_shaders/";

	Renderer::initRendererWrapper(vArray, &r_texture, "r_texture", path);
	Renderer::initRendererWrapper(vArray, &r_depthTexture, "r_depthTexture", path);
	Renderer::initRendererWrapper(vArray, &r_coloredRect, "r_coloredRect", path);
	Renderer::initRendererWrapper(vArray, &r_texturedRect, "r_texturedRect", path);
	Renderer::initRendererWrapper(vArray, &r_listBoxItemHighlight, "r_listBoxItemHighlight", path);
	Renderer::initRendererWrapper(vArray, &r_text, "r_text", path);

	Control::r_coloredRect = r_coloredRect;
	Control::r_texturedRect = r_texturedRect;
	Control::r_listBoxItemHighlight = r_listBoxItemHighlight;
	Control::m_textEngine.r_textRenderer = r_text;


	int xOffset = 55;
	int yOffset = 570;

	int BAR_WIDTH = 60;
	int BAR_HEIGHT = 10;

	xOffset = 0; yOffset = 0;
	m_fpsLabel = new Label("90", xOffset, yOffset, 50, 50, COLOR_GRAY);

	float width = 200;
	float height = 50;
	xOffset = utl::SCREEN_WIDTH - width; yOffset = 0;
	m_drawingModeLabel = new Label("Off", xOffset, yOffset, width, height, COLOR_BLUE);

	addGUIComponent(m_fpsLabel);
	addGUIComponent(m_drawingModeLabel);


	utl::debug("GUI manager initing");
}

Pipeline& GUIManager::getPipeline()
{
	return m_GUIPipeline;
}


void GUIManager::setFPS(int fps)
{
	if (m_fpsLabel != NULL)
	{
		m_fpsLabel->setText(utl::intToStr(fps));
	}
}

void GUIManager::initGUIRenderingSetup()
{
	setupRenderToScreen(0, 0, m_screenWidth, m_screenHeight);
}

void GUIManager::setupRenderToScreen(int x, int y, int width, int height)
{
	glViewport(x, y, width, height);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
}

void GUIManager::renderTextureFullScreen(GLuint textureId)
{
	renderTextureFullScreen(textureId, RENDER_TO_SCREEN);
}

void GUIManager::renderTextureFullScreen(GLuint textureId, GLuint fboTarget)
{
	renderTexture(textureId, fboTarget, 0, 0, m_screenWidth, m_screenHeight, r_texture);
}

void GUIManager::renderDepthTextureFullScreen(GLuint textureId)
{
	renderDepthTextureFullScreen(textureId, RENDER_TO_SCREEN);
}

void GUIManager::renderDepthTextureFullScreen(GLuint textureId, GLuint fboTarget)
{
	renderTexture(textureId, fboTarget, 0, 0, m_screenWidth, m_screenHeight, r_depthTexture);
}

void GUIManager::renderTexture(GLuint textureId, int x, int y, int width, int height, Renderer& r)
{
	renderTexture(textureId, RENDER_TO_SCREEN, x, y, width, height, r);
}

void GUIManager::renderTexture(GLuint textureId, GLuint fboTarget, int x, int y, int width, int height, Renderer& r)
{
	setupRenderToScreen(x, y, width, height);
	r.enableShader();
	r.setData(R_TEXTURE::u_texture, 0, GL_TEXTURE_2D, textureId);

    m_GUIPipeline.pushMatrix();
        m_GUIPipeline.translate(x, y, 0);
        m_GUIPipeline.scale(width, height, 1.0);

		r.setUniLocs(m_GUIPipeline);
        m_textureQuad.render();
    m_GUIPipeline.popMatrix();
	r.disableShader();
}


void GUIManager::renderTexture(GLuint TextureId, GLuint FboTarget, Rect rect)
{
    renderTexture(TextureId, FboTarget, rect.x, rect.y, rect.w, rect.h, r_texture);
}



void GUIManager::addGUIComponent(Control* control)
{
	control->setID(m_GUIComponentsID);
	m_GUIComponents.push_back(control);
}

void GUIManager::addDebugLabel(Control* control)
{
	control->setID(m_GUIComponentsID);
	m_debugLabels.push_back(control);
}

void GUIManager::removeDebugLabels()
{
	for (int i = 0; i < m_debugLabels.size(); i++)
	{
		delete m_debugLabels[i];
	}
	m_debugLabels.clear();
}

int GUIManager::getNumGUIComponent()
{
	return m_GUIComponents.size();
}

void GUIManager::setDrawingModeFlag(bool flag)
{
	if (flag)
	{
		m_drawingModeLabel->setText("On");
	}
	else
	{
		m_drawingModeLabel->setText("Off");
	}
}

void GUIManager::updateAndRender(MouseState mouseState)
{
    for(int i=0; i<m_GUIComponents.size(); i++)
    {
		Control* control = m_GUIComponents[i];
		control->update(mouseState);
		control->render();
	}

	for (int i = 0; i<m_debugLabels.size(); i++)
	{
		Control* control = m_debugLabels[i];
		control->update(mouseState);
		control->render();
	}
}
