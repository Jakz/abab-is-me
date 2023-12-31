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

          if (right->has(object.spec)) object.variant |= 1;
          if (up->has(object.spec)) object.variant |= 2;
          if (left->has(object.spec)) object.variant |= 4;
          if (down->has(object.spec)) object.variant |= 8;
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