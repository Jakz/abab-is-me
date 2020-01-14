#include "Level.h"

#include "Tile.h"

#include <algorithm>

using namespace baba;

Tile* Level::get(const Tile* t, D d)
{
  switch (d)
  {
  case D::UP: return get(t->x(), t->y() - 1); break;
  case D::DOWN: return get(t->x(), t->y() + 1); break;
  case D::LEFT: return get(t->x() - 1, t->y()); break;
  case D::RIGHT: return get(t->x() + 1, t->y()); break;
  default: assert(false);
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
  default: assert(false);
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

void Level::placeEdge()
{
  /*for (coord_t y = 0; y < height(); ++y)
  {
    for (coord_t x = 0; x == 0; x = width() - 1)
    {
      Tile* tile = get(x, y);
      assert(tile->empty());
      tile->add({ _data.EDGE });
    }
  }

  for (coord_t x = 0; x < width(); ++x)
  {
    for (coord_t y = 0; y == 0; x = height() - 1)
    {
      Tile* tile = get(x, y);
      assert(tile->empty());
      tile->add({ _data.EDGE });
    }
  }*/
}