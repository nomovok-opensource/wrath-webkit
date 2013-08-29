#include "config.h"
#include "RenderHTMLCanvas.h"

#include "CanvasRenderingContext.h"
#include "Document.h"
#include "FrameView.h"
#include "GraphicsContext.h"
#include "HTMLCanvasElement.h"
#include "HTMLNames.h"
#include "PaintInfo.h"
#include "RenderView.h"

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
namespace WebCore
{
void RenderHTMLCanvas::readyWRATHWidgetReplaced(PaintedWidgetsOfWRATHHandle&,
                                                PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
  /* [WRATH-TODO] */
  WRATH_UNIMPLEMENTED(paintInfo.wrath_context);
}
}  
#endif

