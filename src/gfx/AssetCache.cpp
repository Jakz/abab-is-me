#include "AssetCache.h"

#include "Common.h"
#include "game/Level.h"
#include "gfx/Gfx.h"

#include "ViewManager.h"

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

void AssetCache::init(Renderer* renderer, const path& baseFolder)
{
  _renderer = renderer;
  
  _dataFolder = baseFolder + "/Data/";
  loadPalettes();

  _loader.init(renderer, baseFolder + "/Assets.dat");
  _loader.cacheOffsets();

  _numberedGfxIndices["level_link_number_00"] = { 10 };
  _numberedGfxIndices["level_link_number_01"] = { 12 };
  _numberedGfxIndices["level_link_number_02"] = { 16 };
  _numberedGfxIndices["level_link_number_03"] = { 17 };
  _numberedGfxIndices["level_link_number_04"] = { 22 };
  _numberedGfxIndices["level_link_number_05"] = { 28 };
  _numberedGfxIndices["level_link_number_06"] = { 29 };
  _numberedGfxIndices["level_link_number_07"] = { 32 };
  _numberedGfxIndices["level_link_number_08"] = { 36 };
  _numberedGfxIndices["level_link_number_09"] = { 42 };
  _numberedGfxIndices["level_link_number_10"] = { 44 };
  _numberedGfxIndices["level_link_number_11"] = { 45 };
  _numberedGfxIndices["level_link_number_12"] = { 46 };
  _numberedGfxIndices["level_link_number_13"] = { 61 };
  _numberedGfxIndices["level_link_number_14"] = { 64 };
  _numberedGfxIndices["level_link_number_15"] = { 66 };
                             
  _numberedGfxIndices["level_link_letter_a"] = { 552 };
  _numberedGfxIndices["level_link_letter_b"] = { 559 };
  _numberedGfxIndices["level_link_letter_c"] = { 796 };
  _numberedGfxIndices["level_link_letter_d"] = { 1034 };
  _numberedGfxIndices["level_link_letter_e"] = { 1036 };

  _numberedGfxIndices["level_link_dot"] = { 462, 463, 464, 465, 466, 470, 471 };


  _numberedGfxIndices["level_link_box"] = { 49, 239, 240 };
  _numberedGfxIndices["level_link_box_bg"] = { 38 };

  auto& smallFont = _numberedGfxIndices["small_font"];
  smallFont.resize(256, -1);

  smallFont[u'!'] = 436;
  smallFont[u'"'] = 3111;
  smallFont[u'%'] = 602;
  smallFont[u'&'] = 601;
  smallFont[u'\''] = 438;
  smallFont[u'('] = 637;
  smallFont[u')'] = 713;
  smallFont[u'*'] = 3181;
  smallFont[u'+'] = 1605;
  smallFont[u','] = 437;
  smallFont[u'-'] = 434;
  smallFont[u'.'] = 433;

  smallFont[u'0'] = 58;
  smallFont[u'1'] = 423;
  smallFont[u'2'] = 424;
  smallFont[u'3'] = 425;
  smallFont[u'4'] = 427;
  smallFont[u'5'] = 428;
  smallFont[u'6'] = 429;
  smallFont[u'7'] = 430;
  smallFont[u'8'] = 431;
  smallFont[u'9'] = 432;

  smallFont[u':'] = 439;
  smallFont[u';'] = 581;
  smallFont[u'<'] = 226;
  smallFont[u'>'] = 50;
  smallFont[u'?'] = 435;
  smallFont[u'_'] = 41;
  
  smallFont[u'a'] = 2270;
  smallFont[u'b'] = 1488;
  smallFont[u'c'] = 1490;
  smallFont[u'd'] = 1491;
  smallFont[u'e'] = 18;
  smallFont[u'f'] = 403;
  smallFont[u'g'] = 404;
  smallFont[u'h'] = 141;
  smallFont[u'i'] = 109;
  smallFont[u'j'] = 405;
  smallFont[u'l'] = 407;
  smallFont[u'm'] = 409;
  smallFont[u'n'] = 408;
  smallFont[u'o'] = 410;
  smallFont[u'p'] = 411;
  smallFont[u'q'] = 412;
  smallFont[u'r'] = 413;
  smallFont[u's'] = 414;
  smallFont[u't'] = 415;
  smallFont[u'u'] = 416;
  smallFont[u'v'] = 417;
  smallFont[u'w'] = 418;
  smallFont[u'x'] = 419;
  smallFont[u'y'] = 420;
  smallFont[u'z'] = 421;

  smallFont[u'à'] = 2120;

  smallFont[u'è'] = 2119;
  smallFont[u'é'] = 2123;

  smallFont[u'ß'] = 34;
  
  

}

const SoundData& AssetCache::sound(uint32_t index)
{
  auto it = _sounds._sounds.find(index);

  if (it != _sounds._sounds.end())
    return it->second;
  else
  {
    auto data = _loader.loadSound(index);
    auto ptr = data.data();
    _sounds._sounds[index] = SoundData(data);
    return _sounds._sounds[index];
  }
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

    Surface surface;

    std::vector<rect_t> rects;

    int32_t spriteWidth = -1, spriteHeight = -1;

    for (uint32_t i = 0; i < frames.size(); ++i)
    {
      for (uint32_t f = 0; f < FRAMES; ++f)
      {
        auto filename = fmt::format("{}_{}_{}.png", base.c_str(), frames[i], f + 1);
        Surface tmp = _renderer->loadImageAsSurface(filename);

        if (!tmp)
          LOGD("Error: missing graphics file %s", filename.c_str());
        else
        {
          spriteWidth = tmp.width();
          spriteHeight = tmp.height();
        }

        if (!surface)
          surface = _renderer->generateSurface(size2d_t(spriteWidth * 3 * frames.size(), spriteHeight));

        rect_t dest = { (i * 3 + f) * spriteWidth, 0, spriteWidth, spriteHeight };

        if (tmp)
        {
          _renderer->blit(&tmp, nullptr, &surface, &dest);
        }

        if (f == 0)
          rects.push_back(dest);
      }
    }

    auto cacheSize = std::accumulate(_objectGfxs.begin(), _objectGfxs.end(), 0ULL, [](uint64_t val, const decltype(_objectGfxs)::value_type& entry) { return entry.second->width() * entry.second->height() * 4 + val; });
    cacheSize += surface.width() * surface.height() * 4;
    LOGD("Caching gfx for %s (%s) in a %dx%d texture, total cache size: %.2f", spec->name.c_str(), spec->sprite.c_str(), surface.width(), surface.height(), cacheSize / 1024.0f);

    auto texture = _renderer->buildTexture(surface);
    texture->setRects(rects);
    
    auto rit = _objectGfxs.emplace(std::make_pair(spec, texture));

    return texture;
  }
}

const Texture* AssetCache::imageGfx(const std::string& image) const
{
  auto it = _imageGfxs.find(image);

  if (it != _imageGfxs.end())
    return it->second.get();

  //TODO: make it relative to world name
  std::string path = _dataFolder + R"(Worlds\baba\Images\)" + image;

  Surface surface;

  std::vector<rect_t> rects;

  for (size_t i = 0; i < 3; ++i)
  {
    auto filename = fmt::format("{}_{}.png", path.c_str(), i + 1);
    Surface image = _renderer->loadImageAsSurface(filename);
    assert(image);

    if (!surface)
    {
      surface = _renderer->generateSurface(size2d_t(image.width() * 3, image.height()));
      assert(surface);
    }

    rect_t dest = { i * image.width(), 0, image.width(), image.height() };
    _renderer->blit(&image, nullptr, &surface, &dest);
   
    rects.push_back(dest);
  }

  auto texture = _renderer->buildTexture(surface);
  texture->setRects(rects);

  auto rit = _imageGfxs.emplace(std::make_pair(image, texture));

  return texture;
}

const Texture* AssetCache::numberedGfx(const std::string& key)
{
  auto it = _numberedGfxs.find(key);

  if (it != _numberedGfxs.end())
    return it->second.get();
  else
  {
    auto nit = _numberedGfxIndices.find(key);

    if (nit != _numberedGfxIndices.end())
    {
      Surface surface;
      
      std::vector<rect_t> rects;
      const auto& indices = nit->second;

      for (int32_t i = 0; i < indices.size(); ++i)
      {        
        if (indices[i] >= 0)
        {
          Surface image = _loader.loadImage(indices[i]);

          if (!surface)
          {
            surface = _renderer->generateSurface(size2d_t(image.width() * indices.size(), image.height()));
            assert(surface);
          }

          rect_t dest = { i * image.width(), 0, image.width(), image.height() };
          _renderer->blit(&image, nullptr, &surface, &dest);
          rects.push_back(dest);
        }
        else
          rects.push_back(rect_t(0, 0, 0, 0));
      }

      if (surface)
      {
        auto asset = _renderer->buildTexture(surface);
        asset->setRects(rects);
        _numberedGfxs[key].reset(asset);
        return asset;
      }
    }
  }

  return nullptr;
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

  auto asset = _renderer->loadImage(path);
  auto rit = _iconGfxs.emplace(std::make_pair(spec, asset));

  return asset;
}



const asset_list AssetMapping::BABA_STEP_SOUND = { 3237, 3238, 3239, 3240, 3241, 3242, 3243, 3244, 3245 };
const asset_list AssetMapping::DEFEAT_SOUND = { 3196, 3197, 3199, 3200 };