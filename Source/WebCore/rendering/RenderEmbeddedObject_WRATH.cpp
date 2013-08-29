#include "config.h"
#include "RenderEmbeddedObject.h"

#include "Chrome.h"
#include "ChromeClient.h"
#include "CSSValueKeywords.h"
#include "Font.h"
#include "FontSelector.h"
#include "Frame.h"
#include "FrameLoaderClient.h"
#include "GraphicsContext.h"
#include "HTMLEmbedElement.h"
#include "HTMLIFrameElement.h"
#include "HTMLNames.h"
#include "HTMLObjectElement.h"
#include "HTMLParamElement.h"
#include "LocalizedStrings.h"
#include "MIMETypeRegistry.h"
#include "MouseEvent.h"
#include "Page.h"
#include "PaintInfo.h"
#include "Path.h"
#include "PluginViewBase.h"
#include "RenderTheme.h"
#include "RenderView.h"
#include "RenderWidgetProtector.h"
#include "Settings.h"
#include "Text.h"
#include "TextRun.h"

#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
#include "HTMLVideoElement.h"
#endif

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
#include "WRATHPaintHelpers.h"
#include "DrawnTextOfWRATH.h"

namespace
{
  class RenderEmbeddedObject_WRATHWidgets:
    public WebCore::PaintedWidgetsOfWRATH<RenderEmbeddedObject_WRATHWidgets>
  {
  public:
    WebCore::PaintedWidgetsOfWRATHHandle m_h0, m_h1;
  };

  class RenderEmbeddedObject_WRATHWidgetReplaced:
    public WebCore::PaintedWidgetsOfWRATH<RenderEmbeddedObject_WRATHWidgetReplaced>
  {
  public:
    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_skip_node;
    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_clip_content_rect;

    WRATHShapeF m_path;
    WebCore::FloatSize m_path_rect_size;
    WebCore::ContextOfWRATH::CColorFamily::DrawnShape::AutoDelete m_shape;

    WebCore::FilledFloatRectOfWRATH m_rect;

    WebCore::DrawnTextOfWRATHData m_text;
  };
}

namespace WebCore
{
using namespace HTMLNames;

static const float replacementTextRoundedRectHeight = 18;
static const float replacementTextRoundedRectLeftRightTextMargin = 6;
static const float replacementTextRoundedRectOpacity = 0.20f;
static const float replacementTextPressedRoundedRectOpacity = 0.65f;
static const float replacementTextRoundedRectRadius = 5;
static const float replacementTextTextOpacity = 0.55f;
static const float replacementTextPressedTextOpacity = 0.65f;

static const Color& replacementTextRoundedRectPressedColor()
{
    static const Color lightGray(205, 205, 205);
    return lightGray;
}

void RenderEmbeddedObject::readyWRATHWidgetReplaced(PaintedWidgetsOfWRATHHandle &handle,
                                                    PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
   
    RenderEmbeddedObject_WRATHWidgetReplaced *d;

    d=RenderEmbeddedObject_WRATHWidgetReplaced::object(this, handle);
    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);
    ContextOfWRATH::AutoPushNode autoPushSkip(paintInfo.wrath_context, d->m_skip_node);


    d->m_skip_node.widget()->visible(false);
  
    if (!pluginCrashedOrWasMissing())
        return;

    if (paintInfo.phase == PaintPhaseSelection)
        return;
  
    /*  
    GraphicsContext* context = paintInfo.context;
    if (context->paintingDisabled())
        return;
    */

    FloatRect contentRect;
    FloatRect pathRect;
    FloatRect replacementTextRect;
    Font font;
    TextRun run("");
    float textWidth;
    if (!getReplacementTextGeometry(tx, ty, contentRect, replacementTextRect, font, run, textWidth, pathRect))
        return;

    d->m_skip_node.widget()->visible(true);
    
    //GraphicsContextStateSaver stateSaver(*context);
    //context->clip(contentRect);
    ContextOfWRATH::AutoPushNode autoPushClipping(paintInfo.wrath_context, d->m_clip_content_rect);
    paintInfo.wrath_context->set_clipping(d->m_clip_content_rect, contentRect);

    float alpha;
    Color fillColor;
    vec4 wrath_color;

    //context->setAlpha(m_missingPluginIndicatorIsPressed ? replacementTextPressedRoundedRectOpacity : replacementTextRoundedRectOpacity);
    //context->setFillColor(m_missingPluginIndicatorIsPressed ? replacementTextRoundedRectPressedColor() : Color::white, style()->colorSpace());
    alpha=m_missingPluginIndicatorIsPressed ? replacementTextPressedRoundedRectOpacity : replacementTextRoundedRectOpacity;

    //[WRATH-DANGER]: does fillColor's alpha value take precendence over the setAlpha()?
    fillColor=m_missingPluginIndicatorIsPressed ? replacementTextRoundedRectPressedColor() : Color::white;
    fillColor.getRGBA(wrath_color.x(), wrath_color.y(), wrath_color.z(), wrath_color.w());
    

    //context->fillPath(path);
    // [WRATH-DANGER]: we get a crash on www.zdnet.com
    // in the WRATH triangulation code, likely becuase the
    // rectangle is somehow degenerate too much.
    #if 0
    {
      if(!d->m_shape.widget()
         || d->m_path_rect_size!=pathRect.size()) {
        
        d->m_path.label("RenderEmbeddedObject");
        d->m_path.clear();
        d->m_path_rect_size=pathRect.size();
        
        RoundedFilledRectOfWRATH::AddToShape_TranslateToOrigin(true, d->m_path, 
                                                               pathRect, replacementTextRoundedRectRadius);
        if(d->m_shape.widget()) {
          d->m_shape.widget()->properties()->change_shape(WRATHWidgetGenerator::shape_value(d->m_path));
          paintInfo.wrath_context->update_generic(d->m_shape);
        } else {
          paintInfo.wrath_context->add_filled_shape(d->m_shape,
                                                    WRATHWidgetGenerator::ColorProperties(wrath_color),
                                                    WRATHWidgetGenerator::shape_value(d->m_path));
        }
        
      }   
      d->m_shape.widget()->position(vec2(pathRect.x(), pathRect.y()));
      d->m_shape.widget()->color(wrath_color);      
    }
    #else
    {
      d->m_rect.update(paintInfo.wrath_context, pathRect, fillColor, CompositeCopy);
                    
    }
    #endif

    const FontMetrics& fontMetrics = font.fontMetrics();
    float labelX = roundf(replacementTextRect.location().x() + (replacementTextRect.size().width() - textWidth) / 2);
    float labelY = roundf(replacementTextRect.location().y() + (replacementTextRect.size().height() - fontMetrics.height()) / 2 + fontMetrics.ascent());

    /*
    context->setAlpha(m_missingPluginIndicatorIsPressed ? replacementTextPressedTextOpacity : replacementTextTextOpacity);
    context->setFillColor(Color::black, style()->colorSpace());
    context->drawBidiText(font, run, FloatPoint(labelX, labelY));
   */
    //[WRATH-DANGER]: does fillColor's alpha value take precendence over the setAlpha()?
    alpha=m_missingPluginIndicatorIsPressed ? replacementTextPressedTextOpacity : replacementTextTextOpacity;
    fillColor=Color::black;

    d->m_text.update(paintInfo.wrath_context, font, run, fillColor);
    if(d->m_text.node()) {
      d->m_text.node()->position(vec2(labelX, labelY));
    }


}

void RenderEmbeddedObject::readyWRATHWidgets(PaintedWidgetsOfWRATHHandle &handle,
                                             PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
    RenderEmbeddedObject_WRATHWidgets *d;

    d=RenderEmbeddedObject_WRATHWidgets::object(this, handle);
    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);
    
    d->m_h0.visible(false);
    d->m_h1.visible(false); 

    if (pluginCrashedOrWasMissing()) {
        d->m_h0.visible(true);
        RenderReplaced::readyWRATHWidgets(d->m_h0, paintInfo, tx, ty);
        return;
    }
    
    d->m_h1.visible(true);
    RenderPart::readyWRATHWidgets(d->m_h1, paintInfo, tx, ty);
}
}
#endif
