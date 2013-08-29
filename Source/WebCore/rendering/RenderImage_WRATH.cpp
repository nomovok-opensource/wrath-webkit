#include "config.h"
#include "RenderImage.h"

#include "Frame.h"
#include "GraphicsContext.h"
#include "HTMLAreaElement.h"
#include "HTMLImageElement.h"
#include "HTMLInputElement.h"
#include "HTMLMapElement.h"
#include "HTMLNames.h"
#include "HitTestResult.h"
#include "Page.h"
#include "RenderLayer.h"
#include "RenderView.h"
#include "SelectionController.h"
#include "TextRun.h"
#include <wtf/UnusedParam.h>

using namespace std;
#if USE(WRATH)

#include "PaintInfoOfWRATH.h"
#include "WRATHPaintHelpers.h"
#include "DrawnTextOfWRATH.h"

namespace
{
    class RenderImage_ReadyWRATHWidgets:
      public WebCore::PaintedWidgetsOfWRATH<RenderImage_ReadyWRATHWidgets>
    {
    public:
	WebCore::PaintedWidgetsOfWRATHHandle m_replaced;
	WebCore::PaintedWidgetsOfWRATHHandle m_area_element;
    };

    class RenderImage_ReadyWRATHWidgetReplaced:
      public WebCore::PaintedWidgetsOfWRATHPassOwner<RenderImage_ReadyWRATHWidgetReplaced,
                                                     WebCore::RenderImage>
    {
    public:
        RenderImage_ReadyWRATHWidgetReplaced(WebCore::RenderImage *p)
            : m_outline_shape(0)
            , m_outline_dirty(true)
        {
            if (p->style()) {
                m_style_connection = p->style()->connect(boost::bind(&RenderImage_ReadyWRATHWidgetReplaced::onStyleChange, this, _1));
            }

            m_object_connection = p->connect_style_change(boost::bind(&RenderImage_ReadyWRATHWidgetReplaced::onObjectChange, this, _1, p));
        }

        ~RenderImage_ReadyWRATHWidgetReplaced()
        {
            m_style_connection.disconnect();
            m_object_connection.disconnect();
            WRATHDelete(m_outline_shape);
        }

        // Naming is unfortunately not the best
        // This function is for when the style object signals something changed...
        void onStyleChange(enum WebCore::RenderStyle::MemberChange E)
        {
            /*
              [WRATH-TODO]: This should check whether E is ANY OF THE CHANGES
              that are about width. That is, width itself, padding, border.
              Funnily enough, there's at least five setters for each of them.
              That's a metric fuckton of case labels already with a 34% chance
              of missing a particular one.
              For now, we just say fuck it and do this regardless of E.
            */
            m_outline_dirty = true;
            m_outline_rect.delete_widget();
        }

        // ... and this function is for when the RenderObject signals the attached style changed
        void onObjectChange(WebCore::StyleDifference diff, WebCore::RenderImage* p)
        {
            m_style_connection.disconnect();
            m_style_connection = p->style()->connect(boost::bind(&RenderImage_ReadyWRATHWidgetReplaced::onStyleChange, this, _1));

            // [WRATH-TODO]: Same as above
            m_outline_dirty = true;
            m_outline_rect.delete_widget();
        }

        WRATHShapeF* m_outline_shape;
        bool m_outline_dirty;
        WebCore::ContextOfWRATH::CColorFamily::DrawnShape::AutoDelete m_outline_rect;
        WebCore::Signal<WebCore::RenderStyle>::connection m_style_connection;
        WebCore::Signal<WebCore::RenderObject>::connection m_object_connection;

	WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::Image> m_error_image;

	WebCore::DrawnTextOfWRATH m_alt_text;

	WebCore::PaintedWidgetsOfWRATHHandle m_into_rect;
    };

    class RenderImage_ReadyWRATHWidgetAreaElementFocusRing:
      public WebCore::PaintedWidgetsOfWRATHPassOwner<RenderImage_ReadyWRATHWidgetAreaElementFocusRing,
                                                     WebCore::RenderImage>
    {
    public:
        RenderImage_ReadyWRATHWidgetAreaElementFocusRing(WebCore::RenderImage *p)
            : m_focus_ring_shape(NULL)
            , m_focus_ring_dirty(true)
        {
            m_focus_change_connection = p->connect_focus_changed(boost::bind(&RenderImage_ReadyWRATHWidgetAreaElementFocusRing::onFocusChange, this));
        }

        ~RenderImage_ReadyWRATHWidgetAreaElementFocusRing()
        {
            m_focus_change_connection.disconnect();
        }

        /*
          [WRATH-TODO]
          The focus ring gets reconstructed when the area element's focus changes to _another_shape_
          in the area map. Reconstructing the painted ring when the dimensions etc. of the _current_
          focused shape change is not done. This needs fixing, in a sane way.
        */

        void onFocusChange()
        {
            m_focus_ring_dirty = true;
            m_focus_ring.delete_widget();
        }

	WebCore::ContextOfWRATH::CColorFamily::DrawnShape::AutoDelete m_focus_ring;
        WRATHShapeF* m_focus_ring_shape;
        bool m_focus_ring_dirty;
        boost::signals2::connection m_focus_change_connection;
    };

    class RenderImage_ReadyWRATHWidgetIntoRect:
      public WebCore::PaintedWidgetsOfWRATH<RenderImage_ReadyWRATHWidgetIntoRect>
    {
    public:
        WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::Image> m_image;
    };
}

namespace WebCore
{
using namespace HTMLNames;

void RenderImage::readyWRATHWidgetReplaced(PaintedWidgetsOfWRATHHandle& handle,
					   PaintInfoOfWRATH& paintInfo, int tx, int ty)
{
    RenderImage_ReadyWRATHWidgetReplaced *d(RenderImage_ReadyWRATHWidgetReplaced::object(this, handle));

    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    // Set all non-visible by default
    if (d->m_outline_rect.widget())
        d->m_outline_rect.widget()->visible(false);
    d->m_error_image.visible(false);
    d->m_alt_text.visible(false);
    d->m_into_rect.visible(false);

    int cWidth = contentWidth();
    int cHeight = contentHeight();
    int leftBorder = borderLeft();
    int topBorder = borderTop();
    int leftPad = paddingLeft();
    int topPad = paddingTop();
    
    if (!m_imageResource->hasImage() || m_imageResource->errorOccurred()) {
        if (paintInfo.phase == PaintPhaseSelection) {
            return;
	}
	
        if (cWidth > 2 && cHeight > 2) {
            // Draw an outline rect where the image should be.
	    /*
            context->setStrokeStyle(SolidStroke);
            context->setStrokeColor(Color::lightGray, style()->colorSpace());
            context->setFillColor(Color::transparent, style()->colorSpace());
            context->drawRect(IntRect(tx + leftBorder + leftPad, ty + topBorder + topPad, cWidth, cHeight));
	    */
            if (!d->m_outline_shape || d->m_outline_dirty) {
                if (!d->m_outline_shape) {
                    d->m_outline_shape = WRATHNew WRATHShapeF;
                }

                d->m_outline_shape->clear();
                d->m_outline_shape->new_outline();
                d->m_outline_shape->current_outline() <<
                    vec2(0, 0) <<
                    vec2(0, cHeight) <<
                    vec2(cWidth, cHeight) <<
                    vec2(cWidth, 0);
            }

            vec2 outline_pos(tx + leftBorder + leftPad,
                             ty + topBorder + topPad);

            /* [WRATH-DANGER]: Color space not used */
            vec4 c;
            Color wc = Color::lightGray;
            wc.getRGBA(c.x(), c.y(), c.z(), c.w());
            /*
              [WRATH-TODO]: if the shape is transparent, we need
              to obey the transparency value.
            */
            paintInfo.wrath_context->add_stroked_shape(d->m_outline_rect,
                                                       WRATHWidgetGenerator::ColorProperties(c),
                                                       WRATHWidgetGenerator::shape_value(*d->m_outline_shape),
                                                       WRATHWidgetGenerator::StrokingParameters()
                                                       .close_outline(true)
                                                       // GraphicsContext::drawRect always strokes with width=1
                                                       .width(1));
            d->m_outline_rect.widget()->visible(true);
            d->m_outline_rect.widget()->position(outline_pos);
	    
            bool errorPictureDrawn = false;
            int imageX = 0;
            int imageY = 0;
            // When calculating the usable dimensions, exclude the pixels of
            // the ouline rect so the error image/alt text doesn't draw on it.
            int usableWidth = cWidth - 2;
            int usableHeight = cHeight - 2;
	    
            RefPtr<Image> image = m_imageResource->image();
	    
            if (m_imageResource->errorOccurred() && !image->isNull() && usableWidth >= image->width() && usableHeight >= image->height()) {
                // Center the error image, accounting for border and padding.
                int centerX = (usableWidth - image->width()) / 2;
                if (centerX < 0)
                    centerX = 0;
                int centerY = (usableHeight - image->height()) / 2;
                if (centerY < 0)
                    centerY = 0;
                imageX = leftBorder + leftPad + centerX + 1;
                imageY = topBorder + topPad + centerY + 1;
                WRATH_drawImage(d->m_error_image, paintInfo.wrath_context, 
                                image.get(), style()->colorSpace(), IntPoint(tx + imageX, ty + imageY));
                d->m_error_image.visible(true);
                errorPictureDrawn = true;
            }
	    
            if (!m_altText.isEmpty()) {
                String text = document()->displayStringModifiedByEncoding(m_altText);
		//                context->setFillColor(style()->visitedDependentColor(CSSPropertyColor), style()->colorSpace());
                int ax = tx + leftBorder + leftPad;
                int ay = ty + topBorder + topPad;
                const Font& font = style()->font();
                const FontMetrics& fontMetrics = font.fontMetrics();
                int ascent = fontMetrics.ascent();
		
                // Only draw the alt text if it'll fit within the content box,
                // and only if it fits above the error image.
                TextRun textRun(text.characters(), text.length());
                int textWidth = font.width(textRun);
                if (errorPictureDrawn) {
                    if (usableWidth >= textWidth && fontMetrics.height() <= imageY) {
                        d->m_alt_text.update(paintInfo.wrath_context, font, textRun,
                                             style()->visitedDependentColor(CSSPropertyColor));
                        d->m_alt_text.node()->position(vec2(ax, ay + ascent));
                        d->m_alt_text.visible(true);
		    }
                } else if (usableWidth >= textWidth && cHeight >= fontMetrics.height()) {
                    d->m_alt_text.update(paintInfo.wrath_context, font, textRun,
                                         style()->visitedDependentColor(CSSPropertyColor));
                    d->m_alt_text.node()->position(vec2(ax, ay + ascent));
                    d->m_alt_text.visible(true);
		}
            }
        }
    } else if (m_imageResource->hasImage() && cWidth > 0 && cHeight > 0) {
        RefPtr<Image> img = m_imageResource->image(cWidth, cHeight);
        if (!img || img->isNull()) {
            return;
	}
	
#if PLATFORM(MAC)
#error This code is here just for completeness, not wrathified
        if (style()->highlight() != nullAtom && !paintInfo.context->paintingDisabled())
            paintCustomHighlight(tx - x(), ty - y(), style()->highlight(), true);
#endif
	
        IntSize contentSize(cWidth, cHeight);
        IntRect rect(IntPoint(tx + leftBorder + leftPad, ty + topBorder + topPad), contentSize);
        d->m_into_rect.visible(true);
	readyWRATHWidgetIntoRect(d->m_into_rect, paintInfo.wrath_context, rect);
    }
}

void RenderImage::readyWRATHWidgets(PaintedWidgetsOfWRATHHandle& handle,
				    PaintInfoOfWRATH& paintInfo, int tx, int ty)
{
    RenderImage_ReadyWRATHWidgets *d(RenderImage_ReadyWRATHWidgets::object(this, handle));
    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    RenderReplaced::readyWRATHWidgets(d->m_replaced, paintInfo, tx, ty);
    
    if (paintInfo.phase == PaintPhaseOutline)
	readyWRATHWidgetAreaElementFocusRing(d->m_area_element, paintInfo);
}

void RenderImage::readyWRATHWidgetAreaElementFocusRing(PaintedWidgetsOfWRATHHandle& handle, PaintInfoOfWRATH& paintInfo)
{
    RenderImage_ReadyWRATHWidgetAreaElementFocusRing *d(RenderImage_ReadyWRATHWidgetAreaElementFocusRing::object(this, handle));
    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    if (d->m_focus_ring.widget())
        d->m_focus_ring.widget()->visible(false);

    Document* document = this->document();
    
    if (document->printing() || !document->frame()->selection()->isFocusedAndActive())
        return;

    /*    
    if (paintInfo.context->paintingDisabled() && !paintInfo.context->updatingControlTints())
        return;
    */

    Node* focusedNode = document->focusedNode();
    if (!focusedNode || !focusedNode->hasTagName(areaTag))
        return;

    HTMLAreaElement* areaElement = static_cast<HTMLAreaElement*>(focusedNode);
    if (areaElement->imageElement() != node())
        return;

    // Even if the theme handles focus ring drawing for entire elements, it won't do it for
    // an area within an image, so we don't call RenderTheme::supportsFocusRing here.

    Path path = areaElement->computePath(this);
    if (path.isEmpty())
        return;

    // FIXME: Do we need additional code to clip the path to the image's bounding box?

    RenderStyle* areaElementStyle = areaElement->computedStyle();
    unsigned short outlineWidth = areaElementStyle->outlineWidth();
    if (!outlineWidth)
        return;

    if (!d->m_focus_ring_shape || d->m_focus_ring_dirty) {
        if (!d->m_focus_ring_shape) {
            d->m_focus_ring_shape = WRATHNew WRATHShapeF;
        }

        d->m_focus_ring_shape->clear();
        d->m_focus_ring_shape->new_outline();
        
        QPainterPath qpath = path.platformPath();
        
	// Skip the first element, it is not needed because it's the same as the last
	for (int i = 1; i < qpath.elementCount(); ++i) {
            const QPainterPath::Element & cur = qpath.elementAt(i);
            
            switch (cur.type) {
            case QPainterPath::MoveToElement: {
                ASSERT_NOT_REACHED();
                break;
            }
            case QPainterPath::LineToElement: {
                QPointF p(cur);
                d->m_focus_ring_shape->current_outline() << vec2(p.x(), p.y());
                break;
            }
            case QPainterPath::CurveToElement: {
                QPainterPath::Element c1 = qpath.elementAt(i+1);
                QPainterPath::Element c2 = qpath.elementAt(i+2);
                
                ASSERT(c1.type == QPainterPath::CurveToDataElement);
                ASSERT(c2.type == QPainterPath::CurveToDataElement);
                
                QPointF p1(cur);
                QPointF p2(c1);
                QPointF p3(c2);
                
                d->m_focus_ring_shape->current_outline() << 
                    WRATHOutlineF::control_point(vec2(p1.x(), p1.y())) <<
                    WRATHOutlineF::control_point(vec2(p2.x(), p2.y())) <<
                    vec2(p3.x(), p3.y());
                
                i += 2;
                
                break;
            }
            case QPainterPath::CurveToDataElement: {
                ASSERT_NOT_REACHED();
                break;
            }
            }
        }
    }

    vec4 c;
    Color wc = areaElementStyle->visitedDependentColor(CSSPropertyOutlineColor);
    wc.getRGBA(c.x(), c.y(), c.z(), c.w());

    paintInfo.wrath_context->add_stroked_shape(d->m_focus_ring,
                                               WRATHWidgetGenerator::ColorProperties(c),
                                               WRATHWidgetGenerator::shape_value(*d->m_focus_ring_shape),
                                               WRATHWidgetGenerator::StrokingParameters()
                                               .close_outline(true)
                                               .width(outlineWidth)
                                               .stroke_curves(WRATHWidgetGenerator::dashed_stroke));
    d->m_focus_ring.widget()->visible(true);
}

void RenderImage::readyWRATHWidgetIntoRect(PaintedWidgetsOfWRATHHandle& handle,
                                           ContextOfWRATH *wrath_context, const IntRect& rect)
{
    RenderImage_ReadyWRATHWidgetIntoRect *d(RenderImage_ReadyWRATHWidgetIntoRect::object(this, handle));
    ContextOfWRATH::AutoPushNode autoPushRoot(wrath_context, d->m_root_node);

    d->m_image.visible(false);

    if (!m_imageResource->hasImage() || m_imageResource->errorOccurred() || rect.width() <= 0 || rect.height() <= 0) {
        return;
    }

    RefPtr<Image> img = m_imageResource->image(rect.width(), rect.height());
    if (!img || img->isNull()) {
        return;
    }

    HTMLImageElement* imageElt = (node() && node()->hasTagName(imgTag)) ? static_cast<HTMLImageElement*>(node()) : 0;
    CompositeOperator compositeOperator = imageElt ? imageElt->compositeOperator() : CompositeSourceOver;
    Image* image = m_imageResource->image().get();
    /*
    bool useLowQualityScaling = shouldPaintAtLowQuality(context, image, image, rect.size());
    [WRATH-TODO]: What to do with the above function call?
    */

    /*
    context->drawImage(m_imageResource->image(rect.width(), rect.height()).get(), style()->colorSpace(), rect, compositeOperator, useLowQualityScaling);
    */

    d->m_image.visible(true);
    WRATH_drawImage(d->m_image, wrath_context, 
                    m_imageResource->image(rect.width(), rect.height()).get(), style()->colorSpace(), rect, compositeOperator, false /*useLowQualityScaling*/);

}
}
#endif
