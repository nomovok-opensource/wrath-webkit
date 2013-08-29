#include "config.h"
#include "RenderBox.h"

#include "CachedImage.h"
#include "Chrome.h"
#include "ChromeClient.h"
#include "Document.h"
#include "FrameView.h"
#include "GraphicsContext.h"
#include "HitTestResult.h"
#include "htmlediting.h"
#include "HTMLElement.h"
#include "HTMLNames.h"
#include "ImageBuffer.h"
#include "FloatQuad.h"
#include "Frame.h"
#include "Page.h"
#include "PaintInfo.h"
#include "RenderArena.h"
#include "RenderFlexibleBox.h"
#include "RenderInline.h"
#include "RenderLayer.h"
#include "RenderTableCell.h"
#include "RenderTheme.h"
#include "RenderView.h"
#include "ScrollbarTheme.h"
#include "TransformState.h"
#include <algorithm>
#include <math.h>

using namespace std;

#if USE(WRATH)
#include "WRATHTextureFontFreeType_Mix.hpp"
#include "WRATHTextDataStream.hpp"
#include "WRATHFontConfig.hpp"
#include "PaintInfoOfWRATH.h"
#include "HierarchyOfHandlesOfWRATH.h"
#include "HierarchyOfHandlesOfWRATH_ChildList.h"
#include "WRATHPaintHelpers.h"

namespace
{
  class RenderBox_PushPopContentsClip:public WebCore::PaintedWidgetsOfWRATHBase
  {
  public:

    RenderBox_PushPopContentsClip(void):
      m_rounded_clipping_rect(WebCore::IntRect())
    {}

    void
    visible(bool v)
    {
      m_push_contents.visible(v);
      m_pop_contents.visible(v);
      if(m_rounded_clipping_canvas.widget())
        m_rounded_clipping_canvas.widget()->visible(v);
    }

    static
    RenderBox_PushPopContentsClip*
    object(WebCore::RenderBox *p, WebCore::PaintedWidgetsOfWRATHHandle &obj)
    {
      return objectNoArgCtor(type_tag<RenderBox_PushPopContentsClip>(), p, obj);
    }

    WebCore::PaintedWidgetsOfWRATHHandle m_push_contents, m_pop_contents;
    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_push_node;

    bool m_does_rounded_clipping;
    WebCore::ContextOfWRATH::DrawnCanvas::AutoDelete m_rounded_clipping_canvas;
    WebCore::ContextOfWRATH::CPlainFamily::DrawnShape::AutoDelete m_clip_item;
    WebCore::RoundedIntRect m_rounded_clipping_rect;

  };

  class RenderBox_ReadyWRATHWidgets:
    public WebCore::PaintedWidgetsOfWRATH<RenderBox_ReadyWRATHWidgets>
  {
  public:
    WebCore::HierarchyOfHandlesOfWRATH<WebCore::RenderObject> m_perRenderObject;
  };

  


  class RenderBox_ReadyWidgetBoxDecorations:
    public WebCore::PaintedWidgetsOfWRATH<RenderBox_ReadyWidgetBoxDecorations>
  {
  public:
      WebCore::PaintedWidgetsOfWRATHHandle m_box_decorations_with_size;
  };

  class RenderBox_ReadyWidgetBoxDecorationsWithSize: 
    public WebCore::PaintedWidgetsOfWRATH<RenderBox_ReadyWidgetBoxDecorationsWithSize>
  {
  public:
      WebCore::PaintedWidgetsOfWRATHHandle m_borderHandle;
      WebCore::PaintedWidgetsOfWRATHHandle m_boxShadowHandle1;
      WebCore::PaintedWidgetsOfWRATHHandle m_boxShadowHandle2;
      WebCore::PaintedWidgetsOfWRATHHandle m_root_box_fill_layers;
      WebCore::PaintedWidgetsOfWRATHHandle m_non_root_fill_layers;
      WebCore::PaintedWidgetsOfWRATHHandle m_theme_paint;
      WebCore::PaintedWidgetsOfWRATHHandle m_theme_decorations;
      WebCore::PaintedWidgetsOfWRATHHandle m_theme_border;

  };

  class RenderBox_ReadyWRATHWidgetFillLayers:
    public WebCore::PaintedWidgetsOfWRATH<RenderBox_ReadyWRATHWidgetFillLayers>
  {
  public:
      WebCore::PaintedWidgetsOfWRATHHandle m_layer;
      WebCore::PaintedWidgetsOfWRATHHandle m_next_layers;
  };

  class RenderBox_ReadyWRATHWidgetRootBoxFillLayers:
    public WebCore::PaintedWidgetsOfWRATH<RenderBox_ReadyWRATHWidgetRootBoxFillLayers>
  {
  public:
      WebCore::PaintedWidgetsOfWRATHHandle m_layer;
  };
}

namespace WebCore
{
using namespace HTMLNames;

void RenderBox::readyWRATHWidgets(PaintedWidgetsOfWRATHHandle &handle, 
                                  PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
  tx+=x();
  ty+=y();

  RenderBox_ReadyWRATHWidgets *d(RenderBox_ReadyWRATHWidgets::object(this, handle));
  ContextOfWRATH::AutoPushNode autoPushVisible(paintInfo.wrath_context, d->m_root_node);
  PaintInfoOfWRATH childInfo(paintInfo);

  d->m_perRenderObject.hideEachObject();

  childInfo.updatePaintingRootForChildren(this);
  for (RenderObject* child = firstChild(); child; child = child->nextSibling()) {
    PaintedWidgetsOfWRATHHandle &ch(d->m_perRenderObject.getHandle(child));
    ch.visible(true);
    child->readyWRATHWidgets(ch, childInfo, tx, ty);
  }

  d->m_perRenderObject.removeNonVisibleHandles();
}

void RenderBox::readyWRATHWidgetObject(PaintedWidgetsOfWRATHHandle&, 
                                       PaintInfoOfWRATH&, int, int)
{
  std::cerr << "\nCall of RenderBox::readyWRATHWidgetObject should never be reached\n";
  
}

bool RenderBox::pushContentsClipWRATH(PaintedWidgetsOfWRATHHandle &handle,
                                      PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
  
  RenderBox_PushPopContentsClip *d;
  d=RenderBox_PushPopContentsClip::object(this, handle);
  
  d->m_push_contents.visible(false);
  d->m_pop_contents.visible(false);

  if(d->m_rounded_clipping_canvas.widget())
    d->m_rounded_clipping_canvas.widget()->visible(false);

  if (paintInfo.phase == PaintPhaseBlockBackground || paintInfo.phase == PaintPhaseSelfOutline || paintInfo.phase == PaintPhaseMask)
    return false;
        
  bool isControlClip = hasControlClip();
  bool isOverflowClip = hasOverflowClip() && !layer()->isSelfPaintingLayer();

  if (!isControlClip && !isOverflowClip) 
    return false;


  if (paintInfo.phase == PaintPhaseOutline)
    paintInfo.phase = PaintPhaseChildOutlines;
  else if (paintInfo.phase == PaintPhaseChildBlockBackground) {
    paintInfo.phase = PaintPhaseBlockBackground;
    d->m_push_contents.visible(true);
    readyWRATHWidgetObject(d->m_push_contents, paintInfo, tx, ty);
    paintInfo.phase = PaintPhaseChildBlockBackgrounds;
  }

  IntRect clipRect(isControlClip ? controlClipRect(tx, ty) : overflowClipRect(tx, ty));

  paintInfo.wrath_context->push_node(d->m_push_node);
  ContextOfWRATH::set_clipping(d->m_push_node, clipRect);

  if(style()->hasBorderRadius()) {
    RoundedIntRect rounded=style()->getRoundedBorderFor(IntRect(tx, ty, width(), height()));

    
    WRATH_PUSH_CANVAS_NODE(paintInfo.wrath_context, d->m_rounded_clipping_canvas);
    d->m_rounded_clipping_canvas.widget()->visible(true);
    
    if(!d->m_clip_item.widget()) {
      WRATHShapeF the_shape;

      RoundedFilledRectOfWRATH::AddToShape_TranslateToOrigin(true, the_shape, rounded);
      paintInfo.wrath_context->canvas_clipping()
        .clip_filled_shape(WRATHWidgetGenerator::clip_inside, d->m_clip_item,
                           WRATHWidgetGenerator::shape_value(the_shape));
    }
    else if(d->m_rounded_clipping_rect.rect().size()!=rounded.rect().size()
            || !(d->m_rounded_clipping_rect.radii()==rounded.radii())) {
      
      WRATHShapeF the_shape;

      RoundedFilledRectOfWRATH::AddToShape_TranslateToOrigin(true, the_shape, rounded);
      d->m_clip_item.widget()->properties()
        ->change_shape(WRATHWidgetGenerator::shape_value(the_shape),
                       WRATHWidgetGenerator::FillingParameters());
    }
    d->m_rounded_clipping_rect=rounded;
    d->m_does_rounded_clipping=true;
    d->m_clip_item.widget()->position(vec2(rounded.rect().x(), rounded.rect().y()));

  } else {
    d->m_does_rounded_clipping=false;
  }

  return true;


}

void RenderBox::popContentsClipWRATH(PaintedWidgetsOfWRATHHandle &handle,
                                     PaintInfoOfWRATH &paintInfo, PaintPhase originalPhase, int tx, int ty)
{
  ASSERT(hasControlClip() || (hasOverflowClip() && !layer()->isSelfPaintingLayer()));

  RenderBox_PushPopContentsClip *d;
  d=RenderBox_PushPopContentsClip::object(this, handle);

  if(d->m_does_rounded_clipping) {
    paintInfo.wrath_context->pop_node();
  }
  paintInfo.wrath_context->pop_node();

  if (originalPhase == PaintPhaseOutline) {
    paintInfo.phase = PaintPhaseSelfOutline;
    d->m_pop_contents.visible(true);
    readyWRATHWidgetObject(d->m_pop_contents, paintInfo, tx, ty);
    paintInfo.phase = originalPhase;
  } else if (originalPhase == PaintPhaseChildBlockBackground)
    paintInfo.phase = originalPhase;
}

void RenderBox::readyWRATHWidgetBoxDecorations(PaintedWidgetsOfWRATHHandle& handle,
                                                PaintInfoOfWRATH& paintInfo, int tx, int ty)
{
    RenderBox_ReadyWidgetBoxDecorations* d;
    d=RenderBox_ReadyWidgetBoxDecorations::object(this, handle);
    WebCore::ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    

    if (!paintInfo.shouldPaintWithinRoot(this)) {
        d->m_box_decorations_with_size.visible(false);
        return;
    }
    else {
      d->m_box_decorations_with_size.visible(true);
      readyWRATHWidgetBoxDecorationsWithSize(d->m_box_decorations_with_size,
                                             paintInfo, tx, ty, width(), height());
    }
}

void RenderBox::readyWRATHWidgetMask(PaintedWidgetsOfWRATHHandle&, PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
  /*
    [WRATH-TODO] from paintMask
   */
  WRATH_UNIMPLEMENTED(paintInfo.wrath_context);
}

void RenderBox::readyWRATHWidgetFillLayer(PaintedWidgetsOfWRATHHandle& handle,
					  const PaintInfoOfWRATH& paintInfo,
					  const Color& c, const FillLayer* fillLayer,
					  int tx, int ty, int width, int height,
					  BackgroundBleedAvoidance bleedAvoidance,
					  CompositeOperator op, RenderObject* backgroundObject)
{
    /* Just forwarding a call, using same handle is ok */
    readyWRATHWidgetFillLayerExtended(handle, paintInfo, c, fillLayer, tx, ty, width, height, bleedAvoidance, 0, 0, 0, op, backgroundObject);
}

void RenderBox::readyWRATHWidgetFillLayers(PaintedWidgetsOfWRATHHandle& handle,
					   const PaintInfoOfWRATH& paintInfo,
					   const Color& c, const FillLayer* fillLayer,
					   int tx, int ty, int width, int height,
					   BackgroundBleedAvoidance bleedAvoidance,
					   CompositeOperator op, RenderObject* backgroundObject)
{
    RenderBox_ReadyWRATHWidgetFillLayers *d(RenderBox_ReadyWRATHWidgetFillLayers::object(this, handle));
    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);


    if (!fillLayer) {
        d->m_layer.clear(); //visible(false);
        d->m_next_layers.clear(); //visible(false);
        return;
      }

    readyWRATHWidgetFillLayers(d->m_next_layers, paintInfo, c, fillLayer->next(), tx, ty, width, height, bleedAvoidance, op, backgroundObject);
    readyWRATHWidgetFillLayer(d->m_layer, paintInfo, c, fillLayer, tx, ty, width, height, bleedAvoidance, op, backgroundObject);
}

void RenderBox::readyWRATHWidgetBoxDecorationsWithSize(PaintedWidgetsOfWRATHHandle& handle, 
						       PaintInfoOfWRATH& paintInfo,
						       int tx, int ty, int width, int height)
{
    RenderBox_ReadyWidgetBoxDecorationsWithSize* d;
    d=RenderBox_ReadyWidgetBoxDecorationsWithSize::object(this, handle);

    ContextOfWRATH::AutoPushNode visiblityRootPush(paintInfo.wrath_context, d->m_root_node);

    d->m_root_box_fill_layers.visible(false);
    d->m_non_root_fill_layers.visible(false);
    d->m_borderHandle.visible(false);
    d->m_theme_paint.visible(false);
    d->m_theme_decorations.visible(false);
    d->m_theme_border.visible(false);

    borderFitAdjust(tx, width);

    readyWRATHWidgetBoxShadow(d->m_boxShadowHandle1, paintInfo.wrath_context,
                              tx, ty, width, height, style(), Normal);


    BackgroundBleedAvoidance bleedAvoidance = BackgroundBleedNone; //determineBackgroundBleedAvoidance(paintInfo.context);

    /******************************************************
      [WRATH-DANGER] We won't bother with this bleed avoidance nonsense... 
     ****************************************************** 

      GraphicsContextStateSaver stateSaver(*paintInfo.context, false);
      if (bleedAvoidance == BackgroundBleedUseTransparencyLayer) {
      // To avoid the background color bleeding out behind the border, we'll render background and border
      // into a transparency layer, and then clip that in one go (which requires setting up the clip before
      // beginning the layer).
      RoundedIntRect border = style()->getRoundedBorderFor(IntRect(tx, ty, width, height));
      stateSaver.save();
      paintInfo.context->addRoundedRectClip(border);
      paintInfo.context->beginTransparencyLayer(1);
      }
    ****************/

    // If we have a native theme appearance, paint that before painting our background.
    // The theme will tell us whether or not we should also paint the CSS background.
    bool themePainted = style()->hasAppearance() 
      && !theme()->readyWRATHWidgets(this, d->m_theme_paint, paintInfo, IntRect(tx, ty, width, height));

    if (!themePainted) {
        if (isRoot()) {
	    d->m_root_box_fill_layers.visible(true);
            readyWRATHWidgetRootBoxFillLayers(d->m_root_box_fill_layers, paintInfo);
	}
        else if (!isBody() || document()->documentElement()->renderer()->hasBackground()) {
            // The <body> only paints its background if the root element has defined a background
            // independent of the body.
	    d->m_non_root_fill_layers.visible(true);
            readyWRATHWidgetFillLayers(d->m_non_root_fill_layers, paintInfo, style()->visitedDependentColor(CSSPropertyBackgroundColor), style()->backgroundLayers(), tx, ty, width, height, bleedAvoidance);
        }
        if (style()->hasAppearance()) {
            theme()->readyWRATHWidgetDecorations(this, d->m_theme_decorations, paintInfo, IntRect(tx, ty, width, height));
      	}
    }

    readyWRATHWidgetBoxShadow(d->m_boxShadowHandle2, paintInfo.wrath_context,
                              tx, ty, width, height, style(), Inset);

    
    if ((!style()->hasAppearance() || (!themePainted && theme()->readyWRATHWidgetBorderOnly(this, d->m_theme_border, paintInfo, IntRect(tx, ty, width, height)))) && style()->hasBorder())
    {
      d->m_borderHandle.visible(true);
      readyWRATHWidgetBorder(d->m_borderHandle, paintInfo.wrath_context,
                             tx, ty, width, height,
                             style(), bleedAvoidance);
    }

    /*************************************
       Again ignoring this background bleed avoidance nonsence...
     ************************************
    if (bleedAvoidance == BackgroundBleedUseTransparencyLayer) {
        paintInfo.context->endTransparencyLayer();
    }
    */
}

void RenderBox::readyWRATHWidgetMaskImages(PaintedWidgetsOfWRATHHandle&, 
					   const PaintInfoOfWRATH&, int tx, int ty, int width, int height)
{
  /*
    TODO from paintMaskImages
   */
}

void RenderBox::readyWRATHWidgetRootBoxFillLayers(PaintedWidgetsOfWRATHHandle& handle,
						  const PaintInfoOfWRATH& paintInfo)
{
    RenderBox_ReadyWRATHWidgetRootBoxFillLayers *d(RenderBox_ReadyWRATHWidgetRootBoxFillLayers::object(this, handle));
    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    const FillLayer* bgLayer = style()->backgroundLayers();
    Color bgColor = style()->visitedDependentColor(CSSPropertyBackgroundColor);
    RenderObject* bodyObject = 0;
    if (!hasBackground() && node() && node()->hasTagName(HTMLNames::htmlTag)) {
        // Locate the <body> element using the DOM.  This is easier than trying
        // to crawl around a render tree with potential :before/:after content and
        // anonymous blocks created by inline <body> tags etc.  We can locate the <body>
        // render object very easily via the DOM.
        HTMLElement* body = document()->body();
        bodyObject = (body && body->hasLocalName(bodyTag)) ? body->renderer() : 0;
        if (bodyObject) {
            bgLayer = bodyObject->style()->backgroundLayers();
            bgColor = bodyObject->style()->visitedDependentColor(CSSPropertyBackgroundColor);
        }
    }

    // The background of the box generated by the root element covers the entire canvas, so just use
    // the RenderView's docTop/Left/Width/Height accessors.

    readyWRATHWidgetFillLayers(d->m_layer, paintInfo, bgColor, bgLayer, view()->docLeft(), view()->docTop(), view()->docWidth(), view()->docHeight(), BackgroundBleedNone, CompositeSourceOver, bodyObject);
}

}

#endif
