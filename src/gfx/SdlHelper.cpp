#include "SdlHelper.h"


bool SDL::init()
{
  if (SDL_Init(SDL_INIT_EVERYTHING))
  {
    printf("Error on SDL_Init().\n");
    return false;
  }

  if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
  {
    printf("Error on IMG_Init().\n");
    return false;
  }

  if (Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG)
  {
    printf("Error on Mix_Init.\n");
    return false;
  }

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096))
  {
    printf("Error on Mix_OpenAudio.\n");
    return false;
  }

  // SDL_WINDOW_FULLSCREEN
  _window = SDL_CreateWindow("Abab Is Me", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _director->windowSize().w, _director->windowSize().h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED);

  return true;
}

void SDL::loop()
{
  while (!willQuit)
  {
    _director->render();
    SDL_RenderPresent(_renderer);

    handleEvents();

    capFPS();
  }
}

void SDL::capFPS()
{
  u32 ticks = SDL_GetTicks();
  u32 elapsed = ticks - SDL::ticks;

  _lastFrameTicks = elapsed;

  if (elapsed < ticksPerFrame)
  {
    SDL_Delay(ticksPerFrame - elapsed);
    _lastFrameTicks = ticksPerFrame;
  }

  SDL::ticks = SDL_GetTicks();
}

void SDL::deinit()
{
  Mix_CloseAudio();
  Mix_Quit();
  IMG_Quit();

  SDL_DestroyRenderer(_renderer);
  SDL_DestroyWindow(_window);

  SDL_Quit();
}

void SDL::handleEvents()
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
      case SDL_QUIT:
        willQuit = true;
        break;

      case SDL_KEYDOWN:
        _director->handle(events::KeyEvent{ KeyCode(event.key.keysym.sym), true });
        break;

      case SDL_KEYUP:
        _director->handle(events::KeyEvent{ KeyCode(event.key.keysym.sym), false });
        break;

#if MOUSE_ENABLED
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
      case SDL_MOUSEMOTION:
#if defined(WINDOW_SCALE)
        event.button.x /= WINDOW_SCALE;
        event.button.y /= WINDOW_SCALE;
#endif
        eventHandler.handleMouseEvent(event);
#endif
    }
  }
}

void SDL::clear(color_t color)
{
  SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, 255);
  SDL_RenderClear(_renderer);
}

void SDL::drawRect(int x, int y, int w, int h, color_t color)
{
  SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
  SDL_Rect rect = { x, y, w, h };
  SDL_RenderDrawRect(_renderer, &rect);
}

void SDL::fillRect(int x, int y, int w, int h, color_t color)
{
  SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
  SDL_Rect rect = { x, y, w, h };
  SDL_RenderFillRect(_renderer, &rect);
}

void SDL::line(int x1, int y1, int x2, int y2, color_t color)
{
  SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
  SDL_RenderDrawLine(_renderer, x1, y1, x2, y2);
}

void SDL::blit(const Texture* texture, int sx, int sy, int w, int h, int dx, int dy, int dw, int dh)
{
  SDL_Rect from = { sx, sy, w, h };
  SDL_Rect to = { dx, dy, dw, dh };
  blit(texture, from, to);
}

void SDL::blit(const Texture* texture, const rect_t& from, int dx, int dy)
{
  SDL_Rect to = { dx, dy, from.w, from.h };
  blit(texture, from, to);
}

void SDL::blit(const Texture* texture, int sx, int sy, int w, int h, int dx, int dy)
{
  blit(texture, { sx, sy, w, h }, dx, dy);
}

void SDL::blit(const Texture* texture, const rect_t& src, const rect_t& dest)
{
  SDL_RenderCopy(_renderer, texture->texture(), &src, &dest);
}

void SDL::blit(const Texture* texture, color_t color, const rect_t& src, const rect_t& dest)
{
  SDL_SetTextureColorMod(texture->texture(), color.r, color.g, color.b);
  SDL_RenderCopy(_renderer, texture->texture(), &src, &dest);
}


void SDL::blit(const Texture* texture, int dx, int dy)
{
  u32 dummy;
  int dummy2;

  SDL_Rect from = { 0, 0, 0, 0 };
  SDL_Rect to = { dx, dy, 0, 0 };

  SDL_QueryTexture(texture->texture(), &dummy, &dummy2, &from.w, &from.h);

  to.w = from.w;
  to.h = from.h;

  SDL_RenderCopy(_renderer, texture->texture(), &from, &to);
}

void SDL::blit(const Surface* src, const rect_t* srcRect, const Surface* dest, const rect_t* destRect)
{
  SDL_BlitSurface(src->_surface, (SDL_Rect*)srcRect, dest->_surface, (SDL_Rect*)destRect);
}


Texture* SDL::loadImage(const path& path)
{
  auto* surface = IMG_Load(path.c_str());
  assert(surface);

  auto texture = SDL_CreateTextureFromSurface(_renderer, surface);
  SDL_FreeSurface(surface);

  return new Texture(texture, surface->w, surface->h);
}

Texture* SDL::buildTexture(const Surface& surface)
{
  auto texture = SDL_CreateTextureFromSurface(_renderer, surface._surface);
  return new Texture(texture, surface.width(), surface.height());
}

Surface SDL::loadImageAsSurface(const path& path)
{
  auto* surface = IMG_Load(path.c_str());
  assert(surface);

  return Surface(surface);
}

Surface SDL::generateSurface(size2d_t size)
{
  auto surface = SDL_CreateRGBSurface(0, size.w, size.h, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
  return Surface(surface);
}