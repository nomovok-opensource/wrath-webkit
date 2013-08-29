#include "config.h"
#include "RenderObject.h"

#include "AXObjectCache.h"
#include "CSSStyleSelector.h"
#include "Chrome.h"
#include "ContentData.h"
#include "CursorList.h"
#include "DashArray.h"
#include "EditingBoundary.h"
#include "FloatQuad.h"
#include "Frame.h"
#include "FrameView.h"
#include "GraphicsContext.h"
#include "HTMLNames.h"
#include "HitTestResult.h"
#include "Page.h"
#include "RenderArena.h"
#include "RenderCounter.h"
#include "RenderFlexibleBox.h"
#include "RenderImage.h"
#include "RenderImageResourceStyleImage.h"
#include "RenderInline.h"
#include "RenderLayer.h"
#include "RenderListItem.h"
#include "RenderRuby.h"
#include "RenderRubyText.h"
#include "RenderTableCell.h"
#include "RenderTableCol.h"
#include "RenderTableRow.h"
#include "RenderTheme.h"
#include "RenderView.h"
#include "TransformState.h"
#include "htmlediting.h"
#include <algorithm>
#include <stdio.h>
#include <wtf/RefCountedLeakCounter.h>
#include <wtf/UnusedParam.h>

#if USE(ACCELERATED_COMPOSITING)
#include "RenderLayerCompositor.h"
#endif

#if ENABLE(SVG)
#include "RenderSVGResourceContainer.h"
#include "SVGRenderSupport.h"
#endif

using namespace std;

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"

namespace {

class RenderObject_WRATHWidgetsOutline:
    public WebCore::PaintedWidgetsOfWRATH<RenderObject_WRATHWidgetsOutline>
{
public:
  WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_skip_node;
  WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_box_sides_node;
  
  WebCore::PaintedWidgetsOfWRATHHandle m_focus_ring;
  vecN<WebCore::PaintedWidgetsOfWRATHHandle, 4> m_sides;
  
};
  
class RenderObject_WRATHWidgetsDrawLineForBoxSide:
    public WebCore::PaintedWidgetsOfWRATH<RenderObject_WRATHWidgetsDrawLineForBoxSide>
{
public:
  
  WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_skip_node;
  
  /*
    drawing a side is really just drawing a rect or stroking a line:
  */
  WebCore::PaintedWidgetsOfWRATHHandle m_dotted_or_dashed;
  WebCore::PaintedWidgetsOfWRATHHandle m_double;
  WebCore::PaintedWidgetsOfWRATHHandle m_ridge_or_groove;
  WebCore::PaintedWidgetsOfWRATHHandle m_inset_outset_or_solid;
};

class RenderObject_WRATHWidgetDrawLineForBoxSideDottedOrDashed:
    public WebCore::PaintedWidgetsOfWRATH<RenderObject_WRATHWidgetDrawLineForBoxSideDottedOrDashed>
{
public:
  /* Add ctor */
  RenderObject_WRATHWidgetDrawLineForBoxSideDottedOrDashed(void):
    m_line_twice_dimensions(-1, -1),
    m_stroke_width(-1)
  {}
  
  static
  void ready(WebCore::RenderObject*,
             WebCore::PaintedWidgetsOfWRATHHandle&,
             WebCore::ContextOfWRATH*,
             WebCore::BoxSide side,
             WebCore::Color color, WebCore::EBorderStyle style,
             int width,
             int x1, int y1, int x2, int y2);
    
  ivec2 m_line_twice_dimensions;
  int m_stroke_width;
  WRATHShapeF m_line_shape;
  enum WRATHWidgetGenerator::pen_style_type m_stroke_style;
  WebCore::ContextOfWRATH::CColorFamily::DrawnShape::AutoDelete m_shape_item;
};

void RenderObject_WRATHWidgetDrawLineForBoxSideDottedOrDashed::ready(WebCore::RenderObject *renderObject,
                                                                     WebCore::PaintedWidgetsOfWRATHHandle &handle,
                                                                     WebCore::ContextOfWRATH *wrath_context,
                                                                     WebCore::BoxSide side,
                                                                     WebCore::Color color, 
                                                                     WebCore::EBorderStyle style,
                                                                     int width,
                                                                     int x1, int y1, int x2, int y2)
{
  WebCore::IntPoint twice_p0, twice_p1;

  switch (side) {
  case WebCore::BSBottom:
  case WebCore::BSTop:
    twice_p0=WebCore::IntPoint(2*x1, (y1 + y2) );
    twice_p1=WebCore::IntPoint(2*x2, (y1 + y2) );
    break;

  case WebCore::BSRight:
  case WebCore::BSLeft:
    twice_p0=WebCore::IntPoint(x1 + x2, 2*y1);
    twice_p1=WebCore::IntPoint(x1 + x2, 2*y2);
    break;
  }

  /*
    add a line connecting p0 to p1 stroked either DAHSED or Dotted:
   */
  RenderObject_WRATHWidgetDrawLineForBoxSideDottedOrDashed *d;

  d=RenderObject_WRATHWidgetDrawLineForBoxSideDottedOrDashed::object(renderObject, handle);
  WebCore::ContextOfWRATH::AutoPushNode autoPushRoot(wrath_context, d->m_root_node);

  ivec2 line_twice_dimensions(twice_p1.x()-twice_p0.x(), twice_p1.y()-twice_p0.y());
  enum WRATHWidgetGenerator::pen_style_type stroke_style;

  stroke_style=(style==WebCore::DASHED)?
    WRATHWidgetGenerator::dashed_stroke:
    WRATHWidgetGenerator::dotted_stroke;

  if(width<=0) {
    if(d->m_shape_item.widget()) {
      d->m_shape_item.widget()->visible(false);
      return;
    }

  }
    
  if(!d->m_shape_item.widget()
     or line_twice_dimensions!=d->m_line_twice_dimensions
     or stroke_style!=d->m_stroke_style
     or width!=d->m_stroke_width)
    {    
      d->m_stroke_style=stroke_style;
      d->m_stroke_width=width;

      if(!d->m_shape_item.widget() or line_twice_dimensions!=d->m_line_twice_dimensions) {
        d->m_line_shape.clear();
        d->m_line_shape.current_outline() 
        << WRATHOutline<float>::position_type(0, 0)
        << WRATHOutline<float>::position_type(static_cast<float>(line_twice_dimensions.x())*0.5f, 
                                              static_cast<float>(line_twice_dimensions.y())*0.5f);
        d->m_line_twice_dimensions=line_twice_dimensions;
      }

      if(!d->m_shape_item.widget()) 
        {
          vec4 c(0.0f, 0.0f, 0.0f, 1.0f);
          /*
            [WRATH-TODO]: if the shape is transparent, we need
            to obey the transparency value.
          */
          color.getRGBA(c.x(), c.y(), c.z(), c.w());
          wrath_context->add_stroked_shape(d->m_shape_item,
                                           WRATHWidgetGenerator::ColorProperties(c),
                                           WRATHWidgetGenerator::shape_value(d->m_line_shape),
                                           WRATHWidgetGenerator::StrokingParameters()
                                           .close_outline(false)
                                           .width(d->m_stroke_width)
                                           .stroke_curves(d->m_stroke_style)
                                           .cap_style(WRATHWidgetGenerator::flat_cap));
          d->m_shape_item.widget()->position( vec2(twice_p0.x(), twice_p0.y())*0.5f );
          return;
        }
      else
        {
          WRATHShapeItem *item;

          item=d->m_shape_item.widget()->properties();

          item->change_shape(WRATHWidgetGenerator::shape_value(d->m_line_shape),
                             WRATHWidgetGenerator::StrokingParameters()
                             .close_outline(false)
                             .width(d->m_stroke_width)
                             .stroke_curves(d->m_stroke_style)
                             .cap_style(WRATHWidgetGenerator::flat_cap));
        }
    }

  /*
    update the color and position...
   */
  vec4 c(0.0f, 0.0f, 0.0f, 1.0f);
  color.getRGBA(c.x(), c.y(), c.z(), c.w());

  wrath_context->update_generic(d->m_shape_item);
  d->m_shape_item.widget()->visible(color.alpha()!=0);
  d->m_shape_item.widget()->position( vec2(twice_p0.x(), twice_p0.y())*0.5f );
  d->m_shape_item.widget()->node()->color(c);        
}


class RenderObject_WRATHWidgetDrawLineForBoxSideDouble:
    public WebCore::PaintedWidgetsOfWRATH<RenderObject_WRATHWidgetDrawLineForBoxSideDouble>
{
public:
  
  
  static void ready(WebCore::RenderObject*, 
                    WebCore::PaintedWidgetsOfWRATHHandle&,
                    WebCore::ContextOfWRATH*, WebCore::BoxSide side,
                    WebCore::Color color, int width,
                    int adjacentWidth1, int adjacentWidth2,
                    int x1, int y1, int x2, int y2);


  vecN<ivec2, 2> m_rect_dims;
  WebCore::ContextOfWRATH::ColorFamily::DrawnRect::AutoDelete m_rect0, m_rect1;
  
  WebCore::PaintedWidgetsOfWRATHHandle m_s0, m_s1;
};

void RenderObject_WRATHWidgetDrawLineForBoxSideDouble::ready(WebCore::RenderObject *renderObject, 
                                                             WebCore::PaintedWidgetsOfWRATHHandle &handle,
                                                             WebCore::ContextOfWRATH *wrath_context,
                                                             WebCore::BoxSide side,
                                                             WebCore::Color color, int width,
                                                             int adjacentWidth1, int adjacentWidth2,
                                                             int x1, int y1, int x2, int y2)
{
  RenderObject_WRATHWidgetDrawLineForBoxSideDouble *d;

  d=RenderObject_WRATHWidgetDrawLineForBoxSideDouble::object(renderObject, handle);
  WebCore::ContextOfWRATH::AutoPushNode autoPushRoot(wrath_context, d->m_root_node);
  

  int third = (width + 1) / 3;
  
  if(d->m_rect0.widget()) {
    WRATHassert(d->m_rect1.widget());
    d->m_rect0.widget()->visible(false);
    d->m_rect1.widget()->visible(false);
  }

  d->m_s0.visible(false);
  d->m_s1.visible(false);


  if(adjacentWidth1==0 or adjacentWidth2==0)
    {
      vecN<WebCore::IntRect,2> rects;
      vecN<ivec2, 2> rects_dim;
      vec4 color_as_float;

      switch (side) {
      case WebCore::BSTop:
      case WebCore::BSBottom:
        rects[0]=WebCore::IntRect(x1, y1, x2 - x1, third);
        rects[1]=WebCore::IntRect(x1, y2 - third, x2 - x1, third);
        break;
      case WebCore::BSLeft:
        rects[0]=WebCore::IntRect(x1, y1 + 1, third, y2 - y1 - 1);
        rects[1]=WebCore::IntRect(x2 - third, y1 + 1, third, y2 - y1 - 1);
        break;
      case WebCore::BSRight:
        rects[0]=WebCore::IntRect(x1, y1 + 1, third, y2 - y1 - 1);
        rects[1]=WebCore::IntRect(x2 - third, y1 + 1, third, y2 - y1 - 1);
        break;
      }

      for(int i=0;i<2;++i)
        {
          rects_dim[i]=ivec2(rects[i].width(), rects[i].height());
        }
      
      

      color.getRGBA(color_as_float.x(), color_as_float.y(), 
                    color_as_float.z(), color_as_float.w());
        

      if(!d->m_rect0.widget())
        {
          wrath_context->add_rect(d->m_rect0,
                                  WRATHWidgetGenerator::Rect(vec2(rects_dim[0])));
        }
      else if (rects_dim[0]!=d->m_rect_dims[0])
        {
          WRATHWidgetGenerator::Rect(vec2(rects_dim[0]))(d->m_rect0.widget());
        }

      wrath_context->update_generic(d->m_rect0);
      d->m_rect0.widget()->position(vec2(rects[0].x(), rects[0].y()));
      d->m_rect0.widget()->color(color_as_float);
      d->m_rect0.widget()->visible(color.alpha()!=0);
      d->m_rect_dims[0]=rects_dim[0];
      
      if(!d->m_rect1.widget())
        {
          wrath_context->add_rect(d->m_rect1,
                                  WRATHWidgetGenerator::Rect(vec2(rects_dim[1])));
        }
      else if (rects_dim[1]!=d->m_rect_dims[1])
        {
          WRATHWidgetGenerator::Rect(vec2(rects_dim[1]));
        }

      wrath_context->update_generic(d->m_rect1);
      d->m_rect1.widget()->position(vec2(rects[1].x(), rects[1].y()));
      d->m_rect1.widget()->color(color_as_float);
      d->m_rect1.widget()->visible(color.alpha()!=0); 
      d->m_rect_dims[1]=rects_dim[1];         

    }
  else
    {
      bool antialias(false);

      int adjacent1BigThird = ((adjacentWidth1 > 0) ? adjacentWidth1 + 1 : adjacentWidth1 - 1) / 3;
      int adjacent2BigThird = ((adjacentWidth2 > 0) ? adjacentWidth2 + 1 : adjacentWidth2 - 1) / 3;

      d->m_s0.visible(true);
      d->m_s1.visible(true);

      switch (side) {
      case WebCore::BSTop:
        renderObject->readyWRATHWidgetDrawLineForBoxSide(d->m_s0, wrath_context,
                                                         x1 +  WebCore::max((-adjacentWidth1 * 2 + 1) / 3, 0),
                                                         y1, x2 -  WebCore::max((-adjacentWidth2 * 2 + 1) / 3, 0), 
                                                         y1 + third,
                                                         side, color, WebCore::SOLID, 
                                                         adjacent1BigThird, adjacent2BigThird, antialias);

        renderObject->readyWRATHWidgetDrawLineForBoxSide(d->m_s1, wrath_context,
                                                         x1 + WebCore::max((adjacentWidth1 * 2 + 1) / 3, 0),
                                                         y2 - third, 
                                                         x2 - WebCore::max((adjacentWidth2 * 2 + 1) / 3, 0), y2,
                                                         side, color, WebCore::SOLID, 
                                                         adjacent1BigThird, adjacent2BigThird, antialias);
        break;

      case WebCore::BSLeft:
        renderObject->readyWRATHWidgetDrawLineForBoxSide(d->m_s0, wrath_context, 
                                                         x1, y1 + WebCore::max((-adjacentWidth1 * 2 + 1) / 3, 0),
                                                         x1 + third, 
                                                         y2 - WebCore::max((-adjacentWidth2 * 2 + 1) / 3, 0),
                                                         side, color, WebCore::SOLID, 
                                                         adjacent1BigThird, adjacent2BigThird, antialias);

        renderObject->readyWRATHWidgetDrawLineForBoxSide(d->m_s1, wrath_context, 
                                                         x2 - third,
                                                         y1 + WebCore::max((adjacentWidth1 * 2 + 1) / 3, 0),
                                                         x2, y2 - WebCore::max((adjacentWidth2 * 2 + 1) / 3, 0),
                                                         side, color, WebCore::SOLID, 
                                                         adjacent1BigThird, adjacent2BigThird, antialias);
        break;

      case WebCore::BSBottom:
        renderObject->readyWRATHWidgetDrawLineForBoxSide(d->m_s0, wrath_context, 
                                                         x1 + WebCore::max((adjacentWidth1 * 2 + 1) / 3, 0),
                                                         y1, 
                                                         x2 - WebCore::max((adjacentWidth2 * 2 + 1) / 3, 0), 
                                                         y1 + third,
                                                         side, color, WebCore::SOLID, 
                                                         adjacent1BigThird, adjacent2BigThird, antialias);

        renderObject->readyWRATHWidgetDrawLineForBoxSide(d->m_s1, wrath_context, 
                                                         x1 + WebCore::max((-adjacentWidth1 * 2 + 1) / 3, 0),
                                                         y2 - third, 
                                                         x2 - WebCore::max((-adjacentWidth2 * 2 + 1) / 3, 0), y2,
                                                         side, color, WebCore::SOLID, 
                                                         adjacent1BigThird, adjacent2BigThird, antialias);
        break;

      case WebCore::BSRight:
        renderObject->readyWRATHWidgetDrawLineForBoxSide(d->m_s0, wrath_context, 
                                                         x1, y1 + WebCore::max((adjacentWidth1 * 2 + 1) / 3, 0),
                                                         x1 + third, 
                                                         y2 - WebCore::max((adjacentWidth2 * 2 + 1) / 3, 0),
                                                         side, color, WebCore::SOLID, 
                                                         adjacent1BigThird, adjacent2BigThird, antialias);

        renderObject->readyWRATHWidgetDrawLineForBoxSide(d->m_s1, wrath_context,
                                                         x2 - third, 
                                                         y1 + WebCore::max((-adjacentWidth1 * 2 + 1) / 3, 0),
                                                         x2, y2 - WebCore::max((-adjacentWidth2 * 2 + 1) / 3, 0),
                                                         side, color, WebCore::SOLID, 
                                                         adjacent1BigThird, adjacent2BigThird, antialias);
        break;
      default:
        break;
      }
    }

}

class RenderObject_WRATHWidgetDrawLineForBoxSideRidgeOrGroove:
    public WebCore::PaintedWidgetsOfWRATH<RenderObject_WRATHWidgetDrawLineForBoxSideRidgeOrGroove>
{
public:

  static void ready(WebCore::RenderObject*,
                    WebCore::PaintedWidgetsOfWRATHHandle&,
                    WebCore::ContextOfWRATH*,
                    WebCore::BoxSide side,
                    WebCore::Color color, WebCore::EBorderStyle style,
                    int adjacentWidth1, int adjacentWidth2,
                    int x1, int y1, int x2, int y2);
  
  WebCore::PaintedWidgetsOfWRATHHandle m_s1, m_s2;
};

void
RenderObject_WRATHWidgetDrawLineForBoxSideRidgeOrGroove::ready(WebCore::RenderObject *renderObject,
                                                               WebCore::PaintedWidgetsOfWRATHHandle &h,
                                                               WebCore::ContextOfWRATH *wrath_context,
                                                               WebCore::BoxSide side,
                                                               WebCore::Color color, WebCore::EBorderStyle style,
                                                               int adjacentWidth1, int adjacentWidth2,
                                                               int x1, int y1, int x2, int y2)
{ //begin ready()
  RenderObject_WRATHWidgetDrawLineForBoxSideRidgeOrGroove *d;
  d=RenderObject_WRATHWidgetDrawLineForBoxSideRidgeOrGroove::object(renderObject, h);

  WebCore::ContextOfWRATH::AutoPushNode autoPushRoot(wrath_context, d->m_root_node);

  WebCore::EBorderStyle s1;
  WebCore::EBorderStyle s2;
  if (style == WebCore::GROOVE) {
    s1 = WebCore::INSET;
    s2 = WebCore::OUTSET;
  } else {
    s1 = WebCore::OUTSET;
    s2 = WebCore::INSET;
  }

  bool antialias(false);
  
  int adjacent1BigHalf = ((adjacentWidth1 > 0) ? adjacentWidth1 + 1 : adjacentWidth1 - 1) / 2;
  int adjacent2BigHalf = ((adjacentWidth2 > 0) ? adjacentWidth2 + 1 : adjacentWidth2 - 1) / 2;
  
  switch (side) {
  case WebCore::BSTop:
    renderObject->readyWRATHWidgetDrawLineForBoxSide(d->m_s1, wrath_context,
                                                     x1 + WebCore::max(-adjacentWidth1, 0) / 2, y1, 
                                                     x2 - WebCore::max(-adjacentWidth2, 0) / 2, (y1 + y2 + 1) / 2,
                                                     side, color, s1, adjacent1BigHalf, adjacent2BigHalf, 
                                                     antialias);

    renderObject->readyWRATHWidgetDrawLineForBoxSide(d->m_s2, wrath_context, 
                                                     x1 + WebCore::max(adjacentWidth1 + 1, 0) / 2, (y1 + y2 + 1) / 2, 
                                                     x2 - WebCore::max(adjacentWidth2 + 1, 0) / 2, y2,
                                                     side, color, s2, adjacentWidth1 / 2, adjacentWidth2 / 2, 
                                                     antialias);
    break;
  case WebCore::BSLeft:
    renderObject->readyWRATHWidgetDrawLineForBoxSide(d->m_s1, wrath_context, 
                                                     x1, y1 + WebCore::max(-adjacentWidth1, 0) / 2, 
                                                     (x1 + x2 + 1) / 2, y2 - WebCore::max(-adjacentWidth2, 0) / 2,
                                                     side, color, s1, adjacent1BigHalf, adjacent2BigHalf, 
                                                     antialias);
    renderObject->readyWRATHWidgetDrawLineForBoxSide(d->m_s2, wrath_context,  
                                                     (x1 + x2 + 1) / 2, y1 + WebCore::max(adjacentWidth1 + 1, 0) / 2, 
                                                     x2, y2 - WebCore::max(adjacentWidth2 + 1, 0) / 2,
                                                     side, color, s2, adjacentWidth1 / 2, adjacentWidth2 / 2, 
                                                     antialias);
    break;
  case WebCore::BSBottom:
    renderObject->readyWRATHWidgetDrawLineForBoxSide(d->m_s1, wrath_context, 
                                                     x1 + WebCore::max(adjacentWidth1, 0) / 2, y1, 
                                                     x2 - WebCore::max(adjacentWidth2, 0) / 2, (y1 + y2 + 1) / 2,
                                                     side, color, s2, adjacent1BigHalf, adjacent2BigHalf, 
                                                     antialias);
    renderObject->readyWRATHWidgetDrawLineForBoxSide(d->m_s2, wrath_context,  
                                                     x1 + WebCore::max(-adjacentWidth1 + 1, 0) / 2, (y1 + y2 + 1) / 2, 
                                                     x2 - WebCore::max(-adjacentWidth2 + 1, 0) / 2, y2,
                                                     side, color, s1, adjacentWidth1 / 2, adjacentWidth2 / 2, 
                                                     antialias);
    break;
  case WebCore::BSRight:
    renderObject->readyWRATHWidgetDrawLineForBoxSide(d->m_s1, wrath_context, 
                                                     x1, y1 + WebCore::max(adjacentWidth1, 0) / 2, 
                                                     (x1 + x2 + 1) / 2, y2 - WebCore::max(adjacentWidth2, 0) / 2,
                                                     side, color, s2, adjacent1BigHalf, adjacent2BigHalf, 
                                                     antialias);
    renderObject->readyWRATHWidgetDrawLineForBoxSide(d->m_s2, wrath_context,  
                                                     (x1 + x2 + 1) / 2, y1 + WebCore::max(-adjacentWidth1 + 1, 0) / 2, 
                                                     x2, y2 - WebCore::max(-adjacentWidth2 + 1, 0) / 2,
                                                     side, color, s1, adjacentWidth1 / 2, adjacentWidth2 / 2, 
                                                     antialias);
    break;
  }
  
}

class RenderObject_WRATHWidgetDrawLineForBoxSideInsetOrOutsetOrSolid:
    public WebCore::PaintedWidgetsOfWRATH<RenderObject_WRATHWidgetDrawLineForBoxSideInsetOrOutsetOrSolid>
{
public:
 
  static
  void ready(WebCore::RenderObject*,
             WebCore::PaintedWidgetsOfWRATHHandle&,
             WebCore::ContextOfWRATH*,
             WebCore::BoxSide side,
             WebCore::Color color, 
             WebCore::EBorderStyle style,
             int adjacentWidth1, int adjacentWidth2,
             int x1, int y1, int x2, int y2);

  WebCore::ContextOfWRATH::ColorFamily::DrawnRect::AutoDelete m_rect_item;
  int m_rect_width, m_rect_height;

  WebCore::ContextOfWRATH::CColorFamily::DrawnShape::AutoDelete m_shape_item;
  vecN<ivec2, 3> m_shape_corners;
};

void RenderObject_WRATHWidgetDrawLineForBoxSideInsetOrOutsetOrSolid::ready(WebCore::RenderObject *renderObject,
                                                                           WebCore::PaintedWidgetsOfWRATHHandle &h,
                                                                           WebCore::ContextOfWRATH *wrath_context,
                                                                           WebCore::BoxSide side,
                                                                           WebCore::Color color, 
                                                                           WebCore::EBorderStyle style,
                                                                           int adjacentWidth1, int adjacentWidth2,
                                                                           int x1, int y1, int x2, int y2)
{ //begin ready()
  RenderObject_WRATHWidgetDrawLineForBoxSideInsetOrOutsetOrSolid *d;
  d=RenderObject_WRATHWidgetDrawLineForBoxSideInsetOrOutsetOrSolid::object(renderObject, h);

  WebCore::ContextOfWRATH::AutoPushNode autoPushRoot(wrath_context, d->m_root_node);
  
  if(d->m_rect_item.widget()) {
    d->m_rect_item.widget()->visible(false);
  }

  if(d->m_shape_item.widget()) {
    d->m_shape_item.widget()->visible(false);
  }


  if (!adjacentWidth1 && !adjacentWidth2) {
    int w, h;
    vec4 c;

    color.getRGBA(c.x(), c.y(), c.z(), c.w());
    w=x2 - x1;
    h=y2 - y1;

    if(!d->m_rect_item.widget() 
       or w!=d->m_rect_width
       or h!=d->m_rect_height) {

      if(!d->m_rect_item.widget())
        {
          wrath_context->add_rect(d->m_rect_item,
                                  WRATHWidgetGenerator::Rect(w,h));
        }
      else
        {
          WRATHWidgetGenerator::Rect(w, h)(d->m_rect_item.widget());
          wrath_context->update_generic(d->m_rect_item);
        }
      
      d->m_rect_width=w;
      d->m_rect_height=h;
    }
    else {
      wrath_context->update_generic(d->m_rect_item);
    }

    d->m_rect_item.widget()->visible(color.alpha()!=0);
    d->m_rect_item.widget()->position( vec2(x1, y1));
    d->m_rect_item.widget()->color(c);
  } else {

    vecN<ivec2, 4> quad;
    switch (side) {
    case WebCore::BSTop:
      quad[0] = ivec2(x1 + WebCore::max(-adjacentWidth1, 0), y1);
      quad[1] = ivec2(x1 + WebCore::max(adjacentWidth1, 0), y2);
      quad[2] = ivec2(x2 - WebCore::max(adjacentWidth2, 0), y2);
      quad[3] = ivec2(x2 - WebCore::max(-adjacentWidth2, 0), y1);
      break;
    case WebCore::BSBottom:
      quad[0] = ivec2(x1 + WebCore::max(adjacentWidth1, 0), y1);
      quad[1] = ivec2(x1 + WebCore::max(-adjacentWidth1, 0), y2);
      quad[2] = ivec2(x2 - WebCore::max(-adjacentWidth2, 0), y2);
      quad[3] = ivec2(x2 - WebCore::max(adjacentWidth2, 0), y1);
      break;
    case WebCore::BSLeft:
      quad[0] = ivec2(x1, y1 + WebCore::max(-adjacentWidth1, 0));
      quad[1] = ivec2(x1, y2 - WebCore::max(-adjacentWidth2, 0));
      quad[2] = ivec2(x2, y2 - WebCore::max(adjacentWidth2, 0));
      quad[3] = ivec2(x2, y1 + WebCore::max(adjacentWidth1, 0));
      break;
    case WebCore::BSRight:
      quad[0] = ivec2(x1, y1 + WebCore::max(adjacentWidth1, 0));
      quad[1] = ivec2(x1, y2 - WebCore::max(adjacentWidth2, 0));
      quad[2] = ivec2(x2, y2 - WebCore::max(-adjacentWidth2, 0));
      quad[3] = ivec2(x2, y1 + WebCore::max(-adjacentWidth1, 0));
      break;
    }

    /*
      we only care about the _shape_, that is the delta's
      between each point:
     */
    vecN<ivec2, 3> point_deltas;

    for(int i=0;i<3;++i) {
      point_deltas[i]=quad[i+1]-quad[0];
    }

    /*
      [WRATH-TODO]: if the shape is transparent, we need
      to obey the transparency value.
    */
    vec4 c(0.0f, 0.0f, 0.0f, 1.0f);
    color.getRGBA(c.x(), c.y(), c.z(), c.w());

    if(!d->m_shape_item.widget() or d->m_shape_corners!=point_deltas) {
      WRATHShapeF shape;

      shape.current_outline() << WRATHOutline<float>::position_type(0, 0)
                              << WRATHOutline<float>::position_type(point_deltas[0].x(), point_deltas[0].y())
                              << WRATHOutline<float>::position_type(point_deltas[1].x(), point_deltas[1].y())
                              << WRATHOutline<float>::position_type(point_deltas[2].x(), point_deltas[2].y());

      if(!d->m_shape_item.widget()) {
        wrath_context->add_filled_shape(d->m_shape_item,
                                        WRATHWidgetGenerator::ColorProperties(c),
                                        WRATHWidgetGenerator::shape_value(shape));
      } else {
        d->m_shape_item.widget()->properties()->change_shape(WRATHWidgetGenerator::shape_value(shape));
      }

      d->m_shape_corners=point_deltas;
    }

    d->m_shape_item.widget()->position( vec2(quad[0].x(), quad[0].y()));
    d->m_shape_item.widget()->node()->color(c);
    d->m_shape_item.widget()->visible(color.alpha()!=0);
    wrath_context->update_generic(d->m_shape_item);
  }
}
} //of anonymous namespace

namespace WebCore {

void RenderObject::readyWRATHWidgetFocusRing(PaintedWidgetsOfWRATHHandle&, ContextOfWRATH*, 
					     int tx, int ty, RenderStyle*)
{
  /*
    TODO: paintFocusRing()
   */
}

void RenderObject::readyWRATHWidgetOutline(PaintedWidgetsOfWRATHHandle& handle,
					   ContextOfWRATH *wrath_context,
					   int tx, int ty, int w, int h)
{
    RenderObject_WRATHWidgetsOutline* d = RenderObject_WRATHWidgetsOutline::object(this, handle);

    ContextOfWRATH::AutoPushNode autoPushRoot(wrath_context, d->m_root_node);
    ContextOfWRATH::AutoPushNode autoPushSkip(wrath_context, d->m_skip_node);

    d->m_skip_node.widget()->visible(true);
    if (!hasOutline()) {
        d->m_skip_node.widget()->visible(false);
        return;
    }

    RenderStyle* styleToUse = style();
    int outlineWidth = styleToUse->outlineWidth();
    EBorderStyle outlineStyle = styleToUse->outlineStyle();

    Color outlineColor = styleToUse->visitedDependentColor(CSSPropertyOutlineColor);

    int offset = styleToUse->outlineOffset();

    d->m_focus_ring.visible(false);
    if (styleToUse->outlineStyleIsAuto() || hasOutlineAnnotation()) {
        if (!theme()->supportsFocusRing(styleToUse)) {
          // Only paint the focus ring by hand if the theme isn't able to draw the focus ring.
          d->m_focus_ring.visible(true);
          readyWRATHWidgetFocusRing(d->m_focus_ring, wrath_context, tx, ty, styleToUse);
        }
    }

    
    ContextOfWRATH::AutoPushNode autoPushSides(wrath_context, d->m_box_sides_node);
    d->m_box_sides_node.widget()->visible(false);

    if (styleToUse->outlineStyleIsAuto() || styleToUse->outlineStyle() == BNONE)
        return;

    tx -= offset;
    ty -= offset;
    w += 2 * offset;
    h += 2 * offset;

    if (h < 0 || w < 0)
      return;

    int leftOuter = tx - outlineWidth;
    int leftInner = tx;
    int rightOuter = tx + w + outlineWidth;
    int rightInner = tx + w;
    int topOuter = ty - outlineWidth;
    int topInner = ty;
    int bottomOuter = ty + h + outlineWidth;
    int bottomInner = ty + h;

    d->m_box_sides_node.widget()->visible(true);
    readyWRATHWidgetDrawLineForBoxSide(d->m_sides[0], wrath_context, leftOuter, topOuter, 
                                       leftInner, bottomOuter, BSLeft,
                                       outlineColor, outlineStyle, outlineWidth, outlineWidth);

    readyWRATHWidgetDrawLineForBoxSide(d->m_sides[1], wrath_context, leftOuter, topOuter, 
                                       rightOuter, topInner, BSTop,
                                       outlineColor, outlineStyle, outlineWidth, outlineWidth);

    readyWRATHWidgetDrawLineForBoxSide(d->m_sides[2], wrath_context, rightInner, topOuter, 
                                       rightOuter, bottomOuter, BSRight,
                                       outlineColor, outlineStyle, outlineWidth, outlineWidth);

    readyWRATHWidgetDrawLineForBoxSide(d->m_sides[3], wrath_context, leftOuter, bottomInner, 
                                       rightOuter, bottomOuter, BSBottom, 
                                       outlineColor, outlineStyle, outlineWidth, outlineWidth);
    
}

void RenderObject::readyWRATHWidgetDrawLineForBoxSide(PaintedWidgetsOfWRATHHandle& handle,
						      ContextOfWRATH *wrath_context,
						      int x1, int y1, int x2, int y2, BoxSide side,
						      Color color, EBorderStyle style,
						      int adjacentWidth1, int adjacentWidth2, 
                                                      bool antialias)
{
  RenderObject_WRATHWidgetsDrawLineForBoxSide* d;
  d=RenderObject_WRATHWidgetsDrawLineForBoxSide::object(this, handle);

  ContextOfWRATH::AutoPushNode autoPushRoot(wrath_context, d->m_root_node);
  ContextOfWRATH::AutoPushNode autoPushSkip(wrath_context, d->m_skip_node);
  
  int width = (side == BSTop || side == BSBottom ? y2 - y1 : x2 - x1);
  if (style == DOUBLE && width < 3)
    style = SOLID;

  d->m_skip_node.widget()->visible(true);
  d->m_dotted_or_dashed.visible(false);
  d->m_double.visible(false);
  d->m_ridge_or_groove.visible(false);
  d->m_inset_outset_or_solid.visible(false);


  switch(style)
    {
    case BNONE:
    case BHIDDEN:
      d->m_skip_node.widget()->visible(false);
      return;

    case DOTTED:
    case DASHED:
      d->m_dotted_or_dashed.visible(true);
      RenderObject_WRATHWidgetDrawLineForBoxSideDottedOrDashed::ready(this,
                                                                      d->m_dotted_or_dashed,
                                                                      wrath_context, side,
                                                                      color, style, width,
                                                                      x1, y1, x2, y2);
      break;

    case DOUBLE:
      d->m_double.visible(true);
      RenderObject_WRATHWidgetDrawLineForBoxSideDouble::ready(this, d->m_double,
                                                              wrath_context, side, 
                                                              color, width, 
                                                              adjacentWidth1, adjacentWidth2,
                                                              x1, y1, x2, y2);
      break;
                                               
    case RIDGE:
    case GROOVE:
      d->m_ridge_or_groove.visible(true);
      RenderObject_WRATHWidgetDrawLineForBoxSideRidgeOrGroove::ready(this, d->m_ridge_or_groove,
                                                                     wrath_context, side, color, style,
                                                                     adjacentWidth1, adjacentWidth2,
                                                                     x1, y1, x2, y2);
      break;

    case INSET:
      // FIXME: Maybe we should lighten the colors on one side like Firefox.
      // https://bugs.webkit.org/show_bug.cgi?id=58608
      if (side == BSTop || side == BSLeft)
        color = color.dark();
      // fall through
    case OUTSET:
      if (style == OUTSET && (side == BSBottom || side == BSRight))
        color = color.dark();
      // fall through
    case SOLID:
      d->m_inset_outset_or_solid.visible(true);
      RenderObject_WRATHWidgetDrawLineForBoxSideInsetOrOutsetOrSolid::ready(this,
                                                                            d->m_inset_outset_or_solid,
                                                                            wrath_context, side, color, style,
                                                                            adjacentWidth1, adjacentWidth2,
                                                                            x1, y1, x2, y2);
    }

}

}
#endif
