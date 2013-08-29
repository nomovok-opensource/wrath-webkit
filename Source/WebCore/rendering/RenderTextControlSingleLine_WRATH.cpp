#include "config.h"
#include "RenderTextControlSingleLine.h"

#include "Chrome.h"
#include "CSSStyleSelector.h"
#include "Event.h"
#include "EventNames.h"
#include "Frame.h"
#include "FrameView.h"
#include "HTMLInputElement.h"
#include "HTMLNames.h"
#include "HitTestResult.h"
#include "InputElement.h"
#include "LocalizedStrings.h"
#include "MouseEvent.h"
#include "PlatformKeyboardEvent.h"
#include "RenderLayer.h"
#include "RenderScrollbar.h"
#include "RenderTheme.h"
#include "SelectionController.h"
#include "Settings.h"
#include "SimpleFontData.h"
#include "TextControlInnerElements.h"

using namespace std;

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
namespace
{
    class RenderTextControlSingleLine_ReadyWRATHWidgets:
      public WebCore::PaintedWidgetsOfWRATH<RenderTextControlSingleLine_ReadyWRATHWidgets>
    {
    public:
	WebCore::PaintedWidgetsOfWRATHHandle m_text_control;
    };
}
namespace WebCore {
void RenderTextControlSingleLine::readyWRATHWidgets(PaintedWidgetsOfWRATHHandle& handle,
						    PaintInfoOfWRATH& paintInfo, int tx, int ty)
{
    RenderTextControlSingleLine_ReadyWRATHWidgets *d(RenderTextControlSingleLine_ReadyWRATHWidgets::object(this, handle));
    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    RenderTextControl::readyWRATHWidgets(d->m_text_control, paintInfo, tx, ty);

    if (paintInfo.phase == PaintPhaseBlockBackground && m_shouldDrawCapsLockIndicator) {
        IntRect contentsRect = contentBoxRect();

        // Center vertically like the text.
        contentsRect.setY((height() - contentsRect.height()) / 2);

        // Convert the rect into the coords used for painting the content
        contentsRect.move(tx + x(), ty + y());
	/* [WRATH-TODO]: Theme painting, not implemented */
        // theme()->paintCapsLockIndicator(this, paintInfo, contentsRect);
    }
}

void RenderTextControlSingleLine::readyWRATHWidgetBoxDecorations(PaintedWidgetsOfWRATHHandle& handle,
								 PaintInfoOfWRATH& paintInfo, int tx, int ty)
{
    /* We're only passing the call forward, so using the same handle is safe */
    readyWRATHWidgetBoxDecorationsWithSize(handle, paintInfo, tx, ty, width() - decorationWidthRight(), height());
}
}
#endif
