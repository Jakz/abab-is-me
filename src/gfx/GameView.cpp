#include "MainView.h"

#include "game/Types.h"
#include "game/Level.h"
#include "game/Rules.h"

using namespace ui;
using namespace baba;

Rules rules;
extern GameData data;
extern Level* level;

SDL_Surface* palette = nullptr;

constexpr uint32_t FRAMES = 3;
struct ObjectGfx
{
  SDL_Texture* texture;
  std::vector<SDL_Rect> sprites;
};

std::unordered_map<const baba::ObjectSpec*, ObjectGfx> objectGfxs;

const ObjectGfx& GameView::objectGfx(const baba::ObjectSpec* spec)
{
  auto it = objectGfxs.find(spec);

  if (it != objectGfxs.end())
    return it->second;
  else
  {
    path base = DATA_FOLDER + R"(Sprites\)" + spec->sprite;
    uint32_t count = 1;

    if (spec->tiling == baba::ObjectSpec::Tiling::Ortho)
      count = 16;

    SDL_Surface* surface = SDL_CreateRGBSurface(0, 24 * 3 * count, 24, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

    ObjectGfx gfx;

    for (uint32_t i = 0; i < count; ++i)
    {
      for (uint32_t f = 0; f < FRAMES; ++f)
      {
        path path = base + "_" + std::to_string(i) + "_" + std::to_string(f+1) + ".png";
        SDL_Surface* tmp = IMG_Load(path.c_str());
        assert(tmp);

        SDL_Rect dest = { (i * 3 + f) * 24, 0, 24, 24 };
        SDL_BlitSurface(tmp, nullptr, surface, &dest);

        SDL_FreeSurface(tmp);

        if (f == 0)
          gfx.sprites.push_back(dest);
      }
    }

    gfx.texture = SDL_CreateTextureFromSurface(manager->renderer(), surface);
    SDL_FreeSurface(surface);

    auto rit = objectGfxs.emplace(std::make_pair(spec, gfx));
    return rit.first->second;
  }
}


GameView::GameView(ViewManager* manager) : manager(manager)
{
}

void GameView::render()
{
  auto* renderer = manager->renderer();

  auto tick = (SDL_GetTicks() / 150) % 3;

  if (!palette)
  {
    palette = IMG_Load((DATA_FOLDER + R"(Palettes\default.png)").c_str());

    rules.state(data.objectsByName["baba"]).properties.set(baba::ObjectProperty::YOU);
  }

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  for (int y = 0; y < HEIGHT/24; ++y)
    for (int x = 0; x < WIDTH/24; ++x)
    {
      auto* tile = level->get(x, y);
      
      if (tile)
      {
        for (const auto& obj : *tile)
        {
          const auto& gfx = objectGfx(obj.spec);

          SDL_Color color;
          assert(palette->format->BytesPerPixel == 3);
          Uint8 *p = (Uint8 *)palette->pixels + obj.spec->color.y * palette->pitch + obj.spec->color.x * palette->format->BytesPerPixel;
          SDL_GetRGB(p[0] | p[1] << 8 | p[2] << 16, palette->format, &color.r, &color.g, &color.b);
          SDL_SetTextureColorMod(gfx.texture, color.r, color.g, color.b);

          SDL_Rect src = gfx.sprites[obj.variant];
          SDL_Rect dest = { x * 24, y * 24, 24, 24 };

          src.x += tick * 24;

          SDL_RenderCopy(renderer, gfx.texture, &src, &dest);
        }
      }
    }
}


void movement(coord_t dx, coord_t dy)
{
  for (auto& tile : *level)
    for (auto& object : tile)
      object.alreadyMoved = false;

  
  for (coord_t y = 0; y < level->height(); ++y)
    for (coord_t x = 0; x < level->width(); ++x)
    {
      Tile* tile = level->get(x, y);
      Tile* dest = level->get(x + dx, y + dy);

      if (dest)
      {
        for (auto it = tile->begin(); it != tile->end(); /**/)
        {
          if (!it->alreadyMoved && rules.state(it->spec).properties.isSet(ObjectProperty::YOU))
          {
            Object object = *it;
            object.alreadyMoved = true;

            it = tile->objects.erase(it);

            dest->objects.push_back(object);
            std::sort(dest->begin(), dest->end(), [](const Object& o1, const Object& o2) { return o1.spec->layer < o2.spec->layer; });
          }
          else
            ++it;
        }
      }
    }
}


void GameView::handleKeyboardEvent(const SDL_Event& event)
{
  if (event.type == SDL_KEYDOWN)
  {
    switch (event.key.keysym.sym)
    {
    case SDLK_ESCAPE: manager->exit(); break;
    case SDLK_LEFT: movement(-1, 0);  break;
    case SDLK_RIGHT: movement(1, 0);  break;
    case SDLK_UP: movement(0, -1); break;
    case SDLK_DOWN: movement(0, 1); break;
    }
  }
}

void GameView::handleMouseEvent(const SDL_Event& event)
{

}


GameView::~GameView()
{

}
