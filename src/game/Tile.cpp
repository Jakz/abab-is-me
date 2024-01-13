#include "Tile.h"

#include <algorithm>

#include "Rules.h"
#include "Level.h"

using namespace baba;

bool Tile::has(const ObjectSpec* spec) const
{
  return any_of([spec](const Object& object) { return object.spec == spec; });
}

bool Tile::has(ObjectProperty property) const
{
  return any_of([this, property](const Object& object) { return level->hasProperty(object.spec, property); });
}

Object* Tile::find(ObjectProperty property)
{
  auto object = find([this, property](const Object& object) { return level->hasProperty(object.spec, property); });
  return object;
}

bool Tile::any_of(const std::function<bool(const Object&)>& predicate) const
{
  return std::any_of(objects.begin(), objects.end(), predicate);
}

Object* Tile::find(const std::function<bool(const Object&)>& predicate)
{
  auto it = std::find_if(objects.begin(), objects.end(), predicate);
  return it != objects.end() ? &(*it) : nullptr;
}

void Tile::remove(const Object* object)
{
  for (auto it = begin(); it != end(); /**/)
    if (object == &(*it))
    {
      it = objects.erase(it);
      return;
    }
    else
      ++it;
}
