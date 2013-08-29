#include "config.h"
#include "RenderReplica.h"

#include "RenderLayer.h"

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
namespace
{
  class RenderReplica_WRATHWidgets:
    public WebCore::PaintedWidgetsOfWRATH<RenderReplica_WRATHWidgets>
  {
  public:
    
    WebCore::PaintedWidgetsOfWRATHHandle m_mask;
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::RenderLayer> m_foreground;
  };
}

namespace WebCore {

void RenderReplica::readyWRATHWidgets(PaintedWidgetsOfWRATHHandle &handle,
                                      PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
  RenderReplica_WRATHWidgets* d;
  d=RenderReplica_WRATHWidgets::object(this, handle);
  WebCore::ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

  d->m_foreground.visible(false);
  d->m_mask.visible(false);

  if (paintInfo.phase != PaintPhaseForeground && paintInfo.phase != PaintPhaseMask)
    return;
 
  tx += x();
  ty += y();

  if (paintInfo.phase == PaintPhaseForeground) {
    // Turn around and paint the parent layer. Use temporary clipRects, 
    //so that the layer doesn't end up caching clip rects
    // computing using the wrong rootLayer
    RenderLayer* rootLayer;

    rootLayer=layer()->transform() ? 
      layer()->parent() : 
      layer()->enclosingTransformedAncestor();

    d->m_foreground.visible(true);
    layer()->parent()->readyWRATHWidgetsLayer(d->m_foreground,
                                              rootLayer,
                                              paintInfo.wrath_context, paintInfo.rect,
                                              PaintBehaviorNormal, 0, 0,
                                              RenderLayer::PaintLayerHaveTransparency | RenderLayer::PaintLayerAppliedTransform | RenderLayer::PaintLayerTemporaryClipRects | RenderLayer::PaintLayerPaintingReflection);
  } else if (paintInfo.phase == PaintPhaseMask) {
    d->m_mask.visible(true);
    readyWRATHWidgetMask(d->m_mask, paintInfo, tx, ty);
  }  
}
}
#endif
