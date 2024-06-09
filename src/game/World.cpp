#include "World.h"

#include "game/Level.h"

#include "io/Loader.h"

using namespace baba;

World::World() : _level(nullptr)
{
  _loader.setDataFolder(path(DATA_PATH).append("Data/"));
  _data = _loader.loadGameData();
}

void World::loadLevel(const std::string& name, bool updateStack)
{
  if (_level && updateStack)
    _levelStack.push_back(_level->info().filename);
  
  delete _level;
  _level = nullptr;

  _level = _loader.load(name, _data);
  _level->updateRules();
  _level->computeTiling();
}

void World::popLevel()
{
  if (!_levelStack.empty())
  {
    std::string name = _levelStack.back();
    _levelStack.pop_back();

    loadLevel(name, false);
  }
}

void World::pushLevel(std::string name)
{
  loadLevel(name, true);
}