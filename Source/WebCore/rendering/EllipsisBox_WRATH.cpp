#include "config.h"
#include "EllipsisBox.h"

#include "Document.h"
#include "GraphicsContext.h"
#include "HitTestResult.h"
#include "PaintInfo.h"
#include "RootInlineBox.h"
#include "TextRun.h"

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
#include "WRATHPaintHelpers.h"
#include "DrawnTextOfWRATH.h"

namespace 
{
  class EllipsisBox_Selection:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::InlineBox, EllipsisBox_Selection>
  {
  public:

    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_clip_node;
    WebCore::FilledFloatRectOfWRATH m_rect_item;
  };

  class EllipsisBox_WRATHWidgets:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::InlineBox, EllipsisBox_WRATHWidgets>
  {
  public:

    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::InlineBox> m_selection;
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::InlineBox> m_markupBox;
    WebCore::DrawnTextOfWRATHData m_text_data;
  };
}

namespace WebCore {

void EllipsisBox::readyWRATHWidgets(PaintedWidgetsOfWRATHHandleT<InlineBox> &handle, 
                                    PaintInfoOfWRATH &paintInfo, int tx, int ty, int lineTop, int lineBottom)
{
    EllipsisBox_WRATHWidgets *d;

    d=EllipsisBox_WRATHWidgets::object(this, handle);
    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    d->m_selection.visible(false);
    d->m_markupBox.visible(false);

    RenderStyle* style = renderer()->style(m_firstLine);
    Color textColor = style->visitedDependentColor(CSSPropertyColor);
    ColorSpace colorSpace(style->colorSpace());

    bool setShadow = false;
    if (style->textShadow()) {
      /*
        [WRATH-TODO]: note the shadow style.

        context->setShadow(IntSize(style->textShadow()->x(), style->textShadow()->y()),
                           style->textShadow()->blur(), style->textShadow()->color(), style->colorSpace());
      */
        setShadow = true;
    }

    if (selectionState() != RenderObject::SelectionNone) {
        d->m_selection.visible(true);
        readyWRATHSelection(d->m_selection, paintInfo, tx, ty, style, style->font());

        // Select the correct color for painting the text.
        Color foreground = paintInfo.forceBlackText ? Color::black : renderer()->selectionForegroundColor();
        if (foreground.isValid() && foreground != textColor) 
          textColor=foreground;
    }

    const String& str = m_str;

    

    // FIXME: Why is this alwasy LTR?
    d->m_text_data.update(paintInfo.wrath_context, style->font(),
                          TextRun(str.characters(), str.length(), false, 0, 0, TextRun::AllowTrailingExpansion, LTR, style->visuallyOrdered()),
                          textColor);
    d->m_text_data.node()->position(vec2(x() + tx, y() + ty + style->fontMetrics().ascent()));

    //context->drawText(style->font(), TextRun(str.characters(), str.length(), false, 0, 0, TextRun::AllowTrailingExpansion, LTR, style->visuallyOrdered()), IntPoint(x() + tx, y() + ty + style->fontMetrics().ascent()));


    if (setShadow) {
      /*
        [WRATH-TODO]: remember to unset whatever state we set from observing the shadow.
       */
    }


    if (m_markupBox) {
        d->m_markupBox.visible(true);

        // Paint the markup box
        tx += x() + logicalWidth() - m_markupBox->x();
        ty += y() + style->fontMetrics().ascent() - (m_markupBox->y() + m_markupBox->renderer()->style(m_firstLine)->fontMetrics().ascent());
        m_markupBox->readyWRATHWidgets(d->m_markupBox, paintInfo, tx, ty, lineTop, lineBottom);
    }
}

void EllipsisBox::readyWRATHSelection(PaintedWidgetsOfWRATHHandleT<InlineBox> &handle, 
                                      PaintInfoOfWRATH &paintInfo, int tx, int ty, RenderStyle* style, const Font& font)
{
    EllipsisBox_Selection *d;

    d=EllipsisBox_Selection::object(this, handle);
    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);
    if(d->m_rect_item.widget()) {
      d->m_rect_item.widget()->visible(false);
    }
    

    Color textColor = style->visitedDependentColor(CSSPropertyColor);
    Color c = renderer()->selectionBackgroundColor();
    if (!c.isValid() || !c.alpha())
        return;

    // If the text color ends up being the same as the selection background, invert the selection
    // background.
    if (textColor == c)
        c = Color(0xff - c.red(), 0xff - c.green(), 0xff - c.blue());

    
    int t = root()->selectionTop();
    int h = root()->selectionHeight();
    FloatRect R;

    R=DrawnTextOfWRATHData::selectionRectForText(font, 
                                                 TextRun(m_str.characters(), m_str.length(), false, 0, 0, 
                                                         TextRun::AllowTrailingExpansion, LTR, style->visuallyOrdered()), 
                                                 IntPoint(x() + tx, y() + ty + ty), h);

    
    ContextOfWRATH::AutoPushNode autoPushClip(paintInfo.wrath_context, d->m_clip_node); 
    ContextOfWRATH::set_clipping(d->m_clip_node,
                                 IntRect(x() + tx, t + ty, logicalWidth(), h));


    d->m_rect_item.update(paintInfo.wrath_context, R, c, CompositeSourceOver);
    if(d->m_rect_item.widget()) {
      d->m_rect_item.widget()->visible(true);
    }
}

}

#endif
