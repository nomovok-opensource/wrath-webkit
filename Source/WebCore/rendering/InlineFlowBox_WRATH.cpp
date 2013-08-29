#include "config.h"
#include "InlineFlowBox.h"

#include "CachedImage.h"
#include "CSSPropertyNames.h"
#include "Document.h"
#include "EllipsisBox.h"
#include "GraphicsContext.h"
#include "InlineTextBox.h"
#include "HitTestResult.h"
#include "RootInlineBox.h"
#include "RenderBlock.h"
#include "RenderInline.h"
#include "RenderLayer.h"
#include "RenderListMarker.h"
#include "RenderRubyBase.h"
#include "RenderRubyRun.h"
#include "RenderRubyText.h"
#include "RenderTableCell.h"
#include "RootInlineBox.h"
#include "Text.h"

#include <math.h>

using namespace std;

#if USE(WRATH)
#include <map>
#include "PaintInfoOfWRATH.h"
#include "HierarchyOfHandlesOfWRATH.h"

namespace {

  

  class InlineFlowBox_Widgets:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::InlineBox, InlineFlowBox_Widgets>
  {
  public:
    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_skip;
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::InlineBox> m_mask, m_box_decorations;
    WebCore::HierarchyOfHandlesOfWRATH<WebCore::InlineBox, WebCore::InlineBox> m_children;
  };

  class InlineFlowBox_BoxShadow:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::InlineBox, InlineFlowBox_BoxShadow>
  {
  public:
    WebCore::PaintedWidgetsOfWRATHHandle m_branch0, m_branch1;
  };

  class InlineFlowBox_BoxDecorations:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::InlineBox, InlineFlowBox_BoxDecorations>
  {
  public:
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::InlineBox> m_box_shadow_normal;
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::InlineBox> m_box_shadow_inset;
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::InlineBox> m_fill_layers;

    WebCore::PaintedWidgetsOfWRATHHandle m_border0;
    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_border1_clip;
    WebCore::PaintedWidgetsOfWRATHHandle m_border1;
    
  };


  class InlineFlowBox_FillLayers:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::InlineBox, InlineFlowBox_FillLayers>
  {
  public:
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::InlineBox> m_fill_layer;
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::InlineBox> m_next;
  };

  class InlineFlowBox_FillLayer:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::InlineBox, InlineFlowBox_FillLayer>
  {
  public:
    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_clip_h1_node;
    WebCore::PaintedWidgetsOfWRATHHandle m_h0, m_h1;
  };

}

namespace WebCore
{

void InlineFlowBox::readyWRATHWidgetFillLayers(PaintedWidgetsOfWRATHHandleT<InlineBox> &handle, 
                                               const PaintInfoOfWRATH &paintInfo, 
                                               const Color &c, const FillLayer *fillLayer, 
                                               int _tx, int _ty, int w, int h, CompositeOperator op)
{
  InlineFlowBox_FillLayers *d;
  d=InlineFlowBox_FillLayers::object(this, handle);

  ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);
  if(!fillLayer) {
    d->m_fill_layer.clear();
    d->m_next.clear();
    return;
  }

  readyWRATHWidgetFillLayers(d->m_next, paintInfo, c, fillLayer->next(), _tx, _ty, w, h, op);
  readyWRATHWidgetFillLayer(d->m_fill_layer, paintInfo, c, fillLayer, _tx, _ty, w, h, op);
}

void InlineFlowBox::readyWRATHWidgetFillLayer(PaintedWidgetsOfWRATHHandleT<InlineBox> &handle, 
                                              const PaintInfoOfWRATH &paintInfo, const Color &c, 
                                              const FillLayer *fillLayer, 
                                              int tx, int ty, int w, int h, CompositeOperator op)
{
    InlineFlowBox_FillLayer *d;

    d=InlineFlowBox_FillLayer::object(this, handle);
    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);
  

    StyleImage* img = fillLayer->image();
    bool hasFillImage = img && img->canRender(renderer()->style()->effectiveZoom());

    
    d->m_h0.visible(false);
    d->m_h1.visible(false);


    if ((!hasFillImage && !renderer()->style()->hasBorderRadius()) || (!prevLineBox() && !nextLineBox()) || !parent()) {
        d->m_h0.visible(true);
        boxModelObject()->readyWRATHWidgetFillLayerExtended(d->m_h0, paintInfo, c, fillLayer, tx, ty, w, h, 
                                                            BackgroundBleedNone, this, w, h, op);
    }
    else {
        d->m_h1.visible(true);
        // We have a fill image that spans multiple lines.
        // We need to adjust tx and ty by the width of all previous lines.
        // Think of background painting on inlines as though you had one long line, a single continuous
        // strip.  Even though that strip has been broken up across multiple lines, you still paint it
        // as though you had one single line.  This means each line has to pick up the background where
        // the previous line left off.
        int logicalOffsetOnLine = 0;
        int totalLogicalWidth;
        if (renderer()->style()->direction() == LTR) {
            for (InlineFlowBox* curr = prevLineBox(); curr; curr = curr->prevLineBox())
                logicalOffsetOnLine += curr->logicalWidth();
            totalLogicalWidth = logicalOffsetOnLine;
            for (InlineFlowBox* curr = this; curr; curr = curr->nextLineBox())
                totalLogicalWidth += curr->logicalWidth();
        } else {
            for (InlineFlowBox* curr = nextLineBox(); curr; curr = curr->nextLineBox())
                logicalOffsetOnLine += curr->logicalWidth();
            totalLogicalWidth = logicalOffsetOnLine;
            for (InlineFlowBox* curr = this; curr; curr = curr->prevLineBox())
                totalLogicalWidth += curr->logicalWidth();
        }
        int stripX = tx - (isHorizontal() ? logicalOffsetOnLine : 0);
        int stripY = ty - (isHorizontal() ? 0 : logicalOffsetOnLine);
        int stripWidth = isHorizontal() ? totalLogicalWidth : width();
        int stripHeight = isHorizontal() ? height() : totalLogicalWidth;
        

        paintInfo.wrath_context->push_node(d->m_clip_h1_node);
        ContextOfWRATH::set_clipping(d->m_clip_h1_node, IntRect(tx, ty, width(), height()) );
        boxModelObject()->readyWRATHWidgetFillLayerExtended(d->m_h1, paintInfo, c, fillLayer, stripX, stripY, 
                                                            stripWidth, stripHeight, BackgroundBleedNone, 
                                                            this, w, h, op);
        paintInfo.wrath_context->pop_node();
    }
}

void InlineFlowBox::readyWRATHWidgetBoxDecorations(PaintedWidgetsOfWRATHHandleT<InlineBox> &handle, 
                                                   PaintInfoOfWRATH &paintInfo, int tx, int ty)
{

    InlineFlowBox_BoxDecorations *d;
    d=InlineFlowBox_BoxDecorations::object(this, handle);
    
    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    d->m_box_shadow_normal.visible(false);
    d->m_box_shadow_inset.visible(false);
    d->m_fill_layers.visible(false);
    d->m_border0.visible(false);
    d->m_border1.visible(false);
    
    if(!paintInfo.shouldPaintWithinRoot(renderer()) || renderer()->style()->visibility() != VISIBLE || paintInfo.phase != PaintPhaseForeground)
      return;
   
    // Pixel snap background/border painting.
    IntRect frameRect = roundedFrameRect();
    int x = frameRect.x();
    int y = frameRect.y();
    int w = frameRect.width();
    int h = frameRect.height();

    // Constrain our background/border painting to the line top and bottom if necessary.
    bool noQuirksMode = renderer()->document()->inNoQuirksMode();
    if (!noQuirksMode && !hasTextChildren() && !(descendantsHaveSameLineHeightAndBaseline() && hasTextDescendants())) {
        RootInlineBox* rootBox = root();
        int& top = isHorizontal() ? y : x;
        int& logicalHeight = isHorizontal() ? h : w;
        int bottom = min(rootBox->lineBottom(), top + logicalHeight);
        top = max(rootBox->lineTop(), top);
        logicalHeight = bottom - top;
    }
    
    // Move x/y to our coordinates.
    IntRect localRect(x, y, w, h);
    flipForWritingMode(localRect);
    tx += localRect.x();
    ty += localRect.y();
    

    RenderStyle* styleToUse = renderer()->style(m_firstLine);
    
    if ((!parent() && m_firstLine && styleToUse != renderer()->style()) || (parent() && renderer()->hasBoxDecorations())) {
        // Shadow comes first and is behind the background and border.
        d->m_box_shadow_normal.visible(true);
        readyWRATHWidgetBoxShadow(d->m_box_shadow_normal, paintInfo.wrath_context, styleToUse, Normal, tx, ty, w, h);
        
        Color c = styleToUse->visitedDependentColor(CSSPropertyBackgroundColor);
        d->m_fill_layers.visible(true);
        readyWRATHWidgetFillLayers(d->m_fill_layers, paintInfo, c, 
                                   styleToUse->backgroundLayers(), tx, ty, w, h);
        d->m_box_shadow_inset.visible(true);
        readyWRATHWidgetBoxShadow(d->m_box_shadow_inset, paintInfo.wrath_context, styleToUse, Inset, tx, ty, w, h);

        if (parent() && renderer()->style()->hasBorder()) {
            StyleImage* borderImage = renderer()->style()->borderImage().image();
            bool hasBorderImage = borderImage && borderImage->canRender(styleToUse->effectiveZoom());
            if (hasBorderImage && !borderImage->isLoaded()) {
                return; 
            }

            if (!hasBorderImage || (!prevLineBox() && !nextLineBox())) {
                d->m_border0.visible(true);
                boxModelObject()->readyWRATHWidgetBorder(d->m_border0, paintInfo.wrath_context,
                                                         tx, ty, w, h, renderer()->style(), 
                                                         BackgroundBleedNone, includeLogicalLeftEdge(), 
                                                         includeLogicalRightEdge());
            }
            else {
                int logicalOffsetOnLine = 0;
                for (InlineFlowBox* curr = prevLineBox(); curr; curr = curr->prevLineBox())
                    logicalOffsetOnLine += curr->logicalWidth();
                int totalLogicalWidth = logicalOffsetOnLine;
                for (InlineFlowBox* curr = this; curr; curr = curr->nextLineBox())
                    totalLogicalWidth += curr->logicalWidth();
                int stripX = tx - (isHorizontal() ? logicalOffsetOnLine : 0);
                int stripY = ty - (isHorizontal() ? 0 : logicalOffsetOnLine);
                int stripWidth = isHorizontal() ? totalLogicalWidth : w;
                int stripHeight = isHorizontal() ? h : totalLogicalWidth;

                

                paintInfo.wrath_context->push_node(d->m_border1_clip);
                ContextOfWRATH::set_clipping(d->m_border1_clip, IntRect(tx, ty, w, h));
                
                d->m_border1.visible(true);
                boxModelObject()->readyWRATHWidgetBorder(d->m_border1, paintInfo.wrath_context, 
                                                         stripX, stripY, 
                                                         stripWidth, stripHeight, renderer()->style());
                paintInfo.wrath_context->pop_node();
            }
        }
    }

}


void InlineFlowBox::readyWRATHWidgetMask(PaintedWidgetsOfWRATHHandleT<InlineBox> &handle, 
                                         PaintInfoOfWRATH&, int tx, int ty)
{
  /*
    TODO:
   */
}

void InlineFlowBox::readyWRATHWidgetBoxShadow(PaintedWidgetsOfWRATHHandleT<InlineBox> &handle, 
                                              ContextOfWRATH *wrath_context,
                                              RenderStyle *s, ShadowStyle shadowStyle, 
                                              int tx, int ty, int w, int h)
{
  InlineFlowBox_BoxShadow *d;

  d=InlineFlowBox_BoxShadow::object(this, handle);
  ContextOfWRATH::AutoPushNode autoPushRoot(wrath_context, d->m_root_node);

  d->m_branch0.visible(false);
  d->m_branch1.visible(false);

  if ((!prevLineBox() && !nextLineBox()) || !parent())
    {
      d->m_branch0.visible(true);
      boxModelObject()->readyWRATHWidgetBoxShadow(d->m_branch0, wrath_context, tx, ty, w, h, s, shadowStyle);
    }
  else 
    {
      d->m_branch1.visible(true);
      boxModelObject()->readyWRATHWidgetBoxShadow(d->m_branch1, wrath_context, tx, ty, w, h, s, shadowStyle, 
                                                  includeLogicalLeftEdge(), includeLogicalRightEdge());
    }
}


void InlineFlowBox::readyWRATHWidgets(PaintedWidgetsOfWRATHHandleT<InlineBox> &handle, 
                                      PaintInfoOfWRATH &paintInfo, int tx, int ty, int lineTop, int lineBottom)
{
  InlineFlowBox_Widgets *d;

  d=InlineFlowBox_Widgets::object(this, handle);

  ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);
  ContextOfWRATH::AutoPushNode autoPushSkip(paintInfo.wrath_context, d->m_skip);

  IntRect overflowRect(visualOverflowRect(lineTop, lineBottom));
  overflowRect.inflate(renderer()->maximalOutlineSize(paintInfo.phase));
  flipForWritingMode(overflowRect);
  overflowRect.move(tx, ty);
    
  
  /**/
  if (!paintInfo.rect.intersects(overflowRect)) {
    d->m_skip.widget()->visible(false);
    return;
  }
  /**/

  d->m_skip.widget()->visible(true);
  d->m_children.hideEachObject();

  if (paintInfo.phase != PaintPhaseChildOutlines) {
    if (paintInfo.phase == PaintPhaseOutline || paintInfo.phase == PaintPhaseSelfOutline) {
      if (renderer()->style()->visibility() == VISIBLE && renderer()->hasOutline() && !isRootInlineBox()) {
        RenderInline* inlineFlow = toRenderInline(renderer());
        
        RenderBlock* cb = 0;
        bool containingBlockPaintsContinuationOutline = inlineFlow->continuation() || inlineFlow->isInlineElementContinuation();
        if (containingBlockPaintsContinuationOutline) {           
          RenderBlock* enclosingAnonymousBlock = renderer()->containingBlock();
          if (!enclosingAnonymousBlock->isAnonymousBlock())
            containingBlockPaintsContinuationOutline = false;
          else {
            cb = enclosingAnonymousBlock->containingBlock();
            for (RenderBoxModelObject* box = boxModelObject(); box != cb; box = box->parent()->enclosingBoxModelObject()) {
              if (box->hasSelfPaintingLayer()) {
                containingBlockPaintsContinuationOutline = false;
                break;
              }
            }
          }
        }
        
        if (containingBlockPaintsContinuationOutline) {
          // Add ourselves to the containing block of the entire continuation so that it can
          // paint us atomically.
          cb->addContinuationWithOutline(toRenderInline(renderer()->node()->renderer()));
        } else if (!inlineFlow->isInlineElementContinuation())
          paintInfo.outlineObjects->add(inlineFlow);
      }
    } else if (paintInfo.phase == PaintPhaseMask) {
      readyWRATHWidgetMask(d->m_mask, paintInfo, tx, ty);
      return;
    } else {
      // Paint our background, border and box-shadow.
      readyWRATHWidgetBoxDecorations(d->m_box_decorations, paintInfo, tx, ty);
    }
  }
  
  if (paintInfo.phase == PaintPhaseMask)
    return;
  
  PaintPhase paintPhase = paintInfo.phase == PaintPhaseChildOutlines ? PaintPhaseOutline : paintInfo.phase;
  PaintInfoOfWRATH childInfo(paintInfo);
  childInfo.phase = paintPhase;
  childInfo.updatePaintingRootForChildren(renderer());
  
  // Paint our children.
  if (paintPhase != PaintPhaseSelfOutline) {

    for (InlineBox* curr = firstChild(); curr; curr = curr->nextOnLine()) {
      if (curr->renderer()->isText() || !curr->boxModelObject()->hasSelfPaintingLayer()) {
        PaintedWidgetsOfWRATHHandleT<InlineBox> &child_handle(d->m_children.getHandle(curr));

        child_handle.visible(true);
        curr->readyWRATHWidgets(child_handle, childInfo, tx, ty, lineTop, lineBottom);
      }
    }
  }
  d->m_children.removeNonVisibleHandles();
}

}
#endif

