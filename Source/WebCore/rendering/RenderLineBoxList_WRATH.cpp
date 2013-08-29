#include "config.h"
#include "RenderLineBoxList.h"

#include "HitTestResult.h"
#include "InlineTextBox.h"
#include "PaintInfo.h"
#include "RenderArena.h"
#include "RenderInline.h"
#include "RenderView.h"
#include "RootInlineBox.h"

using namespace std;

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
#include "HierarchyOfHandlesOfWRATH.h"
namespace {
  class RenderLineBoxList_WRATHWidgets:
    public WebCore::PaintedWidgetsOfWRATH<RenderLineBoxList_WRATHWidgets>
  {
  public:
   
    WebCore::HierarchyOfHandlesOfWRATH<WebCore::InlineFlowBox, WebCore::InlineBox> m_handles;
    WebCore::HierarchyOfHandlesOfWRATH<WebCore::RenderInline> m_outlineHandles;
  };
}

namespace WebCore {

void RenderLineBoxList::readyWRATHWidgets(PaintedWidgetsOfWRATHHandle &handle,
                                          RenderBoxModelObject *renderer,
                                          PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
    RenderLineBoxList_WRATHWidgets *d;
    d=RenderLineBoxList_WRATHWidgets::object(renderer, handle);

    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    /*
      this just.. sucks. WebKit builds a list of 
      RenderInline objects that it will draw outlines.
      It is not clear if and how that list changes,
      so we punt and make all them handle non-visible
      and only those that are found in the list
      are then made visible.
     */
    d->m_handles.hideEachObject();
    d->m_outlineHandles.hideEachObject();

  // Only paint during the foreground/selection phases.
    if (paintInfo.phase != PaintPhaseForeground && paintInfo.phase != PaintPhaseSelection && paintInfo.phase != PaintPhaseOutline 
        && paintInfo.phase != PaintPhaseSelfOutline && paintInfo.phase != PaintPhaseChildOutlines && paintInfo.phase != PaintPhaseTextClip
        && paintInfo.phase != PaintPhaseMask)
        return;

    ASSERT(renderer->isRenderBlock() || (renderer->isRenderInline() && renderer->hasLayer())); // The only way an inline could paint like this is if it has a layer.

    // If we have no lines then we have no work to do.
    if (!firstLineBox())
        return;

    RenderView* v = renderer->view();
    bool usePrintRect = !v->printRect().isEmpty();
    int outlineSize = renderer->maximalOutlineSize(paintInfo.phase);
    if (!anyLineIntersectsRect(renderer, paintInfo.rect, tx, ty, usePrintRect, outlineSize))
        return;

    PaintInfoOfWRATH info(paintInfo);
    ListHashSet<RenderInline*> outlineObjects;
    info.outlineObjects = &outlineObjects;

    

    for (InlineFlowBox* curr = firstLineBox(); curr; curr = curr->nextLineBox()) {
        if (usePrintRect) {

            RootInlineBox* root = curr->root();
            int topForPaginationCheck = curr->logicalTopVisualOverflow(root->lineTop());
            int bottomForPaginationCheck = curr->logicalLeftVisualOverflow();
            if (!curr->parent()) {
                // We're a root box.  Use lineTop and lineBottom as well here.
                topForPaginationCheck = min(topForPaginationCheck, root->lineTop());
                bottomForPaginationCheck = max(bottomForPaginationCheck, root->lineBottom());
            }
            if (bottomForPaginationCheck - topForPaginationCheck <= v->printRect().height()) {
                if (ty + bottomForPaginationCheck > v->printRect().maxY()) {
                    if (RootInlineBox* nextRootBox = curr->root()->nextRootBox())
                        bottomForPaginationCheck = min(bottomForPaginationCheck, min(nextRootBox->logicalTopVisualOverflow(), nextRootBox->lineTop()));
                }
                if (ty + bottomForPaginationCheck > v->printRect().maxY()) {
                    if (ty + topForPaginationCheck < v->truncatedAt())
                        v->setBestTruncatedAt(ty + topForPaginationCheck, renderer);
                    // If we were able to truncate, don't paint.
                    if (ty + topForPaginationCheck >= v->truncatedAt())
                        break;
                }
            }
        }

        if (lineIntersectsDirtyRect(renderer, curr, info, tx, ty)) {
            RootInlineBox* root = curr->root();
            PaintedWidgetsOfWRATHHandleT<InlineBox> &currHandle(d->m_handles.getHandle(curr));

            currHandle.visible(true);
            curr->readyWRATHWidgets(currHandle, info, tx, ty, root->lineTop(), root->lineBottom());
        }
    }

    if (info.phase == PaintPhaseOutline || info.phase == PaintPhaseSelfOutline || info.phase == PaintPhaseChildOutlines) {
        ListHashSet<RenderInline*>::iterator end = info.outlineObjects->end();
        for (ListHashSet<RenderInline*>::iterator it = info.outlineObjects->begin(); it != end; ++it) {
            RenderInline* flow = *it;
            PaintedWidgetsOfWRATHHandle &handle(d->m_outlineHandles.getHandle(flow));

            handle.visible(true);
            flow->readyWRATHWidgetOutline(handle, info.wrath_context, tx, ty);
        }
        info.outlineObjects->clear();
    }

    
    d->m_handles.removeNonVisibleHandles();
    d->m_outlineHandles.removeNonVisibleHandles();
}
}
#endif
