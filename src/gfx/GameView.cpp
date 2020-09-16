#include "MainView.h"

#include "game/Types.h"
#include "game/Tile.h"
#include "game/Level.h"
#include "game/Rules.h"

#include "LevelRenderer.h"

#include <algorithm>

using namespace ui;
using namespace baba;

extern Level* level;
extern void prevLevel();
extern void nextLevel();

History history;

GameView::GameView(ViewManager* gvm) : gvm(gvm), levelRenderer(new LevelRenderer(gvm)), colors({ nullptr })
{
  colors.grid.a = 0;
}

void GameView::levelLoaded()
{
  auto& palette = colors.palette;
  
  if (palette)
    SDL_FreeSurface(palette);

  palette = IMG_Load((DATA_FOLDER + R"(Palettes/)" + level->palette()).c_str());
  assert(palette);
  SDL_Surface* tmp = SDL_ConvertSurfaceFormat(palette, SDL_PIXELFORMAT_BGRA8888, 0);
  SDL_FreeSurface(palette);
  palette = tmp;

  //rules.state(data.objectsByName["baba"]).properties.set(baba::ObjectProperty::YOU);
  //rules.state(data.objectsByName["wall"]).properties.set(baba::ObjectProperty::STOP);

  SDL_GetRGB(*(((uint32_t*)palette->pixels) + 0 * palette->w + 1), palette->format, &colors.outside.r, &colors.outside.g, &colors.outside.b);
  SDL_GetRGB(*(((uint32_t*)palette->pixels) + 4 * palette->w + 0), palette->format, &colors.inside.r, &colors.inside.g, &colors.inside.b);

  colors.outside.a = 255;
  colors.inside.a = 255;

  level->updateRules();
}

void GameView::render()
{
  SDL_Renderer* renderer = gvm->renderer();

  auto tick = (SDL_GetTicks() / 180) % 3;

  constexpr coord_t GFX_TILE_SIZE = 24;
  scaler = Scaler::SCALE_TO_FIT;
  offset = { 0, 0 };
  size = gvm->getWindowSize();

  float bestWidth = size.w / level->width();
  float bestHeight = size.h / level->height();

  float bestScale = std::min(bestWidth, bestHeight);

  if (scaler == Scaler::KEEP_AT_MOST_NATIVE)
    bestScale = std::min(bestScale, float(GFX_TILE_SIZE));

  tileSize = bestScale;
  offset.x = (size.w - level->width() * bestScale) / 2;
  offset.y = (size.h - level->height() * bestScale) / 2;
  const float ratio = bestScale / GFX_TILE_SIZE;


  SDL_Rect bb = { 0,0, size.w, size.h };

  /* TODO: on level 106 (main world) outside and inside are swapped */
  gvm->fillRect(bb, colors.outside);

  const auto& images = level->images();

  {
    const SDL_Rect insideRect = { offset.x + tileSize, offset.y + tileSize, (level->width() - 2)* tileSize, (level->height() - 2)* tileSize };
    gvm->fillRect(insideRect, colors.inside);
  }

  for (const auto& image : images)
  {
    const auto& gfx = levelRenderer->imageGfx(image);

    //SDL_Rect dest = { offset.x + level->width()*tileSize / 2 - gfx.sprites[tick].w / 2, offset.y + level->height()*tileSize / 2 - gfx.sprites[tick].h / 2, gfx.sprites[tick].w, gfx.sprites[tick].h };
    SDL_Rect dest = { offset.x + tileSize, offset.y + tileSize, (level->width() - 2) * tileSize, (level->height() - 2) * tileSize };

    SDL_RenderCopy(renderer, gfx.texture, &gfx.sprites[tick], &dest);
  }

  for (int y = 0; y < size.h / tileSize; ++y)
  {
    for (int x = 0; x < size.w / tileSize; ++x)
    {
      auto* tile = level->get(x, y);

      if (tile)
      {
        //gvm->fillRect(dest, colors.inside);

        for (const auto& obj : *tile)
        {
          SDL_Color color;
          const auto& ocolor = obj.active ? obj.spec->active : obj.spec->color;
          SDL_GetRGB(*(((uint32_t*)colors.palette->pixels) + ocolor.y * colors.palette->w + ocolor.x), colors.palette->format, &color.r, &color.g, &color.b);

          if (obj.spec == level->data()->EDGE)
          {
            //gvm->fillRect(dest, colors.outside);
          }
          else
          {
            const auto& gfx = levelRenderer->objectGfx(obj.spec);
            SDL_SetTextureColorMod(gfx.texture, color.r, color.g, color.b);
            auto variant = obj.variant;
            if (obj.spec->tiling == ObjectSpec::Tiling::None)
              variant = 0;

            SDL_Rect src = gfx.sprites[variant];
            const SDL_Rect dest = { offset.x + x * tileSize + tileSize/2 - (src.w*ratio)/2, offset.y + y * tileSize + tileSize/2 - (src.h*ratio)/2, src.w * ratio, src.h * ratio };

            src.x += tick * src.w;

            SDL_RenderCopy(renderer, gfx.texture, &src, &dest);
          }
        }
      }
    }
  }

  drawGrid(offset, { tileSize, tileSize }, { level->width() + 1, level->height() + 1 });

  const auto& rulesList = level->rules().rules();
  for (coord_t i = 0; i < rulesList.size(); ++i)
    gvm->text(rulesList[i].name(), 5, 5 + i * 10, { 255, 255, 255 }, ui::TextAlign::LEFT, 1.0f);

  if (level->isVictory())
    gvm->text("Victory!", size.w - 5, 5, { 255, 255, 0 }, ui::TextAlign::RIGHT, 1.0f);
  else if (level->isDefeat())
    gvm->text("Defeat!", size.w - 5, 5, { 255, 0, 0 }, ui::TextAlign::RIGHT, 1.0f);

  gvm->text(level->filename() + " - " + level->name(), size.w / 2, size.h - 20, { 255, 255, 255 }, ui::TextAlign::CENTER, 2.0f);

#if MOUSE_ENABLED
  gvm->text(hoverInfo, 5, size.h - 10, { 255, 255, 255 }, ui::TextAlign::LEFT, 1.0f);
#endif
}

void GameView::drawGrid(point_t b, size2d_t size, size2d_t count)
{
  if (colors.grid.a)
  {
    for (int i = 0; i < count.h; ++i)
      gvm->line(b.x, b.y + size.h * i, b.x + size.w * count.w, b.y + size.h * i, colors.grid);

    for (int i = 0; i < count.w; ++i)
      gvm->line(b.x + size.w * i, b.y, b.x + size.w * i, b.y + size.h * count.h, colors.grid);
  }
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

      //TODO: if it has SINK this should be done too
      auto sink = next->find(ObjectProperty::SINK);
      if (sink) //TODO: only on same float level
      {
        tile->objects.erase(it);
        next->remove(sink);
        level->computeTiling(); //TODO: expensive, just do it around affected tile
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
      else if (info.type == MoveInfo::Type::YOU && (nit->spec && ObjectProperty::PUSH || nit->spec->isText))
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
    case SDLK_ESCAPE: gvm->exit(); break;
    case SDLK_LEFT: movement(D::LEFT); updateMoveBounds(); break;
    case SDLK_RIGHT: movement(D::RIGHT); updateMoveBounds(); break;
    case SDLK_UP: movement(D::UP); updateMoveBounds(); break;
    case SDLK_DOWN: movement(D::DOWN); updateMoveBounds(); break;

    case SDLK_g:
    {
      if (colors.grid.a) colors.grid.a = 0;
      else colors.grid = colors.outside;
      break;
    }

    case SDLK_KP_PLUS: { levelRenderer->flushCache(); nextLevel(); break; }
    case SDLK_KP_MINUS: { levelRenderer->flushCache(); prevLevel(); break; }

    case SDLK_z:
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
#if MOUSE_ENABLED
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
#endif
}


GameView::~GameView()
{

}
