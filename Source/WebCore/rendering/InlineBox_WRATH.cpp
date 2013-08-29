#include "config.h"
#include "InlineBox.h"

#include "HitTestResult.h"
#include "InlineFlowBox.h"
#include "PaintInfo.h"
#include "RenderArena.h"
#include "RenderBlock.h"
#include "RootInlineBox.h"

using namespace std;

#if USE(WRATH)
#include "WRATHTextureFontFreeType_Mix.hpp"
#include "WRATHTextDataStream.hpp"
#include "WRATHFontConfig.hpp"
#include "PaintInfoOfWRATH.h"
namespace 
{
  

  class InlineBox_WRATHWidgets:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::InlineBox, InlineBox_WRATHWidgets>
  {
  public:
    vecN<WebCore::PaintedWidgetsOfWRATHHandle, WebCore::NumberPaintPhases> m_phases;
  };
}

namespace WebCore
{

void InlineBox::readyWRATHWidgetsDefault(PaintedWidgetsOfWRATHHandleT<InlineBox> &handle, 
                                         PaintInfoOfWRATH &paintInfo, int tx, int ty, int lineTop, int lineBottom)
{
  
}


void InlineBox::readyWRATHWidgets(PaintedWidgetsOfWRATHHandleT<InlineBox> &handle, 
                                  PaintInfoOfWRATH &paintInfo, int tx, int ty, int /*lineTop*/, int /*lineBottom*/)
{

  if (!paintInfo.shouldPaintWithinRoot(renderer()) || (paintInfo.phase != PaintPhaseForeground && paintInfo.phase != PaintPhaseSelection))
        return;

  InlineBox_WRATHWidgets *d;
  
  d=InlineBox_WRATHWidgets::object(this, handle);
  WebCore::ContextOfWRATH::AutoPushNode autoPushVisible(paintInfo.wrath_context, d->m_root_node);

    IntPoint childPoint = IntPoint(tx, ty);
    if (parent()->renderer()->style()->isFlippedBlocksWritingMode()) // Faster than calling containingBlock().
        childPoint = renderer()->containingBlock()->flipForWritingMode(toRenderBox(renderer()), childPoint, RenderBox::ParentToChildFlippingAdjustment);
    
    // Paint all phases of replaced elements atomically, as though the replaced element established its
    // own stacking context.  (See Appendix E.2, section 6.4 on inline block/table elements in the CSS2.1
    // specification.)
    bool preservePhase = paintInfo.phase == PaintPhaseSelection || paintInfo.phase == PaintPhaseTextClip;
    PaintInfoOfWRATH info(paintInfo);
    
    info.phase = preservePhase ? paintInfo.phase : PaintPhaseBlockBackground;
    renderer()->readyWRATHWidgets(d->m_phases[info.phase], info, childPoint.x(), childPoint.y());
    if (!preservePhase) {

        info.phase = PaintPhaseChildBlockBackgrounds;
        renderer()->readyWRATHWidgets(d->m_phases[info.phase], info, childPoint.x(), childPoint.y());

        info.phase = PaintPhaseFloat;
        renderer()->readyWRATHWidgets(d->m_phases[info.phase], info, childPoint.x(), childPoint.y());

        info.phase = PaintPhaseForeground;
        renderer()->readyWRATHWidgets(d->m_phases[info.phase], info, childPoint.x(), childPoint.y());

        info.phase = PaintPhaseOutline;
        renderer()->readyWRATHWidgets(d->m_phases[info.phase], info, childPoint.x(), childPoint.y());
    }
}

}
#endif
