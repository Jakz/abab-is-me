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

    bool has(const ObjectSpec* spec) const;
    bool any_of(const std::function<bool(const Object&)>& predicate) const;
    Object* find(const std::function<bool(const Object&)>& predicate);
    Object* find(const ObjectSpec* spec) { return find([spec](const Object& obj) { return obj.spec == spec; }); }

    auto begin() const { return objects.begin(); }
    auto end() const { return objects.end(); }
    auto begin() { return objects.begin(); }
    auto end() { return objects.end(); }

    coord_t x() const { return coord.x; }
    coord_t y() const { return coord.y; }
  };
}
