#include <cstdio>
#include <cstdlib>

#include "gfx/view_manager.h"

#include "io/Loader.h"

int main(int argc, char* argv[])
{
  io::Loader loader;
  loader.loadGameData();
  
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