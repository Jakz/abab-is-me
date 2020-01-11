#pragma once

#include "Types.h"

#include <unordered_map>

namespace baba
{
  enum class ObjectProperty
  {
    YOU     = 0x00000001,
    STOP    = 0x00000002,
  };

  using ObjectProperties = bit_mask<ObjectProperty>;

  struct ObjectState
  {
    ObjectProperties properties;

    void clear() { properties.clear(); }
  };

  struct Rules
  {
  private:
    const GameData* data;
    std::unordered_map<const ObjectSpec*, ObjectState> _state;

  public:
    Rules(const GameData* data) : data(data) { }

    ObjectState& state(const ObjectSpec* spec) { return _state[spec]; }

    void clear();
    void generate(baba::Level* level);
  };

}