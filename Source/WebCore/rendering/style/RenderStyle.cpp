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
#include "RenderStyle.h"

#include "ContentData.h"
#include "CursorList.h"
#include "CSSPropertyNames.h"
#include "CSSStyleSelector.h"
#include "FontSelector.h"
#include "QuotesData.h"
#include "RenderArena.h"
#include "RenderObject.h"
#include "ScaleTransformOperation.h"
#include "ShadowData.h"
#include "StyleImage.h"
#include <wtf/StdLibExtras.h>
#include <algorithm>

using namespace std;

namespace WebCore {

inline RenderStyle* defaultStyle()
{
    static RenderStyle* s_defaultStyle = RenderStyle::createDefaultStyle().releaseRef();
    return s_defaultStyle;
}

PassRefPtr<RenderStyle> RenderStyle::create()
{
    return adoptRef(new RenderStyle());
}

PassRefPtr<RenderStyle> RenderStyle::createDefaultStyle()
{
    return adoptRef(new RenderStyle(true));
}

PassRefPtr<RenderStyle> RenderStyle::createAnonymousStyle(const RenderStyle* parentStyle)
{
    RefPtr<RenderStyle> newStyle = RenderStyle::create();
    newStyle->inheritFrom(parentStyle);
    newStyle->inheritUnicodeBidiFrom(parentStyle);
    return newStyle;
}

PassRefPtr<RenderStyle> RenderStyle::clone(const RenderStyle* other)
{
    return adoptRef(new RenderStyle(*other));
}

ALWAYS_INLINE RenderStyle::RenderStyle()
    : m_affectedByAttributeSelectors(false)
    , m_unique(false)
    , m_affectedByEmpty(false)
    , m_emptyState(false)
    , m_childrenAffectedByFirstChildRules(false)
    , m_childrenAffectedByLastChildRules(false)
    , m_childrenAffectedByDirectAdjacentRules(false)
    , m_childrenAffectedByForwardPositionalRules(false)
    , m_childrenAffectedByBackwardPositionalRules(false)
    , m_firstChildState(false)
    , m_lastChildState(false)
    , m_childIndex(0)
    , m_box(defaultStyle()->m_box)
    , m_visual(defaultStyle()->m_visual)
    , m_background(defaultStyle()->m_background)
    , m_surround(defaultStyle()->m_surround)
    , m_rareNonInheritedData(defaultStyle()->m_rareNonInheritedData)
    , m_rareInheritedData(defaultStyle()->m_rareInheritedData)
    , m_inherited(defaultStyle()->m_inherited)
#if ENABLE(SVG)
    , m_svgStyle(defaultStyle()->m_svgStyle)
#endif
{
    setBitDefaults(); // Would it be faster to copy this from the default style?
}

ALWAYS_INLINE RenderStyle::RenderStyle(bool)
    : m_affectedByAttributeSelectors(false)
    , m_unique(false)
    , m_affectedByEmpty(false)
    , m_emptyState(false)
    , m_childrenAffectedByFirstChildRules(false)
    , m_childrenAffectedByLastChildRules(false)
    , m_childrenAffectedByDirectAdjacentRules(false)
    , m_childrenAffectedByForwardPositionalRules(false)
    , m_childrenAffectedByBackwardPositionalRules(false)
    , m_firstChildState(false)
    , m_lastChildState(false)
    , m_childIndex(0)
{
    setBitDefaults();

    m_box.init();
    m_visual.init();
    m_background.init();
    m_surround.init();
    m_rareNonInheritedData.init();
    m_rareNonInheritedData.access()->m_flexibleBox.init();
    m_rareNonInheritedData.access()->m_marquee.init();
    m_rareNonInheritedData.access()->m_multiCol.init();
    m_rareNonInheritedData.access()->m_transform.init();
    m_rareInheritedData.init();
    m_inherited.init();

#if ENABLE(SVG)
    m_svgStyle.init();
#endif
}

ALWAYS_INLINE RenderStyle::RenderStyle(const RenderStyle& o)
    : RefCounted<RenderStyle>()
    , m_affectedByAttributeSelectors(false)
    , m_unique(false)
    , m_affectedByEmpty(false)
    , m_emptyState(false)
    , m_childrenAffectedByFirstChildRules(false)
    , m_childrenAffectedByLastChildRules(false)
    , m_childrenAffectedByDirectAdjacentRules(false)
    , m_childrenAffectedByForwardPositionalRules(false)
    , m_childrenAffectedByBackwardPositionalRules(false)
    , m_firstChildState(false)
    , m_lastChildState(false)
    , m_childIndex(0)
    , m_box(o.m_box)
    , m_visual(o.m_visual)
    , m_background(o.m_background)
    , m_surround(o.m_surround)
    , m_rareNonInheritedData(o.m_rareNonInheritedData)
    , m_rareInheritedData(o.m_rareInheritedData)
    , m_inherited(o.m_inherited)
#if ENABLE(SVG)
    , m_svgStyle(o.m_svgStyle)
#endif
    , m_inheritedFlags(o.m_inheritedFlags)
    , m_noninheritedFlags(o.m_noninheritedFlags)
{
}

void RenderStyle::inheritFrom(const RenderStyle* inheritParent)
{
    m_rareInheritedData = inheritParent->m_rareInheritedData;
    m_inherited = inheritParent->m_inherited;
    m_inheritedFlags = inheritParent->m_inheritedFlags;
#if ENABLE(SVG)
    if (m_svgStyle != inheritParent->m_svgStyle)
        m_svgStyle.access()->inheritFrom(inheritParent->m_svgStyle.get());
#endif
}

RenderStyle::~RenderStyle()
{
}

bool RenderStyle::operator==(const RenderStyle& o) const
{
    // compare everything except the pseudoStyle pointer
    return m_inheritedFlags == o.m_inheritedFlags
        && m_noninheritedFlags == o.m_noninheritedFlags
        && m_box == o.m_box
        && m_visual == o.m_visual
        && m_background == o.m_background
        && m_surround == o.m_surround
        && m_rareNonInheritedData == o.m_rareNonInheritedData
        && m_rareInheritedData == o.m_rareInheritedData
        && m_inherited == o.m_inherited
#if ENABLE(SVG)
        && m_svgStyle == o.m_svgStyle
#endif
            ;
}

bool RenderStyle::isStyleAvailable() const
{
    return this != CSSStyleSelector::styleNotYetAvailable();
}

static inline int pseudoBit(PseudoId pseudo)
{
    return 1 << (pseudo - 1);
}

bool RenderStyle::hasAnyPublicPseudoStyles() const
{
    return PUBLIC_PSEUDOID_MASK & m_noninheritedFlags.m_pseudoBits;
}

bool RenderStyle::hasPseudoStyle(PseudoId pseudo) const
{
    ASSERT(pseudo > NOPSEUDO);
    ASSERT(pseudo < FIRST_INTERNAL_PSEUDOID);
    return pseudoBit(pseudo) & m_noninheritedFlags.m_pseudoBits;
}

void RenderStyle::setHasPseudoStyle(PseudoId pseudo)
{
    ASSERT(pseudo > NOPSEUDO);
    ASSERT(pseudo < FIRST_INTERNAL_PSEUDOID);
    if(hasPseudoStyle(pseudo)) return;
    m_noninheritedFlags.m_pseudoBits |= pseudoBit(pseudo);
#if USE(WRATH)
    m_signal(E_noninheritedFlags_pseudoBits);
#endif
}

RenderStyle* RenderStyle::getCachedPseudoStyle(PseudoId pid) const
{
    ASSERT(styleType() != VISITED_LINK);

    if (!m_cachedPseudoStyles || !m_cachedPseudoStyles->size())
        return 0;

    if (styleType() != NOPSEUDO) {
        if (pid == VISITED_LINK)
            return m_cachedPseudoStyles->at(0)->styleType() == VISITED_LINK ? m_cachedPseudoStyles->at(0).get() : 0;
        return 0;
    }

    for (size_t i = 0; i < m_cachedPseudoStyles->size(); ++i) {
        RenderStyle* pseudoStyle = m_cachedPseudoStyles->at(i).get();
        if (pseudoStyle->styleType() == pid)
            return pseudoStyle;
    }

    return 0;
}

RenderStyle* RenderStyle::addCachedPseudoStyle(PassRefPtr<RenderStyle> pseudo)
{
    if (!pseudo)
        return 0;
    
    RenderStyle* result = pseudo.get();

    if (!m_cachedPseudoStyles)
        m_cachedPseudoStyles = adoptPtr(new PseudoStyleCache);

    m_cachedPseudoStyles->append(pseudo);

    return result;
}

void RenderStyle::removeCachedPseudoStyle(PseudoId pid)
{
    if (!m_cachedPseudoStyles)
        return;
    for (size_t i = 0; i < m_cachedPseudoStyles->size(); ++i) {
        RenderStyle* pseudoStyle = m_cachedPseudoStyles->at(i).get();
        if (pseudoStyle->styleType() == pid) {
            m_cachedPseudoStyles->remove(i);
            return;
        }
    }
}

bool RenderStyle::inheritedNotEqual(const RenderStyle* other) const
{
    return m_inheritedFlags != other->m_inheritedFlags
           || m_inherited != other->m_inherited
#if ENABLE(SVG)
           || m_svgStyle->inheritedNotEqual(other->m_svgStyle.get())
#endif
           || m_rareInheritedData != other->m_rareInheritedData;
}

static bool positionedObjectMoved(const LengthBox& a, const LengthBox& b)
{
    // If any unit types are different, then we can't guarantee
    // that this was just a movement.
    if (a.left().type() != b.left().type()
        || a.right().type() != b.right().type()
        || a.top().type() != b.top().type()
        || a.bottom().type() != b.bottom().type())
        return false;

    // Only one unit can be non-auto in the horizontal direction and
    // in the vertical direction.  Otherwise the adjustment of values
    // is changing the size of the box.
    if (!a.left().isIntrinsicOrAuto() && !a.right().isIntrinsicOrAuto())
        return false;
    if (!a.top().isIntrinsicOrAuto() && !a.bottom().isIntrinsicOrAuto())
        return false;

    // One of the units is fixed or percent in both directions and stayed
    // that way in the new style.  Therefore all we are doing is moving.
    return true;
}

StyleDifference RenderStyle::diff(const RenderStyle* other, unsigned& changedContextSensitiveProperties) const
{
    changedContextSensitiveProperties = ContextSensitivePropertyNone;

#if ENABLE(SVG)
    StyleDifference svgChange = StyleDifferenceEqual;
    if (m_svgStyle != other->m_svgStyle) {
        svgChange = m_svgStyle->diff(other->m_svgStyle.get());
        if (svgChange == StyleDifferenceLayout)
            return svgChange;
    }
#endif

    if (m_box->width() != other->m_box->width()
        || m_box->minWidth() != other->m_box->minWidth()
        || m_box->maxWidth() != other->m_box->maxWidth()
        || m_box->height() != other->m_box->height()
        || m_box->minHeight() != other->m_box->minHeight()
        || m_box->maxHeight() != other->m_box->maxHeight())
        return StyleDifferenceLayout;

    if (m_box->verticalAlign() != other->m_box->verticalAlign() || m_noninheritedFlags.m_verticalAlign != other->m_noninheritedFlags.m_verticalAlign)
        return StyleDifferenceLayout;

    if (m_box->boxSizing() != other->m_box->boxSizing())
        return StyleDifferenceLayout;

    if (m_surround->m_margin != other->m_surround->m_margin)
        return StyleDifferenceLayout;

    if (m_surround->m_padding != other->m_surround->m_padding)
        return StyleDifferenceLayout;

    if (m_rareNonInheritedData.get() != other->m_rareNonInheritedData.get()) {
        if (m_rareNonInheritedData->m_appearance != other->m_rareNonInheritedData->m_appearance 
            || m_rareNonInheritedData->m_marginBeforeCollapse != other->m_rareNonInheritedData->m_marginBeforeCollapse
            || m_rareNonInheritedData->m_marginAfterCollapse != other->m_rareNonInheritedData->m_marginAfterCollapse
            || m_rareNonInheritedData->m_lineClamp != other->m_rareNonInheritedData->m_lineClamp
            || m_rareNonInheritedData->m_textOverflow != other->m_rareNonInheritedData->m_textOverflow)
            return StyleDifferenceLayout;

        if (m_rareNonInheritedData->m_flexibleBox.get() != other->m_rareNonInheritedData->m_flexibleBox.get()
            && *m_rareNonInheritedData->m_flexibleBox.get() != *other->m_rareNonInheritedData->m_flexibleBox.get())
            return StyleDifferenceLayout;

        // FIXME: We should add an optimized form of layout that just recomputes visual overflow.
        if (!m_rareNonInheritedData->shadowDataEquivalent(*other->m_rareNonInheritedData.get()))
            return StyleDifferenceLayout;

        if (!m_rareNonInheritedData->reflectionDataEquivalent(*other->m_rareNonInheritedData.get()))
            return StyleDifferenceLayout;

        if (m_rareNonInheritedData->m_multiCol.get() != other->m_rareNonInheritedData->m_multiCol.get()
            && *m_rareNonInheritedData->m_multiCol.get() != *other->m_rareNonInheritedData->m_multiCol.get())
            return StyleDifferenceLayout;

        if (m_rareNonInheritedData->m_transform.get() != other->m_rareNonInheritedData->m_transform.get()
            && *m_rareNonInheritedData->m_transform.get() != *other->m_rareNonInheritedData->m_transform.get()) {
#if USE(ACCELERATED_COMPOSITING)
            changedContextSensitiveProperties |= ContextSensitivePropertyTransform;
            // Don't return; keep looking for another change
#else
            return StyleDifferenceLayout;
#endif
        }

#if !USE(ACCELERATED_COMPOSITING)
        if (m_rareNonInheritedData.get() != other->m_rareNonInheritedData.get()) {
            if (m_rareNonInheritedData->m_transformStyle3D != other->m_rareNonInheritedData->m_transformStyle3D
                || m_rareNonInheritedData->m_backfaceVisibility != other->m_rareNonInheritedData->m_backfaceVisibility
                || m_rareNonInheritedData->m_perspective != other->m_rareNonInheritedData->m_perspective
                || m_rareNonInheritedData->m_perspectiveOriginX != other->m_rareNonInheritedData->m_perspectiveOriginX
                || m_rareNonInheritedData->m_perspectiveOriginY != other->m_rareNonInheritedData->m_perspectiveOriginY)
                return StyleDifferenceLayout;
        }
#endif

#if ENABLE(DASHBOARD_SUPPORT)
        // If regions change, trigger a relayout to re-calc regions.
        if (m_rareNonInheritedData->m_dashboardRegions != other->m_rareNonInheritedData->m_dashboardRegions)
            return StyleDifferenceLayout;
#endif
    }

    if (m_rareInheritedData.get() != other->m_rareInheritedData.get()) {
        if (m_rareInheritedData->m_highlight != other->m_rareInheritedData->m_highlight
            || m_rareInheritedData->m_indent != other->m_rareInheritedData->m_indent
            || m_rareInheritedData->m_effectiveZoom != other->m_rareInheritedData->m_effectiveZoom
            || m_rareInheritedData->m_textSizeAdjust != other->m_rareInheritedData->m_textSizeAdjust
            || m_rareInheritedData->m_wordBreak != other->m_rareInheritedData->m_wordBreak
            || m_rareInheritedData->m_wordWrap != other->m_rareInheritedData->m_wordWrap
            || m_rareInheritedData->m_nbspMode != other->m_rareInheritedData->m_nbspMode
            || m_rareInheritedData->m_khtmlLineBreak != other->m_rareInheritedData->m_khtmlLineBreak
            || m_rareInheritedData->m_textSecurity != other->m_rareInheritedData->m_textSecurity
            || m_rareInheritedData->m_hyphens != other->m_rareInheritedData->m_hyphens
            || m_rareInheritedData->m_hyphenationLimitBefore != other->m_rareInheritedData->m_hyphenationLimitBefore
            || m_rareInheritedData->m_hyphenationLimitAfter != other->m_rareInheritedData->m_hyphenationLimitAfter
            || m_rareInheritedData->m_hyphenationString != other->m_rareInheritedData->m_hyphenationString
            || m_rareInheritedData->m_locale != other->m_rareInheritedData->m_locale
            || m_rareInheritedData->m_textEmphasisMark != other->m_rareInheritedData->m_textEmphasisMark
            || m_rareInheritedData->m_textEmphasisPosition != other->m_rareInheritedData->m_textEmphasisPosition
            || m_rareInheritedData->m_textEmphasisCustomMark != other->m_rareInheritedData->m_textEmphasisCustomMark
            || m_rareInheritedData->m_lineBoxContain != other->m_rareInheritedData->m_lineBoxContain)
            return StyleDifferenceLayout;

        if (!m_rareInheritedData->shadowDataEquivalent(*other->m_rareInheritedData.get()))
            return StyleDifferenceLayout;

        if (textStrokeWidth() != other->textStrokeWidth())
            return StyleDifferenceLayout;
    }

    if (m_inherited->m_lineHeight != other->m_inherited->m_lineHeight
        || m_inherited->m_listStyleImage != other->m_inherited->m_listStyleImage
        || m_inherited->m_font != other->m_inherited->m_font
        || m_inherited->m_horizontalBorderSpacing != other->m_inherited->m_horizontalBorderSpacing
        || m_inherited->m_verticalBorderSpacing != other->m_inherited->m_verticalBorderSpacing
        || m_inheritedFlags.m_boxDirection != other->m_inheritedFlags.m_boxDirection
        || m_inheritedFlags.m_visuallyOrdered != other->m_inheritedFlags.m_visuallyOrdered
        || m_noninheritedFlags.m_position != other->m_noninheritedFlags.m_position
        || m_noninheritedFlags.m_floating != other->m_noninheritedFlags.m_floating
        || m_noninheritedFlags.m_originalDisplay != other->m_noninheritedFlags.m_originalDisplay)
        return StyleDifferenceLayout;


    if (((int)m_noninheritedFlags.m_effectiveDisplay) >= TABLE) {
        if (m_inheritedFlags.m_borderCollapse != other->m_inheritedFlags.m_borderCollapse
            || m_inheritedFlags.m_emptyCells != other->m_inheritedFlags.m_emptyCells
            || m_inheritedFlags.m_captionSide != other->m_inheritedFlags.m_captionSide
            || m_noninheritedFlags.m_tableLayout != other->m_noninheritedFlags.m_tableLayout)
            return StyleDifferenceLayout;

        // In the collapsing border model, 'hidden' suppresses other borders, while 'none'
        // does not, so these style differences can be width differences.
        if (m_inheritedFlags.m_borderCollapse
            && ((borderTopStyle() == BHIDDEN && other->borderTopStyle() == BNONE)
                || (borderTopStyle() == BNONE && other->borderTopStyle() == BHIDDEN)
                || (borderBottomStyle() == BHIDDEN && other->borderBottomStyle() == BNONE)
                || (borderBottomStyle() == BNONE && other->borderBottomStyle() == BHIDDEN)
                || (borderLeftStyle() == BHIDDEN && other->borderLeftStyle() == BNONE)
                || (borderLeftStyle() == BNONE && other->borderLeftStyle() == BHIDDEN)
                || (borderRightStyle() == BHIDDEN && other->borderRightStyle() == BNONE)
                || (borderRightStyle() == BNONE && other->borderRightStyle() == BHIDDEN)))
            return StyleDifferenceLayout;
    }

    if (m_noninheritedFlags.m_effectiveDisplay == LIST_ITEM) {
        if (m_inheritedFlags.m_listStyleType != other->m_inheritedFlags.m_listStyleType
            || m_inheritedFlags.m_listStylePosition != other->m_inheritedFlags.m_listStylePosition)
            return StyleDifferenceLayout;
    }

    if (m_inheritedFlags.m_textAlign != other->m_inheritedFlags.m_textAlign
        || m_inheritedFlags.m_textTransform != other->m_inheritedFlags.m_textTransform
        || m_inheritedFlags.m_direction != other->m_inheritedFlags.m_direction
        || m_inheritedFlags.m_whiteSpace != other->m_inheritedFlags.m_whiteSpace
        || m_noninheritedFlags.m_clear != other->m_noninheritedFlags.m_clear
        || m_noninheritedFlags.m_unicodeBidi != other->m_noninheritedFlags.m_unicodeBidi)
        return StyleDifferenceLayout;

    // Check block flow direction.
    if (m_inheritedFlags.m_writingMode != other->m_inheritedFlags.m_writingMode)
        return StyleDifferenceLayout;

    // Check text combine mode.
    if (m_rareNonInheritedData->m_textCombine != other->m_rareNonInheritedData->m_textCombine)
        return StyleDifferenceLayout;

    // Overflow returns a layout hint.
    if (m_noninheritedFlags.m_overflowX != other->m_noninheritedFlags.m_overflowX
        || m_noninheritedFlags.m_overflowY != other->m_noninheritedFlags.m_overflowY)
        return StyleDifferenceLayout;

    // If our border widths change, then we need to layout.  Other changes to borders
    // only necessitate a repaint.
    if (borderLeftWidth() != other->borderLeftWidth()
        || borderTopWidth() != other->borderTopWidth()
        || borderBottomWidth() != other->borderBottomWidth()
        || borderRightWidth() != other->borderRightWidth())
        return StyleDifferenceLayout;

    // If the counter directives change, trigger a relayout to re-calculate counter values and rebuild the counter node tree.
    const CounterDirectiveMap* mapA = m_rareNonInheritedData->m_counterDirectives.get();
    const CounterDirectiveMap* mapB = other->m_rareNonInheritedData->m_counterDirectives.get();
    if (!(mapA == mapB || (mapA && mapB && *mapA == *mapB)))
        return StyleDifferenceLayout;
    if (m_rareNonInheritedData->m_counterIncrement != other->m_rareNonInheritedData->m_counterIncrement
        || m_rareNonInheritedData->m_counterReset != other->m_rareNonInheritedData->m_counterReset)
        return StyleDifferenceLayout;

    if ((visibility() == COLLAPSE) != (other->visibility() == COLLAPSE))
        return StyleDifferenceLayout;

    if ((m_rareNonInheritedData->m_opacity == 1 && other->m_rareNonInheritedData->m_opacity < 1)
        || (m_rareNonInheritedData->m_opacity < 1 && other->m_rareNonInheritedData->m_opacity == 1)) {
        // FIXME: We would like to use SimplifiedLayout here, but we can't quite do that yet.
        // We need to make sure SimplifiedLayout can operate correctly on RenderInlines (we will need
        // to add a selfNeedsSimplifiedLayout bit in order to not get confused and taint every line).
        // In addition we need to solve the floating object issue when layers come and go. Right now
        // a full layout is necessary to keep floating object lists sane.
        return StyleDifferenceLayout;
    }

#if ENABLE(SVG)
    // SVGRenderStyle::diff() might have returned StyleDifferenceRepaint, eg. if fill changes.
    // If eg. the font-size changed at the same time, we're not allowed to return StyleDifferenceRepaint,
    // but have to return StyleDifferenceLayout, that's why  this if branch comes after all branches
    // that are relevant for SVG and might return StyleDifferenceLayout.
    if (svgChange != StyleDifferenceEqual)
        return svgChange;
#endif

    // Make sure these left/top/right/bottom checks stay below all layout checks and above
    // all visible checks.
    if (position() != StaticPosition) {
        if (m_surround->m_offset != other->m_surround->m_offset) {
             // Optimize for the case where a positioned layer is moving but not changing size.
            if (position() == AbsolutePosition && positionedObjectMoved(m_surround->m_offset, other->m_surround->m_offset))
                return StyleDifferenceLayoutPositionedMovementOnly;

            // FIXME: We would like to use SimplifiedLayout for relative positioning, but we can't quite do that yet.
            // We need to make sure SimplifiedLayout can operate correctly on RenderInlines (we will need
            // to add a selfNeedsSimplifiedLayout bit in order to not get confused and taint every line).
            return StyleDifferenceLayout;
        } else if (m_box->zIndex() != other->m_box->zIndex() || m_box->hasAutoZIndex() != other->m_box->hasAutoZIndex()
                 || m_visual->m_clip != other->m_visual->m_clip || m_visual->m_hasClip != other->m_visual->m_hasClip)
            return StyleDifferenceRepaintLayer;
    }

    if (m_rareNonInheritedData->m_opacity != other->m_rareNonInheritedData->m_opacity) {
#if USE(ACCELERATED_COMPOSITING)
        changedContextSensitiveProperties |= ContextSensitivePropertyOpacity;
        // Don't return; keep looking for another change.
#else
        return StyleDifferenceRepaintLayer;
#endif
    }

    if (m_rareNonInheritedData->m_mask != other->m_rareNonInheritedData->m_mask
        || m_rareNonInheritedData->m_maskBoxImage != other->m_rareNonInheritedData->m_maskBoxImage)
        return StyleDifferenceRepaintLayer;

    if (m_inherited->m_color != other->m_inherited->m_color
        || m_inheritedFlags.m_visibility != other->m_inheritedFlags.m_visibility
        || m_inheritedFlags.m_textDecorations != other->m_inheritedFlags.m_textDecorations
        || m_inheritedFlags.m_forceBackgroundsToWhite != other->m_inheritedFlags.m_forceBackgroundsToWhite
        || m_inheritedFlags.m_insideLink != other->m_inheritedFlags.m_insideLink
        || m_surround->m_border != other->m_surround->m_border
        || *m_background.get() != *other->m_background.get()
        || m_visual->m_textDecoration != other->m_visual->m_textDecoration
        || m_rareInheritedData->m_userModify != other->m_rareInheritedData->m_userModify
        || m_rareInheritedData->m_userSelect != other->m_rareInheritedData->m_userSelect
        || m_rareNonInheritedData->m_userDrag != other->m_rareNonInheritedData->m_userDrag
        || m_rareNonInheritedData->m_borderFit != other->m_rareNonInheritedData->m_borderFit
        || m_rareInheritedData->m_textFillColor != other->m_rareInheritedData->m_textFillColor
        || m_rareInheritedData->m_textStrokeColor != other->m_rareInheritedData->m_textStrokeColor
        || m_rareInheritedData->m_textEmphasisColor != other->m_rareInheritedData->m_textEmphasisColor
        || m_rareInheritedData->m_textEmphasisFill != other->m_rareInheritedData->m_textEmphasisFill)
        return StyleDifferenceRepaint;

#if USE(ACCELERATED_COMPOSITING)
    if (m_rareNonInheritedData.get() != other->m_rareNonInheritedData.get()) {
        if (m_rareNonInheritedData->m_transformStyle3D != other->m_rareNonInheritedData->m_transformStyle3D
            || m_rareNonInheritedData->m_backfaceVisibility != other->m_rareNonInheritedData->m_backfaceVisibility
            || m_rareNonInheritedData->m_perspective != other->m_rareNonInheritedData->m_perspective
            || m_rareNonInheritedData->m_perspectiveOriginX != other->m_rareNonInheritedData->m_perspectiveOriginX
            || m_rareNonInheritedData->m_perspectiveOriginY != other->m_rareNonInheritedData->m_perspectiveOriginY)
            return StyleDifferenceRecompositeLayer;
    }
#endif

    // Cursors are not checked, since they will be set appropriately in response to mouse events,
    // so they don't need to cause any repaint or layout.

    // Animations don't need to be checked either.  We always set the new style on the RenderObject, so we will get a chance to fire off
    // the resulting transition properly.
    return StyleDifferenceEqual;
}

void RenderStyle::setClip(Length top, Length right, Length bottom, Length left)
{
    setClip(LengthBox(top, right, bottom, left));
}

void RenderStyle::addCursor(PassRefPtr<StyleImage> image, const IntPoint& hotSpot)
{
    if (!m_rareInheritedData.access()->m_cursorData)
        m_rareInheritedData.access()->m_cursorData = CursorList::create();
    m_rareInheritedData.access()->m_cursorData->append(CursorData(image, hotSpot));
}

void RenderStyle::setCursorList(PassRefPtr<CursorList> other)
{
    m_rareInheritedData.access()->m_cursorData = other;
#if USE(WRATH)
    m_signal(E_rareInheritedData_cursorData);
#endif
}

void RenderStyle::setQuotes(PassRefPtr<QuotesData> q)
{
    if (*m_rareInheritedData->m_quotes.get() == *q.get())
        return;
    m_rareInheritedData.access()->m_quotes = q;
#if USE(WRATH)
    m_signal(E_rareInheritedData_quotes);
#endif
}

void RenderStyle::clearCursorList()
{
    if (m_rareInheritedData->m_cursorData)
        m_rareInheritedData.access()->m_cursorData = 0;
#if USE(WRATH)
    m_signal(E_rareInheritedData_cursorData);
#endif
}

void RenderStyle::clearContent()
{
    if (m_rareNonInheritedData->m_content)
        m_rareNonInheritedData->m_content->clear();
#if USE(WRATH)
    m_signal(E_rareInheritedData_content);
#endif
}

ContentData* RenderStyle::prepareToSetContent(StringImpl* string, bool add)
{
    OwnPtr<ContentData>& content = m_rareNonInheritedData.access()->m_content;
    ContentData* lastContent = content.get();
    while (lastContent && lastContent->next())
        lastContent = lastContent->next();

    if (string && add && lastContent && lastContent->isText()) {
        // Augment the existing string and share the existing ContentData node.
        String newText = lastContent->text();
        newText.append(string);
        lastContent->setText(newText.impl());
        return 0;
    }

    bool reuseContent = !add;
    OwnPtr<ContentData> newContentData;
    if (reuseContent && content) {
        content->clear();
        newContentData = content.release();
    } else
        newContentData = adoptPtr(new ContentData);

    ContentData* result = newContentData.get();

    if (lastContent && !reuseContent)
        lastContent->setNext(newContentData.release());
    else
        content = newContentData.release();

    return result;
}

void RenderStyle::setContent(PassRefPtr<StyleImage> image, bool add)
{
    if (!image)
        return;
    prepareToSetContent(0, add)->setImage(image);
}

void RenderStyle::setContent(PassRefPtr<StringImpl> string, bool add)
{
    if (!string)
        return;
    if (ContentData* data = prepareToSetContent(string.get(), add))
        data->setText(string);
}

void RenderStyle::setContent(PassOwnPtr<CounterContent> counter, bool add)
{
    if (!counter)
        return;
    prepareToSetContent(0, add)->setCounter(counter);
}

void RenderStyle::setContent(QuoteType quote, bool add)
{
    prepareToSetContent(0, add)->setQuote(quote);
}

void RenderStyle::applyTransform(TransformationMatrix& transform, const IntSize& borderBoxSize, ApplyTransformOrigin applyOrigin) const
{
    // transform-origin brackets the transform with translate operations.
    // Optimize for the case where the only transform is a translation, since the transform-origin is irrelevant
    // in that case.
    bool applyTransformOrigin = false;
    unsigned s = m_rareNonInheritedData->m_transform->m_operations.operations().size();
    unsigned i;
    if (applyOrigin == IncludeTransformOrigin) {
        for (i = 0; i < s; i++) {
            TransformOperation::OperationType type = m_rareNonInheritedData->m_transform->m_operations.operations()[i]->getOperationType();
            if (type != TransformOperation::TRANSLATE_X
                    && type != TransformOperation::TRANSLATE_Y
                    && type != TransformOperation::TRANSLATE 
                    && type != TransformOperation::TRANSLATE_Z
                    && type != TransformOperation::TRANSLATE_3D
                    ) {
                applyTransformOrigin = true;
                break;
            }
        }
    }

    if (applyTransformOrigin) {
        transform.translate3d(transformOriginX().calcFloatValue(borderBoxSize.width()), transformOriginY().calcFloatValue(borderBoxSize.height()), transformOriginZ());
    }

    for (i = 0; i < s; i++)
        m_rareNonInheritedData->m_transform->m_operations.operations()[i]->apply(transform, borderBoxSize);

    if (applyTransformOrigin) {
        transform.translate3d(-transformOriginX().calcFloatValue(borderBoxSize.width()), -transformOriginY().calcFloatValue(borderBoxSize.height()), -transformOriginZ());
    }
}

void RenderStyle::setPageScaleTransform(float scale)
{
    if (scale == 1)
        return;
    TransformOperations transform;
    transform.operations().append(ScaleTransformOperation::create(scale, scale, ScaleTransformOperation::SCALE));
    setTransform(transform);
    setTransformOriginX(Length(0, Fixed));
    setTransformOriginY(Length(0, Fixed));
}

void RenderStyle::setTextShadow(PassOwnPtr<ShadowData> shadowData, bool add)
{
    ASSERT(!shadowData || (!shadowData->spread() && shadowData->style() == Normal));

    StyleRareInheritedData* rareData = m_rareInheritedData.access();
    if (!add) {
        rareData->m_textShadow = shadowData;
        return;
    }

    shadowData->setNext(rareData->m_textShadow.release());
    rareData->m_textShadow = shadowData;
#if USE(WRATH)
    m_signal(E_rareInheritedData_textShadow);
#endif
}

void RenderStyle::setBoxShadow(PassOwnPtr<ShadowData> shadowData, bool add)
{
    StyleRareNonInheritedData* rareData = m_rareNonInheritedData.access();
    if (!add) {
        rareData->m_boxShadow = shadowData;
        return;
    }

    shadowData->setNext(rareData->m_boxShadow.release());
    rareData->m_boxShadow = shadowData;
#if USE(WRATH)
    m_signal(E_rareInheritedData_boxShadow);
#endif
}

static RoundedIntRect::Radii calcRadiiFor(const BorderData& border, int width, int height)
{
    return RoundedIntRect::Radii(IntSize(border.topLeft().width().calcValue(width), 
                                         border.topLeft().height().calcValue(height)),
                                 IntSize(border.topRight().width().calcValue(width),
                                         border.topRight().height().calcValue(height)),
                                 IntSize(border.bottomLeft().width().calcValue(width), 
                                         border.bottomLeft().height().calcValue(height)),
                                 IntSize(border.bottomRight().width().calcValue(width), 
                                         border.bottomRight().height().calcValue(height)));
}

static float calcConstraintScaleFor(const IntRect& rect, const RoundedIntRect::Radii& radii)
{
    // Constrain corner radii using CSS3 rules:
    // http://www.w3.org/TR/css3-background/#the-border-radius
    
    float factor = 1;
    unsigned radiiSum;

    // top
    radiiSum = static_cast<unsigned>(radii.topLeft().width()) + static_cast<unsigned>(radii.topRight().width()); // Casts to avoid integer overflow.
    if (radiiSum > static_cast<unsigned>(rect.width()))
        factor = min(static_cast<float>(rect.width()) / radiiSum, factor);

    // bottom
    radiiSum = static_cast<unsigned>(radii.bottomLeft().width()) + static_cast<unsigned>(radii.bottomRight().width());
    if (radiiSum > static_cast<unsigned>(rect.width()))
        factor = min(static_cast<float>(rect.width()) / radiiSum, factor);
    
    // left
    radiiSum = static_cast<unsigned>(radii.topLeft().height()) + static_cast<unsigned>(radii.bottomLeft().height());
    if (radiiSum > static_cast<unsigned>(rect.height()))
        factor = min(static_cast<float>(rect.height()) / radiiSum, factor);
    
    // right
    radiiSum = static_cast<unsigned>(radii.topRight().height()) + static_cast<unsigned>(radii.bottomRight().height());
    if (radiiSum > static_cast<unsigned>(rect.height()))
        factor = min(static_cast<float>(rect.height()) / radiiSum, factor);
    
    ASSERT(factor <= 1);
    return factor;
}

RoundedIntRect RenderStyle::getRoundedBorderFor(const IntRect& borderRect, bool includeLogicalLeftEdge, bool includeLogicalRightEdge) const
{
    RoundedIntRect roundedRect(borderRect);
    if (hasBorderRadius()) {
        RoundedIntRect::Radii radii = calcRadiiFor(m_surround->m_border, borderRect.width(), borderRect.height());
        radii.scale(calcConstraintScaleFor(borderRect, radii));
        roundedRect.includeLogicalEdges(radii, isHorizontalWritingMode(), includeLogicalLeftEdge, includeLogicalRightEdge);
    }
    return roundedRect;
}

RoundedIntRect RenderStyle::getRoundedInnerBorderFor(const IntRect& borderRect, bool includeLogicalLeftEdge, bool includeLogicalRightEdge) const
{
    bool horizontal = isHorizontalWritingMode();

    int leftWidth = (!horizontal || includeLogicalLeftEdge) ? borderLeftWidth() : 0;
    int rightWidth = (!horizontal || includeLogicalRightEdge) ? borderRightWidth() : 0;
    int topWidth = (horizontal || includeLogicalLeftEdge) ? borderTopWidth() : 0;
    int bottomWidth = (horizontal || includeLogicalRightEdge) ? borderBottomWidth() : 0;

    return getRoundedInnerBorderFor(borderRect, topWidth, bottomWidth, leftWidth, rightWidth, includeLogicalLeftEdge, includeLogicalRightEdge);
}

RoundedIntRect RenderStyle::getRoundedInnerBorderFor(const IntRect& borderRect,
    int topWidth, int bottomWidth, int leftWidth, int rightWidth, bool includeLogicalLeftEdge, bool includeLogicalRightEdge) const
{
    IntRect innerRect(borderRect.x() + leftWidth, 
            borderRect.y() + topWidth, 
            borderRect.width() - leftWidth - rightWidth, 
            borderRect.height() - topWidth - bottomWidth);

    RoundedIntRect roundedRect(innerRect);

    if (hasBorderRadius()) {
        RoundedIntRect::Radii radii = getRoundedBorderFor(borderRect).radii();
        radii.shrink(topWidth, bottomWidth, leftWidth, rightWidth);
        roundedRect.includeLogicalEdges(radii, isHorizontalWritingMode(), includeLogicalLeftEdge, includeLogicalRightEdge);
    }
    return roundedRect;
}

const CounterDirectiveMap* RenderStyle::counterDirectives() const
{
    return m_rareNonInheritedData->m_counterDirectives.get();
}

CounterDirectiveMap& RenderStyle::accessCounterDirectives()
{
    OwnPtr<CounterDirectiveMap>& map = m_rareNonInheritedData.access()->m_counterDirectives;
    if (!map)
        map = adoptPtr(new CounterDirectiveMap);
    return *map;
}

const AtomicString& RenderStyle::hyphenString() const
{
    ASSERT(hyphens() != HyphensNone);

    const AtomicString& hyphenationString = m_rareInheritedData.get()->m_hyphenationString;
    if (!hyphenationString.isNull())
        return hyphenationString;

    // FIXME: This should depend on locale.
    DEFINE_STATIC_LOCAL(AtomicString, hyphenMinusString, (&hyphenMinus, 1));
    DEFINE_STATIC_LOCAL(AtomicString, hyphenString, (&hyphen, 1));
    return font().primaryFontHasGlyphForCharacter(hyphen) ? hyphenString : hyphenMinusString;
}

const AtomicString& RenderStyle::textEmphasisMarkString() const
{
    switch (textEmphasisMark()) {
    case TextEmphasisMarkNone:
        return nullAtom;
    case TextEmphasisMarkCustom:
        return textEmphasisCustomMark();
    case TextEmphasisMarkDot: {
        DEFINE_STATIC_LOCAL(AtomicString, filledDotString, (&bullet, 1));
        DEFINE_STATIC_LOCAL(AtomicString, openDotString, (&whiteBullet, 1));
        return textEmphasisFill() == TextEmphasisFillFilled ? filledDotString : openDotString;
    }
    case TextEmphasisMarkCircle: {
        DEFINE_STATIC_LOCAL(AtomicString, filledCircleString, (&blackCircle, 1));
        DEFINE_STATIC_LOCAL(AtomicString, openCircleString, (&whiteCircle, 1));
        return textEmphasisFill() == TextEmphasisFillFilled ? filledCircleString : openCircleString;
    }
    case TextEmphasisMarkDoubleCircle: {
        DEFINE_STATIC_LOCAL(AtomicString, filledDoubleCircleString, (&fisheye, 1));
        DEFINE_STATIC_LOCAL(AtomicString, openDoubleCircleString, (&bullseye, 1));
        return textEmphasisFill() == TextEmphasisFillFilled ? filledDoubleCircleString : openDoubleCircleString;
    }
    case TextEmphasisMarkTriangle: {
        DEFINE_STATIC_LOCAL(AtomicString, filledTriangleString, (&blackUpPointingTriangle, 1));
        DEFINE_STATIC_LOCAL(AtomicString, openTriangleString, (&whiteUpPointingTriangle, 1));
        return textEmphasisFill() == TextEmphasisFillFilled ? filledTriangleString : openTriangleString;
    }
    case TextEmphasisMarkSesame: {
        DEFINE_STATIC_LOCAL(AtomicString, filledSesameString, (&sesameDot, 1));
        DEFINE_STATIC_LOCAL(AtomicString, openSesameString, (&whiteSesameDot, 1));
        return textEmphasisFill() == TextEmphasisFillFilled ? filledSesameString : openSesameString;
    }
    case TextEmphasisMarkAuto:
        ASSERT_NOT_REACHED();
        return nullAtom;
    }

    ASSERT_NOT_REACHED();
    return nullAtom;
}

#if ENABLE(DASHBOARD_SUPPORT)
const Vector<StyleDashboardRegion>& RenderStyle::initialDashboardRegions()
{
    DEFINE_STATIC_LOCAL(Vector<StyleDashboardRegion>, emptyList, ());
    return emptyList;
}

const Vector<StyleDashboardRegion>& RenderStyle::noneDashboardRegions()
{
    DEFINE_STATIC_LOCAL(Vector<StyleDashboardRegion>, noneList, ());
    static bool noneListInitialized = false;

    if (!noneListInitialized) {
        StyleDashboardRegion region;
        region.label = "";
        region.offset.m_top  = Length();
        region.offset.m_right = Length();
        region.offset.m_bottom = Length();
        region.offset.m_left = Length();
        region.type = StyleDashboardRegion::None;
        noneList.append(region);
        noneListInitialized = true;
    }
    return noneList;
}
#endif

void RenderStyle::adjustAnimations()
{
    AnimationList* animationList = m_rareNonInheritedData->m_animations.get();
    if (!animationList)
        return;

    // Get rid of empty animations and anything beyond them
    for (size_t i = 0; i < animationList->size(); ++i) {
        if (animationList->animation(i)->isEmpty()) {
            animationList->resize(i);
            break;
        }
    }

    if (animationList->isEmpty()) {
        clearAnimations();
        return;
    }

    // Repeat patterns into layers that don't have some properties set.
    animationList->fillUnsetProperties();
}

void RenderStyle::adjustTransitions()
{
    AnimationList* transitionList = m_rareNonInheritedData->m_transitions.get();
    if (!transitionList)
        return;

    // Get rid of empty transitions and anything beyond them
    for (size_t i = 0; i < transitionList->size(); ++i) {
        if (transitionList->animation(i)->isEmpty()) {
            transitionList->resize(i);
            break;
        }
    }

    if (transitionList->isEmpty()) {
        clearTransitions();
        return;
    }

    // Repeat patterns into layers that don't have some properties set.
    transitionList->fillUnsetProperties();

    // Make sure there are no duplicate properties. This is an O(n^2) algorithm
    // but the lists tend to be very short, so it is probably ok
    for (size_t i = 0; i < transitionList->size(); ++i) {
        for (size_t j = i+1; j < transitionList->size(); ++j) {
            if (transitionList->animation(i)->property() == transitionList->animation(j)->property()) {
                // toss i
                transitionList->remove(i);
                j = i;
            }
        }
    }
}

AnimationList* RenderStyle::accessAnimations()
{
    if (!m_rareNonInheritedData.access()->m_animations)
        m_rareNonInheritedData.access()->m_animations = adoptPtr(new AnimationList());
    return m_rareNonInheritedData->m_animations.get();
}

AnimationList* RenderStyle::accessTransitions()
{
    if (!m_rareNonInheritedData.access()->m_transitions)
        m_rareNonInheritedData.access()->m_transitions = adoptPtr(new AnimationList());
    return m_rareNonInheritedData->m_transitions.get();
}

const Animation* RenderStyle::transitionForProperty(int property) const
{
    if (transitions()) {
        for (size_t i = 0; i < transitions()->size(); ++i) {
            const Animation* p = transitions()->animation(i);
            if (p->property() == cAnimateAll || p->property() == property) {
                return p;
            }
        }
    }
    return 0;
}

void RenderStyle::setBlendedFontSize(int size)
{
    FontSelector* currentFontSelector = font().fontSelector();
    FontDescription desc(fontDescription());
    desc.setSpecifiedSize(size);
    desc.setComputedSize(size);
    setFontDescription(desc);
    font().update(currentFontSelector);
}

void RenderStyle::getShadowExtent(const ShadowData* shadow, int &top, int &right, int &bottom, int &left) const
{
    top = 0;
    right = 0;
    bottom = 0;
    left = 0;

    for ( ; shadow; shadow = shadow->next()) {
        if (shadow->style() == Inset)
            continue;
        int blurAndSpread = shadow->blur() + shadow->spread();

        top = min(top, shadow->y() - blurAndSpread);
        right = max(right, shadow->x() + blurAndSpread);
        bottom = max(bottom, shadow->y() + blurAndSpread);
        left = min(left, shadow->x() - blurAndSpread);
    }
}

void RenderStyle::getShadowHorizontalExtent(const ShadowData* shadow, int &left, int &right) const
{
    left = 0;
    right = 0;

    for ( ; shadow; shadow = shadow->next()) {
        if (shadow->style() == Inset)
            continue;
        int blurAndSpread = shadow->blur() + shadow->spread();

        left = min(left, shadow->x() - blurAndSpread);
        right = max(right, shadow->x() + blurAndSpread);
    }
}

void RenderStyle::getShadowVerticalExtent(const ShadowData* shadow, int &top, int &bottom) const
{
    top = 0;
    bottom = 0;

    for ( ; shadow; shadow = shadow->next()) {
        if (shadow->style() == Inset)
            continue;
        int blurAndSpread = shadow->blur() + shadow->spread();

        top = min(top, shadow->y() - blurAndSpread);
        bottom = max(bottom, shadow->y() + blurAndSpread);
    }
}

static EBorderStyle borderStyleForColorProperty(const RenderStyle* style, int colorProperty)
{
    EBorderStyle borderStyle;
    switch (colorProperty) {
    case CSSPropertyBorderLeftColor:
        borderStyle = style->borderLeftStyle();
        break;
    case CSSPropertyBorderRightColor:
        borderStyle = style->borderRightStyle();
        break;
    case CSSPropertyBorderTopColor:
        borderStyle = style->borderTopStyle();
        break;
    case CSSPropertyBorderBottomColor:
        borderStyle = style->borderBottomStyle();
        break;
    default:
        borderStyle = BNONE;
        break;
    }
    return borderStyle;
}

const Color RenderStyle::colorIncludingFallback(int colorProperty, EBorderStyle borderStyle) const
{
    Color result;
    switch (colorProperty) {
    case CSSPropertyBackgroundColor:
        return backgroundColor(); // Background color doesn't fall back.
    case CSSPropertyBorderLeftColor:
        result = borderLeftColor();
        borderStyle = borderLeftStyle();
        break;
    case CSSPropertyBorderRightColor:
        result = borderRightColor();
        borderStyle = borderRightStyle();
        break;
    case CSSPropertyBorderTopColor:
        result = borderTopColor();
        borderStyle = borderTopStyle();
        break;
    case CSSPropertyBorderBottomColor:
        result = borderBottomColor();
        borderStyle = borderBottomStyle();
        break;
    case CSSPropertyColor:
        result = color();
        break;
    case CSSPropertyOutlineColor:
        result = outlineColor();
        break;
    case CSSPropertyWebkitColumnRuleColor:
        result = columnRuleColor();
        break;
    case CSSPropertyWebkitTextEmphasisColor:
        result = textEmphasisColor();
        break;
    case CSSPropertyWebkitTextFillColor:
        result = textFillColor();
        break;
    case CSSPropertyWebkitTextStrokeColor:
        result = textStrokeColor();
        break;
    default:
        ASSERT_NOT_REACHED();
        break;
    }

    if (!result.isValid()) {
        if ((colorProperty == CSSPropertyBorderLeftColor || colorProperty == CSSPropertyBorderRightColor
            || colorProperty == CSSPropertyBorderTopColor || colorProperty == CSSPropertyBorderBottomColor)
            && (borderStyle == INSET || borderStyle == OUTSET || borderStyle == RIDGE || borderStyle == GROOVE))
            result.setRGB(238, 238, 238);
        else
            result = color();
    }

    return result;
}

const Color RenderStyle::visitedDependentColor(int colorProperty) const
{
    EBorderStyle borderStyle = borderStyleForColorProperty(this, colorProperty);
    Color unvisitedColor = colorIncludingFallback(colorProperty, borderStyle);
    if (insideLink() != InsideVisitedLink)
        return unvisitedColor;

    RenderStyle* visitedStyle = getCachedPseudoStyle(VISITED_LINK);
    if (!visitedStyle)
        return unvisitedColor;
    Color visitedColor = visitedStyle->colorIncludingFallback(colorProperty, borderStyle);

    // FIXME: Technically someone could explicitly specify the color transparent, but for now we'll just
    // assume that if the background color is transparent that it wasn't set. Note that it's weird that
    // we're returning unvisited info for a visited link, but given our restriction that the alpha values
    // have to match, it makes more sense to return the unvisited background color if specified than it
    // does to return black. This behavior matches what Firefox 4 does as well.
    if (colorProperty == CSSPropertyBackgroundColor && visitedColor == Color::transparent)
        return unvisitedColor;

    // Take the alpha from the unvisited color, but get the RGB values from the visited color.
    return Color(visitedColor.red(), visitedColor.green(), visitedColor.blue(), unvisitedColor.alpha());
}

Length RenderStyle::logicalWidth() const
{
    if (isHorizontalWritingMode())
        return width();
    return height();
}

Length RenderStyle::logicalHeight() const
{
    if (isHorizontalWritingMode())
        return height();
    return width();
}

Length RenderStyle::logicalMinWidth() const
{
    if (isHorizontalWritingMode())
        return minWidth();
    return minHeight();
}

Length RenderStyle::logicalMaxWidth() const
{
    if (isHorizontalWritingMode())
        return maxWidth();
    return maxHeight();
}

Length RenderStyle::logicalMinHeight() const
{
    if (isHorizontalWritingMode())
        return minHeight();
    return minWidth();
}

Length RenderStyle::logicalMaxHeight() const
{
    if (isHorizontalWritingMode())
        return maxHeight();
    return maxWidth();
}

const BorderValue& RenderStyle::borderBefore() const
{
    switch (writingMode()) {
    case TopToBottomWritingMode:
        return borderTop();
    case BottomToTopWritingMode:
        return borderBottom();
    case LeftToRightWritingMode:
        return borderLeft();
    case RightToLeftWritingMode:
        return borderRight();
    }
    ASSERT_NOT_REACHED();
    return borderTop();
}

const BorderValue& RenderStyle::borderAfter() const
{
    switch (writingMode()) {
    case TopToBottomWritingMode:
        return borderBottom();
    case BottomToTopWritingMode:
        return borderTop();
    case LeftToRightWritingMode:
        return borderRight();
    case RightToLeftWritingMode:
        return borderLeft();
    }
    ASSERT_NOT_REACHED();
    return borderBottom();
}

const BorderValue& RenderStyle::borderStart() const
{
    if (isHorizontalWritingMode())
        return isLeftToRightDirection() ? borderLeft() : borderRight();
    return isLeftToRightDirection() ? borderTop() : borderBottom();
}

const BorderValue& RenderStyle::borderEnd() const
{
    if (isHorizontalWritingMode())
        return isLeftToRightDirection() ? borderRight() : borderLeft();
    return isLeftToRightDirection() ? borderBottom() : borderTop();
}

unsigned short RenderStyle::borderBeforeWidth() const
{
    switch (writingMode()) {
    case TopToBottomWritingMode:
        return borderTopWidth();
    case BottomToTopWritingMode:
        return borderBottomWidth();
    case LeftToRightWritingMode:
        return borderLeftWidth();
    case RightToLeftWritingMode:
        return borderRightWidth();
    }
    ASSERT_NOT_REACHED();
    return borderTopWidth();
}

unsigned short RenderStyle::borderAfterWidth() const
{
    switch (writingMode()) {
    case TopToBottomWritingMode:
        return borderBottomWidth();
    case BottomToTopWritingMode:
        return borderTopWidth();
    case LeftToRightWritingMode:
        return borderRightWidth();
    case RightToLeftWritingMode:
        return borderLeftWidth();
    }
    ASSERT_NOT_REACHED();
    return borderBottomWidth();
}

unsigned short RenderStyle::borderStartWidth() const
{
    if (isHorizontalWritingMode())
        return isLeftToRightDirection() ? borderLeftWidth() : borderRightWidth();
    return isLeftToRightDirection() ? borderTopWidth() : borderBottomWidth();
}

unsigned short RenderStyle::borderEndWidth() const
{
    if (isHorizontalWritingMode())
        return isLeftToRightDirection() ? borderRightWidth() : borderLeftWidth();
    return isLeftToRightDirection() ? borderBottomWidth() : borderTopWidth();
}
    
Length RenderStyle::marginBefore() const
{
    switch (writingMode()) {
    case TopToBottomWritingMode:
        return marginTop();
    case BottomToTopWritingMode:
        return marginBottom();
    case LeftToRightWritingMode:
        return marginLeft();
    case RightToLeftWritingMode:
        return marginRight();
    }
    ASSERT_NOT_REACHED();
    return marginTop();
}

Length RenderStyle::marginAfter() const
{
    switch (writingMode()) {
    case TopToBottomWritingMode:
        return marginBottom();
    case BottomToTopWritingMode:
        return marginTop();
    case LeftToRightWritingMode:
        return marginRight();
    case RightToLeftWritingMode:
        return marginLeft();
    }
    ASSERT_NOT_REACHED();
    return marginBottom();
}

Length RenderStyle::marginBeforeUsing(const RenderStyle* otherStyle) const
{
    switch (otherStyle->writingMode()) {
    case TopToBottomWritingMode:
        return marginTop();
    case BottomToTopWritingMode:
        return marginBottom();
    case LeftToRightWritingMode:
        return marginLeft();
    case RightToLeftWritingMode:
        return marginRight();
    }
    ASSERT_NOT_REACHED();
    return marginTop();
}

Length RenderStyle::marginAfterUsing(const RenderStyle* otherStyle) const
{
    switch (otherStyle->writingMode()) {
    case TopToBottomWritingMode:
        return marginBottom();
    case BottomToTopWritingMode:
        return marginTop();
    case LeftToRightWritingMode:
        return marginRight();
    case RightToLeftWritingMode:
        return marginLeft();
    }
    ASSERT_NOT_REACHED();
    return marginBottom();
}

Length RenderStyle::marginStart() const
{
    if (isHorizontalWritingMode())
        return isLeftToRightDirection() ? marginLeft() : marginRight();
    return isLeftToRightDirection() ? marginTop() : marginBottom();
}

Length RenderStyle::marginEnd() const
{
    if (isHorizontalWritingMode())
        return isLeftToRightDirection() ? marginRight() : marginLeft();
    return isLeftToRightDirection() ? marginBottom() : marginTop();
}
    
Length RenderStyle::marginStartUsing(const RenderStyle* otherStyle) const
{
    if (otherStyle->isHorizontalWritingMode())
        return otherStyle->isLeftToRightDirection() ? marginLeft() : marginRight();
    return otherStyle->isLeftToRightDirection() ? marginTop() : marginBottom();
}

Length RenderStyle::marginEndUsing(const RenderStyle* otherStyle) const
{
    if (otherStyle->isHorizontalWritingMode())
        return otherStyle->isLeftToRightDirection() ? marginRight() : marginLeft();
    return otherStyle->isLeftToRightDirection() ? marginBottom() : marginTop();
}

void RenderStyle::setMarginStart(Length margin)
{
    if (isHorizontalWritingMode()) {
        if (isLeftToRightDirection())
            setMarginLeft(margin);
        else
            setMarginRight(margin);
    } else {
        if (isLeftToRightDirection())
            setMarginTop(margin);
        else
            setMarginBottom(margin);
    }
}

void RenderStyle::setMarginEnd(Length margin)
{
    if (isHorizontalWritingMode()) {
        if (isLeftToRightDirection())
            setMarginRight(margin);
        else
            setMarginLeft(margin);
    } else {
        if (isLeftToRightDirection())
            setMarginBottom(margin);
        else
            setMarginTop(margin);
    }
}

Length RenderStyle::paddingBefore() const
{
    switch (writingMode()) {
    case TopToBottomWritingMode:
        return paddingTop();
    case BottomToTopWritingMode:
        return paddingBottom();
    case LeftToRightWritingMode:
        return paddingLeft();
    case RightToLeftWritingMode:
        return paddingRight();
    }
    ASSERT_NOT_REACHED();
    return paddingTop();
}

Length RenderStyle::paddingAfter() const
{
    switch (writingMode()) {
    case TopToBottomWritingMode:
        return paddingBottom();
    case BottomToTopWritingMode:
        return paddingTop();
    case LeftToRightWritingMode:
        return paddingRight();
    case RightToLeftWritingMode:
        return paddingLeft();
    }
    ASSERT_NOT_REACHED();
    return paddingBottom();
}

Length RenderStyle::paddingStart() const
{
    if (isHorizontalWritingMode())
        return isLeftToRightDirection() ? paddingLeft() : paddingRight();
    return isLeftToRightDirection() ? paddingTop() : paddingBottom();
}

Length RenderStyle::paddingEnd() const
{
    if (isHorizontalWritingMode())
        return isLeftToRightDirection() ? paddingRight() : paddingLeft();
    return isLeftToRightDirection() ? paddingBottom() : paddingTop();
}

TextEmphasisMark RenderStyle::textEmphasisMark() const
{
    TextEmphasisMark mark = static_cast<TextEmphasisMark>(m_rareInheritedData->m_textEmphasisMark);
    if (mark != TextEmphasisMarkAuto)
        return mark;

    if (isHorizontalWritingMode())
        return TextEmphasisMarkDot;

    return TextEmphasisMarkSesame;
}

} // namespace WebCore
