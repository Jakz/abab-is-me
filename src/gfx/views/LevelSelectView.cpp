#include "Views.h"

using namespace baba;
using namespace ui;

void LevelSelectView::render()
{
  _renderer->clear({0, 0, 0});
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
