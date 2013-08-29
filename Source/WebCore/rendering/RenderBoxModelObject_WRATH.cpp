#include "config.h"
#include "RenderBoxModelObject.h"
#include "BorderEdge.h"

#include "GraphicsContext.h"
#include "HTMLFrameOwnerElement.h"
#include "HTMLNames.h"
#include "ImageBuffer.h"
#include "Path.h"
#include "RenderBlock.h"
#include "RenderInline.h"
#include "RenderLayer.h"
#include "RenderView.h"
#include <wtf/CurrentTime.h>

#include "InlineFlowBox.h"
#include "RenderLayer.h"

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
#include "BorderEdge.h"
#include "WRATHPaintHelpers.h"
#include "NativeImageWRATH.h"
#include "WRATHPolynomial.hpp"
#include "ostream_utility.hpp"

#include <limits>

#define WRATH_WEBKIT_DEBUG_SHAPES


namespace {

class Degree3BernstienPolynomial:public WRATHUtil::BernsteinPolynomial<vec2>
{
public:
  Degree3BernstienPolynomial(void):
    WRATHUtil::BernsteinPolynomial<vec2>(4) //4 control points
  {}
};

class RenderBoxModelObject_WRATHNinePieceImage: 
    public WebCore::PaintedWidgetsOfWRATH<RenderBoxModelObject_WRATHNinePieceImage>
{
public:  
    void
    make_all_invisible(void)
    {
      m_left.visible(false);
      m_left_bottom.visible(false);
      m_left_top.visible(false);
      m_right.visible(false);
      m_right_bottom.visible(false);
      m_right_top.visible(false);
      m_bottom.visible(false);
      m_top.visible(false);
      m_middle.visible(false);
    }    

    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::Image> m_left, m_left_bottom, m_left_top;
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::Image> m_right, m_right_bottom, m_right_top;
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::Image> m_bottom, m_top, m_middle;
};

class RenderBoxModelObject_WRATHWidgetBorder:
    public WebCore::PaintedWidgetsOfWRATH<RenderBoxModelObject_WRATHWidgetBorder>
{
public:

    RenderBoxModelObject_WRATHWidgetBorder(void):
      m_outerBorder(WebCore::IntRect()),
      m_innerBorder(WebCore::IntRect()),
      m_shapes_of_edges_dirty(true)
    {}

    void
    make_all_invisible(void)
    {
      m_nine_piece_image.visible(false);
      for(unsigned int i=0, endi=m_edges.size(); i<endi; ++i) {
        if(m_edges[i].widget())
          m_edges[i].widget()->visible(false);
      }      
    }   

    static
    void
    construct_shapes(vecN<WRATHShapeF, 4> &shapes,
                     const WebCore::RoundedIntRect &outerBorder,
                     const WebCore::RoundedIntRect &innerBorder);

    
    enum CornerIsRoundedType
      {
        CornerIsRounded,
        CornerIsNotRounded
      };

  /*
    If the corner is rounded,
      computes the points of the Bezier Curve
      connecting from start_point to start_point+radius.
      along a circular counter-clockwise arc between the 2.

    if the corner is not rounded
    then pts[0]=pts[1]=pts[2]=pts[3]=start_point
   */
    static
    enum CornerIsRoundedType
    compute_corner_curve(vecN<vec2, 4> &pts,
                         const ivec2 &start_point,
                         const ivec2 &radius,
                         bool xchanges_first_ctl_point);

  /*
    Warning: these values are rigged to match
    with enum BoxSide so that 
      BoxSide(I) is RectCornerType(I-1) to RectCornerType(I)
   */
    enum RectCornerType
      {
        CornerTopRight,
        CornerBottomRight,
        CornerBottomLeft,
        CornerTopLeft,
      };

    
    static
    enum CornerIsRoundedType
    compute_corner_curve(vec2 &corner_pt,
                         vecN<Degree3BernstienPolynomial, 2> &poly,
                         enum RectCornerType corner,
                         const WebCore::RoundedIntRect &roundedRect);

    static
    void
    compute_corners_of_rect(vecN<vec2, 4> &corner_pts,
                            vecN<vecN<Degree3BernstienPolynomial, 2>, 4> &poly,
                            vecN<CornerIsRoundedType, 4> &types,
                            const WebCore::RoundedIntRect &rect);


    WebCore::PaintedWidgetsOfWRATHHandle m_nine_piece_image;


  /*
    input values:
   */
    WebCore::RoundedIntRect m_outerBorder;
    WebCore::RoundedIntRect m_innerBorder;
    ivec2 m_rect_delta;
    vecN<WebCore::Color, 4> m_colors;

  /*
    break the set m_outerBorder \ m_innerBorder  
    into 4 pieces, enumerated by BSLeft, BSTop, BSRight, BSBottom
    of WebCore::BoxSide defined in RenderObject.h
   */
    bool m_shapes_of_edges_dirty;
    vecN<WRATHShapeF, 4> m_shapes_of_edges;
    vecN<WebCore::ContextOfWRATH::CColorFamily::DrawnShape::AutoDelete, 4> m_edges;
  
    vec2 m_position;

};


enum RenderBoxModelObject_WRATHWidgetBorder::CornerIsRoundedType
RenderBoxModelObject_WRATHWidgetBorder::
compute_corner_curve(vecN<vec2, 4> &pts,
                     const ivec2 &start_point,
                     const ivec2 &radius,
                     bool xchanges_first_ctl_point)
{
  // Approximation of control point positions on a bezier to simulate a quarter of a circle.
  static const float gCircleControlPoint = 1.0f - 0.448f;

  vec2 st(start_point.x(), start_point.y());
  vec2 rd(radius.x(), radius.y());

  if(radius.x()==0 or radius.y()==0)
    {
      pts[0]=pts[1]=st;
      pts[2]=pts[3]=st+rd;
      return CornerIsNotRounded;
    }

  pts[0]=st;

  if(xchanges_first_ctl_point)
    {
      pts[1]=st + vec2(gCircleControlPoint*rd.x(), 0.0f);
      pts[2]=st + rd - vec2(0.0f, gCircleControlPoint*rd.y());
    }
  else
    {
      pts[1]=st + vec2(0.0f, gCircleControlPoint*rd.y());
      pts[2]=st + rd - vec2(gCircleControlPoint*rd.x(), 0.0f);
    }
  pts[3]=st + rd;
  
  return CornerIsRounded;
}


enum RenderBoxModelObject_WRATHWidgetBorder::CornerIsRoundedType
RenderBoxModelObject_WRATHWidgetBorder::
compute_corner_curve(vec2 &corner_pt,
                     vecN<Degree3BernstienPolynomial, 2> &poly,
                     enum RectCornerType corner,
                     const WebCore::RoundedIntRect &R)
{
  ivec2 pt(R.rect().x(), R.rect().y());
  ivec2 radius;
  vecN<vec2, 4> pts;
  bool xchanges_first_ctl_point;

  
  switch(corner)
    {
    default:
      WRATHwarning("Bad enumeration for corner, value=" << (unsigned int)corner << "\n" << std::flush);

    case CornerTopLeft:
      xchanges_first_ctl_point=false;
      pt.y() += R.radii().topLeft().height();
      radius.x() =  R.radii().topLeft().width();
      radius.y() = -R.radii().topLeft().height();
      corner_pt=vec2(R.rect().x(), R.rect().y());
      break;

    case CornerTopRight:
      xchanges_first_ctl_point=true;
      pt.x()+= R.rect().width() - R.radii().topRight().width();
      radius.x() = R.radii().topRight().width();
      radius.y() = R.radii().topRight().height();
      corner_pt=vec2(R.rect().x()+R.rect().width(), R.rect().y());
      break;

    case CornerBottomRight:
      xchanges_first_ctl_point=false;
      pt.x()+=R.rect().width();
      pt.y()+=R.rect().height() - R.radii().bottomRight().height();
      radius.x() = -R.radii().bottomRight().width();
      radius.y() =  R.radii().bottomRight().height();
      corner_pt=vec2(R.rect().x()+R.rect().width(), R.rect().y()+R.rect().height());
      break;

    case CornerBottomLeft:
      xchanges_first_ctl_point=true;
      pt.x() += R.radii().bottomLeft().width();
      pt.y() += R.rect().height();
      radius.x() = -R.radii().bottomLeft().width();
      radius.y() = -R.radii().bottomLeft().height();
      corner_pt=vec2(R.rect().x(), R.rect().y()+R.rect().height());
      break;
      
    }

  CornerIsRoundedType return_value;
  return_value=compute_corner_curve(pts, pt, radius, xchanges_first_ctl_point);

  if(return_value==CornerIsRounded)
    {
      WRATHUtil::BernsteinPolynomial<vec2> bp(pts);
      bp.split_curve(poly[0], poly[1], 0.5f);
    }

  return return_value;
}

void
RenderBoxModelObject_WRATHWidgetBorder::
compute_corners_of_rect(vecN<vec2, 4> &corner_pts,
                        vecN< vecN<Degree3BernstienPolynomial, 2>, 4> &polys,
                        vecN<CornerIsRoundedType, 4> &types,
                        const WebCore::RoundedIntRect &rect)
{
  for(int i=0; i<4; ++i)
    {
      enum RectCornerType corner(static_cast<enum RectCornerType>(i));
      types[i]=compute_corner_curve(corner_pts[i], polys[i], corner, rect);
    }
}  
  
void
RenderBoxModelObject_WRATHWidgetBorder::
construct_shapes(vecN<WRATHShapeF, 4> &shapes,
                 const WebCore::RoundedIntRect &outerBorder,
                 const WebCore::RoundedIntRect &innerBorder)
{
  
  /*
    each corner is enumerated by enum RectCornerType
   */
  vec2 outerRectCenter;
  vecN< vecN<Degree3BernstienPolynomial, 2>, 4> outerPolynomials, innerPolynomials;
  vecN<CornerIsRoundedType, 4> outerCornerTypes, innerCornerTypes;
  bool innerRectNonDegenerate;
  vecN<vec2, 4> outerCorners, innerCorners;

  /*
    get the corner paths for each corner of each rounded rect
   */
  compute_corners_of_rect(outerCorners, outerPolynomials, outerCornerTypes, outerBorder);
  innerRectNonDegenerate=innerBorder.rect().width()>0 && innerBorder.rect().height()>0;
  if(innerRectNonDegenerate)
    {
      compute_corners_of_rect(innerCorners, innerPolynomials, innerCornerTypes, innerBorder);
    }
  else
    {
      outerRectCenter=vec2(innerBorder.rect().x(),  innerBorder.rect().y());
    }

  /*
    now contruct the path. The shape is:

     {Bezier outerPolynomials[i-1][1] } - {Bezier outerPolynomials[i][0] }
     - {Bezier innerPolynomials[i][0] } - {Bezier innerPolynomials[i-1][1] }
   */

  for(int i=0; i<4; ++i)
    {
      int prev_i(i==0?3:i-1);

      shapes[i].clear();
      if(outerCornerTypes[prev_i]==CornerIsRounded)
        {
          shapes[i].current_outline() 
            << outerPolynomials[prev_i][1].control_point(0)
            << WRATHOutlineF::control_point(outerPolynomials[prev_i][1].control_point(1))
            << WRATHOutlineF::control_point(outerPolynomials[prev_i][1].control_point(2))
            << outerPolynomials[prev_i][1].control_point(3);
        }
      else
        {
          shapes[i].current_outline() << outerCorners[prev_i];
        }


      if(outerCornerTypes[i]==CornerIsRounded)
        {
          shapes[i].current_outline() 
            << outerPolynomials[i][0].control_point(0)
            << WRATHOutlineF::control_point(outerPolynomials[i][0].control_point(1))
            << WRATHOutlineF::control_point(outerPolynomials[i][0].control_point(2))
            << outerPolynomials[i][0].control_point(3);
        }
      else
        {
          shapes[i].current_outline() << outerCorners[i];
        }
      
      
      if(innerRectNonDegenerate)
        {
          if(innerCornerTypes[i]==CornerIsRounded)
            {
              shapes[i].current_outline() 
                << innerPolynomials[i][0].control_point(3)
                << WRATHOutlineF::control_point(innerPolynomials[i][0].control_point(2))
                << WRATHOutlineF::control_point(innerPolynomials[i][0].control_point(1))
                << innerPolynomials[i][0].control_point(0);
            }
          else
            {
              shapes[i].current_outline() << innerCorners[i];
            }
          
          
          if(innerCornerTypes[prev_i]==CornerIsRounded)
            {
              shapes[i].current_outline() 
                << innerPolynomials[prev_i][1].control_point(3)
                << WRATHOutlineF::control_point(innerPolynomials[prev_i][1].control_point(2))
                << WRATHOutlineF::control_point(innerPolynomials[prev_i][1].control_point(1))
                << innerPolynomials[prev_i][1].control_point(0);
            }
          else
            {
              shapes[i].current_outline() << innerCorners[prev_i];
            }
        }
      else
        {
          shapes[i].current_outline() << outerRectCenter;
        }

    }
  
}

class RenderBoxModelObject_ReadyWRATHWidgetFillLayerExtended: 
    public WebCore::PaintedWidgetsOfWRATH<RenderBoxModelObject_ReadyWRATHWidgetFillLayerExtended>
{
public:
    RenderBoxModelObject_ReadyWRATHWidgetFillLayerExtended(void)
      : m_border_radius_clip_rect(0, 0, 0, 0)
    {}

    void makeAllNonVisible()
    {
	if (m_simple_color_round.widget())
	    m_simple_color_round.widget()->visible(false);
	if (m_simple_color_nonround.widget())
	    m_simple_color_nonround.widget()->visible(false);
	if (m_normal_bg.widget())
	    m_normal_bg.widget()->visible(false);
        if (m_border_radius_clip_canvas.widget())
            m_border_radius_clip_canvas.widget()->visible(false);
	if (m_color_under_images.widget())
	    m_color_under_images.widget()->visible(false);
        if(m_clear_rect.widget())
            m_clear_rect.widget()->visible(false);
        if(m_local_scroll_clip_node.widget())
          m_local_scroll_clip_node.widget()->visible(false);
        m_bg_image.visible(false);
    }

    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_local_scroll_clip_node;
    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_background_clip_node;
  
    WebCore::RoundedFilledRectOfWRATH m_simple_color_round;
    WebCore::FilledIntRectOfWRATH m_simple_color_nonround;
    WebCore::FilledIntRectOfWRATH m_normal_bg;

    WebCore::ContextOfWRATH::CPlainFamily::DrawnShape::AutoDelete m_border_radius_clip;
    WebCore::ContextOfWRATH::DrawnCanvas::AutoDelete m_border_radius_clip_canvas;
    WRATHShapeF m_border_radius_clip_shape;
    WebCore::RoundedIntRect m_border_radius_clip_rect;
    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_border_extra_clipping;


    WebCore::FilledIntRectOfWRATH m_color_under_images;

    WebCore::FilledIntRectOfWRATH m_clear_rect;
    WebCore::IntRect m_clear_rect_size;

    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::Image> m_bg_image;
};

}

namespace WebCore {



static IntRect backgroundRectAdjustedForBleedAvoidance_WRATH(const IntRect& borderRect, BackgroundBleedAvoidance bleedAvoidance)
{
  return borderRect;

    if (bleedAvoidance != BackgroundBleedShrinkBackground)
        return borderRect;

    IntRect adjustedRect = borderRect;
    // We need to shrink the border by one device pixel on each side.
    /* [WRATH-TODO]: what to do with affine transforms */
    //    AffineTransform ctm = context->getCTM();
    //    FloatSize contextScale(static_cast<float>(ctm.xScale()), static_cast<float>(ctm.yScale()));
    FloatSize contextScale(1, 1);
    adjustedRect.inflateX(-ceilf(1 / contextScale.width()));
    adjustedRect.inflateY(-ceilf(1 / contextScale.height()));
    return adjustedRect;
}

void RenderBoxModelObject::readyWRATHWidgetBorder(PaintedWidgetsOfWRATHHandle& handle,
                                                  ContextOfWRATH *wrath_context,
                                                  int tx, int ty,
                                                  int w, int h,
                                                  const RenderStyle* style,
                                                  BackgroundBleedAvoidance bleedAvoidance,
                                                  bool includeLogicalLeftEdge,
                                                  bool includeLogicalRightEdge )
{

    RenderBoxModelObject_WRATHWidgetBorder *d;

    d=RenderBoxModelObject_WRATHWidgetBorder::object(this, handle);
    ContextOfWRATH::AutoPushNode autoPushRoot(wrath_context, d->m_root_node);
    
    d->make_all_invisible();
    
    if(readyWRATHWidgetNinePieceImage(d->m_nine_piece_image, wrath_context, tx, ty, w, h, style, style->borderImage())) {
      d->m_nine_piece_image.visible(true);
      return;
    }
    
    BorderEdge edges[4];
    getBorderEdgeInfo(edges, includeLogicalLeftEdge, includeLogicalRightEdge);

    IntRect borderRect(tx, ty, w, h);
    RoundedIntRect outerBorder = style->getRoundedBorderFor(borderRect, includeLogicalLeftEdge, includeLogicalRightEdge);
    RoundedIntRect innerBorder = style->getRoundedInnerBorderFor(borderRect, includeLogicalLeftEdge, includeLogicalRightEdge);

       
    
    /*
      first check if we need to reconstruct the shapes.
     */
    ivec2 delta;
    delta.x()=outerBorder.rect().x() - innerBorder.rect().x();
    delta.y()=outerBorder.rect().y() - innerBorder.rect().y();

    d->m_shapes_of_edges_dirty=d->m_shapes_of_edges_dirty
      or delta!=d->m_rect_delta
      or d->m_outerBorder.radii()!=outerBorder.radii()
      or d->m_outerBorder.rect().size()!=outerBorder.rect().size()
      or d->m_innerBorder.rect().size()!=innerBorder.rect().size()
      or d->m_innerBorder.radii()!=innerBorder.radii();


    if(d->m_shapes_of_edges_dirty) {

      d->m_outerBorder=outerBorder;
      d->m_innerBorder=innerBorder;
      d->m_rect_delta=delta;
      
      IntSize moveBy(-d->m_outerBorder.rect().x(),
                     -d->m_outerBorder.rect().y());
      
      d->m_outerBorder.move(moveBy);
      d->m_innerBorder.move(moveBy);
      
      RenderBoxModelObject_WRATHWidgetBorder::construct_shapes(d->m_shapes_of_edges, 
                                                               d->m_outerBorder,
                                                               d->m_innerBorder);
    }
      
    d->m_position=vec2(outerBorder.rect().x(), outerBorder.rect().y());

    for (int i = BSTop; i <= BSLeft; ++i) {
        const BorderEdge& currEdge = edges[i];
        vec4 c;

        /*
          [WRATH-TODO]: if the shape is transparent, we need
          to obey the transparency value.
         */
        currEdge.color.getRGBA(c.x(), c.y(), c.z(), c.w());
        if(d->m_edges[i].widget()) {

          if(d->m_shapes_of_edges_dirty) {
            d->m_edges[i].widget()->properties()->change_shape(WRATHWidgetGenerator::shape_value(d->m_shapes_of_edges[i]));
          }

          d->m_edges[i].widget()->node()->color(c);
          wrath_context->update_generic(d->m_edges[i]);
        }
        else {
          wrath_context->add_filled_shape(d->m_edges[i],
                                          WRATHWidgetGenerator::ColorProperties(c),
                                          WRATHWidgetGenerator::shape_value(d->m_shapes_of_edges[i]));
        }
        
        /*
          [WRATH-TODO] support various values of currEdge.style
         */
        d->m_edges[i].widget()->visible(currEdge.isPresent && currEdge.hasVisibleColorAndStyle() and currEdge.width>0);
        d->m_edges[i].widget()->position(d->m_position);
    }
    d->m_shapes_of_edges_dirty=false;

    

    

}

bool RenderBoxModelObject::readyWRATHWidgetNinePieceImage(PaintedWidgetsOfWRATHHandle& handle,
                                                          ContextOfWRATH *wrath_context,
                                                          int tx, int ty,
                                                          int w, int h,
                                                          const RenderStyle *style,
                                                          const NinePieceImage &ninePieceImage,
                                                          CompositeOperator op)
{
    RenderBoxModelObject_WRATHNinePieceImage *d;
    d=RenderBoxModelObject_WRATHNinePieceImage::object(this, handle);
    
    ContextOfWRATH::AutoPushNode autoPushRoot(wrath_context, d->m_root_node);
    d->make_all_invisible();

    StyleImage* styleImage = ninePieceImage.image();
    if (!styleImage)
        return false;

    if (!styleImage->isLoaded())
        return true; // Never paint a nine-piece image incrementally, but don't paint the fallback borders either.

    if (!styleImage->canRender(style->effectiveZoom()))
        return false;

    // FIXME: border-image is broken with full page zooming when tiling has to happen, since the tiling function
    // doesn't have any understanding of the zoom that is in effect on the tile.
    styleImage->setImageContainerSize(IntSize(w, h));
    IntSize imageSize = styleImage->imageSize(this, 1.0f);
    int imageWidth = imageSize.width();
    int imageHeight = imageSize.height();

    int topSlice = std::min(imageHeight, ninePieceImage.slices().top().calcValue(imageHeight));
    int bottomSlice = std::min(imageHeight, ninePieceImage.slices().bottom().calcValue(imageHeight));
    int leftSlice = std::min(imageWidth, ninePieceImage.slices().left().calcValue(imageWidth));
    int rightSlice = std::min(imageWidth, ninePieceImage.slices().right().calcValue(imageWidth));

    ENinePieceImageRule hRule = ninePieceImage.horizontalRule();
    ENinePieceImageRule vRule = ninePieceImage.verticalRule();

    bool fitToBorder = style->borderImage() == ninePieceImage;
    
    int leftWidth = fitToBorder ? style->borderLeftWidth() : leftSlice;
    int topWidth = fitToBorder ? style->borderTopWidth() : topSlice;
    int rightWidth = fitToBorder ? style->borderRightWidth() : rightSlice;
    int bottomWidth = fitToBorder ? style->borderBottomWidth() : bottomSlice;

    bool drawLeft = leftSlice > 0 && leftWidth > 0;
    bool drawTop = topSlice > 0 && topWidth > 0;
    bool drawRight = rightSlice > 0 && rightWidth > 0;
    bool drawBottom = bottomSlice > 0 && bottomWidth > 0;
    bool drawMiddle = (imageWidth - leftSlice - rightSlice) > 0 && (w - leftWidth - rightWidth) > 0 &&
                      (imageHeight - topSlice - bottomSlice) > 0 && (h - topWidth - bottomWidth) > 0;

    RefPtr<Image> image = styleImage->image(this, imageSize);
    ColorSpace colorSpace = style->colorSpace();

    if (drawLeft) {
        // Paint the top and bottom left corners.

        // The top left corner rect is (tx, ty, leftWidth, topWidth)
        // The rect to use from within the image is obtained from our slice, and is (0, 0, leftSlice, topSlice)
        if (drawTop) {
          d->m_left_top.visible(true);
          WRATH_drawImage(d->m_left_top, wrath_context, 
                          image.get(), colorSpace, IntRect(tx, ty, leftWidth, topWidth),
                          IntRect(0, 0, leftSlice, topSlice), op);
        }

        // The bottom left corner rect is (tx, ty + h - bottomWidth, leftWidth, bottomWidth)
        // The rect to use from within the image is (0, imageHeight - bottomSlice, leftSlice, botomSlice)
        if (drawBottom) {
          d->m_left_bottom.visible(true);
          WRATH_drawImage(d->m_left_bottom, wrath_context,
                          image.get(), colorSpace, IntRect(tx, ty + h - bottomWidth, leftWidth, bottomWidth),
                          IntRect(0, imageHeight - bottomSlice, leftSlice, bottomSlice), op);
        }

        // Paint the left edge.
        // Have to scale and tile into the border rect.
        d->m_left.visible(true);
        WRATH_drawTiledImage(d->m_left, wrath_context,
                             image.get(), colorSpace, IntRect(tx, ty + topWidth, leftWidth,
                                                              h - topWidth - bottomWidth),
                             IntRect(0, topSlice, leftSlice, imageHeight - topSlice - bottomSlice),
                             Image::StretchTile, (Image::TileRule)vRule, op);
    }

    if (drawRight) {
        // Paint the top and bottom right corners
        // The top right corner rect is (tx + w - rightWidth, ty, rightWidth, topWidth)
        // The rect to use from within the image is obtained from our slice, and is (imageWidth - rightSlice, 0, rightSlice, topSlice)
        if (drawTop) {
          d->m_right_top.visible(true);
          WRATH_drawImage(d->m_right_top, wrath_context,
                          image.get(), colorSpace, IntRect(tx + w - rightWidth, ty, rightWidth, topWidth),
                          IntRect(imageWidth - rightSlice, 0, rightSlice, topSlice), op);
        }

        // The bottom right corner rect is (tx + w - rightWidth, ty + h - bottomWidth, rightWidth, bottomWidth)
        // The rect to use from within the image is (imageWidth - rightSlice, imageHeight - bottomSlice, rightSlice, bottomSlice)
        if (drawBottom) {
          d->m_right_bottom.visible(true);
          WRATH_drawImage(d->m_right_bottom, wrath_context,
                          image.get(), colorSpace, 
                          IntRect(tx + w - rightWidth, ty + h - bottomWidth, rightWidth, bottomWidth),
                          IntRect(imageWidth - rightSlice, imageHeight - bottomSlice,
                                  rightSlice, bottomSlice), op);
        }

        // Paint the right edge.
        d->m_right.visible(true);
        WRATH_drawTiledImage(d->m_right, wrath_context,
                             image.get(), colorSpace, 
                             IntRect(tx + w - rightWidth, ty + topWidth, rightWidth,
                                     h - topWidth - bottomWidth),
                             IntRect(imageWidth - rightSlice, topSlice, 
                                     rightSlice, imageHeight - topSlice - bottomSlice),
                             Image::StretchTile, (Image::TileRule)vRule, op);
    }

    // Paint the top edge.
    if (drawTop) {
      d->m_top.visible(true);
      WRATH_drawTiledImage(d->m_top, wrath_context,
                           image.get(), colorSpace, 
                           IntRect(tx + leftWidth, ty, w - leftWidth - rightWidth, topWidth),
                           IntRect(leftSlice, 0, imageWidth - rightSlice - leftSlice, topSlice),
                           (Image::TileRule)hRule, Image::StretchTile, op);
    }

    // Paint the bottom edge.
    if (drawBottom) {
      d->m_bottom.visible(true);
      WRATH_drawTiledImage(d->m_bottom, wrath_context, 
                           image.get(), colorSpace, 
                           IntRect(tx + leftWidth, ty + h - bottomWidth,
                                   w - leftWidth - rightWidth, bottomWidth),
                           IntRect(leftSlice, imageHeight - bottomSlice, 
                                   imageWidth - rightSlice - leftSlice, bottomSlice),
                           (Image::TileRule)hRule, Image::StretchTile, op);
    }

    // Paint the middle.
    if (drawMiddle) {
        d->m_middle.visible(true);
        WRATH_drawTiledImage(d->m_middle, wrath_context, 
                             image.get(), colorSpace, 
                             IntRect(tx + leftWidth, ty + topWidth,
                                     w - leftWidth - rightWidth,
                                     h - topWidth - bottomWidth),
                             IntRect(leftSlice, topSlice, 
                                     imageWidth - rightSlice - leftSlice, 
                                     imageHeight - topSlice - bottomSlice),
                             (Image::TileRule)hRule, (Image::TileRule)vRule, op);
    }
    
    return true;


}





void RenderBoxModelObject::readyWRATHWidgetBoxShadow(PaintedWidgetsOfWRATHHandle&, ContextOfWRATH *ctx,
                                                     int tx, int ty, int w, int h, const RenderStyle *s, ShadowStyle shadowStyle,
                                                     bool includeLogicalLeftEdge, bool includeLogicalRightEdge)
{
    if (!s->boxShadow())
      return;

    WRATH_UNIMPLEMENTED(ctx);
    /*[WRATH-TODO]: paintBoxShadow */
}




void RenderBoxModelObject::readyWRATHWidgetFillLayerExtended(PaintedWidgetsOfWRATHHandle& handle,
                                                             const PaintInfoOfWRATH& paintInfo,
                                                             const Color& color, const FillLayer* bgLayer,
                                                             int tx, int ty, int w, int h,
                                                             BackgroundBleedAvoidance bleedAvoidance,
							     InlineFlowBox* box,
                                                             int inlineBoxWidth, int inlineBoxHeight, 
                                                             CompositeOperator op, RenderObject* backgroundObject)
{
    RenderBoxModelObject_ReadyWRATHWidgetFillLayerExtended *d(RenderBoxModelObject_ReadyWRATHWidgetFillLayerExtended::object(this, handle));
    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    d->makeAllNonVisible();

    IntRect borderRect(tx, ty, w, h);
    if (borderRect.isEmpty())
        return;

    bool includeLeftEdge = box ? box->includeLogicalLeftEdge() : true;
    bool includeRightEdge = box ? box->includeLogicalRightEdge() : true;

    bool hasRoundedBorder = style()->hasBorderRadius() && (includeLeftEdge || includeRightEdge);
    bool clippedWithLocalScrolling = hasOverflowClip() && bgLayer->attachment() == LocalBackgroundAttachment;
    bool isBorderFill = bgLayer->clip() == BorderFillBox;
    bool isRoot = this->isRoot();

    Color bgColor = color;
    StyleImage* bgImage = bgLayer->image();
    bool shouldPaintBackgroundImage = bgImage && bgImage->canRender(style()->effectiveZoom());
    
    // When this style flag is set, change existing background colors and images to a solid white background.
    // If there's no bg color or image, leave it untouched to avoid affecting transparency.
    // We don't try to avoid loading the background images, because this style flag is only set
    // when printing, and at that point we've already loaded the background images anyway. (To avoid
    // loading the background images we'd have to do this check when applying styles rather than
    // while rendering.)
    if (style()->forceBackgroundsToWhite()) {
        // Note that we can't reuse this variable below because the bgColor might be changed
        bool shouldPaintBackgroundColor = !bgLayer->next() && bgColor.isValid() && bgColor.alpha() > 0;
        if (shouldPaintBackgroundImage || shouldPaintBackgroundColor) {
            bgColor = Color::white;
            shouldPaintBackgroundImage = false;
        }
    }

    bool colorVisible = bgColor.isValid() && bgColor.alpha() > 0;
    
    // Fast path for drawing simple color backgrounds.
    if (!isRoot && !clippedWithLocalScrolling && !shouldPaintBackgroundImage && isBorderFill) {

        if (!colorVisible)
            return;

        if (hasRoundedBorder && bleedAvoidance != BackgroundBleedUseTransparencyLayer) {
	      RoundedIntRect border = getBackgroundRoundedRect(backgroundRectAdjustedForBleedAvoidance_WRATH(borderRect, bleedAvoidance), box, inlineBoxWidth, inlineBoxHeight, includeLeftEdge, includeRightEdge);

              d->m_simple_color_round.update(paintInfo.wrath_context, border, bgColor, CompositeSourceOver);
              if(d->m_simple_color_round.widget())
                d->m_simple_color_round.widget()->visible(true);
        } else {
            d->m_simple_color_nonround.update(paintInfo.wrath_context, borderRect, bgColor, CompositeSourceOver);
            if(d->m_simple_color_nonround.widget())
              d->m_simple_color_nonround.widget()->visible(true);
	}
        
        return;
    }

    bool clipToBorderRadius = true && hasRoundedBorder && bleedAvoidance != BackgroundBleedUseTransparencyLayer;

    if (clipToBorderRadius) {
        bool shape_changed(false);
        RoundedIntRect border = getBackgroundRoundedRect(backgroundRectAdjustedForBleedAvoidance_WRATH(borderRect, bleedAvoidance), box, inlineBoxWidth, inlineBoxHeight, includeLeftEdge, includeRightEdge);

	if (!d->m_border_radius_clip.widget() ||
            d->m_border_radius_clip_rect.rect().size() != border.rect().size() ||
	    !(d->m_border_radius_clip_rect.radii() == border.radii()))  {

          shape_changed=true;
          d->m_border_radius_clip_shape.clear();
          RoundedFilledRectOfWRATH::AddToShape_TranslateToOrigin(true, d->m_border_radius_clip_shape, border);
          d->m_border_radius_clip_rect = border;
        }

	WRATH_PUSH_CANVAS_NODE(paintInfo.wrath_context, d->m_border_radius_clip_canvas);

        if(!d->m_border_radius_clip.widget()) {
          paintInfo.wrath_context->canvas_clipping()
            .clip_filled_shape(WRATHWidgetGenerator::clip_inside, d->m_border_radius_clip,
                               WRATHWidgetGenerator::shape_value(d->m_border_radius_clip_shape));
        }
        else if(shape_changed) {
          d->m_border_radius_clip.widget()->properties()
            ->change_shape(WRATHWidgetGenerator::shape_value(d->m_border_radius_clip_shape),
                           WRATHWidgetGenerator::FillingParameters());
        }

        d->m_border_radius_clip.widget()->position(vec2(border.rect().x(), border.rect().y()));
        d->m_border_radius_clip_canvas.widget()->visible(true);

        paintInfo.wrath_context->push_node(d->m_border_extra_clipping);
        ContextOfWRATH::set_clipping(d->m_border_extra_clipping, border.rect());
    }
    
    int bLeft = includeLeftEdge ? borderLeft() : 0;
    int bRight = includeRightEdge ? borderRight() : 0;
    int pLeft = includeLeftEdge ? paddingLeft() : 0;
    int pRight = includeRightEdge ? paddingRight() : 0;

    /*
      make those autopushed elements get popped
      before we pop d->m_border_extra_clipping
      and pop d->m_border_radius_clip_canvas
     */
    {
      ContextOfWRATH::AutoPushNode autoPushLocalScrollClip(paintInfo.wrath_context, d->m_local_scroll_clip_node);
      d->m_local_scroll_clip_node.widget()->node()->clipping_active(false);
      d->m_local_scroll_clip_node.widget()->node()->visible(true);
      
      if (clippedWithLocalScrolling) {
        ContextOfWRATH::set_clipping(d->m_local_scroll_clip_node,
                                     toRenderBox(this)->overflowClipRect(tx, ty));
        d->m_local_scroll_clip_node.widget()->node()->clipping_active(true);
        
        // Now adjust our tx, ty, w, h to reflect a scrolled content box with borders at the ends.
        IntSize offset = layer()->scrolledContentOffset();
        tx -= offset.width();
        ty -= offset.height();
        w = bLeft + layer()->scrollWidth() + bRight;
        h = borderTop() + layer()->scrollHeight() + borderBottom();
      }
      
      ContextOfWRATH::AutoPushNode autoPushBackgroundClip(paintInfo.wrath_context, d->m_background_clip_node);
      d->m_background_clip_node.widget()->node()->clipping_active(false);
      if (bgLayer->clip() == PaddingFillBox || bgLayer->clip() == ContentFillBox) {
        // Clip to the padding or content boxes as necessary.
        bool includePadding = bgLayer->clip() == ContentFillBox;
        int x = tx + bLeft + (includePadding ? pLeft : 0);
        int y = ty + borderTop() + (includePadding ? paddingTop() : 0);
        int width = w - bLeft - bRight - (includePadding ? pLeft + pRight : 0);
        int height = h - borderTop() - borderBottom() - (includePadding ? paddingTop() + paddingBottom() : 0);
        ContextOfWRATH::set_clipping(d->m_background_clip_node, IntRect(x, y, width, height));
        d->m_background_clip_node.widget()->node()->clipping_active(true);
      } else if (bgLayer->clip() == TextFillBox) {
        std::cout << "\nUnimplemented for madness bgLayer->clip() == TextFillBox" << std::flush;
#if 0
	/* [WRATH-TODO]: Implement this branch */
        // We have to draw our text into a mask that can then be used to clip background drawing.
        // First figure out how big the mask has to be.  It should be no bigger than what we need
        // to actually render, so we should intersect the dirty rect with the border box of the background.
        IntRect maskRect(tx, ty, w, h);
        maskRect.intersect(paintInfo.rect);
        
        // Now create the mask.
        OwnPtr<ImageBuffer> maskImage = ImageBuffer::create(maskRect.size());
        if (!maskImage)
          return;
        
	GraphicsContext* maskImageContext = maskImage->context();
        maskImageContext->translate(-maskRect.x(), -maskRect.y());
        
        // Now add the text to the clip.  We do this by painting using a special paint phase that signals to
        // InlineTextBoxes that they should just add their contents to the clip.
        PaintInfo info(maskImageContext, maskRect, PaintPhaseTextClip, true, 0, 0);
        if (box) {
          RootInlineBox* root = box->root();
          box->paint(info, tx - box->x(), ty - box->y(), root->lineTop(), root->lineBottom());
        } else {
            int x = isBox() ? toRenderBox(this)->x() : 0;
            int y = isBox() ? toRenderBox(this)->y() : 0;
            paint(info, tx - x, ty - y);
        }
        
        // The mask has been created.  Now we just need to clip to it.
        backgroundClipStateSaver.save();
        context->clipToImageBuffer(maskImage.get(), maskRect);
#endif  // #if 0
      }
      
      // Only fill with a base color (e.g., white) if we're the root document, since iframes/frames with
      // no background in the child document should show the parent's background.
      bool isOpaqueRoot = false;
      if (isRoot) {
        isOpaqueRoot = true;
        if (!bgLayer->next() && !(bgColor.isValid() && bgColor.alpha() == 255) && view()->frameView()) {
          Element* ownerElement = document()->ownerElement();
          if (ownerElement) {
            if (!ownerElement->hasTagName(HTMLNames::frameTag)) {
              // Locate the <body> element using the DOM.  This is easier than trying
              // to crawl around a render tree with potential :before/:after content and
              // anonymous blocks created by inline <body> tags etc.  We can locate the <body>
              // render object very easily via the DOM.
              HTMLElement* body = document()->body();
              if (body) {
                // Can't scroll a frameset document anyway.
                isOpaqueRoot = body->hasLocalName(HTMLNames::framesetTag);
              }
#if ENABLE(SVG)
              else {
                // SVG documents and XML documents with SVG root nodes are transparent.
                isOpaqueRoot = !document()->hasSVGRootNode();
              }
#endif
            }
          } else
            isOpaqueRoot = !view()->frameView()->isTransparent();
        }
        view()->frameView()->setContentIsOpaque(isOpaqueRoot);
      }

      // Paint the color first underneath all images.
      if (!bgLayer->next()) {
        IntRect rect(tx, ty, w, h);
        rect.intersect(paintInfo.rect);
        // If we have an alpha and we are painting the root element, go ahead and blend with the base background color.
        if (isOpaqueRoot) {
          Color baseColor = view()->frameView()->baseBackgroundColor();
          if (baseColor.alpha() > 0) {
            /*
              CompositeOperator previousOperator = context->compositeOperation();
              context->setCompositeOperation(CompositeCopy);
              context->fillRect(rect, baseColor, style()->colorSpace());
              context->setCompositeOperation(previousOperator);
            */
            d->m_color_under_images.update(paintInfo.wrath_context, rect, baseColor, CompositeCopy);
            if(d->m_color_under_images.widget())
              d->m_color_under_images.widget()->visible(true);            
          } else {
            d->m_clear_rect.update(paintInfo.wrath_context, rect);
            if(d->m_clear_rect.widget())
              d->m_clear_rect.widget()->visible(true);
          }
        }
        
        if (bgColor.isValid() && bgColor.alpha() > 0) {
          d->m_normal_bg.update(paintInfo.wrath_context, rect, bgColor, CompositeSourceOver);
          if(d->m_normal_bg.widget())
            d->m_normal_bg.widget()->visible(true);
	}
      }
      
      // no progressive loading of the background image
      if (shouldPaintBackgroundImage) {
        IntRect destRect;
        IntPoint phase;
        IntSize tileSize;
        
        calculateBackgroundImageGeometry(bgLayer, tx, ty, w, h, destRect, phase, tileSize);
        IntPoint destOrigin = destRect.location();
        destRect.intersect(paintInfo.rect);
        if (!destRect.isEmpty()) {
          phase += destRect.location() - destOrigin;
          CompositeOperator compositeOp = op == CompositeSourceOver ? bgLayer->composite() : op;
          RenderObject* clientForBackgroundImage = backgroundObject ? backgroundObject : this;
          RefPtr<Image> image = bgImage->image(clientForBackgroundImage, tileSize);
          /*
            [WRATH-DANGER]: What to do with this call?
            bool useLowQualityScaling = shouldPaintAtLowQuality(context, image.get(), bgLayer, tileSize);
          */

          d->m_bg_image.visible(true);
          WRATH_drawTiledImage(d->m_bg_image, paintInfo.wrath_context, image.get(), 
                               style()->colorSpace(), destRect, phase, tileSize, compositeOp, false /*useLowQualityScaling*/);
        }
      }
    }


    if (clipToBorderRadius) {
	paintInfo.wrath_context->pop_node();
        paintInfo.wrath_context->pop_node();
    }
}


} /* namespace WebCore */

#endif /* USE(WRATH) */
