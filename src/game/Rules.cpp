#include "Rules.h"

#include "game/Level.h"

#include <algorithm>

using namespace baba;

void Rules::clear()
{
  std::for_each(_state.begin(), _state.end(), [](decltype(_state)::value_type& pair) { pair.second.clear(); });
}


void Rules::generate(const baba::Level* level)
{
  /* search trivial rules NOUN IS PROPERTY */

  for (coord_t y = 0; y < level->height(); ++y)
  {
    for (coord_t x = 0; x < level->width(); ++x)
    {
      const Tile* tile = level->get(x, y);

      if (tile->has(data->IS))
      {

      }
    }
  }

}