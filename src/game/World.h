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
    std::unordered_set<std::string> _unlocked;
    
    io::Loader _loader;
    GameData _data;

    std::vector<std::string> _levelStack;
    Level* _level;

  public:
    World();
    
    bool isUnlocked(const std::string& levelName) { return _unlocked.contains(levelName); }

    void loadLevel(const std::string& name, bool updateStack = false);

    void popLevel();
    void pushLevel(std::string name);

    Level* level() { return _level; }
  };
}