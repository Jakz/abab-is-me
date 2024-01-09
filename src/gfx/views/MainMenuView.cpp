#include "Views.h"

using namespace baba;
using namespace ui;

void MainMenuView::render()
{
  _renderer->clear(color_t(0, 0, 0));
}

void MainMenuView::handleKeyboardEvent(const events::KeyEvent& event)
{
  if (event.press)
  {
    switch (event.code)
    {
      case KeyCode::BindExit:
        break;
    }
  }
}
