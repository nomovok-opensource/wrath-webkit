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
#include "StyleFlexibleBoxData.h"

#include "RenderStyle.h"

namespace WebCore {

StyleFlexibleBoxData::StyleFlexibleBoxData()
    : m_flex(RenderStyle::initialBoxFlex())
    , m_flexGroup(RenderStyle::initialBoxFlexGroup())
    , m_ordinalGroup(RenderStyle::initialBoxOrdinalGroup())
    , m_align(RenderStyle::initialBoxAlign())
    , m_pack(RenderStyle::initialBoxPack())
    , m_orient(RenderStyle::initialBoxOrient())
    , m_lines(RenderStyle::initialBoxLines())
{
}

StyleFlexibleBoxData::StyleFlexibleBoxData(const StyleFlexibleBoxData& o)
    : RefCounted<StyleFlexibleBoxData>()
    , m_flex(o.m_flex)
    , m_flexGroup(o.m_flexGroup)
    , m_ordinalGroup(o.m_ordinalGroup)
    , m_align(o.m_align)
    , m_pack(o.m_pack)
    , m_orient(o.m_orient)
    , m_lines(o.m_lines)
{
}

bool StyleFlexibleBoxData::operator==(const StyleFlexibleBoxData& o) const
{
    return m_flex == o.m_flex && m_flexGroup == o.m_flexGroup &&
           m_ordinalGroup == o.m_ordinalGroup && m_align == o.m_align &&
           m_pack == o.m_pack && m_orient == o.m_orient && m_lines == o.m_lines;
}

} // namespace WebCore
