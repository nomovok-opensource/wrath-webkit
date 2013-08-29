#include "config.h"

#if USE(WRATH)
#include "PaintInfoOfWRATH.h"

namespace
{

  typedef std::pair<GLenum, GLenum> blender_key;

  blender_key blenderKey(WebCore::CompositeOperator op)
  {
    blender_key K(GL_INVALID_ENUM, GL_INVALID_ENUM);
    switch(op)
      {
      case WebCore::CompositeCopy:
        //composite copy rquires NOT blending operation.
        //K=blender_key(GL_ONE, GL_ZERO);
        break;
        
      case WebCore::CompositeSourceOver:
        K=blender_key(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        break;

      case WebCore::CompositeSourceIn:
        K=blender_key(GL_DST_ALPHA, GL_ZERO);
        break;

      case WebCore::CompositeSourceOut:
        K=blender_key(GL_ONE_MINUS_DST_ALPHA, GL_ZERO);
        break;

      case WebCore::CompositeSourceAtop:
        K=blender_key(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;

      case WebCore::CompositeDestinationOver:
        K=blender_key(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
        break;

      case WebCore::CompositeDestinationOut:
        K=blender_key(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
        break;

      case WebCore::CompositeDestinationAtop:
        K=blender_key(GL_ONE_MINUS_DST_ALPHA, GL_SRC_ALPHA);
        break;

      case WebCore::CompositeXOR:
        K=blender_key(GL_ONE_MINUS_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;

      case WebCore::CompositePlusDarker:
        //[WRATH-TODO]: what should we do for this composite operator?
        K=blender_key(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        break;

      case WebCore::CompositePlusLighter:
        //[WRATH-TODO]: what should we do for this composite operator?
        K=blender_key(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        break;

      case WebCore::CompositeHighlight:
        //[WRATH-TODO]: what should we do for this composite operator?
        K=blender_key(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        break;
      
      }
    return K;
  }

  class BlendersFromCompositeModes
  {
  public:
    BlendersFromCompositeModes(void)
    {
      for(int i=0; i<WebCore::CompositeNumberOperators; ++i)
        {
          blender_key K;

          K=blenderKey(static_cast<enum WebCore::CompositeOperator>(i));
          if(K.first!=GL_INVALID_ENUM and K.second!=GL_INVALID_ENUM)
            {
              m_handles[i]=WRATHNew WRATHGLStateChange::blend_state(K.first, K.second);
            }
        }
    }

    const WRATHGLStateChange::state_change::handle&
    getHandle(enum WebCore::CompositeOperator op) const
    {
      return m_handles[op];
    }

  private:
    vecN<WRATHGLStateChange::state_change::handle, WebCore::CompositeNumberOperators> m_handles;
  };
}

namespace WebCore {

/////////////////////////////////////////
// ContextOfWRATH  methods
const WRATHGLStateChange::state_change::handle&
ContextOfWRATH::
getBlenderFromCompositeOp(enum CompositeOperator op)
{
  WRATHStaticInit();
  static BlendersFromCompositeModes v;
  return v.getHandle(op);
}

void
ContextOfWRATH::
push_canvas_node(DrawnCanvas &q)
{
  WebCore::LayerOfWRATH *parent_layer(canvas());

  ContextOfWRATHBase::push_canvas_node(q);
  parent_layer->inform_push(q.widget()->properties()->contents());
}

//////////////////////////////////
// ContextOfWRATH::CanvasNodeForTransparentSingleton methods
void
ContextOfWRATH::CanvasNodeForTransparentSingleton::
push_details(ContextOfWRATH *pctx)
{
  WebCore::ContextOfWRATH::NodeWidget *prev_parent(pctx->stack_top()); 

  WRATH_PUSH_CANVAS_NODE(pctx, m_canvas);

  //inform of the push so the canvas is drawn in the correct order
  LayerOfWRATH *C(m_canvas.widget()->properties()->contents());

  //set both the transformation modifier and clip drawer
  //as NULL-handles. We do this because we are going
  //to have that m_clip_node's node parent is the parent
  //before the push. We need to do this because the
  //default way WRATHLayer handles getting the clipping
  //from the node is to draw with the stencil buffer the
  //clipping region. Since there is no transformation
  //to worry about, we can skip worrying about drawing
  //the clipping rectangle, but we need to have the node
  //know about the clipping (and local transformation)
  //that data is put to m_clip_node, we make the parent
  //of m_clip_node as whatever the previous parent was.

  /**/
  C->simulation_clip_drawer(WRATHLayerClipDrawer::handle());
  C->simulation_transformation_modifier(WRATHLayer::modelview_matrix,
					WRATHLayerIntermediateTransformation::handle());
  pctx->push_node(m_clip_node);
  m_clip_node.widget()->node()->parent(prev_parent->node());
  /**/
}

void
ContextOfWRATH::CanvasNodeForTransparentSingleton::
pop_details(ContextOfWRATH *pctx)
{
  pctx->pop_node();
  pctx->pop_node();
}

////////////////////////////
// LayerOfWRATH methods
LayerOfWRATH::
LayerOfWRATH(const WRATHTripleBufferEnabler::handle &tr,
	     const WRATHLayerClipDrawer::handle &pclipper):
  WRATHLayer(tr, pclipper)
{
  init();
}

LayerOfWRATH::
LayerOfWRATH(WRATHLayer *pparent, 
	     const WRATHLayerClipDrawer::handle &pclipper):
  WRATHLayer(pparent, pclipper)
{
  init();
}

LayerOfWRATH::
LayerOfWRATH(WRATHLayer *pparent, 
	     enum WRATHLayer::inherit_values_type t,
	     const WRATHLayerClipDrawer::handle &pclipper):
  WRATHLayer(pparent, t, pclipper)
{
  init();
}

LayerOfWRATH::
~LayerOfWRATH()
{
  m_set_child_order_connect.disconnect();
}

void
LayerOfWRATH::
init(void)
{
  /*
    connect to before the ID's get updated so that the changes
    we make to the child order happen.
   */
  m_set_child_order_connect=connect(WRATHTripleBufferEnabler::on_complete_simulation_frame,
  				    WRATHTripleBufferEnabler::pre_update_no_lock,
  				    boost::bind(&LayerOfWRATH::set_child_order, this));
}

void
LayerOfWRATH::
set_child_order(void)
{
  std::vector<WRATHLayer*>::iterator iter, end;
  int current_count;

  for(current_count=0, iter=m_children_to_order.begin(), end=m_children_to_order.end();
      iter!=end; ++iter, ++current_count)
    {
      (*iter)->child_order(current_count);
    }

  /*
    reset the list for the next frame.
   */
  m_children_to_order.clear();
  
}

void
LayerOfWRATH::
inform_push(LayerOfWRATH *q)
{
  m_children_to_order.push_back(q);
}








}

#endif
