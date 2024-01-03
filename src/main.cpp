#include <cstdio>
#include <cstdlib>

#include "gfx/ViewManager.h"
#include "gfx/views/Views.h"

#include "io/Loader.h"

#include "game/Types.h"
#include "game/Level.h"

baba::GameData data;

std::vector<std::string> levelStack;
baba::Level* level = nullptr;
int levelIndex = 106;

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

void enterLevel(std::string filename)
{  
  levelStack.push_back(level->info().filename);

  delete level;
  level = nullptr;
  
  level = loader.load(filename, data);

  level->computeTiling();
  vm.gameView()->levelLoaded();
}

void exitLevel()
{
  if (!levelStack.empty())
  {
    auto filename = levelStack.back();
    levelStack.pop_back();

    delete level;
    level = nullptr;

    level = loader.load(filename, data);

    level->computeTiling();
    vm.gameView()->levelLoaded();
  }
}

#include "io/Assets.h"

#include "gfx/Gfx.h"

int main(int argc, char* argv[])
{
  //AssetLoader aloader;
  //aloader.decode(R"(E:\Games\Steam\SteamApps\common\Baba Is You\Assets.dat)", "assets");
  
  gfx::Gfx::i.cache()->init(R"(E:\Games\Steam\SteamApps\common\Baba Is You)");
  
  loader.setDataFolder(R"(E:\Games\Steam\SteamApps\common\Baba Is You\Data\)");

  data = loader.loadGameData();
  sprintf(buffer, "%dlevel", levelIndex);
  level = loader.load(buffer, data);
  level->computeTiling();
  
  if (!vm.init())
    return -1;



  gfx::Gfx::i.renderer = vm.renderer();

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



