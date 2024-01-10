#include "SoftwareRenderer.h"

#include "LodePng.h"

bool SDL::init()
{
  auto ws = _director->windowSize();
  _frameBuffer.resize(ws.w * ws.h);
  return true;
}

void SDL::deinit()
{
  _frameBuffer.clear();
  _frameBuffer.shrink_to_fit();
}

void SDL::clear(color_t color)
{
  std::fill(_frameBuffer.begin(), _frameBuffer.end(), color);
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
  return buildTexture(loadImageAsSurface(path));
}

Texture* SDL::buildTexture(const Surface& surface)
{
  return new Texture(surface);
}

Surface SDL::loadImageAsSurface(const path& path)
{
  std::vector<unsigned char> png;
  std::vector<unsigned char> image;
  unsigned width, height;

  //load and decode
  unsigned error = lodepng::load_file(png, path);
  if (!error) error = lodepng::decode(image, width, height, png);

  auto surface = Surface(width, height);
  memcpy(&surface._pixels[0], &image[0], image.size());
}

Surface SDL::generateSurface(size2d_t size)
{
  return Surface(size.w, size.h);
}