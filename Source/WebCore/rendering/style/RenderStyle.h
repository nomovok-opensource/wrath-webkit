/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003, 2005, 2006, 2007, 2008, 2009, 2010 Apple Inc. All rights reserved.
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

#ifndef RenderStyle_h
#define RenderStyle_h

#include "AnimationList.h"
#include "BorderValue.h"
#include "CSSLineBoxContainValue.h"
#include "CSSPrimitiveValue.h"
#include "CSSPropertyNames.h"
#include "Color.h"
#include "ColorSpace.h"
#include "CounterDirectives.h"
#include "DataRef.h"
#include "FillLayer.h"
#include "Font.h"
#include "GraphicsTypes.h"
#include "Length.h"
#include "LengthBox.h"
#include "LengthSize.h"
#include "LineClampValue.h"
#include "NinePieceImage.h"
#include "OutlineValue.h"
#include "RenderStyleConstants.h"
#include "RoundedIntRect.h"
#include "ShadowData.h"
#include "StyleBackgroundData.h"
#include "StyleBoxData.h"
#include "StyleFlexibleBoxData.h"
#include "StyleInheritedData.h"
#include "StyleMarqueeData.h"
#include "StyleMultiColData.h"
#include "StyleRareInheritedData.h"
#include "StyleRareNonInheritedData.h"
#include "StyleReflection.h"
#include "StyleSurroundData.h"
#include "StyleTransformData.h"
#include "StyleVisualData.h"
#include "TextDirection.h"
#include "TextOrientation.h"
#include "ThemeTypes.h"
#include "TransformOperations.h"
#include "UnicodeBidi.h"
#include "Signal.h"
#include <wtf/Forward.h>
#include <wtf/OwnPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/StdLibExtras.h>
#include <wtf/Vector.h>

#if ENABLE(DASHBOARD_SUPPORT)
#include "StyleDashboardRegion.h"
#endif

#if ENABLE(SVG)
#include "SVGRenderStyle.h"
#endif

#if COMPILER(WINSCW)
#define compareEqual(t, u)      ((t) == (u))
#else
template<typename T, typename U> inline bool compareEqual(const T& t, const U& u) { return t == static_cast<T>(u); }
#endif

#if USE(WRATH)
#define SET_VAR(group, variable, value) \
    do if (!compareEqual(group->variable, value)) \
        group.access()->variable = value; while(0)

// Set variable and generate a signal
#define SET_VAR_GEN_SIG(group, variable, value) \
    do if (!compareEqual(m_##group->m_##variable, value)) \
    { \
        m_##group.access()->m_##variable = (value); \
        m_signal(E_##group##_##variable); \
    } while(0)

// Same as above, but variable has a member variable that is set...
#define SET_VAR_GEN_SIG2(group, variable, member, value) \
    do if (!compareEqual(m_##group->m_##variable.m_##member, value)) \
    { \
        m_##group.access()->m_##variable.m_##member = (value); \
        m_signal(E_##group##_##variable##_##member); \
    } while(0)

#define SET_VAR_GEN_SIG3(group, subgroup, variable, value) \
    do if (!compareEqual(m_##group->m_##subgroup->m_##variable, value)) \
    { \
        m_##group.access()->m_##subgroup.access()->m_##variable = (value); \
        m_signal(E_##group##_##subgroup##_##variable); \
    } while(0)

#define SET_VAR_GEN_SIG4(group, variable, subvariable, member, value) \
    do if (!compareEqual(m_##group->m_##variable.m_##subvariable.m_##member, value)) \
    { \
        m_##group.access()->m_##variable.m_##subvariable.m_##member = (value); \
        m_signal(E_##group##_##variable##_##subvariable##_##member); \
    } while(0)

#define SET_FLAG(group, variable, value) \
    do if (m_##group.m_##variable != (value)) \
    { \
        m_##group.m_##variable = (value); \
        m_signal(E_##group##_##variable); \
    } while(0)
#else
#define SET_VAR(group, variable, value) \
    do if (!compareEqual(group->variable, value)) \
        group.access()->variable = value; while(0)

// Set variable and generate a signal
#define SET_VAR_GEN_SIG(group, variable, value) \
    do if (!compareEqual(m_##group->m_##variable, value)) \
    { \
        m_##group.access()->m_##variable = (value); \
    } while(0)

// Same as above, but variable has a member variable that is set...
#define SET_VAR_GEN_SIG2(group, variable, member, value) \
    do if (!compareEqual(m_##group->m_##variable.m_##member, value)) \
    { \
        m_##group.access()->m_##variable.m_##member = (value); \
    } while(0)

#define SET_VAR_GEN_SIG3(group, subgroup, variable, value) \
    do if (!compareEqual(m_##group->m_##subgroup->m_##variable, value)) \
    { \
        m_##group.access()->m_##subgroup.access()->m_##variable = (value); \
    } while(0)

#define SET_VAR_GEN_SIG4(group, variable, subvariable, member, value) \
    do if (!compareEqual(m_##group->m_##variable.m_##subvariable.m_##member, value)) \
    { \
        m_##group.access()->m_##variable.m_##subvariable.m_##member = (value); \
    } while(0)

#define SET_FLAG(group, variable, value) \
    do if (m_##group.m_##variable != (value)) \
    { \
        m_##group.m_##variable = (value); \
    } while(0)
#endif

namespace WebCore {

using std::max;

class BorderData;
class CSSStyleSelector;
class CounterContent;
class CursorList;
class IntRect;
class Pair;
class ShadowData;
class StyleImage;
class TransformationMatrix;

struct ContentData;

typedef Vector<RefPtr<RenderStyle>, 4> PseudoStyleCache;

class RenderStyle: public RefCounted<RenderStyle> {
    friend class AnimationBase; // Used by CSS animations. We can't allow them to animate based off visited colors.
    friend class ApplyStyleCommand; // Editing has to only reveal unvisited info.
    friend class EditingStyle; // Editing has to only reveal unvisited info.
    friend class CSSStyleApplyProperty; // Sets members directly.
    friend class CSSStyleSelector; // Sets members directly.
    friend class CSSComputedStyleDeclaration; // Ignores visited styles, so needs to be able to see unvisited info.
    friend class PropertyWrapperMaybeInvalidColor; // Used by CSS animations. We can't allow them to animate based off visited colors.
    friend class RenderSVGResource; // FIXME: Needs to alter the visited state by hand. Should clean the SVG code up and move it into RenderStyle perhaps.
    friend class RenderTreeAsText; // FIXME: Only needed so the render tree can keep lying and dump the wrong colors.  Rebaselining would allow this to be yanked.
protected:

    // The following bitfield is 32-bits long, which optimizes padding with the
    // int refCount in the base class. Beware when adding more bits.
    bool m_affectedByAttributeSelectors : 1;
    bool m_unique : 1;

    // Bits for dynamic child matching.
    bool m_affectedByEmpty : 1;
    bool m_emptyState : 1;

    // We optimize for :first-child and :last-child.  The other positional child selectors like nth-child or
    // *-child-of-type, we will just give up and re-evaluate whenever children change at all.
    bool m_childrenAffectedByFirstChildRules : 1;
    bool m_childrenAffectedByLastChildRules : 1;
    bool m_childrenAffectedByDirectAdjacentRules : 1;
    bool m_childrenAffectedByForwardPositionalRules : 1;
    bool m_childrenAffectedByBackwardPositionalRules : 1;
    bool m_firstChildState : 1;
    bool m_lastChildState : 1;
    unsigned m_childIndex : 21; // Plenty of bits to cache an index.

    // non-inherited attributes
    DataRef<StyleBoxData> m_box;
    DataRef<StyleVisualData> m_visual;
    DataRef<StyleBackgroundData> m_background;
    DataRef<StyleSurroundData> m_surround;
    DataRef<StyleRareNonInheritedData> m_rareNonInheritedData;

    // inherited attributes
    DataRef<StyleRareInheritedData> m_rareInheritedData;
    DataRef<StyleInheritedData> m_inherited;

    // list of associated pseudo styles
    OwnPtr<PseudoStyleCache> m_cachedPseudoStyles;

#if ENABLE(SVG)
    DataRef<SVGRenderStyle> m_svgStyle;
#endif

// !START SYNC!: Keep this in sync with the copy constructor in RenderStyle.cpp

    // inherit
    struct InheritedFlags {
        bool operator==(const InheritedFlags& other) const
        {
            return (m_emptyCells == other.m_emptyCells)
                && (m_captionSide == other.m_captionSide)
                && (m_listStyleType == other.m_listStyleType)
                && (m_listStylePosition == other.m_listStylePosition)
                && (m_visibility == other.m_visibility)
                && (m_textAlign == other.m_textAlign)
                && (m_textTransform == other.m_textTransform)
                && (m_textDecorations == other.m_textDecorations)
                && (m_cursorStyle == other.m_cursorStyle)
                && (m_direction == other.m_direction)
                && (m_borderCollapse == other.m_borderCollapse)
                && (m_whiteSpace == other.m_whiteSpace)
                && (m_boxDirection == other.m_boxDirection)
                && (m_visuallyOrdered == other.m_visuallyOrdered)
                && (m_forceBackgroundsToWhite == other.m_forceBackgroundsToWhite)
                && (m_pointerEvents == other.m_pointerEvents)
                && (m_insideLink == other.m_insideLink)
                && (m_writingMode == other.m_writingMode);
        }

        bool operator!=(const InheritedFlags& other) const { return !(*this == other); }

        unsigned m_emptyCells : 1; // EEmptyCell
        unsigned m_captionSide : 2; // ECaptionSide
        unsigned m_listStyleType : 7; // EListStyleType
        unsigned m_listStylePosition : 1; // EListStylePosition
        unsigned m_visibility : 2; // EVisibility
        unsigned m_textAlign : 4; // ETextAlign
        unsigned m_textTransform : 2; // ETextTransform
        unsigned m_textDecorations : 4;
        unsigned m_cursorStyle : 6; // ECursor
        unsigned m_direction : 1; // TextDirection
        unsigned m_borderCollapse : 1; // EBorderCollapse
        unsigned m_whiteSpace : 3; // EWhiteSpace
        unsigned m_boxDirection : 1; // EBoxDirection (CSS3 box_direction property, flexible box layout module)
        // 34 bits
        
        // non CSS2 inherited
        bool m_visuallyOrdered : 1;
        bool m_forceBackgroundsToWhite : 1;
        unsigned m_pointerEvents : 4; // EPointerEvents
        unsigned m_insideLink : 2; // EInsideLink
        // 43 bits

        // CSS Text Layout Module Level 3: Vertical writing support
        unsigned m_writingMode : 2; // WritingMode
        // 45 bits
    } m_inheritedFlags;

// don't inherit
    struct NonInheritedFlags {
        bool operator==(const NonInheritedFlags& other) const
        {
            return m_effectiveDisplay == other.m_effectiveDisplay
                && m_originalDisplay == other.m_originalDisplay
                && m_overflowX == other.m_overflowX
                && m_overflowY == other.m_overflowY
                && m_verticalAlign == other.m_verticalAlign
                && m_clear == other.m_clear
                && m_position == other.m_position
                && m_floating == other.m_floating
                && m_tableLayout == other.m_tableLayout
                && m_pageBreakBefore == other.m_pageBreakBefore
                && m_pageBreakAfter == other.m_pageBreakAfter
                && m_pageBreakInside == other.m_pageBreakInside
                && m_styleType == other.m_styleType
                && m_affectedByHover == other.m_affectedByHover
                && m_affectedByActive == other.m_affectedByActive
                && m_affectedByDrag == other.m_affectedByDrag
                && m_pseudoBits == other.m_pseudoBits
                && m_unicodeBidi == other.m_unicodeBidi
                && m_isLink == other.m_isLink;
        }

        bool operator!=(const NonInheritedFlags& other) const { return !(*this == other); }

        unsigned m_effectiveDisplay : 5; // EDisplay
        unsigned m_originalDisplay : 5; // EDisplay
        unsigned m_overflowX : 3; // EOverflow
        unsigned m_overflowY : 3; // EOverflow
        unsigned m_verticalAlign : 4; // EVerticalAlign
        unsigned m_clear : 2; // EClear
        unsigned m_position : 2; // EPosition
        unsigned m_floating : 2; // EFloat
        unsigned m_tableLayout : 1; // ETableLayout

        unsigned m_pageBreakBefore : 2; // EPageBreak
        unsigned m_pageBreakAfter : 2; // EPageBreak
        unsigned m_pageBreakInside : 2; // EPageBreak

        unsigned m_styleType : 6; // PseudoId
        bool m_affectedByHover : 1;
        bool m_affectedByActive : 1;
        bool m_affectedByDrag : 1;
        unsigned m_pseudoBits : 7;
        unsigned m_unicodeBidi : 2; // EUnicodeBidi
        bool m_isLink : 1;
        // 50 bits
    } m_noninheritedFlags;

// !END SYNC!

protected:
    void setBitDefaults()
    {
        m_inheritedFlags.m_emptyCells = initialEmptyCells();
        m_inheritedFlags.m_captionSide = initialCaptionSide();
        m_inheritedFlags.m_listStyleType = initialListStyleType();
        m_inheritedFlags.m_listStylePosition = initialListStylePosition();
        m_inheritedFlags.m_visibility = initialVisibility();
        m_inheritedFlags.m_textAlign = initialTextAlign();
        m_inheritedFlags.m_textTransform = initialTextTransform();
        m_inheritedFlags.m_textDecorations = initialTextDecoration();
        m_inheritedFlags.m_cursorStyle = initialCursor();
        m_inheritedFlags.m_direction = initialDirection();
        m_inheritedFlags.m_borderCollapse = initialBorderCollapse();
        m_inheritedFlags.m_whiteSpace = initialWhiteSpace();
        m_inheritedFlags.m_visuallyOrdered = initialVisuallyOrdered();
        m_inheritedFlags.m_boxDirection = initialBoxDirection();
        m_inheritedFlags.m_forceBackgroundsToWhite = false;
        m_inheritedFlags.m_pointerEvents = initialPointerEvents();
        m_inheritedFlags.m_insideLink = NotInsideLink;
        m_inheritedFlags.m_writingMode = initialWritingMode();

        m_noninheritedFlags.m_effectiveDisplay = m_noninheritedFlags.m_originalDisplay = initialDisplay();
        m_noninheritedFlags.m_overflowX = initialOverflowX();
        m_noninheritedFlags.m_overflowY = initialOverflowY();
        m_noninheritedFlags.m_verticalAlign = initialVerticalAlign();
        m_noninheritedFlags.m_clear = initialClear();
        m_noninheritedFlags.m_position = initialPosition();
        m_noninheritedFlags.m_floating = initialFloating();
        m_noninheritedFlags.m_tableLayout = initialTableLayout();
        m_noninheritedFlags.m_pageBreakBefore = initialPageBreak();
        m_noninheritedFlags.m_pageBreakAfter = initialPageBreak();
        m_noninheritedFlags.m_pageBreakInside = initialPageBreak();
        m_noninheritedFlags.m_styleType = NOPSEUDO;
        m_noninheritedFlags.m_affectedByHover = false;
        m_noninheritedFlags.m_affectedByActive = false;
        m_noninheritedFlags.m_affectedByDrag = false;
        m_noninheritedFlags.m_pseudoBits = 0;
        m_noninheritedFlags.m_unicodeBidi = initialUnicodeBidi();
        m_noninheritedFlags.m_isLink = false;
    }

private:
    ALWAYS_INLINE RenderStyle();
    // used to create the default style.
    ALWAYS_INLINE RenderStyle(bool);
    ALWAYS_INLINE RenderStyle(const RenderStyle&);

public:
    enum MemberChange {
        E_affectedByAttributeSelectors,
        E_unique,
        E_affectedByEmpty,
        E_emptyState,
        E_childrenAffectedByFirstChildRules,
        E_childrenAffectedByLastChildRules,
        E_childrenAffectedByDirectAdjacentRules,
        E_childrenAffectedByForwardPositionalRules,
        E_childrenAffectedByBackwardPositionalRules,
        E_firstChildState,
        E_lastChildState,
        E_childIndex,

        E_noninheritedFlags_styleType,
        E_noninheritedFlags_affectedByHover,
        E_noninheritedFlags_affectedByActive,
        E_noninheritedFlags_affectedByDrag,
        E_noninheritedFlags_effectiveDisplay,
        E_noninheritedFlags_originalDisplay,
        E_noninheritedFlags_position,
        E_noninheritedFlags_floating,
        E_noninheritedFlags_overflowX,
        E_noninheritedFlags_overflowY,
        E_noninheritedFlags_verticalAlign,
        E_noninheritedFlags_unicodeBidi,
        E_noninheritedFlags_clear,
        E_noninheritedFlags_tableLayout,
        E_noninheritedFlags_isLink,
        E_noninheritedFlags_pageBreakInside,
        E_noninheritedFlags_pageBreakBefore,
        E_noninheritedFlags_pageBreakAfter,

        E_inheritedFlags_visuallyOrdered,
        E_inheritedFlags_visibility,
        E_inheritedFlags_textAlign,
        E_inheritedFlags_textTransform,
        E_inheritedFlags_textDecorations,
        E_inheritedFlags_direction,
        E_inheritedFlags_borderCollapse,
        E_inheritedFlags_whiteSpace,
        E_inheritedFlags_emptyCells,
        E_inheritedFlags_captionSide,
        E_inheritedFlags_listStyleType,
        E_inheritedFlags_listStylePosition,
        E_inheritedFlags_cursorStyle,
        E_inheritedFlags_insideLink,
        E_inheritedFlags_forceBackgroundsToWhite,
        E_inheritedFlags_boxDirection,
        E_inheritedFlags_pointerEvents,
        E_inheritedFlags_writingMode,

        E_box_width,
        E_box_height,
        E_box_minWidth,
        E_box_maxWidth,
        E_box_minHeight,
        E_box_maxHeight,
        E_rareNonInheritedData_dashboardRegions,
        E_background_outline,
        E_background_color,
        E_box_verticalAlign,
        E_visual_hasClip,
        E_visual_clip,
        E_inherited_color,
        E_visual_textDecoration,
        E_inherited_lineHeight,
        E_visual_zoom,
        E_box_hasAutoZIndex,
        E_box_zIndex,
        E_box_boxSizing,
        E_inherited_horizontalBorderSpacing,

        E_rareInheritedData_indent,
        E_rareInheritedData_effectiveZoom,
        E_rareNonInheritedData_maskBoxImage,
        E_inherited_verticalBorderSpacing,
        E_rareNonInheritedData_counterIncrement,
        E_rareNonInheritedData_counterReset,
        E_surround_margin,
        E_surround_padding,
        E_rareInheritedData_widows,
        E_rareInheritedData_orphans,
        E_rareInheritedData_textStrokeColor,
        E_rareInheritedData_textStrokeWidth,
        E_rareInheritedData_textFillColor,
        E_rareInheritedData_colorSpace,
        E_rareNonInheritedData_opacity,
        E_rareNonInheritedData_appearance,
        E_rareInheritedData_userModify,
        E_rareNonInheritedData_userDrag,
        E_rareInheritedData_userSelect,
        E_rareNonInheritedData_textOverflow,
        E_rareNonInheritedData_marginBeforeCollapse,
        E_rareNonInheritedData_marginAfterCollapse,
        E_rareInheritedData_wordBreak,
        E_rareInheritedData_wordWrap,
        E_rareInheritedData_nbspMode,
        E_rareInheritedData_khtmlLineBreak,
        E_rareNonInheritedData_matchNearestMailBlockquoteColor,
        E_rareInheritedData_highlight,
        E_rareInheritedData_hyphens,
        E_rareInheritedData_hyphenationLimitBefore,
        E_rareInheritedData_hyphenationLimitAfter,
        E_rareInheritedData_hyphenationString,
        E_rareInheritedData_locale,
        E_rareNonInheritedData_borderFit,
        E_rareInheritedData_resize,
        E_rareInheritedData_speak,
        E_rareNonInheritedData_textCombine,
        E_rareInheritedData_textEmphasisColor,
        E_rareInheritedData_textEmphasisFill,
        E_rareInheritedData_textEmphasisMark,
        E_rareInheritedData_textEmphasisCustomMark,
        E_rareInheritedData_textEmphasisPosition,
        E_rareNonInheritedData_transformStyle3D,
        E_rareNonInheritedData_backfaceVisibility,
        E_rareNonInheritedData_perspective,
        E_rareNonInheritedData_perspectiveOriginX,
        E_rareNonInheritedData_perspectiveOriginY,
        E_rareNonInheritedData_pageSize,
        E_rareNonInheritedData_pageSizeType,
        E_rareNonInheritedData_runningAcceleratedAnimation,
        E_rareInheritedData_lineBoxContain,
        E_rareNonInheritedData_lineClamp,
        E_rareInheritedData_textSizeAdjust,
        E_rareInheritedData_textSecurity,

        E_surround_offset_left,
        E_surround_offset_right,
        E_surround_offset_top,
        E_surround_offset_bottom,
        E_surround_border_top,
        E_surround_border_bottom,
        E_surround_border_right,
        E_surround_border_left,
        E_surround_border_image,
        E_background_background_xPosition,
        E_background_background_yPosition,
        E_background_background_sizeType,
        E_background_background_sizeLength,
        E_surround_border_topLeft,
        E_surround_border_topRight,
        E_surround_border_bottomLeft,
        E_surround_border_bottomRight,
        E_background_outline_width,
        E_background_outline_style,
        E_background_outline_isAuto,
        E_background_outline_color,
        E_visual_clip_left,
        E_visual_clip_right,
        E_visual_clip_top,
        E_visual_clip_bottom,
        E_rareNonInheritedData_mask_xPosition,
        E_rareNonInheritedData_mask_yPosition,
        E_rareNonInheritedData_mask_sizeLength,
        E_surround_margin_top,
        E_surround_margin_bottom,
        E_surround_margin_left,
        E_surround_margin_right,
        E_surround_padding_top,
        E_surround_padding_bottom,
        E_surround_padding_left,
        E_surround_padding_right,
        E_background_outline_offset,

        E_rareNonInheritedData_flexibleBox_align,
        E_rareNonInheritedData_flexibleBox_flex,
        E_rareNonInheritedData_flexibleBox_flexGroup,
        E_rareNonInheritedData_flexibleBox_lines,
        E_rareNonInheritedData_flexibleBox_ordinalGroup,
        E_rareNonInheritedData_flexibleBox_orient,
        E_rareNonInheritedData_flexibleBox_pack,
        E_rareNonInheritedData_marquee_increment,
        E_rareNonInheritedData_marquee_direction,
        E_rareNonInheritedData_marquee_speed,
        E_rareNonInheritedData_marquee_behavior,
        E_rareNonInheritedData_marquee_loops,
        E_rareNonInheritedData_multiCol_autoWidth,
        E_rareNonInheritedData_multiCol_width,
        E_rareNonInheritedData_multiCol_autoCount,
        E_rareNonInheritedData_multiCol_count,
        E_rareNonInheritedData_multiCol_normalGap,
        E_rareNonInheritedData_multiCol_gap,
        E_rareNonInheritedData_multiCol_rule,
        E_rareNonInheritedData_multiCol_columnSpan,
        E_rareNonInheritedData_multiCol_breakBefore,
        E_rareNonInheritedData_multiCol_breakInside,
        E_rareNonInheritedData_multiCol_breakAfter,
        E_rareNonInheritedData_transform_operations,
        E_rareNonInheritedData_transform_x,
        E_rareNonInheritedData_transform_y,
        E_rareNonInheritedData_transform_z,

        E_surround_border_left_width,
        E_surround_border_left_style,
        E_surround_border_left_color,
        E_surround_border_right_width,
        E_surround_border_right_style,
        E_surround_border_right_color,
        E_surround_border_top_width,
        E_surround_border_top_style,
        E_surround_border_top_color,
        E_surround_border_bottom_width,
        E_surround_border_bottom_style,
        E_surround_border_bottom_color,

        E_rareNonInheritedData_multiCol_rule_color,
        E_rareNonInheritedData_multiCol_rule_style,
        E_rareNonInheritedData_multiCol_rule_width,

        E_noninheritedFlags_pseudoBits,
        E_rareInheritedData_cursorData,
        E_rareInheritedData_quotes,
        E_rareInheritedData_content,
        E_rareInheritedData_textShadow,
        E_rareInheritedData_boxShadow,

        E_numberOfChanges
    };

    static PassRefPtr<RenderStyle> create();
    static PassRefPtr<RenderStyle> createDefaultStyle();
    static PassRefPtr<RenderStyle> createAnonymousStyle(const RenderStyle* parentStyle);
    static PassRefPtr<RenderStyle> clone(const RenderStyle*);

    ~RenderStyle();

    void inheritFrom(const RenderStyle* inheritParent);

    PseudoId styleType() const { return static_cast<PseudoId>(m_noninheritedFlags.m_styleType); }
    void setStyleType(PseudoId styleType) { SET_FLAG(noninheritedFlags, styleType, styleType); }

    RenderStyle* getCachedPseudoStyle(PseudoId) const;
    RenderStyle* addCachedPseudoStyle(PassRefPtr<RenderStyle>);
    void removeCachedPseudoStyle(PseudoId);

    const PseudoStyleCache* cachedPseudoStyles() const { return m_cachedPseudoStyles.get(); }

    bool affectedByHoverRules() const { return m_noninheritedFlags.m_affectedByHover; }
    bool affectedByActiveRules() const { return m_noninheritedFlags.m_affectedByActive; }
    bool affectedByDragRules() const { return m_noninheritedFlags.m_affectedByDrag; }


    void setAffectedByHoverRules(bool b) { SET_FLAG(noninheritedFlags, affectedByHover, b); }
    void setAffectedByActiveRules(bool b) { SET_FLAG(noninheritedFlags, affectedByActive, b); }
    void setAffectedByDragRules(bool b) { SET_FLAG(noninheritedFlags, affectedByDrag, b); }

    bool operator==(const RenderStyle& other) const;
    bool operator!=(const RenderStyle& other) const { return !(*this == other); }
    bool isFloating() const { return !(m_noninheritedFlags.m_floating == FNONE); }
    bool hasMargin() const { return m_surround->m_margin.nonZero(); }
    bool hasBorder() const { return m_surround->m_border.hasBorder(); }
    bool hasPadding() const { return m_surround->m_padding.nonZero(); }
    bool hasOffset() const { return m_surround->m_offset.nonZero(); }

    bool hasBackgroundImage() const { return m_background->background().hasImage(); }
    bool hasFixedBackgroundImage() const { return m_background->background().hasFixedImage(); }
    bool hasAppearance() const { return appearance() != NoControlPart; }

    bool hasBackground() const
    {
        Color color = visitedDependentColor(CSSPropertyBackgroundColor);
        if (color.isValid() && color.alpha() > 0)
            return true;
        return hasBackgroundImage();
    }

    bool visuallyOrdered() const { return m_inheritedFlags.m_visuallyOrdered; }
    void setVisuallyOrdered(bool b) { SET_FLAG(inheritedFlags, visuallyOrdered, b); }

    bool isStyleAvailable() const;

    bool hasAnyPublicPseudoStyles() const;
    bool hasPseudoStyle(PseudoId pseudo) const;
    void setHasPseudoStyle(PseudoId pseudo);

    // attribute getter methods

    EDisplay display() const { return static_cast<EDisplay>(m_noninheritedFlags.m_effectiveDisplay); }
    EDisplay originalDisplay() const { return static_cast<EDisplay>(m_noninheritedFlags.m_originalDisplay); }

    Length left() const { return m_surround->m_offset.left(); }
    Length right() const { return m_surround->m_offset.right(); }
    Length top() const { return m_surround->m_offset.top(); }
    Length bottom() const { return m_surround->m_offset.bottom(); }

    // Accessors for positioned object edges that take into account writing mode.
    Length logicalLeft() const { return isHorizontalWritingMode() ? left() : top(); }
    Length logicalRight() const { return isHorizontalWritingMode() ? right() : bottom(); }
    Length logicalTop() const { return isHorizontalWritingMode() ? (isFlippedBlocksWritingMode() ? bottom() : top()) : (isFlippedBlocksWritingMode() ? right() : left()); }
    Length logicalBottom() const { return isHorizontalWritingMode() ? (isFlippedBlocksWritingMode() ? top() : bottom()) : (isFlippedBlocksWritingMode() ? left() : right()); }

    // Whether or not a positioned element requires normal flow x/y to be computed
    // to determine its position.
    bool hasAutoLeftAndRight() const { return left().isAuto() && right().isAuto(); }
    bool hasAutoTopAndBottom() const { return top().isAuto() && bottom().isAuto(); }
    bool hasStaticInlinePosition(bool horizontal) const { return horizontal ? hasAutoLeftAndRight() : hasAutoTopAndBottom(); }
    bool hasStaticBlockPosition(bool horizontal) const { return horizontal ? hasAutoTopAndBottom() : hasAutoLeftAndRight(); }

    EPosition position() const { return static_cast<EPosition>(m_noninheritedFlags.m_position); }
    EFloat floating() const { return static_cast<EFloat>(m_noninheritedFlags.m_floating); }

    Length width() const { return m_box->width(); }
    Length height() const { return m_box->height(); }
    Length minWidth() const { return m_box->minWidth(); }
    Length maxWidth() const { return m_box->maxWidth(); }
    Length minHeight() const { return m_box->minHeight(); }
    Length maxHeight() const { return m_box->maxHeight(); }
    
    Length logicalWidth() const;
    Length logicalHeight() const;
    Length logicalMinWidth() const;
    Length logicalMaxWidth() const;
    Length logicalMinHeight() const;
    Length logicalMaxHeight() const;

    const BorderData& border() const { return m_surround->m_border; }
    const BorderValue& borderLeft() const { return m_surround->m_border.left(); }
    const BorderValue& borderRight() const { return m_surround->m_border.right(); }
    const BorderValue& borderTop() const { return m_surround->m_border.top(); }
    const BorderValue& borderBottom() const { return m_surround->m_border.bottom(); }

    const BorderValue& borderBefore() const;
    const BorderValue& borderAfter() const;
    const BorderValue& borderStart() const;
    const BorderValue& borderEnd() const;

    const NinePieceImage& borderImage() const { return m_surround->m_border.image(); }

    const LengthSize& borderTopLeftRadius() const { return m_surround->m_border.topLeft(); }
    const LengthSize& borderTopRightRadius() const { return m_surround->m_border.topRight(); }
    const LengthSize& borderBottomLeftRadius() const { return m_surround->m_border.bottomLeft(); }
    const LengthSize& borderBottomRightRadius() const { return m_surround->m_border.bottomRight(); }
    bool hasBorderRadius() const { return m_surround->m_border.hasBorderRadius(); }

    unsigned short borderLeftWidth() const { return m_surround->m_border.borderLeftWidth(); }
    EBorderStyle borderLeftStyle() const { return m_surround->m_border.left().style(); }
    bool borderLeftIsTransparent() const { return m_surround->m_border.left().isTransparent(); }
    unsigned short borderRightWidth() const { return m_surround->m_border.borderRightWidth(); }
    EBorderStyle borderRightStyle() const { return m_surround->m_border.right().style(); }
    bool borderRightIsTransparent() const { return m_surround->m_border.right().isTransparent(); }
    unsigned short borderTopWidth() const { return m_surround->m_border.borderTopWidth(); }
    EBorderStyle borderTopStyle() const { return m_surround->m_border.top().style(); }
    bool borderTopIsTransparent() const { return m_surround->m_border.top().isTransparent(); }
    unsigned short borderBottomWidth() const { return m_surround->m_border.borderBottomWidth(); }
    EBorderStyle borderBottomStyle() const { return m_surround->m_border.bottom().style(); }
    bool borderBottomIsTransparent() const { return m_surround->m_border.bottom().isTransparent(); }
    
    unsigned short borderBeforeWidth() const;
    unsigned short borderAfterWidth() const;
    unsigned short borderStartWidth() const;
    unsigned short borderEndWidth() const;

    unsigned short outlineSize() const { return max(0, outlineWidth() + outlineOffset()); }
    unsigned short outlineWidth() const
    {
        if (m_background->outline().style() == BNONE)
            return 0;
        return m_background->outline().width();
    }
    bool hasOutline() const { return outlineWidth() > 0 && outlineStyle() > BHIDDEN; }
    EBorderStyle outlineStyle() const { return m_background->outline().style(); }
    bool outlineStyleIsAuto() const { return m_background->outline().isAuto(); }
    
    EOverflow overflowX() const { return static_cast<EOverflow>(m_noninheritedFlags.m_overflowX); }
    EOverflow overflowY() const { return static_cast<EOverflow>(m_noninheritedFlags.m_overflowY); }

    EVisibility visibility() const { return static_cast<EVisibility>(m_inheritedFlags.m_visibility); }
    EVerticalAlign verticalAlign() const { return static_cast<EVerticalAlign>(m_noninheritedFlags.m_verticalAlign); }
    Length verticalAlignLength() const { return m_box->verticalAlign(); }

    Length clipLeft() const { return m_visual->m_clip.left(); }
    Length clipRight() const { return m_visual->m_clip.right(); }
    Length clipTop() const { return m_visual->m_clip.top(); }
    Length clipBottom() const { return m_visual->m_clip.bottom(); }
    LengthBox clip() const { return m_visual->m_clip; }
    bool hasClip() const { return m_visual->m_hasClip; }

    EUnicodeBidi unicodeBidi() const { return static_cast<EUnicodeBidi>(m_noninheritedFlags.m_unicodeBidi); }

    EClear clear() const { return static_cast<EClear>(m_noninheritedFlags.m_clear); }
    ETableLayout tableLayout() const { return static_cast<ETableLayout>(m_noninheritedFlags.m_tableLayout); }

    const Font& font() const { return m_inherited->m_font; }
    const FontMetrics& fontMetrics() const { return m_inherited->m_font.fontMetrics(); }
    const FontDescription& fontDescription() const { return m_inherited->m_font.fontDescription(); }
    int fontSize() const { return m_inherited->m_font.pixelSize(); }

    Length textIndent() const { return m_rareInheritedData->m_indent; }
    ETextAlign textAlign() const { return static_cast<ETextAlign>(m_inheritedFlags.m_textAlign); }
    ETextTransform textTransform() const { return static_cast<ETextTransform>(m_inheritedFlags.m_textTransform); }
    int textDecorationsInEffect() const { return m_inheritedFlags.m_textDecorations; }
    int textDecoration() const { return m_visual->m_textDecoration; }
    int wordSpacing() const { return m_inherited->m_font.wordSpacing(); }
    int letterSpacing() const { return m_inherited->m_font.letterSpacing(); }

    float zoom() const { return m_visual->m_zoom; }
    float effectiveZoom() const { return m_rareInheritedData->m_effectiveZoom; }

    TextDirection direction() const { return static_cast<TextDirection>(m_inheritedFlags.m_direction); }
    bool isLeftToRightDirection() const { return direction() == LTR; }

    Length lineHeight() const { return m_inherited->m_lineHeight; }
    int computedLineHeight() const
    {
        Length lh = lineHeight();

        // Negative value means the line height is not set.  Use the font's built-in spacing.
        if (lh.isNegative())
            return fontMetrics().lineSpacing();

        if (lh.isPercent())
            return lh.calcMinValue(fontSize());

        return lh.value();
    }

    EWhiteSpace whiteSpace() const { return static_cast<EWhiteSpace>(m_inheritedFlags.m_whiteSpace); }
    static bool autoWrap(EWhiteSpace ws)
    {
        // Nowrap and pre don't automatically wrap.
        return ws != NOWRAP && ws != PRE;
    }

    bool autoWrap() const
    {
        return autoWrap(whiteSpace());
    }

    static bool preserveNewline(EWhiteSpace ws)
    {
        // Normal and nowrap do not preserve newlines.
        return ws != NORMAL && ws != NOWRAP;
    }

    bool preserveNewline() const
    {
        return preserveNewline(whiteSpace());
    }

    static bool collapseWhiteSpace(EWhiteSpace ws)
    {
        // Pre and prewrap do not collapse whitespace.
        return ws != PRE && ws != PRE_WRAP;
    }

    bool collapseWhiteSpace() const
    {
        return collapseWhiteSpace(whiteSpace());
    }

    bool isCollapsibleWhiteSpace(UChar c) const
    {
        switch (c) {
            case ' ':
            case '\t':
                return collapseWhiteSpace();
            case '\n':
                return !preserveNewline();
        }
        return false;
    }

    bool breakOnlyAfterWhiteSpace() const
    {
        return whiteSpace() == PRE_WRAP || khtmlLineBreak() == AFTER_WHITE_SPACE;
    }

    bool breakWords() const
    {
        return wordBreak() == BreakWordBreak || wordWrap() == BreakWordWrap;
    }

    EFillRepeat backgroundRepeatX() const { return static_cast<EFillRepeat>(m_background->background().repeatX()); }
    EFillRepeat backgroundRepeatY() const { return static_cast<EFillRepeat>(m_background->background().repeatY()); }
    CompositeOperator backgroundComposite() const { return static_cast<CompositeOperator>(m_background->background().composite()); }
    EFillAttachment backgroundAttachment() const { return static_cast<EFillAttachment>(m_background->background().attachment()); }
    EFillBox backgroundClip() const { return static_cast<EFillBox>(m_background->background().clip()); }
    EFillBox backgroundOrigin() const { return static_cast<EFillBox>(m_background->background().origin()); }
    Length backgroundXPosition() const { return m_background->background().xPosition(); }
    Length backgroundYPosition() const { return m_background->background().yPosition(); }
    EFillSizeType backgroundSizeType() const { return m_background->background().sizeType(); }
    LengthSize backgroundSizeLength() const { return m_background->background().sizeLength(); }
    FillLayer* accessBackgroundLayers() { return &(m_background.access()->m_background); }
    const FillLayer* backgroundLayers() const { return &(m_background->background()); }

    StyleImage* maskImage() const { return m_rareNonInheritedData->m_mask.image(); }
    EFillRepeat maskRepeatX() const { return static_cast<EFillRepeat>(m_rareNonInheritedData->m_mask.repeatX()); }
    EFillRepeat maskRepeatY() const { return static_cast<EFillRepeat>(m_rareNonInheritedData->m_mask.repeatY()); }
    CompositeOperator maskComposite() const { return static_cast<CompositeOperator>(m_rareNonInheritedData->m_mask.composite()); }
    EFillAttachment maskAttachment() const { return static_cast<EFillAttachment>(m_rareNonInheritedData->m_mask.attachment()); }
    EFillBox maskClip() const { return static_cast<EFillBox>(m_rareNonInheritedData->m_mask.clip()); }
    EFillBox maskOrigin() const { return static_cast<EFillBox>(m_rareNonInheritedData->m_mask.origin()); }
    Length maskXPosition() const { return m_rareNonInheritedData->m_mask.xPosition(); }
    Length maskYPosition() const { return m_rareNonInheritedData->m_mask.yPosition(); }
    EFillSizeType maskSizeType() const { return m_rareNonInheritedData->m_mask.sizeType(); }
    LengthSize maskSizeLength() const { return m_rareNonInheritedData->m_mask.sizeLength(); }
    FillLayer* accessMaskLayers() { return &(m_rareNonInheritedData.access()->m_mask); }
    const FillLayer* maskLayers() const { return &(m_rareNonInheritedData->m_mask); }
    const NinePieceImage& maskBoxImage() const { return m_rareNonInheritedData->m_maskBoxImage; }

    EBorderCollapse borderCollapse() const { return static_cast<EBorderCollapse>(m_inheritedFlags.m_borderCollapse); }
    short horizontalBorderSpacing() const { return m_inherited->m_horizontalBorderSpacing; }
    short verticalBorderSpacing() const { return m_inherited->m_verticalBorderSpacing; }
    EEmptyCell emptyCells() const { return static_cast<EEmptyCell>(m_inheritedFlags.m_emptyCells); }
    ECaptionSide captionSide() const { return static_cast<ECaptionSide>(m_inheritedFlags.m_captionSide); }

    short counterIncrement() const { return m_rareNonInheritedData->m_counterIncrement; }
    short counterReset() const { return m_rareNonInheritedData->m_counterReset; }

    EListStyleType listStyleType() const { return static_cast<EListStyleType>(m_inheritedFlags.m_listStyleType); }
    StyleImage* listStyleImage() const { return m_inherited->m_listStyleImage.get(); }
    EListStylePosition listStylePosition() const { return static_cast<EListStylePosition>(m_inheritedFlags.m_listStylePosition); }

    Length marginTop() const { return m_surround->m_margin.top(); }
    Length marginBottom() const { return m_surround->m_margin.bottom(); }
    Length marginLeft() const { return m_surround->m_margin.left(); }
    Length marginRight() const { return m_surround->m_margin.right(); }
    Length marginBefore() const;
    Length marginAfter() const;
    Length marginStart() const;
    Length marginEnd() const;
    Length marginStartUsing(const RenderStyle* otherStyle) const;
    Length marginEndUsing(const RenderStyle* otherStyle) const;
    Length marginBeforeUsing(const RenderStyle* otherStyle) const;
    Length marginAfterUsing(const RenderStyle* otherStyle) const;

    LengthBox paddingBox() const { return m_surround->m_padding; }
    Length paddingTop() const { return m_surround->m_padding.top(); }
    Length paddingBottom() const { return m_surround->m_padding.bottom(); }
    Length paddingLeft() const { return m_surround->m_padding.left(); }
    Length paddingRight() const { return m_surround->m_padding.right(); }
    Length paddingBefore() const;
    Length paddingAfter() const;
    Length paddingStart() const;
    Length paddingEnd() const;

    ECursor cursor() const { return static_cast<ECursor>(m_inheritedFlags.m_cursorStyle); }

    CursorList* cursors() const { return m_rareInheritedData->m_cursorData.get(); }

    EInsideLink insideLink() const { return static_cast<EInsideLink>(m_inheritedFlags.m_insideLink); }
    bool isLink() const { return m_noninheritedFlags.m_isLink; }

    short widows() const { return m_rareInheritedData->m_widows; }
    short orphans() const { return m_rareInheritedData->m_orphans; }
    EPageBreak pageBreakInside() const { return static_cast<EPageBreak>(m_noninheritedFlags.m_pageBreakInside); }
    EPageBreak pageBreakBefore() const { return static_cast<EPageBreak>(m_noninheritedFlags.m_pageBreakBefore); }
    EPageBreak pageBreakAfter() const { return static_cast<EPageBreak>(m_noninheritedFlags.m_pageBreakAfter); }

    // CSS3 Getter Methods

    int outlineOffset() const
    {
        if (m_background->outline().style() == BNONE)
            return 0;
        return m_background->outline().offset();
    }

    const ShadowData* textShadow() const { return m_rareInheritedData->m_textShadow.get(); }
    void getTextShadowExtent(int& top, int& right, int& bottom, int& left) const { getShadowExtent(textShadow(), top, right, bottom, left); }
    void getTextShadowHorizontalExtent(int& left, int& right) const { getShadowHorizontalExtent(textShadow(), left, right); }
    void getTextShadowVerticalExtent(int& top, int& bottom) const { getShadowVerticalExtent(textShadow(), top, bottom); }
    void getTextShadowInlineDirectionExtent(int& logicalLeft, int& logicalRight) { getShadowInlineDirectionExtent(textShadow(), logicalLeft, logicalRight); }
    void getTextShadowBlockDirectionExtent(int& logicalTop, int& logicalBottom) { getShadowBlockDirectionExtent(textShadow(), logicalTop, logicalBottom); }

    float textStrokeWidth() const { return m_rareInheritedData->m_textStrokeWidth; }
    ColorSpace colorSpace() const { return static_cast<ColorSpace>(m_rareInheritedData->m_colorSpace); }
    float opacity() const { return m_rareNonInheritedData->m_opacity; }
    ControlPart appearance() const { return static_cast<ControlPart>(m_rareNonInheritedData->m_appearance); }
    EBoxAlignment boxAlign() const { return static_cast<EBoxAlignment>(m_rareNonInheritedData->m_flexibleBox->m_align); }
    EBoxDirection boxDirection() const { return static_cast<EBoxDirection>(m_inheritedFlags.m_boxDirection); }
    float boxFlex() { return m_rareNonInheritedData->m_flexibleBox->m_flex; }
    unsigned int boxFlexGroup() const { return m_rareNonInheritedData->m_flexibleBox->m_flexGroup; }
    EBoxLines boxLines() { return static_cast<EBoxLines>(m_rareNonInheritedData->m_flexibleBox->m_lines); }
    unsigned int boxOrdinalGroup() const { return m_rareNonInheritedData->m_flexibleBox->m_ordinalGroup; }
    EBoxOrient boxOrient() const { return static_cast<EBoxOrient>(m_rareNonInheritedData->m_flexibleBox->m_orient); }
    EBoxAlignment boxPack() const { return static_cast<EBoxAlignment>(m_rareNonInheritedData->m_flexibleBox->m_pack); }

    const ShadowData* boxShadow() const { return m_rareNonInheritedData->m_boxShadow.get(); }
    void getBoxShadowExtent(int& top, int& right, int& bottom, int& left) const { getShadowExtent(boxShadow(), top, right, bottom, left); }
    void getBoxShadowHorizontalExtent(int& left, int& right) const { getShadowHorizontalExtent(boxShadow(), left, right); }
    void getBoxShadowVerticalExtent(int& top, int& bottom) const { getShadowVerticalExtent(boxShadow(), top, bottom); }
    void getBoxShadowInlineDirectionExtent(int& logicalLeft, int& logicalRight) { getShadowInlineDirectionExtent(boxShadow(), logicalLeft, logicalRight); }
    void getBoxShadowBlockDirectionExtent(int& logicalTop, int& logicalBottom) { getShadowBlockDirectionExtent(boxShadow(), logicalTop, logicalBottom); }

    StyleReflection* boxReflect() const { return m_rareNonInheritedData->m_boxReflect.get(); }
    EBoxSizing boxSizing() const { return m_box->boxSizing(); }
    Length marqueeIncrement() const { return m_rareNonInheritedData->m_marquee->m_increment; }
    int marqueeSpeed() const { return m_rareNonInheritedData->m_marquee->m_speed; }
    int marqueeLoopCount() const { return m_rareNonInheritedData->m_marquee->m_loops; }
    EMarqueeBehavior marqueeBehavior() const { return static_cast<EMarqueeBehavior>(m_rareNonInheritedData->m_marquee->m_behavior); }
    EMarqueeDirection marqueeDirection() const { return static_cast<EMarqueeDirection>(m_rareNonInheritedData->m_marquee->m_direction); }
    EUserModify userModify() const { return static_cast<EUserModify>(m_rareInheritedData->m_userModify); }
    EUserDrag userDrag() const { return static_cast<EUserDrag>(m_rareNonInheritedData->m_userDrag); }
    EUserSelect userSelect() const { return static_cast<EUserSelect>(m_rareInheritedData->m_userSelect); }
    bool textOverflow() const { return m_rareNonInheritedData->m_textOverflow; }
    EMarginCollapse marginBeforeCollapse() const { return static_cast<EMarginCollapse>(m_rareNonInheritedData->m_marginBeforeCollapse); }
    EMarginCollapse marginAfterCollapse() const { return static_cast<EMarginCollapse>(m_rareNonInheritedData->m_marginAfterCollapse); }
    EWordBreak wordBreak() const { return static_cast<EWordBreak>(m_rareInheritedData->m_wordBreak); }
    EWordWrap wordWrap() const { return static_cast<EWordWrap>(m_rareInheritedData->m_wordWrap); }
    ENBSPMode nbspMode() const { return static_cast<ENBSPMode>(m_rareInheritedData->m_nbspMode); }
    EKHTMLLineBreak khtmlLineBreak() const { return static_cast<EKHTMLLineBreak>(m_rareInheritedData->m_khtmlLineBreak); }
    EMatchNearestMailBlockquoteColor matchNearestMailBlockquoteColor() const { return static_cast<EMatchNearestMailBlockquoteColor>(m_rareNonInheritedData->m_matchNearestMailBlockquoteColor); }
    const AtomicString& highlight() const { return m_rareInheritedData->m_highlight; }
    Hyphens hyphens() const { return static_cast<Hyphens>(m_rareInheritedData->m_hyphens); }
    short hyphenationLimitBefore() const { return m_rareInheritedData->m_hyphenationLimitBefore; }
    short hyphenationLimitAfter() const { return m_rareInheritedData->m_hyphenationLimitAfter; }
    const AtomicString& hyphenationString() const { return m_rareInheritedData->m_hyphenationString; }
    const AtomicString& locale() const { return m_rareInheritedData->m_locale; }
    EBorderFit borderFit() const { return static_cast<EBorderFit>(m_rareNonInheritedData->m_borderFit); }
    EResize resize() const { return static_cast<EResize>(m_rareInheritedData->m_resize); }
    float columnWidth() const { return m_rareNonInheritedData->m_multiCol->m_width; }
    bool hasAutoColumnWidth() const { return m_rareNonInheritedData->m_multiCol->m_autoWidth; }
    unsigned short columnCount() const { return m_rareNonInheritedData->m_multiCol->m_count; }
    bool hasAutoColumnCount() const { return m_rareNonInheritedData->m_multiCol->m_autoCount; }
    bool specifiesColumns() const { return !hasAutoColumnCount() || !hasAutoColumnWidth(); }
    float columnGap() const { return m_rareNonInheritedData->m_multiCol->m_gap; }
    bool hasNormalColumnGap() const { return m_rareNonInheritedData->m_multiCol->m_normalGap; }
    EBorderStyle columnRuleStyle() const { return m_rareNonInheritedData->m_multiCol->m_rule.style(); }
    unsigned short columnRuleWidth() const { return m_rareNonInheritedData->m_multiCol->ruleWidth(); }
    bool columnRuleIsTransparent() const { return m_rareNonInheritedData->m_multiCol->m_rule.isTransparent(); }
    bool columnSpan() const { return m_rareNonInheritedData->m_multiCol->m_columnSpan; }
    EPageBreak columnBreakBefore() const { return static_cast<EPageBreak>(m_rareNonInheritedData->m_multiCol->m_breakBefore); }
    EPageBreak columnBreakInside() const { return static_cast<EPageBreak>(m_rareNonInheritedData->m_multiCol->m_breakInside); }
    EPageBreak columnBreakAfter() const { return static_cast<EPageBreak>(m_rareNonInheritedData->m_multiCol->m_breakAfter); }
    const TransformOperations& transform() const { return m_rareNonInheritedData->m_transform->m_operations; }
    Length transformOriginX() const { return m_rareNonInheritedData->m_transform->m_x; }
    Length transformOriginY() const { return m_rareNonInheritedData->m_transform->m_y; }
    float transformOriginZ() const { return m_rareNonInheritedData->m_transform->m_z; }
    bool hasTransform() const { return !m_rareNonInheritedData->m_transform->m_operations.operations().isEmpty(); }

    TextEmphasisFill textEmphasisFill() const { return static_cast<TextEmphasisFill>(m_rareInheritedData->m_textEmphasisFill); }
    TextEmphasisMark textEmphasisMark() const;
    const AtomicString& textEmphasisCustomMark() const { return m_rareInheritedData->m_textEmphasisCustomMark; }
    TextEmphasisPosition textEmphasisPosition() const { return static_cast<TextEmphasisPosition>(m_rareInheritedData->m_textEmphasisPosition); }
    const AtomicString& textEmphasisMarkString() const;
    
    // Return true if any transform related property (currently transform, transformStyle3D or perspective) 
    // indicates that we are transforming
    bool hasTransformRelatedProperty() const { return hasTransform() || preserves3D() || hasPerspective(); }

    enum ApplyTransformOrigin { IncludeTransformOrigin, ExcludeTransformOrigin };
    void applyTransform(TransformationMatrix&, const IntSize& borderBoxSize, ApplyTransformOrigin = IncludeTransformOrigin) const;
    void setPageScaleTransform(float);

    bool hasMask() const { return m_rareNonInheritedData->m_mask.hasImage() || m_rareNonInheritedData->m_maskBoxImage.hasImage(); }

    TextCombine textCombine() const { return static_cast<TextCombine>(m_rareNonInheritedData->m_textCombine); }
    bool hasTextCombine() const { return textCombine() != TextCombineNone; }
    // End CSS3 Getters

    // Apple-specific property getter methods
    EPointerEvents pointerEvents() const { return static_cast<EPointerEvents>(m_inheritedFlags.m_pointerEvents); }
    const AnimationList* animations() const { return m_rareNonInheritedData->m_animations.get(); }
    const AnimationList* transitions() const { return m_rareNonInheritedData->m_transitions.get(); }

    AnimationList* accessAnimations();
    AnimationList* accessTransitions();

    bool hasAnimations() const { return m_rareNonInheritedData->m_animations && m_rareNonInheritedData->m_animations->size() > 0; }
    bool hasTransitions() const { return m_rareNonInheritedData->m_transitions && m_rareNonInheritedData->m_transitions->size() > 0; }

    // return the first found Animation (including 'all' transitions)
    const Animation* transitionForProperty(int property) const;

    ETransformStyle3D transformStyle3D() const { return m_rareNonInheritedData->m_transformStyle3D; }
    bool preserves3D() const { return m_rareNonInheritedData->m_transformStyle3D == TransformStyle3DPreserve3D; }

    EBackfaceVisibility backfaceVisibility() const { return m_rareNonInheritedData->m_backfaceVisibility; }
    float perspective() const { return m_rareNonInheritedData->m_perspective; }
    bool hasPerspective() const { return m_rareNonInheritedData->m_perspective > 0; }
    Length perspectiveOriginX() const { return m_rareNonInheritedData->m_perspectiveOriginX; }
    Length perspectiveOriginY() const { return m_rareNonInheritedData->m_perspectiveOriginY; }
    LengthSize pageSize() const { return m_rareNonInheritedData->m_pageSize; }
    PageSizeType pageSizeType() const { return m_rareNonInheritedData->m_pageSizeType; }
    
#if USE(ACCELERATED_COMPOSITING)
    // When set, this ensures that styles compare as different. Used during accelerated animations.
    bool isRunningAcceleratedAnimation() const { return m_rareNonInheritedData->m_runningAcceleratedAnimation; }
#endif

    LineBoxContain lineBoxContain() const { return m_rareInheritedData->m_lineBoxContain; }
    const LineClampValue& lineClamp() const { return m_rareNonInheritedData->m_lineClamp; }
    bool textSizeAdjust() const { return m_rareInheritedData->m_textSizeAdjust; }
    ETextSecurity textSecurity() const { return static_cast<ETextSecurity>(m_rareInheritedData->m_textSecurity); }

    WritingMode writingMode() const { return static_cast<WritingMode>(m_inheritedFlags.m_writingMode); }
    bool isHorizontalWritingMode() const { return writingMode() == TopToBottomWritingMode || writingMode() == BottomToTopWritingMode; }
    bool isFlippedLinesWritingMode() const { return writingMode() == LeftToRightWritingMode || writingMode() == BottomToTopWritingMode; }
    bool isFlippedBlocksWritingMode() const { return writingMode() == RightToLeftWritingMode || writingMode() == BottomToTopWritingMode; }

    ESpeak speak() { return static_cast<ESpeak>(m_rareInheritedData->m_speak); }
        
// attribute setter methods

    void setDisplay(EDisplay v) { SET_FLAG(noninheritedFlags, effectiveDisplay, v); }
    void setOriginalDisplay(EDisplay v) { SET_FLAG(noninheritedFlags, originalDisplay, v); }
    void setPosition(EPosition v) { SET_FLAG(noninheritedFlags, position, v); }
    void setFloating(EFloat v) { SET_FLAG(noninheritedFlags, floating, v); }

    void setLeft(Length v) { SET_VAR_GEN_SIG2(surround, offset, left, v); }
    void setRight(Length v) { SET_VAR_GEN_SIG2(surround, offset, right, v); }
    void setTop(Length v) { SET_VAR_GEN_SIG2(surround, offset, top, v); };
    void setBottom(Length v) { SET_VAR_GEN_SIG2(surround, offset, bottom, v); }

    void setWidth(Length v) { SET_VAR_GEN_SIG(box, width, v); }
    void setHeight(Length v) { SET_VAR_GEN_SIG(box, height, v); }

    void setMinWidth(Length v) { SET_VAR_GEN_SIG(box, minWidth, v); }
    void setMaxWidth(Length v) { SET_VAR_GEN_SIG(box, maxWidth, v); }
    void setMinHeight(Length v) { SET_VAR_GEN_SIG(box, minHeight, v); }
    void setMaxHeight(Length v) { SET_VAR_GEN_SIG(box, maxHeight, v); }

#if ENABLE(DASHBOARD_SUPPORT)
    Vector<StyleDashboardRegion> dashboardRegions() const { return m_rareNonInheritedData->m_dashboardRegions; }
    void setDashboardRegions(Vector<StyleDashboardRegion> regions) { SET_VAR_GEN_SIG(rareNonInheritedData, dashboardRegions, regions); }

    void setDashboardRegion(int type, const String& label, Length t, Length r, Length b, Length l, bool append)
    {
        StyleDashboardRegion region;
        region.label = label;
        region.offset.m_top = t;
        region.offset.m_right = r;
        region.offset.m_bottom = b;
        region.offset.m_left = l;
        region.type = type;
        if (!append)
            m_rareNonInheritedData.access()->m_dashboardRegions.clear();
        m_rareNonInheritedData.access()->m_dashboardRegions.append(region);
    }
#endif

    void resetBorder() { resetBorderImage(); resetBorderTop(); resetBorderRight(); resetBorderBottom(); resetBorderLeft(); resetBorderRadius(); }
    void resetBorderTop() { SET_VAR_GEN_SIG2(surround, border, top, BorderValue()); }
    void resetBorderRight() { SET_VAR_GEN_SIG2(surround, border, right, BorderValue()); }
    void resetBorderBottom() { SET_VAR_GEN_SIG2(surround, border, bottom, BorderValue()); }
    void resetBorderLeft() { SET_VAR_GEN_SIG2(surround, border, left, BorderValue()); }
    void resetBorderImage() { SET_VAR_GEN_SIG2(surround, border, image, NinePieceImage()); }
    void resetBorderRadius() { resetBorderTopLeftRadius(); resetBorderTopRightRadius(); resetBorderBottomLeftRadius(); resetBorderBottomRightRadius(); }
    void resetBorderTopLeftRadius() { SET_VAR_GEN_SIG2(surround, border, topLeft, initialBorderRadius()); }
    void resetBorderTopRightRadius() { SET_VAR_GEN_SIG2(surround, border, topRight, initialBorderRadius()); }
    void resetBorderBottomLeftRadius() { SET_VAR_GEN_SIG2(surround, border, bottomLeft, initialBorderRadius()); }
    void resetBorderBottomRightRadius() { SET_VAR_GEN_SIG2(surround, border, bottomRight, initialBorderRadius()); }

    void resetOutline() { SET_VAR_GEN_SIG(background, outline, OutlineValue()); }

    void setBackgroundColor(const Color& v) { SET_VAR_GEN_SIG(background, color, v); }

    void setBackgroundXPosition(Length l) { SET_VAR_GEN_SIG2(background, background, xPosition, l); }
    void setBackgroundYPosition(Length l) { SET_VAR_GEN_SIG2(background, background, yPosition, l); }
    void setBackgroundSize(EFillSizeType b) { SET_VAR_GEN_SIG2(background, background, sizeType, b); }
    void setBackgroundSizeLength(LengthSize l) { SET_VAR_GEN_SIG2(background, background, sizeLength, l); }
    
    void setBorderImage(const NinePieceImage& b) { SET_VAR_GEN_SIG2(surround, border, image, b); }

    void setBorderTopLeftRadius(const LengthSize& s) { SET_VAR_GEN_SIG2(surround, border, topLeft, s); }
    void setBorderTopRightRadius(const LengthSize& s) { SET_VAR_GEN_SIG2(surround, border, topRight, s); }
    void setBorderBottomLeftRadius(const LengthSize& s) { SET_VAR_GEN_SIG2(surround, border, bottomLeft, s); }
    void setBorderBottomRightRadius(const LengthSize& s) { SET_VAR_GEN_SIG2(surround, border, bottomRight, s); }

    void setBorderRadius(const LengthSize& s)
    {
        setBorderTopLeftRadius(s);
        setBorderTopRightRadius(s);
        setBorderBottomLeftRadius(s);
        setBorderBottomRightRadius(s);
    }
    void setBorderRadius(const IntSize& s)
    {
        setBorderRadius(LengthSize(Length(s.width(), Fixed), Length(s.height(), Fixed)));
    }
    
    RoundedIntRect getRoundedBorderFor(const IntRect& borderRect, bool includeLogicalLeftEdge = true, bool includeLogicalRightEdge = true) const;
    RoundedIntRect getRoundedInnerBorderFor(const IntRect& borderRect, bool includeLogicalLeftEdge = true, bool includeLogicalRightEdge = true) const;

    RoundedIntRect getRoundedInnerBorderFor(const IntRect& borderRect,
        int topWidth, int bottomWidth, int leftWidth, int rightWidth, bool includeLogicalLeftEdge, bool includeLogicalRightEdge) const;

    void setBorderLeftWidth(unsigned short v) { SET_VAR_GEN_SIG4(surround, border, left, width, v); }
    void setBorderLeftStyle(EBorderStyle v) { SET_VAR_GEN_SIG4(surround, border, left, style, v); }
    void setBorderLeftColor(const Color& v) { SET_VAR_GEN_SIG4(surround, border, left, color, v); }
    void setBorderRightWidth(unsigned short v) { SET_VAR_GEN_SIG4(surround, border, right, width, v); }
    void setBorderRightStyle(EBorderStyle v) { SET_VAR_GEN_SIG4(surround, border, right, style, v); }
    void setBorderRightColor(const Color& v) { SET_VAR_GEN_SIG4(surround, border, right, color, v); }
    void setBorderTopWidth(unsigned short v) { SET_VAR_GEN_SIG4(surround, border, top, width, v); }
    void setBorderTopStyle(EBorderStyle v) { SET_VAR_GEN_SIG4(surround, border, top, style, v); }
    void setBorderTopColor(const Color& v) { SET_VAR_GEN_SIG4(surround, border, top, color, v); }
    void setBorderBottomWidth(unsigned short v) { SET_VAR_GEN_SIG4(surround, border, bottom, width, v); }
    void setBorderBottomStyle(EBorderStyle v) { SET_VAR_GEN_SIG4(surround, border, bottom, style, v); }
    void setBorderBottomColor(const Color& v) { SET_VAR_GEN_SIG4(surround, border, bottom, color, v); }
    void setOutlineWidth(unsigned short v) { SET_VAR_GEN_SIG2(background, outline, width, v); }

    void setOutlineStyle(EBorderStyle v, bool isAuto = false)
    {
        SET_VAR_GEN_SIG2(background, outline, style, v);
        SET_VAR_GEN_SIG2(background, outline, isAuto, isAuto);
    }

    void setOutlineColor(const Color& v) { SET_VAR_GEN_SIG2(background, outline, color, v); }

    void setOverflowX(EOverflow v) { SET_FLAG(noninheritedFlags, overflowX, v); }
    void setOverflowY(EOverflow v) { SET_FLAG(noninheritedFlags, overflowY, v); }
    void setVisibility(EVisibility v) { SET_FLAG(inheritedFlags, visibility, v); }
    void setVerticalAlign(EVerticalAlign v) { SET_FLAG(noninheritedFlags, verticalAlign, v); }
    void setVerticalAlignLength(Length l) { SET_VAR_GEN_SIG(box, verticalAlign, l); }

    void setHasClip(bool b = true) { SET_VAR_GEN_SIG(visual, hasClip, b); }
    void setClipLeft(Length v) { SET_VAR_GEN_SIG2(visual, clip, left, v); }
    void setClipRight(Length v) { SET_VAR_GEN_SIG2(visual, clip, right, v); }
    void setClipTop(Length v) { SET_VAR_GEN_SIG2(visual, clip, top, v); }
    void setClipBottom(Length v) { SET_VAR_GEN_SIG2(visual, clip, bottom, v); }
    void setClip(Length top, Length right, Length bottom, Length left);
    void setClip(LengthBox box) { SET_VAR_GEN_SIG(visual, clip, box); }

    void setUnicodeBidi(EUnicodeBidi b) { SET_FLAG(noninheritedFlags, unicodeBidi, b); }

    void setClear(EClear v) { SET_FLAG(noninheritedFlags, clear, v); }
    void setTableLayout(ETableLayout v) { SET_FLAG(noninheritedFlags, tableLayout, v); }

    bool setFontDescription(const FontDescription& v)
    {
        if (m_inherited->m_font.fontDescription() != v) {
            m_inherited.access()->m_font = Font(v, m_inherited->m_font.letterSpacing(), m_inherited->m_font.wordSpacing());
            return true;
        }
        return false;
    }

    // Only used for blending font sizes when animating.
    void setBlendedFontSize(int);

    void setColor(const Color& v) { SET_VAR_GEN_SIG(inherited, color, v); }
    void setTextIndent(Length v) { SET_VAR_GEN_SIG(rareInheritedData, indent, v); }
    void setTextAlign(ETextAlign v) { SET_FLAG(inheritedFlags, textAlign, v); }
    void setTextTransform(ETextTransform v) { SET_FLAG(inheritedFlags, textTransform, v); }
    void addToTextDecorationsInEffect(int v) { SET_FLAG(inheritedFlags, textDecorations, m_inheritedFlags.m_textDecorations | v); }
    void setTextDecorationsInEffect(int v) { SET_FLAG(inheritedFlags, textDecorations, v); }
    void setTextDecoration(int v) { SET_VAR_GEN_SIG(visual, textDecoration, v); }
    void setDirection(TextDirection v) { SET_FLAG(inheritedFlags, direction, v); }
    void setLineHeight(Length v) { SET_VAR_GEN_SIG(inherited, lineHeight, v); }
    void setZoom(float f) { SET_VAR_GEN_SIG(visual, zoom, f); setEffectiveZoom(effectiveZoom() * zoom()); }
    void setEffectiveZoom(float f) { SET_VAR_GEN_SIG(rareInheritedData, effectiveZoom, f); }

    void setWhiteSpace(EWhiteSpace v) { SET_FLAG(inheritedFlags, whiteSpace, v); }

    void setWordSpacing(int v) { m_inherited.access()->m_font.setWordSpacing(v); }
    void setLetterSpacing(int v) { m_inherited.access()->m_font.setLetterSpacing(v); }

    void clearBackgroundLayers() { m_background.access()->m_background = FillLayer(BackgroundFillLayer); }
    void inheritBackgroundLayers(const FillLayer& parent) { m_background.access()->m_background = parent; }

    void adjustBackgroundLayers()
    {
        if (backgroundLayers()->next()) {
            accessBackgroundLayers()->cullEmptyLayers();
            accessBackgroundLayers()->fillUnsetProperties();
        }
    }

    void clearMaskLayers() { m_rareNonInheritedData.access()->m_mask = FillLayer(MaskFillLayer); }
    void inheritMaskLayers(const FillLayer& parent) { m_rareNonInheritedData.access()->m_mask = parent; }

    void adjustMaskLayers()
    {
        if (maskLayers()->next()) {
            accessMaskLayers()->cullEmptyLayers();
            accessMaskLayers()->fillUnsetProperties();
        }
    }

    void setMaskBoxImage(const NinePieceImage& b) { SET_VAR_GEN_SIG(rareNonInheritedData, maskBoxImage, b); }
    void setMaskXPosition(Length l) { SET_VAR_GEN_SIG2(rareNonInheritedData, mask, xPosition, l); }
    void setMaskYPosition(Length l) { SET_VAR_GEN_SIG2(rareNonInheritedData, mask, yPosition, l); }
    void setMaskSize(LengthSize l) { SET_VAR_GEN_SIG2(rareNonInheritedData, mask, sizeLength, l); }

    void setBorderCollapse(EBorderCollapse collapse) { SET_FLAG(inheritedFlags, borderCollapse, collapse); }
    void setHorizontalBorderSpacing(short v) { SET_VAR_GEN_SIG(inherited, horizontalBorderSpacing, v); }
    void setVerticalBorderSpacing(short v) { SET_VAR_GEN_SIG(inherited, verticalBorderSpacing, v); }
    void setEmptyCells(EEmptyCell v) { SET_FLAG(inheritedFlags, emptyCells, v); }
    void setCaptionSide(ECaptionSide v) { SET_FLAG(inheritedFlags, captionSide, v); }

    void setCounterIncrement(short v) { SET_VAR_GEN_SIG(rareNonInheritedData, counterIncrement, v); }
    void setCounterReset(short v) { SET_VAR_GEN_SIG(rareNonInheritedData, counterReset, v); }

    void setListStyleType(EListStyleType v) { SET_FLAG(inheritedFlags, listStyleType, v); }
    void setListStyleImage(PassRefPtr<StyleImage> v) { if (m_inherited->m_listStyleImage != v) m_inherited.access()->m_listStyleImage = v; }
    void setListStylePosition(EListStylePosition v) { SET_FLAG(inheritedFlags, listStylePosition, v); }

    void resetMargin() { SET_VAR_GEN_SIG(surround, margin, LengthBox(Fixed)); }
    void setMarginTop(Length v) { SET_VAR_GEN_SIG2(surround, margin, top, v); }
    void setMarginBottom(Length v) { SET_VAR_GEN_SIG2(surround, margin, bottom, v); }
    void setMarginLeft(Length v) { SET_VAR_GEN_SIG2(surround, margin, left, v); }
    void setMarginRight(Length v) { SET_VAR_GEN_SIG2(surround, margin, right, v); }
    void setMarginStart(Length);
    void setMarginEnd(Length);

    void resetPadding() { SET_VAR_GEN_SIG(surround, padding, LengthBox(Auto)); }
    void setPaddingBox(const LengthBox& b) { SET_VAR_GEN_SIG(surround, padding, b); }
    void setPaddingTop(Length v) { SET_VAR_GEN_SIG2(surround, padding, top, v); }
    void setPaddingBottom(Length v) { SET_VAR_GEN_SIG2(surround, padding, bottom, v); }
    void setPaddingLeft(Length v) { SET_VAR_GEN_SIG2(surround, padding, left, v); }
    void setPaddingRight(Length v) { SET_VAR_GEN_SIG2(surround, padding, right, v); }

    void setCursor(ECursor c) { SET_FLAG(inheritedFlags, cursorStyle, c); }
    void addCursor(PassRefPtr<StyleImage>, const IntPoint& hotSpot = IntPoint());
    void setCursorList(PassRefPtr<CursorList>);
    void clearCursorList();

    void setInsideLink(EInsideLink insideLink) { SET_FLAG(inheritedFlags, insideLink, insideLink); }
    void setIsLink(bool b) { SET_FLAG(noninheritedFlags, isLink, b); }

    bool forceBackgroundsToWhite() const { return m_inheritedFlags.m_forceBackgroundsToWhite; }
    void setForceBackgroundsToWhite(bool b=true) { SET_FLAG(inheritedFlags, forceBackgroundsToWhite, b); }

    bool hasAutoZIndex() const { return m_box->hasAutoZIndex(); }
    void setHasAutoZIndex() { SET_VAR_GEN_SIG(box, hasAutoZIndex, true); SET_VAR_GEN_SIG(box, zIndex, 0); }
    int zIndex() const { return m_box->zIndex(); }
    void setZIndex(int v) { SET_VAR_GEN_SIG(box, hasAutoZIndex, false); SET_VAR_GEN_SIG(box, zIndex, v); }

    void setWidows(short w) { SET_VAR_GEN_SIG(rareInheritedData, widows, w); }
    void setOrphans(short o) { SET_VAR_GEN_SIG(rareInheritedData, orphans, o); }
    void setPageBreakInside(EPageBreak b) { SET_FLAG(noninheritedFlags, pageBreakInside, b); }
    void setPageBreakBefore(EPageBreak b) { SET_FLAG(noninheritedFlags, pageBreakBefore, b); }
    void setPageBreakAfter(EPageBreak b) { SET_FLAG(noninheritedFlags, pageBreakAfter, b); }

    // CSS3 Setters
    void setOutlineOffset(int v) { SET_VAR_GEN_SIG2(background, outline, offset, v); }
    void setTextShadow(PassOwnPtr<ShadowData>, bool add = false);
    void setTextStrokeColor(const Color& c) { SET_VAR_GEN_SIG(rareInheritedData, textStrokeColor, c); }
    void setTextStrokeWidth(float w) { SET_VAR_GEN_SIG(rareInheritedData, textStrokeWidth, w); }
    void setTextFillColor(const Color& c) { SET_VAR_GEN_SIG(rareInheritedData, textFillColor, c); }
    void setColorSpace(ColorSpace space) { SET_VAR_GEN_SIG(rareInheritedData, colorSpace, space); }
    void setOpacity(float f) { SET_VAR_GEN_SIG(rareNonInheritedData, opacity, f); }
    void setAppearance(ControlPart a) { SET_VAR_GEN_SIG(rareNonInheritedData, appearance, a); }
    void setBoxAlign(EBoxAlignment a) { SET_VAR_GEN_SIG3(rareNonInheritedData, flexibleBox, align, a); }
    void setBoxDirection(EBoxDirection d) { SET_FLAG(inheritedFlags, boxDirection, d); }
    void setBoxFlex(float f) { SET_VAR_GEN_SIG3(rareNonInheritedData, flexibleBox, flex, f); }
    void setBoxFlexGroup(unsigned int fg) { SET_VAR_GEN_SIG3(rareNonInheritedData, flexibleBox, flexGroup, fg); }
    void setBoxLines(EBoxLines l) { SET_VAR_GEN_SIG3(rareNonInheritedData, flexibleBox, lines, l); }
    void setBoxOrdinalGroup(unsigned int og) { SET_VAR_GEN_SIG3(rareNonInheritedData, flexibleBox, ordinalGroup, og); }
    void setBoxOrient(EBoxOrient o) { SET_VAR_GEN_SIG3(rareNonInheritedData, flexibleBox, orient, o); }
    void setBoxPack(EBoxAlignment p) { SET_VAR_GEN_SIG3(rareNonInheritedData, flexibleBox, pack, p); }
    void setBoxShadow(PassOwnPtr<ShadowData>, bool add = false);
    void setBoxReflect(PassRefPtr<StyleReflection> reflect) { if (m_rareNonInheritedData->m_boxReflect != reflect) m_rareNonInheritedData.access()->m_boxReflect = reflect; }
    void setBoxSizing(EBoxSizing s) { SET_VAR_GEN_SIG(box, boxSizing, s); }
    void setMarqueeIncrement(const Length& f) { SET_VAR_GEN_SIG3(rareNonInheritedData, marquee, increment, f); }
    void setMarqueeSpeed(int f) { SET_VAR_GEN_SIG3(rareNonInheritedData, marquee, speed, f); }
    void setMarqueeDirection(EMarqueeDirection d) { SET_VAR_GEN_SIG3(rareNonInheritedData, marquee, direction, d); }
    void setMarqueeBehavior(EMarqueeBehavior b) { SET_VAR_GEN_SIG3(rareNonInheritedData, marquee, behavior, b); }
    void setMarqueeLoopCount(int i) { SET_VAR_GEN_SIG3(rareNonInheritedData, marquee, loops, i); }
    void setUserModify(EUserModify u) { SET_VAR_GEN_SIG(rareInheritedData, userModify, u); }
    void setUserDrag(EUserDrag d) { SET_VAR_GEN_SIG(rareNonInheritedData, userDrag, d); }
    void setUserSelect(EUserSelect s) { SET_VAR_GEN_SIG(rareInheritedData, userSelect, s); }
    void setTextOverflow(bool b) { SET_VAR_GEN_SIG(rareNonInheritedData, textOverflow, b); }
    void setMarginBeforeCollapse(EMarginCollapse c) { SET_VAR_GEN_SIG(rareNonInheritedData, marginBeforeCollapse, c); }
    void setMarginAfterCollapse(EMarginCollapse c) { SET_VAR_GEN_SIG(rareNonInheritedData, marginAfterCollapse, c); }
    void setWordBreak(EWordBreak b) { SET_VAR_GEN_SIG(rareInheritedData, wordBreak, b); }
    void setWordWrap(EWordWrap b) { SET_VAR_GEN_SIG(rareInheritedData, wordWrap, b); }
    void setNBSPMode(ENBSPMode b) { SET_VAR_GEN_SIG(rareInheritedData, nbspMode, b); }
    void setKHTMLLineBreak(EKHTMLLineBreak b) { SET_VAR_GEN_SIG(rareInheritedData, khtmlLineBreak, b); }
    void setMatchNearestMailBlockquoteColor(EMatchNearestMailBlockquoteColor c) { SET_VAR_GEN_SIG(rareNonInheritedData, matchNearestMailBlockquoteColor, c); }
    void setHighlight(const AtomicString& h) { SET_VAR_GEN_SIG(rareInheritedData, highlight, h); }
    void setHyphens(Hyphens h) { SET_VAR_GEN_SIG(rareInheritedData, hyphens, h); }
    void setHyphenationLimitBefore(short limit) { SET_VAR_GEN_SIG(rareInheritedData, hyphenationLimitBefore, limit); }
    void setHyphenationLimitAfter(short limit) { SET_VAR_GEN_SIG(rareInheritedData, hyphenationLimitAfter, limit); }
    void setHyphenationString(const AtomicString& h) { SET_VAR_GEN_SIG(rareInheritedData, hyphenationString, h); }
    void setLocale(const AtomicString& locale) { SET_VAR_GEN_SIG(rareInheritedData, locale, locale); }
    void setBorderFit(EBorderFit b) { SET_VAR_GEN_SIG(rareNonInheritedData, borderFit, b); }
    void setResize(EResize r) { SET_VAR_GEN_SIG(rareInheritedData, resize, r); }
    void setColumnWidth(float f)
    { 
        SET_VAR_GEN_SIG3(rareNonInheritedData, multiCol, autoWidth, false);
        SET_VAR_GEN_SIG3(rareNonInheritedData, multiCol, width, f);
    }

    void setHasAutoColumnWidth()
    {
        SET_VAR_GEN_SIG3(rareNonInheritedData, multiCol, autoWidth, true);
        SET_VAR_GEN_SIG3(rareNonInheritedData, multiCol, width, 0);
    }

    void setColumnCount(unsigned short c)
    {
        SET_VAR_GEN_SIG3(rareNonInheritedData, multiCol, autoCount, false);
        SET_VAR_GEN_SIG3(rareNonInheritedData, multiCol, count, c);
    }

    void setHasAutoColumnCount()
    {
        SET_VAR_GEN_SIG3(rareNonInheritedData, multiCol, autoCount, true);
        SET_VAR_GEN_SIG3(rareNonInheritedData, multiCol, count, 0);
    }

    void setColumnGap(float f)
    {
        SET_VAR_GEN_SIG3(rareNonInheritedData, multiCol, normalGap, false);
        SET_VAR_GEN_SIG3(rareNonInheritedData, multiCol, gap, f);
    }

    void setHasNormalColumnGap()
    {
        SET_VAR_GEN_SIG3(rareNonInheritedData, multiCol, normalGap, true);
        SET_VAR_GEN_SIG3(rareNonInheritedData, multiCol, gap, 0);
    }

    void setColumnRuleColor(const Color& c)
    { 
        SET_VAR(m_rareNonInheritedData.access()->m_multiCol, m_rule.m_color, c);
#if USE(WRATH)
        m_signal(E_rareNonInheritedData_multiCol_rule_color);
#endif
    }

    void setColumnRuleStyle(EBorderStyle b)
    {
        SET_VAR(m_rareNonInheritedData.access()->m_multiCol, m_rule.m_style, b);
#if USE(WRATH)
        m_signal(E_rareNonInheritedData_multiCol_rule_style);
#endif
    }

    void setColumnRuleWidth(unsigned short w)
    {
        SET_VAR(m_rareNonInheritedData.access()->m_multiCol, m_rule.m_width, w);
#if USE(WRATH)
        m_signal(E_rareNonInheritedData_multiCol_rule_width);
#endif
    }

    void resetColumnRule() { SET_VAR_GEN_SIG3(rareNonInheritedData, multiCol, rule, BorderValue()); }
    void setColumnSpan(bool b) { SET_VAR_GEN_SIG3(rareNonInheritedData, multiCol, columnSpan, b); }
    void setColumnBreakBefore(EPageBreak p) { SET_VAR_GEN_SIG3(rareNonInheritedData, multiCol, breakBefore, p); }
    void setColumnBreakInside(EPageBreak p) { SET_VAR_GEN_SIG3(rareNonInheritedData, multiCol, breakInside, p); }
    void setColumnBreakAfter(EPageBreak p) { SET_VAR_GEN_SIG3(rareNonInheritedData, multiCol, breakAfter, p); }
    void inheritColumnPropertiesFrom(RenderStyle* parent) { m_rareNonInheritedData.access()->m_multiCol = parent->m_rareNonInheritedData->m_multiCol; }
    void setTransform(const TransformOperations& ops) { SET_VAR_GEN_SIG3(rareNonInheritedData, transform, operations, ops); }
    void setTransformOriginX(Length l) { SET_VAR_GEN_SIG3(rareNonInheritedData, transform, x, l); }
    void setTransformOriginY(Length l) { SET_VAR_GEN_SIG3(rareNonInheritedData, transform, y, l); }
    void setTransformOriginZ(float f) { SET_VAR_GEN_SIG3(rareNonInheritedData, transform, z, f); }
    void setSpeak(ESpeak s) { SET_VAR_GEN_SIG(rareInheritedData, speak, s); }
    void setTextCombine(TextCombine v) { SET_VAR_GEN_SIG(rareNonInheritedData, textCombine, v); }
    void setTextEmphasisColor(const Color& c) { SET_VAR_GEN_SIG(rareInheritedData, textEmphasisColor, c); }
    void setTextEmphasisFill(TextEmphasisFill fill) { SET_VAR_GEN_SIG(rareInheritedData, textEmphasisFill, fill); }
    void setTextEmphasisMark(TextEmphasisMark mark) { SET_VAR_GEN_SIG(rareInheritedData, textEmphasisMark, mark); }
    void setTextEmphasisCustomMark(const AtomicString& mark) { SET_VAR_GEN_SIG(rareInheritedData, textEmphasisCustomMark, mark); }
    void setTextEmphasisPosition(TextEmphasisPosition position) { SET_VAR_GEN_SIG(rareInheritedData, textEmphasisPosition, position); }
    // End CSS3 Setters

    // Apple-specific property setters
    void setPointerEvents(EPointerEvents p) { SET_FLAG(inheritedFlags, pointerEvents, p); }

    void clearAnimations()
    {
        m_rareNonInheritedData.access()->m_animations.clear();
    }

    void clearTransitions()
    {
        m_rareNonInheritedData.access()->m_transitions.clear();
    }

    void inheritAnimations(const AnimationList* parent) { m_rareNonInheritedData.access()->m_animations = parent ? adoptPtr(new AnimationList(*parent)) : nullptr; }
    void inheritTransitions(const AnimationList* parent) { m_rareNonInheritedData.access()->m_transitions = parent ? adoptPtr(new AnimationList(*parent)) : nullptr; }
    void adjustAnimations();
    void adjustTransitions();

    void setTransformStyle3D(ETransformStyle3D b) { SET_VAR_GEN_SIG(rareNonInheritedData, transformStyle3D, b); }
    void setBackfaceVisibility(EBackfaceVisibility b) { SET_VAR_GEN_SIG(rareNonInheritedData, backfaceVisibility, b); }
    void setPerspective(float p) { SET_VAR_GEN_SIG(rareNonInheritedData, perspective, p); }
    void setPerspectiveOriginX(Length l) { SET_VAR_GEN_SIG(rareNonInheritedData, perspectiveOriginX, l); }
    void setPerspectiveOriginY(Length l) { SET_VAR_GEN_SIG(rareNonInheritedData, perspectiveOriginY, l); }
    void setPageSize(LengthSize s) { SET_VAR_GEN_SIG(rareNonInheritedData, pageSize, s); }
    void setPageSizeType(PageSizeType t) { SET_VAR_GEN_SIG(rareNonInheritedData, pageSizeType, t); }
    void resetPageSizeType() { SET_VAR_GEN_SIG(rareNonInheritedData, pageSizeType, PAGE_SIZE_AUTO); }

#if USE(ACCELERATED_COMPOSITING)
    void setIsRunningAcceleratedAnimation(bool b = true) { SET_VAR_GEN_SIG(rareNonInheritedData, runningAcceleratedAnimation, b); }
#endif

    void setLineBoxContain(LineBoxContain c) { SET_VAR_GEN_SIG(rareInheritedData, lineBoxContain, c); }
    void setLineClamp(LineClampValue c) { SET_VAR_GEN_SIG(rareNonInheritedData, lineClamp, c); }
    void setTextSizeAdjust(bool b) { SET_VAR_GEN_SIG(rareInheritedData, textSizeAdjust, b); }
    void setTextSecurity(ETextSecurity aTextSecurity) { SET_VAR_GEN_SIG(rareInheritedData, textSecurity, aTextSecurity); }

#if ENABLE(SVG)
    const SVGRenderStyle* svgStyle() const { return m_svgStyle.get(); }
    SVGRenderStyle* accessSVGStyle() { return m_svgStyle.access(); }
    
    float fillOpacity() const { return svgStyle()->fillOpacity(); }
    void setFillOpacity(float f) { accessSVGStyle()->setFillOpacity(f); }
    
    float strokeOpacity() const { return svgStyle()->strokeOpacity(); }
    void setStrokeOpacity(float f) { accessSVGStyle()->setStrokeOpacity(f); }
    
    float floodOpacity() const { return svgStyle()->floodOpacity(); }
    void setFloodOpacity(float f) { accessSVGStyle()->setFloodOpacity(f); }
#endif

    const ContentData* contentData() const { return m_rareNonInheritedData->m_content.get(); }
    bool contentDataEquivalent(const RenderStyle* otherStyle) const { return const_cast<RenderStyle*>(this)->m_rareNonInheritedData->contentDataEquivalent(*const_cast<RenderStyle*>(otherStyle)->m_rareNonInheritedData); }
    void clearContent();
    void setContent(PassRefPtr<StringImpl>, bool add = false);
    void setContent(PassRefPtr<StyleImage>, bool add = false);
    void setContent(PassOwnPtr<CounterContent>, bool add = false);
    void setContent(QuoteType, bool add = false);

    const CounterDirectiveMap* counterDirectives() const;
    CounterDirectiveMap& accessCounterDirectives();

    QuotesData* quotes() const { return m_rareInheritedData->m_quotes.get(); }
    void setQuotes(PassRefPtr<QuotesData>);

    const AtomicString& hyphenString() const;

    bool inheritedNotEqual(const RenderStyle*) const;

    StyleDifference diff(const RenderStyle*, unsigned& changedContextSensitiveProperties) const;

    bool isDisplayReplacedType() const
    {
        return display() == INLINE_BLOCK || display() == INLINE_BOX || display() == INLINE_TABLE;
    }

    bool isDisplayInlineType() const
    {
        return display() == INLINE || isDisplayReplacedType();
    }

    bool isOriginalDisplayInlineType() const
    {
        return originalDisplay() == INLINE || originalDisplay() == INLINE_BLOCK
            || originalDisplay() == INLINE_BOX || originalDisplay() == INLINE_TABLE;
    }

    void setWritingMode(WritingMode v) { SET_FLAG(inheritedFlags, writingMode, v); }

    // To tell if this style matched attribute selectors. This makes it impossible to share.
    bool affectedByAttributeSelectors() const { return m_affectedByAttributeSelectors; }
    void setAffectedByAttributeSelectors()
    {
        if(!m_affectedByAttributeSelectors)  
        {
            m_affectedByAttributeSelectors = true;
#if USE(WRATH)
            m_signal(E_affectedByAttributeSelectors);
#endif
        }
    }

    bool unique() const { return m_unique; }
    void setUnique()
    { 
        if(m_unique) return;
        m_unique = true;
#if USE(WRATH)
        m_signal(E_unique);
#endif
    }

    // Methods for indicating the style is affected by dynamic updates (e.g., children changing, our position changing in our sibling list, etc.)
    bool affectedByEmpty() const { return m_affectedByEmpty; }
    bool emptyState() const { return m_emptyState; }
    void setEmptyState(bool b)
    { 
        if(!m_affectedByEmpty)
        {
            m_affectedByEmpty = true;
#if USE(WRATH)
            m_signal(E_affectedByEmpty);
#endif
        }
        setUnique();
        if(b != m_emptyState)
        {
            m_emptyState = b;
#if USE(WRATH)
            m_signal(E_emptyState);
#endif
        }
    }
    bool childrenAffectedByPositionalRules() const { return childrenAffectedByForwardPositionalRules() || childrenAffectedByBackwardPositionalRules(); }
    bool childrenAffectedByFirstChildRules() const { return m_childrenAffectedByFirstChildRules; }
    void setChildrenAffectedByFirstChildRules()
    { 
        if(m_childrenAffectedByFirstChildRules) return;
        m_childrenAffectedByFirstChildRules = true;
#if USE(WRATH)
        m_signal(E_childrenAffectedByFirstChildRules);
#endif
    }
    bool childrenAffectedByLastChildRules() const { return m_childrenAffectedByLastChildRules; }
    void setChildrenAffectedByLastChildRules()
    { 
        if(m_childrenAffectedByLastChildRules) return;
        m_childrenAffectedByLastChildRules = true;
#if USE(WRATH)
        m_signal(E_childrenAffectedByLastChildRules);
#endif
    }
    bool childrenAffectedByDirectAdjacentRules() const { return m_childrenAffectedByDirectAdjacentRules; }
    void setChildrenAffectedByDirectAdjacentRules()
    { 
        if(m_childrenAffectedByDirectAdjacentRules) return; 
        m_childrenAffectedByDirectAdjacentRules = true;
#if USE(WRATH)
        m_signal(E_childrenAffectedByDirectAdjacentRules);
#endif
    }

    bool childrenAffectedByForwardPositionalRules() const { return m_childrenAffectedByForwardPositionalRules; }
    void setChildrenAffectedByForwardPositionalRules()
    {
        if(m_childrenAffectedByForwardPositionalRules) return;
        m_childrenAffectedByForwardPositionalRules = true;
#if USE(WRATH)
        m_signal(E_childrenAffectedByForwardPositionalRules);
#endif
    }
    bool childrenAffectedByBackwardPositionalRules() const { return m_childrenAffectedByBackwardPositionalRules; }
    void setChildrenAffectedByBackwardPositionalRules()
    {
        if(m_childrenAffectedByBackwardPositionalRules) return;            
        m_childrenAffectedByBackwardPositionalRules = true;
#if USE(WRATH)
        m_signal(E_childrenAffectedByBackwardPositionalRules);
#endif
    }
    bool firstChildState() const { return m_firstChildState; }
    void setFirstChildState()
    {
        setUnique();
        if(m_firstChildState) return;
        m_firstChildState = true;
#if USE(WRATH)
        m_signal(E_firstChildState);
#endif
    }

    bool lastChildState() const { return m_lastChildState; }
    void setLastChildState()
    {
        setUnique();
        if(m_lastChildState) return;
        m_lastChildState = true;
#if USE(WRATH)
        m_signal(E_lastChildState);
#endif
    }
    unsigned childIndex() const { return m_childIndex; }
    void setChildIndex(unsigned index)
    { 
        setUnique();
        if(m_childIndex != index)
        {
            m_childIndex = index;
#if USE(WRATH)
            m_signal(E_childIndex);
#endif
        }
    }

    const Color visitedDependentColor(int colorProperty) const;

    // Initial values for all the properties
    static EBorderCollapse initialBorderCollapse() { return BSEPARATE; }
    static EBorderStyle initialBorderStyle() { return BNONE; }
    static NinePieceImage initialNinePieceImage() { return NinePieceImage(); }
    static LengthSize initialBorderRadius() { return LengthSize(Length(0, Fixed), Length(0, Fixed)); }
    static ECaptionSide initialCaptionSide() { return CAPTOP; }
    static EClear initialClear() { return CNONE; }
    static TextDirection initialDirection() { return LTR; }
    static WritingMode initialWritingMode() { return TopToBottomWritingMode; }
    static TextCombine initialTextCombine() { return TextCombineNone; }
    static TextOrientation initialTextOrientation() { return TextOrientationVerticalRight; }
    static EDisplay initialDisplay() { return INLINE; }
    static EEmptyCell initialEmptyCells() { return SHOW; }
    static EFloat initialFloating() { return FNONE; }
    static EListStylePosition initialListStylePosition() { return OUTSIDE; }
    static EListStyleType initialListStyleType() { return Disc; }
    static EOverflow initialOverflowX() { return OVISIBLE; }
    static EOverflow initialOverflowY() { return OVISIBLE; }
    static EPageBreak initialPageBreak() { return PBAUTO; }
    static EPosition initialPosition() { return StaticPosition; }
    static ETableLayout initialTableLayout() { return TAUTO; }
    static EUnicodeBidi initialUnicodeBidi() { return UBNormal; }
    static ETextTransform initialTextTransform() { return TTNONE; }
    static EVisibility initialVisibility() { return VISIBLE; }
    static EWhiteSpace initialWhiteSpace() { return NORMAL; }
    static short initialHorizontalBorderSpacing() { return 0; }
    static short initialVerticalBorderSpacing() { return 0; }
    static ECursor initialCursor() { return CURSOR_AUTO; }
    static Color initialColor() { return Color::black; }
    static StyleImage* initialListStyleImage() { return 0; }
    static unsigned short initialBorderWidth() { return 3; }
    static int initialLetterWordSpacing() { return 0; }
    static Length initialSize() { return Length(); }
    static Length initialMinSize() { return Length(0, Fixed); }
    static Length initialMaxSize() { return Length(undefinedLength, Fixed); }
    static Length initialOffset() { return Length(); }
    static Length initialMargin() { return Length(Fixed); }
    static Length initialPadding() { return Length(Fixed); }
    static Length initialTextIndent() { return Length(Fixed); }
    static EVerticalAlign initialVerticalAlign() { return BASELINE; }
    static int initialWidows() { return 2; }
    static int initialOrphans() { return 2; }
    static Length initialLineHeight() { return Length(-100.0, Percent); }
    static ETextAlign initialTextAlign() { return TAAUTO; }
    static ETextDecoration initialTextDecoration() { return TDNONE; }
    static float initialZoom() { return 1.0f; }
    static int initialOutlineOffset() { return 0; }
    static float initialOpacity() { return 1.0f; }
    static EBoxAlignment initialBoxAlign() { return BSTRETCH; }
    static EBoxDirection initialBoxDirection() { return BNORMAL; }
    static EBoxLines initialBoxLines() { return SINGLE; }
    static EBoxOrient initialBoxOrient() { return HORIZONTAL; }
    static EBoxAlignment initialBoxPack() { return BSTART; }
    static float initialBoxFlex() { return 0.0f; }
    static int initialBoxFlexGroup() { return 1; }
    static int initialBoxOrdinalGroup() { return 1; }
    static EBoxSizing initialBoxSizing() { return CONTENT_BOX; }
    static StyleReflection* initialBoxReflect() { return 0; }
    static int initialMarqueeLoopCount() { return -1; }
    static int initialMarqueeSpeed() { return 85; }
    static Length initialMarqueeIncrement() { return Length(6, Fixed); }
    static EMarqueeBehavior initialMarqueeBehavior() { return MSCROLL; }
    static EMarqueeDirection initialMarqueeDirection() { return MAUTO; }
    static EUserModify initialUserModify() { return READ_ONLY; }
    static EUserDrag initialUserDrag() { return DRAG_AUTO; }
    static EUserSelect initialUserSelect() { return SELECT_TEXT; }
    static bool initialTextOverflow() { return false; }
    static EMarginCollapse initialMarginBeforeCollapse() { return MCOLLAPSE; }
    static EMarginCollapse initialMarginAfterCollapse() { return MCOLLAPSE; }
    static EWordBreak initialWordBreak() { return NormalWordBreak; }
    static EWordWrap initialWordWrap() { return NormalWordWrap; }
    static ENBSPMode initialNBSPMode() { return NBNORMAL; }
    static EKHTMLLineBreak initialKHTMLLineBreak() { return LBNORMAL; }
    static EMatchNearestMailBlockquoteColor initialMatchNearestMailBlockquoteColor() { return BCNORMAL; }
    static const AtomicString& initialHighlight() { return nullAtom; }
    static ESpeak initialSpeak() { return SpeakNormal; }
    static Hyphens initialHyphens() { return HyphensManual; }
    static short initialHyphenationLimitBefore() { return -1; }
    static short initialHyphenationLimitAfter() { return -1; }
    static const AtomicString& initialHyphenationString() { return nullAtom; }
    static const AtomicString& initialLocale() { return nullAtom; }
    static EBorderFit initialBorderFit() { return BorderFitBorder; }
    static EResize initialResize() { return RESIZE_NONE; }
    static ControlPart initialAppearance() { return NoControlPart; }
    static bool initialVisuallyOrdered() { return false; }
    static float initialTextStrokeWidth() { return 0; }
    static unsigned short initialColumnCount() { return 1; }
    static bool initialColumnSpan() { return false; }
    static const TransformOperations& initialTransform() { DEFINE_STATIC_LOCAL(TransformOperations, ops, ()); return ops; }
    static Length initialTransformOriginX() { return Length(50.0, Percent); }
    static Length initialTransformOriginY() { return Length(50.0, Percent); }
    static EPointerEvents initialPointerEvents() { return PE_AUTO; }
    static float initialTransformOriginZ() { return 0; }
    static ETransformStyle3D initialTransformStyle3D() { return TransformStyle3DFlat; }
    static EBackfaceVisibility initialBackfaceVisibility() { return BackfaceVisibilityVisible; }
    static float initialPerspective() { return 0; }
    static Length initialPerspectiveOriginX() { return Length(50.0, Percent); }
    static Length initialPerspectiveOriginY() { return Length(50.0, Percent); }
    static Color initialBackgroundColor() { return Color::transparent; }
    static Color initialTextEmphasisColor() { return TextEmphasisFillFilled; }
    static TextEmphasisFill initialTextEmphasisFill() { return TextEmphasisFillFilled; }
    static TextEmphasisMark initialTextEmphasisMark() { return TextEmphasisMarkNone; }
    static const AtomicString& initialTextEmphasisCustomMark() { return nullAtom; }
    static TextEmphasisPosition initialTextEmphasisPosition() { return TextEmphasisPositionOver; }
    static LineBoxContain initialLineBoxContain() { return LineBoxContainBlock | LineBoxContainInline | LineBoxContainReplaced; }

    // Keep these at the end.
    static LineClampValue initialLineClamp() { return LineClampValue(); }
    static bool initialTextSizeAdjust() { return true; }
    static ETextSecurity initialTextSecurity() { return TSNONE; }
#if ENABLE(DASHBOARD_SUPPORT)
    static const Vector<StyleDashboardRegion>& initialDashboardRegions();
    static const Vector<StyleDashboardRegion>& noneDashboardRegions();
#endif

private:
    void inheritUnicodeBidiFrom(const RenderStyle* parent) { SET_FLAG(noninheritedFlags, unicodeBidi, parent->m_noninheritedFlags.m_unicodeBidi); }
    void getShadowExtent(const ShadowData*, int& top, int& right, int& bottom, int& left) const;
    void getShadowHorizontalExtent(const ShadowData*, int& left, int& right) const;
    void getShadowVerticalExtent(const ShadowData*, int& top, int& bottom) const;
    void getShadowInlineDirectionExtent(const ShadowData* shadow, int& logicalLeft, int& logicalRight) const
    {
        return isHorizontalWritingMode() ? getShadowHorizontalExtent(shadow, logicalLeft, logicalRight) : getShadowVerticalExtent(shadow, logicalLeft, logicalRight);
    }
    void getShadowBlockDirectionExtent(const ShadowData* shadow, int& logicalTop, int& logicalBottom) const
    {
        return isHorizontalWritingMode() ? getShadowVerticalExtent(shadow, logicalTop, logicalBottom) : getShadowHorizontalExtent(shadow, logicalTop, logicalBottom);
    }

    // Color accessors are all private to make sure callers use visitedDependentColor instead to access them.
    const Color& borderLeftColor() const { return m_surround->m_border.left().color(); }
    const Color& borderRightColor() const { return m_surround->m_border.right().color(); }
    const Color& borderTopColor() const { return m_surround->m_border.top().color(); }
    const Color& borderBottomColor() const { return m_surround->m_border.bottom().color(); }
    const Color& backgroundColor() const { return m_background->color(); }
    const Color& color() const { return m_inherited->m_color; }
    const Color& columnRuleColor() const { return m_rareNonInheritedData->m_multiCol->m_rule.color(); }
    const Color& outlineColor() const { return m_background->outline().color(); }
    const Color& textEmphasisColor() const { return m_rareInheritedData->m_textEmphasisColor; }
    const Color& textFillColor() const { return m_rareInheritedData->m_textFillColor; }
    const Color& textStrokeColor() const { return m_rareInheritedData->m_textStrokeColor; }
    
    const Color colorIncludingFallback(int colorProperty, EBorderStyle borderStyle) const;

    ContentData* prepareToSetContent(StringImpl*, bool add);

#if USE(WRATH)
public:
    typedef Signal<RenderStyle> SignalType;
    SignalType::connection connect(const SignalType::slot_type & slot) { return m_signal.connect(slot); }
private:
    SignalType m_signal;
#endif
};

inline int adjustForAbsoluteZoom(int value, const RenderStyle* style)
{
    double zoomFactor = style->effectiveZoom();
    if (zoomFactor == 1)
        return value;
    // Needed because computeLengthInt truncates (rather than rounds) when scaling up.
    if (zoomFactor > 1) {
        if (value < 0)
            value--;
        else 
            value++;
    }

    return roundForImpreciseConversion<int, INT_MAX, INT_MIN>(value / zoomFactor);
}

inline float adjustFloatForAbsoluteZoom(float value, const RenderStyle* style)
{
    return value / style->effectiveZoom();
}

} // namespace WebCore

#endif // RenderStyle_h
