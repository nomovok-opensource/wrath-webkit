#include "config.h"
#include "RenderTable.h"

#include "AutoTableLayout.h"
#include "CollapsedBorderValue.h"
#include "DeleteButtonController.h"
#include "Document.h"
#include "FixedTableLayout.h"
#include "FrameView.h"
#include "HitTestResult.h"
#include "HTMLNames.h"
#include "RenderLayer.h"
#include "RenderTableCell.h"
#include "RenderTableCol.h"
#include "RenderTableSection.h"
#include "RenderView.h"

using namespace std;

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
#include "HierarchyOfHandlesOfWRATH.h"
namespace
{
    class RenderTable_ReadyWRATHWidgets:
      public WebCore::PaintedWidgetsOfWRATH<RenderTable_ReadyWRATHWidgets>
    {
    public:
        WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_skip_node;
        WebCore::PaintedWidgetsOfWRATHHandle m_push_pop;
        WebCore::PaintedWidgetsOfWRATHHandle m_object;
    };

    class RenderTable_ReadyWRATHWidgetObject:
      public WebCore::PaintedWidgetsOfWRATH<RenderTable_ReadyWRATHWidgetObject>
    {
    public:
        WebCore::PaintedWidgetsOfWRATHHandle m_box_decorations;
        WebCore::PaintedWidgetsOfWRATHHandle m_mask;
        WebCore::HierarchyOfHandlesOfWRATH<WebCore::RenderObject> m_child_table_section_handles;
        WebCore::HierarchyOfHandlesOfWRATH<WebCore::RenderObject> m_child_table_cell_handles;
        WebCore::PaintedWidgetsOfWRATHHandle m_outline;
    };

    class RenderTable_ReadyWRATHWidgetBoxDecorations:
      public WebCore::PaintedWidgetsOfWRATH<RenderTable_ReadyWRATHWidgetBoxDecorations>
    {
    public:
        WebCore::PaintedWidgetsOfWRATHHandle m_box_shadow;
        WebCore::PaintedWidgetsOfWRATHHandle m_root_box_fill_layers;
        WebCore::PaintedWidgetsOfWRATHHandle m_fill_layers;
        WebCore::PaintedWidgetsOfWRATHHandle m_latter_box_shadow;
        WebCore::PaintedWidgetsOfWRATHHandle m_border;
    };
}
namespace WebCore {

void RenderTable::readyWRATHWidgetBoxDecorations(PaintedWidgetsOfWRATHHandle& handle,
                                                PaintInfoOfWRATH& paintInfo, int tx, int ty)
{
    RenderTable_ReadyWRATHWidgetBoxDecorations *d(RenderTable_ReadyWRATHWidgetBoxDecorations::object(this, handle));
    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    d->m_box_shadow.visible(false);
    d->m_root_box_fill_layers.visible(false);
    d->m_fill_layers.visible(false);
    d->m_latter_box_shadow.visible(false);
    d->m_border.visible(false);
    
    if (!paintInfo.shouldPaintWithinRoot(this))
        return;

    IntRect rect(tx, ty, width(), height());
    subtractCaptionRect(rect);

    readyWRATHWidgetBoxShadow(d->m_box_shadow, paintInfo.wrath_context, rect.x(), rect.y(), rect.width(), rect.height(), style(), Normal);
    d->m_box_shadow.visible(true);
    
    if (isRoot()) {
        readyWRATHWidgetRootBoxFillLayers(d->m_root_box_fill_layers, paintInfo);
    }
    else if (!isBody() || document()->documentElement()->renderer()->hasBackground()) {
        // The <body> only paints its background if the root element has defined a background
        // independent of the body.
        readyWRATHWidgetFillLayers(d->m_fill_layers, paintInfo, style()->visitedDependentColor(CSSPropertyBackgroundColor), style()->backgroundLayers(), rect.x(), rect.y(), rect.width(), rect.height());
        d->m_fill_layers.visible(true);
    }

    readyWRATHWidgetBoxShadow(d->m_latter_box_shadow, paintInfo.wrath_context, rect.x(), rect.y(), rect.width(), rect.height(), style(), Inset);
    d->m_latter_box_shadow.visible(true);

    if (style()->hasBorder() && !collapseBorders()) {
        readyWRATHWidgetBorder(d->m_border, paintInfo.wrath_context, rect.x(), rect.y(), rect.width(), rect.height(), style());
        d->m_border.visible(true);
    }
}

void RenderTable::readyWRATHWidgetMask(PaintedWidgetsOfWRATHHandle&,
                                       PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
  /* [WRATH-TODO] */
  WRATH_UNIMPLEMENTED(paintInfo.wrath_context);
  
}

void RenderTable::readyWRATHWidgets(PaintedWidgetsOfWRATHHandle& handle,
                                    PaintInfoOfWRATH& paintInfo, int tx, int ty)
{
    RenderTable_ReadyWRATHWidgets *d(RenderTable_ReadyWRATHWidgets::object(this, handle));

    tx += x();
    ty += y();

    PaintPhase paintPhase = paintInfo.phase;

    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);
    ContextOfWRATH::AutoPushNode autoPushSkip(paintInfo.wrath_context, d->m_skip_node);

    d->m_skip_node.widget()->visible(false);

    /* [WRATH-DANGER]: This if block basically avoids drawing if the
       drawn element wouldn't be visible anyway (based on the dirty
       region in paintinfo). We want to walk the elements in that case
       too. */
    if (!isRoot()) {
        IntRect overflowBox = visualOverflowRect();
        flipForWritingMode(overflowBox);
        overflowBox.inflate(maximalOutlineSize(paintInfo.phase));
        overflowBox.move(tx, ty);
        if (!overflowBox.intersects(paintInfo.rect)) {
            return;
        }
    }

    d->m_skip_node.widget()->visible(true);

    bool pushedClip = pushContentsClipWRATH(d->m_push_pop, paintInfo, tx, ty);    
    readyWRATHWidgetObject(d->m_object, paintInfo, tx, ty);
    if (pushedClip)
        popContentsClipWRATH(d->m_push_pop, paintInfo, paintPhase, tx, ty);
}

void RenderTable::readyWRATHWidgetObject(PaintedWidgetsOfWRATHHandle& handle,
                                         PaintInfoOfWRATH& paintInfo, int tx, int ty)
{
    RenderTable_ReadyWRATHWidgetObject *d(RenderTable_ReadyWRATHWidgetObject::object(this, handle));

    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    
    d->m_box_decorations.visible(false);
    d->m_mask.visible(false);
    d->m_child_table_section_handles.hideEachObject();
    d->m_child_table_cell_handles.hideEachObject();
    d->m_outline.visible(false);

    PaintPhase paintPhase = paintInfo.phase;
    if ((paintPhase == PaintPhaseBlockBackground || paintPhase == PaintPhaseChildBlockBackground) && hasBoxDecorations() && style()->visibility() == VISIBLE) {
        d->m_box_decorations.visible(true);
        readyWRATHWidgetBoxDecorations(d->m_box_decorations, paintInfo, tx, ty);
    }

    if (paintPhase == PaintPhaseMask) {
        d->m_mask.visible(true);
        readyWRATHWidgetMask(d->m_mask, paintInfo, tx, ty);
        return;
    }

    // We're done.  We don't bother painting any children.
    if (paintPhase == PaintPhaseBlockBackground)
        return;
    
    // We don't paint our own background, but we do let the kids paint their backgrounds.
    if (paintPhase == PaintPhaseChildBlockBackgrounds)
        paintPhase = PaintPhaseChildBlockBackground;

    PaintInfoOfWRATH info(paintInfo);
    info.phase = paintPhase;
    info.updatePaintingRootForChildren(this);

    for (RenderObject* child = firstChild(); child; child = child->nextSibling()) {
        if (child->isBox() && !toRenderBox(child)->hasSelfPaintingLayer() && (child->isTableSection() || child == m_caption)) {
            IntPoint childPoint = flipForWritingMode(toRenderBox(child), IntPoint(tx, ty), ParentToChildFlippingAdjustment);
            PaintedWidgetsOfWRATHHandle &ch(d->m_child_table_section_handles.getHandle(child));
            ch.visible(true);
            child->readyWRATHWidgets(ch, info, childPoint.x(), childPoint.y());
            //std::cout << "\nIsBox TableChildType=" << typeid(*child).name();
        }
    }
    d->m_child_table_section_handles.removeNonVisibleHandles();

    
    if (collapseBorders() && paintPhase == PaintPhaseChildBlockBackground && style()->visibility() == VISIBLE) {
        // Collect all the unique border styles that we want to paint in a sorted list.  Once we
        // have all the styles sorted, we then do individual passes, painting each style of border
        // from lowest precedence to highest precedence.
        info.phase = PaintPhaseCollapsedTableBorders;
        RenderTableCell::CollapsedBorderStyles borderStyles;
        RenderObject* stop = nextInPreOrderAfterChildren();
        for (RenderObject* o = firstChild(); o && o != stop; o = o->nextInPreOrder()) {
            if (o->isTableCell())
                toRenderTableCell(o)->collectBorderStyles(borderStyles);
        }
        RenderTableCell::sortBorderStyles(borderStyles);
        size_t count = borderStyles.size();
        for (size_t i = 0; i < count; ++i) {
            m_currentBorder = &borderStyles[i];
            for (RenderObject* child = firstChild(); child; child = child->nextSibling())
                if (child->isTableSection()) {
                    IntPoint childPoint = flipForWritingMode(toRenderTableSection(child), IntPoint(tx, ty), ParentToChildFlippingAdjustment);
                    PaintedWidgetsOfWRATHHandle &ch(d->m_child_table_cell_handles.getHandle(child));
                    ch.visible(true);
                    child->readyWRATHWidgets(ch, info, childPoint.x(), childPoint.y());
                    //std::cout << "\nIsTableSection TableChildType=" << typeid(*child).name();
                }
        }
        m_currentBorder = 0;
    }
    d->m_child_table_cell_handles.removeNonVisibleHandles();

    // Paint outline.
    if ((paintPhase == PaintPhaseOutline || paintPhase == PaintPhaseSelfOutline) && hasOutline() && style()->visibility() == VISIBLE) {
        d->m_outline.visible(true);
        readyWRATHWidgetOutline(d->m_outline, paintInfo.wrath_context, tx, ty, width(), height());
    } 
}
}
#endif
