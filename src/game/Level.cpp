#include "Level.h"

#include "Tile.h"
#include "Rules.h"

#include <algorithm>

using namespace baba;

Level::Level(const GameData& data) : _data(data), _rules(&_data), _width(0), _height(0)
{
}

void Level::resize(coord_t width, coord_t height)
{
  _width = width;
  _height = height;
  
  _tiles.resize(width*height);

  for (coord_t y = 0; y < height; ++y)
    for (coord_t x = 0; x < width; ++x)
      *get(x, y) = Tile(this, { x, y });
}

Tile* Level::get(const Tile* t, D d)
{
  switch (d)
  {
  case D::UP: return get(t->x(), t->y() - 1); break;
  case D::DOWN: return get(t->x(), t->y() + 1); break;
  case D::LEFT: return get(t->x() - 1, t->y()); break;
  case D::RIGHT: return get(t->x() + 1, t->y()); break;
  default: 
    assert(false);
    return nullptr;
  }
}

const Tile* Level::get(const Tile* t, D d) const
{
  switch (d)
  {
  case D::UP: return get(t->x(), t->y() - 1); break;
  case D::DOWN: return get(t->x(), t->y() + 1); break;
  case D::LEFT: return get(t->x() - 1, t->y()); break;
  case D::RIGHT: return get(t->x() + 1, t->y()); break;
  default: 
    assert(false);
    return nullptr;
  }
}

void Level::computeTiling()
{  
  /* set links on tiles */
  for (const auto& link : _metalevel._levels)
    get(link.coord)->link = &link;
  
  for (coord_t y = 0; y < height(); ++y)
  {
    for (coord_t x = 0; x < width(); ++x)
    {
      Tile* tile = get(x, y);

      for (auto& object : tile->objects)
      {
        if (object.spec->tiling == baba::ObjectSpec::Tiling::Tiled)
        {
          object.variant = 0;

          auto up = get(tile, D::UP), down = get(tile, D::DOWN);
          auto left = get(tile, D::LEFT), right = get(tile, D::RIGHT);

          if (right->has(object.spec) || right->link) object.variant |= 1;
          if (up->has(object.spec) || up->link) object.variant |= 2;
          if (left->has(object.spec) || left->link) object.variant |= 4;
          if (down->has(object.spec) || down->link) object.variant |= 8;
        }
      }
    }
  }
}

void Level::forEachObject(std::function<void(Object&)> lambda)
{
  std::for_each(_tiles.begin(), _tiles.end(), [&lambda](Tile& tile) {
    std::for_each(tile.begin(), tile.end(), [&lambda](Object& object) {
      lambda(object);
    });
  });
}

void Level::forEachTile(std::function<void(Tile&)> lambda)
{
  std::for_each(_tiles.begin(), _tiles.end(), [&lambda](Tile& tile) {
    lambda(tile);
  });
}

bool Level::isDefeat()
{
  bool anyYou = false;
  forEachObject([&anyYou, this](const Object& object) {
    anyYou |= _rules.hasProperty(object.spec, ObjectProperty::YOU);
  });

  return !anyYou;
}

bool Level::isVictory()
{
  bool anyVictory = false;
  forEachTile([&anyVictory](const Tile& tile) {
    const bool hasYou = tile.has(ObjectProperty::YOU);
    const bool hasWin = tile.has(ObjectProperty::WIN);

    anyVictory |= (hasYou && hasWin);
  });

  return anyVictory;
}

void Level::updateRules()
{
  _rules.clear();
  _rules.generate(this);
  _rules.apply(this);
}


#include "game/World.h"
extern baba::World* world;
bool operator&&(const ObjectSpec* spec, ObjectProperty prop)
{
  return world->level()->rules().hasProperty(spec, prop);
}

struct MoveInfo
{
  enum class Type { YOU, MOVE, CURSOR } type;
  Tile* tile;
  decltype(Tile::objects)::iterator it;
  MoveInfo(Type type, Tile* tile, decltype(it) it) : type(type), tile(tile), it(it) { }
};

bool Level::isMovementAllowed(MoveInfo info, D d)
{
  bool allowed = true;
  bool finished = false;

  Tile* next = get(info.tile, d);

  while (next)
  {
    if (next->has(ObjectProperty::STOP))
      return false;
    else if (!next->has(ObjectProperty::PUSH))
      return true;

    next = get(next, d);
  }

  return true;
}

bool Level::movement(MoveInfo info, D d)
{
  Tile* tile = info.tile;
  Tile* next = get(tile, d);

  const auto& it = info.it;

  bool isStopped = false;
  bool updateVariant = false;

  //TODO: move in its own method
  if (info.type == MoveInfo::Type::CURSOR)
  {
    Object object = *info.it;
    Tile* tile = info.tile;

    tile->objects.erase(info.it);
    get(tile, d)->objects.push_back(object);

    return true;
  }

  if (next->empty())
  {
  }
  else
  {
    for (auto nit = next->begin(); nit != next->end(); ++nit)
    {
      if (it->spec && ObjectProperty::YOU && next->has(ObjectProperty::DEFEAT))
      {
        //const auto& data = assets()->sound(AssetMapping::DEFEAT_SOUND.random());
        //Mix_PlayChannel(-1, data.nativeData.chunk(), 0);

        tile->objects.erase(it);
        return false;
      }

      //TODO: if it has SINK this should be done too
      auto sink = next->find(ObjectProperty::SINK);
      if (sink) //TODO: only on same float level
      {
        tile->objects.erase(it);
        next->remove(sink);
        computeTiling(); //TODO: expensive, just do it around affected tile
        return false;
      }

      /* next is stop, just break from the cycle, we can't move */
      if (nit->spec && ObjectProperty::STOP)
      {
        if (info.type == MoveInfo::Type::MOVE)
        {
          it->direction = ~it->direction;
          updateVariant = true;
        }

        isStopped = true;

        break;
      }
      /* it's push, it could move must we must see if it can be moved too*/
      else if (nit->spec && ObjectProperty::PUSH)
      {
        /* if current can be moved we check next, otherwise we can stop */
        isStopped |= !movement(MoveInfo(info.type, next, nit), d);
        if (!isStopped && next->begin() != next->end())
          nit = next->begin(); // restart
        else
        {
          if (isStopped && info.type == MoveInfo::Type::MOVE)
            it->direction = ~it->direction;

          break;
        }
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

  // TODO: shift
  /* for each object which has SHIFT move all object on it according to the direction */

  if (!isStopped)
  {
    Object object = *info.it;
    Tile* tile = info.tile;

    tile->objects.erase(info.it);
    get(tile, d)->objects.push_back(object);

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

MoveResult Level::movement(D d)
{
  //const auto& data = gfx::Gfx::i.cache()->sound(AssetMapping::BABA_STEP_SOUND.random());
  //Mix_PlayChannel(-1, data.nativeData.chunk(), 0);

  forEachObject([] (Object& object) { object.alreadyMoved = false; });

  std::vector<MoveInfo> you;
  std::vector<std::pair<MoveInfo, D>> move;

  for (coord_t y = 0; y < height(); ++y)
    for (coord_t x = 0; x < width(); ++x)
    {
      Tile* tile = get(x, y);

      for (auto it = tile->begin(); it != tile->end(); ++it)
      {
        if (it->spec && ObjectProperty::YOU)
          you.emplace_back(MoveInfo::Type::YOU, tile, it);
        if (it->spec && ObjectProperty::SELECT)
          you.emplace_back(MoveInfo::Type::CURSOR, tile, it);
        if (it->spec && ObjectProperty::MOVE)
          move.emplace_back(MoveInfo(MoveInfo::Type::MOVE, tile, it), it->direction);
      }
    }

  if (!you.empty())
  {
    if (d != D::NONE)
      for (const auto& pair : you)
        movement(pair, d);
    /* wait you is select -> enter level */
    else if (d == D::NONE && !you.empty() && you[0].type == MoveInfo::Type::CURSOR)
    {
      if (isMeta() && !metalevel()._levels.empty())
      {
        const auto& levels = metalevel()._levels;
        auto it = std::find_if(levels.begin(), levels.end(), [coord = you[0].tile->coord] (const baba::LevelLink& link)
          {
          return coord == link.coord;
        });

        if (it != levels.end())
        {
          return MoveResult(MoveResult::Type::LevelEnter, it->file);
        }
      }


    }

    for (const auto& pair : move)
      movement(pair.first, pair.second);

    for (auto& tile : *this)
      std::sort(tile.begin(), tile.end(), [] (const Object& o1, const Object& o2) { return o1.spec->layer < o2.spec->layer; });

    forEachObject([] (Object& object) { object.active = false; });
    updateRules();

    for (baba::Tile& tile : *this)
    {
      bool hasHot = tile.any_of([] (const Object& o) { return o.spec && ObjectProperty::HOT; });

      if (hasHot)
      {
        const Object* o = nullptr;
        while ((o = tile.find(ObjectProperty::MELT)))
        {
          tile.remove(o);
          /* burn particle + sound */
        }
      }
    }

    for (const auto& props : rules())
    {
      auto* spec = props.first;

      if (props.second.properties.isSet(ObjectProperty::WIN))
        forEachObject([spec] (Object& object) { if (object.spec == spec) object.active = true; });
    }
  }

  return MoveResult(MoveResult::Type::Ok);
}