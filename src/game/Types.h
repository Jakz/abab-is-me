#pragma once

#include "Common.h"

#include <vector>
#include <unordered_map>

namespace baba
{
  using coord_t = int32_t;

  struct point_t
  {
    coord_t x, y;

    bool operator==(const point_t& o) const 
    {
      return x == o.x && y == o.y;
    }

    struct hash
    {
      size_t operator()(const point_t& p) const { return p.y << 16 | p.x; }
    };
  };

  enum class D { UP, DOWN, LEFT, RIGHT };
  
  struct ObjectSpec
  {
    enum class Type { Noun = 0, Verb, Property, Adjective, Negative, Unused, Conjunction, Preposition };
    enum class Tiling { None = -1, Directions, Tiled, Character, Belt };

    Type type;
    int32_t id;
    point_t color;
    point_t active;
    point_t grid;
    std::string name;
    std::string sprite;
    bool spriteInRoot;
    bool isText;
    Tiling tiling;
    int32_t layer;
  };

  struct GameData
  {
    std::vector<ObjectSpec> objects;
    std::unordered_map<int32_t, const ObjectSpec*> objectsByID;
    std::unordered_map<std::string, const ObjectSpec*> objectsByName;
    std::unordered_map<point_t, const ObjectSpec*, point_t::hash> objectsByGrid;

    const ObjectSpec* IS = nullptr;

    void finalize()
    {
      for (const auto& spec : objects)
      {
        objectsByID[spec.id] = &spec;
        objectsByName[spec.name] = &spec;
        objectsByGrid[spec.grid] = &spec;
      }

      IS = objectsByName.find("text_is")->second;
    }
  };

  struct Object
  {
    const ObjectSpec* spec;
    uint32_t variant;
    bool alreadyMoved;
    bool active;

    Object(const ObjectSpec* spec) : spec(spec), variant(0), alreadyMoved(false), active(false) { }
  };
}
