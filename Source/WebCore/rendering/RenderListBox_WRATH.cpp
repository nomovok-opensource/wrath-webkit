#include "config.h"
#include "RenderListBox.h"

#include "AXObjectCache.h"
#include "CSSStyleSelector.h"
#include "Document.h"
#include "EventHandler.h"
#include "EventQueue.h"
#include "FocusController.h"
#include "Frame.h"
#include "FrameView.h"
#include "GraphicsContext.h"
#include "HTMLNames.h"
#include "HitTestResult.h"
#include "NodeRenderStyle.h"
#include "OptionGroupElement.h"
#include "OptionElement.h"
#include "Page.h"
#include "PaintInfo.h"
#include "RenderLayer.h"
#include "RenderScrollbar.h"
#include "RenderTheme.h"
#include "RenderView.h"
#include "Scrollbar.h"
#include "ScrollbarTheme.h"
#include "SelectElement.h"
#include "SelectionController.h"
#include "TextRun.h"
#include <math.h>

using namespace std;

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
#include "ArrayOfHandlesOfWRATH.h"
#include "DrawnTextOfWRATH.h"

namespace {

  class RenderListBox_WRATHWidgetScrollbar:
    public WebCore::PaintedWidgetsOfWRATH<RenderListBox_WRATHWidgetScrollbar>
  {
  public:
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::Widget> m_scrollbar;
  };

  class RenderListBox_WRATHWidgetItemBackground:
    public WebCore::PaintedWidgetsOfWRATH<RenderListBox_WRATHWidgetItemBackground>
  {
  public:
    
    WebCore::ContextOfWRATH::ColorFamily::DrawnRect::AutoDelete m_rect;
    ivec2 m_rect_size;
  };


  class RenderListBox_WRATHWidgetObject:
    public WebCore::PaintedWidgetsOfWRATH<RenderListBox_WRATHWidgetObject>
  {
  public:
    
    WebCore::PaintedWidgetsOfWRATHHandle m_render_block;
    WebCore::PaintedWidgetsOfWRATHHandle m_scroll_bar;
    WebCore::ArrayOfHandlesOfWRATH<WebCore::RenderObject> m_backgrounds;
    WebCore::ArrayOfHandlesOfWRATH<WebCore::RenderObject> m_foregrounds;
    
  };


  class RenderListBox_WRATHWidgetItemForeground:
    public WebCore::PaintedWidgetsOfWRATH<RenderListBox_WRATHWidgetItemForeground>
  {
  public:    
    WebCore::DrawnTextOfWRATHData m_text;
  };
  
}//namespace

namespace WebCore {

using namespace HTMLNames;
 
const int rowSpacing = 1;

const int optionsSpacingHorizontal = 2;

const int minSize = 4;
const int maxDefaultSize = 10;

// FIXME: This hardcoded baselineAdjustment is what we used to do for the old
// widget, but I'm not sure this is right for the new control.
const int baselineAdjustment = 7;

void RenderListBox::readyWRATHWidgetObject(PaintedWidgetsOfWRATHHandle &handle,
                                           PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
  RenderListBox_WRATHWidgetObject *d;

  d=RenderListBox_WRATHWidgetObject::object(this, handle);
  ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

  d->m_render_block.visible(false);
  d->m_scroll_bar.visible(false);
  d->m_backgrounds.visible_each(false);
  d->m_foregrounds.visible_each(false);

  if (style()->visibility() != VISIBLE)
    return;
    
  int listItemsSize = numItems();
  if (paintInfo.phase == PaintPhaseForeground) {
    int index = m_indexOffset;
    while (index < listItemsSize && index <= m_indexOffset + numVisibleItems()) {
      /*
        [WRATH-TODO]: make this less stupid and do the correct thing
        when it comes to array size, rather than just punting this way.
       */
      if(index>=d->m_foregrounds.size()) {
        d->m_foregrounds.resize(index+1);
      }
      d->m_foregrounds[index].visible(true);
      readyWRATHWidgetItemForeground(d->m_foregrounds[index], paintInfo, tx, ty, index);
      index++;
    }
  }

  d->m_render_block.visible(true);
  RenderBlock::readyWRATHWidgetObject(d->m_render_block, paintInfo, tx, ty);


  switch (paintInfo.phase) {
    // Depending on whether we have overlay scrollbars they
    // get rendered in the foreground or background phases
    case PaintPhaseForeground:
        if (m_vBar->isOverlayScrollbar()) {
          d->m_scroll_bar.visible(true);
          readyWRATHWidgetScrollbar(d->m_scroll_bar, paintInfo, tx, ty);
        }
        break;
    case PaintPhaseBlockBackground:
        if (!m_vBar->isOverlayScrollbar()) {
          d->m_scroll_bar.visible(true);
          readyWRATHWidgetScrollbar(d->m_scroll_bar, paintInfo, tx, ty);
        }
        break;
    case PaintPhaseChildBlockBackground:
    case PaintPhaseChildBlockBackgrounds: {
        int index = m_indexOffset;
        while (index < listItemsSize && index <= m_indexOffset + numVisibleItems()) {
            /*
              [WRATH-TODO]: make this less stupid and do the correct thing
              when it comes to array size, rather than just punting this way.
            */
            if(index>=d->m_backgrounds.size()) {
              d->m_backgrounds.resize(index+1);
            }
            d->m_backgrounds[index].visible(true);
            readyWRATHWidgetItemBackground(d->m_backgrounds[index], paintInfo, tx, ty, index);
            index++;
        }
        break;
    }
  default:
    break;
    
  }
}

void RenderListBox::readyWRATHWidgetScrollbar(PaintedWidgetsOfWRATHHandle &handle,
                                              PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
  RenderListBox_WRATHWidgetScrollbar *d;
    
  d=RenderListBox_WRATHWidgetScrollbar::object(this, handle);
  ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

  d->m_scrollbar.visible(false);
  if (m_vBar) {
    IntRect scrollRect(tx + width() - borderRight() - m_vBar->width(),
                       ty + borderTop(),
                       m_vBar->width(),
                       height() - (borderTop() + borderBottom()));
    m_vBar->setFrameRect(scrollRect);

    d->m_scrollbar.visible(true);
    m_vBar->readyWRATHWidgets(paintInfo.wrath_context, d->m_scrollbar);
  }
}

static IntSize itemOffsetForAlignment(TextRun textRun, RenderStyle* itemStyle, Font itemFont, IntRect itemBoudingBox)
{
    ETextAlign actualAlignment = itemStyle->textAlign();
    // FIXME: Firefox doesn't respect JUSTIFY. Should we?
    if (actualAlignment == TAAUTO || actualAlignment == JUSTIFY)
      actualAlignment = itemStyle->isLeftToRightDirection() ? LEFT : RIGHT;

    IntSize offset = IntSize(0, itemFont.fontMetrics().ascent());
    if (actualAlignment == RIGHT || actualAlignment == WEBKIT_RIGHT) {
        float textWidth = itemFont.width(textRun);
        offset.setWidth(itemBoudingBox.width() - textWidth - optionsSpacingHorizontal);
    } else if (actualAlignment == CENTER || actualAlignment == WEBKIT_CENTER) {
        float textWidth = itemFont.width(textRun);
        offset.setWidth((itemBoudingBox.width() - textWidth) / 2);
    } else
        offset.setWidth(optionsSpacingHorizontal);
    return offset;
}

void RenderListBox::readyWRATHWidgetItemForeground(PaintedWidgetsOfWRATHHandle &handle,
                                                   PaintInfoOfWRATH &paintInfo, int tx, int ty, int listIndex)
{
    RenderListBox_WRATHWidgetItemForeground *d;
    
    d=RenderListBox_WRATHWidgetItemForeground::object(this, handle);
    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    SelectElement* select = toSelectElement(static_cast<Element*>(node()));
    const Vector<Element*>& listItems = select->listItems();
    Element* element = listItems[listIndex];
    OptionElement* optionElement = toOptionElement(element);

    RenderStyle* itemStyle = element->renderStyle();
    if (!itemStyle)
        itemStyle = style();

    if (itemStyle->visibility() == HIDDEN)
        return;

    String itemText;
    if (optionElement)
        itemText = optionElement->textIndentedToRespectGroupLabel();
    else if (OptionGroupElement* optionGroupElement = toOptionGroupElement(element))
        itemText = optionGroupElement->groupLabelText();
    
    Color textColor = element->renderStyle() ? element->renderStyle()->visitedDependentColor(CSSPropertyColor) : style()->visitedDependentColor(CSSPropertyColor);
    if (optionElement && optionElement->selected()) {
        if (frame()->selection()->isFocusedAndActive() && document()->focusedNode() == node())
            textColor = theme()->activeListBoxSelectionForegroundColor();
        // Honor the foreground color for disabled items
        else if (!element->disabled())
            textColor = theme()->inactiveListBoxSelectionForegroundColor();
    }

    //ColorSpace colorSpace = itemStyle->colorSpace();
    //paintInfo.context->setFillColor(textColor, colorSpace);

    unsigned length = itemText.length();
    const UChar* string = itemText.characters();
    TextRun textRun(string, length, false, 0, 0, TextRun::AllowTrailingExpansion, itemStyle->direction(), itemStyle->unicodeBidi() == Override);
    Font itemFont = style()->font();
    IntRect r = itemBoundingBoxRect(tx, ty, listIndex);
    r.move(itemOffsetForAlignment(textRun, itemStyle, itemFont, r));

    if (isOptionGroupElement(element)) {
        FontDescription d = itemFont.fontDescription();
        d.setWeight(d.bolderWeight());
        itemFont = Font(d, itemFont.letterSpacing(), itemFont.wordSpacing());
        itemFont.update(document()->styleSelector()->fontSelector());
    }

    // Draw the item text
    if (itemStyle->visibility() != HIDDEN) {
      //paintInfo.context->drawBidiText(itemFont, textRun, r.location());

      d->m_text.update(paintInfo.wrath_context, itemFont, textRun, textColor);
      d->m_text.text_visible(true);

      vec2 text_position(r.location().x(), r.location().y());
      d->m_text.node()->position(text_position);
    } else {
      d->m_text.text_visible(false);
    }

}


void RenderListBox::readyWRATHWidgetItemBackground(PaintedWidgetsOfWRATHHandle &handle,
                                                   PaintInfoOfWRATH &paintInfo, int tx, int ty, int listIndex)
{
    RenderListBox_WRATHWidgetItemBackground *d;
    
    d=RenderListBox_WRATHWidgetItemBackground::object(this, handle);
    ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

    if(d->m_rect.widget()) {
      d->m_rect.widget()->visible(false);
    }

    SelectElement* select = toSelectElement(static_cast<Element*>(node()));
    const Vector<Element*>& listItems = select->listItems();
    Element* element = listItems[listIndex];
    OptionElement* optionElement = toOptionElement(element);

    Color backColor;
    if (optionElement && optionElement->selected()) {
        if (frame()->selection()->isFocusedAndActive() && document()->focusedNode() == node())
            backColor = theme()->activeListBoxSelectionBackgroundColor();
        else
            backColor = theme()->inactiveListBoxSelectionBackgroundColor();
    } else
        backColor = element->renderStyle() ? element->renderStyle()->visitedDependentColor(CSSPropertyBackgroundColor) : style()->visitedDependentColor(CSSPropertyBackgroundColor);

    // Draw the background for this list box item
    if (!element->renderStyle() || element->renderStyle()->visibility() != HIDDEN) {
      //ColorSpace colorSpace = element->renderStyle() ? element->renderStyle()->colorSpace() : style()->colorSpace();
        IntRect itemRect = itemBoundingBoxRect(tx, ty, listIndex);
        itemRect.intersect(controlClipRect(tx, ty));
        //paintInfo.context->fillRect(itemRect, backColor, colorSpace);
        
        if(!d->m_rect.widget() 
           or d->m_rect_size.x()!=itemRect.width()
           or d->m_rect_size.y()!=itemRect.height()) {

          d->m_rect_size.x()=itemRect.width();
          d->m_rect_size.y()=itemRect.height();

          if(!d->m_rect.widget()) {
            paintInfo.wrath_context->add_rect(d->m_rect,
                                              WRATHWidgetGenerator::Rect(d->m_rect_size.x(), d->m_rect_size.y()));
          }
          else {
            WRATHWidgetGenerator::Rect(d->m_rect_size.x(), d->m_rect_size.y())(d->m_rect.widget());
	    paintInfo.wrath_context->update_generic(d->m_rect);
          }

        } else {
          paintInfo.wrath_context->update_generic(d->m_rect);
        }

        vec4 c;
        backColor.getRGBA(c.x(), c.y(), c.z(), c.w());

        d->m_rect.widget()->visible(true);
        d->m_rect.widget()->color(c);
        d->m_rect.widget()->position(vec2(itemRect.x(), itemRect.y()));

        /*
          [WRATH-DANGER] WebKit wants to blend, always wants to blend.
          Which is insanity. 
         */
        if(backColor.alpha()==0) {
          d->m_rect.widget()->visible(false);
        }
    }

}
}
#endif
