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
    WIN     = 0x00000008,
    DEFEAT  = 0x00000010,
  };

  using ObjectProperties = bit_mask<ObjectProperty>;

  struct ObjectState
  {
    ObjectProperties properties;

    void clear() { properties.clear(); }
  };


  struct Rule
  {
    std::vector<Object*> terms;
    std::string name() const;

    Object* operator[](size_t i) const { return terms[i]; }
  };

  struct Rules
  {
  private:
    const GameData* _data;
    std::unordered_map<const ObjectSpec*, ObjectState> _state;
    std::vector<Rule> _rules;

  public:
    Rules(const GameData* data) : _data(data) { }

    ObjectState& state(const ObjectSpec* spec) { return _state[spec]; }

    decltype(_state)::iterator begin() { return _state.begin(); }
    decltype(_state)::iterator end() { return _state.end(); }

    const decltype(_rules)& rules() const { return _rules; }

    bool hasProperty(const ObjectSpec* spec, ObjectProperty property) { return state(spec).properties.isSet(property); }

    void clear();
    void generate(baba::Level* level);
    void apply();

    void resolve(baba::Level* level);
  };

}
