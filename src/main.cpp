#include <cstdio>
#include <cstdlib>

#include "gfx/ViewManager.h"
#include "gfx/views/Views.h"

#include "io/Loader.h"

#include "game/Types.h"
#include "game/Level.h"
#include "game/World.h"

baba::GameData data;

baba::World* world = nullptr;

int levelIndex = 106;

//TODO: on index 15 check layer ordering

ui::ViewManager vm;

char buffer[256];

/*
void nextLevel()
{
  delete level;
  level = nullptr;

  while (!level)
  {
    ++levelIndex;
    level = loader.load(fmt::format("{}level", levelIndex), data);
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
    level = loader.load(fmt::format("{}level", levelIndex), data);
  }

  level->computeTiling();
  vm.gameView()->levelLoaded();
}
*/

#include "io/Assets.h"

#include "gfx/Gfx.h"

int main(int argc, char* argv[])
{  
  world = new baba::World();

  
  if (!vm.init())
    return -1;

  world->loadLevel("106level", true);
  vm.gameView()->levelLoaded();
  
  vm.loop();

  //loader.load("1level.l");

  //getchar();
  return 0;
}



