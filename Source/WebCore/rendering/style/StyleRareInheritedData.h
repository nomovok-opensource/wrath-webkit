/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Graham Dennis (graham.dennis@gmail.com)
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

#ifndef StyleRareInheritedData_h
#define StyleRareInheritedData_h

#include "Color.h"
#include "Length.h"
#include <wtf/RefCounted.h>
#include <wtf/PassRefPtr.h>
#include <wtf/text/AtomicString.h>

namespace WebCore {

class CursorList;
class QuotesData;
class ShadowData;

// This struct is for rarely used inherited CSS3, CSS2, and WebKit-specific properties.
// By grouping them together, we save space, and only allocate this object when someone
// actually uses one of these properties.
class StyleRareInheritedData : public RefCounted<StyleRareInheritedData> {
public:
    static PassRefPtr<StyleRareInheritedData> create() { return adoptRef(new StyleRareInheritedData); }
    PassRefPtr<StyleRareInheritedData> copy() const { return adoptRef(new StyleRareInheritedData(*this)); }
    ~StyleRareInheritedData();

    bool operator==(const StyleRareInheritedData& o) const;
    bool operator!=(const StyleRareInheritedData& o) const
    {
        return !(*this == o);
    }
    bool shadowDataEquivalent(const StyleRareInheritedData&) const;

    Color m_textStrokeColor;
    float m_textStrokeWidth;
    Color m_textFillColor;
    Color m_textEmphasisColor;

    OwnPtr<ShadowData> m_textShadow; // Our text shadow information for shadowed text drawing.
    AtomicString m_highlight; // Apple-specific extension for custom highlight rendering.
    
    RefPtr<CursorList> m_cursorData;
    Length m_indent;
    float m_effectiveZoom;

    // Paged media properties.
    short m_widows;
    short m_orphans;
    
    unsigned m_textSecurity : 2; // ETextSecurity
    unsigned m_userModify : 2; // EUserModify (editing)
    unsigned m_wordBreak : 2; // EWordBreak
    unsigned m_wordWrap : 1; // EWordWrap 
    unsigned m_nbspMode : 1; // ENBSPMode
    unsigned m_khtmlLineBreak : 1; // EKHTMLLineBreak
    bool m_textSizeAdjust : 1; // An Apple extension.
    unsigned m_resize : 2; // EResize
    unsigned m_userSelect : 1;  // EUserSelect
    unsigned m_colorSpace : 1; // ColorSpace
    unsigned m_speak : 3; // ESpeak
    unsigned m_hyphens : 2; // Hyphens
    unsigned m_textEmphasisFill : 1; // TextEmphasisFill
    unsigned m_textEmphasisMark : 3; // TextEmphasisMark
    unsigned m_textEmphasisPosition : 1; // TextEmphasisPosition
    unsigned m_lineBoxContain: 7; // LineBoxContain

    AtomicString m_hyphenationString;
    short m_hyphenationLimitBefore;
    short m_hyphenationLimitAfter;

    AtomicString m_locale;

    AtomicString m_textEmphasisCustomMark;
    RefPtr<QuotesData> m_quotes;

private:
    StyleRareInheritedData();
    StyleRareInheritedData(const StyleRareInheritedData&);
};

} // namespace WebCore

#endif // StyleRareInheritedData_h
