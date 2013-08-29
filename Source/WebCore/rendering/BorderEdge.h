#ifndef BorderEdge_h
#define BorderEdge_h 

#include "config.h"
#include "RenderBoxModelObject.h"

namespace WebCore {

enum BorderEdgeFlag {
    TopBorderEdge = 1 << BSTop,
    RightBorderEdge = 1 << BSRight,
    BottomBorderEdge = 1 << BSBottom,
    LeftBorderEdge = 1 << BSLeft,
    AllBorderEdges = TopBorderEdge | BottomBorderEdge | LeftBorderEdge | RightBorderEdge
};

class BorderEdge {
public:
    BorderEdge(int edgeWidth, const Color& edgeColor, EBorderStyle edgeStyle, bool edgeIsTransparent, bool edgeIsPresent = true)
        : width(edgeWidth)
        , color(edgeColor)
        , style(edgeStyle)
        , isTransparent(edgeIsTransparent)
        , isPresent(edgeIsPresent)
    {
        if (style == DOUBLE && edgeWidth < 3)
            style = SOLID;
    }
    
    BorderEdge()
        : width(0)
        , style(BHIDDEN)
        , isTransparent(false)
        , isPresent(false)
    {
    }
    
    bool hasVisibleColorAndStyle() const { return style > BHIDDEN && !isTransparent; }
    bool shouldRender() const { return isPresent && hasVisibleColorAndStyle(); }
    bool presentButInvisible() const { return usedWidth() && !hasVisibleColorAndStyle(); }
    bool obscuresBackgroundEdge(float scale) const
    {
        if (!isPresent || isTransparent || width < (2 * scale) || color.hasAlpha() || style == BHIDDEN)
            return false;

        if (style == DOTTED || style == DASHED)
            return false;

        if (style == DOUBLE)
            return width >= 5 * scale; // The outer band needs to be >= 2px wide at unit scale.

        return true;
    }

    int usedWidth() const { return isPresent ? width : 0; }
    
    void getDoubleBorderStripeWidths(int& outerWidth, int& innerWidth) const
    {
        int fullWidth = usedWidth();
        outerWidth = fullWidth / 3;
        innerWidth = fullWidth * 2 / 3;

        // We need certain integer rounding results
        if (fullWidth % 3 == 2)
            outerWidth += 1;

        if (fullWidth % 3 == 1)
            innerWidth += 1;
    }
    
    int width;
    Color color;
    EBorderStyle style;
    bool isTransparent;
    bool isPresent;
};

#if HAVE(PATH_BASED_BORDER_RADIUS_DRAWING)
static bool borderWillArcInnerEdge(const IntSize& firstRadius, const IntSize& secondRadius)
{
    return !firstRadius.isZero() || !secondRadius.isZero();
}

static inline BorderEdgeFlag edgeFlagForSide(BoxSide side)
{
    return static_cast<BorderEdgeFlag>(1 << side);
}

static inline bool includesEdge(BorderEdgeFlags flags, BoxSide side)
{
    return flags & edgeFlagForSide(side);
}

inline bool edgesShareColor(const BorderEdge& firstEdge, const BorderEdge& secondEdge)
{
    return firstEdge.color == secondEdge.color;
}

inline bool styleRequiresClipPolygon(EBorderStyle style)
{
    return style == DOTTED || style == DASHED; // These are drawn with a stroke, so we have to clip to get corner miters.
}

inline bool borderStyleFillsBorderArea(EBorderStyle style)
{
    return !(style == DOTTED || style == DASHED || style == DOUBLE);
}

inline bool borderStyleHasInnerDetail(EBorderStyle style)
{
    return style == GROOVE || style == RIDGE || style == DOUBLE;
}

inline bool borderStyleIsDottedOrDashed(EBorderStyle style)
{
    return style == DOTTED || style == DASHED;
}
#endif

} /* namespace WebCore */

#endif /* BorderEdge_h */
