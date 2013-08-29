#include "config.h"
#include "RenderView.h"

#include "Document.h"
#include "Element.h"
#include "FloatQuad.h"
#include "Frame.h"
#include "FrameView.h"
#include "GraphicsContext.h"
#include "HTMLFrameOwnerElement.h"
#include "HitTestResult.h"
#include "RenderLayer.h"
#include "RenderSelectionInfo.h"
#include "RenderWidget.h"
#include "RenderWidgetProtector.h"
#include "TransformState.h"

#if USE(ACCELERATED_COMPOSITING)
#include "RenderLayerCompositor.h"
#endif

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
#include "WRATHPaintHelpers.h"
namespace {

  class RenderView_WRATHWidgetBoxDecorations:
      public WebCore::PaintedWidgetsOfWRATH<RenderView_WRATHWidgetBoxDecorations>
  {
  public:
    WebCore::FilledIntRectOfWRATH m_rect, m_clear_rect;
  };
}

namespace WebCore {

static inline bool isComposited(RenderObject* object)
{
    return object->hasLayer() && toRenderBoxModelObject(object)->layer()->isComposited();
}

static inline bool rendererObscuresBackground(RenderObject* object)
{
    return object && object->style()->visibility() == VISIBLE
        && object->style()->opacity() == 1
        && !object->style()->hasTransform()
        && !isComposited(object);
}
    
void RenderView::readyWRATHWidgets(PaintedWidgetsOfWRATHHandle &handle,
                                     PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
  // If we ever require layout but receive a paint anyway, something has gone horribly wrong.
  ASSERT(!needsLayout());

  /*
    ok to use handle directly since it is a direct call to readyWRATHWidgetsObject
   */
  readyWRATHWidgetObject(handle, paintInfo, tx, ty);
}

void RenderView::readyWRATHWidgetBoxDecorations(PaintedWidgetsOfWRATHHandle &handle,
                                                PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
  
    RenderView_WRATHWidgetBoxDecorations *d;

    d=RenderView_WRATHWidgetBoxDecorations::object(this, handle);
    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    if(d->m_rect.widget()) {
      d->m_rect.widget()->visible(false);
    }

    if(d->m_clear_rect.widget()) {
      d->m_clear_rect.widget()->visible(false);
    }

   // Check to see if we are enclosed by a layer that requires complex painting rules.  If so, we cannot blit
    // when scrolling, and we need to use slow repaints.  Examples of layers that require this are transparent layers,
    // layers with reflections, or transformed layers.
    // FIXME: This needs to be dynamic.  We should be able to go back to blitting if we ever stop being inside
    // a transform, transparency layer, etc.
    Element* elt;
    for (elt = document()->ownerElement(); view() && elt && elt->renderer(); elt = elt->document()->ownerElement()) {
        RenderLayer* layer = elt->renderer()->enclosingLayer();
        if (layer->requiresSlowRepaints()) {
            frameView()->setUseSlowRepaints();
            break;
        }

#if USE(ACCELERATED_COMPOSITING)
        if (RenderLayer* compositingLayer = layer->enclosingCompositingLayer()) {
            if (!compositingLayer->backing()->paintingGoesToWindow()) {
                frameView()->setUseSlowRepaints();
                break;
            }
        }
#endif
    }

    if (document()->ownerElement() || !view())
        return;

    bool rootFillsViewport = false;
    Node* documentElement = document()->documentElement();
    if (RenderObject* rootRenderer = documentElement ? documentElement->renderer() : 0) {
        // The document element's renderer is currently forced to be a block, but may not always be.
        RenderBox* rootBox = rootRenderer->isBox() ? toRenderBox(rootRenderer) : 0;
        rootFillsViewport = rootBox && !rootBox->x() && !rootBox->y() && rootBox->width() >= width() && rootBox->height() >= height();
    }

    float pageScaleFactor = 1;
    if (Frame* frame = m_frameView->frame())
        pageScaleFactor = frame->pageScaleFactor();

    // If painting will entirely fill the view, no need to fill the background.
    if (rootFillsViewport && rendererObscuresBackground(firstChild()) && pageScaleFactor >= 1)
        return;

    // This code typically only executes if the root element's visibility has been set to hidden,
    // if there is a transform on the <html>, or if there is a page scale factor less than 1.
    // Only fill with the base background color (typically white) if we're the root document, 
    // since iframes/frames with no background in the child document should show the parent's background.
    if (frameView()->isTransparent()) // FIXME: This needs to be dynamic.  We should be able to go back to blitting if we ever stop being transparent.
        frameView()->setUseSlowRepaints(); // The parent must show behind the child.
    else {
        Color baseColor = frameView()->baseBackgroundColor();

        
        if (baseColor.alpha() > 0) {
            //CompositeOperator previousOperator = paintInfo.context->compositeOperation();
            // paintInfo.context->setCompositeOperation(CompositeCopy);
            //paintInfo.context->fillRect(paintInfo.rect, baseColor, style()->colorSpace());
            //paintInfo.context->setCompositeOperation(previousOperator);
          d->m_rect.update(paintInfo.wrath_context, paintInfo.rect, baseColor, CompositeCopy);
	  if(d->m_rect.widget()) {
	    d->m_rect.widget()->visible(true);
	  }
        } else {
          d->m_clear_rect.update(paintInfo.wrath_context, paintInfo.rect);
	  if(d->m_clear_rect.widget()) {
	    d->m_clear_rect.widget()->visible(true);
	  }
	}


    }
}
}
#endif
