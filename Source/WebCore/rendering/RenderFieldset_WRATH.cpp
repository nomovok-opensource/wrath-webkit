#include "config.h"
#include "RenderFieldset.h"

#include "CSSPropertyNames.h"
#include "GraphicsContext.h"
#include "HTMLNames.h"
#include "PaintInfo.h"

using std::min;
using std::max;

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
#endif

#if USE(WRATH)
namespace WebCore {
void RenderFieldset::readyWRATHWidgetBoxDecorations(PaintedWidgetsOfWRATHHandle&,
                                                    PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
  /* [WRATH-TODO] */
  WRATH_UNIMPLEMENTED(paintInfo.wrath_context);
}

void RenderFieldset::readyWRATHWidgetMask(PaintedWidgetsOfWRATHHandle&,
                                          PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
  /* [WRATH-TODO] */
  WRATH_UNIMPLEMENTED(paintInfo.wrath_context);
}
}
#endif
