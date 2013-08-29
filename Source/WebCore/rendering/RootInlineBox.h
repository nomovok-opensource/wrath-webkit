/*
 * Copyright (C) 2003, 2006, 2007, 2008 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef RootInlineBox_h
#define RootInlineBox_h

#include "BidiContext.h"
#include "InlineFlowBox.h"

namespace WebCore {

class EllipsisBox;
class HitTestResult;

struct BidiStatus;
struct GapRects;

class RootInlineBox : public InlineFlowBox {
public:
#if USE(WRATH)
    enum MemberChange
    {
        E_paginationStrut,
        E_floats,
        E_baseLineType,
        E_hasAnnotations,
        E_endsWithBreak,
        E_lineBreakInfo,
        E_lineBreakPos,
        E_blockLogicalHeight,
        E_lineTopBottomPositions,
        E_numberOfChanges
    };
    typedef Signal<RootInlineBox> SignalType;
    SignalType::connection connect(const SignalType::slot_type & slot) { return m_signal.connect(slot); }
#endif

    RootInlineBox(RenderBlock* block);

    virtual void destroy(RenderArena*);

    virtual bool isRootInlineBox() const { return true; }

    void detachEllipsisBox(RenderArena*);

    RootInlineBox* nextRootBox() const { return static_cast<RootInlineBox*>(m_nextLineBox); }
    RootInlineBox* prevRootBox() const { return static_cast<RootInlineBox*>(m_prevLineBox); }

    virtual void adjustPosition(float dx, float dy);

    int lineTop() const { return m_lineTop; }
    int lineBottom() const { return m_lineBottom; }

    int paginationStrut() const { return m_paginationStrut; }
    void setPaginationStrut(int s) { SET_VALUE_AND_EMIT(m_paginationStrut, s, m_signal, E_paginationStrut); }

    int selectionTop() const;
    int selectionBottom() const;
    int selectionHeight() const { return max(0, selectionBottom() - selectionTop()); }

    int alignBoxesInBlockDirection(int heightOfBlock, GlyphOverflowAndFallbackFontsMap&, VerticalPositionCache&);
    void setLineTopBottomPositions(int top, int bottom);

    virtual RenderLineBoxList* rendererLineBoxes() const;

    RenderObject* lineBreakObj() const { return m_lineBreakObj; }
    BidiStatus lineBreakBidiStatus() const;
    void setLineBreakInfo(RenderObject*, unsigned breakPos, const BidiStatus&);

    unsigned lineBreakPos() const { return m_lineBreakPos; }
    void setLineBreakPos(unsigned p) { SET_VALUE_AND_EMIT(m_lineBreakPos, p, m_signal, E_lineBreakPos); }

    int blockLogicalHeight() const { return m_blockLogicalHeight; }
    void setBlockLogicalHeight(int h) { SET_VALUE_AND_EMIT(m_blockLogicalHeight, h, m_signal, E_blockLogicalHeight); }

    bool endsWithBreak() const { return m_endsWithBreak; }
    void setEndsWithBreak(bool b)
    {
        SET_VALUE_AND_EMIT(m_endsWithBreak, b, m_signal, E_endsWithBreak);
    }

    void childRemoved(InlineBox* box);

    bool lineCanAccommodateEllipsis(bool ltr, int blockEdge, int lineBoxEdge, int ellipsisWidth);
    void placeEllipsis(const AtomicString& ellipsisStr, bool ltr, float blockLeftEdge, float blockRightEdge, float ellipsisWidth, InlineBox* markupBox = 0);
    virtual float placeEllipsisBox(bool ltr, float blockLeftEdge, float blockRightEdge, float ellipsisWidth, bool& foundBox);

    EllipsisBox* ellipsisBox() const;

    void paintEllipsisBox(PaintInfo&, int tx, int ty, int lineTop, int lineBottom) const;

    virtual void clearTruncation();

    virtual int baselinePosition(FontBaseline baselineType) const { return boxModelObject()->baselinePosition(baselineType, m_firstLine, isHorizontal() ? HorizontalLine : VerticalLine, PositionOfInteriorLineBoxes); }
    virtual int lineHeight() const { return boxModelObject()->lineHeight(m_firstLine, isHorizontal() ? HorizontalLine : VerticalLine, PositionOfInteriorLineBoxes); }

#if PLATFORM(MAC)
    void addHighlightOverflow();
    void paintCustomHighlight(PaintInfo&, int tx, int ty, const AtomicString& highlightType);
#endif

    virtual void paint(PaintInfo&, int tx, int ty, int lineTop, int lineBottom);
    virtual bool nodeAtPoint(const HitTestRequest&, HitTestResult&, int x, int y, int tx, int ty, int lineTop, int lineBottom);
#if USE(WRATH)
  virtual void readyWRATHWidgets(PaintedWidgetsOfWRATHHandleT<InlineBox> &h, 
                                 PaintInfoOfWRATH&, int tx, int ty, int lineTop, int lineBottom);
  void readyWRATHEllipsisBox(PaintedWidgetsOfWRATHHandleT<InlineBox> &handle, 
                             PaintInfoOfWRATH &paintInfo, int tx, int ty, int lineTop, int lineBottom) const;
#endif

    bool hasSelectedChildren() const { return m_hasSelectedChildrenOrCanHaveLeadingExpansion; }
    void setHasSelectedChildren(bool hasSelectedChildren) { m_hasSelectedChildrenOrCanHaveLeadingExpansion = hasSelectedChildren; }

    virtual RenderObject::SelectionState selectionState();
    InlineBox* firstSelectedBox();
    InlineBox* lastSelectedBox();

    GapRects lineSelectionGap(RenderBlock* rootBlock, const IntPoint& rootBlockPhysicalPosition, const IntSize& offsetFromRootBlock, int selTop, int selHeight, const PaintInfo*);

    RenderBlock* block() const;

    InlineBox* closestLeafChildForLogicalLeftPosition(int, bool onlyEditableLeaves = false);

    void appendFloat(RenderBox* floatingBox)
    {
        ASSERT(!isDirty());
        if (m_floats)
            m_floats->append(floatingBox);
        else
        {
            m_floats= adoptPtr(new Vector<RenderBox*>(1, floatingBox));
#if USE(WRATH)
            m_signal(E_floats);
#endif
        }
    }

    Vector<RenderBox*>* floatsPtr() { ASSERT(!isDirty()); return m_floats.get(); }

    virtual void extractLineBoxFromRenderObject();
    virtual void attachLineBoxToRenderObject();
    virtual void removeLineBoxFromRenderObject();
    
    FontBaseline baselineType() const { return static_cast<FontBaseline>(m_baselineType); }

    bool hasAnnotationsBefore() const { return m_hasAnnotationsBefore; }
    bool hasAnnotationsAfter() const { return m_hasAnnotationsAfter; }

    IntRect paddedLayoutOverflowRect(int endPadding) const;

    void ascentAndDescentForBox(InlineBox*, GlyphOverflowAndFallbackFontsMap&, int& ascent, int& descent, bool& affectsAscent, bool& affectsDescent) const;
    int verticalPositionForBox(InlineBox*, VerticalPositionCache&);
    bool includeLeadingForBox(InlineBox*) const;
    bool includeFontForBox(InlineBox*) const;
    bool includeGlyphsForBox(InlineBox*) const;
    bool includeMarginForBox(InlineBox*) const;
    bool fitsToGlyphs() const;
    bool includesRootLineBoxFontOrLeading() const;
    
    int logicalTopVisualOverflow() const
    {
        return InlineFlowBox::logicalTopVisualOverflow(lineTop());
    }
    int logicalBottomVisualOverflow() const
    {
        return InlineFlowBox::logicalBottomVisualOverflow(lineBottom());
    }
    int logicalTopLayoutOverflow() const
    {
        return InlineFlowBox::logicalTopLayoutOverflow(lineTop());
    }
    int logicalBottomLayoutOverflow() const
    {
        return InlineFlowBox::logicalBottomLayoutOverflow(lineBottom());
    }

    Node* getLogicalStartBoxWithNode(InlineBox*&) const;
    Node* getLogicalEndBoxWithNode(InlineBox*&) const;
#ifndef NDEBUG
    virtual const char* boxName() const;
#endif
private:
    bool hasEllipsisBox() const { return m_hasEllipsisBoxOrHyphen; }
    void setHasEllipsisBox(bool hasEllipsisBox) { m_hasEllipsisBoxOrHyphen = hasEllipsisBox; }

    void incLineTop(int i)
    {
        m_lineTop += i;
#if USE(WRATH)
        m_signal(E_lineTopBottomPositions);
#endif
    }

    void incLineBottom(int i)
    {
        m_lineBottom += i;
#if USE(WRATH)
        m_signal(E_lineTopBottomPositions);
#endif
    }

    void incBlockLogicalHeight(int i)
    {
        m_blockLogicalHeight += i;
#if USE(WRATH)
        m_signal(E_blockLogicalHeight);
#endif
    }

    void setBaselineType(unsigned t)
    {
        SET_VALUE_AND_EMIT(m_baselineType, t, m_signal, E_baseLineType);
    }

    void setHasAnnotations(bool before, bool after)
    {
        if(m_hasAnnotationsBefore != before or m_hasAnnotationsAfter != after)
        {
            m_hasAnnotationsBefore = before;
            m_hasAnnotationsAfter = after;
#if USE(WRATH)
            m_signal(E_hasAnnotations);
#endif
        }
    }

    int beforeAnnotationsAdjustment() const;

    // Where this line ended.  The exact object and the position within that object are stored so that
    // we can create an InlineIterator beginning just after the end of this line.
    RenderObject* m_lineBreakObj;
    unsigned m_lineBreakPos;
    RefPtr<BidiContext> m_lineBreakContext;

    int m_lineTop;
    int m_lineBottom;

    int m_paginationStrut;

    // Floats hanging off the line are pushed into this vector during layout. It is only
    // good for as long as the line has not been marked dirty.
    OwnPtr<Vector<RenderBox*> > m_floats;

    // The logical height of the block at the end of this line.  This is where the next line starts.
    int m_blockLogicalHeight;

    // Whether or not this line uses alphabetic or ideographic baselines by default.
    unsigned m_baselineType : 1; // FontBaseline
    
    // If the line contains any ruby runs, then this will be true.
    bool m_hasAnnotationsBefore : 1;
    bool m_hasAnnotationsAfter : 1;

    WTF::Unicode::Direction m_lineBreakBidiStatusEor : 5;
    WTF::Unicode::Direction m_lineBreakBidiStatusLastStrong : 5;
    WTF::Unicode::Direction m_lineBreakBidiStatusLast : 5;

private:
#if USE(WRATH)
    SignalType m_signal;
#endif
};

inline void RootInlineBox::setLineTopBottomPositions(int top, int bottom) 
{ 
    if(m_lineTop != top or
       m_lineBottom != bottom)
    {
        m_lineTop = top; 
        m_lineBottom = bottom;
#if USE(WRATH)
        m_signal(E_lineTopBottomPositions);
#endif
    }
}

} // namespace WebCore

#endif // RootInlineBox_h
