#include "config.h"
#include "RenderTableSection.h"
#include "CachedImage.h"
#include "Document.h"
#include "HitTestResult.h"
#include "HTMLNames.h"
#include "PaintInfo.h"
#include "RenderTableCell.h"
#include "RenderTableCol.h"
#include "RenderTableRow.h"
#include "RenderView.h"
#include <limits>
#include <wtf/HashSet.h>
#include <wtf/Vector.h>

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
#include "HierarchyOfHandlesOfWRATH.h"
namespace
{
    class RenderTableSection_ReadyWRATHWidgets:
      public WebCore::PaintedWidgetsOfWRATH<RenderTableSection_ReadyWRATHWidgets>
    {
    public:
        WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_skip_node;

        WebCore::PaintedWidgetsOfWRATHHandle m_push_pop;
        WebCore::PaintedWidgetsOfWRATHHandle m_object;
    };

    class RenderTableSection_ReadyWRATHWidgetObject:
      public WebCore::PaintedWidgetsOfWRATH<RenderTableSection_ReadyWRATHWidgetObject>
    {
    public:
        WebCore::HierarchyOfHandlesOfWRATH<WebCore::RenderObject> m_multiple_cell_levels;
        WebCore::HierarchyOfHandlesOfWRATH<WebCore::RenderObject> m_single_cell_level;
    };

    class RenderTableSection_ReadyWRATHWidgetCell:
      public WebCore::PaintedWidgetsOfWRATH<RenderTableSection_ReadyWRATHWidgetCell>
    {
    public:
        WebCore::PaintedWidgetsOfWRATHHandle m_col_group;
        WebCore::PaintedWidgetsOfWRATHHandle m_col;
        WebCore::PaintedWidgetsOfWRATHHandle m_row_group;
        WebCore::PaintedWidgetsOfWRATHHandle m_row;
        WebCore::PaintedWidgetsOfWRATHHandle m_cell;
    };
}

namespace WebCore {

static inline bool compareCellPositions(RenderTableCell* elem1, RenderTableCell* elem2)
{
    return elem1->row() < elem2->row();
}

void RenderTableSection::readyWRATHWidgets(PaintedWidgetsOfWRATHHandle &handle,
                                           PaintInfoOfWRATH &paintInfo, int tx, int ty)
{ 
    RenderTableSection_ReadyWRATHWidgets *d(RenderTableSection_ReadyWRATHWidgets::object(this, handle));

    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);
    ContextOfWRATH::AutoPushNode autoPushSkip(paintInfo.wrath_context, d->m_skip_node);

    d->m_skip_node.widget()->visible(true);

    // put this back in when all layout tests can handle it
    // ASSERT(!needsLayout());
    // avoid crashing on bugs that cause us to paint with dirty layout
    if (needsLayout()) {
        d->m_skip_node.widget()->visible(false);
        return;
    }
    
    unsigned totalRows = m_gridRows;
    unsigned totalCols = table()->columns().size();

    if (!totalRows || !totalCols) {
        d->m_skip_node.widget()->visible(false);
        return;
    }

    tx += x();
    ty += y();

    PaintPhase phase = paintInfo.phase;
    bool pushedClip = pushContentsClipWRATH(d->m_push_pop, paintInfo, tx, ty);
    readyWRATHWidgetObject(d->m_object, paintInfo, tx, ty);
    if (pushedClip)
        popContentsClipWRATH(d->m_push_pop, paintInfo, phase, tx, ty);
}

void RenderTableSection::readyWRATHWidgetCell(PaintedWidgetsOfWRATHHandle& handle,
                                              RenderTableCell* cell, PaintInfoOfWRATH& paintInfo, int tx, int ty)

{
    RenderTableSection_ReadyWRATHWidgetCell *d(RenderTableSection_ReadyWRATHWidgetCell::object(this, handle));

    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    IntPoint cellPoint = flipForWritingMode(cell, IntPoint(tx, ty), ParentToChildFlippingAdjustment);
    PaintPhase paintPhase = paintInfo.phase;
    RenderTableRow* row = toRenderTableRow(cell->parent());

    if (paintPhase == PaintPhaseBlockBackground || paintPhase == PaintPhaseChildBlockBackground) {
        // We need to handle painting a stack of backgrounds.  This stack (from bottom to top) consists of
        // the column group, column, row group, row, and then the cell.
        RenderObject* col = table()->colElement(cell->col());
        RenderObject* colGroup = 0;
        if (col && col->parent()->style()->display() == TABLE_COLUMN_GROUP)
            colGroup = col->parent();

        // Column groups and columns first.
        // FIXME: Columns and column groups do not currently support opacity, and they are being painted "too late" in
        // the stack, since we have already opened a transparency layer (potentially) for the table row group.
        // Note that we deliberately ignore whether or not the cell has a layer, since these backgrounds paint "behind" the
        // cell.
        cell->readyWRATHWidgetBackgroundsBehindCell(d->m_col_group, paintInfo, cellPoint.x(), cellPoint.y(), colGroup);
        cell->readyWRATHWidgetBackgroundsBehindCell(d->m_col, paintInfo, cellPoint.x(), cellPoint.y(), col);

        // Paint the row group next.
        cell->readyWRATHWidgetBackgroundsBehindCell(d->m_row_group, paintInfo, cellPoint.x(), cellPoint.y(), this);

        // Paint the row next, but only if it doesn't have a layer.  If a row has a layer, it will be responsible for
        // painting the row background for the cell.
        if (!row->hasSelfPaintingLayer()) {
            d->m_row.visible(true);
            cell->readyWRATHWidgetBackgroundsBehindCell(d->m_row, paintInfo, cellPoint.x(), cellPoint.y(), row);
        }
        else
            d->m_row.visible(false);
    }
    if ((!cell->hasSelfPaintingLayer() && !row->hasSelfPaintingLayer()) || paintInfo.phase == PaintPhaseCollapsedTableBorders) {
        d->m_cell.visible(true);
        cell->readyWRATHWidgets(d->m_cell, paintInfo, cellPoint.x(), cellPoint.y());
    } else {
        d->m_cell.visible(false);
    }
}

void RenderTableSection::readyWRATHWidgetObject(PaintedWidgetsOfWRATHHandle &handle,
                                                PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
    RenderTableSection_ReadyWRATHWidgetObject *d(RenderTableSection_ReadyWRATHWidgetObject::object(this, handle));

    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    d->m_single_cell_level.hideEachObject();
    d->m_multiple_cell_levels.hideEachObject();


    // Check which rows and cols are visible and only paint these.
    // FIXME: Could use a binary search here.
    unsigned totalRows = m_gridRows;
    unsigned totalCols = table()->columns().size();

    PaintPhase paintPhase = paintInfo.phase;

    int os = 2 * maximalOutlineSize(paintPhase);
    unsigned startrow = 0;
    unsigned endrow = totalRows;

    IntRect localRepaintRect = paintInfo.rect;
    localRepaintRect.move(-tx, -ty);
    if (style()->isFlippedBlocksWritingMode()) {
        if (style()->isHorizontalWritingMode())
            localRepaintRect.setY(height() - localRepaintRect.maxY());
        else
            localRepaintRect.setX(width() - localRepaintRect.maxX());
    }

    // If some cell overflows, just paint all of them.
    /* [WRATH-DANGER]: Screw that if statement (and the one below), we paint all of them */
#if 1
    if (!m_hasOverflowingCell) {
        int before = (style()->isHorizontalWritingMode() ? localRepaintRect.y() : localRepaintRect.x()) - os;
        // binary search to find a row
        startrow = std::lower_bound(m_rowPos.begin(), m_rowPos.end(), before) - m_rowPos.begin();

        // The binary search above gives us the first row with
        // a y position >= the top of the paint rect. Thus, the previous
        // may need to be repainted as well.
        if (startrow == m_rowPos.size() || (startrow > 0 && (m_rowPos[startrow] >  before)))
          --startrow;

        int after = (style()->isHorizontalWritingMode() ? localRepaintRect.maxY() : localRepaintRect.maxX()) + os;
        endrow = std::lower_bound(m_rowPos.begin(), m_rowPos.end(), after) - m_rowPos.begin();
        if (endrow == m_rowPos.size())
          --endrow;

        if (!endrow && m_rowPos[0] - table()->outerBorderBefore() <= after)
            ++endrow;
    }
#endif

    unsigned startcol = 0;
    unsigned endcol = totalCols;
    // FIXME: Implement RTL.
    /* [WRATH-DANGER]: See above: We paint all of them */
#if 1
    if (!m_hasOverflowingCell && style()->isLeftToRightDirection()) {
        int start = (style()->isHorizontalWritingMode() ? localRepaintRect.x() : localRepaintRect.y()) - os;
        Vector<int>& columnPos = table()->columnPositions();
        startcol = std::lower_bound(columnPos.begin(), columnPos.end(), start) - columnPos.begin();
        if ((startcol == columnPos.size()) || (startcol > 0 && (columnPos[startcol] > start)))
            --startcol;

        int end = (style()->isHorizontalWritingMode() ? localRepaintRect.maxX() : localRepaintRect.maxY()) + os;
        endcol = std::lower_bound(columnPos.begin(), columnPos.end(), end) - columnPos.begin();
        if (endcol == columnPos.size())
            --endcol;

        if (!endcol && columnPos[0] - table()->outerBorderStart() <= end)
            ++endcol;
    }
#endif
    if (startcol < endcol) {
        if (!m_hasMultipleCellLevels) {
            // Draw the dirty cells in the order that they appear.
            d->m_multiple_cell_levels.clear();
            for (unsigned r = startrow; r < endrow; r++) {
                for (unsigned c = startcol; c < endcol; c++) {
                    CellStruct& current = cellAt(r, c);
                    RenderTableCell* cell = current.primaryCell();
                    if (!cell || (r > startrow && primaryCellAt(r - 1, c) == cell) || (c > startcol && primaryCellAt(r, c - 1) == cell))
                        continue;
                    PaintedWidgetsOfWRATHHandle &ch(d->m_single_cell_level.getHandle(cell));
                    ch.visible(true);
                    readyWRATHWidgetCell(ch, cell, paintInfo, tx, ty);
                }
            }
            d->m_single_cell_level.removeNonVisibleHandles();
        } else {
            // Draw the cells in the correct paint order.
            d->m_single_cell_level.clear();
            Vector<RenderTableCell*> cells;
            HashSet<RenderTableCell*> spanningCells;
            for (unsigned r = startrow; r < endrow; r++) {
                for (unsigned c = startcol; c < endcol; c++) {
                    CellStruct& current = cellAt(r, c);
                    if (!current.hasCells())
                        continue;
                    for (unsigned i = 0; i < current.cells.size(); ++i) {
                        if (current.cells[i]->rowSpan() > 1 || current.cells[i]->colSpan() > 1) {
                            if (spanningCells.contains(current.cells[i]))
                                continue;
                            spanningCells.add(current.cells[i]);
                        }
                        cells.append(current.cells[i]);
                    }
                }
            }
            // Sort the dirty cells by paint order.
            std::stable_sort(cells.begin(), cells.end(), compareCellPositions);
            int size = cells.size();
            // Paint the cells.
            for (int i = 0; i < size; ++i) {
                PaintedWidgetsOfWRATHHandle &ch(d->m_multiple_cell_levels.getHandle(cells[i]));
                ch.visible(true);
                readyWRATHWidgetCell(ch, cells[i], paintInfo, tx, ty);
            }
            d->m_multiple_cell_levels.removeNonVisibleHandles();
        }
    }
}
}
#endif
