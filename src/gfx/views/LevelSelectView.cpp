#include "Views.h"

using namespace baba;
using namespace ui;

void LevelSelectView::render()
{
  _renderer->clear(color_t(0, 0, 0));
}

void LevelSelectView::handleKeyboardEvent(const events::KeyEvent& event)
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

void LevelSelectView::handleMouseEvent(const events::MouseEvent &event)
{

}
