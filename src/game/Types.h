#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "Common.h"

namespace baba
{
  class World;
  class Level;
  
  enum class D { UP, DOWN, LEFT, RIGHT, NONE, INVALID };

  static inline D operator~(D d)
  {
    switch (d)
    { 
      case D::DOWN: return D::UP; 
      case D::UP: return D::DOWN; 
      case D::LEFT: return D::RIGHT; 
      case D::RIGHT: return D::LEFT;
      case D::NONE: return D::NONE;
      case D::INVALID: return D::INVALID;
    }
  }

  struct ObjectSpec
  {
    enum class Type { Noun = 0, Verb, Property, Adjective, Negative, Unused, Conjunction, Preposition };
    enum class Tiling { None = -1, Directions, Tiled, Character, Animated, SingleAnimated }; //TODO: unknown, eg cog in 7level

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

    const ObjectSpec* HAS = nullptr;
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
      return *this;
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
      mapDefault(HAS, "text_has");
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
    bool path;
    mutable bool active;

    Object(const ObjectSpec* spec) : spec(spec), variant(0), alreadyMoved(false), path(false), active(false) { }
  };

  struct Icon
  {
    std::string sprite;
    bool spriteInRoot;

    Icon(const std::string& sprite, bool spriteInRoot) : sprite(sprite), spriteInRoot(spriteInRoot) { }
    Icon() : Icon("", false) { }
  };

  struct LevelLink
  {
    enum class Style { Number, Icon, Letter, Dot };
    enum class State { Hidden = 0, Normal = 1, Opened = 2 };

    point_t coord;
    coord_t z;
    int32_t number;
    point_t color;
    point_t clearcolour;
    Style style;
    State state;
    std::string file;

    coord_t x() const { return coord.x; }
    coord_t y() const { return coord.y; }
  };
}
