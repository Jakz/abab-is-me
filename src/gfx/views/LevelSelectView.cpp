#include "Views.h"

using namespace baba;
using namespace ui;

void LevelSelectView::render()
{
  auto r = gvm->renderer();

  SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
  SDL_RenderClear(r);
}

void LevelSelectView::handleKeyboardEvent(const events::KeyEvent& event)
{
  if (event.press)
  {
    switch (event.code)
    {
      case KeyCode::BindExit: gvm->exit(); break;
    }
  }
}

void LevelSelectView::handleMouseEvent(const events::MouseEvent &event)
{

}
