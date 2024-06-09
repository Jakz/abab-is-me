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
    Word      = 0x00100000ULL,
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
    auto begin() const { return terms.begin(); }
  };

  

  namespace parser
  {
    enum class SymbolType { Noun, Property, Verb };

    using token_t = const Object*;
    using token_sequence = std::vector<token_t>;

    class RuleParser
    {
      const GameData* _data;
      const Rule* _rule;
      token_sequence _tokens;

      token_sequence::const_iterator _it;

      void advance() { ++_it; }
      void rewind() { --_it; }

      const token_t& peek() const { return *_it; }
      decltype(_it) previous() { return std::prev(_it); }

      bool accept(SymbolType type);

      bool match(SymbolType type)
      {
        bool matched = accept(type);

        if (matched)
          advance();

        return matched;
      }

    public:
      struct Subject { Subject(const Object* object) : object(object) { } const Object* object; };
      

    public:
      RuleParser(const GameData* data) : _data(data), _rule(nullptr)
      {

      }

      Subject* subject()
      {
        if (match(SymbolType::Noun))
        {
          return new Subject(*previous());
        }
      }

      Rule* rule()
      {
        auto subj = subject();
      }

      void parse(const token_sequence& tokens)
      {
        _tokens = tokens;
        _it = _tokens.begin();
      }
    };
  }

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