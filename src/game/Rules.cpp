#include "Rules.h"

#include "game/Level.h"

#include <algorithm>

using namespace baba;

void Rules::clear()
{
  std::for_each(_state.begin(), _state.end(), [](decltype(_state)::value_type& pair) { pair.second.clear(); });
}

void Rules::generate(baba::Level* level)
{
  /* search trivial rules NOUN IS PROPERTY */

  for (coord_t y = 0; y < level->height(); ++y)
  {
    for (coord_t x = 0; x < level->width(); ++x)
    {
      Tile* tile = level->get(x, y);
      Tile* left = level->get(tile, D::LEFT);
      Tile* right = level->get(tile, D::RIGHT);
      Tile* up = level->get(tile, D::UP);
      Tile* down = level->get(tile, D::DOWN);

      if (tile->has(data->IS))
      {
        std::vector<const ObjectSpec*> horizontal;
        std::vector<const ObjectSpec*> vertical;

        if (left)
        {
          auto* noun = left->find([](const Object& obj) { return obj.spec->type == ObjectSpec::Type::Noun; });
          if (noun)
            horizontal.push_back(noun->spec);
        }

        horizontal.push_back(data->IS);

        if (right)
        {
          auto* property = right->find([](const Object& obj) { return obj.spec->type == ObjectSpec::Type::Property; });
          if (property)
            horizontal.push_back(property->spec);
        }



        if (up)
        {
          auto* noun = up->find([](const Object& obj) { return obj.spec->type == ObjectSpec::Type::Noun; });
          if (noun)
            vertical.push_back(noun->spec);
        }

        vertical.push_back(data->IS);

        if (down)
        {
          auto* property = down->find([](const Object& obj) { return obj.spec->type == ObjectSpec::Type::Property; });
          if (property)
            vertical.push_back(property->spec);
        }

        const decltype(horizontal)* rules[] = { &horizontal, &vertical };
        
        for (const auto* rule : rules)
        {
          if (rule->size() == 3)
          {
            std::string text = "";
            for (const auto& term : *rule)
              text += term->name.substr(5) + " ";
            LOGD("Found rule: %s", text.c_str());
          }
            
        }
      }

    }
  }

}