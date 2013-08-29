/*
 * Copyright (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
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

#include "RenderStyle.h"
#include "StyleImage.h"

namespace WebCore {

StyleInheritedData::StyleInheritedData()
    : m_lineHeight(RenderStyle::initialLineHeight())
    , m_listStyleImage(RenderStyle::initialListStyleImage())
    , m_color(RenderStyle::initialColor())
    , m_horizontalBorderSpacing(RenderStyle::initialHorizontalBorderSpacing())
    , m_verticalBorderSpacing(RenderStyle::initialVerticalBorderSpacing())
{
}

StyleInheritedData::~StyleInheritedData()
{
}

StyleInheritedData::StyleInheritedData(const StyleInheritedData& o)
    : RefCounted<StyleInheritedData>()
    , m_lineHeight(o.m_lineHeight)
    , m_listStyleImage(o.m_listStyleImage)
    , m_font(o.m_font)
    , m_color(o.m_color)
    , m_horizontalBorderSpacing(o.m_horizontalBorderSpacing)
    , m_verticalBorderSpacing(o.m_verticalBorderSpacing)
{
}

bool StyleInheritedData::operator==(const StyleInheritedData& o) const
{
    return
        m_lineHeight == o.m_lineHeight &&
        StyleImage::imagesEquivalent(m_listStyleImage.get(), o.m_listStyleImage.get()) &&
        m_font == o.m_font &&
        m_color == o.m_color &&
        m_horizontalBorderSpacing == o.m_horizontalBorderSpacing &&
        m_verticalBorderSpacing == o.m_verticalBorderSpacing;
}

} // namespace WebCore
