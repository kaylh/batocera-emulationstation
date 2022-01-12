#include "Renderer_SDL2.h"

#ifdef RENDERER_SDL2

#include "renderers/Renderer.h"
#include "math/Transform4x4f.h"
#include "Log.h"
#include "Settings.h"

#include <SDL_opengl.h>
#include <SDL.h>
#include <vector>

namespace Renderer
{
	static SDL_Renderer*	sdlRenderer = nullptr;
	static unsigned int boundTexture = 0;

	static GLenum convertBlendFactor(const Blend::Factor _blendFactor)
	{
		switch(_blendFactor)
		{
			case Blend::ZERO:                { return GL_ZERO;                } break;
			case Blend::ONE:                 { return GL_ONE;                 } break;
			case Blend::SRC_COLOR:           { return GL_SRC_COLOR;           } break;
			case Blend::ONE_MINUS_SRC_COLOR: { return GL_ONE_MINUS_SRC_COLOR; } break;
			case Blend::SRC_ALPHA:           { return GL_SRC_ALPHA;           } break;
			case Blend::ONE_MINUS_SRC_ALPHA: { return GL_ONE_MINUS_SRC_ALPHA; } break;
			case Blend::DST_COLOR:           { return GL_DST_COLOR;           } break;
			case Blend::ONE_MINUS_DST_COLOR: { return GL_ONE_MINUS_DST_COLOR; } break;
			case Blend::DST_ALPHA:           { return GL_DST_ALPHA;           } break;
			case Blend::ONE_MINUS_DST_ALPHA: { return GL_ONE_MINUS_DST_ALPHA; } break;
			default:                         { return GL_ZERO;                }
		}

	} // convertBlendFactor

	unsigned int SDL2Renderer::getWindowFlags()
	{
		return 0;// SDL_WINDOW_OPENGL;

	} // getWindowFlags


	void SDL2Renderer::setupWindow()
	{		
		/*
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,  24);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);*/
		
	} // setupWindow

	std::string SDL2Renderer::getDriverName()
	{
		return "SDL2";
	}

	std::vector<std::pair<std::string, std::string>> SDL2Renderer::getDriverInformation()
	{
		std::vector<std::pair<std::string, std::string>> info;

		info.push_back(std::pair<std::string, std::string>("GRAPHICS API", getDriverName()));

		const std::string vendor = glGetString(GL_VENDOR) ? (const char*)glGetString(GL_VENDOR) : "";
		if (!vendor.empty())
			info.push_back(std::pair<std::string, std::string>("VENDOR", vendor));

		const std::string renderer = glGetString(GL_RENDERER) ? (const char*)glGetString(GL_RENDERER) : "";
		if (!renderer.empty())
			info.push_back(std::pair<std::string, std::string>("RENDERER", renderer));

		const std::string version = glGetString(GL_VERSION) ? (const char*)glGetString(GL_VERSION) : "";
		if (!version.empty())
			info.push_back(std::pair<std::string, std::string>("VERSION", version));

		return info;
	}

	void SDL2Renderer::createContext()
	{
		sdlRenderer = SDL_CreateRenderer(getSDLWindow(), -1, SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_ACCELERATED/* | SDL_RENDERER_PRESENTVSYNC*/);
		if (sdlRenderer == nullptr)
			sdlRenderer = SDL_CreateRenderer(getSDLWindow(), -1, SDL_RENDERER_ACCELERATED);

		/*
		sdlContext = SDL_GL_CreateContext(getSDLWindow());
		SDL_GL_MakeCurrent(getSDLWindow(), sdlContext);

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

		std::string glExts = (const char*)glGetString(GL_EXTENSIONS);
		LOG(LogInfo) << "Checking available OpenGL extensions...";
		LOG(LogInfo) << " ARB_texture_non_power_of_two: " << (glExts.find("ARB_texture_non_power_of_two") != std::string::npos ? "ok" : "MISSING");
		*/
	} // createContext

	void SDL2Renderer::destroyContext()
	{
		if (sdlRenderer != nullptr)
			SDL_DestroyRenderer(sdlRenderer);

		sdlRenderer = nullptr;

	} // destroyContext

	static std::map<unsigned int, SDL_Texture*> _textures;	

	static Uint32 convertTextureType(const Texture::Type _type)
	{
		switch (_type)
		{
		case Texture::RGBA: { return SDL_PIXELFORMAT_RGBA32;  } break;
		case Texture::ALPHA: { return SDL_PIXELFORMAT_RGBA32; } break;
		default: { return GL_ZERO;  }
		}

	} // convertTextureType

	unsigned int SDL2Renderer::createTexture(const Texture::Type _type, const bool _linear, const bool _repeat, const unsigned int _width, const unsigned int _height, void* _data)
	{
		SDL_Texture* texture = SDL_CreateTexture(sdlRenderer, convertTextureType(_type), SDL_TEXTUREACCESS_STREAMING, _width, _height);
	
		if (_data)
		{
			int pitch;
			void* pixels = nullptr;
			if (SDL_LockTexture(texture, NULL, &pixels, &pitch) == 0)
			{			
				for (int y = 0; y < _height; y++)
					memcpy((char *)pixels + y * pitch, (char *)_data + (_width * (_height - y - 1)) * 4, _width * 4);

				SDL_UnlockTexture(texture);
			}
		}

		return (unsigned int)texture;
	}
	
	void SDL2Renderer::destroyTexture(const unsigned int _texture)
	{
		if (_texture != 0)
		{
			SDL_Texture* text = (SDL_Texture*)_texture;
			SDL_DestroyTexture(text);
		}
	}

	void SDL2Renderer::updateTexture(const unsigned int _texture, const Texture::Type _type, const unsigned int _x, const unsigned _y, const unsigned int _width, const unsigned int _height, void* _data)
	{
		return;

		if (_data && _texture)
		{
			SDL_Texture* texture = (SDL_Texture*)_texture;

			SDL_Rect rect = { _x, _y, _width, _height };

			int pitch;
			void* pixels = nullptr;
			if (SDL_LockTexture(texture, nullptr/*&rect*/, &pixels, &pitch) == 0)
			{
			
				SDL_UpdateTexture(texture, &rect, _data, pitch);	
				SDL_UnlockTexture(texture);
			}
		}

	} // updateTexture

	void SDL2Renderer::bindTexture(const unsigned int _texture)
	{
		if (boundTexture == _texture)
			return;

		boundTexture = _texture;
		/*
		glBindTexture(GL_TEXTURE_2D, _texture);

		if(_texture == 0) glDisable(GL_TEXTURE_2D);
		else              glEnable(GL_TEXTURE_2D);*/

	} // bindTexture

	static Transform4x4f projectionMatrix = Transform4x4f::Identity();
	static Transform4x4f mvpMatrix = Transform4x4f::Identity();
	static Transform4x4f worldViewMatrix = Transform4x4f::Identity();

	void SDL2Renderer::setProjection(const Transform4x4f& _projection)
	{
		projectionMatrix = _projection;
		mvpMatrix = projectionMatrix * worldViewMatrix;
		
	} // setProjection

	void SDL2Renderer::setMatrix(const Transform4x4f& _matrix)
	{
		worldViewMatrix = _matrix;
		worldViewMatrix.round();
		mvpMatrix = projectionMatrix * worldViewMatrix;

	} // setMatrix

	void SDL2Renderer::setViewport(const Rect& _viewport)
	{
		SDL_Rect viewport;
		viewport.x = _viewport.x;
		viewport.y = _viewport.y;
		viewport.w = _viewport.w;
		viewport.h = _viewport.h;
		SDL_RenderSetViewport(sdlRenderer, &viewport);

	} // setViewport

	void SDL2Renderer::setScissor(const Rect& _scissor)
	{
		if ((_scissor.x == 0) && (_scissor.y == 0) && (_scissor.w == 0) && (_scissor.h == 0))
		{
			SDL_RenderSetClipRect(sdlRenderer, NULL);
			return;
		}

		SDL_Rect rect;
		rect.x = _scissor.x;
		rect.y = _scissor.y;
		rect.w = _scissor.w;
		rect.h = _scissor.h;
		SDL_RenderSetClipRect(sdlRenderer, &rect);
	} // setScissor

	void SDL2Renderer::setSwapInterval()
	{
		// vsync
		if(Settings::getInstance()->getBool("VSync"))
		{
			// SDL_GL_SetSwapInterval(0) for immediate updates (no vsync, default), 
			// 1 for updates synchronized with the vertical retrace, 
			// or -1 for late swap tearing.
			// SDL_GL_SetSwapInterval returns 0 on success, -1 on error.
			// if vsync is requested, try normal vsync; if that doesn't work, try late swap tearing
			// if that doesn't work, report an error
			if(SDL_GL_SetSwapInterval(1) != 0 && SDL_GL_SetSwapInterval(-1) != 0)
				LOG(LogWarning) << "Tried to enable vsync, but failed! (" << SDL_GetError() << ")";
		}
		else
			SDL_GL_SetSwapInterval(0);

	} // setSwapInterval

	void SDL2Renderer::swapBuffers()
	{		
		SDL_RenderPresent(sdlRenderer);
		SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(sdlRenderer);
	} // swapBuffers
	













	void SDL2Renderer::drawLines(const Vertex* _vertices, const unsigned int _numVertices, const Blend::Factor _srcBlendFactor, const Blend::Factor _dstBlendFactor)
	{
		/*
		glEnable(GL_BLEND);
		glBlendFunc(convertBlendFactor(_srcBlendFactor), convertBlendFactor(_dstBlendFactor));

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

		glVertexPointer(  2, GL_FLOAT,         sizeof(Vertex), &_vertices[0].pos);
		glTexCoordPointer(2, GL_FLOAT,         sizeof(Vertex), &_vertices[0].tex);
		glColorPointer(   4, GL_UNSIGNED_BYTE, sizeof(Vertex), &_vertices[0].col);

		glDrawArrays(GL_LINES, 0, _numVertices);

		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

		glDisable(GL_BLEND);
		*/
	} // drawLines

	void SDL2Renderer::drawTriangleStrips(const Vertex* _vertices, const unsigned int _numVertices, const Blend::Factor _srcBlendFactor, const Blend::Factor _dstBlendFactor, bool verticesChanged)
	{
		SDL_Texture* texture = (SDL_Texture*)boundTexture;
		/*
		if (_numVertices == 4)
		{
			Vertex v[4];
			for (int i = 0; i < 4; i++)
				v[i] = _vertices[i];

			auto translate = mvpMatrix.translation();

			SDL_Rect dstRect{ v[0].pos.x(), v[0].pos.y(),v[3].pos.x() - v[0].pos.x(),v[3].pos.y() - v[0].pos.y() };

			dstRect.x += (int)translate.x();
			dstRect.y += (int)translate.y();

			auto color = _vertices[0].col;

			Uint8 red = (color & 0xFF000000) >> 24;
			Uint8 green = (color & 0x00FF0000) >> 16;
			Uint8 blue = (color & 0x0000FF00) >> 8;
			Uint8 alpha = (color & 0x000000FF);

			SDL_SetRenderDrawColor(sdlRenderer, red, green, blue, alpha);

			if (boundTexture == 0)
			{
				SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
				SDL_RenderFillRect(sdlRenderer, &dstRect);
			}
			else
			{

				SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
				SDL_SetTextureColorMod(texture, 255, 255, 255);
				SDL_RenderCopy(sdlRenderer, texture, nullptr, &dstRect);
			}

			return;
		}
		*/
		SDL_Vertex* vertices = new SDL_Vertex[_numVertices];

		auto translate = mvpMatrix.translation();

		for (int i = 0; i < _numVertices; i++)
		{			
			vertices[i].position.x = _vertices[i].pos.x() + translate.x();
			vertices[i].position.y = _vertices[i].pos.y() + translate.y();

			/**< Normalized texture coordinates, if needed */
			vertices[i].tex_coord.x = _vertices[i].tex.x();
			vertices[i].tex_coord.y = _vertices[i].tex.y();
			
			unsigned int colour = _vertices[i].col;

			vertices[i].color.a = (colour >> 24) & 0xFF;
			vertices[i].color.r = (colour >> 16) & 0xFF;
			vertices[i].color.g = (colour >> 8) & 0xFF;
			vertices[i].color.b = colour & 0xFF;
		}

		SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);
		SDL_SetTextureColorMod(texture, 255, 255, 255);
		int ret = SDL_RenderGeometry(sdlRenderer, texture, vertices, _numVertices, nullptr, 0);
		if (ret != 0)
		{
			delete[] vertices;
			return;
		}
		delete[] vertices;
		/*
		return;

		if (_numVertices == 4)
		{	
			Vertex v[4];
			for (int i = 0; i < 4; i++)
				v[i] = _vertices[i];

			auto translate = mvpMatrix.translation();

			SDL_Rect dstRect { v[0].pos.x(), v[0].pos.y(),v[3].pos.x() - v[0].pos.x(),v[3].pos.y() - v[0].pos.y() };

			dstRect.x += (int)translate.x();
			dstRect.y += (int)translate.y();

			auto color = _vertices[0].col;

			Uint8 red = (color & 0xFF000000) >> 24;
			Uint8 green = (color & 0x00FF0000) >> 16;
			Uint8 blue = (color & 0x0000FF00) >> 8;
			Uint8 alpha = (color & 0x000000FF);

			SDL_SetRenderDrawColor(sdlRenderer, red, green, blue, alpha);

			if (boundTexture == 0)
			{
				SDL_SetRenderDrawBlendMode(sdlRenderer, SDL_BLENDMODE_BLEND);				 
				SDL_RenderFillRect(sdlRenderer, &dstRect);
			}
			else
			{
			
				SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
				SDL_SetTextureColorMod(texture, 255, 255, 255);
				SDL_RenderCopy(sdlRenderer, texture, nullptr, &dstRect);
			}
		}
		*/
		/*
		SDL_RenderCopy(
		
		glEnable(GL_BLEND);
		glBlendFunc(convertBlendFactor(_srcBlendFactor), convertBlendFactor(_dstBlendFactor));

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

		glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &_vertices[0].pos);
		glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &_vertices[0].tex);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), &_vertices[0].col);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, _numVertices);

		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

		glDisable(GL_BLEND);
		*/
	} // drawTriangleStrips

	void SDL2Renderer::drawTriangleFan(const Vertex* _vertices, const unsigned int _numVertices, const Blend::Factor _srcBlendFactor, const Blend::Factor _dstBlendFactor)
	{
		/*
		glEnable(GL_MULTISAMPLE);

		glEnable(GL_BLEND);
		glBlendFunc(convertBlendFactor(_srcBlendFactor), convertBlendFactor(_dstBlendFactor));

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

		glVertexPointer(2, GL_FLOAT, sizeof(Vertex), &_vertices[0].pos);
		glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), &_vertices[0].tex);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), &_vertices[0].col);

		glDrawArrays(GL_TRIANGLE_FAN, 0, _numVertices);

		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);

		glDisable(GL_BLEND);
		glDisable(GL_MULTISAMPLE);*/
	}

	void SDL2Renderer::setStencil(const Vertex* _vertices, const unsigned int _numVertices)
	{
		/*
		bool tx = glIsEnabled(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_2D);

		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_STENCIL_TEST);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);
		glStencilFunc(GL_NEVER, 1, 0xFF);
		glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);

		glStencilMask(0xFF);
		glClear(GL_STENCIL_BUFFER_BIT);

		drawTriangleFan(_vertices, _numVertices);

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glStencilMask(0x00);
		glStencilFunc(GL_EQUAL, 0, 0xFF);
		glStencilFunc(GL_EQUAL, 1, 0xFF);

		if (tx)
			glEnable(GL_TEXTURE_2D);*/
	}

	void SDL2Renderer::disableStencil()
	{
	//	glDisable(GL_STENCIL_TEST);
	}

} // Renderer::

#endif // USE_OPENGL_21
