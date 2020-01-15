#pragma once

#include "Common.h"

#include <vector>
#include <unordered_map>

namespace baba
{
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
    std::string key;
    bool spriteInRoot;
    bool isText;
    Tiling tiling;
    int32_t layer;
  };

  struct GameData
  {
    std::vector<ObjectSpec> objects;
    std::unordered_map<int32_t, const ObjectSpec*> objectsByID;
    std::unordered_map<std::string, ObjectSpec*> objectsByKey;
    std::unordered_map<std::string, const ObjectSpec*> objectsByName;
    std::unordered_map<point_t, const ObjectSpec*, point_t::hash> objectsByGrid;

    const ObjectSpec* IS = nullptr;
    const ObjectSpec* EDGE = nullptr;

    GameData() = default;

    GameData(const GameData& other)
    {
      this->objects = other.objects;
      finalize();
    }

    GameData& operator=(const GameData& other)
    {
      this->objects = other.objects;
      finalize();
    }

    void finalize()
    {
      clearMaps();
      
      for (auto& spec : objects)
      {
        objectsByID[spec.id] = &spec;
        objectsByName[spec.name] = &spec;
        objectsByGrid[spec.grid] = &spec;
        objectsByKey[spec.key] = &spec;
      }

      mapDefault(IS, "text_is");
      mapDefault(EDGE, "edge");
    }

  private:
    void mapDefault(const ObjectSpec*& dest, const char* name)
    {
      auto it = objectsByName.find(name);
      assert(it != objectsByName.end());
      dest = it->second;
    }

    void clearMaps()
    {
      objectsByID.clear();
      objectsByKey.clear();
      objectsByName.clear();
      objectsByGrid.clear();
    }
  };

  struct Object
  {
    const ObjectSpec* spec;
    D direction;
    uint32_t variant;
    bool alreadyMoved;
    bool active;

    Object(const ObjectSpec* spec) : spec(spec), variant(0), alreadyMoved(false), active(false) { }
  };
}
