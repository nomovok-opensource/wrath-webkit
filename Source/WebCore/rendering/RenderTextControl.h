/*
 * Copyright (C) 2006, 2007 Apple Inc. All rights reserved.
 *           (C) 2008 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/) 
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

#ifndef RenderTextControl_h
#define RenderTextControl_h

#include "RenderBlock.h"

namespace WebCore {

class VisibleSelection;
class TextControlInnerElement;
class TextControlInnerTextElement;

class RenderTextControl : public RenderBlock {
public:
#if USE(WRATH)
    enum MemberChange
    {
        E_placeholderVisible,
        E_lastChangeWasUserEdit,
        E_innerText,
        E_numberOfChanges
    };
#endif

    virtual ~RenderTextControl();

    HTMLElement* innerTextElement() const;

    bool lastChangeWasUserEdit() const { return m_lastChangeWasUserEdit; }
    void setLastChangeWasUserEdit(bool lastChangeWasUserEdit);

    int selectionStart() const;
    int selectionEnd() const;
    PassRefPtr<Range> selection(int start, int end) const;

    virtual void subtreeHasChanged();
    String text();
    String textWithHardLineBreaks();
    void selectionChanged(bool userTriggered);

    VisiblePosition visiblePositionForIndex(int index) const;
    int indexForVisiblePosition(const VisiblePosition&) const;

    void updatePlaceholderVisibility(bool, bool);

protected:
    RenderTextControl(Node*, bool);

    int scrollbarThickness() const;
    void adjustInnerTextStyle(const RenderStyle* startStyle, RenderStyle* textBlockStyle) const;
    void setInnerTextValue(const String&);

    virtual void styleDidChange(StyleDifference, const RenderStyle* oldStyle);

    void createSubtreeIfNeeded(TextControlInnerElement* innerBlock);
    void hitInnerTextElement(HitTestResult&, int x, int y, int tx, int ty);
    void forwardEvent(Event*);

    int textBlockWidth() const;
    int textBlockHeight() const;

    float scaleEmToUnits(int x) const;

    static bool hasValidAvgCharWidth(AtomicString family);
    virtual float getAvgCharWidth(AtomicString family);
    virtual int preferredContentWidth(float charWidth) const = 0;
    virtual void adjustControlHeightBasedOnLineHeight(int lineHeight) = 0;
    virtual void cacheSelection(int start, int end) = 0;
    virtual PassRefPtr<RenderStyle> createInnerTextStyle(const RenderStyle* startStyle) const = 0;
    virtual RenderStyle* textBaseStyle() const = 0;

    virtual void updateFromElement();
    virtual void computeLogicalHeight();

    bool m_placeholderVisible;

private:
    virtual const char* renderName() const { return "RenderTextControl"; }
    virtual bool isTextControl() const { return true; }
    virtual void computePreferredLogicalWidths();
    virtual void removeLeftoverAnonymousBlock(RenderBlock*) { }
    virtual bool canHaveChildren() const { return false; }
    virtual bool avoidsFloats() const { return true; }
    void setInnerTextStyle(PassRefPtr<RenderStyle>);
    virtual void paintObject(PaintInfo&, int tx, int ty);
#if USE(WRATH)
    virtual void readyWRATHWidgetObject(PaintedWidgetsOfWRATHHandle&,
                                        PaintInfoOfWRATH&, int tx, int ty);
#endif
    
    virtual void addFocusRingRects(Vector<IntRect>&, int tx, int ty);

    virtual bool canBeProgramaticallyScrolled(bool) const { return true; }

    virtual bool requiresForcedStyleRecalcPropagation() const { return true; }

    String finishText(Vector<UChar>&) const;

    bool hasVisibleTextArea() const;
    friend void setSelectionRange(Node*, int start, int end);
    bool isSelectableElement(Node*) const;
    
    virtual int textBlockInsetLeft() const = 0;
    virtual int textBlockInsetRight() const = 0;
    virtual int textBlockInsetTop() const = 0;

    void paintPlaceholder(PaintInfo&, int tx, int ty);
#if USE(WRATH)
    void readyWRATHWidgetPlaceholder(PaintedWidgetsOfWRATHHandle&,
				     PaintInfoOfWRATH&, int tx, int ty);
#endif

    bool m_lastChangeWasUserEdit;
    RefPtr<TextControlInnerTextElement> m_innerText;

#if USE(WRATH)
protected:
    typedef Signal<RenderTextControl> SignalType;
    SignalType::connection connect(const SignalType::slot_type & slot) { return m_signal.connect(slot); }
private:
    SignalType m_signal;
#endif
};

void setSelectionRange(Node*, int start, int end);

inline RenderTextControl* toRenderTextControl(RenderObject* object)
{ 
    ASSERT(!object || object->isTextControl());
    return static_cast<RenderTextControl*>(object);
}

inline const RenderTextControl* toRenderTextControl(const RenderObject* object)
{ 
    ASSERT(!object || object->isTextControl());
    return static_cast<const RenderTextControl*>(object);
}

// This will catch anyone doing an unnecessary cast.
void toRenderTextControl(const RenderTextControl*);

} // namespace WebCore

#endif // RenderTextControl_h
