#include "config.h"
#include "RenderDetailsMarker.h"

#if ENABLE(DETAILS)

#include "GraphicsContext.h"
#include "HTMLNames.h"
#include "PaintInfo.h"
#include "RenderDetails.h"

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"

namespace WebCore
{
void RenderDetailsMarker::readyWRATHWidgets(PaintedWidgetsOfWRATHHandle&,
                                            PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
  /* [WRATH-TODO] */
  WRATH_UNIMPLEMENTED(paintInfo.wrath_context);
}
}
#endif
#endif
