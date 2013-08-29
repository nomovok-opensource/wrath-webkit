#include "config.h"
#include "RenderReplaced.h"

#include "GraphicsContext.h"
#include "RenderBlock.h"
#include "RenderLayer.h"
#include "RenderTheme.h"
#include "RenderView.h"
#include "VisiblePosition.h"

#if USE(WRATH)

#include "PaintInfoOfWRATH.h"
#include "WRATHPaintHelpers.h"

namespace
{
    class RenderReplaced_ReadyWRATHWidgets:
      public WebCore::PaintedWidgetsOfWRATH<RenderReplaced_ReadyWRATHWidgets>
    {
    public:
        RenderReplaced_ReadyWRATHWidgets(void)
        : m_border_radius_clip_rect(0, 0, 0, 0)
        {
        }

	WebCore::PaintedWidgetsOfWRATHHandle m_box_decorations;
	WebCore::PaintedWidgetsOfWRATHHandle m_mask;
	WebCore::PaintedWidgetsOfWRATHHandle m_outline;
	WebCore::PaintedWidgetsOfWRATHHandle m_replaced;

        WebCore::ContextOfWRATH::CPlainFamily::DrawnShape::AutoDelete m_border_radius_clip;
        WebCore::ContextOfWRATH::DrawnCanvas::AutoDelete m_border_radius_clip_canvas;
        WRATHShapeF m_border_radius_clip_shape;
        WebCore::RoundedIntRect m_border_radius_clip_rect;
        WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_border_extra_clipping;

        WebCore::FilledIntRectOfWRATH m_selection_tint;
    };
}

namespace WebCore {

void RenderReplaced::readyWRATHWidgets(PaintedWidgetsOfWRATHHandle& handle, PaintInfoOfWRATH& paintInfo, int tx, int ty)
{
    RenderReplaced_ReadyWRATHWidgets *d(RenderReplaced_ReadyWRATHWidgets::object(this, handle));
    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    d->m_box_decorations.visible(false);
    d->m_mask.visible(false);
    d->m_outline.visible(false);
    d->m_replaced.visible(false);
    if (d->m_border_radius_clip_canvas.widget())
        d->m_border_radius_clip_canvas.widget()->visible(false);

    if(d->m_selection_tint.widget())
      d->m_selection_tint.widget()->visible(false);

    if (!shouldPaint(paintInfo, tx, ty)) {
        return;
    }
    
    tx += x();
    ty += y();
    
    if (hasBoxDecorations() && (paintInfo.phase == PaintPhaseForeground || paintInfo.phase == PaintPhaseSelection)) {
        d->m_box_decorations.visible(true);
        readyWRATHWidgetBoxDecorations(d->m_box_decorations, paintInfo, tx, ty);
    }
        
    if (paintInfo.phase == PaintPhaseMask) {
        d->m_mask.visible(true);
        readyWRATHWidgetMask(d->m_mask, paintInfo, tx, ty);
        return;
    }

    if ((paintInfo.phase == PaintPhaseOutline || paintInfo.phase == PaintPhaseSelfOutline) && style()->outlineWidth()) {
        d->m_outline.visible(true);
        readyWRATHWidgetOutline(d->m_outline, paintInfo.wrath_context, tx, ty, width(), height());
    }
    
    if (paintInfo.phase != PaintPhaseForeground && paintInfo.phase != PaintPhaseSelection) {
        return;
    }
    
    if (!paintInfo.shouldPaintWithinRoot(this)) {
        return;
    }
    
    bool drawSelectionTint = selectionState() != SelectionNone && !document()->printing();
    if (paintInfo.phase == PaintPhaseSelection) {
        if (selectionState() == SelectionNone) {
            return;
        }
        drawSelectionTint = false;
    }

    bool completelyClippedOut = false;
    if (style()->hasBorderRadius()) {
        IntRect borderRect = IntRect(tx, ty, width(), height());

        if (borderRect.isEmpty())
            completelyClippedOut = true;
        else {
            // Push a clip if we have a border radius, since we want to round the foreground content that gets painted.
            RoundedIntRect border(style()->getRoundedBorderFor(borderRect));

            if (d->m_border_radius_clip_rect.rect().size() != border.rect().size() ||
                !(d->m_border_radius_clip_rect.radii() == border.radii())) {
                
                d->m_border_radius_clip_shape.clear();
                RoundedFilledRectOfWRATH::AddToShape_TranslateToOrigin(true, d->m_border_radius_clip_shape, border);
                d->m_border_radius_clip.delete_widget();
                d->m_border_radius_clip_rect = border;
            }
            
            WRATH_PUSH_CANVAS_NODE(paintInfo.wrath_context, d->m_border_radius_clip_canvas);
            
            if(!d->m_border_radius_clip.widget())
              {
                paintInfo.wrath_context->canvas_clipping()
                  .clip_filled_shape(WRATHWidgetGenerator::clip_inside, d->m_border_radius_clip,
                                     WRATHWidgetGenerator::shape_value(d->m_border_radius_clip_shape));
              }

            d->m_border_radius_clip.widget()->position(vec2(border.rect().location().x(), 
                                                            border.rect().location().y()));
            d->m_border_radius_clip_canvas.widget()->visible(true);

            paintInfo.wrath_context->push_node(d->m_border_extra_clipping);
            ContextOfWRATH::set_clipping(d->m_border_extra_clipping, d->m_border_radius_clip_rect.rect());
        }
    }

    if (!completelyClippedOut) {
        d->m_replaced.visible(true);
        readyWRATHWidgetReplaced(d->m_replaced, paintInfo, tx, ty);

        if (style()->hasBorderRadius()) {
            paintInfo.wrath_context->pop_node();
            paintInfo.wrath_context->pop_node();
        }
    }
        
    // The selection tint never gets clipped by border-radius rounding, since we want it to run right up to the edges of
    // surrounding content.
    if (drawSelectionTint) {
        IntRect selectionPaintingRect = localSelectionRect();
        selectionPaintingRect.move(tx, ty);

        d->m_selection_tint.update(paintInfo.wrath_context, 
                                   selectionPaintingRect,
                                   selectionBackgroundColor(), CompositeSourceOver);
	if(d->m_selection_tint.widget()) {
	  d->m_selection_tint.widget()->visible(true);
	}
    }
}
}
#endif
