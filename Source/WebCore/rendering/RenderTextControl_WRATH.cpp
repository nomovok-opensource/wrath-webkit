#include "config.h"
#include "RenderTextControl.h"

#include "AXObjectCache.h"
#include "Editor.h"
#include "Event.h"
#include "EventNames.h"
#include "Frame.h"
#include "HTMLBRElement.h"
#include "HTMLFormControlElement.h"
#include "HTMLInputElement.h"
#include "HTMLNames.h"
#include "HitTestResult.h"
#include "Position.h"
#include "RenderLayer.h"
#include "RenderText.h"
#include "ScrollbarTheme.h"
#include "SelectionController.h"
#include "Text.h"
#include "TextControlInnerElements.h"
#include "TextIterator.h"
#include "TextRun.h"
#include <wtf/unicode/CharacterNames.h>

using namespace std;

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
#include "DrawnTextOfWRATH.h"
namespace
{
    class RenderTextControl_ReadyWRATHWidgetObject:
      public WebCore::PaintedWidgetsOfWRATH<RenderTextControl_ReadyWRATHWidgetObject>
    {
    public:
	WebCore::PaintedWidgetsOfWRATHHandle m_placeholder;
	WebCore::PaintedWidgetsOfWRATHHandle m_block;
    };

    class RenderTextControl_ReadyWRATHWidgetPlaceholder:
      public WebCore::PaintedWidgetsOfWRATH<RenderTextControl_ReadyWRATHWidgetPlaceholder>
    {
    public:
	WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_clip_node;
        WebCore::DrawnTextOfWRATHData m_text;
    };
}
namespace WebCore {
void RenderTextControl::readyWRATHWidgetPlaceholder(PaintedWidgetsOfWRATHHandle& handle,
						    PaintInfoOfWRATH& paintInfo, int tx, int ty)
{
    RenderTextControl_ReadyWRATHWidgetPlaceholder *d(RenderTextControl_ReadyWRATHWidgetPlaceholder::object(this, handle));

    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);
    d->m_text.text_visible(false);

    if (style()->visibility() != VISIBLE)
        return;
    
    IntRect clipRect(tx + borderLeft(), ty + borderTop(), width() - borderLeft() - borderRight(), height() - borderBottom() - borderTop());
    if (clipRect.isEmpty())
        return;

    ContextOfWRATH::AutoPushNode autoPushClip(paintInfo.wrath_context, d->m_clip_node);
    ContextOfWRATH::set_clipping(d->m_clip_node, clipRect);
    
    RefPtr<RenderStyle> placeholderStyle = getCachedPseudoStyle(INPUT_PLACEHOLDER);
    if (!placeholderStyle)
        placeholderStyle = style();
    
    /* [WRATH-DANGER]: ColorSpace is not used */
    Color fillColor = placeholderStyle->visitedDependentColor(CSSPropertyColor);
    
    String placeholderText = static_cast<HTMLTextFormControlElement*>(node())->strippedPlaceholder();
    TextRun textRun(placeholderText.characters(), placeholderText.length(), false, 0, 0, TextRun::AllowTrailingExpansion, placeholderStyle->direction(), placeholderStyle->unicodeBidi() == Override);
    
    RenderBox* textRenderer = innerTextElement() ? innerTextElement()->renderBox() : 0;
    if (textRenderer) {
        IntPoint textPoint;
        textPoint.setY(ty + textBlockInsetTop() + placeholderStyle->fontMetrics().ascent());
        int styleTextIndent = placeholderStyle->textIndent().isFixed() ? placeholderStyle->textIndent().calcMinValue(0) : 0;
        if (placeholderStyle->isLeftToRightDirection())
            textPoint.setX(tx + styleTextIndent + textBlockInsetLeft());
        else
            textPoint.setX(tx + width() - textBlockInsetRight() - styleTextIndent - style()->font().width(textRun));

	vec2 text_position(textPoint.x(), textPoint.y());
	d->m_text.update(paintInfo.wrath_context, placeholderStyle->font(), textRun, fillColor);
	d->m_text.node()->position(text_position);
        d->m_text.text_visible(true);
    }
}

void RenderTextControl::readyWRATHWidgetObject(PaintedWidgetsOfWRATHHandle &handle,
					       PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
    RenderTextControl_ReadyWRATHWidgetObject *d(RenderTextControl_ReadyWRATHWidgetObject::object(this, handle));
    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    d->m_placeholder.visible(false);

    if (m_placeholderVisible && paintInfo.phase == PaintPhaseForeground) {
	d->m_placeholder.visible(true);
        readyWRATHWidgetPlaceholder(d->m_placeholder, paintInfo, tx, ty);
    }
    
    RenderBlock::readyWRATHWidgetObject(d->m_block, paintInfo, tx, ty);
}
}
#endif
