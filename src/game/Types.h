#pragma once

#include "Common.h"

#include <vector>
#include <unordered_map>

namespace baba
{
  using coord_t = int32_t;
  
  struct ObjectSpec
  {
    int32_t id;
    std::string name;
    std::string sprite;
  };

  struct GameData
  {
    std::vector<ObjectSpec> objects;
    std::unordered_map<int32_t, const ObjectSpec*> objectsByID;

    void finalize()
    {
      for (const auto& spec : objects)
        objectsByID[spec.id] = &spec;
    }
  };

  struct Object
  {
    const ObjectSpec* spec;
  };

  struct Coordinate
  {
    coord_t x, y;
  };

  struct Tile
  {
    std::vector<Object> objects;
    Coordinate coord;

    void add(Object object) { objects.push_back(object); }
    const Object* object() const { return !objects.empty() ? &objects[0] : nullptr; }
  };

  struct Level
  {
  private:
    coord_t _width, _height;
    std::vector<Tile> _tiles;

  public:
    Level(coord_t width, coord_t height) : _width(width), _height(height)
    {
      _tiles.resize(width*height);
      for (coord_t y = 0; y < height; ++y)
        for (coord_t x = 0; x < width; ++x)
          get(x, y)->coord = { x, y };
    }

    Tile* get(coord_t i) { return &_tiles[i]; }

    Tile* get(coord_t x, coord_t y) { return &_tiles[y*_width + x]; }
    const Tile* get(coord_t x, coord_t y) const { return &_tiles[y*_width + x]; }
  };
}
