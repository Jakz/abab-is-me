#include "AssetCache.h"

#include "Common.h"
#include "game/Level.h"
#include "gfx/Gfx.h"

#include "SDL.h"

#include <filesystem>

AssetCache::AssetCache()
{

}

void AssetCache::flushCache()
{
  return;
  _objectGfxs.clear();
  _imageGfxs.clear();
  _iconGfxs.clear();
}

void AssetCache::loadPalettes()
{
  auto folder = _dataFolder + R"(Palettes/)";

  for (const auto& file : std::filesystem::directory_iterator(folder, std::filesystem::directory_options::skip_permission_denied))
  {
    if (file.is_regular_file() && file.path().extension() == ".png")
    {
      auto paletteName = file.path().filename().string();

      auto paletteSurface = IMG_Load(file.path().string().c_str());
      assert(paletteSurface);
      
      SDL_Surface* tmp = SDL_ConvertSurfaceFormat(paletteSurface, SDL_PIXELFORMAT_BGRA8888, 0);
      SDL_FreeSurface(paletteSurface);
      paletteSurface = tmp;

      Palette::data_t data;

      for (int32_t y = 0; y < Palette::H; ++y)
      {
        for (int32_t x = 0; x < Palette::W; ++x)
        {
          color_t& color = data[y * Palette::W + x];
          color.a = 255;
          
          SDL_GetRGB(*(((uint32_t*)paletteSurface->pixels) + y * Palette::W + x), paletteSurface->format, &color.r, &color.g, &color.b);
        }
      }

      _palettes[paletteName].reset(new Palette(data));
    }
  }
}

const Palette* AssetCache::palette(const std::string& name) const
{
  auto it = _palettes.find(name);
  return it != _palettes.end()  ? it->second.get() : nullptr;
}

const Texture* AssetCache::objectGfx(const baba::ObjectSpec* spec) const
{
  auto it = _objectGfxs.find(spec);

  if (it != _objectGfxs.end())
    return it->second.get();
  else
  {
    path base;
    
    if (spec->spriteInRoot)
      base = _dataFolder + R"(Sprites/)" + spec->sprite;
    else /* TODO: use world folder, not hardcoded one */
      base = _dataFolder + R"(Worlds/baba/Sprites/)" + spec->sprite;

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
    case baba::ObjectSpec::Tiling::Animated:
    {
      //int32_t f[] = { 31, 0, 1, 2, 3, 7, 8, 9, 10, 11, 15, 16, 17, 18, 19, 23, 24, 25, 26, 27 };
      int32_t f[] = { 0, 1, 2, 3, 8, 9, 10, 11, 16, 17, 18, 19, 24, 25, 26, 27 };
      for (int32_t i : f)
        frames.push_back(i);
      break;
    }
    case baba::ObjectSpec::Tiling::SingleAnimated:
      for (int32_t i = 0; i < 4; ++i)
        frames.push_back(i);
      break;
    }

    SDL_Surface* surface = nullptr;

    std::vector<rect_t> rects;

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
          surface = SDL_CreateRGBSurface(0, tmp->w * 3 * frames.size(), tmp->h, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

        SDL_Rect dest = { (i * 3 + f) * tmp->w, 0, tmp->w, tmp->h };
        SDL_BlitSurface(tmp, nullptr, surface, &dest);

        SDL_FreeSurface(tmp);

        if (f == 0)
          rects.push_back(dest);
      }
    }

    auto cacheSize = std::accumulate(_objectGfxs.begin(), _objectGfxs.end(), 0ULL, [](uint64_t val, const decltype(_objectGfxs)::value_type& entry) { return entry.second->width() * entry.second->height() * 4 + val; });
    cacheSize += surface->w * surface->h * 4;
    LOGD("Caching gfx for %s (%s) in a %dx%d texture, total cache size: %.2f", spec->name.c_str(), spec->sprite.c_str(), surface->w, surface->h, cacheSize / 1024.0f);

    auto texture = SDL_CreateTextureFromSurface(gfx::Gfx::i.renderer, surface);
    auto* asset = new Texture(texture, size2d_t(surface->w, surface->h), rects);
    SDL_FreeSurface(surface);

    auto rit = _objectGfxs.emplace(std::make_pair(spec, asset));

    return asset;
  }
}

const Texture* AssetCache::imageGfx(const std::string& image) const
{
  auto it = _imageGfxs.find(image);

  if (it != _imageGfxs.end())
    return it->second.get();


  //TODO: make it relative to world name
  std::string path = _dataFolder + R"(Worlds\baba\Images\)" + image;

  SDL_Surface* surface = nullptr;

  std::vector<rect_t> rects;

  for (size_t i = 0; i < 3; ++i)
  {
    char buffer[128];
    sprintf(buffer, "%s_%d.png", path.c_str(), i + 1);
    auto image = IMG_Load(buffer);
    assert(image);

    if (!surface)
    {
      surface = SDL_CreateRGBSurface(0, image->w * 3, image->h, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
      assert(surface);
    }

    SDL_Rect dest = { i * image->w, 0, image->w, image->h };
    SDL_BlitSurface(image, nullptr, surface, &dest);

    rects.push_back(dest);

    SDL_FreeSurface(image);
  }

  auto texture = SDL_CreateTextureFromSurface(gfx::Gfx::i.renderer, surface);
  auto* asset = new Texture(texture, size2d_t(surface->w, surface->h), rects);
  SDL_FreeSurface(surface);

  auto rit = _imageGfxs.emplace(std::make_pair(image, asset));

  return asset;
}

const Texture* AssetCache::iconGfx(const baba::Icon* spec) const
{
  auto it = _iconGfxs.find(spec);

  if (it != _iconGfxs.end())
    return it->second.get();

  assert(!spec->spriteInRoot);

  std::string path;

  if (spec->spriteInRoot)
    path = _dataFolder + R"(Sprites\)" + spec->sprite + ".png";
  else /* TODO: use world folder, not hardcoded one */
    path = _dataFolder + R"(Worlds\baba\Sprites\)" + spec->sprite + ".png";

  SDL_Surface* image = IMG_Load(path.c_str());
  assert(image);

  std::vector<rect_t> rects = { { 0, 0, image->w, image->h } };
  auto texture = SDL_CreateTextureFromSurface(gfx::Gfx::i.renderer, image);
  auto* asset = new Texture(texture, size2d_t(image->w, image->h), rects);

  auto rit = _iconGfxs.emplace(std::make_pair(spec, asset));

  return asset;
}