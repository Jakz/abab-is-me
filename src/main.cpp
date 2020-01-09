#include <cstdio>
#include <cstdlib>

#include "io/Loader.h"

int main(int argc, char* argv[])
{
  io::Loader loader;
  loader.loadGameData();
  //loader.load("1level.l");

  
  getchar();
  return 0;
}