/*
 * Copyright (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010 Apple Inc. All rights reserved.
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

#include "config.h"
#include "StyleRareInheritedData.h"

#include "CursorList.h"
#include "QuotesData.h"
#include "RenderStyle.h"
#include "RenderStyleConstants.h"
#include "ShadowData.h"

namespace WebCore {

StyleRareInheritedData::StyleRareInheritedData()
    : m_textStrokeWidth(RenderStyle::initialTextStrokeWidth())
    , m_indent(RenderStyle::initialTextIndent())
    , m_effectiveZoom(RenderStyle::initialZoom())
    , m_widows(RenderStyle::initialWidows())
    , m_orphans(RenderStyle::initialOrphans())
    , m_textSecurity(RenderStyle::initialTextSecurity())
    , m_userModify(READ_ONLY)
    , m_wordBreak(RenderStyle::initialWordBreak())
    , m_wordWrap(RenderStyle::initialWordWrap())
    , m_nbspMode(NBNORMAL)
    , m_khtmlLineBreak(LBNORMAL)
    , m_textSizeAdjust(RenderStyle::initialTextSizeAdjust())
    , m_resize(RenderStyle::initialResize())
    , m_userSelect(RenderStyle::initialUserSelect())
    , m_colorSpace(ColorSpaceDeviceRGB)
    , m_speak(SpeakNormal)
    , m_hyphens(HyphensManual)
    , m_textEmphasisFill(TextEmphasisFillFilled)
    , m_textEmphasisMark(TextEmphasisMarkNone)
    , m_textEmphasisPosition(TextEmphasisPositionOver)
    , m_lineBoxContain(RenderStyle::initialLineBoxContain())
    , m_hyphenationLimitBefore(-1)
    , m_hyphenationLimitAfter(-1)
{
}

StyleRareInheritedData::StyleRareInheritedData(const StyleRareInheritedData& o)
    : RefCounted<StyleRareInheritedData>()
    , m_textStrokeColor(o.m_textStrokeColor)
    , m_textStrokeWidth(o.m_textStrokeWidth)
    , m_textFillColor(o.m_textFillColor)
    , m_textEmphasisColor(o.m_textEmphasisColor)
    , m_textShadow(o.m_textShadow ? adoptPtr(new ShadowData(*o.m_textShadow)) : nullptr)
    , m_highlight(o.m_highlight)
    , m_cursorData(o.m_cursorData)
    , m_indent(o.m_indent)
    , m_effectiveZoom(o.m_effectiveZoom)
    , m_widows(o.m_widows)
    , m_orphans(o.m_orphans)
    , m_textSecurity(o.m_textSecurity)
    , m_userModify(o.m_userModify)
    , m_wordBreak(o.m_wordBreak)
    , m_wordWrap(o.m_wordWrap)
    , m_nbspMode(o.m_nbspMode)
    , m_khtmlLineBreak(o.m_khtmlLineBreak)
    , m_textSizeAdjust(o.m_textSizeAdjust)
    , m_resize(o.m_resize)
    , m_userSelect(o.m_userSelect)
    , m_colorSpace(o.m_colorSpace)
    , m_speak(o.m_speak)
    , m_hyphens(o.m_hyphens)
    , m_textEmphasisFill(o.m_textEmphasisFill)
    , m_textEmphasisMark(o.m_textEmphasisMark)
    , m_textEmphasisPosition(o.m_textEmphasisPosition)
    , m_lineBoxContain(o.m_lineBoxContain)
    , m_hyphenationString(o.m_hyphenationString)
    , m_hyphenationLimitBefore(o.m_hyphenationLimitBefore)
    , m_hyphenationLimitAfter(o.m_hyphenationLimitAfter)
    , m_locale(o.m_locale)
    , m_textEmphasisCustomMark(o.m_textEmphasisCustomMark)
{ 
}

StyleRareInheritedData::~StyleRareInheritedData()
{
}

static bool cursorDataEquivalent(const CursorList* c1, const CursorList* c2)
{
    if (c1 == c2)
        return true;
    if ((!c1 && c2) || (c1 && !c2))
        return false;
    return (*c1 == *c2);
}

bool StyleRareInheritedData::operator==(const StyleRareInheritedData& o) const
{
    return m_textStrokeColor == o.m_textStrokeColor
        && m_textStrokeWidth == o.m_textStrokeWidth
        && m_textFillColor == o.m_textFillColor
        && m_textEmphasisColor == o.m_textEmphasisColor
        && shadowDataEquivalent(o)
        && m_highlight == o.m_highlight
        && cursorDataEquivalent(m_cursorData.get(), o.m_cursorData.get())
        && m_indent == o.m_indent
        && m_effectiveZoom == o.m_effectiveZoom
        && m_widows == o.m_widows
        && m_orphans == o.m_orphans
        && m_textSecurity == o.m_textSecurity
        && m_userModify == o.m_userModify
        && m_wordBreak == o.m_wordBreak
        && m_wordWrap == o.m_wordWrap
        && m_nbspMode == o.m_nbspMode
        && m_khtmlLineBreak == o.m_khtmlLineBreak
        && m_textSizeAdjust == o.m_textSizeAdjust
        && m_resize == o.m_resize
        && m_userSelect == o.m_userSelect
        && m_colorSpace == o.m_colorSpace
        && m_speak == o.m_speak
        && m_hyphens == o.m_hyphens
        && m_hyphenationLimitBefore == o.m_hyphenationLimitBefore
        && m_hyphenationLimitAfter == o.m_hyphenationLimitAfter
        && m_textEmphasisFill == o.m_textEmphasisFill
        && m_textEmphasisMark == o.m_textEmphasisMark
        && m_textEmphasisPosition == o.m_textEmphasisPosition
        && m_lineBoxContain == o.m_lineBoxContain
        && m_hyphenationString == o.m_hyphenationString
        && m_locale == o.m_locale
        && m_textEmphasisCustomMark == o.m_textEmphasisCustomMark
        && *m_quotes == *o.m_quotes;
}

bool StyleRareInheritedData::shadowDataEquivalent(const StyleRareInheritedData& o) const
{
    if ((!m_textShadow && o.m_textShadow) || (m_textShadow && !o.m_textShadow))
        return false;
    if (m_textShadow && o.m_textShadow && (*m_textShadow != *o.m_textShadow))
        return false;
    return true;
}

} // namespace WebCore
