/*
 * Copyright (C) 2006, 2007, 2008, 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "Gradient.h"

#include "Color.h"
#include "FloatRect.h"
#include <wtf/UnusedParam.h>

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
#include "WRATHGradient.hpp"
#include "Generator_readyWRATHWidgetArguments.h"
#include "WRATHScaleXYTranslate.hpp"

#define BLEND_THRESH 0.9f

namespace
{
  template<typename Linear, typename Radial>
  class Gradient_WidgetsCommon:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::Generator, Gradient_WidgetsCommon<Linear,Radial> >
  {
  public:
    ~Gradient_WidgetsCommon(void)
    {
      m_gradient_ctor_connection.disconnect();
    }

    void
    hide(void)
    {
      if(m_linear_item.widget())
        {
          m_linear_item.widget()->visible(false);
        }
      if(m_radial_item.widget())
        {
          m_radial_item.widget()->visible(false);
        }
    }

    void
    clear_item(void)
    {
      m_gradient_ctor_connection.disconnect();

      {
        typename Linear::Widget *q;
        q=m_linear_item.release_widget();
        if(q)
          {
            WRATHPhasedDelete(q);
          }
      }

      {
        typename Radial::Widget *q;
        q=m_radial_item.release_widget();
        if(q)
          {
            WRATHPhasedDelete(q);
          }
      }
    }

    void
    readyWidget(WebCore::Gradient *webcore_gradient,
                const WebCore::FloatRect &srcRect,
                const WebCore::FloatRect &dstRect,
                WebCore::ContextOfWRATH *ctx,
                WebCore::ColorSpace styleColorSpace,
                WebCore::CompositeOperator compositeOp,
                bool has_interesting_alpha,
                bool radial);
    
    float m_aspect;
    WebCore::FloatRect m_src_rect;
    vec2 m_dest_rect_size;
    WebCore::CompositeOperator m_op;
    boost::signals2::connection m_gradient_ctor_connection;
    typename Linear::AutoDelete m_linear_item;
    typename Radial::AutoDelete m_radial_item;
  };


  typedef Gradient_WidgetsCommon<WebCore::ContextOfWRATH::LinearGradientFamily::DrawnRect,
                                 WebCore::ContextOfWRATH::RadialGradientFamily::DrawnRect> Gradient_Widgets;

  typedef Gradient_WidgetsCommon<WebCore::ContextOfWRATH::LinearRepeatGradientFamily::DrawnRect,
                                 WebCore::ContextOfWRATH::RadialRepeatGradientFamily::DrawnRect> GeneratedImage_WidgetPattern;


  
}

#endif

template<typename Linear, typename Radial>
void
Gradient_WidgetsCommon<Linear, Radial>::
readyWidget(WebCore::Gradient *webcore_gradient,
            const WebCore::FloatRect &srcRect,
            const WebCore::FloatRect &dstRect,
            WebCore::ContextOfWRATH *ctx,
            WebCore::ColorSpace styleColorSpace,
            WebCore::CompositeOperator compositeOp,
            bool has_interesting_alpha,
            bool radial)
{
  bool recreate;
  WRATHGradient *gradient(webcore_gradient->gradientOfWRATH());
  vec2 dest_rect_size(dstRect.width(), dstRect.height());

  hide();
  if(!gradient)
    {
      return;
    }

  WebCore::ContextOfWRATH::AutoPushNode autoPushRoot(ctx, this->m_root_node);
  m_gradient_ctor_connection.disconnect();

  recreate=(!radial && !m_linear_item.widget())
    || (radial && !m_radial_item.widget())
    || m_op!=compositeOp;
  
  WRATHGLStateChange::state_change::handle blender;
  if(has_interesting_alpha or compositeOp!=WebCore::CompositeSourceOver)
    {
      blender=WebCore::ContextOfWRATH::getBlenderFromCompositeOp(compositeOp);
    }
  
  vec2 p0(webcore_gradient->p0().x(), webcore_gradient->p0().y());
  vec2 p1(webcore_gradient->p1().x(), webcore_gradient->p1().y());

  if(recreate 
     || m_aspect!=webcore_gradient->aspectRatio()
     || m_dest_rect_size!=dest_rect_size
     || m_src_rect!=srcRect)
    {
      m_dest_rect_size=dest_rect_size;
      m_op=compositeOp;
      m_aspect=webcore_gradient->aspectRatio();
      m_src_rect=srcRect;
      /*
        The item coordinate coordinates
        go from 0 to dest_rect_size.
        We want to feed the gradient
        values m_src_rect.x()/.y() to
        m_src_rect.maxX()/.maxY()  
       */
      WRATHDefaultRectAttributePacker::Rect::handle rect;
      rect=WRATHNew WRATHDefaultRectAttributePacker::Rect(m_dest_rect_size);
      if(m_dest_rect_size.x()!=0.0f and m_dest_rect_size.y()!=0.0f)
        {
          vec2 sz_ratio;
          sz_ratio=vec2(srcRect.width(), srcRect.height())/m_dest_rect_size;

          if(m_aspect!=1.0f and m_aspect!=0.0f)
            {
              WRATHScaleXYTranslate tr;          

              tr= WRATHScaleXYTranslate().translation(vec2(srcRect.x(), srcRect.y()))
                * WRATHScaleXYTranslate().scale(sz_ratio)
                * WRATHScaleXYTranslate().translation(p0)
                * WRATHScaleXYTranslate().scale(vec2(1.0f, 1.0f/m_aspect))
                * WRATHScaleXYTranslate().translation(-p0);
              rect->m_brush_offset=tr.translation();
              rect->m_brush_stretch = tr.scale();
            }
          else
            {
              rect->m_brush_offset=vec2(srcRect.x(), srcRect.y());
              rect->m_brush_stretch=sz_ratio;
            }
        }

      if(recreate)
        {
          WRATHDrawType pass;
          WRATHWidgetGenerator::Brush brush(gradient);

          if(blender.valid())
            {
              pass=WRATHDrawType(WebCore::TransparentImagePassEnumerationOfWRATH, 
                                 WRATHDrawType::transparent_draw);
              brush.premultiply_alpha(true);
              brush.gradient_interpolate_enforce_by_blend(true);
              brush.m_draw_state.add_gl_state_change(blender);
            }
          else
            {
              pass=WRATHDrawType(0, WRATHDrawType::opaque_draw);
            }

          /*
            should we enforce the [0,1]?
           
            brush.gradient_interpolate_enforce_positive(true);
            brush.gradient_interpolate_enforce_greater_than_one(true);
          */

          if(radial)
            {
              m_radial_item.delete_widget();
              ctx->add_rect(m_radial_item, WRATHWidgetGenerator::Rect(rect), brush, pass);
            }
          else
            {
              m_linear_item.delete_widget();
              ctx->add_rect(m_linear_item, WRATHWidgetGenerator::Rect(rect), brush, pass);
            }
          m_gradient_ctor_connection=gradient->connect_dtor(boost::bind(&Gradient_WidgetsCommon::clear_item, this));
        }
      else
        {
          if(radial)
            {
              WRATHWidgetGenerator::Rect(rect)(m_radial_item.widget());
            }
          else
            {
              WRATHWidgetGenerator::Rect(rect)(m_linear_item.widget());
            }
        }
    }

  if(!recreate)
    {
      if(radial)
        {
          ctx->update_generic(m_radial_item);
        }
      else
        {
          ctx->update_generic(m_linear_item);
        }
    }

  

  if(radial)
    {
      m_radial_item.widget()->visible(true);
      m_radial_item.widget()->position(vec2(dstRect.x(), dstRect.y()));
      m_radial_item.widget()->set_gradient(p0, webcore_gradient->startRadius(), 
                                           p1, webcore_gradient->endRadius());
    }
  else
    {
      m_linear_item.widget()->visible(true);
      m_linear_item.widget()->position(vec2(dstRect.x(), dstRect.y()));
      m_linear_item.widget()->set_gradient(p0, p1);
    }

  if(blender.valid()) 
    {
      ctx->m_number_transparent_rects++;
    }
}


namespace WebCore {

Gradient::Gradient(const FloatPoint& p0, const FloatPoint& p1)
    : m_radial(false)
    , m_p0(p0)
    , m_p1(p1)
    , m_r0(0)
    , m_r1(0)
    , m_aspectRatio(1)
    , m_stopsSorted(false)
    , m_lastStop(0)
    , m_spreadMethod(SpreadMethodPad)
#if USE(WRATH)
    , m_gradientOfWRATH(0)
#endif
{
    platformInit();
}

Gradient::Gradient(const FloatPoint& p0, float r0, const FloatPoint& p1, float r1, float aspectRatio)
    : m_radial(true)
    , m_p0(p0)
    , m_p1(p1)
    , m_r0(r0)
    , m_r1(r1)
    , m_aspectRatio(aspectRatio)
    , m_stopsSorted(false)
    , m_lastStop(0)
    , m_spreadMethod(SpreadMethodPad)
#if USE(WRATH)
    , m_gradientOfWRATH(0)
#endif
{
    platformInit();
}

Gradient::~Gradient()
{
    platformDestroy();

#if USE(WRATH)
    purgeGradientOfWRATH();
#endif
}

void Gradient::adjustParametersForTiledDrawing(IntSize& size, FloatRect& srcRect)
{
    if (m_radial)
        return;

    if (srcRect.isEmpty())
        return;

    if (m_p0.x() == m_p1.x()) {
        size.setWidth(1);
        srcRect.setWidth(1);
        srcRect.setX(0);
        return;
    }
    if (m_p0.y() != m_p1.y())
        return;

    size.setHeight(1);
    srcRect.setHeight(1);
    srcRect.setY(0);
}

void Gradient::addColorStop(float value, const Color& color)
{
    float r;
    float g;
    float b;
    float a;
    color.getRGBA(r, g, b, a);
    m_stops.append(ColorStop(value, r, g, b, a));

    m_stopsSorted = false;
    platformDestroy();
#if USE(WRATH)
    /*
      no need to recreate the WRATHGradient.
     */
    if(m_gradientOfWRATH) {
      m_gradientOfWRATH->set_color(value, WRATHGradient::color(r, g, b, a));
      m_has_interesting_alpha=m_has_interesting_alpha || (a<BLEND_THRESH);
    }
#endif
}

void Gradient::addColorStop(const Gradient::ColorStop& stop)
{
    m_stops.append(stop);

    m_stopsSorted = false;
    platformDestroy();
#if USE(WRATH)
    /*
      no need to recreate the WRATHGradient, it can add the stop.
     */
    if(m_gradientOfWRATH) {
      m_gradientOfWRATH->set_color(stop.stop, WRATHGradient::color(stop.red, stop.green, stop.blue, stop.alpha));
      m_has_interesting_alpha=m_has_interesting_alpha || (stop.alpha<BLEND_THRESH);
    }
#endif


}

static inline bool compareStops(const Gradient::ColorStop& a, const Gradient::ColorStop& b)
{
    return a.stop < b.stop;
}

void Gradient::sortStopsIfNecessary()
{
    if (m_stopsSorted)
        return;

    m_stopsSorted = true;

    if (!m_stops.size())
        return;

    std::stable_sort(m_stops.begin(), m_stops.end(), compareStops);
}

void Gradient::getColor(float value, float* r, float* g, float* b, float* a) const
{
    ASSERT(value >= 0);
    ASSERT(value <= 1);

    if (m_stops.isEmpty()) {
        *r = 0;
        *g = 0;
        *b = 0;
        *a = 0;
        return;
    }
    if (!m_stopsSorted) {
        if (m_stops.size())
            std::stable_sort(m_stops.begin(), m_stops.end(), compareStops);
        m_stopsSorted = true;
    }
    if (value <= 0 || value <= m_stops.first().stop) {
        *r = m_stops.first().red;
        *g = m_stops.first().green;
        *b = m_stops.first().blue;
        *a = m_stops.first().alpha;
        return;
    }
    if (value >= 1 || value >= m_stops.last().stop) {
        *r = m_stops.last().red;
        *g = m_stops.last().green;
        *b = m_stops.last().blue;
        *a = m_stops.last().alpha;
        return;
    }

    // Find stop before and stop after and interpolate.
    int stop = findStop(value);
    const ColorStop& lastStop = m_stops[stop];    
    const ColorStop& nextStop = m_stops[stop + 1];
    float stopFraction = (value - lastStop.stop) / (nextStop.stop - lastStop.stop);
    *r = lastStop.red + (nextStop.red - lastStop.red) * stopFraction;
    *g = lastStop.green + (nextStop.green - lastStop.green) * stopFraction;
    *b = lastStop.blue + (nextStop.blue - lastStop.blue) * stopFraction;
    *a = lastStop.alpha + (nextStop.alpha - lastStop.alpha) * stopFraction;
}

int Gradient::findStop(float value) const
{
    ASSERT(value >= 0);
    ASSERT(value <= 1);
    ASSERT(m_stopsSorted);

    int numStops = m_stops.size();
    ASSERT(numStops >= 2);
    ASSERT(m_lastStop < numStops - 1);

    int i = m_lastStop;
    if (value < m_stops[i].stop)
        i = 1;
    else
        i = m_lastStop + 1;

    for (; i < numStops - 1; ++i)
        if (value < m_stops[i].stop)
            break;

    m_lastStop = i - 1;
    return m_lastStop;
}

bool Gradient::hasAlpha() const
{
    for (size_t i = 0; i < m_stops.size(); i++) {
        if (m_stops[i].alpha < 1)
            return true;
    }

    return false;
}

void Gradient::setSpreadMethod(GradientSpreadMethod spreadMethod)
{
    // FIXME: Should it become necessary, allow calls to this method after m_gradient has been set.
    ASSERT(m_gradient == 0);
    m_spreadMethod = spreadMethod;
}

void Gradient::setGradientSpaceTransform(const AffineTransform& gradientSpaceTransformation)
{ 
    m_gradientSpaceTransformation = gradientSpaceTransformation;
    setPlatformGradientSpaceTransform(gradientSpaceTransformation);
}

#if !USE(SKIA) && !USE(CAIRO)
void Gradient::setPlatformGradientSpaceTransform(const AffineTransform&)
{
}
#endif

#if USE(WRATH)
void Gradient::purgeGradientOfWRATH(void)
{
  if(m_gradientOfWRATH) {
    m_purgeSignal();
    WRATHPhasedDelete(m_gradientOfWRATH);
  }
}

void Gradient::readyWRATHWidgets(const Generator_readyWRATHWidgetsArguments &args)
{
  /*
    we get same output on http://www.westciv.com/tools/radialgradients/index.html
    as QtWebKit, but that output is different than Chrome.
   */
  Gradient_Widgets *d;
  d=Gradient_Widgets::object(this, args.handle);
  d->readyWidget(this,
                 args.srcRect, args.dstRect, 
                 args.ctx, 
                 args.styleColorSpace,
                 args.compositeOp,
                 m_has_interesting_alpha,
                 m_radial);
}

void Gradient::readyWRATHWidgetPattern(const Generator_readyWRATHWidgetPatternArguments &args)
{


  GeneratedImage_WidgetPattern *d;
  d=GeneratedImage_WidgetPattern::object(this, args.handle);
  /*
    see comments in Image.cpp how we generate 
    texRect to feed readyWidget.
   */
  
  FloatRect tileRectInTargetCoords(args.patternTransform.mapRect(args.srcRect));
  vec2 oneRectTexelCoordsLocation(tileRectInTargetCoords.x(), tileRectInTargetCoords.y());
  vec2 oneRectTexelCoordsSize(tileRectInTargetCoords.width(), tileRectInTargetCoords.height());
  vec2 srcRectLocation(args.srcRect.x(), args.srcRect.y()), srcRectSize(args.srcRect.width(), args.srcRect.height());
  vec2 min_p, max_p, delta_p, count_before, count_after;
  count_before= ( - oneRectTexelCoordsLocation )/oneRectTexelCoordsSize;
  count_after=  ( vec2(args.dstRect.width(), args.dstRect.height()) - oneRectTexelCoordsLocation)/oneRectTexelCoordsSize;
  min_p= srcRectLocation + count_before*srcRectSize;
  max_p= srcRectLocation + count_after*srcRectSize;
  delta_p=max_p - min_p;

  FloatRect texRect(FloatPoint(min_p.x(), min_p.y()),
                    FloatSize(delta_p.x(), delta_p.y()) );

  d->readyWidget(this,
                 texRect, args.dstRect, 
                 args.ctx, 
                 args.styleColorSpace,
                 args.op,
                 m_has_interesting_alpha,
                 m_radial);
  
  if(m_radial)
    {
      if(d->m_radial_item.widget())
        {
          d->m_radial_item.widget()->set_window(vec2(args.srcRect.x(), args.srcRect.y()), 
                                                vec2(args.srcRect.maxX(), args.srcRect.maxY()) );
        }
    }
  else
    {
      if(d->m_linear_item.widget())
        {
          d->m_linear_item.widget()->set_window(vec2(args.srcRect.x(), args.srcRect.y()), 
                                                vec2(args.srcRect.maxX(), args.srcRect.maxY()) );
        }
    }
}

WRATHGradient *Gradient::gradientOfWRATH(void)
{
  if(!m_gradientOfWRATH) {

    /*
      create the gradient of WRATH
     */
    sortStopsIfNecessary();
    m_has_interesting_alpha=false;

    if(m_stops.isEmpty()) {
      m_gradientOfWRATH=WRATHNew WRATHGradient(WRATHGradient::parameters(WRATHGradient::Clamp,
                                                                         1));
      m_gradientOfWRATH->set_color(0.0f, vec4(0.0f, 0.0f, 0.0f, 0.0f));
      m_gradientOfWRATH->set_color(1.0f, vec4(0.0f, 0.0f, 0.0f, 0.0f));
    }
    else {
      
      enum WRATHGradient::repeat_type_t repeat_mode;

      switch(m_spreadMethod) {
      case SpreadMethodPad:
        repeat_mode=WRATHGradient::Clamp;
        break;
      case SpreadMethodReflect:
        repeat_mode=WRATHGradient::MirrorRepeat;
        break;
        
      default:
        //fall through
      case SpreadMethodRepeat:
        repeat_mode=WRATHGradient::Repeat;
        break;
      }

      /*
        [WRATH-TODO]: examine the location of the stops to make a good
        guess on the resolution of the WRATHGradient.
       */
      m_gradientOfWRATH=WRATHNew WRATHGradient(repeat_mode);
      for(Vector<ColorStop>::iterator iter=m_stops.begin(), end=m_stops.end(); iter!=end; ++iter) {

        vec4 color(iter->red, iter->green, iter->blue, iter->alpha);

        m_has_interesting_alpha=m_has_interesting_alpha || (iter->alpha<BLEND_THRESH);
        m_gradientOfWRATH->set_color(iter->stop, color);

      } //for

    } //else

  } //if

  return m_gradientOfWRATH;
}


#endif


} //namespace
