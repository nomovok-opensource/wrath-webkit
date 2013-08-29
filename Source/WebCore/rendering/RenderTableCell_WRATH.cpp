#include "config.h"
#include "RenderTableCell.h"

#include "CollapsedBorderValue.h"
#include "FloatQuad.h"
#include "GraphicsContext.h"
#include "HTMLNames.h"
#include "HTMLTableCellElement.h"
#include "PaintInfo.h"
#include "RenderTableCol.h"
#include "RenderView.h"
#include "TransformState.h"

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
namespace
{
    class RenderTableCell_ReadyWRATHWidgets:
      public WebCore::PaintedWidgetsOfWRATH<RenderTableCell_ReadyWRATHWidgets>
    {
    public:

        WebCore::PaintedWidgetsOfWRATHHandle m_collapsed_border;
        WebCore::PaintedWidgetsOfWRATHHandle m_block;
    };

    class RenderTableCell_ReadyWRATHWidgetCollapsedBorder:
      public WebCore::PaintedWidgetsOfWRATH<RenderTableCell_ReadyWRATHWidgetCollapsedBorder>
    {
    public:
        WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_skip_node;

        WebCore::PaintedWidgetsOfWRATHHandle m_top;
        WebCore::PaintedWidgetsOfWRATHHandle m_bottom;
        WebCore::PaintedWidgetsOfWRATHHandle m_left;
        WebCore::PaintedWidgetsOfWRATHHandle m_right;
    };

    class RenderTableCell_ReadyWRATHWidgetBoxDecorations:
      public WebCore::PaintedWidgetsOfWRATH<RenderTableCell_ReadyWRATHWidgetBoxDecorations>
    {
    public:
        WebCore::PaintedWidgetsOfWRATHHandle m_box_shadow;
        WebCore::PaintedWidgetsOfWRATHHandle m_backgrounds;
        WebCore::PaintedWidgetsOfWRATHHandle m_latter_box_shadow;
        WebCore::PaintedWidgetsOfWRATHHandle m_border;
    };

    class RenderTableCell_ReadyWRATHWidgetBackgroundsBehindCell:
      public WebCore::PaintedWidgetsOfWRATH<RenderTableCell_ReadyWRATHWidgetBackgroundsBehindCell>
    {
    public:
        WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_clip_node;
        WebCore::PaintedWidgetsOfWRATHHandle m_fill_layers;
    };
}

namespace WebCore {
using namespace HTMLNames;

static EBorderStyle collapsedBorderStyle(EBorderStyle style)
{
    if (style == OUTSET)
        return GROOVE;
    if (style == INSET)
        return RIDGE;
    return style;
}

struct CollapsedBorder {
    CollapsedBorderValue borderValue;
    BoxSide side;
    bool shouldPaint;
    int x1;
    int y1;
    int x2;
    int y2;
    EBorderStyle style;
};

class CollapsedBorders {
public:
    CollapsedBorders()
        : m_count(0)
    {
    }
    
    void addBorder(const CollapsedBorderValue& borderValue, BoxSide borderSide, bool shouldPaint,
                   int x1, int y1, int x2, int y2, EBorderStyle borderStyle)
    {
        if (borderValue.exists() && shouldPaint) {
            m_borders[m_count].borderValue = borderValue;
            m_borders[m_count].side = borderSide;
            m_borders[m_count].shouldPaint = shouldPaint;
            m_borders[m_count].x1 = x1;
            m_borders[m_count].x2 = x2;
            m_borders[m_count].y1 = y1;
            m_borders[m_count].y2 = y2;
            m_borders[m_count].style = borderStyle;
            m_count++;
        }
    }

    CollapsedBorder* nextBorder()
    {
        for (int i = 0; i < m_count; i++) {
            if (m_borders[i].borderValue.exists() && m_borders[i].shouldPaint) {
                m_borders[i].shouldPaint = false;
                return &m_borders[i];
            }
        }
        
        return 0;
    }
    
    CollapsedBorder m_borders[4];
    int m_count;
};

static void addBorderStyle(RenderTableCell::CollapsedBorderStyles& borderStyles, CollapsedBorderValue borderValue)
{
    if (!borderValue.exists())
        return;
    size_t count = borderStyles.size();
    for (size_t i = 0; i < count; ++i)
        if (borderStyles[i] == borderValue)
            return;
    borderStyles.append(borderValue);
}

void RenderTableCell::readyWRATHWidgetBoxDecorations(PaintedWidgetsOfWRATHHandle& handle,
                                                     PaintInfoOfWRATH& paintInfo, int tx, int ty)
{
    RenderTableCell_ReadyWRATHWidgetBoxDecorations *d(RenderTableCell_ReadyWRATHWidgetBoxDecorations::object(this, handle));
    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    d->m_box_shadow.visible(false);
    d->m_backgrounds.visible(false);
    d->m_latter_box_shadow.visible(false);
    d->m_border.visible(false);

    if (!paintInfo.shouldPaintWithinRoot(this))
        return;

    RenderTable* tableElt = table();
    if (!tableElt->collapseBorders() && style()->emptyCells() == HIDE && !firstChild())
        return;

    int w = width();
    int h = height();
   
    readyWRATHWidgetBoxShadow(d->m_box_shadow, paintInfo.wrath_context, tx, ty, w, h, style(), Normal);
    d->m_box_shadow.visible(true);
    
    // Paint our cell background.
    readyWRATHWidgetBackgroundsBehindCell(d->m_backgrounds, paintInfo, tx, ty, this);
    d->m_backgrounds.visible(true);

    readyWRATHWidgetBoxShadow(d->m_latter_box_shadow, paintInfo.wrath_context, tx, ty, w, h, style(), Inset);
    d->m_latter_box_shadow.visible(true);

    if (!style()->hasBorder() || tableElt->collapseBorders())
        return;

    readyWRATHWidgetBorder(d->m_border, paintInfo.wrath_context, tx, ty, w, h, style());
    d->m_border.visible(true);
}

void RenderTableCell::readyWRATHWidgetMask(PaintedWidgetsOfWRATHHandle&,
                                           PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
  /* [WRATH-TODO] */
  WRATH_UNIMPLEMENTED(paintInfo.wrath_context);
}

void RenderTableCell::readyWRATHWidgetBackgroundsBehindCell(PaintedWidgetsOfWRATHHandle& handle,
                                                            PaintInfoOfWRATH& paintInfo, int tx, int ty,
                                                            RenderObject* backgroundObject)
{
    RenderTableCell_ReadyWRATHWidgetBackgroundsBehindCell *d(RenderTableCell_ReadyWRATHWidgetBackgroundsBehindCell::object(this, handle));
    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    d->m_fill_layers.visible(false);

    if (!paintInfo.shouldPaintWithinRoot(this))
        return;

    if (!backgroundObject)
        return;

    if (style()->visibility() != VISIBLE)
        return;

    RenderTable* tableElt = table();
    if (!tableElt->collapseBorders() && style()->emptyCells() == HIDE && !firstChild())
        return;

    if (backgroundObject != this) {
        tx += x();
        ty += y();
    }

    int w = width();
    int h = height();

    Color c = backgroundObject->style()->visitedDependentColor(CSSPropertyBackgroundColor);
    const FillLayer* bgLayer = backgroundObject->style()->backgroundLayers();

    if (bgLayer->hasImage() || c.isValid()) {
        // We have to clip here because the background would paint
        // on top of the borders otherwise.  This only matters for cells and rows.
        bool shouldClip = backgroundObject->hasLayer() && (backgroundObject == this || backgroundObject == parent()) && tableElt->collapseBorders();

        ContextOfWRATH::AutoPushNode autoPushClip(paintInfo.wrath_context, d->m_clip_node);
        d->m_clip_node.widget()->node()->clipping_active(false);

        if (shouldClip) {
            IntRect clipRect(tx + borderLeft(), ty + borderTop(),
                w - borderLeft() - borderRight(), h - borderTop() - borderBottom());
            ContextOfWRATH::set_clipping(d->m_clip_node, clipRect);
            d->m_clip_node.widget()->node()->clipping_active(true);
        }
        readyWRATHWidgetFillLayers(d->m_fill_layers, paintInfo, c, bgLayer, tx, ty, w, h, BackgroundBleedNone, CompositeSourceOver, backgroundObject);
        d->m_fill_layers.visible(true);
    }
}

void RenderTableCell::readyWRATHWidgets(PaintedWidgetsOfWRATHHandle& handle,
                                        PaintInfoOfWRATH& paintInfo, int tx, int ty)
{
    RenderTableCell_ReadyWRATHWidgets *d(RenderTableCell_ReadyWRATHWidgets::object(this, handle));

    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    d->m_collapsed_border.visible(false);
    d->m_block.visible(false);

    if (paintInfo.phase == PaintPhaseCollapsedTableBorders && style()->visibility() == VISIBLE) {
        if (!paintInfo.shouldPaintWithinRoot(this)) {
            return;
        }

        tx += x();
        ty += y();
        int os = 2 * maximalOutlineSize(paintInfo.phase);
        if (ty - table()->outerBorderTop() < paintInfo.rect.maxY() + os
            && ty + height() + table()->outerBorderBottom() > paintInfo.rect.y() - os) {
            d->m_collapsed_border.visible(true);
            readyWRATHWidgetCollapsedBorder(d->m_collapsed_border, paintInfo.wrath_context, tx, ty, width(), height());
        }
        return;
    } 
    

    d->m_block.visible(true);
    RenderBlock::readyWRATHWidgets(d->m_block, paintInfo, tx, ty);
    
}

void RenderTableCell::readyWRATHWidgetCollapsedBorder(PaintedWidgetsOfWRATHHandle& handle,
                                                      ContextOfWRATH *wrath_context,
                                                      int tx, int ty, int w, int h)
{
    RenderTableCell_ReadyWRATHWidgetCollapsedBorder *d(RenderTableCell_ReadyWRATHWidgetCollapsedBorder::object(this, handle));

    ContextOfWRATH::AutoPushNode autoPushRoot(wrath_context, d->m_root_node);
    ContextOfWRATH::AutoPushNode autoPushSkip(wrath_context, d->m_skip_node);

    if (!table()->currentBorderStyle()) {
        d->m_skip_node.widget()->visible(false);
        return;
    }

    d->m_skip_node.widget()->visible(true);
    
    CollapsedBorderValue leftVal = collapsedLeftBorder();
    CollapsedBorderValue rightVal = collapsedRightBorder();
    CollapsedBorderValue topVal = collapsedTopBorder();
    CollapsedBorderValue bottomVal = collapsedBottomBorder();
     
    // Adjust our x/y/width/height so that we paint the collapsed borders at the correct location.
    int topWidth = topVal.width();
    int bottomWidth = bottomVal.width();
    int leftWidth = leftVal.width();
    int rightWidth = rightVal.width();
    
    tx -= leftWidth / 2;
    ty -= topWidth / 2;
    w += leftWidth / 2 + (rightWidth + 1) / 2;
    h += topWidth / 2 + (bottomWidth + 1) / 2;
    
    EBorderStyle topStyle = collapsedBorderStyle(topVal.style());
    EBorderStyle bottomStyle = collapsedBorderStyle(bottomVal.style());
    EBorderStyle leftStyle = collapsedBorderStyle(leftVal.style());
    EBorderStyle rightStyle = collapsedBorderStyle(rightVal.style());
    
    bool renderTop = topStyle > BHIDDEN && !topVal.isTransparent();
    bool renderBottom = bottomStyle > BHIDDEN && !bottomVal.isTransparent();
    bool renderLeft = leftStyle > BHIDDEN && !leftVal.isTransparent();
    bool renderRight = rightStyle > BHIDDEN && !rightVal.isTransparent();

    // We never paint diagonals at the joins.  We simply let the border with the highest
    // precedence paint on top of borders with lower precedence.  
    CollapsedBorders borders;
    borders.addBorder(topVal, BSTop, renderTop, tx, ty, tx + w, ty + topWidth, topStyle);
    borders.addBorder(bottomVal, BSBottom, renderBottom, tx, ty + h - bottomWidth, tx + w, ty + h, bottomStyle);
    borders.addBorder(leftVal, BSLeft, renderLeft, tx, ty, tx + leftWidth, ty + h, leftStyle);
    borders.addBorder(rightVal, BSRight, renderRight, tx + w - rightWidth, ty, tx + w, ty + h, rightStyle);

    /*
      [WRATH-DANGER]: These are not used at the moment
      const AffineTransform& currentCTM = graphicsContext->getCTM();
      bool antialias = !currentCTM.isIdentityOrTranslationOrFlipped();
    */
    
    d->m_top.visible(false);
    d->m_bottom.visible(false);
    d->m_left.visible(false);
    d->m_right.visible(false);
    for (CollapsedBorder* border = borders.nextBorder(); border; border = borders.nextBorder()) {
        if (border->borderValue == *table()->currentBorderStyle()) {
            PaintedWidgetsOfWRATHHandle* bh = NULL;
            if (border->borderValue == topVal)
                bh = &d->m_top;
            else if (border->borderValue == bottomVal)
                bh = &d->m_bottom;
            else if (border->borderValue == leftVal)
                bh = &d->m_left;
            else if (border->borderValue == rightVal)
                bh = &d->m_right;
            else
                ASSERT_NOT_REACHED();
            bh->visible(true);
            readyWRATHWidgetDrawLineForBoxSide(*bh, wrath_context, border->x1, border->y1, border->x2, border->y2, border->side, 
					       border->borderValue.color(), border->style, 0, 0, false /*antialias*/);
        }
    }
}
}
#endif
