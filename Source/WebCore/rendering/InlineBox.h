/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2009, 2010, 2011 Apple Inc. All rights reserved.
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

#ifndef InlineBox_h
#define InlineBox_h

#include "RenderBoxModelObject.h"
#include "TextDirection.h"

namespace WebCore {

class HitTestRequest;
class HitTestResult;
class RootInlineBox;

// InlineBox represents a rectangle that occurs on a line.  It corresponds to
// some RenderObject (i.e., it represents a portion of that RenderObject).
class InlineBox {
public:

#if USE(WRATH)
    enum MemberChange
    {
        E_x,
        E_y,
        E_logicalWidth,
        E_firstLine,
        E_constructed,
        E_bidiEmbeddingLevel,
        E_dirty,
        E_extracted,
        E_hasVirtualLogicalHeight,
        E_isHorizontal,
        E_knownToHaveNoOverflow,
        E_isText,
        E_dirOverride,
        E_determinedIfPrevOnLineExists,
        E_determinedIfNextOnLineExists,
        E_nextOnLineExists,
        E_prevOnLineExists,
        
        E_parent,
        E_remove,

        E_numberOfChanges
    };

    typedef boost::signals2::signal<void () >::slot_type on_dtor_slot_type;
    boost::signals2::connection connect_dtor(const on_dtor_slot_type &S)
    {
      return m_dtor_signal.connect(S);
    }


    typedef Signal<InlineBox> SignalType;
    SignalType::connection connect(const SignalType::slot_type & slot) { return m_signal.connect(slot); }

#endif

    InlineBox(RenderObject* obj)
        : m_next(0)
        , m_prev(0)
        , m_parent(0)
        , m_renderer(obj)
        , m_x(0)
        , m_y(0)
        , m_logicalWidth(0)
        , m_firstLine(false)
        , m_constructed(false)
        , m_bidiEmbeddingLevel(0)
        , m_dirty(false)
        , m_extracted(false)
#if ENABLE(SVG)
        , m_hasVirtualLogicalHeight(false)
#endif
        , m_isHorizontal(true)
        , m_endsWithBreak(false)
        , m_hasSelectedChildrenOrCanHaveLeadingExpansion(false)
        , m_knownToHaveNoOverflow(true)
        , m_hasEllipsisBoxOrHyphen(false)
        , m_dirOverride(false)
        , m_isText(false)
        , m_determinedIfNextOnLineExists(false)
        , m_determinedIfPrevOnLineExists(false)
        , m_nextOnLineExists(false)
        , m_prevOnLineExists(false)
        , m_expansion(0)
#ifndef NDEBUG
        , m_hasBadParent(false)
#endif
    {
    }

    InlineBox(RenderObject* obj, float x, float y, float logicalWidth, bool firstLine, bool constructed,
              bool dirty, bool extracted, bool isHorizontal, InlineBox* next, InlineBox* prev, InlineFlowBox* parent)
        : m_next(next)
        , m_prev(prev)
        , m_parent(parent)
        , m_renderer(obj)
        , m_x(x)
        , m_y(y)
        , m_logicalWidth(logicalWidth)
        , m_firstLine(firstLine)
        , m_constructed(constructed)
        , m_bidiEmbeddingLevel(0)
        , m_dirty(dirty)
        , m_extracted(extracted)
#if ENABLE(SVG)
        , m_hasVirtualLogicalHeight(false)
#endif
        , m_isHorizontal(isHorizontal)
        , m_endsWithBreak(false)
        , m_hasSelectedChildrenOrCanHaveLeadingExpansion(false)
        , m_knownToHaveNoOverflow(true)  
        , m_hasEllipsisBoxOrHyphen(false)
        , m_dirOverride(false)
        , m_isText(false)
        , m_determinedIfNextOnLineExists(false)
        , m_determinedIfPrevOnLineExists(false)
        , m_nextOnLineExists(false)
        , m_prevOnLineExists(false)
        , m_expansion(0)
#ifndef NDEBUG
        , m_hasBadParent(false)
#endif
    {
    }

    virtual ~InlineBox();

    virtual void destroy(RenderArena*);

    virtual void deleteLine(RenderArena*);
    virtual void extractLine();
    virtual void attachLine();

    virtual bool isLineBreak() const { return false; }

    virtual void adjustPosition(float dx, float dy);
    void adjustLineDirectionPosition(float delta)
    {
        if (isHorizontal())
            adjustPosition(delta, 0);
        else
            adjustPosition(0, delta);
    }
    void adjustBlockDirectionPosition(float delta)
    {
        if (isHorizontal())
            adjustPosition(0, delta);
        else
            adjustPosition(delta, 0);
    }

    virtual void paint(PaintInfo&, int tx, int ty, int lineTop, int lineBottom);
#if USE(WRATH)
  virtual void readyWRATHWidgets(PaintedWidgetsOfWRATHHandleT<InlineBox>&, 
                                 PaintInfoOfWRATH&, int tx, int ty, int lineTop, int lineBottom);

  /*
    temporary placeholder for those InlineBox derived objects that 
    we have not yet implemented readyWRATHWidgets; using InlineBox::readyWRATHWidgets
    for some InlineBox derived types causes a crash.
   */
  void readyWRATHWidgetsDefault(PaintedWidgetsOfWRATHHandleT<InlineBox>&, 
                                PaintInfoOfWRATH&, int tx, int ty, int lineTop, int lineBottom);
#endif


    virtual bool nodeAtPoint(const HitTestRequest&, HitTestResult&, int x, int y, int tx, int ty, int lineTop, int lineBottom);

    InlineBox* next() const { return m_next; }

    // Overloaded new operator.
    void* operator new(size_t, RenderArena*) throw();

    // Overridden to prevent the normal delete from being called.
    void operator delete(void*, size_t);

private:
    // The normal operator new is disallowed.
    void* operator new(size_t) throw();

public:
#ifndef NDEBUG
    void showTreeForThis() const;
    void showLineTreeForThis() const;
    
    virtual void showBox(int = 0) const;
    virtual void showLineTreeAndMark(const InlineBox* = 0, const char* = 0, const InlineBox* = 0, const char* = 0, const RenderObject* = 0, int = 0) const;
    virtual const char* boxName() const;
#endif

    bool isText() const { return m_isText; }
    void setIsText(bool b) { SET_VALUE_AND_EMIT(m_isText, b, m_signal, E_isText); }
 
    virtual bool isInlineFlowBox() const { return false; }
    virtual bool isInlineTextBox() const { return false; }
    virtual bool isRootInlineBox() const { return false; }
#if ENABLE(SVG)
    virtual bool isSVGInlineTextBox() const { return false; }
    virtual bool isSVGInlineFlowBox() const { return false; }
    virtual bool isSVGRootInlineBox() const { return false; }
#endif

    bool hasVirtualLogicalHeight() const { return m_hasVirtualLogicalHeight; }
    void setHasVirtualLogicalHeight() { SET_VALUE_AND_EMIT(m_hasVirtualLogicalHeight, true, m_signal, E_hasVirtualLogicalHeight); }
    virtual int virtualLogicalHeight() const
    {
        ASSERT_NOT_REACHED();
        return 0;
    }

    bool isHorizontal() const { return m_isHorizontal; }
    void setIsHorizontal(bool horizontal) { SET_VALUE_AND_EMIT(m_isHorizontal, horizontal, m_signal, E_isHorizontal); }

    virtual IntRect calculateBoundaries() const
    {
        ASSERT_NOT_REACHED();
        return IntRect();
    }

    bool isConstructed() { return m_constructed; }
    virtual void setConstructed() { SET_VALUE_AND_EMIT(m_constructed, true, m_signal, E_constructed); }

    void setExtracted(bool b = true) { SET_VALUE_AND_EMIT(m_extracted, b, m_signal, E_extracted); }
    
    void setFirstLineStyleBit(bool f) { SET_VALUE_AND_EMIT(m_firstLine, f, m_signal, E_firstLine); }
    bool isFirstLineStyle() const { return m_firstLine; }

    void remove();

    InlineBox* nextOnLine() const { return m_next; }
    InlineBox* prevOnLine() const { return m_prev; }
    void setNextOnLine(InlineBox* next)
    {
        ASSERT(m_parent || !next);
        m_next = next;
    }
    void setPrevOnLine(InlineBox* prev)
    {
        ASSERT(m_parent || !prev);
        m_prev = prev;
    }
    bool nextOnLineExists() const;
    bool prevOnLineExists() const;

    virtual bool isLeaf() const { return true; }
    
    InlineBox* nextLeafChild() const;
    InlineBox* prevLeafChild() const;
        
    RenderObject* renderer() const { return m_renderer; }

    InlineFlowBox* parent() const
    {
        ASSERT(!m_hasBadParent);
        return m_parent;
    }
    void setParent(InlineFlowBox* par) 
    { 
      SET_VALUE_AND_EMIT(m_parent, par, m_signal, E_parent); 
    }

    const RootInlineBox* root() const;
    RootInlineBox* root();

    // x() is the left side of the box in the containing block's coordinate system.
    void setX(float x) { SET_VALUE_AND_EMIT(m_x, x, m_signal, E_x); }
    float x() const { return m_x; }

    // y() is the top side of the box in the containing block's coordinate system.
    void setY(float y) { SET_VALUE_AND_EMIT(m_y, y, m_signal, E_y); }
    float y() const { return m_y; }

    float width() const { return isHorizontal() ? logicalWidth() : logicalHeight(); }
    float height() const { return isHorizontal() ? logicalHeight() : logicalWidth(); }

    // The logicalLeft position is the left edge of the line box in a horizontal line and the top edge in a vertical line.
    float logicalLeft() const { return isHorizontal() ? x() : y(); }
    float logicalRight() const { return logicalLeft() + logicalWidth(); }
    void setLogicalLeft(float left)
    {
        if (isHorizontal())
            setX(left);
        else
            setY(left);
    }
    int pixelSnappedLogicalLeft() const { return logicalLeft(); }
    int pixelSnappedLogicalRight() const { return ceilf(logicalRight()); }

    // The logicalTop[ position is the top edge of the line box in a horizontal line and the left edge in a vertical line.
    int logicalTop() const { return isHorizontal() ? y() : x(); }
    int logicalBottom() const { return logicalTop() + logicalHeight(); }
    void setLogicalTop(int top)
    {
        if (isHorizontal())
            setY(top);
        else
            setX(top);
    }

    // The logical width is our extent in the line's overall inline direction, i.e., width for horizontal text and height for vertical text.
    void setLogicalWidth(float w) { SET_VALUE_AND_EMIT(m_logicalWidth, w, m_signal, E_logicalWidth); }
    float logicalWidth() const { return m_logicalWidth; }

    // The logical height is our extent in the block flow direction, i.e., height for horizontal text and width for vertical text.
    int logicalHeight() const;

    FloatRect logicalFrameRect() const { return isHorizontal() ? IntRect(x(), y(), logicalWidth(), logicalHeight()) : IntRect(y(), x(), logicalWidth(), logicalHeight()); }

    virtual int baselinePosition(FontBaseline baselineType) const { return boxModelObject()->baselinePosition(baselineType, m_firstLine, isHorizontal() ? HorizontalLine : VerticalLine, PositionOnContainingLine); }
    virtual int lineHeight() const { return boxModelObject()->lineHeight(m_firstLine, isHorizontal() ? HorizontalLine : VerticalLine, PositionOnContainingLine); }
    
    virtual int caretMinOffset() const;
    virtual int caretMaxOffset() const;
    virtual unsigned caretMaxRenderedOffset() const;

    unsigned char bidiLevel() const { return m_bidiEmbeddingLevel; }
    void setBidiLevel(unsigned char level) { SET_VALUE_AND_EMIT(m_bidiEmbeddingLevel, level, m_signal, E_bidiEmbeddingLevel); }
    TextDirection direction() const { return m_bidiEmbeddingLevel % 2 ? RTL : LTR; }
    bool isLeftToRightDirection() const { return direction() == LTR; }
    int caretLeftmostOffset() const { return isLeftToRightDirection() ? caretMinOffset() : caretMaxOffset(); }
    int caretRightmostOffset() const { return isLeftToRightDirection() ? caretMaxOffset() : caretMinOffset(); }

    virtual void clearTruncation() { }

    bool isDirty() const { return m_dirty; }
    void markDirty(bool dirty = true) { SET_VALUE_AND_EMIT(m_dirty, dirty, m_signal, E_dirty); }

    void dirtyLineBoxes();
    
    virtual RenderObject::SelectionState selectionState();

    virtual bool canAccommodateEllipsis(bool ltr, int blockEdge, int ellipsisWidth);
    // visibleLeftEdge, visibleRightEdge are in the parent's coordinate system.
    virtual float placeEllipsisBox(bool ltr, float visibleLeftEdge, float visibleRightEdge, float ellipsisWidth, bool&);

    void setHasBadParent();

    int expansion() const { return m_expansion; }
    
    bool visibleToHitTesting() const { return renderer()->style()->visibility() == VISIBLE && renderer()->style()->pointerEvents() != PE_NONE; }
    
    EVerticalAlign verticalAlign() const { return renderer()->style(m_firstLine)->verticalAlign(); }

    // Use with caution! The type is not checked!
    RenderBoxModelObject* boxModelObject() const
    { 
        if (!m_renderer->isText())
            return toRenderBoxModelObject(m_renderer);
        return 0;
    }

    FloatPoint locationIncludingFlipping();
    void flipForWritingMode(FloatRect&);
    FloatPoint flipForWritingMode(const FloatPoint&);
    void flipForWritingMode(IntRect&);
    IntPoint flipForWritingMode(const IntPoint&);

    bool knownToHaveNoOverflow() const { return m_knownToHaveNoOverflow; }
    void clearKnownToHaveNoOverflow();

private:
    void setKnownToHaveNoOverflow(bool b=true) { SET_VALUE_AND_EMIT(m_knownToHaveNoOverflow, b, m_signal, E_knownToHaveNoOverflow); } 

    InlineBox* m_next; // The next element on the same line as us.
    InlineBox* m_prev; // The previous element on the same line as us.

    InlineFlowBox* m_parent; // The box that contains us.
    RenderObject* m_renderer;

    float m_x;
    float m_y;
    float m_logicalWidth;
    
    // Some of these bits are actually for subclasses and moved here to compact the structures.

    // for this class
protected:
    bool m_firstLine : 1;
private:
    bool m_constructed : 1;
    unsigned char m_bidiEmbeddingLevel : 6;
protected:
    bool m_dirty : 1;
    bool m_extracted : 1;
    bool m_hasVirtualLogicalHeight : 1;

    bool m_isHorizontal : 1;

    // for RootInlineBox
    bool m_endsWithBreak : 1;  // Whether the line ends with a <br>.
    // shared between RootInlineBox and InlineTextBox
    bool m_hasSelectedChildrenOrCanHaveLeadingExpansion : 1; // Whether we have any children selected (this bit will also be set if the <br> that terminates our line is selected).
    bool m_knownToHaveNoOverflow : 1;
    bool m_hasEllipsisBoxOrHyphen : 1;

    // for InlineTextBox
public:
    void setDirOverride(bool b=true)
    {
        SET_VALUE_AND_EMIT(m_dirOverride, b, m_signal, E_dirOverride);
    }
    bool dirOverride() const { return m_dirOverride; }

private:
    bool m_dirOverride : 1;
    bool m_isText : 1; // Whether or not this object represents text with a non-zero height. Includes non-image list markers, text boxes.

protected:
    void setDeterminedIfNextOnLineExists(bool b=true) const
    {
        SET_VALUE_AND_EMIT(m_determinedIfNextOnLineExists, b, m_signal, E_determinedIfNextOnLineExists);
    }

    void setDeterminedIfPrevOnLineExists(bool b=true) const
    {
        SET_VALUE_AND_EMIT(m_determinedIfPrevOnLineExists, b, m_signal, E_determinedIfPrevOnLineExists);
    }

    bool determinedIfNextOnLineExists() const { return m_determinedIfNextOnLineExists; }
    bool determinedIfPrevOnLineExists() const { return m_determinedIfPrevOnLineExists; }
    void setNextOnLineExists(bool b=true) const
    {
        SET_VALUE_AND_EMIT(m_nextOnLineExists, b, m_signal, E_nextOnLineExists);
    }

    void setPrevOnLineExists(bool b=true) const
    {
        SET_VALUE_AND_EMIT(m_prevOnLineExists, b, m_signal, E_prevOnLineExists);
    }

    mutable bool m_determinedIfNextOnLineExists : 1;
    mutable bool m_determinedIfPrevOnLineExists : 1;
    mutable bool m_nextOnLineExists : 1;
    mutable bool m_prevOnLineExists : 1;
    int m_expansion : 11; // for justified text

#ifndef NDEBUG
private:
    bool m_hasBadParent;
#endif

#if USE(WRATH)
    boost::signals2::signal<void () > m_dtor_signal;
    SignalType m_signal;
#endif
};


inline void InlineBox::setHasBadParent()
{
#ifndef NDEBUG
    m_hasBadParent = true;
#endif
}

} // namespace WebCore

#ifndef NDEBUG
// Outside the WebCore namespace for ease of invocation from gdb.
void showTree(const WebCore::InlineBox*);
void showLineTree(const WebCore::InlineBox*);
#endif

#endif // InlineBox_h
