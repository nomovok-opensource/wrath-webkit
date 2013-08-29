#include "config.h"
#include "RenderTableRow.h"

#include "CachedImage.h"
#include "Document.h"
#include "HTMLNames.h"
#include "PaintInfo.h"
#include "RenderTableCell.h"
#include "RenderView.h"

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
#include "HierarchyOfHandlesOfWRATH.h"
namespace
{
    class RenderTableRow_ReadyWRATHWidgets:
      public WebCore::PaintedWidgetsOfWRATH<RenderTableRow_ReadyWRATHWidgets>
    {
    public:
	WebCore::HierarchyOfHandlesOfWRATH<WebCore::RenderObject> m_cell_backgrounds;
	WebCore::HierarchyOfHandlesOfWRATH<WebCore::RenderObject> m_box;
    };
}

namespace WebCore {

void RenderTableRow::readyWRATHWidgets(PaintedWidgetsOfWRATHHandle& handle,
				       PaintInfoOfWRATH& paintInfo, int tx, int ty)
{
   

    RenderTableRow_ReadyWRATHWidgets *d(RenderTableRow_ReadyWRATHWidgets::object(this, handle));
    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    d->m_cell_backgrounds.hideEachObject();
    d->m_box.hideEachObject();
    
    ASSERT(hasSelfPaintingLayer());
    if (!layer())
      return;


    for (RenderObject* child = firstChild(); child; child = child->nextSibling()) {
        if (child->isTableCell()) {
            // Paint the row background behind the cell.
            if (paintInfo.phase == PaintPhaseBlockBackground || paintInfo.phase == PaintPhaseChildBlockBackground) {
                RenderTableCell* cell = toRenderTableCell(child);
		PaintedWidgetsOfWRATHHandle &ch(d->m_cell_backgrounds.getHandle(cell));
		ch.visible(true);
                cell->readyWRATHWidgetBackgroundsBehindCell(ch, paintInfo, tx, ty, this);
            }
            if (!toRenderBox(child)->hasSelfPaintingLayer()) {
		PaintedWidgetsOfWRATHHandle &ch(d->m_box.getHandle(child));
		ch.visible(true);
                child->readyWRATHWidgets(ch, paintInfo, tx, ty);
	    }
        }
    }
    d->m_cell_backgrounds.removeNonVisibleHandles();
    d->m_box.removeNonVisibleHandles();
}
}
#endif
