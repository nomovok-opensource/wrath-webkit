#include "config.h"
#include "RenderBlock.h"

#include "ColumnInfo.h"
#include "Document.h"
#include "Element.h"
#include "FloatQuad.h"
#include "Frame.h"
#include "FrameView.h"
#include "GraphicsContext.h"
#include "HTMLFormElement.h"
#include "HTMLNames.h"
#include "HitTestResult.h"
#include "InlineIterator.h"
#include "InlineTextBox.h"
#include "PaintInfo.h"
#include "RenderCombineText.h"
#include "RenderFlexibleBox.h"
#include "RenderImage.h"
#include "RenderInline.h"
#include "RenderLayer.h"
#include "RenderMarquee.h"
#include "RenderReplica.h"
#include "RenderTableCell.h"
#include "RenderTextFragment.h"
#include "RenderTheme.h"
#include "RenderView.h"
#include "SelectionController.h"
#include "Settings.h"
#include "TextRun.h"
#include "TransformState.h"
#include <wtf/StdLibExtras.h>

using namespace std;
using namespace WTF;
using namespace Unicode;

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"
#include "HierarchyOfHandlesOfWRATH.h"
#include "HierarchyOfHandlesOfWRATH_ChildList.h"
namespace
{

  

  class RenderBlock_WRATHWidgetPerFloat:
    public WebCore::PaintedWidgetsOfWRATH<RenderBlock_WRATHWidgetPerFloat>
  {
  public:
    vecN<WebCore::PaintedWidgetsOfWRATHHandle, WebCore::NumberPaintPhases> m_per_phase;
  };

  class RenderBlock_WRATHWidgetContents:
    public WebCore::PaintedWidgetsOfWRATH<RenderBlock_WRATHWidgetContents>
  {
  public:
    WebCore::PaintedWidgetsOfWRATHHandle m_line_boxes;
    WebCore::PaintedWidgetsOfWRATHHandle m_widget_children;
  };

  class RenderBlock_WRATHWidgetColumnContents:
    public WebCore::PaintedWidgetsOfWRATH<RenderBlock_WRATHWidgetColumnContents>
  {
  public:
  };

  class RenderBlock_WRATHWidgetColumnRules:
    public WebCore::PaintedWidgetsOfWRATH<RenderBlock_WRATHWidgetColumnRules>
  {
  public:
  };

  class RenderBlock_WRATHWidgetChildren:
    public WebCore::PaintedWidgetsOfWRATH<RenderBlock_WRATHWidgetChildren>
  {
  public:
    RenderBlock_WRATHWidgetChildren(WebCore::RenderBlock *p):
      m_handles(p->children())
    {}

    static
    RenderBlock_WRATHWidgetChildren*
    object(WebCore::RenderBlock *ptr,
           WebCore::PaintedWidgetsOfWRATHHandle &handle)
    {
      return objectPassOwnerToCtor(type_tag<RenderBlock_WRATHWidgetChildren>(), ptr, handle);
    }

    
    WebCore::HierarchyOfHandlesOfWRATHChildList<WebCore::RenderObject> m_handles;
  };
  
  class RenderBlock_WRATHWidgetCaret:
    public WebCore::PaintedWidgetsOfWRATH<RenderBlock_WRATHWidgetCaret>
  {
  public:

    WebCore::PaintedWidgetsOfWRATHHandle m_drag_caret;
    WebCore::PaintedWidgetsOfWRATHHandle m_caret;
  };

  class RenderBlock_WRATHWidgetFloats:
    public WebCore::PaintedWidgetsOfWRATH<RenderBlock_WRATHWidgetFloats>
  {
  public:
    RenderBlock_WRATHWidgetFloats(WebCore::RenderBlock *p)
    {
      /*
        when a float is removed we need to know
       */
      m_connection=p->connectRemoveFloat( boost::bind(&handles_type::removeObject,
                                                      &m_handles, _1));
    }

    ~RenderBlock_WRATHWidgetFloats()
    {
      m_connection.disconnect();
    }

    static
    RenderBlock_WRATHWidgetFloats*
    object(WebCore::RenderBlock *ptr,
           WebCore::PaintedWidgetsOfWRATHHandle &handle)
    {
      return objectPassOwnerToCtor(type_tag<RenderBlock_WRATHWidgetFloats>(), ptr, handle);
    }


    typedef WebCore::HierarchyOfHandlesOfWRATH<WebCore::RenderObject> handles_type;
    WebCore::HierarchyOfHandlesOfWRATH<WebCore::RenderObject> m_handles;
    

  private:
    boost::signals2::connection m_connection;
  };


  class RenderBlock_WRATHWidgetSelection:
    public WebCore::PaintedWidgetsOfWRATH<RenderBlock_WRATHWidgetSelection>
  {
  public:
  };

  class RenderBlock_WRATHWidgetContinuationOutlines:
    public WebCore::PaintedWidgetsOfWRATH<RenderBlock_WRATHWidgetContinuationOutlines>
  {
  public:

    WebCore::HierarchyOfHandlesOfWRATH<WebCore::RenderInline> m_handles;
  };

  class RenderBlock_WRATHWidgets:
    public WebCore::PaintedWidgetsOfWRATH<RenderBlock_WRATHWidgets>
  {
  public:

    WebCore::PaintedWidgetsOfWRATHHandle m_push_pop_contents_clip;
    WebCore::PaintedWidgetsOfWRATHHandle m_ready_widgets_object;
    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_skip_node;
  };

  class RenderBlock_WRATHWidgetsObject:
    public WebCore::PaintedWidgetsOfWRATH<RenderBlock_WRATHWidgetsObject>
  {
  public:

    WebCore::PaintedWidgetsOfWRATHHandle m_box_decorations;
    WebCore::PaintedWidgetsOfWRATHHandle m_column_rules;
    WebCore::PaintedWidgetsOfWRATHHandle m_paint_mask;
    WebCore::PaintedWidgetsOfWRATHHandle m_column_contents;
    WebCore::PaintedWidgetsOfWRATHHandle m_contents;
    WebCore::PaintedWidgetsOfWRATHHandle m_selection;
    WebCore::PaintedWidgetsOfWRATHHandle m_column_contents_floats;
    WebCore::PaintedWidgetsOfWRATHHandle m_floats;
    WebCore::PaintedWidgetsOfWRATHHandle m_outline;
    WebCore::PaintedWidgetsOfWRATHHandle m_inline_renderer_outline;
    WebCore::PaintedWidgetsOfWRATHHandle m_contuniation_outlines;
    WebCore::PaintedWidgetsOfWRATHHandle m_cursor_caret, m_draw_caret;
  };


}

namespace WebCore
{
using namespace HTMLNames;
typedef WTF::HashMap<RenderBlock*, ListHashSet<RenderInline*>*> ContinuationOutlineTableMap;

static ContinuationOutlineTableMap* continuationOutlineTable()
{
    DEFINE_STATIC_LOCAL(ContinuationOutlineTableMap, table, ());
    return &table;
}


void RenderBlock::readyWRATHWidgets(PaintedWidgetsOfWRATHHandle &handle,
                                    PaintInfoOfWRATH &paintInfo, int tx, int ty)
{

  PaintPhase phase = paintInfo.phase;

  RenderBlock_WRATHWidgets *d;
  d=RenderBlock_WRATHWidgets::object(this, handle);
  ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);
  ContextOfWRATH::AutoPushNode autoPushSkip(paintInfo.wrath_context, d->m_skip_node);

  tx += x();
  ty += y();

  ContextOfWRATH::set_clipping(d->m_root_node, paintInfo.rect);
  //ContextOfWRATH::set_clipping(d->m_skip_node, IntRect(tx, ty, width(), height()));


  d->m_skip_node.widget()->visible(false);
  
  if (!isRoot()) {
    IntRect overflowBox = visualOverflowRect();
    flipForWritingMode(overflowBox);
    overflowBox.inflate(maximalOutlineSize(paintInfo.phase));
    overflowBox.move(tx, ty);
    if (!overflowBox.intersects(paintInfo.rect))
      return;
  }
  
  d->m_skip_node.widget()->visible(true);

    
  bool pushedClip = pushContentsClipWRATH(d->m_push_pop_contents_clip, paintInfo, tx, ty);
  readyWRATHWidgetObject(d->m_ready_widgets_object, paintInfo, tx, ty);
  if (pushedClip)
    popContentsClipWRATH(d->m_push_pop_contents_clip, paintInfo, phase, tx, ty);
  
  /*
    [WRATH-TODO]: overlay controls from RenderLayer:
    // Our scrollbar widgets paint exactly when we tell them to, so that they work properly with
    // z-index.  We paint after we painted the background/border, so that the scrollbars will
    // sit above the background/border.
    if (hasOverflowClip() && style()->visibility() == VISIBLE && (phase == PaintPhaseBlockBackground || phase == PaintPhaseChildBlockBackground) && paintInfo.shouldPaintWithinRoot(this))
    layer()->paintOverflowControls(paintInfo.context, tx, ty, paintInfo.rect);
  */  
}


void RenderBlock::readyWRATHWidgetChildren(PaintedWidgetsOfWRATHHandle &handle,
                                           PaintInfoOfWRATH& paintInfo, int tx, int ty)
{

  RenderBlock_WRATHWidgetChildren *d;
  d=RenderBlock_WRATHWidgetChildren::object(this, handle);
  ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

  
  /*
    hide children that are not walked...
   */
  d->m_handles.hideEachObject();

  PaintPhase newPhase = (paintInfo.phase == PaintPhaseChildOutlines) ? PaintPhaseOutline : paintInfo.phase;
  newPhase = (newPhase == PaintPhaseChildBlockBackgrounds) ? PaintPhaseChildBlockBackground : newPhase;
  
  // We don't paint our own background, but we do let the kids paint their backgrounds.
  PaintInfoOfWRATH info(paintInfo);
  info.phase = newPhase;
  info.updatePaintingRootForChildren(this);
  
  // FIXME: Paint-time pagination is obsolete and is now only used by embedded WebViews inside AppKit
  // NSViews.  Do not add any more code for this.
  RenderView* renderView = view();
  bool usePrintRect = !renderView->printRect().isEmpty();
  
  for (RenderBox* child = firstChildBox(); child; child = child->nextSiblingBox()) {        
    // Check for page-break-before: always, and if it's set, break and bail.
    bool checkBeforeAlways = !childrenInline() && (usePrintRect && child->style()->pageBreakBefore() == PBALWAYS);
    if (checkBeforeAlways
        && (ty + child->y()) > paintInfo.rect.y()
        && (ty + child->y()) < paintInfo.rect.maxY()) {
      view()->setBestTruncatedAt(ty + child->y(), this, true);
      d->m_handles.removeNonVisibleHandles();
      return;
    }
    
    if (!child->isFloating() && child->isReplaced() && usePrintRect && child->height() <= renderView->printRect().height()) {
      // Paginate block-level replaced elements.
      if (ty + child->y() + child->height() > renderView->printRect().maxY()) {
        if (ty + child->y() < renderView->truncatedAt())
          renderView->setBestTruncatedAt(ty + child->y(), child);
        // If we were able to truncate, don't paint.
        if (ty + child->y() >= renderView->truncatedAt())
          break;
      }
    }
    
    IntPoint childPoint = flipForWritingMode(child, IntPoint(tx, ty), ParentToChildFlippingAdjustment);
    if (!child->hasSelfPaintingLayer() && !child->isFloating()) {
      PaintedWidgetsOfWRATHHandle &ch(d->m_handles.getHandle(child));
      ch.visible(true);
      child->readyWRATHWidgets(ch, info, childPoint.x(), childPoint.y());
    }

    // Check for page-break-after: always, and if it's set, break and bail.
    bool checkAfterAlways = !childrenInline() && (usePrintRect && child->style()->pageBreakAfter() == PBALWAYS);
    if (checkAfterAlways
        && (ty + child->y() + child->height()) > paintInfo.rect.y()
        && (ty + child->y() + child->height()) < paintInfo.rect.maxY()) {
      view()->setBestTruncatedAt(ty + child->y() + child->height() + max(0, child->collapsedMarginAfter()), this, true);

      d->m_handles.removeNonVisibleHandles();
      return;
    }
  }
  d->m_handles.removeNonVisibleHandles();

}

void RenderBlock::readyWRATHWidgetContents(PaintedWidgetsOfWRATHHandle &handle,
                                           PaintInfoOfWRATH &paintInfo, int tx, int ty)
{


  RenderBlock_WRATHWidgetContents *d;
  d=RenderBlock_WRATHWidgetContents::object(this, handle);
  ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

  d->m_line_boxes.visible(false);
  d->m_widget_children.visible(false);

  if (document()->didLayoutWithPendingStylesheets() && !isRenderView())
    return;


  if (childrenInline()) {
    m_lineBoxes.readyWRATHWidgets(d->m_line_boxes, this, paintInfo, tx, ty);
    d->m_line_boxes.visible(true);
  }
  else {
    readyWRATHWidgetChildren(d->m_widget_children, paintInfo, tx, ty);
    d->m_widget_children.visible(true);
  }
  
}

void RenderBlock::readyWRATHWidgetFloats(PaintedWidgetsOfWRATHHandle &handle,
                                         PaintInfoOfWRATH &paintInfo, int tx, int ty, bool preservePhase)
{
  RenderBlock_WRATHWidgetFloats *d;

  d=RenderBlock_WRATHWidgetFloats::object(this, handle);
  ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

  /*
    hide those objects not "in the loop"
   */
  d->m_handles.hideEachObject();

  if (!m_floatingObjects) {
    return;
  }
  

  FloatingObjectSet& floatingObjectSet = m_floatingObjects->set();
  FloatingObjectSetIterator end = floatingObjectSet.end();
  for (FloatingObjectSetIterator it = floatingObjectSet.begin(); it != end; ++it) {
    FloatingObject* r = *it;
    PaintedWidgetsOfWRATHHandle &r_handle(d->m_handles.getHandle(r->m_renderer));
    RenderBlock_WRATHWidgetPerFloat *rd;
    bool float_visible;

    rd=RenderBlock_WRATHWidgetPerFloat::object(this, r_handle);
    {
      ContextOfWRATH::AutoPushNode autoPushFloatRoot(paintInfo.wrath_context, rd->m_root_node);
      
      // Only paint the object if our m_shouldPaint flag is set.
      float_visible=r->m_shouldPaint && !r->m_renderer->hasSelfPaintingLayer();
      r_handle.visible(float_visible);
      
      for(int i=0; i<rd->m_per_phase.size(); ++i)
        {
          rd->m_per_phase[i].visible(false);
        }
      
      if (float_visible) {
        PaintInfoOfWRATH currentPaintInfo(paintInfo);
        currentPaintInfo.phase = preservePhase ? paintInfo.phase : PaintPhaseBlockBackground;
        IntPoint childPoint = flipFloatForWritingMode(r, IntPoint(tx + xPositionForFloatIncludingMargin(r) - r->m_renderer->x(), ty + yPositionForFloatIncludingMargin(r) - r->m_renderer->y()));
        
        
        rd->m_per_phase[currentPaintInfo.phase].visible(true);
        r->m_renderer->readyWRATHWidgets(rd->m_per_phase[currentPaintInfo.phase], currentPaintInfo, 
                                         childPoint.x(), childPoint.y());      
        if (!preservePhase) {
          currentPaintInfo.phase = PaintPhaseChildBlockBackgrounds;
          rd->m_per_phase[currentPaintInfo.phase].visible(true);
          r->m_renderer->readyWRATHWidgets(rd->m_per_phase[currentPaintInfo.phase],
                                           currentPaintInfo, childPoint.x(), childPoint.y());
          
          currentPaintInfo.phase = PaintPhaseFloat;
          rd->m_per_phase[currentPaintInfo.phase].visible(true);
          r->m_renderer->readyWRATHWidgets(rd->m_per_phase[currentPaintInfo.phase],
                                           currentPaintInfo, childPoint.x(), childPoint.y());
          
          currentPaintInfo.phase = PaintPhaseForeground;
          rd->m_per_phase[currentPaintInfo.phase].visible(true);
          r->m_renderer->readyWRATHWidgets(rd->m_per_phase[currentPaintInfo.phase],
                                           currentPaintInfo, childPoint.x(), childPoint.y());
          
          currentPaintInfo.phase = PaintPhaseOutline;
          rd->m_per_phase[currentPaintInfo.phase].visible(true);
          r->m_renderer->readyWRATHWidgets(rd->m_per_phase[currentPaintInfo.phase],
                                           currentPaintInfo, childPoint.x(), childPoint.y());
        }
      }
    }
  }
}


void RenderBlock::readyWRATHWidgetColumnContents(PaintedWidgetsOfWRATHHandle &handle,
                                                 PaintInfoOfWRATH &paintInfo, int tx, int ty, bool paintFloats)
{
  /*
    [WRATH-TODO] from paintColumnContents
   */
  WRATH_UNIMPLEMENTED(paintInfo.wrath_context);
}

void RenderBlock::readyWRATHWidgetColumnRules(PaintedWidgetsOfWRATHHandle &column_rules_handle,
                                              PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
  /*
    [WRATH-TODO] from paintColumnRules.
   */
  WRATH_UNIMPLEMENTED(paintInfo.wrath_context);  
}

void RenderBlock::readyWRATHWidgetSelection(PaintedWidgetsOfWRATHHandle &handle,
                                            PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
  /*
    [WRATH-TODO] from paintSelection
   */
  WRATH_UNIMPLEMENTED(paintInfo.wrath_context);
}

void RenderBlock::readyWRATHWidgetContinuationOutlines(PaintedWidgetsOfWRATHHandle &handle,
                                                       PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
  RenderBlock_WRATHWidgetContinuationOutlines *d;

  d=RenderBlock_WRATHWidgetContinuationOutlines::object(this, handle);
  ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);
  
  d->m_handles.hideEachObject();


  ContinuationOutlineTableMap* table = continuationOutlineTable();
  if (table->isEmpty()) {
    return;
  }
  
  ListHashSet<RenderInline*>* continuations = table->get(this);
  if (!continuations) {
    return;
  }

    
  // Paint each continuation outline.
  ListHashSet<RenderInline*>::iterator end = continuations->end();
  for (ListHashSet<RenderInline*>::iterator it = continuations->begin(); it != end; ++it) {
    // Need to add in the coordinates of the intervening blocks.
    RenderInline* flow = *it;
    RenderBlock* block = flow->containingBlock();
    for ( ; block && block != this; block = block->containingBlock()) {
      tx += block->x();
      ty += block->y();
    }
    ASSERT(block); 
    flow->readyWRATHWidgetOutline(d->m_handles.getHandle(flow), paintInfo.wrath_context, tx, ty);
  }
  
  // Delete
  delete continuations;
  table->remove(this);
}

void RenderBlock::readyWRATHWidgetCaret(PaintedWidgetsOfWRATHHandle &handle,
                                        PaintInfoOfWRATH &paintInfo, int tx, int ty, CaretType type)
{
  SelectionController* selection = type == CursorCaret ? frame()->selection() : frame()->page()->dragCaretController();
  RenderBlock_WRATHWidgetCaret *d;

  d=RenderBlock_WRATHWidgetCaret::object(this, handle);
  ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

  
  d->m_caret.visible(false);
  d->m_drag_caret.visible(false);

  // Paint the caret if the SelectionController says so or if caret browsing is enabled
  bool caretBrowsing = frame()->settings() && frame()->settings()->caretBrowsingEnabled();
  RenderObject* caretPainter = selection->caretRenderer();
  if (caretPainter == this && (selection->isContentEditable() || caretBrowsing)) {
    // Convert the painting offset into the local coordinate system of this renderer,
    // to match the localCaretRect computed by the SelectionController
    offsetForContents(tx, ty);
    
    if (type == CursorCaret) {
      d->m_caret.visible(true);
      frame()->selection()->readyWRATHWidgetCaret(this, d->m_caret, paintInfo, tx, ty);
    }
    else {
      d->m_drag_caret.visible(true);
      frame()->selection()->readyWRATHWidgetDragCaret(this, d->m_drag_caret, paintInfo, tx, ty);
    }
  } 
  
}


void RenderBlock::readyWRATHWidgetObject(PaintedWidgetsOfWRATHHandle &handle,
                                         PaintInfoOfWRATH &paintInfo, int tx, int ty)
{
  
  RenderBlock_WRATHWidgetsObject *d;
  PaintPhase paintPhase = paintInfo.phase;

  d=RenderBlock_WRATHWidgetsObject::object(this, handle);

  ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

  d->m_box_decorations.visible(false);
  d->m_column_rules.visible(false);
  d->m_paint_mask.visible(false);
  d->m_contents.visible(false);
  d->m_column_contents.visible(false);
  d->m_selection.visible(false);
  d->m_floats.visible(false);
  d->m_column_contents_floats.visible(false);
  d->m_outline.visible(false);
  d->m_inline_renderer_outline.visible(false);        
  d->m_contuniation_outlines.visible(false);
  d->m_cursor_caret.visible(false);
  d->m_draw_caret.visible(false);

  // 1. paint background, borders etc
  if ((paintPhase == PaintPhaseBlockBackground || paintPhase == PaintPhaseChildBlockBackground) && style()->visibility() == VISIBLE) {

    if (hasBoxDecorations()) {
      d->m_box_decorations.visible(true);
      readyWRATHWidgetBoxDecorations(d->m_box_decorations, paintInfo, tx, ty);
    }
    
    if (hasColumns()) {
      d->m_column_rules.visible(true);
      readyWRATHWidgetColumnRules(d->m_column_rules, paintInfo, tx, ty);
    }
  }

  if (paintPhase == PaintPhaseMask && style()->visibility() == VISIBLE) {  
    d->m_paint_mask.visible(true);
    readyWRATHWidgetMask(d->m_paint_mask, paintInfo, tx, ty);
    return;
  }

  // We're done.  We don't bother painting any children.
  if (paintPhase == PaintPhaseBlockBackground) {
    return;
  }
  
  // Adjust our painting position if we're inside a scrolled layer (e.g., an overflow:auto div).
  int scrolledX = tx;
  int scrolledY = ty;
  if (hasOverflowClip()) {
    IntSize offset = layer()->scrolledContentOffset();
    scrolledX -= offset.width();
    scrolledY -= offset.height();
  }
  
  // 2. paint contents
  if (paintPhase != PaintPhaseSelfOutline) {
    if (hasColumns()) {
      readyWRATHWidgetColumnContents(d->m_column_contents, paintInfo, scrolledX, scrolledY);
      d->m_column_contents.visible(true);
    }
    else {
      readyWRATHWidgetContents(d->m_contents, paintInfo, scrolledX, scrolledY);
      d->m_contents.visible(true);
    }
  }
  
  if (!hasColumns()) {
    d->m_selection.visible(true);
    readyWRATHWidgetSelection(d->m_selection, paintInfo, scrolledX, scrolledY); 
  }


  // 4. paint floats.
  if (paintPhase == PaintPhaseFloat || paintPhase == PaintPhaseSelection || paintPhase == PaintPhaseTextClip) {
    if (hasColumns()) {
      readyWRATHWidgetColumnContents(d->m_column_contents_floats, paintInfo, scrolledX, scrolledY, true);
      d->m_column_contents_floats.visible(true);
    }
    else {
      readyWRATHWidgetFloats(d->m_floats, paintInfo, scrolledX, scrolledY, 
                             paintPhase == PaintPhaseSelection || paintPhase == PaintPhaseTextClip);
      d->m_floats.visible(true);
    }
  }

  

  // 5. paint outline.
  if ((paintPhase == PaintPhaseOutline || paintPhase == PaintPhaseSelfOutline) && hasOutline() && style()->visibility() == VISIBLE) {
    d->m_outline.visible(true);
    readyWRATHWidgetOutline(d->m_outline, paintInfo.wrath_context, tx, ty, width(), height());
  }


  // 6. paint continuation outlines.
  if ((paintPhase == PaintPhaseOutline || paintPhase == PaintPhaseChildOutlines)) {
    RenderInline* inlineCont = inlineElementContinuation();
    if (inlineCont && inlineCont->hasOutline() && inlineCont->style()->visibility() == VISIBLE) {
      RenderInline* inlineRenderer = toRenderInline(inlineCont->node()->renderer());
      RenderBlock* cb = containingBlock();
      
      bool inlineEnclosedInSelfPaintingLayer = false;
      for (RenderBoxModelObject* box = inlineRenderer; box != cb; box = box->parent()->enclosingBoxModelObject()) {
        if (box->hasSelfPaintingLayer()) {
          inlineEnclosedInSelfPaintingLayer = true;
          break;
        }
      }
      
      if (!inlineEnclosedInSelfPaintingLayer) {
        cb->addContinuationWithOutline(inlineRenderer);
      }
      else if (!inlineRenderer->firstLineBox()) {
        inlineRenderer->readyWRATHWidgetOutline(d->m_inline_renderer_outline, paintInfo.wrath_context,
						tx - x() + inlineRenderer->containingBlock()->x(),
						ty - y() + inlineRenderer->containingBlock()->y());
        d->m_inline_renderer_outline.visible(true);
      }
    }
    d->m_contuniation_outlines.visible(true);
    readyWRATHWidgetContinuationOutlines(d->m_contuniation_outlines, paintInfo, tx, ty);
  }
  
  // 7. paint caret.
  // If the caret's node's render object's containing block is this block, and the paint action is PaintPhaseForeground,
  // then paint the caret.
  if (paintPhase == PaintPhaseForeground) {    
    d->m_cursor_caret.visible(true);
    d->m_draw_caret.visible(true);
    
    readyWRATHWidgetCaret(d->m_cursor_caret, paintInfo, scrolledX, scrolledY, CursorCaret);
    readyWRATHWidgetCaret(d->m_draw_caret, paintInfo, scrolledX, scrolledY, DragCaret);
  }
}

}
#endif 

