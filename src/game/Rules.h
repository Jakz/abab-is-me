#pragma once

#include "Types.h"

#include <unordered_map>

namespace baba
{
  enum class ObjectProperty : uint64_t
  {
    YOU       = 0x00000001ULL,
    STOP      = 0x00000002ULL,
    PUSH      = 0x00000004ULL,
    WIN       = 0x00000008ULL,
    DEFEAT    = 0x00000010ULL,
    MOVE      = 0x00000020ULL,
    FLOAT     = 0x00000040ULL,
    SINK      = 0x00000080ULL,
    SHIFT     = 0x00000100ULL,
    BEST      = 0x00000200ULL,
    HOT       = 0x00000400ULL,
    MELT      = 0x00000800ULL,
    TEXT      = 0x00001000ULL,
    SHUT      = 0x00002000ULL,
    OPEN      = 0x00004000ULL,
    WEAK      = 0x00008000ULL,
    MORE      = 0x00010000ULL,
    TELE      = 0x00020000ULL,
    SELECT    = 0x00040000ULL,
    PULL      = 0x00080000ULL,
  };

  using ObjectProperties = bit_mask<ObjectProperty>;

  struct ObjectState
  {
    ObjectProperties properties;
    std::vector<const baba::ObjectSpec*> products;

    void clear() { properties.clear(); }
  };


  struct Rule
  {
    std::vector<const Object*> terms;
    std::string name() const;

    const Object* operator[](size_t i) const { return terms[i]; }
  };

  struct Rules
  {
  private:
    const GameData* _data;
    mutable std::unordered_map<const ObjectSpec*, ObjectState> _state;
    std::vector<Rule> _rules;

    void generateBaseRules(baba::Level* level);

  public:
    Rules(const GameData* data) : _data(data) { }

    ObjectState& state(const ObjectSpec* spec) const { return _state[spec]; }

    decltype(_state)::iterator begin() { return _state.begin(); }
    decltype(_state)::iterator end() { return _state.end(); }
    decltype(_state)::const_iterator begin() const { return _state.begin(); }
    decltype(_state)::const_iterator end() const { return _state.end(); }

    const decltype(_rules)& rules() const { return _rules; }

    bool hasProperty(const ObjectSpec* spec, ObjectProperty property) const { return state(spec).properties.isSet(property); }

    void clear();
    void generate(baba::Level* level);
    void apply(baba::Level* level);

    void resolve(baba::Level* level);
  };
}