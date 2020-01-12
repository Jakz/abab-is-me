#include "MainView.h"

#include "game/Types.h"
#include "game/Tile.h"
#include "game/Level.h"
#include "game/Rules.h"

using namespace ui;
using namespace baba;

extern GameData data;
Rules rules(&data);
extern Level* level;

SDL_Surface* palette = nullptr;

constexpr uint32_t FRAMES = 3;
struct ObjectGfx
{
  SDL_Texture* texture;
  std::vector<SDL_Rect> sprites;
  /* R U L D */
};

std::unordered_map<const baba::ObjectSpec*, ObjectGfx> objectGfxs;

const ObjectGfx& GameView::objectGfx(const baba::ObjectSpec* spec)
{
  auto it = objectGfxs.find(spec);

  if (it != objectGfxs.end())
    return it->second;
  else
  {
    path base = DATA_FOLDER + R"(Sprites\)" + spec->sprite;
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
      int32_t f[] = { 31, 0, 1, 2, 3, 7, 8, 9, 10, 11, 15, 16, 17, 18, 19, 23, 24, 25, 26, 27 };
      for (int32_t i : f)
        frames.push_back(i);
      break;
    }

    SDL_Surface* surface = SDL_CreateRGBSurface(0, 24 * 3 * frames.size(), 24, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

    ObjectGfx gfx;

    for (uint32_t i = 0; i < frames.size(); ++i)
    {
      for (uint32_t f = 0; f < FRAMES; ++f)
      {
        path path = base + "_" + std::to_string(frames[i]) + "_" + std::to_string(f+1) + ".png";
        SDL_Surface* tmp = IMG_Load(path.c_str());
        assert(tmp);

        SDL_Rect dest = { (i * 3 + f) * 24, 0, 24, 24 };
        SDL_BlitSurface(tmp, nullptr, surface, &dest);

        SDL_FreeSurface(tmp);

        if (f == 0)
          gfx.sprites.push_back(dest);
      }
    }

    gfx.texture = SDL_CreateTextureFromSurface(manager->renderer(), surface);
    SDL_FreeSurface(surface);

    auto rit = objectGfxs.emplace(std::make_pair(spec, gfx));
    return rit.first->second;
  }
}


GameView::GameView(ViewManager* manager) : manager(manager)
{
}

void GameView::render()
{
  SDL_Renderer* renderer = manager->renderer();

  auto tick = (SDL_GetTicks() / 150) % 3;

  if (!palette)
  {
    palette = IMG_Load((DATA_FOLDER + R"(Palettes\default.png)").c_str());
    SDL_Surface* tmp = SDL_ConvertSurfaceFormat(palette, SDL_PIXELFORMAT_BGRA8888, 0);
    SDL_FreeSurface(palette);
    palette = tmp;

    //rules.state(data.objectsByName["baba"]).properties.set(baba::ObjectProperty::YOU);
    //rules.state(data.objectsByName["wall"]).properties.set(baba::ObjectProperty::STOP);

    rules.clear();
    rules.generate(level);
    rules.apply();
  }

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  constexpr coord_t GFX_TILE_SIZE = 24;
  scaler = Scaler::SCALE_TO_FIT;
  offset = { 0, 0 };
  size = { 1024, 768 };

  float bestWidth = size.w / level->width();
  float bestHeight = size.h / level->height();

  float bestScale = std::min(bestWidth, bestHeight);

  if (scaler == Scaler::KEEP_AT_MOST_NATIVE)
    bestScale = std::min(bestScale, float(GFX_TILE_SIZE));
  
  tileSize = bestScale;
  offset.x = (size.w - level->width() * bestScale) / 2;
  offset.y = (size.h - level->height() * bestScale) / 2;


  SDL_Rect bb = { 0,0, size.w, size.h };
  //TODO: check if this depends on some level property like theme
  SDL_SetRenderDrawColor(renderer, 21, 24, 31, 255); 
  SDL_RenderFillRect(renderer, &bb);

  for (int y = 0; y < size.h/tileSize; ++y)
    for (int x = 0; x < size.w/tileSize; ++x)
    {
      auto* tile = level->get(x, y);
      
      if (tile)
      {        
        const SDL_Rect dest = { offset.x + x * tileSize, offset.y + y * tileSize, tileSize, tileSize };
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &dest);
        
        for (const auto& obj : *tile)
        {
          const auto& gfx = objectGfx(obj.spec);

          SDL_Color color;
          assert(palette->format->BytesPerPixel == 4);

          const auto& ocolor = obj.active ? obj.spec->active : obj.spec->color;

          SDL_GetRGB(*(((uint32_t*)palette->pixels) + ocolor.y * palette->w + ocolor.x), palette->format, &color.r, &color.g, &color.b);
          SDL_SetTextureColorMod(gfx.texture, color.r, color.g, color.b);

          SDL_Rect src = gfx.sprites[obj.variant];

          src.x += tick * GFX_TILE_SIZE;

          SDL_RenderCopy(renderer, gfx.texture, &src, &dest);
        }
      }
    }


}

void movement(Tile* tile, decltype(Tile::objects)::iterator object, D d, coord_t dx, coord_t dy)
{  
  std::vector<std::pair<Tile*, decltype(Tile::objects)::iterator>> objects;

  Tile* next = level->get(tile->x() + dx, tile->y() + dy);

  objects.push_back(std::make_pair(tile, object));

  while (next)
  {
    if (!next)
      return;
    else if (next->empty())
      break;
    else
    {
      bool found = false;
      
      for (auto it = next->begin(); it != next->end(); ++it)
      {
        if (rules.hasProperty(it->spec, ObjectProperty::STOP))
          return;
        else if (rules.hasProperty(it->spec, ObjectProperty::PUSH))
        {
          objects.push_back(std::make_pair(next, it));
          found = true;
        }
        else if (it->spec->isText)
        {
          objects.push_back(std::make_pair(next, it));
          found = true;
        }
      }

      if (!found)
        break;
    }

    next = level->get(next->x() + dx, next->y() + dy);
  }

  for (auto rit = objects.rbegin(); rit != objects.rend(); ++rit)
  {
    Object object = *rit->second;
    Tile* tile = rit->first;

    if (object.spec->tiling == ObjectSpec::Tiling::Character)
    {
      uint32_t variantBase = 0;
      if (d == D::RIGHT) variantBase = 0;
      else if (d == D::UP) variantBase = 5;
      else if (d == D::LEFT) variantBase = 10;
      else if (d == D::DOWN) variantBase = 15;
      
      object.variant = variantBase + (object.variant + 1) % 5;
    }


    tile->objects.erase(rit->second);
    level->get(tile->x() + dx, tile->y() + dy)->objects.push_back(object);


  }
}


void movement(D d, coord_t dx, coord_t dy)
{
  level->forEachObject([](Object& object) { object.alreadyMoved = false; });

  std::vector<std::pair<Tile*, decltype(Tile::objects)::iterator>> movable;

  for (coord_t y = 0; y < level->height(); ++y)
    for (coord_t x = 0; x < level->width(); ++x)
    {
      Tile* tile = level->get(x, y);

      for (auto it = tile->begin(); it != tile->end(); ++it)
        if (rules.hasProperty(it->spec, ObjectProperty::YOU))
          movable.push_back(std::make_pair(tile, it));
    }

  for (auto& pair : movable)
    movement(pair.first, pair.second, d, dx, dy);

  for (auto& tile : *level)
    std::sort(tile.begin(), tile.end(), [](const Object& o1, const Object& o2) { return o1.spec->layer < o2.spec->layer; });

  level->forEachObject([](Object& object) { object.active = false; });
  rules.clear();
  rules.generate(level);
  rules.apply();

  for (const auto& props : rules)
  {
    auto* spec = props.first;
    
    if (props.second.properties.isSet(ObjectProperty::WIN))
      level->forEachObject([spec](Object& object) { if (object.spec == spec) object.active = true; });
  }
}


void GameView::handleKeyboardEvent(const SDL_Event& event)
{
  if (event.type == SDL_KEYDOWN)
  {
    switch (event.key.keysym.sym)
    {
    case SDLK_ESCAPE: manager->exit(); break;
    case SDLK_LEFT: movement(D::LEFT, -1, 0);  break;
    case SDLK_RIGHT: movement(D::RIGHT, 1, 0);  break;
    case SDLK_UP: movement(D::UP, 0, -1); break;
    case SDLK_DOWN: movement(D::DOWN, 0, 1); break;
    }
  }
}

void GameView::handleMouseEvent(const SDL_Event& event)
{

}


GameView::~GameView()
{

}
