#include "config.h"
#include "RenderFileUploadControl.h"

#include "Chrome.h"
#include "FileList.h"
#include "Frame.h"
#include "FrameView.h"
#include "GraphicsContext.h"
#include "HTMLInputElement.h"
#include "HTMLNames.h"
#include "ShadowElement.h"
#include "Icon.h"
#include "LocalizedStrings.h"
#include "Page.h"
#include "PaintInfo.h"
#include "RenderButton.h"
#include "RenderText.h"
#include "RenderTheme.h"
#include "RenderView.h"
#include "TextRun.h"
#include <math.h>

using namespace std;

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"

namespace WebCore
{
using namespace HTMLNames;

void RenderFileUploadControl::readyWRATHWidgetObject(PaintedWidgetsOfWRATHHandle&,
                                                     PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
  /* [WRATH-TODO] */
  WRATH_UNIMPLEMENTED(paintInfo.wrath_context);
}
}
#endif
