#include <cstdio>
#include <cstdlib>

#include "gfx/ViewManager.h"
#include "gfx/MainView.h"

#include "io/Loader.h"

#include "game/Types.h"
#include "game/Level.h"

baba::GameData data;
baba::Level* level = nullptr;
int levelIndex = 0;

io::Loader loader;
ui::ViewManager vm;

void nextLevel()
{
  delete level;
  level = nullptr;

  while (!level)
  {
    ++levelIndex;
    level = loader.load(std::to_string(levelIndex) + "level", data);
  }

  level->computeTiling();
  vm.gameView()->levelLoaded();
}

void prevLevel()
{
  delete level;
  level = nullptr;

  while (!level)
  {
    --levelIndex;
    level = loader.load(std::to_string(levelIndex) + "level", data);
  }

  level->computeTiling();
  vm.gameView()->levelLoaded();
}

int main(int argc, char* argv[])
{
  data = loader.loadGameData();
  level = loader.load(std::to_string(levelIndex) + "level", data);
  level->computeTiling();
  
  if (!vm.init())
    return -1;

  if (!vm.loadData())
  {
    printf("Error while loading and initializing data.\n");
    vm.deinit();
    return -1;
  }

  vm.gameView()->levelLoaded();

  vm.loop();
  vm.deinit();

  //loader.load("1level.l");

  getchar();
  return 0;
}