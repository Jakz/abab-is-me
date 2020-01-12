#pragma once

#include "Types.h"

#include <unordered_map>

namespace baba
{
  enum class ObjectProperty
  {
    YOU     = 0x00000001,
    STOP    = 0x00000002,
    PUSH    = 0x00000004,
  };

  using ObjectProperties = bit_mask<ObjectProperty>;

  struct ObjectState
  {
    ObjectProperties properties;

    void clear() { properties.clear(); }
  };

  using Rule = std::vector<Object*>;

  struct Rules
  {
  private:
    const GameData* _data;
    std::unordered_map<const ObjectSpec*, ObjectState> _state;
    std::vector<Rule> _rules;

  public:
    Rules(const GameData* data) : _data(data) { }

    ObjectState& state(const ObjectSpec* spec) { return _state[spec]; }

    bool hasProperty(const ObjectSpec* spec, ObjectProperty property) { return state(spec).properties.isSet(property); }

    void clear();
    void generate(baba::Level* level);
    void apply();
  };

}