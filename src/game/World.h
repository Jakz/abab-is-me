#pragma once

#include "game/Types.h"
#include "io/Loader.h"

#include <unordered_set>
#include <string>

namespace baba
{
  class Level;
  
  class World
  {
  protected:
    std::unordered_set<std::string> _won;

    
    io::Loader _loader;
    GameData _data;

    std::vector<std::string> _levelStack;
    Level* _level;

  public:
    World();
    
    bool isWon(const std::string& levelName) { return _won.contains(levelName); }

    void loadLevel(const std::string& name, bool updateStack = false);

    void popLevel();
    void pushLevel(std::string name);

    const auto& data() { return _data; }

    Level* level() { return _level; }
  };
}