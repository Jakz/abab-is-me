#pragma once

#include "SDL.h"
#include "SDL_image.h"

#include <unordered_map>
#include <numeric>

namespace ui
{
  class ViewManager;
}

class LevelRenderer
{
private:
  static constexpr uint32_t FRAMES = 3;
  
  struct ObjectGfx
  {
    SDL_Texture* texture;
    std::vector<SDL_Rect> sprites;
    size_t w, h;
    /* R U L D */
  };

  mutable std::unordered_map<const baba::ObjectSpec*, ObjectGfx> objectGfxs;
  mutable std::unordered_map<std::string, ObjectGfx> imageGfxs;
  ui::ViewManager* gvm;

public:
  const ObjectGfx& objectGfx(const baba::ObjectSpec* spec) const;
  const ObjectGfx& imageGfx(const std::string& image) const;

  void flushCache();

public:
  LevelRenderer(ui::ViewManager* gvm) : gvm(gvm)
  {

  }
};

#include "Common.h"
#include "game/Level.h"
#include "gfx/ViewManager.h"

void LevelRenderer::flushCache()
{
  objectGfxs.clear();
  imageGfxs.clear();
}

const LevelRenderer::ObjectGfx& LevelRenderer::objectGfx(const baba::ObjectSpec* spec) const
{
  auto it = objectGfxs.find(spec);

  if (it != objectGfxs.end())
    return it->second;
  else
  {
    path base;
    
    if (spec->spriteInRoot)
      base = DATA_FOLDER + R"(Sprites/)" + spec->sprite;
    else /* TODO: use world folder, not hardcoded one */
      base = DATA_FOLDER + R"(Worlds/baba/Sprites/)" + spec->sprite;

    std::vector<uint32_t> frames;

    switch (spec->tiling)
    {
    case baba::ObjectSpec::Tiling::None:
      frames.push_back(0);
      break;
    case baba::ObjectSpec::Tiling::Tiled:
      for (int32_t i = 0; i < 16; ++i)
        frames.push_back(i);
      break;
    case baba::ObjectSpec::Tiling::Directions:
      frames.push_back(0);
      frames.push_back(8);
      frames.push_back(16);
      frames.push_back(24);
      break;
    case baba::ObjectSpec::Tiling::Character:
      //int32_t f[] = { 31, 0, 1, 2, 3, 7, 8, 9, 10, 11, 15, 16, 17, 18, 19, 23, 24, 25, 26, 27 };
      int32_t f[] = { 0, 1, 2, 3, 8, 9, 10, 11, 16, 17, 18, 19, 24, 25, 26, 27 };
      for (int32_t i : f)
        frames.push_back(i);
      break;
    }

    SDL_Surface* surface = nullptr;


    ObjectGfx gfx;

    for (uint32_t i = 0; i < frames.size(); ++i)
    {
      for (uint32_t f = 0; f < FRAMES; ++f)
      {
        static char buffer[128];
        sprintf(buffer, "%s_%d_%d.png", base.c_str(), frames[i], f + 1);
        SDL_Surface* tmp = IMG_Load(buffer);
        if (!tmp)
          LOGD("Error: missing graphics file %s", buffer);

        if (!surface)
        {
          if (tmp->w != 24)
            printf("asad\n");
          surface = SDL_CreateRGBSurface(0, tmp->w * 3 * frames.size(), tmp->h, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        }

        SDL_Rect dest = { (i * 3 + f) * tmp->w, 0, tmp->w, tmp->h };
        SDL_BlitSurface(tmp, nullptr, surface, &dest);

        SDL_FreeSurface(tmp);

        if (f == 0)
          gfx.sprites.push_back(dest);
      }
    }

    auto cacheSize = std::accumulate(objectGfxs.begin(), objectGfxs.end(), 0ULL, [](uint64_t val, const auto& entry) { return entry.second.w * entry.second.h * 4 + val; });
    cacheSize += surface->w * surface->h * 4;
    LOGD("Caching gfx for %s (%s) in a %dx%d texture, total cache size: %.2f", spec->name.c_str(), spec->sprite.c_str(), surface->w, surface->h, cacheSize / 1024.0f);

    gfx.texture = SDL_CreateTextureFromSurface(gvm->renderer(), surface);
    gfx.w = surface->w;
    gfx.h = surface->h;
    SDL_FreeSurface(surface);

    auto rit = objectGfxs.emplace(std::make_pair(spec, gfx));

    return rit.first->second;
  }
}

const LevelRenderer::ObjectGfx& LevelRenderer::imageGfx(const std::string& image) const
{
  auto it = imageGfxs.find(image);

  if (it != imageGfxs.end())
    return it->second;


  std::string path = DATA_FOLDER + R"(Worlds\baba\Images\)" + image;

  SDL_Surface* surface = nullptr;
  ObjectGfx gfx;

  for (size_t i = 0; i < 3; ++i)
  {
    auto image = IMG_Load((path + "_" + std::to_string(i + 1) + ".png").c_str());
    assert(image);

    if (!surface)
    {
      surface = SDL_CreateRGBSurface(0, image->w * 3, image->h, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
      gfx.w = image->w;
      gfx.h = image->h;
      assert(surface);
    }

    SDL_Rect dest = { i * image->w, 0, image->w, image->h };
    SDL_BlitSurface(image, nullptr, surface, &dest);

    gfx.sprites.push_back(dest);

    SDL_FreeSurface(image);
  }

  gfx.texture = SDL_CreateTextureFromSurface(gvm->renderer(), surface);
  SDL_FreeSurface(surface);

  auto rit = imageGfxs.emplace(std::make_pair(image, gfx));

  return rit.first->second;
}
