#include "config.h"
#include "RenderListMarker.h"

#include "CachedImage.h"
#include "Document.h"
#include "GraphicsContext.h"
#include "RenderLayer.h"
#include "RenderListItem.h"
#include "RenderView.h"
#include "TextRun.h"
#include <wtf/unicode/CharacterNames.h>

using namespace std;
using namespace WTF;
using namespace Unicode;

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"

namespace WebCore {
void RenderListMarker::readyWRATHWidgets(PaintedWidgetsOfWRATHHandle&,
                                         PaintInfoOfWRATH &paintInto, int tx, int ty)
{
  /* [WRATH-TODO] */
  WRATH_UNIMPLEMENTED(paintInto.wrath_context);
}
}
#endif

