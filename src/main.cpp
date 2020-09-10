#include <cstdio>
#include <cstdlib>

#include "gfx/ViewManager.h"

#include "io/Loader.h"

#include "game/Types.h"
#include "game/Level.h"

baba::GameData data;
baba::Level* level;

int main(int argc, char* argv[])
{
  io::Loader loader(data);
  loader.loadGameData();
  level = loader.load("95level");
  level->computeTiling();
  
  ui::ViewManager ui;

  if (!ui.init())
    return -1;

  if (!ui.loadData())
  {
    printf("Error while loading and initializing data.\n");
    ui.deinit();
    return -1;
  }

  ui.loop();
  ui.deinit();

  //loader.load("1level.l");

  getchar();
  return 0;
}