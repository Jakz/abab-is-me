#pragma once

#include "Types.h"

#include <vector>
#include <functional>

namespace baba
{  
  struct Tile
  {
    const Level* level;
    point_t coord;

    const LevelLink* link;
    std::vector<Object> objects;

    Tile() = default;
    Tile(const Level* level, point_t coord) : level(level), coord(coord), link(nullptr) { }

    void add(Object object) { objects.push_back(object); }
    const Object* object() const { return !objects.empty() ? &objects[0] : nullptr; }

    void remove(const Object* object);

    bool empty() const { return objects.empty(); }

    bool has(const ObjectSpec* spec) const;
    bool has(ObjectProperty property) const;
    Object* find(ObjectProperty property);


    bool any_of(const std::function<bool(const Object&)>& predicate) const;
    Object* find(const std::function<bool(const Object&)>& predicate);
    Object* find(const ObjectSpec* spec) { return find([spec](const Object& obj) { return obj.spec == spec; }); }

    bool isAllowedForCursorMovement() const { return link || any_of([] (const Object& o) { return o.path; }); }


    decltype(objects)::const_iterator begin() const { return objects.begin(); }
    decltype(objects)::const_iterator end() const { return objects.end(); }
    decltype(objects)::iterator begin() { return objects.begin(); }
    decltype(objects)::iterator end() { return objects.end(); }

    coord_t x() const { return coord.x; }
    coord_t y() const { return coord.y; }
  };
}
