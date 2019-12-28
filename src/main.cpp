#include <cstdio>
#include <cstdlib>

#include "io/Loader.h"

int main(int argc, char* argv[])
{
  io::Loader loader;
  loader.load("1level.l");
  
  return 0;
}