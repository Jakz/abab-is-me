#include "LuaBridge.h"

#include "lua/lua.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"

#include <iostream>

using namespace lua;

void LuaBridge::init(const path& dataFolder)
{
  lua_State* L = luaL_newstate();
  
  luaL_openlibs(L);

  path file = dataFolder + "/Data/clears.lua";
  
  if (luaL_loadfile(L, file.c_str()))
  {
    std::cerr << "Error: " << lua_tostring(L, -1) << std::endl;
  }

  lua_close(L);
}