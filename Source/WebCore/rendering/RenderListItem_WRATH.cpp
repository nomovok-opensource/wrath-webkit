#include "config.h"
#include "RenderListItem.h"

#include "CachedImage.h"
#include "HTMLNames.h"
#include "HTMLOListElement.h"
#include "RenderListMarker.h"
#include "RenderView.h"
#include <wtf/StdLibExtras.h>

using namespace std;

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
namespace {
  class RenderListItem_WRATHWidgets:
    public WebCore::PaintedWidgetsOfWRATH<RenderListItem_WRATHWidgets>
  {
  public:
    WebCore::PaintedWidgetsOfWRATHHandle m_render_block;
  };
}

namespace WebCore {
void RenderListItem::readyWRATHWidgets(PaintedWidgetsOfWRATHHandle &handle,
                                       PaintInfoOfWRATH &paintInfo, int tx, int ty)

{
  RenderListItem_WRATHWidgets *d;

  d=RenderListItem_WRATHWidgets::object(this, handle);
  ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

  d->m_render_block.visible(false);

  if (!logicalHeight()) {
    return;
  }

  
  d->m_render_block.visible(true);
  RenderBlock::readyWRATHWidgets(d->m_render_block, paintInfo, tx, ty);
}
}
#endif
