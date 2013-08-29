#include "config.h"
#include "InlineTextBox.h"

#include "Chrome.h"
#include "ChromeClient.h"
#include "Document.h"
#include "DocumentMarkerController.h"
#include "Editor.h"
#include "EllipsisBox.h"
#include "Frame.h"
#include "GraphicsContext.h"
#include "HitTestResult.h"
#include "Page.h"
#include "PaintInfo.h"
#include "RenderArena.h"
#include "RenderBR.h"
#include "RenderBlock.h"
#include "RenderCombineText.h"
#include "RenderRubyRun.h"
#include "RenderRubyText.h"
#include "RenderTheme.h"
#include "Text.h"
#include "break_lines.h"
#include <wtf/AlwaysInline.h>

using namespace std;

#if USE(WRATH)
#include <map>
#include "PaintInfoOfWRATH.h"
#include "HierarchyOfHandlesOfWRATH.h"
#include "ArrayOfHandlesOfWRATH.h"
#include "DrawnTextOfWRATH.h"
#include "WRATHPaintHelpers.h"

namespace
{
  typedef WebCore::ContextOfWRATH::CColorFamily::DrawnShape DrawnShape;

  class WRATHWidgetTextWithShadowElement:public WebCore::DrawnTextOfWRATH
  {
  public:
   
    WRATHWidgetTextWithShadowElement(void)
    {}
    
    static
    WRATHWidgetTextWithShadowElement*
    object(WebCore::InlineBox *I,
           WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::InlineBox> &handle)
    {
      return objectNoArgCtor(type_tag<WRATHWidgetTextWithShadowElement>(), I, handle);
    }
  
  };

  class InlineBox_WRATHWidgetTextWithShadow:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::InlineBox, InlineBox_WRATHWidgetTextWithShadow>
  {
  public:

    enum { number_text_items=3 };
    vecN<WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::InlineBox>, number_text_items> m_texts;
  };


  class InlineTextBox_CompositionUnderline:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::InlineBox, InlineTextBox_CompositionUnderline>
  {
  public:

    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_skip_node;
    int m_width;
    int m_thickness;
    DrawnShape::AutoDelete m_shape_item;

  };

  class InlineTextBox_WidgetDecorationElement:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::InlineBox, InlineTextBox_WidgetDecorationElement>
  {
  public:

    static void ready(WebCore::InlineBox *I,
                      WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::InlineBox> &handle,
                      WebCore::ContextOfWRATH *wrath_context,
                      WebCore::FloatPoint pt, 
                      float width, 
                      WebCore::Color color)
    {
      int iwidth(static_cast<int>(width));
      vec4 c(0.0f, 0.0f, 0.0f, 1.0f);
      InlineTextBox_WidgetDecorationElement *d;

      color.getRGBA(c.x(), c.y(), c.z(), c.w());

      d=InlineTextBox_WidgetDecorationElement::object(I, handle);
      WebCore::ContextOfWRATH::AutoPushNode autoPushRoot(wrath_context, d->m_root_node);

      if(!d->m_shape_item.widget() or d->m_width!=iwidth) {
        WRATHShapeF shape;
        
        shape.current_outline() << WRATHOutline<float>::position_type(0, 0)
                                << WRATHOutline<float>::position_type(iwidth, 0);
        d->m_width=iwidth;
        
        if(!d->m_shape_item.widget()) {
          /*
            [WRATH-TODO]: if the shape is transparent, we need
            to obey the transparency value.
          */
          wrath_context->add_stroked_shape(d->m_shape_item,
                                           WRATHWidgetGenerator::NullItemProperties(),
                                           WRATHWidgetGenerator::shape_value(shape),
                                           WRATHWidgetGenerator::StrokingParameters()
                                           .close_outline(false)
                                           .width(1.0f)
                                           .cap_style(WRATHWidgetGenerator::flat_cap));
        } else {
          d->m_shape_item.widget()->properties()->change_shape(WRATHWidgetGenerator::shape_value(shape),
                                                               WRATHWidgetGenerator::StrokingParameters()
                                                               .close_outline(false)
                                                               .width(1.0f)
                                                               .cap_style(WRATHWidgetGenerator::flat_cap));
          wrath_context->update_generic(d->m_shape_item);
        }
      } else {
        wrath_context->update_generic(d->m_shape_item);
      }
    
      d->m_shape_item.widget()->node()->color(c);
      d->m_shape_item.widget()->position(vec2(pt.x(), pt.y()));
    }

    DrawnShape::AutoDelete m_shape_item;
    int m_width;
  };
  

  class InlineTextBox_WidgetDecoration:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::InlineBox, InlineTextBox_WidgetDecoration>
  {
  public:
    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_skip_node;  
    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_clip_node;        
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::InlineBox> m_underline;
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::InlineBox> m_overline;
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::InlineBox> m_line_through;
    
  };


  class InlineTextBox_Selection:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::InlineBox, InlineTextBox_Selection>
  {
  public:    
    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_clip_rect;
    WebCore::FilledFloatRectOfWRATH m_rect_item;
  };


  class InlineTextBox_WRATHWidgets:
    public WebCore::PaintedWidgetsOfWRATHT<WebCore::InlineBox, InlineTextBox_WRATHWidgets>
  {
  public:

    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_skip_node;

    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::InlineBox> m_composition_background;
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::InlineBox> m_document_marker;
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::InlineBox> m_paint_selection;

    WebCore::ArrayOfHandlesOfWRATH<WebCore::InlineBox> m_underlines;
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::InlineBox> m_text1, m_text2;
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::InlineBox> m_text3;

    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::InlineBox> m_box_decorations;
    WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::InlineBox> m_document_markers;
  };
}

namespace WebCore
{
typedef Vector<UChar, 256> BufferForAppendingHyphen;

static void adjustCharactersAndLengthForHyphen(BufferForAppendingHyphen& charactersWithHyphen, RenderStyle* style, const UChar*& characters, int& length)
{
    const AtomicString& hyphenString = style->hyphenString();
    charactersWithHyphen.reserveCapacity(length + hyphenString.length());
    charactersWithHyphen.append(characters, length);
    charactersWithHyphen.append(hyphenString.characters(), hyphenString.length());
    characters = charactersWithHyphen.data();
    length += hyphenString.length();
}

static void readyWRATHWidgetTextWithShadow(PaintedWidgetsOfWRATHHandleT<InlineBox> &handle, 
                                           InlineTextBox *ptr,
                                           ContextOfWRATH *wrath_context,
                                           const Font& font, const TextRun& textRun, 
                                           const AtomicString& emphasisMark, int emphasisMarkOffset, 
                                           int startOffset, int endOffset, int truncationPoint, 
                                           const FloatPoint& textOrigin,
                                           const FloatRect& boxRect, const ShadowData* shadow, 
                                           bool stroked, bool horizontal,
                                           const Color& pfillColor, const Color& pstrokeColor, float strokeThickness)
{
  InlineBox_WRATHWidgetTextWithShadow *d;

  d=InlineBox_WRATHWidgetTextWithShadow::object(ptr, handle);
  ContextOfWRATH::AutoPushNode autoPushRoot(wrath_context, d->m_root_node);
  
  /*
    [WRATH-TODO]:
    - observe shadows
    - observe opacity
    - observe horizontal flag
   */
  vec2 text_position;

  text_position=vec2(textOrigin.x(), textOrigin.y());
  d->m_root_node.widget()->position(text_position);

  Color color;
  if(strokeThickness>0) {
    color=pstrokeColor;
  }
  else {
    color=pfillColor;
  }
  

  vecN<WRATHWidgetTextWithShadowElement*, InlineBox_WRATHWidgetTextWithShadow::number_text_items> texts;

  for(unsigned int i=0; i<d->m_texts.size(); ++i)
    {
      d->m_texts[i].visible(false);
      texts[i]=WRATHWidgetTextWithShadowElement::object(ptr, d->m_texts[i]);
    }
  
  if (startOffset <= endOffset) {
    if (emphasisMark.isEmpty()) {
      d->m_texts[0].visible(true);
      texts[0]->update(wrath_context, font, textRun, color, startOffset, endOffset);
    } else {
      /*
        painting path does drawEmphasisMarks which does nothing 
        in Qt builds anyways..
      */
    }
  } else {
    if (endOffset > 0) {
      if (emphasisMark.isEmpty()) {
        d->m_texts[1].visible(true);
        texts[1]->update(wrath_context, font, textRun, color, 0, endOffset);
      } else {
        /*
          painting path does drawEmphasisMarks which does nothing 
          in Qt builds anyways..
        */
      }
    } if (startOffset < truncationPoint) {
        if (emphasisMark.isEmpty()) {
          d->m_texts[2].visible(true);
          texts[2]->update(wrath_context, font, textRun, color, startOffset, truncationPoint);
        } else {
          /*
            painting path does drawEmphasisMarks which does nothing 
            in Qt builds anyways..
          */
        }
    }
  }

}
                                           
void InlineTextBox::readyWRATHWidgetCompositionBackground(PaintedWidgetsOfWRATHHandleT<InlineBox> &handle,
                                                          ContextOfWRATH *wrath_context,
                                                          const FloatPoint& boxOrigin, RenderStyle*, 
                                                          const Font&, int startPos, int endPos)
{
  /*
    [WRATH-TODO] from paintCompositionBackground
   */
  WRATH_UNIMPLEMENTED(wrath_context);
}

void InlineTextBox::readyWRATHWidgetDocumentMarkers(PaintedWidgetsOfWRATHHandleT<InlineBox> &handle,
                                                    ContextOfWRATH *wrath_context, const FloatPoint& boxOrigin, 
                                                    RenderStyle*, const Font&, bool background)
{
  /*
    [WRATH-TODO] from paintDocumentMarkers
   */
  WRATH_UNIMPLEMENTED(wrath_context);
}

void InlineTextBox::readyWRATHWidgetCompositionUnderline(PaintedWidgetsOfWRATHHandleT<InlineBox> &handle,
                                                         ContextOfWRATH *wrath_context,
                                                         const FloatPoint& boxOrigin, 
                                                         const CompositionUnderline &underline)
{
    InlineTextBox_CompositionUnderline *d;

    d=InlineTextBox_CompositionUnderline::object(this, handle);
  
    ContextOfWRATH::AutoPushNode autoPushRoot(wrath_context, d->m_root_node);
    ContextOfWRATH::AutoPushNode autoPushSkip(wrath_context, d->m_skip_node);
    
    if (truncation() == cFullTruncation) {
      d->m_skip_node.widget()->visible(false);
      return;
    }

    
    d->m_skip_node.widget()->visible(true);
  
    float istart = 0; // start of line to draw, relative to tx
    float width = logicalWidth(); // how much line to draw
    bool useWholeWidth = true;
    unsigned paintStart = start();
    unsigned paintEnd = end() + 1; // end points at the last char, not past it
    if (paintStart <= underline.startOffset) {
        paintStart = underline.startOffset;
        useWholeWidth = false;
        istart = toRenderText(renderer())->width(start(), paintStart - start(), textPos(), m_firstLine);
    }
    if (paintEnd != underline.endOffset) {      // end points at the last char, not past it
        paintEnd = min(paintEnd, (unsigned)underline.endOffset);
        useWholeWidth = false;
    }
    if (truncation() != cNoTruncation) {
        paintEnd = min(paintEnd, (unsigned)start() + truncation());
        useWholeWidth = false;
    }
    if (!useWholeWidth) {
        width = toRenderText(renderer())->width(paintStart, paintEnd - paintStart, textPos() + istart, m_firstLine);
    }

    // Thick marked text underlines are 2px thick as long as there is room for the 2px line under the baseline.
    // All other marked text underlines are 1px thick.
    // If there's not enough space the underline will touch or overlap characters.
    int lineThickness = 1;
    int baseline = renderer()->style(m_firstLine)->fontMetrics().ascent();
    if (underline.thick && logicalHeight() - baseline >= 2)
        lineThickness = 2;

    // We need to have some space between underlines of subsequent clauses, because some input methods do not use different underline styles for those.
    // We make each line shorter, which has a harmless side effect of shortening the first and last clauses, too.
    istart += 1;
    width -= 2;

    /*
      [WRATH-DANGER]:
        we are going to cache the line by integer rounding of the dimensions,
        note that GraphicsContextQt.cpp does drawLineForText by calling
        drawLine connecting the IntPoint(x,y) to IntPoint(x+width, y), i.e.
        it rounds to int's anyways

        The line to stoke is a horizontal line of length iwidth, it's position is at 
     */

    int iwidth(static_cast<int>(width));
    vec4 c(0.0f, 0.0f, 0.0f, 1.0f);
    underline.color.getRGBA(c.x(), c.y(), c.z(), c.w());
    

    if(!d->m_shape_item.widget() or d->m_width!=iwidth or d->m_thickness!=lineThickness) {
      WRATHShapeF shape;

      shape.current_outline() << WRATHOutline<float>::position_type(0, 0)
                              << WRATHOutline<float>::position_type(iwidth, 0);

      d->m_width=iwidth;
      d->m_thickness=lineThickness;

      if(!d->m_shape_item.widget()) {
        wrath_context->add_stroked_shape(d->m_shape_item,
                                         WRATHWidgetGenerator::ColorProperties(c),
                                         WRATHWidgetGenerator::shape_value(shape),
                                         WRATHWidgetGenerator::StrokingParameters()
                                         .close_outline(false)
                                         .width(lineThickness)
                                         .cap_style(WRATHWidgetGenerator::flat_cap));
      } else {
        d->m_shape_item.widget()->properties()->change_shape(WRATHWidgetGenerator::shape_value(shape),
                                                             WRATHWidgetGenerator::StrokingParameters()
                                                             .close_outline(false)
                                                             .width(lineThickness)
                                                             .cap_style(WRATHWidgetGenerator::flat_cap));
        wrath_context->update_generic(d->m_shape_item);
      }

    } else {
      wrath_context->update_generic(d->m_shape_item);
    }
    
    d->m_shape_item.widget()->node()->color(c);

    vec2 pt0( boxOrigin.x() + istart,
              boxOrigin.y() + logicalHeight() - lineThickness);

    d->m_shape_item.widget()->position(pt0);

    

}

void InlineTextBox::readyWRATHWidgetDecoration(PaintedWidgetsOfWRATHHandleT<InlineBox> &handle,
                                               ContextOfWRATH *wrath_context,
                                               const FloatPoint& boxOrigin, int deco, const ShadowData *shadow)
{
  InlineTextBox_WidgetDecoration *d;

  d=InlineTextBox_WidgetDecoration::object(this, handle);

  ContextOfWRATH::AutoPushNode autoPushRoot(wrath_context, d->m_root_node);
  ContextOfWRATH::AutoPushNode autoPushSkip(wrath_context, d->m_skip_node);

  

  if (truncation() == cFullTruncation) {
    d->m_skip_node.widget()->visible(false);
    return;
  }

  d->m_skip_node.widget()->visible(true);

  FloatPoint localOrigin = boxOrigin;

  float width = logicalWidth();
  if (truncation() != cNoTruncation) {
    width = toRenderText(renderer())->width(start(), truncation(), textPos(), m_firstLine);
    if (!isLeftToRightDirection())
      localOrigin.move(logicalWidth() - width, 0);
  }
  
  // Get the text decoration colors.
  Color underline, overline, linethrough;
  renderer()->getTextDecorationColors(deco, underline, overline, linethrough, true);
  
  // Use a special function for underlines to get the positioning exactly right.
  bool isPrinting = textRenderer()->document()->printing();
  //context->setStrokeThickness(1.0f); // FIXME: We should improve this rule and not always just assume 1.
  
  bool linesAreOpaque = !isPrinting && (!(deco & UNDERLINE) || underline.alpha() == 255) && (!(deco & OVERLINE) || overline.alpha() == 255) && (!(deco & LINE_THROUGH) || linethrough.alpha() == 255);
  
  RenderStyle* styleToUse = renderer()->style(m_firstLine);
  int baseline = styleToUse->fontMetrics().ascent();
  
  bool setClip = false;
  int extraOffset = 0;

  /*************************************
    [WRATH-DANGER] we are not supporting shadows for NOW!! 
  ******************************************/
  shadow=0;

  

  if (!linesAreOpaque && shadow && shadow->next()) {
    FloatRect clipRect(localOrigin, FloatSize(width, baseline + 2));
    for (const ShadowData* s = shadow; s; s = s->next()) {
      FloatRect shadowRect(localOrigin, FloatSize(width, baseline + 2));
      shadowRect.inflate(s->blur());
      int shadowX = isHorizontal() ? s->x() : s->y();
      int shadowY = isHorizontal() ? s->y() : -s->x();
      shadowRect.move(shadowX, shadowY);
      clipRect.unite(shadowRect);
      extraOffset = max(extraOffset, max(0, shadowY) + s->blur());
    }
    
    WRATHBBox<2> wrathClipRect( vec2(clipRect.x(), clipRect.y()),
                                vec2(clipRect.maxX(), clipRect.maxY()));

    
    wrath_context->push_node(d->m_clip_node);
    d->m_clip_node.widget()->clip_rect(wrathClipRect);
    d->m_clip_node.widget()->clipping_active(true);

    extraOffset += baseline + 2;
    localOrigin.move(0, extraOffset);
    setClip = true;
  }
  
  ColorSpace colorSpace = renderer()->style()->colorSpace();
  bool setShadow = false;
  
  do {
    
    if (shadow) {
      if (!shadow->next()) {
        // The last set of lines paints normally inside the clip.
        localOrigin.move(0, -extraOffset);
        extraOffset = 0;
      }
      int shadowX = isHorizontal() ? shadow->x() : shadow->y();
      int shadowY = isHorizontal() ? shadow->y() : -shadow->x();
      //context->setShadow(FloatSize(shadowX, shadowY - extraOffset), shadow->blur(), shadow->color(), colorSpace);
      setShadow = true;
      shadow = shadow->next();
    }

    /**********************************
      [WRATH-DANGER]: when we support shadows,
      the below code will need to observe the different 
      values of shadow correctly.
    **********************************/

    
    d->m_underline.visible(false);
    d->m_overline.visible(false);
    d->m_line_through.visible(false);
    
    if (deco & UNDERLINE) {
      d->m_underline.visible(true);
      InlineTextBox_WidgetDecorationElement::ready(this, d->m_underline, wrath_context,
                                                   FloatPoint(localOrigin.x(), localOrigin.y() + baseline + 1),
                                                   width, underline);
    }

    if (deco & OVERLINE) {
      d->m_overline.visible(true);
      InlineTextBox_WidgetDecorationElement::ready(this, d->m_overline, wrath_context,
                                                   localOrigin, 
                                                   width, overline);
    }

    if (deco & LINE_THROUGH) {
      d->m_line_through.visible(true);
      InlineTextBox_WidgetDecorationElement::ready(this, d->m_line_through, wrath_context,
                                                   FloatPoint(localOrigin.x(), localOrigin.y() + 2 * baseline / 3), 
                                                   width, linethrough);

      
    }
  } while (shadow);
  
  
  if (setClip)
    wrath_context->pop_node();

}

void InlineTextBox::readyWRATHWidgetSelection(PaintedWidgetsOfWRATHHandleT<InlineBox> &handle,
                                              ContextOfWRATH *wrath_context,
                                              const FloatPoint& boxOrigin, RenderStyle *style, 
                                              const Font &fnt)
{
    InlineTextBox_Selection *d;

    d=InlineTextBox_Selection::object(this, handle);
    ContextOfWRATH::AutoPushNode autoPushRoot(wrath_context, d->m_root_node);
    ContextOfWRATH::AutoPushNode autoPushClip(wrath_context, d->m_clip_rect);
    
    d->m_clip_rect.widget()->visible(false);

    // See if we have a selection to paint at all.
    int sPos, ePos;
    selectionStartEnd(sPos, ePos);
    if (sPos >= ePos)
        return;

    Color textColor = style->visitedDependentColor(CSSPropertyColor);
    Color c = renderer()->selectionBackgroundColor();
    if (!c.isValid() || c.alpha() == 0)
        return;

    // If the text color ends up being the same as the selection background, invert the selection
    // background.  This should basically never happen, since the selection has transparency.
    if (textColor == c)
        c = Color(0xff - c.red(), 0xff - c.green(), 0xff - c.blue());

        
    // If the text is truncated, let the thing being painted in the truncation
    // draw its own highlight.
    int length = truncation() != cNoTruncation ? truncation() : len();
    const UChar* characters = textRenderer()->text()->characters() + start();

    BufferForAppendingHyphen charactersWithHyphen;
    if (ePos == length && hasHyphen()) {
        adjustCharactersAndLengthForHyphen(charactersWithHyphen, style, characters, length);
        ePos = length;
    }

    int deltaY = renderer()->style()->isFlippedLinesWritingMode() ? selectionBottom() - logicalBottom() : logicalTop() - selectionTop();
    int selHeight = selectionHeight();
    FloatPoint localOrigin(boxOrigin.x(), boxOrigin.y() - deltaY);

    FloatRect clipRect(localOrigin, FloatSize(logicalWidth(), selHeight));
    float maxX = floorf(clipRect.maxX());
    clipRect.setX(floorf(clipRect.x()));
    clipRect.setWidth(maxX - clipRect.x());

    ContextOfWRATH::set_clipping(d->m_clip_rect, clipRect);
    
    TextRun text_run(characters, length, textRenderer()->allowTabs(), textPos(), 
                     expansion(), expansionBehavior(), 
                     direction(), dirOverride() || style->visuallyOrdered());

    d->m_rect_item.update(wrath_context, 
                          fnt.selectionRectForText(text_run, localOrigin, selHeight, sPos, ePos),
                          c, CompositeSourceOver);
                             
}

void InlineTextBox::readyWRATHWidgetSpellingOrGrammarMarker(PaintedWidgetsOfWRATHHandleT<InlineBox> &handle,
                                                            ContextOfWRATH *wrath_context, 
                                                            const FloatPoint& boxOrigin, const DocumentMarker&, 
                                                            RenderStyle*, const Font&, bool grammar)
{
  WRATH_UNIMPLEMENTED(wrath_context);
}

void InlineTextBox::readyWRATHWidgetTextMatchMarker(PaintedWidgetsOfWRATHHandleT<InlineBox> &handle,
                                                    ContextOfWRATH *wrath_context,
                                                    const FloatPoint& boxOrigin, const DocumentMarker&, 
                                                    RenderStyle*, const Font&)
{
  WRATH_UNIMPLEMENTED(wrath_context);
}

void InlineTextBox::readyWRATHWidgets(PaintedWidgetsOfWRATHHandleT<InlineBox> &handle, 
                                      PaintInfoOfWRATH &paintInfo, int tx, int ty, int lineTop, int lineBottom)
{
  InlineTextBox_WRATHWidgets *d;

  d=InlineTextBox_WRATHWidgets::object(this, handle);
  ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);
  ContextOfWRATH::AutoPushNode autoPushSkip(paintInfo.wrath_context, d->m_skip_node);

  d->m_skip_node.widget()->visible(true);

  

  if (isLineBreak() || !paintInfo.shouldPaintWithinRoot(renderer()) || renderer()->style()->visibility() != VISIBLE ||
      truncation() == cFullTruncation || paintInfo.phase == PaintPhaseOutline || !len()) {
    d->m_skip_node.widget()->visible(false);
    return;
  }

  ASSERT(paintInfo.phase != PaintPhaseSelfOutline && paintInfo.phase != PaintPhaseChildOutlines);

  int logicalLeftSide = logicalLeftVisualOverflow();
  int logicalRightSide = logicalRightVisualOverflow();
  int logicalStart = logicalLeftSide + (isHorizontal() ? tx : ty);
  int logicalExtent = logicalRightSide - logicalLeftSide;
  
  int paintEnd = isHorizontal() ? paintInfo.rect.maxX() : paintInfo.rect.maxY();
  int paintStart = isHorizontal() ? paintInfo.rect.x() : paintInfo.rect.y();
  
  if (logicalStart >= paintEnd || logicalStart + logicalExtent <= paintStart) {
    d->m_skip_node.widget()->visible(false);
    return;
  }
  
  bool isPrinting = textRenderer()->document()->printing();
  
  // Determine whether or not we're selected.
  bool haveSelection = !isPrinting && paintInfo.phase != PaintPhaseTextClip && selectionState() != RenderObject::SelectionNone;
  if (!haveSelection && paintInfo.phase == PaintPhaseSelection) {
    // When only painting the selection, don't bother to paint if there is none.
    d->m_skip_node.widget()->visible(false);
    return;
  }
  
  if (truncation() != cNoTruncation) {
    if (renderer()->containingBlock()->style()->isLeftToRightDirection() != isLeftToRightDirection()) {
      int widthOfVisibleText = toRenderText(renderer())->width(start(), truncation(), textPos(), m_firstLine);
      int widthOfHiddenText = logicalWidth() - widthOfVisibleText;
      // FIXME: The hit testing logic also needs to take this translation int account.
      if (isHorizontal())
        tx += isLeftToRightDirection() ? widthOfHiddenText : -widthOfHiddenText;
      else
        ty += isLeftToRightDirection() ? widthOfHiddenText : -widthOfHiddenText;
    }
  }

  
  RenderStyle* styleToUse = renderer()->style(m_firstLine);
    
  ty -= styleToUse->isHorizontalWritingMode() ? 0 : logicalHeight();
  
  FloatPoint boxOrigin = locationIncludingFlipping();
  boxOrigin.move(tx, ty);    
  FloatRect boxRect(boxOrigin, IntSize(logicalWidth(), logicalHeight()));

  RenderCombineText* combinedText = styleToUse->hasTextCombine() && textRenderer()->isCombineText() && toRenderCombineText(textRenderer())->isCombined() ? toRenderCombineText(textRenderer()) : 0;
  
  bool shouldRotate = !isHorizontal() && !combinedText;

  

  if (shouldRotate) {
    /*
      [WRATH-TODO]: we need to make it so that the text is drawn
      vertically and do all 90 degree rotation our selves
      OR we need to add to ContextOfWRATH ability to rotate by 90 degrees
      by pushing a node.
      context->concatCTM(rotation(boxRect, Clockwise));
    */
    std::cout << "\n[WRATH-DANGER]: shouldRotate in InlineTextBox::readyWRATHWidgets not supported";
  }

  // Determine whether or not we have composition underlines to draw.
  bool containsComposition = renderer()->node() && renderer()->frame()->editor()->compositionNode() == renderer()->node();
  bool useCustomUnderlines = containsComposition && renderer()->frame()->editor()->compositionUsesCustomUnderlines();

  // Set our font.
  const Font& font = styleToUse->font();

  FloatPoint textOrigin = FloatPoint(boxOrigin.x(), boxOrigin.y() + font.fontMetrics().ascent());

  if (combinedText)
    combinedText->adjustTextOrigin(textOrigin, boxRect);

  // 1. Paint backgrounds behind text if needed. Examples of such backgrounds include selection
  // and composition underlines.
  d->m_composition_background.visible(false);
  d->m_document_marker.visible(false);
  d->m_paint_selection.visible(false);

  if (paintInfo.phase != PaintPhaseSelection && paintInfo.phase != PaintPhaseTextClip && !isPrinting) {
    if (containsComposition && !useCustomUnderlines) {

      d->m_composition_background.visible(true);
      readyWRATHWidgetCompositionBackground(d->m_composition_background,
                                            paintInfo.wrath_context, boxOrigin, styleToUse, font,
                                            renderer()->frame()->editor()->compositionStart(),
                                            renderer()->frame()->editor()->compositionEnd());
    } 

    d->m_document_marker.visible(true);
    readyWRATHWidgetDocumentMarkers(d->m_document_marker, paintInfo.wrath_context, boxOrigin, styleToUse, font, true);

    if (haveSelection && !useCustomUnderlines) {
      d->m_paint_selection.visible(true);
      readyWRATHWidgetSelection(d->m_paint_selection, paintInfo.wrath_context,
                                boxOrigin, styleToUse, font);
    } 
  }

  // 2. Now paint the foreground, including text and decorations like underline/overline (in quirks mode only).
  Color textFillColor;
  Color textStrokeColor;
  Color emphasisMarkColor;
  float textStrokeWidth = styleToUse->textStrokeWidth();
  const ShadowData* textShadow = paintInfo.forceBlackText ? 0 : styleToUse->textShadow();
  
  if (paintInfo.forceBlackText) {
    textFillColor = Color::black;
    textStrokeColor = Color::black;
    emphasisMarkColor = Color::black;
  } else {
    textFillColor = styleToUse->visitedDependentColor(CSSPropertyWebkitTextFillColor);
    
    // Make the text fill color legible against a white background
    if (styleToUse->forceBackgroundsToWhite())
      textFillColor = correctedTextColor(textFillColor, Color::white);
    
    textStrokeColor = styleToUse->visitedDependentColor(CSSPropertyWebkitTextStrokeColor);
    
    // Make the text stroke color legible against a white background
    if (styleToUse->forceBackgroundsToWhite())
      textStrokeColor = correctedTextColor(textStrokeColor, Color::white);
    
    emphasisMarkColor = styleToUse->visitedDependentColor(CSSPropertyWebkitTextEmphasisColor);
    
    // Make the text stroke color legible against a white background
    if (styleToUse->forceBackgroundsToWhite())
      emphasisMarkColor = correctedTextColor(emphasisMarkColor, Color::white);
  }
  
  bool paintSelectedTextOnly = (paintInfo.phase == PaintPhaseSelection);
  bool paintSelectedTextSeparately = false;
  
  Color selectionFillColor = textFillColor;
  Color selectionStrokeColor = textStrokeColor;
  Color selectionEmphasisMarkColor = emphasisMarkColor;
  float selectionStrokeWidth = textStrokeWidth;
  const ShadowData* selectionShadow = textShadow;
  if (haveSelection) {
    // Check foreground color first.
    Color foreground = paintInfo.forceBlackText ? Color::black : renderer()->selectionForegroundColor();
    if (foreground.isValid() && foreground != selectionFillColor) {
      if (!paintSelectedTextOnly)
        paintSelectedTextSeparately = true;
      selectionFillColor = foreground;
    }
    
    Color emphasisMarkForeground = paintInfo.forceBlackText ? Color::black : renderer()->selectionEmphasisMarkColor();
    if (emphasisMarkForeground.isValid() && emphasisMarkForeground != selectionEmphasisMarkColor) {
      if (!paintSelectedTextOnly)
        paintSelectedTextSeparately = true;
      selectionEmphasisMarkColor = emphasisMarkForeground;
    }
    
    if (RenderStyle* pseudoStyle = renderer()->getCachedPseudoStyle(SELECTION)) {
      const ShadowData* shadow = paintInfo.forceBlackText ? 0 : pseudoStyle->textShadow();
      if (shadow != selectionShadow) {
        if (!paintSelectedTextOnly)
          paintSelectedTextSeparately = true;
        selectionShadow = shadow;
      }
      
      float strokeWidth = pseudoStyle->textStrokeWidth();
      if (strokeWidth != selectionStrokeWidth) {
        if (!paintSelectedTextOnly)
          paintSelectedTextSeparately = true;
        selectionStrokeWidth = strokeWidth;
      }
      
      Color stroke = paintInfo.forceBlackText ? Color::black : pseudoStyle->visitedDependentColor(CSSPropertyWebkitTextStrokeColor);
      if (stroke != selectionStrokeColor) {
        if (!paintSelectedTextOnly)
          paintSelectedTextSeparately = true;
        selectionStrokeColor = stroke;
      }
    }
  }
  
  int length = len();
  const UChar* characters;
  if (!combinedText)
    characters = textRenderer()->text()->characters() + start();
  else
    combinedText->charactersToRender(start(), characters, length);
  
  BufferForAppendingHyphen charactersWithHyphen;
  if (hasHyphen())
    adjustCharactersAndLengthForHyphen(charactersWithHyphen, styleToUse, characters, length);
  
  TextRun textRun(characters, length, textRenderer()->allowTabs(), textPos(), 
                  expansion(), expansionBehavior(), direction(), 
                  dirOverride() || styleToUse->visuallyOrdered());
  
  int sPos = 0;
  int ePos = 0;
  if (paintSelectedTextOnly || paintSelectedTextSeparately)
    selectionStartEnd(sPos, ePos);
  
  if (truncation() != cNoTruncation) {
    sPos = min<int>(sPos, truncation());
    ePos = min<int>(ePos, truncation());
    length = truncation();
  }
  
  int emphasisMarkOffset = 0;
  TextEmphasisPosition emphasisMarkPosition;
  bool hasTextEmphasis = getEmphasisMarkPosition(styleToUse, emphasisMarkPosition);
  const AtomicString& emphasisMark = hasTextEmphasis ? styleToUse->textEmphasisMarkString() : nullAtom;
  if (!emphasisMark.isEmpty())
    emphasisMarkOffset = emphasisMarkPosition == TextEmphasisPositionOver ? 
      -font.fontMetrics().ascent() - font.emphasisMarkDescent(emphasisMark) : 
      font.fontMetrics().descent() + font.emphasisMarkAscent(emphasisMark);
  

  d->m_text1.visible(false);
  d->m_text2.visible(false);

  if (!paintSelectedTextOnly) {
    if (!paintSelectedTextSeparately || ePos <= sPos) {
      d->m_text1.visible(true);
      readyWRATHWidgetTextWithShadow(d->m_text1, this, paintInfo.wrath_context, font, textRun,
                                     nullAtom, 0, 0, length, length, 
                                     textOrigin, boxRect, textShadow, textStrokeWidth > 0, isHorizontal(),
                                     textFillColor, textStrokeColor, textStrokeWidth);
    } else {
      d->m_text2.visible(true);
      readyWRATHWidgetTextWithShadow(d->m_text2, this, paintInfo.wrath_context, font, textRun,
                                     nullAtom, 0, ePos, sPos, length, 
                                     textOrigin, boxRect, textShadow, textStrokeWidth > 0, isHorizontal(),
                                     textFillColor, textStrokeColor, textStrokeWidth);
    }
     /*
        [WRATH-TODO]
        We culled out the entire if(!emphasisMark.isEmpty())
        block since Qt build of WebKit does not draw emphasis marks anyways..
     */
  }

  
  d->m_text3.visible(false);
  if ((paintSelectedTextOnly || paintSelectedTextSeparately) && sPos < ePos) {
    d->m_text3.visible(true);
    readyWRATHWidgetTextWithShadow(d->m_text3, this, paintInfo.wrath_context,
                                   font, textRun, nullAtom, 0, sPos, ePos, length, textOrigin, 
                                   boxRect, selectionShadow, selectionStrokeWidth > 0, isHorizontal(),
                                   selectionFillColor, selectionStrokeColor, selectionStrokeWidth);
    /*
      [WRATH-TODO]:
      We cut out the block !emphasisMark.isEmpty() because Qt build
      of WebKit does not implement drawEmphasisMark.
     */
  }

  
  // Paint decorations
  int textDecorations = styleToUse->textDecorationsInEffect();

  d->m_box_decorations.visible(false);
  if (textDecorations != TDNONE && paintInfo.phase != PaintPhaseSelection) {

    d->m_box_decorations.visible(true);
    readyWRATHWidgetDecoration(d->m_box_decorations, paintInfo.wrath_context, boxOrigin, textDecorations, textShadow);

  } 


  d->m_document_markers.visible(false);
  if (paintInfo.phase == PaintPhaseForeground) {

    d->m_document_markers.visible(true);
    readyWRATHWidgetDocumentMarkers(d->m_document_markers, paintInfo.wrath_context, boxOrigin, styleToUse, font, false);   
    if (useCustomUnderlines) {
      const Vector<CompositionUnderline>& underlines = renderer()->frame()->editor()->customCompositionUnderlines();
      size_t numUnderlines = underlines.size();
      
      d->m_underlines.resize(numUnderlines);

      for (size_t index = 0; index < numUnderlines; ++index) {
        const CompositionUnderline& underline = underlines[index];
        
        d->m_underlines[index].visible(false);

        if (underline.endOffset <= start())
          continue;
        
        if (underline.startOffset <= end()) {
          d->m_underlines[index].visible(true);
          readyWRATHWidgetCompositionUnderline(d->m_underlines[index],
                                               paintInfo.wrath_context, boxOrigin, underline);

          if (underline.endOffset > end() + 1) {
            d->m_underlines.resize(index+1);
            break;
          }
        } else {
          d->m_underlines.resize(index+1);
          break;
        }
      }
    }
    else {
      d->m_underlines.clear();
    }
  }
  
  if (shouldRotate) {
    /*
      [WRATH-TODO]: pop the node we would push for 90 degree rotation.
      context->concatCTM(rotation(boxRect, Counterclockwise));
    */
  }
}

}
#endif
