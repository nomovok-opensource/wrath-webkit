#include "config.h"
#include "RenderInline.h"

#include "Chrome.h"
#include "FloatQuad.h"
#include "GraphicsContext.h"
#include "HitTestResult.h"
#include "InlineTextBox.h"
#include "Page.h"
#include "RenderArena.h"
#include "RenderBlock.h"
#include "RenderLayer.h"
#include "RenderTheme.h"
#include "RenderView.h"
#include "TransformState.h"
#include "VisiblePosition.h"

#if ENABLE(DASHBOARD_SUPPORT)
#include "Frame.h"
#endif

using namespace std;
#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
#include "ArrayOfHandlesOfWRATH.h"
namespace {

  class RenderInline_WRATHWidgetOutlineForLine:
    public WebCore::PaintedWidgetsOfWRATH<RenderInline_WRATHWidgetOutlineForLine>
  {
  public:
    vecN<WebCore::PaintedWidgetsOfWRATHHandle, 6> m_sides;
    
  };

  class RenderInline_WRATHWidgetOutline:
    public WebCore::PaintedWidgetsOfWRATH<RenderInline_WRATHWidgetOutline>
  {
  public:
    
    WebCore::PaintedWidgetsOfWRATHHandle m_outline;
    WebCore::ArrayOfHandlesOfWRATH<WebCore::RenderObject> m_lines;
    
  };
}

namespace WebCore {
void RenderInline::readyWRATHWidgetOutlineForLine(PaintedWidgetsOfWRATHHandle &handle, 
                                                  ContextOfWRATH *wrath_context,
						  int tx, int ty, 
						  const IntRect& lastline, const IntRect& thisline, 
						  const IntRect& nextline)
{
    RenderInline_WRATHWidgetOutlineForLine *d;

    d=RenderInline_WRATHWidgetOutlineForLine::object(this, handle);
    WebCore::ContextOfWRATH::AutoPushNode autoPushRoot(wrath_context, d->m_root_node);

    RenderStyle* styleToUse = style();
    int ow = styleToUse->outlineWidth();
    EBorderStyle os = styleToUse->outlineStyle();
    Color oc = styleToUse->visitedDependentColor(CSSPropertyOutlineColor);

    //const AffineTransform& currentCTM = graphicsContext->getCTM();
    bool antialias = false; //!currentCTM.isIdentityOrTranslationOrFlipped();

    int offset = style()->outlineOffset();

    int t = ty + thisline.y() - offset;
    int l = tx + thisline.x() - offset;
    int b = ty + thisline.maxY() + offset;
    int r = tx + thisline.maxX() + offset;
    
    for(int i=0, endi=d->m_sides.size(); i<endi; ++i) {
      d->m_sides[i].visible(false);
    }

    // left edge
    d->m_sides[0].visible(true);
    readyWRATHWidgetDrawLineForBoxSide(d->m_sides[0], wrath_context,
                                       l - ow,
                                       t - (lastline.isEmpty() || thisline.x() < lastline.x() || (lastline.maxX() - 1) <= thisline.x() ? ow : 0),
                                       l,
                                       b + (nextline.isEmpty() || thisline.x() <= nextline.x() || (nextline.maxX() - 1) <= thisline.x() ? ow : 0),
                                       BSLeft,
                                       oc, os,
                                       (lastline.isEmpty() || thisline.x() < lastline.x() || (lastline.maxX() - 1) <= thisline.x() ? ow : -ow),
                                       (nextline.isEmpty() || thisline.x() <= nextline.x() || (nextline.maxX() - 1) <= thisline.x() ? ow : -ow),
                                       antialias);
    
    // right edge
    d->m_sides[1].visible(true);
    readyWRATHWidgetDrawLineForBoxSide(d->m_sides[1], wrath_context,
                                       r,
                                       t - (lastline.isEmpty() || lastline.maxX() < thisline.maxX() || (thisline.maxX() - 1) <= lastline.x() ? ow : 0),
                                       r + ow,
                                       b + (nextline.isEmpty() || nextline.maxX() <= thisline.maxX() || (thisline.maxX() - 1) <= nextline.x() ? ow : 0),
                                       BSRight,
                                       oc, os,
                                       (lastline.isEmpty() || lastline.maxX() < thisline.maxX() || (thisline.maxX() - 1) <= lastline.x() ? ow : -ow),
                                       (nextline.isEmpty() || nextline.maxX() <= thisline.maxX() || (thisline.maxX() - 1) <= nextline.x() ? ow : -ow),
                                       antialias);

    // upper edge
    if (thisline.x() < lastline.x()) {
      d->m_sides[2].visible(true);
      readyWRATHWidgetDrawLineForBoxSide(d->m_sides[2], wrath_context,
                                         l - ow,
                                         t - ow,
                                         min(r+ow, (lastline.isEmpty() ? 1000000 : tx + lastline.x())),
                                         t ,
                                         BSTop, oc, os,
                                         ow,
                                         (!lastline.isEmpty() && tx + lastline.x() + 1 < r + ow) ? -ow : ow,
                                         antialias);
    }
    
    if (lastline.maxX() < thisline.maxX()){
      d->m_sides[3].visible(true);
      readyWRATHWidgetDrawLineForBoxSide(d->m_sides[3], wrath_context,
                                         max(lastline.isEmpty() ? -1000000 : tx + lastline.maxX(), l - ow),
                                         t - ow,
                                         r + ow,
                                         t ,
                                         BSTop, oc, os,
                                         (!lastline.isEmpty() && l - ow < tx + lastline.maxX()) ? -ow : ow,
                                         ow, antialias);
    }
    
    // lower edge
    if (thisline.x() < nextline.x()) {
      d->m_sides[4].visible(true);
      readyWRATHWidgetDrawLineForBoxSide(d->m_sides[4], wrath_context,
                                         l - ow,
                                         b,
                                         min(r + ow, !nextline.isEmpty() ? tx + nextline.x() + 1 : 1000000),
                                         b + ow,
                                         BSBottom, oc, os,
                                         ow,
                                         (!nextline.isEmpty() && tx + nextline.x() + 1 < r + ow) ? -ow : ow,
                                         antialias);
    }
    
    if (nextline.maxX() < thisline.maxX()) {
      d->m_sides[5].visible(true);
      readyWRATHWidgetDrawLineForBoxSide(d->m_sides[5], wrath_context,
                                         max(!nextline.isEmpty() ? tx + nextline.maxX() : -1000000, l - ow),
                                         b,
                                         r + ow,
                                         b + ow,
                                         BSBottom, oc, os,
                                         (!nextline.isEmpty() && l - ow < tx + nextline.maxX()) ? -ow : ow,
                                         ow, antialias);
    }


}
 
void RenderInline::readyWRATHWidgetOutline(PaintedWidgetsOfWRATHHandle &handle, 
                                           ContextOfWRATH *wrath_context,
					   int tx, int ty)
{
    RenderInline_WRATHWidgetOutline *d;

    d=RenderInline_WRATHWidgetOutline::object(this, handle);
    WebCore::ContextOfWRATH::AutoPushNode autoPushRoot(wrath_context, d->m_root_node);

    d->m_outline.visible(false);
    d->m_lines.visible_each(false);

  /*
    TODO: from paintOutline
   */
    if (!hasOutline())
        return;
    
    RenderStyle* styleToUse = style();
    if (styleToUse->outlineStyleIsAuto() || hasOutlineAnnotation()) {
        if (!theme()->supportsFocusRing(styleToUse)) {
            // Only paint the focus ring by hand if the theme isn't able to draw the focus ring.
            d->m_outline.visible(true);
            readyWRATHWidgetFocusRing(d->m_outline, wrath_context, tx, ty, styleToUse);
        }
    }

    if (styleToUse->outlineStyleIsAuto() || styleToUse->outlineStyle() == BNONE)
        return;

    Vector<IntRect> rects;

    rects.append(IntRect());
    for (InlineFlowBox* curr = firstLineBox(); curr; curr = curr->nextLineBox()) {
        RootInlineBox* root = curr->root();
        int top = max(root->lineTop(), curr->logicalTop());
        int bottom = min(root->lineBottom(), curr->logicalBottom());
        rects.append(IntRect(curr->x(), top, curr->logicalWidth(), bottom - top));
    }
    rects.append(IntRect());

    unsigned int needed_size(rects.size() - 1);

    if(needed_size>d->m_lines.size()) {
      d->m_lines.resize(needed_size);
    }

    for (unsigned i = 1; i < rects.size() - 1; i++) {
      d->m_lines[i-1].visible(true);
      readyWRATHWidgetOutlineForLine(d->m_lines[i-1], wrath_context, tx, ty, rects.at(i - 1), rects.at(i), rects.at(i + 1));
    }
}

void RenderInline::readyWRATHWidgets(PaintedWidgetsOfWRATHHandle &handle,
                                     PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
  m_lineBoxes.readyWRATHWidgets(handle, this, paintInfo, tx, ty); 
}
}

#endif
