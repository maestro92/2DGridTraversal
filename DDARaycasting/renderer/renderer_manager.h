#ifndef RENDERER_MANAGER_H_
#define RENDERER_MANAGER_H_

#include "quad_model.h"
#include "renderer_constants.h"
#include "renderer.h"
#include "global.h"

class RendererManager
{
    public:
        RendererManager();
        ~RendererManager();

        Renderer		r_fullVertexColor;
		Renderer		r_fullColor;
		Renderer		r_fullTexture;

		void init(int width, int height);
};

#endif
