#include "Views.h"

#include "game/Types.h"
#include "game/Tile.h"
#include "game/Level.h"
#include "game/World.h"
#include "game/Rules.h"

#include "gfx/Gfx.h"

#include <algorithm>

constexpr coord_t GFX_TILE_SIZE = 24;

using namespace ui;
using namespace baba;

extern World* world;
//extern void prevLevel();
//extern void nextLevel();

History history;

GameView::GameView(ViewManager* director, Renderer* renderer) : View(director, renderer), colors({ nullptr }), scaler(Scaler::SCALE_TO_ATMOST_NATIVE)
{ 
  colors.grid.a = 0;
}

void GameView::levelLoaded()
{
  auto* palette = assets()->palette(world->level()->palette());

  if (palette)
  {
    colors.palette = palette;
    colors.outside = palette->at(1, 0);
    colors.inside = palette->at(0, 4);
  }
}

int32_t GameView::tick() const
{
  int32_t tick = (SDL_GetTicks() / 180) % 3;
  return tick;
}

float GameView::ratio() const
{
  //point_t offset = this->offset;
  auto size = _director->windowSize();
  auto level = world->level();

  float bestWidth = size.w / level->width();
  float bestHeight = size.h / level->height();

  float bestScale;

  if (scaler == Scaler::SCALE_TO_ATMOST_NATIVE)
  {
    bestScale = std::min(std::min(bestWidth, bestHeight), float(GFX_TILE_SIZE));
    offset.x = (size.w - level->width() * bestScale) / 2;
    offset.y = (size.h - level->height() * bestScale) / 2;
  }
  else if (scaler == Scaler::SCALE_TO_FIT)
  {
    bestScale = std::min(bestWidth, bestHeight);
    offset.x = (size.w - level->width() * bestScale) / 2;
    offset.y = (size.h - level->height() * bestScale) / 2;
  }
  else if (scaler == Scaler::NATIVE_WITH_SCROLL)
    bestScale = GFX_TILE_SIZE;
  else
    assert(false);

  tileSize = bestScale;

  const float ratio = bestScale / GFX_TILE_SIZE;
  return ratio;
}

void GameView::renderLevelLink(coord_t x, coord_t y)
{
  auto tile = world->level()->get(x, y);
  auto link = tile->link;

  auto cache = assets();
  auto ratio = this->ratio();
  auto* bgAsset = cache->numberedGfx("level_link_box_with_background");
  const auto& src = bgAsset->rect(tick());

  int fy = offset.y + y * tileSize + tileSize / 2 - (src.h * ratio) / 2;
  const rect_t dest = { offset.x + x * tileSize + tileSize / 2 - (src.w * ratio) / 2, fy, src.w * ratio, src.h * ratio };
  const auto& ocolor = link->color;
  color_t color = colors.palette->at(ocolor.x, ocolor.y);
  _renderer->blit(bgAsset, color, src, dest);


  const Texture* asset = nullptr;
  size_t rectIndex = 0;

  switch (link->style)
  {
    case LevelLink::Style::Icon:
    {
      const baba::Icon* icon = &world->level()->metalevel()._icons[link->number];
      asset = cache->iconGfx(icon);
      break;
    }

    case LevelLink::Style::Number:
    {
      std::string key = fmt::format("level_link_number_{:02}", link->number);
      asset = cache->numberedGfx(key);
      break;
    }

    case LevelLink::Style::Letter:
    {
      std::string key = fmt::format("level_link_letter_{}", (char)('a' + link->number));
      asset = cache->numberedGfx(key);
      break;
    }

    case LevelLink::Style::Dot:
    {
      asset = cache->numberedGfx("level_link_dot");
      rectIndex = link->number;
      break;
    }
  }

  if (asset /* && link->state == LevelLink::State::Opened*/)
  {
    const rect_t& src = asset->rect(rectIndex);

    int fy = offset.y + y * tileSize + tileSize / 2 - (src.h * ratio) / 2;
    const rect_t dest = { offset.x + x * tileSize + tileSize / 2 - (src.w * ratio) / 2, fy, src.w * ratio, src.h * ratio };

    const auto& ocolor = link->color;
    color_t color = colors.palette->at(ocolor.x, ocolor.y);
    _renderer->blit(asset, color, src, dest);
  }
}

void GameView::render()
{
  //TODO: particles
  //TODO BEST property

  auto level = world->level();
  auto cache = assets();
  
  int32_t tick = this->tick();
  constexpr int32_t fspan = 2, fcount = 1 + fspan * 4;
  int32_t ftick = ((SDL_GetTicks() / 300) % fspan);
  
  //point_t offset = this->offset;
  size = _director->windowSize();
  auto ratio = this->ratio();


  SDL_Rect bb = { 0,0, size.w, size.h };

  /* TODO: on level 106 (main world) outside and inside are swapped */
  _renderer->fillRect(bb, colors.outside);

  const auto& images = level->images();

  {
    const rect_t insideRect = { offset.x + tileSize, offset.y + tileSize, (level->width() - 2)* tileSize, (level->height() - 2)* tileSize };
    _renderer->fillRect(insideRect, colors.inside);
  }

  for (const auto& image : images)
  {
    const auto* asset = cache->imageGfx(image);

    //SDL_Rect dest = { offset.x + level->width()*tileSize / 2 - gfx.sprites[tick].w / 2, offset.y + level->height()*tileSize / 2 - gfx.sprites[tick].h / 2, gfx.sprites[tick].w, gfx.sprites[tick].h };
    SDL_Rect dest = { offset.x + tileSize, offset.y + tileSize, (level->width() - 2) * tileSize, (level->height() - 2) * tileSize };

    _renderer->blit(asset, asset->rect(tick), dest);
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
          const point_t& ocolor = obj.active ? obj.spec->active : obj.spec->color;
          color_t color = colors.palette->at(ocolor.x, ocolor.y);

          if (obj.spec == level->data()->EDGE)
          {
            //gvm->fillRect(dest, colors.outside);
          }
          else
          {
            const auto* asset = cache->objectGfx(obj.spec);

            auto variant = obj.variant;
            if (obj.spec->tiling == ObjectSpec::Tiling::None)
              variant = 0;

            bool isFloat = level->hasProperty(obj.spec, ObjectProperty::FLOAT);
            float dy = ratio * ftick;

            rect_t src = asset->rect(variant);

            int fy = offset.y + y * tileSize + tileSize / 2 - (src.h*ratio) / 2;
            if (isFloat) fy += dy;

            const rect_t dest = { offset.x + x * tileSize + tileSize/2 - (src.w*ratio)/2, fy, src.w * ratio, src.h * ratio };

            src.x += tick * src.w;

            _renderer->blit(asset, color, src, dest);
          }
        }

        if (tile->link)
          renderLevelLink(x, y);
      }
    }
  }

  drawGrid(offset, { tileSize, tileSize }, { level->width() + 1, level->height() + 1 });

  _director->text(level->info().filename, 5, 5, { 255, 255, 255 }, ui::TextAlign::LEFT, 1.0f);
  const auto& rulesList = level->rules().rules();
  for (coord_t i = 0; i < rulesList.size(); ++i)
    _director->text(rulesList[i].name(), 5, 5 + (i + 1) * 10, { 255, 255, 255 }, ui::TextAlign::LEFT, 1.0f);


  if (level->isVictory())
    _director->text("Victory!", size.w - 5, 5, { 255, 255, 0 }, ui::TextAlign::RIGHT, 1.0f);
  else if (level->isDefeat())
    _director->text("Defeat!", size.w - 5, 5, { 255, 0, 0 }, ui::TextAlign::RIGHT, 1.0f);

  _director->text(level->filename() + " - " + level->name(), size.w / 2, size.h - 20, { 255, 255, 255 }, ui::TextAlign::CENTER, 1.0f);

  //_director->text(cache->numberedGfx("small_font"), "la penna è sul tavolo", 50, 50);

#if MOUSE_ENABLED
  _director->text(hoverInfo, 5, size.h - 10, { 255, 255, 255 }, ui::TextAlign::LEFT, 1.0f);
#endif
}

void GameView::drawGrid(point_t b, size2d_t size, size2d_t count)
{
  if (colors.grid.a)
  {
    for (int i = 0; i < count.h; ++i)
      _renderer->line(b.x, b.y + size.h * i, b.x + size.w * count.w, b.y + size.h * i, colors.grid);

    for (int i = 0; i < count.w; ++i)
      _renderer->line(b.x + size.w * i, b.y, b.x + size.w * i, b.y + size.h * count.h, colors.grid);
  }
}

void GameView::updateMoveBounds()
{
  world->level()->forEachTile([this] (Tile& tile) {
    if (tile.has(ObjectProperty::YOU))
    {
      moveBounds[0].x = std::min(moveBounds[0].x, tile.x());
      moveBounds[0].y = std::min(moveBounds[0].y, tile.y());

      moveBounds[1].x = std::max(moveBounds[1].x, tile.x());
      moveBounds[1].y = std::max(moveBounds[1].y, tile.y());
    }
  });
}


#include "io/Assets.h"

void GameView::movement(D d)
{
  history.push(world->level()->state());

  auto result = world->level()->movement(d);

  if (result.type == MoveResult::Type::LevelEnter)
  {
    world->pushLevel(result.levelName);
    levelLoaded();
  }
}

void GameView::handleKeyboardEvent(const events::KeyEvent& event)
{
  if (event.press)
  { 
    switch (event.code)
    {
      case KeyCode::BindExit: 
        world->popLevel();
        levelLoaded();
        //gvm->exit(); 
      break;

        
      case KeyCode::BindLeft: movement(D::LEFT); updateMoveBounds(); break;
      case KeyCode::BindRight: movement(D::RIGHT); updateMoveBounds(); break;
      case KeyCode::BindUp: movement(D::UP); updateMoveBounds(); break;
      case KeyCode::BindDown: movement(D::DOWN); updateMoveBounds(); break;
      case KeyCode::BindWait: 
      {
        movement(D::NONE);
        updateMoveBounds();
        break;
      }

      case KeyCode::KeyG:
      {
        if (colors.grid.a) colors.grid.a = 0;
        else colors.grid = colors.outside;
        break;
      }

      case KeyCode::KeyR:
      case KeyCode::KeyKpPlus:
      { 
        assets()->flushCache();
        //nextLevel();
        break;
      }
      case KeyCode::KeyL:
      case KeyCode::KeyKpMinus:
      { 
        assets()->flushCache();
        //prevLevel();
        break;
      }

      case KeyCode::BindUndo:
        if (!history.empty())
        {
          world->level()->restore(history.pop());
          world->level()->updateRules();
        }
      
        break;

      case KeyCode::KeyM:
      {
        if (scaler == Scaler::NATIVE_WITH_SCROLL)
          scaler = Scaler::SCALE_TO_ATMOST_NATIVE;
        else if (scaler == Scaler::SCALE_TO_ATMOST_NATIVE)
          scaler = Scaler::NATIVE_WITH_SCROLL;
        break;
      }

      /*
      case SDLK_w:
      {        
        //if (offset.y >= GFX_TILE_SIZE)
          offset.y -= GFX_TILE_SIZE;
        break;
      }

      case SDLK_s:
      {
        size2d_t size = gvm->getWindowSize();

        //if (offset.y + size.h / GFX_TILE_SIZE <= GFX_TILE_SIZE * level->height())
          offset.y += GFX_TILE_SIZE;
      }

      case SDLK_a:
      {
        //if (offset.x >= GFX_TILE_SIZE)
          offset.x -= GFX_TILE_SIZE;
        break;
      }

      case SDLK_d:
      {
        size2d_t size = gvm->getWindowSize();
        
        //if (offset.x + size.w / GFX_TILE_SIZE <= GFX_TILE_SIZE * level->width())
          offset.x += GFX_TILE_SIZE;
      }*/
    }
  }
}

void GameView::handleMouseEvent(const events::MouseEvent& event)
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
