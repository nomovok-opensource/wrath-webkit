#include "config.h"
#include "RenderWidget.h"

#include "AXObjectCache.h"
#include "AnimationController.h"
#include "GraphicsContext.h"
#include "HitTestResult.h"
#include "RenderCounter.h"
#include "RenderLayer.h"
#include "RenderView.h"
#include "RenderWidgetProtector.h"

#if USE(ACCELERATED_COMPOSITING)
#include "RenderLayerBacking.h"
#endif


using namespace std;

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
namespace
{
    class RenderWidget_ReadyWRATHWidgets:
      public WebCore::PaintedWidgetsOfWRATH<RenderWidget_ReadyWRATHWidgets>
    {
    public:

	void makeAllNonVisible()
	{
	    m_box_decorations.visible(false);
	    m_mask.visible(false);
	    m_frame.visible(false);
	}

        WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_clip_node;
        WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_widget_position;
        WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_widget_clip;

	WebCore::PaintedWidgetsOfWRATHHandle m_box_decorations;
	WebCore::PaintedWidgetsOfWRATHHandle m_mask;
	WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::Widget> m_frame;
    };
}

namespace WebCore {

void RenderWidget::readyWRATHWidgets(PaintedWidgetsOfWRATHHandle& handle,
				     PaintInfoOfWRATH& paintInfo, int tx, int ty)
{
    RenderWidget_ReadyWRATHWidgets *d(RenderWidget_ReadyWRATHWidgets::object(this, handle));

    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    d->makeAllNonVisible();

    if (!shouldPaint(paintInfo, tx, ty))
        return;

    tx += x();
    ty += y();

    if (hasBoxDecorations() && (paintInfo.phase == PaintPhaseForeground || paintInfo.phase == PaintPhaseSelection)) {
	readyWRATHWidgetBoxDecorations(d->m_box_decorations, paintInfo, tx, ty);
	d->m_box_decorations.visible(true);
    }

    if (paintInfo.phase == PaintPhaseMask) {
        readyWRATHWidgetMask(d->m_mask, paintInfo, tx, ty);
	d->m_mask.visible(true);
        return;
    }

    if (!m_frameView || paintInfo.phase != PaintPhaseForeground || style()->visibility() != VISIBLE)
        return;

#if PLATFORM(MAC)
    if (style()->highlight() != nullAtom && !paintInfo.context->paintingDisabled())
        paintCustomHighlight(tx - x(), ty - y(), style()->highlight(), true);
#endif

    if (style()->hasBorderRadius()) {
        IntRect borderRect = IntRect(tx, ty, width(), height());

        if (borderRect.isEmpty())
            return;

        // Push a clip if we have a border radius, since we want to round the foreground content that gets painted.
	/* [WRATH-TODO]: Push clip node with rounded clip instead of rect
        paintInfo.context->save();
        paintInfo.context->addRoundedRectClip(style()->getRoundedBorderFor(borderRect));
	*/
        paintInfo.wrath_context->push_node(d->m_clip_node);
        ContextOfWRATH::set_clipping(d->m_clip_node, borderRect);
    }

    if (m_widget) {
        // Tell the widget to paint now.  This is the only time the widget is allowed
        // to paint itself.  That way it will composite properly with z-indexed layers.
        if (m_substituteImage) {
	    /* [WRATH-TODO]: Draw the image
            paintInfo.context->drawImage(m_substituteImage.get(), style()->colorSpace(), m_widget->frameRect());
	    */
	}
        else {
            IntPoint widgetLocation = m_widget->frameRect().location();
            IntPoint paintLocation(tx + borderLeft() + paddingLeft(), ty + borderTop() + paddingTop());
            IntRect paintRect = paintInfo.rect;

            IntSize paintOffset = paintLocation - widgetLocation;
            // When painting widgets into compositing layers, tx and ty are relative to the enclosing compositing layer,
            // not the root. In this case, shift the CTM and adjust the paintRect to be root-relative to fix plug-in drawing.
            
            if (!paintOffset.isZero()) {
                paintRect.move(-paintOffset);
            }

	    ContextOfWRATH::AutoPushNode autoPushPos(paintInfo.wrath_context, d->m_widget_position);
	    d->m_widget_position.widget()->position(vec2(paintOffset.width(), paintOffset.height()));

            
            ContextOfWRATH::AutoPushNode autoPushClip(paintInfo.wrath_context, d->m_widget_clip);
            ContextOfWRATH::set_clipping(d->m_widget_clip, paintRect);

            d->m_frame.visible(true);
            m_widget->readyWRATHWidgets(paintInfo.wrath_context, d->m_frame);
        }

        if (m_widget->isFrameView()) {
            FrameView* frameView = static_cast<FrameView*>(m_widget.get());
            bool runOverlapTests = !frameView->useSlowRepaintsIfNotOverlapped() || frameView->hasCompositedContentIncludingDescendants();
            if (paintInfo.overlapTestRequests && runOverlapTests) {
                ASSERT(!paintInfo.overlapTestRequests->contains(this));
                paintInfo.overlapTestRequests->set(this, m_widget->frameRect());
            }
         }
    }

    if (style()->hasBorderRadius()) {
	/* [WRATH-TODO]: Pop clip node with rounded clip
        paintInfo.context->restore();
	*/
      paintInfo.wrath_context->pop_node();
    }

    // Paint a partially transparent wash over selected widgets.
    if (isSelected() && !document()->printing()) {
        // FIXME: selectionRect() is in absolute, not painting coordinates.
	/* [WRATH-TODO]: Draw the selection
        paintInfo.context->fillRect(selectionRect(), selectionBackgroundColor(), style()->colorSpace());
	*/
    }
}
}
#endif
