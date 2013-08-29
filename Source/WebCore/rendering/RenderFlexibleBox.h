/*
 * This file is part of the render object implementation for KHTML.
 *
 * Copyright (C) 2003 Apple Computer, Inc.
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

#ifndef RenderFlexibleBox_h
#define RenderFlexibleBox_h

#include "RenderBlock.h"

namespace WebCore {

class FlexBoxIterator;

class RenderFlexibleBox : public RenderBlock {
public:
    enum MemberChange
    {
        E_flexingChildren,
        E_stretchingChildren,
        E_numberOfChanges
    };

    RenderFlexibleBox(Node*);
    virtual ~RenderFlexibleBox();

    virtual const char* renderName() const;

    virtual void computePreferredLogicalWidths();
    void calcHorizontalPrefWidths();
    void calcVerticalPrefWidths();

    virtual void layoutBlock(bool relayoutChildren, int pageHeight);
    void layoutHorizontalBox(bool relayoutChildren);
    void layoutVerticalBox(bool relayoutChildren);

    virtual bool avoidsFloats() const { return true; }

    virtual bool isFlexibleBox() const { return true; }
    virtual bool isFlexingChildren() const { return m_flexingChildren; }
    virtual bool isStretchingChildren() const { return m_stretchingChildren; }

    void placeChild(RenderBox* child, int x, int y);

protected:
    int allowedChildFlex(RenderBox* child, bool expanding, unsigned group);

    bool hasMultipleLines() const { return style()->boxLines() == MULTIPLE; }
    bool isVertical() const { return style()->boxOrient() == VERTICAL; }
    bool isHorizontal() const { return style()->boxOrient() == HORIZONTAL; }

private:
    bool m_flexingChildren : 1;
    bool m_stretchingChildren : 1;

    void applyLineClamp(FlexBoxIterator&, bool relayoutChildren);

    void setFlexingChildren(bool t=true)
    {
        SET_VALUE_AND_EMIT(m_flexingChildren, t, m_signal, E_flexingChildren);
    }

    void setStretchingChildren(bool t=true)
    {
        SET_VALUE_AND_EMIT(m_stretchingChildren, t, m_signal, E_stretchingChildren);
    }

#if USE(WRATH)
protected:
    typedef Signal<RenderFlexibleBox> SignalType;
    SignalType::connection connect(const SignalType::slot_type & slot) { return m_signal.connect(slot); }
private:
    SignalType m_signal;
#endif
};

} // namespace WebCore

#endif // RenderFlexibleBox_h
