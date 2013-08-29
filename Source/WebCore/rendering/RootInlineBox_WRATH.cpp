#include "config.h"
#include "RootInlineBox.h"

#include "BidiResolver.h"
#include "Chrome.h"
#include "ChromeClient.h"
#include "Document.h"
#include "EllipsisBox.h"
#include "Frame.h"
#include "GraphicsContext.h"
#include "HitTestResult.h"
#include "InlineTextBox.h"
#include "Page.h"
#include "PaintInfo.h"
#include "RenderArena.h"
#include "RenderBlock.h"
#include "VerticalPositionCache.h"

using namespace std;

#if USE(WRATH)

#include "PaintInfoOfWRATH.h"

namespace {
  class RootInlineBox_WRATHWidgets:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::InlineBox, RootInlineBox_WRATHWidgets>
  {
  public:
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::InlineBox> m_inlineFlowBox;
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::InlineBox> m_ellipsis;
  };

}

namespace WebCore {

void RootInlineBox::readyWRATHWidgets(PaintedWidgetsOfWRATHHandleT<InlineBox> &handle, 
                                      PaintInfoOfWRATH &paintInfo, int tx, int ty, int lineTop, int lineBottom)
{
  RootInlineBox_WRATHWidgets *d;

  d=RootInlineBox_WRATHWidgets::object(this, handle);

  ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);  

  InlineFlowBox::readyWRATHWidgets(d->m_inlineFlowBox, paintInfo, tx, ty, lineTop, lineBottom);
  readyWRATHEllipsisBox(d->m_ellipsis, paintInfo, tx, ty, lineTop, lineBottom);
  
}

void RootInlineBox::readyWRATHEllipsisBox(PaintedWidgetsOfWRATHHandleT<InlineBox> &handle, 
                                          PaintInfoOfWRATH &paintInfo, int tx, int ty, int lineTop, int lineBottom) const
{
  /*
    readyWRATHEllipsisBox comes from paintEllipsisBox which 
    just calls ellipsisBox()->paint() if it should be drawn,
    it could (and likely should) be absorbed into readyWRATHWidgets
   */
  handle.visible(false);

  if (hasEllipsisBox() && paintInfo.shouldPaintWithinRoot(renderer()) && renderer()->style()->visibility() == VISIBLE
      && paintInfo.phase == PaintPhaseForeground) {
    handle.visible(true);
    ellipsisBox()->readyWRATHWidgets(handle, paintInfo, tx, ty, lineTop, lineBottom);
  }
          
}
}
#endif
