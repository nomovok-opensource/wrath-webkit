#include "config.h"
#include "RenderLayer.h"

#include "ColumnInfo.h"
#include "CSSPropertyNames.h"
#include "CSSStyleDeclaration.h"
#include "CSSStyleSelector.h"
#include "Chrome.h"
#include "Document.h"
#include "EventHandler.h"
#include "EventQueue.h"
#include "FloatPoint3D.h"
#include "FloatRect.h"
#include "FocusController.h"
#include "Frame.h"
#include "FrameTree.h"
#include "FrameView.h"
#include "Gradient.h"
#include "GraphicsContext.h"
#include "HTMLFrameOwnerElement.h"
#include "HTMLNames.h"
#include "HitTestRequest.h"
#include "HitTestResult.h"
#include "OverflowEvent.h"
#include "OverlapTestRequestClient.h"
#include "Page.h"
#include "PlatformMouseEvent.h"
#include "RenderArena.h"
#include "RenderInline.h"
#include "RenderMarquee.h"
#include "RenderReplica.h"
#include "RenderScrollbar.h"
#include "RenderScrollbarPart.h"
#include "RenderTheme.h"
#include "RenderTreeAsText.h"
#include "RenderView.h"
#include "ScaleTransformOperation.h"
#include "Scrollbar.h"
#include "ScrollbarTheme.h"
#include "SelectionController.h"
#include "TextStream.h"
#include "TransformState.h"
#include "TransformationMatrix.h"
#include "TranslateTransformOperation.h"
#include <wtf/StdLibExtras.h>
#include <wtf/UnusedParam.h>
#include <wtf/text/CString.h>

#if USE(ACCELERATED_COMPOSITING)
#include "RenderLayerBacking.h"
#include "RenderLayerCompositor.h"
#endif

#if ENABLE(SVG)
#include "SVGNames.h"
#endif

#define MIN_INTERSECT_FOR_REVEAL 32

using namespace std;

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
#include "HierarchyOfHandlesOfWRATH.h"
#include "ostream_utility.hpp"
#include "WRATHPaintHelpers.h"

namespace {
  class AutoIncrementDecrement
  {
  public:
    explicit 
    AutoIncrementDecrement(int &v):
      m_v(v)
    {
      ++m_v;
    }

    ~AutoIncrementDecrement()
    {
      --m_v;
    }
  private:
    int &m_v;
  };

  class RenderLayerOfWRATH:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::RenderLayer, RenderLayerOfWRATH>
  {
  public:

    static
    bool
    is_zero_ish(float v)
    {
      return std::abs(v)<0.01f;
    }

    static
    bool
    is_one_ish(float v)
    {
      return is_zero_ish(v-1.0f);
    }

    static
    bool
    is_same_ish(float a, float b)
    {
      return std::abs(a-b) <= 0.01f * std::min( std::abs(a), std::abs(b));
    }
    
    static
    bool
    transformation_can_be_done_with_scale_translate(const float4x4 &matrix,
                                                    WRATHScaleTranslate &out_scale_translate)
    {
      /*
        we require that 
          (0,0)=(1,1)
          (0,1)=(0,2)=0
          (1,0)=(1,2)=0
          (3,0)=(3,1)=(3,2)=0
          (3,3)=1.
       */
      bool return_value;

      return_value=is_zero_ish( matrix(0,1))
        and is_zero_ish( matrix(0,2))
        and is_zero_ish( matrix(1,0))
        and is_zero_ish( matrix(1,2))
        and is_zero_ish( matrix(3,0))
        and is_zero_ish( matrix(3,1))
        and is_zero_ish( matrix(3,2))
        and matrix(3,3) > 0.0f
        and matrix(0,0) > 0.0f
        and is_same_ish(matrix(0,0), matrix(1,1));

      if(return_value) {
        out_scale_translate.scale(matrix(0,0)/matrix(3,3));
        out_scale_translate.translation_x(matrix(0,3)/matrix(3,3));
        out_scale_translate.translation_y(matrix(1,3)/matrix(3,3));
      }

      return return_value;

    }

    void
    hide_all(void)
    {
       m_node.widget()->visible(false);
       m_reflection.visible(false);
       m_rotation.visible(false);
       for(int i=0; i<WebCore::NumberPaintPhases; ++i) {
         m_per_pass[i].visible(false);
       }
    
       if(m_fake_canvas.widget()) {
         m_fake_canvas.widget()->visible(false);
       }
    
       if(m_canvas.widget()) {
         m_canvas.widget()->visible(false);
       }
    
       if(m_transparent_canvas.widget()) {
         m_transparent_canvas.widget()->visible(false);
       }

       m_neg_list.visible(false);
       m_pos_list.visible(false);
       m_flow_list.visible(false);
       m_overflow_controls.visible(false);
    }


    WebCore::ContextOfWRATH::DrawnCanvas::AutoDelete m_canvas;
    WebCore::ContextOfWRATH::DrawnCanvas::AutoDelete m_transparent_canvas;
    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_fake_canvas;
    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_canvas_clipping;
    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_node;
    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_damageRect_node;
    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_clipRect_node;
    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_outlineRect_node;
    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_mask_node;
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::RenderLayer> m_reflection;

    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::RenderLayer> m_rotation;
    vecN<WebCore::PaintedWidgetsOfWRATHHandle, WebCore::NumberPaintPhases> m_per_pass;

    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::RenderLayer> m_neg_list, m_pos_list, m_flow_list;
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::RenderLayer> m_begin_end_transparency_layer;

    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_overflow_node;
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::RenderLayer> m_overflow_controls;
  };

  class RenderLayerOfWRATH_Resizer:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::RenderLayer, RenderLayerOfWRATH_Resizer>
  {
  public:

    void
    hide_all(void)
    {
      m_resizer.visible(false);
    }

    WebCore::PaintedWidgetsOfWRATHHandle m_resizer;
  };

  class RenderLayerOfWRATH_ScrollCorner:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::RenderLayer, RenderLayerOfWRATH_ScrollCorner>
  {
  public:

    void
    hide_all(void)
    {
      m_scroll_corner.visible(false);
      if(m_rect_item.widget())
        {
          m_rect_item.widget()->visible(false);
        }
    }

    WebCore::PaintedWidgetsOfWRATHHandle m_scroll_corner;
    WebCore::FilledIntRectOfWRATH m_rect_item;
  };


  class RenderLayerOfWRATH_OverlayScrollbars:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::RenderLayer, RenderLayerOfWRATH_OverlayScrollbars>
  {
  public:

    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::RenderLayer> m_layer;
  };
  
  class RenderLayerOfWRATH_OverFlowControls:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::RenderLayer, RenderLayerOfWRATH_OverFlowControls>
  {
  public:

    void
    hide_all(void)
    {
      m_scroll_corner.visible(false);
      m_resizer.visible(false);
      m_vbar.visible(false);
      m_hbar.visible(false);
    }
   
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::RenderLayer> m_scroll_corner;
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::RenderLayer> m_resizer;
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::Widget> m_vbar, m_hbar;
  };

  class RenderLayerOfWRATH_List:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::RenderLayer, RenderLayerOfWRATH_List>
  {
  public:

    WebCore::HierarchyOfHandlesOfWRATH<WebCore::RenderLayer, WebCore::RenderLayer> m_list;
  };
}

namespace WebCore {

static IntRect cornerRect(const RenderLayer* layer, const IntRect& bounds)
{
    int horizontalThickness;
    int verticalThickness;
    if (!layer->verticalScrollbar() && !layer->horizontalScrollbar()) {
        // FIXME: This isn't right.  We need to know the thickness of custom scrollbars
        // even when they don't exist in order to set the resizer square size properly.
        horizontalThickness = ScrollbarTheme::nativeTheme()->scrollbarThickness();
        verticalThickness = horizontalThickness;
    } else if (layer->verticalScrollbar() && !layer->horizontalScrollbar()) {
        horizontalThickness = layer->verticalScrollbar()->width();
        verticalThickness = horizontalThickness;
    } else if (layer->horizontalScrollbar() && !layer->verticalScrollbar()) {
        verticalThickness = layer->horizontalScrollbar()->height();
        horizontalThickness = verticalThickness;
    } else {
        horizontalThickness = layer->verticalScrollbar()->width();
        verticalThickness = layer->horizontalScrollbar()->height();
    }
    return IntRect(bounds.maxX() - horizontalThickness - layer->renderer()->style()->borderRightWidth(), 
                   bounds.maxY() - verticalThickness - layer->renderer()->style()->borderBottomWidth(),
                   horizontalThickness, verticalThickness);
}

static IntRect transparencyClipBox(const RenderLayer* l, const RenderLayer* rootLayer, PaintBehavior paintBehavior);

static void expandClipRectForDescendantsAndReflection(IntRect& clipRect, const RenderLayer* l, const RenderLayer* rootLayer, PaintBehavior paintBehavior)
{
    // If we have a mask, then the clip is limited to the border box area (and there is
    // no need to examine child layers).
    if (!l->renderer()->hasMask()) {
        // Note: we don't have to walk z-order lists since transparent elements always establish
        // a stacking context.  This means we can just walk the layer tree directly.
        for (RenderLayer* curr = l->firstChild(); curr; curr = curr->nextSibling()) {
            if (!l->reflection() || l->reflectionLayer() != curr)
                clipRect.unite(transparencyClipBox(curr, rootLayer, paintBehavior));
        }
    }

    // If we have a reflection, then we need to account for that when we push the clip.  Reflect our entire
    // current transparencyClipBox to catch all child layers.
    // FIXME: Accelerated compositing will eventually want to do something smart here to avoid incorporating this
    // size into the parent layer.
    if (l->renderer()->hasReflection()) {
        int deltaX = 0;
        int deltaY = 0;
        l->convertToLayerCoords(rootLayer, deltaX, deltaY);
        clipRect.move(-deltaX, -deltaY);
        clipRect.unite(l->renderBox()->reflectedRect(clipRect));
        clipRect.move(deltaX, deltaY);
    }
}

static IntRect transparencyClipBox(const RenderLayer* l, const RenderLayer* rootLayer, PaintBehavior paintBehavior)
{
    // FIXME: Although this function completely ignores CSS-imposed clipping, we did already intersect with the
    // paintDirtyRect, and that should cut down on the amount we have to paint.  Still it
    // would be better to respect clips.
    
    if (rootLayer != l && l->paintsWithTransform(paintBehavior)) {
        // The best we can do here is to use enclosed bounding boxes to establish a "fuzzy" enough clip to encompass
        // the transformed layer and all of its children.
        int x = 0;
        int y = 0;
        l->convertToLayerCoords(rootLayer, x, y);

        TransformationMatrix transform;
        transform.translate(x, y);
        transform = transform * *l->transform();

        IntRect clipRect = l->boundingBox(l);
        expandClipRectForDescendantsAndReflection(clipRect, l, l, paintBehavior);
        return transform.mapRect(clipRect);
    }
    
    IntRect clipRect = l->boundingBox(rootLayer);
    expandClipRectForDescendantsAndReflection(clipRect, l, rootLayer, paintBehavior);
    return clipRect;
}

static void performOverlapTests(OverlapTestRequestMap& overlapTestRequests, const RenderLayer* rootLayer, const RenderLayer* layer)
{
    Vector<OverlapTestRequestClient*> overlappedRequestClients;
    OverlapTestRequestMap::iterator end = overlapTestRequests.end();
    IntRect boundingBox = layer->boundingBox(rootLayer);
    for (OverlapTestRequestMap::iterator it = overlapTestRequests.begin(); it != end; ++it) {
        if (!boundingBox.intersects(it->second))
            continue;

        it->first->setOverlapTestResult(true);
        overlappedRequestClients.append(it->first);
    }
    for (size_t i = 0; i < overlappedRequestClients.size(); ++i)
        overlapTestRequests.remove(overlappedRequestClients[i]);
}

static IntRect resizerCornerRect(const RenderLayer* layer, const IntRect& bounds)
{
    ASSERT(layer->renderer()->isBox());
    if (layer->renderer()->style()->resize() == RESIZE_NONE)
        return IntRect();
    return cornerRect(layer, bounds);
}

void RenderLayer::readyWRATHOverlayScrollbars(ContextOfWRATH *ctx,
                                              PaintedWidgetsOfWRATHHandleT<RenderLayer> &handle,
                                              const IntRect& damageRect, PaintBehavior paintBehavior, 
                                              RenderObject* paintingRoot)
{
    RenderLayerOfWRATH_OverlayScrollbars *d;

    d=RenderLayerOfWRATH_OverlayScrollbars::object(this, handle);
    ContextOfWRATH::AutoPushNode autoPushRoot(ctx, d->m_root_node);

    d->m_layer.visible(false);
    if (!containsDirtyOverlayScrollbars())
        return;

    d->m_layer.visible(true);
    readyWRATHWidgetsLayer(d->m_layer, this, ctx, damageRect, paintBehavior, paintingRoot, 0,
                           PaintLayerHaveTransparency | PaintLayerTemporaryClipRects | PaintLayerPaintingOverlayScrollbars);
    
    setContainsDirtyOverlayScrollbars(false);
}


void RenderLayer::readyWRATHOverflowControls(ContextOfWRATH *ctx,
                                             PaintedWidgetsOfWRATHHandleT<RenderLayer> &handle,
                                             int tx, int ty, const IntRect& damageRect, 
                                             bool paintingOverlayControls)
{
  
    RenderLayerOfWRATH_OverFlowControls *d;

    d=RenderLayerOfWRATH_OverFlowControls::object(this, handle);
    ContextOfWRATH::AutoPushNode autoPushRoot(ctx, d->m_root_node);
    
    d->hide_all();

    // Don't do anything if we have no overflow.
    if (!renderer()->hasOverflowClip())
        return;

    // Overlay scrollbars paint in a second pass through the layer tree so that they will paint
    // on top of everything else. If this is the normal painting pass, paintingOverlayControls
    // will be false, and we should just tell the root layer that there are overlay scrollbars
    // that need to be painted. That will cause the second pass through the layer tree to run,
    // and we'll paint the scrollbars then. In the meantime, cache tx and ty so that the 
    // second pass doesn't need to re-enter the RenderTree to get it right.
    if (hasOverlayScrollbars() && !paintingOverlayControls) {
        RenderView* renderView = renderer()->view();
        renderView->layer()->setContainsDirtyOverlayScrollbars(true);
        setCachedOverlayScrollbarOffset(IntPoint(tx, ty));
        renderView->frameView()->setContainsScrollableAreaWithOverlayScrollbars(true);
        return;
    }

    int offsetX = tx;
    int offsetY = ty;
    if (paintingOverlayControls) {
        offsetX = m_cachedOverlayScrollbarOffset.x();
        offsetY = m_cachedOverlayScrollbarOffset.y();
    }

    // Move the scrollbar widgets if necessary.  We normally move and resize widgets during layout, but sometimes
    // widgets can move without layout occurring (most notably when you scroll a document that
    // contains fixed positioned elements).
    positionOverflowControls(offsetX, offsetY);

     // Now that we're sure the scrollbars are in the right place, paint them.
    if (m_hBar
#if USE(ACCELERATED_COMPOSITING)
        && !layerForHorizontalScrollbar()
#endif
         ) {
      d->m_hbar.visible(true);
      m_hBar->readyWRATHWidgets(ctx, d->m_hbar);
      
    }


    if (m_vBar
#if USE(ACCELERATED_COMPOSITING)
        && !layerForVerticalScrollbar()
#endif
        ) {
      d->m_vbar.visible(true);
      m_vBar->readyWRATHWidgets(ctx, d->m_vbar);
    }
#if USE(ACCELERATED_COMPOSITING)
    if (layerForScrollCorner())
        return;
#endif

    // We fill our scroll corner with white if we have a scrollbar that doesn't run all the way up to the
    // edge of the box.
    d->m_scroll_corner.visible(true);
    readyWRATHScrollCorner(ctx, d->m_scroll_corner, offsetX, offsetY, damageRect);
    
    // Paint our resizer last, since it sits on top of the scroll corner.
    d->m_resizer.visible(true);
    readyWRATHResizer(ctx, d->m_resizer, offsetX, offsetY, damageRect);
}

void RenderLayer::readyWRATHScrollCorner(ContextOfWRATH *ctx, PaintedWidgetsOfWRATHHandleT<RenderLayer> &handle,
                                         int tx, int ty, const IntRect& damageRect)
{

    RenderLayerOfWRATH_ScrollCorner *d;

    d=RenderLayerOfWRATH_ScrollCorner::object(this, handle);
    ContextOfWRATH::AutoPushNode autoPushRoot(ctx, d->m_root_node);
  
    d->hide_all();

    RenderBox* box = renderBox();
    ASSERT(box);

    IntRect cornerRect = scrollCornerRect();
    IntRect absRect = IntRect(cornerRect.x() + tx, cornerRect.y() + ty, cornerRect.width(), cornerRect.height());
    if (!absRect.intersects(damageRect))
        return;

    /*
    if (context->updatingControlTints()) {
        updateScrollCornerStyle();
        return;
    }
    */

    if (m_scrollCorner) {
        d->m_scroll_corner.visible(true);
        m_scrollCorner->readyWRATHWidgetIntoRect(d->m_scroll_corner, ctx, tx, ty, absRect);
        return;
    }

    // We don't want to paint white if we have overlay scrollbars, since we need
    // to see what is behind it.
    if (!hasOverlayScrollbars()) {
      d->m_rect_item.update(ctx, absRect, Color::white, CompositeSourceOver);
      if(d->m_rect_item.widget()) {
	d->m_rect_item.widget()->visible(true);
      }
    }
}

static IntRect resizerCornerRect(const RenderLayer* layer, const IntRect& bounds);
void RenderLayer::readyWRATHResizer(ContextOfWRATH *ctx, PaintedWidgetsOfWRATHHandleT<RenderLayer> &handle,
                                    int tx, int ty, const IntRect& damageRect)
{
    RenderLayerOfWRATH_Resizer *d;

    d=RenderLayerOfWRATH_Resizer::object(this, handle);
    ContextOfWRATH::AutoPushNode autoPushRoot(ctx, d->m_root_node);
  
    d->hide_all();


    if (renderer()->style()->resize() == RESIZE_NONE)
        return;

    RenderBox* box = renderBox();
    ASSERT(box);

    IntRect cornerRect = resizerCornerRect(this, box->borderBoxRect());
    IntRect absRect = IntRect(cornerRect.x() + tx, cornerRect.y() + ty, cornerRect.width(), cornerRect.height());
    if (!absRect.intersects(damageRect))
        return;

    /*
    if (context->updatingControlTints()) {
        updateResizerStyle();
        return;
    }
    */
    
    if (m_resizer) {
        d->m_resizer.visible(true);
        m_resizer->readyWRATHWidgetIntoRect(d->m_resizer, ctx, tx, ty, absRect);
        return;
    }


    /***********************
       [WRATH-TODO]:



    // Paint the resizer control.
    DEFINE_STATIC_LOCAL(RefPtr<Image>, resizeCornerImage, (Image::loadPlatformResource("textAreaResizeCorner")));
    IntPoint imagePoint(absRect.maxX() - resizeCornerImage->width(), absRect.maxY() - resizeCornerImage->height());
    context->drawImage(resizeCornerImage.get(), box->style()->colorSpace(), imagePoint);

    // Draw a frame around the resizer (1px grey line) if there are any scrollbars present.
    // Clipping will exclude the right and bottom edges of this frame.
    if (!hasOverlayScrollbars() && (m_vBar || m_hBar)) {
        GraphicsContextStateSaver stateSaver(*context);
        context->clip(absRect);
        IntRect largerCorner = absRect;
        largerCorner.setSize(IntSize(largerCorner.width() + 1, largerCorner.height() + 1));
        context->setStrokeColor(Color(makeRGB(217, 217, 217)), ColorSpaceDeviceRGB);
        context->setStrokeThickness(1.0f);
        context->setFillColor(Color::transparent, ColorSpaceDeviceRGB);
        context->drawRect(largerCorner);
    }
    *********************************/

}



void RenderLayer::readyWRATHWidgetsList(PaintedWidgetsOfWRATHHandleT<RenderLayer> &handle,
                                        Vector<RenderLayer*>* list, 
                                        RenderLayer* rootLayer, 
                                        ContextOfWRATH *p, 
                                        const IntRect& paintDirtyRect, 
                                        PaintBehavior paintBehavior,
                                        RenderObject* paintingRoot, 
                                        OverlapTestRequestMap *overlapTestRequests,
                                        PaintLayerFlags paintFlags)
{
  RenderLayerOfWRATH_List *d;

  d=RenderLayerOfWRATH_List::object(this, handle);
  ContextOfWRATH::AutoPushNode autoPushRoot(p, d->m_root_node);

  d->m_list.hideEachObject();

  if(list) {
    /*
      TODO: examine the flag: list->at(i)->isPaginated()
      and if true, then paint the child into columns..
      the WRATH analgoue of paintPaginatedChildLayer
     */
    for (size_t i = 0; i < list->size(); ++i) {
      RenderLayer *layer(list->at(i));
      PaintedWidgetsOfWRATHHandleT<RenderLayer> &layer_handle(d->m_list.getHandle(layer));

      if(list->at(i)->isPaginated()) {
        std::cout << __PRETTY_FUNCTION__ << ", warning: paginated list element\n";
      }

      layer_handle.visible(true);
      layer->readyWRATHWidgetsLayer(layer_handle,
                                    rootLayer, p,  
                                    paintDirtyRect, paintBehavior, paintingRoot, 
                                    overlapTestRequests, paintFlags);
    }
  }
  d->m_list.removeNonVisibleHandles();
}




void RenderLayer::readyWRATHWidgets(ContextOfWRATH *p,
                                    PaintedWidgetsOfWRATHHandleT<RenderLayer> &handle,
                                    const IntRect& paintDirtyRect, PaintBehavior paintBehavior,
                                    RenderObject* paintingRoot)
{
  OverlapTestRequestMap overlapTestRequests;

  readyWRATHWidgetsLayer(handle, this, p, paintDirtyRect, paintBehavior, paintingRoot, &overlapTestRequests);

  OverlapTestRequestMap::iterator end = overlapTestRequests.end();
  for (OverlapTestRequestMap::iterator it = overlapTestRequests.begin(); it != end; ++it)
    it->first->setOverlapTestResult(false);
}

static IntRect transparencyClipBox(const RenderLayer* l, const RenderLayer* rootLayer, PaintBehavior paintBehavior);

void RenderLayer::readyWRATHWidgetsLayer(PaintedWidgetsOfWRATHHandleT<RenderLayer> &handle,
                                         RenderLayer* rootLayer, 
                                         ContextOfWRATH *p, 
                                         const IntRect& paintDirtyRect, PaintBehavior paintBehavior, 
                                         RenderObject* paintingRoot, 
                                         OverlapTestRequestMap* overlapTestRequests,
                                         PaintLayerFlags paintFlags)
{
  /*
    Let the hacking begin! We are going to essentially just
    copy the function paintLayer but oh-so slightly change
    it to use WRATH style drawing jazz.
   */
    RenderLayerOfWRATH *d;
    d=RenderLayerOfWRATH::object(this, handle);

    ContextOfWRATH::AutoPushNode auto_push_root(p, d->m_root_node);
    ContextOfWRATH::AutoPushNode auto_push(p, d->m_node);


    d->hide_all();

   

    
    
    if (renderer()->document()->didLayoutWithPendingStylesheets() 
        && !renderer()->isRenderView() 
        && !renderer()->isRoot()) {
      
      return;
    }

    if (!renderer()->opacity()) {
      return;
    }

    d->m_node.widget()->visible(true);
    ContextOfWRATH::set_clipping(d->m_node, paintDirtyRect);
    
    /*
      in WRATH, we do not need to force the RenderLayer to be
      drawn in a seperate pass if it is composited, it makes
      no difference, thus rather than using paintsWithTransparency(PaintBehavior),
      we need only check if it is transparent..
     */
    if (isTransparent() /*paintsWithTransparency(paintBehavior)*/) {
        paintFlags |= PaintLayerHaveTransparency;
    }
        
    if (paintsWithTransform(paintBehavior)&& !(paintFlags & PaintLayerAppliedTransform)) {

        TransformationMatrix layerTransform = renderableTransform(paintBehavior);
        if (!layerTransform.isInvertible()) {
          d->m_node.widget()->visible(false);
          return;
        }
       
        // Adjust the transform such that the renderer's upper left corner will paint at (0,0) in user space.
        // This involves subtracting out the position of the layer in our current coordinate space.
        int x = 0;
        int y = 0;
        convertToLayerCoords(rootLayer, x, y);
        TransformationMatrix transform(layerTransform);
        transform.translateRight(x, y);

        IntRect clipRect = paintDirtyRect;
        if (parent()) {
          clipRect = backgroundClipRect(rootLayer, paintFlags & PaintLayerTemporaryClipRects);
          clipRect.intersect(paintDirtyRect);
        }
        
        p->push_node(d->m_canvas_clipping);
        ContextOfWRATH::set_clipping(d->m_canvas_clipping, clipRect);

        WRATHScaleTranslate sc;
        float4x4 m(ContextOfWRATH::wrath_matrix(transform));

        if(RenderLayerOfWRATH::transformation_can_be_done_with_scale_translate(m, sc)) {
          p->push_node(d->m_fake_canvas);
          d->m_fake_canvas.widget()->visible(true);
          d->m_fake_canvas.widget()->scaling_factor(sc.scale());
          d->m_fake_canvas.widget()->position(sc.translation());
        }
        else {

          WRATH_PUSH_CANVAS_NODE(p, d->m_canvas);

          d->m_canvas.widget()->visible(true);
          d->m_canvas.widget()
            ->properties()
            ->contents()->simulation_matrix(LayerOfWRATH::modelview_matrix, m);
        }

        d->m_rotation.visible(true);
        readyWRATHWidgetsLayer(d->m_rotation, this, p, transform.inverse().mapRect(paintDirtyRect),
                               paintBehavior, paintingRoot, overlapTestRequests, 
                               paintFlags | PaintLayerAppliedTransform);

        p->pop_node(); //d->m_canvas or d->m_fake_canvas
        p->pop_node(); //d->m_canvas_clipping
        return;
    }

    PaintLayerFlags localPaintFlags = paintFlags&~PaintLayerAppliedTransform;
    bool haveTransparency = localPaintFlags & PaintLayerHaveTransparency;


    d->m_reflection.visible(false);
    if (m_reflection && !paintingInsideReflection()) {
        setPaintingInsideReflection(true);
        d->m_reflection.visible(true);
        reflectionLayer()->readyWRATHWidgetsLayer(d->m_reflection, rootLayer, p, paintDirtyRect, paintBehavior, 
                                                  paintingRoot, 
                                                  overlapTestRequests, localPaintFlags | PaintLayerPaintingReflection);
        setPaintingInsideReflection(false);
    } 

    // Ensure our lists are up-to-date.
    updateLayerListsIfNeeded();

    // Calculate the clip rects we should use.
    IntRect layerBounds, damageRect, clipRectToApply, outlineRect;

    calculateRects(rootLayer, 
                   paintDirtyRect,
                   layerBounds, damageRect, clipRectToApply, outlineRect, 
                   localPaintFlags & PaintLayerTemporaryClipRects);
    int x = layerBounds.x();
    int y = layerBounds.y();
    int tx = x - renderBoxX();
    int ty = y - renderBoxY();
                           
    

    bool forceBlackText = paintBehavior & PaintBehaviorForceBlackText;
    bool selectionOnly  = paintBehavior & PaintBehaviorSelectionOnly;
    
    
    RenderObject* paintingRootForRenderer = 0;
    if (paintingRoot && !renderer()->isDescendantOf(paintingRoot))
        paintingRootForRenderer = paintingRoot;

    if (overlapTestRequests && isSelfPaintingLayer())
        performOverlapTests(*overlapTestRequests, rootLayer, this);

    bool paintingOverlayScrollbars = paintFlags & PaintLayerPaintingOverlayScrollbars;
    
    bool shouldPaint = intersectsDamageRect(layerBounds, damageRect, rootLayer) && m_hasVisibleContent && isSelfPaintingLayer();
    

    //haveTransparency=haveTransparency && renderer()->opacity()<0.8f;
    if (haveTransparency) {
      IntRect clipRect = transparencyClipBox(this, rootLayer, paintBehavior);
       /*
          [WRATH-TODO]:
          - add state to d->m_transparent_canvas indicating it is a 
            transparent LayerOfWRATH with opacity
            renderer()->opacity()
         */
      WRATH_PUSH_CANVAS_NODE(p, d->m_transparent_canvas);
      ContextOfWRATH::set_clipping(d->m_transparent_canvas, clipRect);
      d->m_transparent_canvas.widget()->visible(true);
    } 

    {
      ContextOfWRATH::AutoPushNode autopush(p, d->m_damageRect_node);

      d->m_damageRect_node.widget()->visible(shouldPaint && !selectionOnly && !damageRect.isEmpty() && !paintingOverlayScrollbars);
      if (d->m_damageRect_node.widget()->visible()) {
        
        ContextOfWRATH::set_clipping(d->m_damageRect_node, damageRect);
        PaintInfoOfWRATH paintInfo(p, damageRect, PaintPhaseBlockBackground, 
                                 false, paintingRootForRenderer, 0);
        
        d->m_per_pass[paintInfo.phase].visible(true);
        renderer()->readyWRATHWidgets(d->m_per_pass[paintInfo.phase], paintInfo, tx, ty);
      } 
    }

    d->m_neg_list.visible(true);
    readyWRATHWidgetsList(d->m_neg_list, m_negZOrderList, rootLayer, p, paintDirtyRect,
                          paintBehavior, paintingRoot, overlapTestRequests, localPaintFlags);


    {
      ContextOfWRATH::AutoPushNode autopush(p, d->m_clipRect_node);
      ContextOfWRATH::set_clipping(d->m_clipRect_node, clipRectToApply, 
                                   shouldPaint && !paintingOverlayScrollbars && !clipRectToApply.isEmpty());
      
      if (shouldPaint && !clipRectToApply.isEmpty() && !paintingOverlayScrollbars) {
        
        PaintInfoOfWRATH paintInfo(p, clipRectToApply, 
                                 selectionOnly ? PaintPhaseSelection : PaintPhaseChildBlockBackgrounds,
                                 forceBlackText, paintingRootForRenderer, 0);
        
        d->m_per_pass[paintInfo.phase].visible(true);
        renderer()->readyWRATHWidgets(d->m_per_pass[paintInfo.phase], paintInfo, tx, ty);
        
        if(!selectionOnly) {
          
          paintInfo.phase = PaintPhaseFloat;
          d->m_per_pass[paintInfo.phase].visible(true);
          renderer()->readyWRATHWidgets(d->m_per_pass[paintInfo.phase], paintInfo, tx, ty);
          
          paintInfo.phase = PaintPhaseForeground;
          paintInfo.overlapTestRequests = overlapTestRequests;
          d->m_per_pass[paintInfo.phase].visible(true);
          renderer()->readyWRATHWidgets(d->m_per_pass[paintInfo.phase], paintInfo, tx, ty);
          
          paintInfo.phase = PaintPhaseChildOutlines;
          d->m_per_pass[paintInfo.phase].visible(true);
          renderer()->readyWRATHWidgets(d->m_per_pass[paintInfo.phase], paintInfo, tx, ty);
        } 
      } 
    }

    {
      ContextOfWRATH::AutoPushNode autopush(p, d->m_outlineRect_node);
      ContextOfWRATH::set_clipping(d->m_outlineRect_node, outlineRect, 
                                   !outlineRect.isEmpty() && isSelfPaintingLayer() && !paintingOverlayScrollbars);
      
      if (!outlineRect.isEmpty() && isSelfPaintingLayer() && !paintingOverlayScrollbars) {
        PaintInfoOfWRATH paintInfo(p, outlineRect, PaintPhaseSelfOutline, false, paintingRootForRenderer, 0);
        
        d->m_per_pass[paintInfo.phase].visible(true);
        renderer()->readyWRATHWidgets(d->m_per_pass[paintInfo.phase], paintInfo, tx, ty);
      } 
    }


    // Paint any child layers that have overflow.
    d->m_flow_list.visible(true);
    readyWRATHWidgetsList(d->m_flow_list, m_normalFlowList, rootLayer, p, paintDirtyRect, paintBehavior, 
                          paintingRoot, overlapTestRequests, localPaintFlags);
    
    // Now walk the sorted list of children with positive z-indices.
    d->m_pos_list.visible(true);
    readyWRATHWidgetsList(d->m_pos_list, m_posZOrderList, rootLayer, p, paintDirtyRect, paintBehavior, 
                          paintingRoot, overlapTestRequests, localPaintFlags);
       

    {
      ContextOfWRATH::AutoPushNode autopush(p, d->m_mask_node);
      ContextOfWRATH::set_clipping(d->m_mask_node,  damageRect,
                                   renderer()->hasMask() && shouldPaint  && !damageRect.isEmpty() && !selectionOnly && !paintingOverlayScrollbars);
      
      if (renderer()->hasMask() && shouldPaint && !selectionOnly 
          && !damageRect.isEmpty() && !paintingOverlayScrollbars) {
        
        // Paint the mask.
        PaintInfoOfWRATH paintInfo(p, damageRect, PaintPhaseMask, false, paintingRootForRenderer, 0);
        
        d->m_per_pass[paintInfo.phase].visible(true);
        renderer()->readyWRATHWidgets(d->m_per_pass[paintInfo.phase], paintInfo, tx, ty);
      } 
    }

    {
      ContextOfWRATH::AutoPushNode autopush(p, d->m_overflow_node);
      
      ContextOfWRATH::set_clipping(d->m_overflow_node, damageRect, paintingOverlayScrollbars);
      if(paintingOverlayScrollbars) {
        d->m_overflow_controls.visible(true);
        readyWRATHOverflowControls(p, 
                                   d->m_overflow_controls,
                                   tx, ty, damageRect, true);
      }
    }


    // End our transparency layer
    if (haveTransparency) {
      p->pop_node();
    }

}

}
#endif
