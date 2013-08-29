#include "config.h"
#include "RenderScrollbar.h"

#include "Frame.h"
#include "FrameView.h"
#include "RenderPart.h"
#include "RenderScrollbarPart.h"
#include "RenderScrollbarTheme.h"

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"

namespace {

  class RenderScrollbar_RenderPart:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::Scrollbar, RenderScrollbar_RenderPart>
  {
  public:
    WebCore::PaintedWidgetsOfWRATHHandle m_part;
  };

}

namespace WebCore {

void RenderScrollbar::readyWRATHWidgets(ContextOfWRATH *ctx,
                                        PaintedWidgetsOfWRATHHandleT<Widget> &hnd)
{
  Scrollbar::readyWRATHWidgets(ctx, hnd);
}

void RenderScrollbar::readyWRATHWidgetPart(ContextOfWRATH *ctx,
                                           PaintedWidgetsOfWRATHHandleT<Scrollbar> &hnd,
                                           ScrollbarPart partType, const IntRect &rect)
{
    RenderScrollbar_RenderPart *d;
    
    d=RenderScrollbar_RenderPart::object(this, hnd);
    ContextOfWRATH::AutoPushNode autoPushRoot(ctx, d->m_root_node);
    
    d->m_part.visible(false);

    RenderScrollbarPart* partRenderer = m_parts.get(partType);
    if (!partRenderer)
        return;
    
    d->m_part.visible(true);
    partRenderer->readyWRATHWidgetIntoRect(d->m_part, ctx, x(), y(), rect);
}
}
#endif
