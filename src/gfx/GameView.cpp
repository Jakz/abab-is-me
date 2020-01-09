#include "MainView.h"

#include "game/Types.h"

using namespace ui;


extern baba::Level* level;

std::unordered_map<std::string, SDL_Texture*> cache;


GameView::GameView(ViewManager* manager) : manager(manager)
{
}

void GameView::render()
{
  auto* renderer = manager->renderer();

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  for (int y = 0; y < 240/24; ++y)
    for (int x = 0; x < 320 / 24; ++x)
    {
      auto* tile = level->get(x+5, y+6);
      auto* obj = tile->object();

      if (obj)
      {
        path image = DATA_FOLDER + R"(Sprites\)" + obj->spec->sprite + "_0_1.png";
        SDL_Texture* texture = nullptr;
        auto tit = cache.find(image);

        if (tit == cache.end())
        {
          SDL_Surface* surface = IMG_Load(image.c_str());
          assert(surface);
          texture = SDL_CreateTextureFromSurface(renderer, surface);
          auto pit = cache.emplace(std::make_pair(image, texture));
          SDL_FreeSurface(surface);
        }
        else
          texture = tit->second;

        SDL_Rect dest = { x * 24, y * 24, 24, 24 };
        SDL_RenderCopy(renderer, texture, nullptr, &dest);
      }

    }
}


void GameView::handleKeyboardEvent(const SDL_Event& event)
{
  if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
    manager->exit();
}

void GameView::handleMouseEvent(const SDL_Event& event)
{

}


GameView::~GameView()
{

}
