#include "config.h"
#include "ScrollbarTheme.h"
#include "ScrollView.h"

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
#include "WRATHPaintHelpers.h"

namespace {

class ScrollbarTheme_WRATHCorner:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::ScrollView, ScrollbarTheme_WRATHCorner>
{
public:
  WebCore::FilledIntRectOfWRATH m_item;
};
  
}


namespace WebCore {




void ScrollbarTheme::readyWRATHScrollCorner(ScrollView *sc, PaintedWidgetsOfWRATHHandleT<ScrollView> &hnd,
                                            ContextOfWRATH *context, const IntRect& cornerRect)
{
  ScrollbarTheme_WRATHCorner *d;
  
  d=ScrollbarTheme_WRATHCorner::object(sc, hnd);
  ContextOfWRATH::AutoPushNode autoPushRoot(context, d->m_root_node);

  d->m_item.update(context, cornerRect, WebCore::Color(Color::white),
                   CompositeCopy);
}

} //namespace WebCore
#endif
