#include "config.h"

#include "IntPoint.h"
#include "WRATHPaintHelpers.h"

#if USE(WRATH)

#include <map>

namespace 
{
  

  WRATHGLStateChange::state_change::handle
  getBlenderOpForImage(WebCore::CompositeOperator op, bool hasInterestingAlpha)
  {
    if(op==WebCore::CompositeSourceOver && !hasInterestingAlpha) 
      {
        op=WebCore::CompositeCopy;
      }
    return WebCore::ContextOfWRATH::getBlenderFromCompositeOp(op);
  }
 
  void
  rescale_if_needed(vec2 &c,
                    WRATHImage *image,
                    WebCore::NativeImageWRATH *webcore_image)
  {
    /*
      we may have rescaled the image to fit it in a wrath image,
      thus we may need to rescale bl and sz:
    */
    if(webcore_image->origSize().width()!=image->size().x())
      {
        float sc;
        sc=static_cast<float>(image->size().x())/static_cast<float>(webcore_image->origSize().width());
        c.x()*=sc;
      }
    if(webcore_image->origSize().height()!=image->size().y())
      {
        float sc;
        sc=static_cast<float>(image->size().y())/static_cast<float>(webcore_image->origSize().height());
        c.y()*=sc;
      }
  }


  template<typename F, typename FS>
  void
  rect_update(WebCore::ContextOfWRATH *ctx,
              const F& newrect,
              const WebCore::Color& newcolor,
              WebCore::CompositeOperator op,
              FS &m_rect_size,
              WebCore::CompositeOperator &m_op,
              WebCore::ContextOfWRATH::ColorFamily::DrawnRect::AutoDelete &m_item,
              WebCore::ContextOfWRATH::CanvasNodeForTransparentSingleton &m_node)
  {
    

    bool remake_widget;
    
    if(newcolor.alpha()==255 && op==WebCore::CompositeSourceOver)
      {
        op=WebCore::CompositeCopy;
      }

    if(newcolor.alpha()==0 && op==WebCore::CompositeSourceOver)
      {
        if(m_item.widget()) {
          m_item.widget()->visible(false);
        }
        return;
      }

    remake_widget=!m_item.widget()
      or m_op!=op;
    
    if(remake_widget or m_rect_size!=newrect.size()) 
      {
        m_rect_size=newrect.size();             
        if(remake_widget) 
          {
            m_item.delete_widget();
            
            WRATHGLStateChange::state_change::handle blender;
            WRATHDrawType pass;
            WRATHWidgetGenerator::Brush brush;

            blender=getBlenderOpForImage(op, newcolor.alpha()!=0 and newcolor.alpha()!=255);
            if(blender.valid())
              {
                brush.m_draw_state.add_gl_state_change(blender);
                pass=WRATHDrawType(WebCore::TransparentImagePassEnumerationOfWRATH, 
                                   WRATHDrawType::transparent_draw);
              }
            else
              {
                pass=WRATHDrawType(0, WRATHDrawType::opaque_draw);
              }

	    if(pass.m_type==WRATHDrawType::transparent_draw)
	      {
		WRATH_PUSH_CANVAS_NODE_TRANSPARENT(ctx, m_node);
	      }
	    else
	      {
		m_node.delete_widget();
	      }
            
	    ctx->add_rect(m_item,
                          WRATHWidgetGenerator::Rect(newrect.width(), newrect.height()),
                          brush, pass);
            m_op=op;
          }
        else
          {
            WRATHWidgetGenerator::Rect(newrect.width(), newrect.height())(m_item.widget());
          }
      }
    
    if(!remake_widget)
      {
	if(m_node.active())
	  {
	    WRATH_PUSH_CANVAS_NODE_TRANSPARENT(ctx, m_node);
	  }
        ctx->update_generic(m_item);
      }

    if(m_node.active())
      {
	WRATH_POP_CANVAS_NODE_TRANSPARENT(ctx, m_node);
        ctx->m_number_transparent_rects++;
      }

    vec4 c;     
    newcolor.getRGBA(c.x(), c.y(), c.z(), c.w());    
    m_item.widget()->position(vec2(newrect.x(), newrect.y()));
    m_item.widget()->visible(true);
    m_item.widget()->color(c);
  }

  class WRATH_tiledPassthrough : 
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::Image, WRATH_tiledPassthrough>
  {
  public:
    WebCore::ImageRectOfWRATH m_data;
  };
}







namespace WebCore {

//////////////////////////////////////////////////////
// FilledIntRectOfWRATH methods
FilledIntRectOfWRATH::
FilledIntRectOfWRATH(void)
{}




void 
FilledIntRectOfWRATH::
AddToShape_TranslateToOrigin(bool translate,
                             WRATHShapeF & shape,
                             const IntRect & rect)
{
  FilledFloatRectOfWRATH::AddToShape_TranslateToOrigin(translate, shape, FloatRect(rect) );
}

void
FilledIntRectOfWRATH::
update(ContextOfWRATH *ctx,
       const IntRect& newrect,
       const Color& newcolor,
       CompositeOperator op)
{
  rect_update<IntRect, IntSize>(ctx, newrect, newcolor, op, m_rect_size, m_op, 
                                m_item, m_node);
}

void
FilledIntRectOfWRATH::
update(ContextOfWRATH *ctx,
       const IntRect& newrect)
{
  //[WRATH-DANGER]: we do clear as setting all colors as (0,0,0,0)
  //as what a GraphicsContext backed by a QPainter does,
  //but it might be best to set the thing as clip outside rect
  update(ctx, newrect, Color(0,0,0,0), CompositeCopy);
}



//////////////////////////////////////////////////////
// FilledFloatRectOfWRATH methods
FilledFloatRectOfWRATH::
FilledFloatRectOfWRATH(void)
{}

void 
FilledFloatRectOfWRATH::
AddToShape_TranslateToOrigin(bool translate,
                             WRATHShapeF & shape,
                             const FloatRect & rect)
{
  //skip adding if it is degenerate.
    if(rect.width() <= 0.0f or rect.height() <=0.0f) {
      return;
    }

    vec2 p0, p1;

    if(translate) {
      p0=vec2(0.0f, 0.0f);
      p1=vec2(rect.width(), rect.height());
    } else {
      p0=vec2(rect.x(), rect.y());
      p1=vec2(rect.maxX(), rect.maxY());
    }
    
    shape
      .move_to(vec2(p0.x(), p0.y()))
      .line_to(vec2(p1.x(), p0.y()))
      .line_to(vec2(p1.x(), p1.y()))
      .line_to(vec2(p0.x(), p1.y()));
    shape.new_outline();
}


void
FilledFloatRectOfWRATH::
update(ContextOfWRATH *ctx,
       const FloatRect& newrect,
       const Color& newcolor,
       CompositeOperator op)
{
  rect_update<FloatRect, FloatSize>(ctx, newrect, newcolor, op, m_rect_size,  
                                    m_op, m_item, m_node);
}

void
FilledFloatRectOfWRATH::
update(ContextOfWRATH *ctx,
       const FloatRect& newrect)
{
  //[WRATH-DANGER]: we do clear as setting all colors as (0,0,0,0)
  //as what a GraphicsContext backed by a QPainter does,
  //but it might be best to set the thing as clip outside rect
  update(ctx, newrect, Color(0,0,0,0), CompositeCopy);
}

//////////////////////////////////////////
// ImageRectOfWRATH methods
ImageRectOfWRATH::
ImageRectOfWRATH(void):
  m_image(0)
{}

ImageRectOfWRATH::
~ImageRectOfWRATH()
{
  m_image_ctor_connection.disconnect();
}

void
ImageRectOfWRATH::
clear(void)
{
  m_image_ctor_connection.disconnect();
  m_image=0;
  m_query=NativeImageWRATH::NonTrivialAlphaQuery();

  RectType::Widget *p;
  p=m_item.release_widget();

  if(p) 
    {
      WRATHPhasedDelete(p);
    }
}


void
ImageRectOfWRATH::
update_common(ContextOfWRATH *ctx,
              NativeImageWRATH *webcore_image,
              FloatRect dest, 
              IntRect src,
              FloatRect corners,
              CompositeOperator op)
{
  bool remake_widget, hasInterestingAlpha;
  WRATHImage *image(webcore_image->getWrathImage());
             
  if(m_image!=image) 
    {
      clear();
      if(image)
        {
          m_image_ctor_connection=image->connect_dtor(boost::bind(&ImageRectOfWRATH::clear, this));
        }
    }

  hasInterestingAlpha=m_query.query(src, webcore_image);
  remake_widget=!m_item.widget()
    or m_image!=image
    or m_op!=op
    or m_hasInterestingAlpha!=hasInterestingAlpha;

  

  FloatPoint pos = dest.location();
  if(remake_widget or m_dest_size!=dest.size() or m_corners!=corners)
    {      
      WRATHDefaultRectAttributePacker::Rect::handle rect;

      m_image=image;
      m_op=op;
      m_hasInterestingAlpha=hasInterestingAlpha;
      m_dest_size=dest.size();
      m_corners=corners;
      rect=WRATHNew WRATHDefaultRectAttributePacker::Rect(m_dest_size.width(), 
							  m_dest_size.height());

      /*
        m_corners gives the texel coordinates to use
        for each corner. The pixel used is given by:

        m_brush_stretch*p + m_brush_offset

        where p is in item coordinates. 
        p is (0,0) at the top left and
        m_dest_size at the bottom right.
       */
      rect->m_brush_stretch=vec2(m_corners.width(), m_corners.height())/rect->m_width_height;
      rect->m_brush_offset=vec2(m_corners.x(), m_corners.y());
      rescale_if_needed(rect->m_brush_stretch, image, webcore_image);
      rescale_if_needed(rect->m_brush_offset, image, webcore_image);

      if(remake_widget)
        {
          WRATHDrawType pass;
          WRATHWidgetGenerator::Brush brush(m_image);
          WRATHGLStateChange::state_change::handle blender;

          blender=getBlenderOpForImage(m_op, hasInterestingAlpha);
          if(blender.valid())
            {
              pass=WRATHDrawType(TransparentImagePassEnumerationOfWRATH, 
                                 WRATHDrawType::transparent_draw);
              brush.premultiply_alpha(true);
              brush.m_draw_state.add_gl_state_change(blender);
            }
          else
            {
              pass=WRATHDrawType(0, WRATHDrawType::opaque_draw);
              brush.image_alpha_test(true);
            }

	  if(pass.m_type==WRATHDrawType::transparent_draw)
	    {
	      WRATH_PUSH_CANVAS_NODE_TRANSPARENT(ctx, m_node);
	    }
	  else
	    {
	      m_node.delete_widget();
	    }
          
          m_item.delete_widget();
          ctx->add_rect(m_item,
                        WRATHWidgetGenerator::Rect(rect),
                        brush, pass);
        } //remake widget
      else 
        {
          WRATHWidgetGenerator::Rect(rect)(m_item.widget());
        }
      
    } //of if(remake_widget or.. )

  if(!remake_widget)
    {
      if(m_node.active())
	{
	  WRATH_PUSH_CANVAS_NODE_TRANSPARENT(ctx, m_node);
	}
      ctx->update_generic(m_item);
    }

  if(m_node.active())
    {
      WRATH_POP_CANVAS_NODE_TRANSPARENT(ctx, m_node);
      ctx->m_number_transparent_images++;
    }
  
  m_item.widget()->position(vec2(pos.x(), pos.y()));

  vec2 bl(src.x(), src.y()), sz(src.width(), src.height());  
  rescale_if_needed(bl, m_image, webcore_image); 
  rescale_if_needed(sz, m_image, webcore_image);

  m_item.widget()->sub_image(ivec2(bl), ivec2(sz), true, true);
}

void
ImageRectOfWRATH::update(ContextOfWRATH *ctx,
                         NativeImageWRATH *webcore_image,
                         FloatRect dest, 
                         IntRect src,
                         CompositeOperator op)
{
  
  update_common(ctx, webcore_image, dest, src, 
                FloatRect(0, 0, src.width(), src.height()), 
                op);
  //m_item.widget()->set(WRATHTextureCoordinate::clamp, WRATHTextureCoordinate::clamp);
}


void
ImageRectOfWRATH::
update_tiled(ContextOfWRATH *ctx,
             NativeImageWRATH *webcore_image,
             FloatRect dest, 
             IntRect src,
             CompositeOperator op,
             FloatRect relativeTexelCoords)
{
  
  update_common(ctx, webcore_image, dest, src, relativeTexelCoords, op);
  //m_item.widget()->set(WRATHTextureCoordinate::repeat, WRATHTextureCoordinate::repeat);
}


void 
ImageRectOfWRATH::
update_through_handle(ContextOfWRATH *ctx,
                      PaintedWidgetsOfWRATHHandleT<Image>& handle,
                      Image* image,
                      NativeImageWRATH* newImage,
                      const FloatRect &dest, 
                      const IntRect &src,
                      CompositeOperator op)
{
    WRATH_tiledPassthrough *d(WRATH_tiledPassthrough::object(image, handle));

    ContextOfWRATH::AutoPushNode autoPushRoot(ctx, d->m_root_node);
    d->m_data.update(ctx, newImage, dest, src, op);
}


void 
ImageRectOfWRATH::
update_tiled_through_handle(ContextOfWRATH *ctx,
                            PaintedWidgetsOfWRATHHandleT<Image>& handle,
                            Image* image,
                            NativeImageWRATH* newImage,
                            const FloatRect &dest, 
                            const IntRect &src,
                            CompositeOperator op,
                            const FloatRect &relativeTexelCoords)
{
    WRATH_tiledPassthrough *d(WRATH_tiledPassthrough::object(image, handle));

    ContextOfWRATH::AutoPushNode autoPushRoot(ctx, d->m_root_node);
    d->m_data.update_tiled(ctx, newImage, dest, src, op, relativeTexelCoords);
  
}


//////////////////////////
// RoundedFilledRectOfWRATH methods
RoundedFilledRectOfWRATH::
RoundedFilledRectOfWRATH(void):
  m_rect(0, 0, 0, 0)
{}


void 
RoundedFilledRectOfWRATH::
AddToShape_TranslateToOrigin(bool translate_to_origin,
                             WRATHShapeF & shape,
                             const WebCore::FloatRect &in_rect,
                             const float radius)
{
    float twice_radius(2.0f*radius);

    if(in_rect.isEmpty()) {
      return;
    }

    if (in_rect.width() < twice_radius || in_rect.height() < twice_radius) {
        FilledFloatRectOfWRATH::AddToShape_TranslateToOrigin(translate_to_origin, shape, in_rect);
	return;
    }

    FloatRect rect(in_rect);
    if(translate_to_origin) {
      rect.move(FloatSize(-rect.x(), -rect.y()));
    }

    {
      FloatSize topLeftRadius(radius, radius);
      FloatSize topRightRadius(radius, radius);
      FloatSize bottomLeftRadius(radius, radius);
      FloatSize bottomRightRadius(radius, radius);
      static const float gCircleControlPoint = 0.448f;
    
      shape
        .move_to(vec2(rect.x() + topLeftRadius.width(), rect.y()))
        .line_to(vec2(rect.x() + rect.width() - topRightRadius.width(), rect.y()))
        .cubic_to(vec2(rect.x() + rect.width() - topRightRadius.width() * gCircleControlPoint, rect.y()),
                  vec2(rect.x() + rect.width(), rect.y() + topRightRadius.height() * gCircleControlPoint),
                  vec2(rect.x() + rect.width(), rect.y() + topRightRadius.height()))
        .line_to(vec2(rect.x() + rect.width(), rect.y() + rect.height() - bottomRightRadius.height()))
        .cubic_to(vec2(rect.x() + rect.width(), rect.y() + rect.height() - bottomRightRadius.height() * gCircleControlPoint),
                  vec2(rect.x() + rect.width() - bottomRightRadius.width() * gCircleControlPoint, rect.y() + rect.height()),
                  vec2(rect.x() + rect.width() - bottomRightRadius.width(), rect.y() + rect.height()))
        .line_to(vec2(rect.x() + bottomLeftRadius.width(), rect.y() + rect.height()))
        .cubic_to(vec2(rect.x() + bottomLeftRadius.width() * gCircleControlPoint, rect.y() + rect.height()),
                  vec2(rect.x(), rect.y() + rect.height() - bottomLeftRadius.height() * gCircleControlPoint),
                  vec2(rect.x(), rect.y() + rect.height() - bottomLeftRadius.height()))
        .line_to(vec2(rect.x(), rect.y() + topLeftRadius.height()))
        .cubic_to(vec2(rect.x(), rect.y() + topLeftRadius.height() * gCircleControlPoint),
                  vec2(rect.x() + topLeftRadius.width() * gCircleControlPoint, rect.y()),
                  vec2(rect.x() + topLeftRadius.width(), rect.y()));
    }

    shape.new_outline();
}

void
RoundedFilledRectOfWRATH::
AddToShape_TranslateToOrigin(bool translate_to_origin,
                             WRATHShapeF & shape,
                             const RoundedIntRect &rect)
{
   if(rect.rect().isEmpty()) {
      return;
    }

   if (rect.rect().width() < rect.radii().topLeft().width() + rect.radii().topRight().width()
       || rect.rect().width() < rect.radii().bottomLeft().width() + rect.radii().bottomRight().width()
       || rect.rect().height() < rect.radii().topLeft().height() + rect.radii().bottomLeft().height()
       || rect.rect().height() < rect.radii().topRight().height() + rect.radii().bottomRight().height()) {

        FilledIntRectOfWRATH::AddToShape_TranslateToOrigin(translate_to_origin, shape, rect.rect());
	return;
    }

    RoundedIntRect moved_rect(rect);

    if(translate_to_origin)
      {
        moved_rect.move(IntSize(-rect.rect().x(), -rect.rect().y()));
      }

    /*
      add the rounded rect via WRATHShape calls directly,
      the following code is essentially that which is found
      in platform/graphics/Path.cpp
     */
    {
      FloatRect rect(moved_rect.rect());
      FloatSize topLeftRadius(moved_rect.radii().topLeft());
      FloatSize topRightRadius(moved_rect.radii().topRight());
      FloatSize bottomLeftRadius(moved_rect.radii().bottomLeft());
      FloatSize bottomRightRadius(moved_rect.radii().bottomRight());
      static const float gCircleControlPoint = 0.448f;
    
      shape
        .move_to(vec2(rect.x() + topLeftRadius.width(), rect.y()))
        .line_to(vec2(rect.x() + rect.width() - topRightRadius.width(), rect.y()))
        .cubic_to(vec2(rect.x() + rect.width() - topRightRadius.width() * gCircleControlPoint, rect.y()),
                  vec2(rect.x() + rect.width(), rect.y() + topRightRadius.height() * gCircleControlPoint),
                  vec2(rect.x() + rect.width(), rect.y() + topRightRadius.height()))
        .line_to(vec2(rect.x() + rect.width(), rect.y() + rect.height() - bottomRightRadius.height()))
        .cubic_to(vec2(rect.x() + rect.width(), rect.y() + rect.height() - bottomRightRadius.height() * gCircleControlPoint),
                  vec2(rect.x() + rect.width() - bottomRightRadius.width() * gCircleControlPoint, rect.y() + rect.height()),
                  vec2(rect.x() + rect.width() - bottomRightRadius.width(), rect.y() + rect.height()))
        .line_to(vec2(rect.x() + bottomLeftRadius.width(), rect.y() + rect.height()))
        .cubic_to(vec2(rect.x() + bottomLeftRadius.width() * gCircleControlPoint, rect.y() + rect.height()),
                  vec2(rect.x(), rect.y() + rect.height() - bottomLeftRadius.height() * gCircleControlPoint),
                  vec2(rect.x(), rect.y() + rect.height() - bottomLeftRadius.height()))
        .line_to(vec2(rect.x(), rect.y() + topLeftRadius.height()))
        .cubic_to(vec2(rect.x(), rect.y() + topLeftRadius.height() * gCircleControlPoint),
                  vec2(rect.x() + topLeftRadius.width() * gCircleControlPoint, rect.y()),
                  vec2(rect.x() + topLeftRadius.width(), rect.y()));
    }

    shape.new_outline();
    
    
}


void
RoundedFilledRectOfWRATH::
update(ContextOfWRATH *ctx,
       const RoundedIntRect &rect,
       const Color &color,
       CompositeOperator op)
{
  vec4 c;

  /*
    [WRATH-TODO]: obey CompositeOperator op
   */

  color.getRGBA(c.x(), c.y(), c.z(), c.w());
  if(!m_item.widget() or m_rect.rect().size()!=rect.rect().size() or !(m_rect.radii()==rect.radii()) )
    {
      WRATHShapeF shape;

      AddToShape_TranslateToOrigin(true, shape, rect);
      m_rect=rect;

      if(!m_item.widget())
        {
          ctx->add_filled_shape(m_item,
                                WRATHWidgetGenerator::ColorProperties(c),
                                WRATHWidgetGenerator::shape_value(shape));
          m_op=op;
        }
      else
        {
          m_item.widget()->properties()->change_shape(WRATHWidgetGenerator::shape_value(shape));
        }
    }

  ctx->update_generic(m_item);
  m_item.widget()->node()->color(c);
  m_item.widget()->position( vec2(rect.rect().x(), rect.rect().y()) );

}

}






namespace {
    class WRATH_drawImage_oneHandle : 
        public WebCore::PaintedWidgetsOfWRATHT<WebCore::Image, WRATH_drawImage_oneHandle>
    {
    public:
        WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::Image> m_handle;
    };

    class WRATH_drawImage_twoHandles : 
        public WebCore::PaintedWidgetsOfWRATHT<WebCore::Image, WRATH_drawImage_twoHandles>
    {
    public:
        WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::Image> m_handleOne;
        WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::Image> m_handleTwo;
    };
}


void WebCore::WRATH_drawImage(WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::Image>& handle, WebCore::ContextOfWRATH *ctx,
                              WebCore::Image* image, WebCore::ColorSpace styleColorSpace,
                              const WebCore::FloatRect& dest, const WebCore::FloatRect& src,
                              CompositeOperator op, bool useLowQualityScale)
{
    WRATH_drawImage_oneHandle *d(WRATH_drawImage_oneHandle::object(image, handle));

    ContextOfWRATH::AutoPushNode autoPushRoot(ctx, d->m_root_node);

    d->m_handle.visible(false);

    if (!image)
        return;
        
    float tsw = src.width();
    float tsh = src.height();
    float tw = dest.width();
    float th = dest.height();
        
    if (tsw == -1)
        tsw = image->width();
    if (tsh == -1)
        tsh = image->height();
        
    if (tw == -1)
        tw = image->width();
    if (th == -1)
        th = image->height();
        
    /*
      if (useLowQualityScale) {
      InterpolationQuality previousInterpolationQuality = imageInterpolationQuality();
      // FIXME: Should be InterpolationLow
      setImageInterpolationQuality(InterpolationNone);
      image->draw(this, FloatRect(dest.location(), FloatSize(tw, th)), FloatRect(src.location(), FloatSize(tsw, tsh)), styleColorSpace, op);
      setImageInterpolationQuality(previousInterpolationQuality);
      } else
    */

    d->m_handle.visible(true);
    image->readyWRATHWidgets(d->m_handle, ctx, FloatRect(dest.location(), FloatSize(tw, th)), FloatRect(src.location(), FloatSize(tsw, tsh)), styleColorSpace, op);
}

void WebCore::WRATH_drawImage(WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::Image>& handle, WebCore::ContextOfWRATH *ctx,
                              WebCore::Image* image, WebCore::ColorSpace styleColorSpace, const WebCore::IntPoint& p, WebCore::CompositeOperator op)
{
    // Just passing the call along, same handle usage is safe
    WRATH_drawImage(handle, ctx, image, styleColorSpace, p, IntRect(0, 0, -1, -1), op);
}

void WebCore::WRATH_drawImage(WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::Image>& handle, WebCore::ContextOfWRATH *ctx,
                              WebCore::Image* image, WebCore::ColorSpace styleColorSpace,
                              const WebCore::IntRect& r, WebCore::CompositeOperator op, bool useLowQualityScale)
{
    // Just passing the call along, same handle usage is safe
    WRATH_drawImage(handle, ctx, image, styleColorSpace, r, IntRect(0, 0, -1, -1), op, useLowQualityScale);
}

void WebCore::WRATH_drawImage(WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::Image>& handle, WebCore::ContextOfWRATH *ctx,
                              WebCore::Image* image, WebCore::ColorSpace styleColorSpace,
                              const WebCore::IntPoint& dest, const WebCore::IntRect& srcRect, WebCore::CompositeOperator op)
{
    // Just passing the call along, same handle usage is safe
    WRATH_drawImage(handle, ctx, image, styleColorSpace, IntRect(dest, srcRect.size()), srcRect, op);
}

void WebCore::WRATH_drawImage(WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::Image>& handle, WebCore::ContextOfWRATH *ctx,
                              WebCore::Image* image, WebCore::ColorSpace styleColorSpace,
                              const WebCore::IntRect& dest, const WebCore::IntRect& srcRect,
                              WebCore::CompositeOperator op, bool useLowQualityScale)
{
    // Just passing the call along, same handle usage is safe
    WRATH_drawImage(handle, ctx, image, styleColorSpace, FloatRect(dest), srcRect, op, useLowQualityScale);
}

void WebCore::WRATH_drawTiledImage(WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::Image>& handle, WebCore::ContextOfWRATH *ctx,
                                   WebCore::Image* image, WebCore::ColorSpace styleColorSpace,
                                   const WebCore::IntRect& rect, const WebCore::IntPoint& srcPoint,
                                   const WebCore::IntSize& tileSize, WebCore::CompositeOperator op, bool useLowQualityScale)
{
    WRATH_drawImage_oneHandle *d(WRATH_drawImage_oneHandle::object(image, handle));

    ContextOfWRATH::AutoPushNode autoPushRoot(ctx, d->m_root_node);

    d->m_handle.visible(false);

    if (!image)
        return;

    /*
      Nope on this low quality scale thing

    if (useLowQualityScale) {
        InterpolationQuality previousInterpolationQuality = imageInterpolationQuality();
        setImageInterpolationQuality(InterpolationLow);
        image->drawTiled(this, rect, srcPoint, tileSize, styleColorSpace, op);
        setImageInterpolationQuality(previousInterpolationQuality);
        } else
    */

    d->m_handle.visible(true);
    image->readyWRATHWidgetTiled(d->m_handle, ctx, rect, srcPoint, tileSize, styleColorSpace, op);
}

void WebCore::WRATH_drawTiledImage(WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::Image>& handle, WebCore::ContextOfWRATH *ctx,
                                   WebCore::Image* image, WebCore::ColorSpace styleColorSpace,
                                   const WebCore::IntRect& dest, const WebCore::IntRect& srcRect,
                                   WebCore::Image::TileRule hRule, WebCore::Image::TileRule vRule,
                                   WebCore::CompositeOperator op, bool useLowQualityScale)
{
    WRATH_drawImage_twoHandles *d(WRATH_drawImage_twoHandles::object(image, handle));

    ContextOfWRATH::AutoPushNode autoPushRoot(ctx, d->m_root_node);

    d->m_handleOne.visible(false);
    d->m_handleTwo.visible(false);

    if (!image)
        return;

    if (hRule == Image::StretchTile && vRule == Image::StretchTile) {
        // Just do a scale.
        d->m_handleOne.visible(true);
        WRATH_drawImage(d->m_handleOne, ctx, image, styleColorSpace, dest, srcRect, op);
        return;
    }

    /*
      Nope.

    if (useLowQualityScale) {
        InterpolationQuality previousInterpolationQuality = imageInterpolationQuality();
        setImageInterpolationQuality(InterpolationLow);
        image->drawTiled(this, dest, srcRect, hRule, vRule, styleColorSpace, op);
        setImageInterpolationQuality(previousInterpolationQuality);
    } else
    */

    d->m_handleTwo.visible(true);
    image->readyWRATHWidgetTiled(d->m_handleTwo, ctx, dest, srcRect, hRule, vRule, styleColorSpace, op);
}


#endif
