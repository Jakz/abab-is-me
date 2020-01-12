#pragma once

#include "ViewManager.h"
#include "Common.h"

struct ObjectGfx;

namespace ui
{
  enum class Scaler
  {
    SCALE_TO_FIT,
    KEEP_AT_MOST_NATIVE
  };

  
  class GameView : public View
  {
  private:
    ViewManager* manager;

    const ObjectGfx& objectGfx(const baba::ObjectSpec* spec);

    void render();

    Scaler scaler;
    point_t offset;
    size2d_t size;
    coord_t tileSize;

  public:
    GameView(ViewManager* manager);
    ~GameView();

    void handleKeyboardEvent(const SDL_Event& event);
    void handleMouseEvent(const SDL_Event& event);
  };
}
