#include "config.h"
#include "RenderScrollbarTheme.h"
#include "RenderScrollbar.h"
#include <wtf/StdLibExtras.h>

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
#include "WRATHPaintHelpers.h"
#include "ScrollView.h"

namespace {

class RenderScrollbarTheme_WRATHCorner:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::ScrollView, RenderScrollbarTheme_WRATHCorner>
{
public:
  WebCore::FilledIntRectOfWRATH m_item;
};
  
}

namespace WebCore {

void RenderScrollbarTheme::readyWRATHScrollCorner(ScrollView *sc, PaintedWidgetsOfWRATHHandleT<ScrollView> &hnd,
                                                  ContextOfWRATH *context, const IntRect& cornerRect)
{
    // FIXME: Implement.
    RenderScrollbarTheme_WRATHCorner *d;
  
    d=RenderScrollbarTheme_WRATHCorner::object(sc, hnd);
    ContextOfWRATH::AutoPushNode autoPushRoot(context, d->m_root_node);
    
    d->m_item.update(context, cornerRect, WebCore::Color(Color::white),
                     CompositeCopy);
}

void RenderScrollbarTheme::readyWRATHScrollbarBackground(PaintedWidgetsOfWRATHHandleT<Scrollbar> &hnd,
                                                         ContextOfWRATH *ctx, Scrollbar *scrollbar)
{
  toRenderScrollbar(scrollbar)->readyWRATHWidgetPart(ctx, hnd, ScrollbarBGPart, scrollbar->frameRect());
}

void RenderScrollbarTheme::readyWRATHTrackBackground(PaintedWidgetsOfWRATHHandleT<Scrollbar> &hnd,
                                                     ContextOfWRATH *ctx, Scrollbar *scrollbar, const IntRect &rect)
{
  toRenderScrollbar(scrollbar)->readyWRATHWidgetPart(ctx, hnd, TrackBGPart, rect);
}

void RenderScrollbarTheme::readyWRATHTrackPiece(PaintedWidgetsOfWRATHHandleT<Scrollbar> &hnd,
                                                ContextOfWRATH *ctx, Scrollbar *scrollbar, const IntRect &rect, ScrollbarPart part)
{
  toRenderScrollbar(scrollbar)->readyWRATHWidgetPart(ctx, hnd, part, rect);
}

void RenderScrollbarTheme::readyWRATHButton(PaintedWidgetsOfWRATHHandleT<Scrollbar> &hnd,
                                            ContextOfWRATH *ctx, Scrollbar *scrollbar, const IntRect &rect, ScrollbarPart part)
{
  toRenderScrollbar(scrollbar)->readyWRATHWidgetPart(ctx, hnd, part, rect);
}

void RenderScrollbarTheme::readyWRATHThumb(PaintedWidgetsOfWRATHHandleT<Scrollbar> &hnd,
                                           ContextOfWRATH *ctx, Scrollbar *scrollbar, const IntRect &rect)
{
  toRenderScrollbar(scrollbar)->readyWRATHWidgetPart(ctx, hnd, ThumbPart, rect);
}
}
#endif
