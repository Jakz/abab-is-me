#include <cstdio>
#include <cstdlib>

#include "gfx/ViewManager.h"
#include "gfx/MainView.h"

#include "io/Loader.h"

#include "game/Types.h"
#include "game/Level.h"

baba::GameData data;
baba::Level* level = nullptr;
int levelIndex = 5;

//TODO: on index 15 check layer ordering

io::Loader loader;
ui::ViewManager vm;


char buffer[256];

void nextLevel()
{
  delete level;
  level = nullptr;

  while (!level)
  {
    ++levelIndex;
    sprintf(buffer, "%dlevel", levelIndex);
    level = loader.load(buffer, data);
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
    sprintf(buffer, "%dlevel", levelIndex);
    level = loader.load(buffer, data);
  }

  level->computeTiling();
  vm.gameView()->levelLoaded();
}

#include "io/Assets.h"

int main(int argc, char* argv[])
{
  AssetsLoader aloader;
  aloader.decode(R"(E:\Games\Steam\SteamApps\common\Baba Is You\Assets.dat)", "assets");
  
  data = loader.loadGameData();
  sprintf(buffer, "%dlevel", levelIndex);
  level = loader.load(buffer, data);
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

  //getchar();
  return 0;
}



