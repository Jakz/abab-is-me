#include "MainView.h"

#include "game/Types.h"
#include "game/Level.h"

using namespace ui;


extern baba::Level* level;

std::unordered_map<std::string, SDL_Texture*> cache;
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
  }

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  for (int y = 0; y < WIDTH/24; ++y)
    for (int x = 0; x < HEIGHT/24; ++x)
    {
      auto* tile = level->get(x, y);
      
      if (tile)
      {
        auto* obj = tile->object();

        if (obj)
        {
          const auto& gfx = objectGfx(obj->spec);

          SDL_Color color;
          assert(palette->format->BytesPerPixel == 3);
          Uint8 *p = (Uint8 *)palette->pixels + obj->spec->color.y * palette->pitch + obj->spec->color.x * palette->format->BytesPerPixel;
          SDL_GetRGB(p[0] | p[1] << 8 | p[2] << 16, palette->format, &color.r, &color.g, &color.b);
          SDL_SetTextureColorMod(gfx.texture, color.r, color.g, color.b);
         
          SDL_Rect src = gfx.sprites[obj->variant];
          SDL_Rect dest = { x * 24, y * 24, 24, 24 };

          src.x += tick * 24;
          
          SDL_RenderCopy(renderer, gfx.texture, &src, &dest);
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
    case SDLK_LEFT: break;
    case SDLK_RIGHT: break;
    case SDLK_UP: break;
    case SDLK_DOWN: break;
    }
  }
}

void GameView::handleMouseEvent(const SDL_Event& event)
{

}


GameView::~GameView()
{

}
