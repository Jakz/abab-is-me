#pragma once

#include "Common.h"

#include <vector>
#include <unordered_map>
#include <algorithm>

namespace baba
{
  using coord_t = int32_t;

  struct point_t
  {
    coord_t x, y;
  };

  enum class D { UP, DOWN, LEFT, RIGHT };
  
  struct ObjectSpec
  {
    enum class Type { Noun = 0, Verb, Property, Adjective, Negative, Unused, Conjunction, Preposition };
    enum class Tiling { None = -1, Zero, Ortho, Player, Belt };

    Type type;
    int32_t id;
    point_t color;
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

    void finalize()
    {
      for (const auto& spec : objects)
      {
        objectsByID[spec.id] = &spec;
        objectsByName[spec.name] = &spec;
      }
    }
  };

  struct Object
  {
    const ObjectSpec* spec;
    uint32_t variant;
    bool alreadyMoved;

    Object(const ObjectSpec* spec) : spec(spec), variant(0), alreadyMoved(false) { }
  };

  struct Tile
  {
    std::vector<Object> objects;
    point_t coord;

    void add(Object object) { objects.push_back(object); }
    const Object* object() const { return !objects.empty() ? &objects[0] : nullptr; }

    bool has(const ObjectSpec* spec) const {
      return std::any_of(objects.begin(), objects.end(), [spec](const Object& object) { return object.spec == spec; });
    }

    auto begin() const { return objects.begin(); }
    auto end() const { return objects.end(); }
    auto begin() { return objects.begin(); }
    auto end() { return objects.end(); }

    coord_t x() const { return coord.x; }
    coord_t y() const { return coord.y; }
  };
}
