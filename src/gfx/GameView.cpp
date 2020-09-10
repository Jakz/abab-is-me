#include "MainView.h"

#include "game/Types.h"
#include "game/Tile.h"
#include "game/Level.h"
#include "game/Rules.h"

#include <algorithm>
#include <numeric>

using namespace ui;
using namespace baba;

extern Level* level;
History history;

SDL_Surface* palette = nullptr;

constexpr uint32_t FRAMES = 3;
struct ObjectGfx
{
  SDL_Texture* texture;
  std::vector<SDL_Rect> sprites;
  size_t w, h;
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
    path base = DATA_FOLDER + R"(Sprites/)" + spec->sprite;
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

    SDL_Surface* surface = SDL_CreateRGBSurface(0, 24 * 3 * frames.size(), 24, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

    auto cacheSize = std::accumulate(objectGfxs.begin(), objectGfxs.end(), 0ULL, [](uint64_t val, const auto& entry) { return entry.second.w * entry.second.h * 4 + val; });
    cacheSize += surface->w * surface->h * 4;

    LOGD("Caching gfx for %s in a %dx%d texture, total cache size: %.2f", spec->name.c_str(), 24 * 3 * frames.size(), 24, cacheSize / 1024.0f);


    ObjectGfx gfx;

    for (uint32_t i = 0; i < frames.size(); ++i)
    {
      for (uint32_t f = 0; f < FRAMES; ++f)
      {
        static char buffer[128];
        sprintf(buffer, "%s_%d_%d.png", base.c_str(), frames[i], f+1);
        SDL_Surface* tmp = IMG_Load(buffer);
        if (!tmp)
          LOGD("Error: missing graphics file %s", buffer);

        SDL_Rect dest = { (i * 3 + f) * 24, 0, 24, 24 };
        SDL_BlitSurface(tmp, nullptr, surface, &dest);

        SDL_FreeSurface(tmp);

        if (f == 0)
          gfx.sprites.push_back(dest);
      }
    }

    gfx.texture = SDL_CreateTextureFromSurface(manager->renderer(), surface);
    gfx.w = surface->w;
    gfx.h = surface->h;
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

  //TODO: should be reloaded if level changes
  if (!palette)
  {
    palette = IMG_Load((DATA_FOLDER + R"(Palettes/)" + level->palette()).c_str());
    assert(palette);
    SDL_Surface* tmp = SDL_ConvertSurfaceFormat(palette, SDL_PIXELFORMAT_BGRA8888, 0);
    SDL_FreeSurface(palette);
    palette = tmp;

    //rules.state(data.objectsByName["baba"]).properties.set(baba::ObjectProperty::YOU);
    //rules.state(data.objectsByName["wall"]).properties.set(baba::ObjectProperty::STOP);

    SDL_GetRGB(*(((uint32_t*)palette->pixels) + 0 * palette->w + 1), palette->format, &colors.outside.r, &colors.outside.g, &colors.outside.b);
    SDL_GetRGB(*(((uint32_t*)palette->pixels) + 4 * palette->w + 0), palette->format, &colors.inside.r, &colors.inside.g, &colors.inside.b);

    level->updateRules();
  }

  constexpr coord_t GFX_TILE_SIZE = 24;
  scaler = Scaler::SCALE_TO_FIT;
  offset = { 0, 0 };
  size = { WIDTH, HEIGHT };

  float bestWidth = size.w / level->width();
  float bestHeight = size.h / level->height();

  float bestScale = std::min(bestWidth, bestHeight);

  if (scaler == Scaler::KEEP_AT_MOST_NATIVE)
    bestScale = std::min(bestScale, float(GFX_TILE_SIZE));

  tileSize = bestScale;
  offset.x = (size.w - level->width() * bestScale) / 2;
  offset.y = (size.h - level->height() * bestScale) / 2;


  SDL_Rect bb = { 0,0, size.w, size.h };

  SDL_SetRenderDrawColor(renderer, colors.outside.r, colors.outside.g, colors.outside.b, 255);
  SDL_RenderFillRect(renderer, &bb);

  for (int y = 0; y < size.h / tileSize; ++y)
  {
    for (int x = 0; x < size.w / tileSize; ++x)
    {
      auto* tile = level->get(x, y);

      if (tile)
      {
        const SDL_Rect dest = { offset.x + x * tileSize, offset.y + y * tileSize, tileSize, tileSize };

        SDL_SetRenderDrawColor(renderer, colors.inside.r, colors.inside.g, colors.inside.b, 255);
        SDL_RenderFillRect(renderer, &dest);

        for (const auto& obj : *tile)
        {
          SDL_Color color;
          const auto& ocolor = obj.active ? obj.spec->active : obj.spec->color;
          SDL_GetRGB(*(((uint32_t*)palette->pixels) + ocolor.y * palette->w + ocolor.x), palette->format, &color.r, &color.g, &color.b);

          if (obj.spec == level->data()->EDGE)
          {
            SDL_SetRenderDrawColor(renderer, colors.outside.r, colors.outside.g, colors.outside.b, 255);
            SDL_RenderFillRect(renderer, &dest);
          }
          else
          {
            const auto& gfx = objectGfx(obj.spec);
            SDL_SetTextureColorMod(gfx.texture, color.r, color.g, color.b);
            auto variant = obj.variant;
            if (obj.spec->tiling == ObjectSpec::Tiling::None)
              variant = 0;

            SDL_Rect src = gfx.sprites[variant];

            src.x += tick * GFX_TILE_SIZE;

            SDL_RenderCopy(renderer, gfx.texture, &src, &dest);
          }
        }
      }
    }
  }

  const auto& rulesList = level->rules().rules();
  for (coord_t i = 0; i < rulesList.size(); ++i)
    manager->text(rulesList[i].name(), 5, 5 + i * 10, { 255, 255, 255 }, ui::TextAlign::LEFT, 1.0f);

  if (level->isVictory())
    manager->text("Victory!", WIDTH - 5, 5, { 255, 255, 0 }, ui::TextAlign::RIGHT, 1.0f);
  else if (level->isDefeat())
    manager->text("Defeat!", WIDTH - 5, 5, { 255, 0, 0 }, ui::TextAlign::RIGHT, 1.0f);

  manager->text(level->name(), WIDTH / 2, HEIGHT - 20, { 255, 255, 255 }, ui::TextAlign::CENTER, 2.0f);

#if MOUSE_ENABLED
  manager->text(hoverInfo, 5, HEIGHT - 10, { 255, 255, 255 }, ui::TextAlign::LEFT, 1.0f);
#endif
}

void GameView::updateMoveBounds()
{
  level->forEachTile([this](Tile& tile) {
    if (tile.has(ObjectProperty::YOU))
    {
      moveBounds[0].x = std::min(moveBounds[0].x, tile.x());
      moveBounds[0].y = std::min(moveBounds[0].y, tile.y());

      moveBounds[1].x = std::max(moveBounds[1].x, tile.x());
      moveBounds[1].y = std::max(moveBounds[1].y, tile.y());
    }
  });
}


bool operator&&(const ObjectSpec* spec, ObjectProperty prop)
{
  return level->rules().hasProperty(spec, prop);
}

struct MoveInfo
{
  enum class Type { YOU, MOVE } type;
  Tile* tile;
  decltype(Tile::objects)::iterator it;
  MoveInfo(Type type, Tile* tile, decltype(it) it) : type(type), tile(tile), it(it) { }
};

bool isMovementAllowed(MoveInfo info, D d)
{
  bool allowed = true;
  bool finished = false;

  Tile* next = level->get(info.tile, d);

  while (next)
  {    
    if (next->has(ObjectProperty::STOP))
      return false;
    else if (!next->has(ObjectProperty::PUSH))
      return true;

    next = level->get(next, d);
  }

  return true;
}

bool movement(MoveInfo info, D d)
{
  Tile* tile = info.tile;
  Tile* next = level->get(tile, d);

  auto it = info.it;

  bool isStopped = false;
  bool updateVariant = false;

  if (next->empty())
  {
  }
  else
  {
    for (auto nit = next->begin(); nit != next->end(); ++nit)
    {
      if (it->spec && ObjectProperty::YOU && next->has(ObjectProperty::DEFEAT))
      {
        tile->objects.erase(it);
        return false;
      }
      
      /* next is stop, just break from the cycle, we can't move */
      if (nit->spec && ObjectProperty::STOP)
      {
        if (info.type == MoveInfo::Type::MOVE)
        {
          if (it->direction == D::DOWN) it->direction = D::UP;
          else if (it->direction == D::UP) it->direction = D::DOWN;
          else if (it->direction == D::LEFT) it->direction = D::RIGHT;
          else if (it->direction == D::RIGHT) it->direction = D::LEFT;
          updateVariant = true;
        }

        isStopped = true;
        break;
      }
      /* it's push, it could move must we must see if it can be moved too*/
      else if (info.type == MoveInfo::Type::YOU && (it->spec && ObjectProperty::PUSH || nit->spec->isText))
      {
        /* if current can be moved we check next, otherwise we can stop */
        isStopped |= !movement(MoveInfo(info.type, next, nit), d);
        if (!isStopped && next->begin() != next->end())
          nit = next->begin(); // restart
        else
          break;
      }
    }
  }

  if (!isStopped || updateVariant)
  {
    if (info.it->spec->tiling == ObjectSpec::Tiling::Character)
    {
      uint32_t variantBase = 0;
      if (d == D::RIGHT) variantBase = 0;
      else if (d == D::UP) variantBase = 4;
      else if (d == D::LEFT) variantBase = 8;
      else if (d == D::DOWN) variantBase = 12;

      info.it->variant = variantBase + (info.it->variant + 1) % 4;
    }
  }

  if (!isStopped)
  {
    Object object = *info.it;
    Tile* tile = info.tile;

    tile->objects.erase(info.it);
    level->get(tile, d)->objects.push_back(object);

    return true;
  }

  if (!isStopped || updateVariant)
  {
    if (info.it->spec->tiling == ObjectSpec::Tiling::Character)
    {
      uint32_t variantBase = 0;
      if (d == D::RIGHT) variantBase = 0;
      else if (d == D::UP) variantBase = 4;
      else if (d == D::LEFT) variantBase = 8;
      else if (d == D::DOWN) variantBase = 12;

      info.it->variant = variantBase + (info.it->variant + 1) % 4;
    }
  }

  return false;
}

void movement(D d)
{
  level->forEachObject([](Object& object) { object.alreadyMoved = false; });

  std::vector<MoveInfo> you;
  std::vector<std::pair<MoveInfo, D>> move;

  for (coord_t y = 0; y < level->height(); ++y)
    for (coord_t x = 0; x < level->width(); ++x)
    {
      Tile* tile = level->get(x, y);

      for (auto it = tile->begin(); it != tile->end(); ++it)
      {
        if (it->spec && ObjectProperty::YOU)
          you.emplace_back(MoveInfo::Type::YOU, tile, it);
        if (it->spec && ObjectProperty::MOVE)
          move.emplace_back(MoveInfo(MoveInfo::Type::MOVE, tile, it), it->direction);
      }
    }

  if (!you.empty())
  {
    history.push(level->state());

    for (const auto& pair : you)
      movement(pair, d);

    for (const auto& pair : move)
      movement(pair.first, pair.second);

    for (auto& tile : *level)
      std::sort(tile.begin(), tile.end(), [](const Object& o1, const Object& o2) { return o1.spec->layer < o2.spec->layer; });

    level->forEachObject([](Object& object) { object.active = false; });
    level->updateRules();

    for (const auto& props : level->rules())
    {
      auto* spec = props.first;

      if (props.second.properties.isSet(ObjectProperty::WIN))
        level->forEachObject([spec](Object& object) { if (object.spec == spec) object.active = true; });
    }
  }

  
}


void GameView::handleKeyboardEvent(const SDL_Event& event)
{
  if (event.type == SDL_KEYDOWN)
  {
    switch (event.key.keysym.sym)
    {
    case SDLK_ESCAPE: manager->exit(); break;
    case SDLK_LEFT: movement(D::LEFT); updateMoveBounds(); break;
    case SDLK_RIGHT: movement(D::RIGHT); updateMoveBounds(); break;
    case SDLK_UP: movement(D::UP); updateMoveBounds(); break;
    case SDLK_DOWN: movement(D::DOWN); updateMoveBounds(); break;

    case SDLK_TAB:
      if (!history.empty())
      {
        level->restore(history.pop());
        level->updateRules();
      }
      
      break;
    }
  }
}

void GameView::handleMouseEvent(const SDL_Event& event)
{
  if (event.type == SDL_MOUSEMOTION)
  {
    coord_t x = (event.motion.x - offset.x) / tileSize;
    coord_t y = (event.motion.y - offset.y) / tileSize;

    Tile* tile = level->get(x, y);

    if (tile)
    {
      hoverInfo.resize(256);
      memset(hoverInfo.data(), 0, 256);

      const auto* object = tile->object();

      if (tile->object())
      {
        snprintf(hoverInfo.data(), 256, "%d,%d: %s, variant: %d", x, y, object->spec->name.c_str(), object->variant );
      }
      else
        snprintf(hoverInfo.data(), 256, "%d,%d: empty", x, y);


    }
  }
  else
    hoverInfo = "";
}


GameView::~GameView()
{

}
