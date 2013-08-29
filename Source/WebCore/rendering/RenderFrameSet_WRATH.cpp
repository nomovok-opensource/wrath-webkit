#include "config.h"
#include "RenderFrameSet.h"

#include "Document.h"
#include "EventHandler.h"
#include "EventNames.h"
#include "Frame.h"
#include "FrameView.h"
#include "GraphicsContext.h"
#include "HTMLFrameSetElement.h"
#include "HitTestRequest.h"
#include "HitTestResult.h"
#include "MouseEvent.h"
#include "PaintInfo.h"
#include "RenderFrame.h"
#include "RenderView.h"
#include "Settings.h"

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
#include "HierarchyOfHandlesOfWRATH.h"
#include "WRATHPaintHelpers.h"
namespace
{
    class RenderFrameSet_ReadyWRATHWidgets:
      public WebCore::PaintedWidgetsOfWRATH<RenderFrameSet_ReadyWRATHWidgets>
    {
    public:
        WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_skip_node;

	WebCore::HierarchyOfHandlesOfWRATH<WebCore::RenderObject> m_children;
	WebCore::HierarchyOfHandlesOfWRATH<WebCore::RenderObject> m_column_borders;
	WebCore::HierarchyOfHandlesOfWRATH<WebCore::RenderObject> m_row_borders;
    };

    class RenderFrameSet_ReadyWRATHWidgetColumnBorder:
      public WebCore::PaintedWidgetsOfWRATH<RenderFrameSet_ReadyWRATHWidgetColumnBorder>
    {
    public:
      WebCore::FilledIntRectOfWRATH m_fill, m_edge_stroke_begin, m_edge_stroke_end;
    };

    class RenderFrameSet_ReadyWRATHWidgetRowBorder:
      public WebCore::PaintedWidgetsOfWRATH<RenderFrameSet_ReadyWRATHWidgetRowBorder>
    {
    public:
      WebCore::FilledIntRectOfWRATH m_fill, m_edge_stroke_begin, m_edge_stroke_end;
    };

    
}

namespace WebCore
{

inline HTMLFrameSetElement* RenderFrameSet::frameSet() const
{
    return static_cast<HTMLFrameSetElement*>(node());
}

static Color borderStartEdgeColor()
{
    return Color(170, 170, 170);
}

static Color borderEndEdgeColor()
{
    return Color::black;
}

static Color borderFillColor()
{
    return Color(208, 208, 208);
}

void RenderFrameSet::readyWRATHWidgetColumnBorder(PaintedWidgetsOfWRATHHandle& handle,
						  const PaintInfoOfWRATH& paintInfo, const IntRect& borderRect)
{
    if (!paintInfo.rect.intersects(borderRect))
        return;
    
    RenderFrameSet_ReadyWRATHWidgetColumnBorder *d(RenderFrameSet_ReadyWRATHWidgetColumnBorder::object(this, handle));
    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    if (d->m_edge_stroke_begin.widget())
	d->m_edge_stroke_begin.widget()->visible(false);
    if (d->m_edge_stroke_end.widget())
	d->m_edge_stroke_end.widget()->visible(false);

    // FIXME: We should do something clever when borders from distinct framesets meet at a join.
    
    // Fill first.

    Color color = (frameSet()->hasBorderColor() ? style()->visitedDependentColor(CSSPropertyBorderLeftColor) : borderFillColor());
    d->m_fill.update(paintInfo.wrath_context, borderRect, color, CompositeSourceOver);
    
    // Now stroke the edges but only if we have enough room to paint both edges with a little
    // bit of the fill color showing through.
    if (borderRect.width() >= 3) {
        d->m_edge_stroke_begin.update(paintInfo.wrath_context, 
                                      IntRect(borderRect.location(), IntSize(1, height())),
                                      borderStartEdgeColor(), CompositeSourceOver);

	d->m_edge_stroke_end.update(paintInfo.wrath_context, 
                                    IntRect(IntPoint(borderRect.maxX() - 1, borderRect.y()), IntSize(1, height())),
                                    borderEndEdgeColor(), CompositeSourceOver);

	if(d->m_edge_stroke_begin.widget()) {
	  d->m_edge_stroke_begin.widget()->visible(true);
	}
	if(d->m_edge_stroke_end.widget()) {
	  d->m_edge_stroke_end.widget()->visible(true);
	}
    }
}

void RenderFrameSet::readyWRATHWidgetRowBorder(PaintedWidgetsOfWRATHHandle& handle,
					       const PaintInfoOfWRATH& paintInfo, const IntRect& borderRect)
{
    if (!paintInfo.rect.intersects(borderRect))
        return;

    RenderFrameSet_ReadyWRATHWidgetRowBorder *d(RenderFrameSet_ReadyWRATHWidgetRowBorder::object(this, handle));
    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    if (d->m_edge_stroke_begin.widget())
	d->m_edge_stroke_begin.widget()->visible(false);
    if (d->m_edge_stroke_end.widget())
	d->m_edge_stroke_end.widget()->visible(false);

    // FIXME: We should do something clever when borders from distinct framesets meet at a join.
    
    // Fill first.

    Color color = (frameSet()->hasBorderColor() ? style()->visitedDependentColor(CSSPropertyBorderLeftColor) : borderFillColor());
    d->m_fill.update(paintInfo.wrath_context, borderRect, color, CompositeSourceOver);

    // Now stroke the edges but only if we have enough room to paint both edges with a little
    // bit of the fill color showing through.
    if (borderRect.height() >= 3) {
        d->m_edge_stroke_begin.update(paintInfo.wrath_context, 
                                      IntRect(borderRect.location(), IntSize(width(), 1)),
                                      borderStartEdgeColor(), CompositeSourceOver);
        d->m_edge_stroke_end.update(paintInfo.wrath_context, 
                                    IntRect(IntPoint(borderRect.x(), borderRect.maxY() - 1), IntSize(width(), 1)),
                                    borderEndEdgeColor(), CompositeSourceOver);
	if(d->m_edge_stroke_begin.widget()) {
	  d->m_edge_stroke_begin.widget()->visible(true);
	}
	if(d->m_edge_stroke_end.widget()) {
	  d->m_edge_stroke_end.widget()->visible(true);
	}
    }
}

void RenderFrameSet::readyWRATHWidgets(PaintedWidgetsOfWRATHHandle& handle,
				       PaintInfoOfWRATH& paintInfo, int tx, int ty)
{
    RenderFrameSet_ReadyWRATHWidgets *d(RenderFrameSet_ReadyWRATHWidgets::object(this, handle));
    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    ContextOfWRATH::AutoPushNode autoPushSkip(paintInfo.wrath_context, d->m_skip_node);
    d->m_skip_node.widget()->visible(false);

    if (paintInfo.phase != PaintPhaseForeground)
        return;
    
    RenderObject* child = firstChild();
    if (!child)
        return;

    d->m_skip_node.widget()->visible(true);

    // Add in our offsets.
    tx += x();
    ty += y();

    int rows = frameSet()->totalRows();
    int cols = frameSet()->totalCols();
    int borderThickness = frameSet()->border();

    d->m_children.hideEachObject();
    d->m_column_borders.hideEachObject();
    d->m_row_borders.hideEachObject();

    int yPos = 0;
    for (int r = 0; r < rows; r++) {
        int xPos = 0;
        for (int c = 0; c < cols; c++) {
	    PaintedWidgetsOfWRATHHandle& ch(d->m_children.getHandle(child));
	    ch.visible(true);
            child->readyWRATHWidgets(ch, paintInfo, tx, ty);
            xPos += m_cols.m_sizes[c];
            if (borderThickness && m_cols.m_allowBorder[c + 1]) {
		PaintedWidgetsOfWRATHHandle& cbh(d->m_column_borders.getHandle(child));
		cbh.visible(true);
                readyWRATHWidgetColumnBorder(cbh, paintInfo, IntRect(tx + xPos, ty + yPos, borderThickness, height()));
                xPos += borderThickness;
            }
            child = child->nextSibling();
            if (!child) {
		d->m_children.removeNonVisibleHandles();
		d->m_column_borders.removeNonVisibleHandles();
		d->m_row_borders.removeNonVisibleHandles();
                return;
	    }
        }
        yPos += m_rows.m_sizes[r];
        if (borderThickness && m_rows.m_allowBorder[r + 1]) {
	    /* It's not exactly related to the child but it matches the separation */
	    /* TODO: Improve the explanation. */
	    PaintedWidgetsOfWRATHHandle& crh(d->m_row_borders.getHandle(child));
	    crh.visible(true);
	    readyWRATHWidgetRowBorder(crh, paintInfo, IntRect(tx, ty + yPos, width(), borderThickness));
            yPos += borderThickness;
        }
    }
    d->m_children.removeNonVisibleHandles();
    d->m_column_borders.removeNonVisibleHandles();
    d->m_row_borders.removeNonVisibleHandles();
}
}
#endif
