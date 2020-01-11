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
  };

  enum class D { UP, DOWN, LEFT, RIGHT };
  
  struct ObjectSpec
  {
    enum class Type { Noun = 0, Verb, Property, Adjective, Negative, Unused, Conjunction, Preposition };
    enum class Tiling { None = -1, Directions, Tiled, Character, Belt };

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

    const ObjectSpec* IS = nullptr;

    void finalize()
    {
      for (const auto& spec : objects)
      {
        objectsByID[spec.id] = &spec;
        objectsByName[spec.name] = &spec;
      }

      IS = objectsByName.find("is")->second;
    }
  };

  struct Object
  {
    const ObjectSpec* spec;
    uint32_t variant;
    bool alreadyMoved;
    bool active;

    Object(const ObjectSpec* spec) : spec(spec), variant(0), alreadyMoved(false) { }
  };
}
