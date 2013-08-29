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
#include "StyleBackgroundData.h"

#include "RenderStyle.h"

namespace WebCore {

StyleMarqueeData::StyleMarqueeData()
    : m_increment(RenderStyle::initialMarqueeIncrement())
    , m_speed(RenderStyle::initialMarqueeSpeed())
    , m_loops(RenderStyle::initialMarqueeLoopCount())
    , m_behavior(RenderStyle::initialMarqueeBehavior())
    , m_direction(RenderStyle::initialMarqueeDirection())
{
}

StyleMarqueeData::StyleMarqueeData(const StyleMarqueeData& o)
    : RefCounted<StyleMarqueeData>()
    , m_increment(o.m_increment)
    , m_speed(o.m_speed)
    , m_loops(o.m_loops)
    , m_behavior(o.m_behavior)
    , m_direction(o.m_direction) 
{
}

bool StyleMarqueeData::operator==(const StyleMarqueeData& o) const
{
    return m_increment == o.m_increment && m_speed == o.m_speed && m_direction == o.m_direction &&
           m_behavior == o.m_behavior && m_loops == o.m_loops;
}

} // namespace WebCore
