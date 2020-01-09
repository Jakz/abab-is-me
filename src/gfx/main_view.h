#pragma once

#include "view_manager.h"

namespace ui
{
  class GameView : public View
  {
  private:
    ViewManager* manager;

    void render();

  public:
    GameView(ViewManager* manager);
    ~GameView();

    void handleKeyboardEvent(const SDL_Event& event);
    void handleMouseEvent(const SDL_Event& event);
  };
}
