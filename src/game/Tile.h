#pragma once

#include "Types.h"

#include <vector>
#include <functional>

namespace baba
{
  struct Tile
  {
    std::vector<Object> objects;
    point_t coord;

    void add(Object object) { objects.push_back(object); }
    const Object* object() const { return !objects.empty() ? &objects[0] : nullptr; }

    bool empty() const { return objects.empty(); }

    bool has(const ObjectSpec* spec) const;
    bool any_of(const std::function<bool(const Object&)>& predicate) const;
    Object* find(const std::function<bool(const Object&)>& predicate);
    Object* find(const ObjectSpec* spec) { return find([spec](const Object& obj) { return obj.spec == spec; }); }



    decltype(objects)::const_iterator begin() const { return objects.begin(); }
    decltype(objects)::const_iterator end() const { return objects.end(); }
    decltype(objects)::iterator begin() { return objects.begin(); }
    decltype(objects)::iterator end() { return objects.end(); }

    coord_t x() const { return coord.x; }
    coord_t y() const { return coord.y; }
  };
}
