#include "config.h"
#include "RenderScrollbarPart.h"

#include "PaintInfo.h"
#include "RenderScrollbar.h"
#include "RenderScrollbarTheme.h"
#include "RenderView.h"

using namespace std;

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
#include "HierarchyOfHandlesOfWRATH.h"
#include "HierarchyOfHandlesOfWRATH_ChildList.h"
namespace
{
  class RenderScrollbarPart_WRATHWidgetsIntoRect:
      public WebCore::PaintedWidgetsOfWRATH<RenderScrollbarPart_WRATHWidgetsIntoRect>
  {
  public:
    vecN<WebCore::PaintedWidgetsOfWRATHHandle, WebCore::NumberPaintPhases> m_per_pass;
  };
}

namespace WebCore {
void RenderScrollbarPart::readyWRATHWidgetIntoRect(PaintedWidgetsOfWRATHHandle &handle,
                                                   ContextOfWRATH *ctx, int tx, int ty, const IntRect& rect)
{
  RenderScrollbarPart_WRATHWidgetsIntoRect *d;

  d=RenderScrollbarPart_WRATHWidgetsIntoRect::object(this, handle);
  ContextOfWRATH::AutoPushNode autoPushRoot(ctx, d->m_root_node);

  // Make sure our dimensions match the rect.
  setLocation(rect.x() - tx, rect.y() - ty);
  setWidth(rect.width());
  setHeight(rect.height());

  // Now do the paint.
  PaintInfoOfWRATH paintInfo(ctx, rect, PaintPhaseBlockBackground, false, 0, 0);
  readyWRATHWidgets(d->m_per_pass[paintInfo.phase], paintInfo, tx, ty);

  paintInfo.phase = PaintPhaseChildBlockBackgrounds;
  readyWRATHWidgets(d->m_per_pass[paintInfo.phase], paintInfo, tx, ty);

  paintInfo.phase = PaintPhaseFloat;
  readyWRATHWidgets(d->m_per_pass[paintInfo.phase], paintInfo, tx, ty);

  paintInfo.phase = PaintPhaseForeground;
  readyWRATHWidgets(d->m_per_pass[paintInfo.phase], paintInfo, tx, ty);

  paintInfo.phase = PaintPhaseOutline;
  readyWRATHWidgets(d->m_per_pass[paintInfo.phase], paintInfo, tx, ty);
}
}
#endif
