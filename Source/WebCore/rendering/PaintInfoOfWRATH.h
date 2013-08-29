/*  -*- C++ -*- */
#ifndef _PaintInfoOfWRATH_
#define _PaintInfoOfWRATH_

#include "PaintInfoOfWRATHForwardDeclare.h"
#include "GraphicsTypes.h"

#include "WRATHLayerItemWidgetsTranslate.hpp"
#include "WRATHBBox.hpp"
#include "PaintInfo.h"
#include <boost/signals2.hpp>
#include <boost/bind.hpp>

#include <iostream>

//#ifndef NDEBUG
#define WRATH_SHOW_UNIMPEMENTED
//#endif

//#ifndef NDEBUG
#define WRATH_SHOW_CANVAS_PUSH
//#endif


#ifdef WRATH_SHOW_UNIMPEMENTED
#define WRATH_UNIMPLEMENTED(ctx) do {           \
  (ctx)->note_unimplemented(__PRETTY_FUNCTION__, __FILE__, __LINE__); \
  } while(0)
#else
  #define WRATH_UNIMPLEMENTED do {} while(0)
#endif

/*
  USE WRATH_SHOW_CANVAS_PUSH(context, node) 
  to push canvas nodes as it records the
  line and file where the canvas node
  is pushed
 */
#ifdef WRATH_SHOW_CANVAS_PUSH
#define WRATH_PUSH_CANVAS_NODE(ctx, node) do {		\
    WebCore::ContextOfWRATH *ptctx(ctx);		\
    ptctx->push_canvas_node(node);			\
    ptctx->note_canvas_push(__FILE__, __LINE__);	\
    } while(0)
#else
#define WRATH_PUSH_CANVAS_NODE(ctx, node) do {		\
    WebCore::ContextOfWRATH *ptctx(ctx);		\
    ptctx->push_canvas_node(node);			\
  } while(0)
#endif

/*
  WRATH_PUSH_CANVAS_NODE_TRANSPARENT is for pushing
  a node that is to draw one transparent item, to avoid
  excess drawing to stencil buffer, it disables the
  clip drawer but pushes a node whose clipping is
  that from the current head. Note that
  WRATH_PUSH_CANVAS_NODE_TRANSPARENT pushes _two_ nodes
 */
#define WRATH_PUSH_CANVAS_NODE_TRANSPARENT(ctx, tr_node) do {	\
    tr_node.push_details(ctx);					\
  } while(0)

/*
  the pop command that corresponds to WRATH_PUSH_CANVAS_NODE_TRANSPARENT
 */
#define WRATH_POP_CANVAS_NODE_TRANSPARENT(ctx, tr_node) do { \
    tr_node.pop_details(ctx);				     \
  } while(0)

namespace WebCore {
  
/*
  LayerOfWRATH uses a sorter which
  sorts transparent items by a z-value
*/
class LayerOfWRATH:public WRATHLayer
{
public:

  explicit
  LayerOfWRATH(const WRATHTripleBufferEnabler::handle &tr,
               const WRATHLayerClipDrawer::handle &pclipper=
               WRATHLayerClipDrawer::handle());

  explicit
  LayerOfWRATH(WRATHLayer *pparent, 
               const WRATHLayerClipDrawer::handle &pclipper=
               WRATHLayerClipDrawer::handle());

  LayerOfWRATH(WRATHLayer *pparent, 
               enum WRATHLayer::inherit_values_type t,
               const WRATHLayerClipDrawer::handle &pclipper=
               WRATHLayerClipDrawer::handle());

  ~LayerOfWRATH();

  void
  inform_push(LayerOfWRATH*);

private:
  /*
    handling transparencies between layers.
    WRATHLayer draws contents as follows:

    0) set stencil value an text
    1) draw to depth buffer only clip out items
    2) draw opaque items
    3) recurse to children
    4) draw transparent items

    However we want transparent items to be drawn _in_
    the exact order they are added via the ContextOfWRATH.
    
    We get out of this in a quite simple manner: all transparent
    items are each in a unique LayerOfWRATH. These
    LayerOfWRATH objects do NOT have a clip drawer
    and also are tweaked so that they also do not
    observe the clipping from the node structure.
    All we need to make sure then is that they are 
    drawn in the exact correct order. This is handled
    by the function inform_push(LayerOfWRATH*) which
    appends the argument to an internal list which
    is then walked to assign the child orders.
   */

  void
  init(void);

  void
  set_child_order(void);

  WRATHTripleBufferEnabler::connect_t m_set_child_order_connect;
  std::vector<WRATHLayer*> m_children_to_order;
};

class RenderObject;

typedef WRATHLayerItemNodeTranslate NodeOfWRATH;
typedef WRATHLayerNodeValuePackerUniformArrays NodePackerOfWRATH;

typedef WRATHLayerItemWidget<NodeOfWRATH,  //base node type
                             NodePackerOfWRATH, //packer type
                             LayerOfWRATH>::Generator ContextOfWRATHBase;



/*
  ContextOfWRATH is a WRATHWidgetGenerator via
  typedef ContextOfWRATHBase, but also provides
  tracking data of when canvas nodes are
  pushed in member variable m_canvas_pushes
 */
class ContextOfWRATH:public ContextOfWRATHBase
{
public:

  /*
    type to be used with
     WRATH_PUSH_CANVAS_NODE_TRANSPARENT and WRATH_POP_CANVAS_NODE_TRANSPARENT
   */
  class CanvasNodeForTransparentSingleton
  {
  public:
    bool
    active(void)
    {
      return m_canvas.widget();
    }

    void
    delete_widget(void)
    {
      m_canvas.delete_widget();
      m_clip_node.delete_widget();
    }

    void
    push_details(ContextOfWRATH *ctx);

    void
    pop_details(ContextOfWRATH *ctx);


  private:
    DrawnCanvas::AutoDelete m_canvas;
    NodeHandle::AutoDelete m_clip_node;
  };
  

  /////////////////////////////////////////////////////////////////////////////
  // BEGIN DEBUG-STATS CODE, only needed for understanding draw counts, ect
  
  class Counter
  {
  public:
    Counter(void):
      m_count(0)
    {}

    unsigned int m_count;
  };

  class Location:public std::pair<std::string, int>
  {
  public:
    Location(const char *pfile, int pline):
      std::pair<std::string, int>(pfile, pline)
    {}

    Location(void)
    {}

    const std::string& file(void) const { return first; }
    int line(void) const { return second; }
  };

  class NamedLocation:public std::pair<Location, std::string>
  {
  public:
    NamedLocation(const char *pfunc,
                  const char *pfile,
                  int pline):
      std::pair<Location, std::string>(Location(pfile, pline), pfunc)
    {}

    const std::string& file(void) const { return first.first; }
    int line(void) const { return first.second; }
    const std::string& function(void) const { return second; }
  };

  typedef std::map<Location, Counter> LocationCounterRecord;
  typedef std::map<NamedLocation, Counter> NamedLocationCounterRecord;
  
  LocationCounterRecord m_canvas_pushes; // updated via note_canvas_push()
  NamedLocationCounterRecord m_unimplementeds; // updated via note_unimplemented

  int m_number_transparent_rects;
  int m_number_transparent_images;

  void // used by the macro WRATH_PUSH_CANVAS_NODE to record a canvas node push.
  note_canvas_push(const char *file, int line)
  {
    ++m_canvas_pushes[Location(file, line)].m_count;
  }

  void // used by mactro WRATH_UNIMPLEMENTED to record unimplemented calls
  note_unimplemented(const char *func, const char *file, int line)
  {
    ++m_unimplementeds[ NamedLocation(func, file, line) ].m_count;
  }

  // END DEBUG-STATS CODE, only needed for understanding draw counts, ect
  ///////////////////////////////////////////////////////////////////


  ContextOfWRATH(NodeWidget *proot_widget, int &pz):
    ContextOfWRATHBase(proot_widget, pz),
    m_number_transparent_rects(0),
    m_number_transparent_images(0)
  {
  }

  ContextOfWRATH(Canvas *pCanvas,
                 NodeHandle &proot_widget, int &pz):
    ContextOfWRATHBase(pCanvas, proot_widget, pz),
    m_number_transparent_rects(0),
    m_number_transparent_images(0)
  {
  }

  /*
    WRATHWidgetGenerator defines the function
    push_canvas_node(), ContextOfWRATH is re-implementing,
    although it is not virtual to make sure
    that inform_push() gets called.
   */
  void
  push_canvas_node(DrawnCanvas &canvas);

  /*
    \param clipping_active if true, turns clipping on
    \param clipRect specifies the clipping rectangle
    \param shouldPaint value to which to set visible of the node
    \param hnd handle, can be a handle to a Canvas, Node, or
                       item.
   */
  template<typename T>
  static
  void
  set_clipping(bool clipping_active,
               T &hnd,
               const IntRect &clipRect, bool shouldPaint=true)
  {
    WRATHassert(hnd.widget());
    hnd.widget()->node()->clipping_active(clipping_active);
    hnd.widget()->visible(shouldPaint && !clipRect.isEmpty());
    if(hnd.widget()->visible() and clipping_active)
      {
        vec2 c0(clipRect.x(), clipRect.y());
        vec2 c1(clipRect.maxX(), clipRect.maxY());

        WRATHBBox<2> bbox(c0, c1);
        hnd.widget()->clip_rect(bbox);
        
      }
  }

  /*
    same as set_clipping(true, hnd, clipRect, shouldPaint)
   */
  template<typename T>
  static
  void
  set_clipping(T &hnd,
               const IntRect &clipRect, bool shouldPaint=true)
  {
    set_clipping(true, hnd, clipRect, shouldPaint);
  }

  /*
    \param clipping_active if true, turns clipping on
    \param clipRect specifies the clipping rectangle
    \param shouldPaint value to which to set visible of the node
    \param hnd handle, can be a handle to a Canvas, Node, or
                       item.
   */
  template<typename T>
  static
  void
  set_clipping(bool clipping_active,
               T &hnd,
               const FloatRect &clipRect, bool shouldPaint=true)
  {
    WRATHassert(hnd.widget());
    hnd.widget()->node()->clipping_active(clipping_active);
    hnd.widget()->visible(shouldPaint && !clipRect.isEmpty());
    if(hnd.widget()->visible() and clipping_active)
      {
        vec2 c0(clipRect.x(), clipRect.y());
        vec2 c1(clipRect.maxX(), clipRect.maxY());

        WRATHBBox<2> bbox(c0, c1);
        hnd.widget()->clip_rect(bbox);
        
      }
  }

  /*
    same as set_clipping(true, hnd, clipRect, shouldPaint)
   */
  template<typename T>
  static
  void
  set_clipping(T &hnd,
               const FloatRect &clipRect, bool shouldPaint=true)
  {
    set_clipping(true, hnd, clipRect, shouldPaint);
  }

  /*
    produce a float4x4 (type defined in WRATH headers)
    from a WebCore::TransformationMatrix.
   */
  static
  void
  wrath_matrix(float4x4 &transformWRATH,
               const TransformationMatrix &transform)
  {
    for(int row=0; row<4; ++row)
      {
        for(int col=0;col<4;++col)
          {
            transformWRATH(row, col)=transform.row_column(row, col);
          }
      }
  }

  /*
    produce a float4x4 (type defined in WRATH headers)
    from a WebCore::TransformationMatrix.
   */
  static
  float4x4
  wrath_matrix(const TransformationMatrix &transform)
  {
    float4x4 R;

    wrath_matrix(R, transform);
    return R;
  }

  /*
    Get the GL-state change object from a
    composite operator. Note that if no
    blending is required, then the return
    will be a invalid handle (i.e. a NULL
    handle).
   */
  static
  const WRATHGLStateChange::state_change::handle&
  getBlenderFromCompositeOp(enum CompositeOperator op);
};

/*
  WRATH analgoue of PaintInfo, inherits publically
  from PaintInfo. However, the GraphicsContext field,
  PaintInfo::context is set as 0. The ContextOfWRATH
  is in the member variable wrath_context
*/
class PaintInfoOfWRATH:public PaintInfo {
public:

  PaintInfoOfWRATH(ContextOfWRATH *newContext,
                 const IntRect& newRect, PaintPhase newPhase, bool newForceBlackText,
                 RenderObject* newPaintingRoot, ListHashSet<RenderInline*>* newOutlineObjects,
                 OverlapTestRequestMap* overlapTestRequests = 0)
    : PaintInfo(0,
                newRect, newPhase, newForceBlackText,
                newPaintingRoot, newOutlineObjects,
                overlapTestRequests)
    , wrath_context(newContext)
  {}


  ContextOfWRATH *wrath_context;
};  

  /*
    The base pimple class. 
   */
  class PaintedWidgetsOfWRATHBase:boost::noncopyable
  {
  public:
    
    virtual
    ~PaintedWidgetsOfWRATHBase()
    {}

    /*
      To be implemented by derived class to make
      the widgets of a draw call visible
      or invisible, default is to be visible.
     */
    virtual
    void
    visible(bool)=0;

    template<typename T>
    T*
    cast(void)
    {
      WRATHassert(dynamic_cast<T*>(this));
      return static_cast<T*>(this);
    }

    /*
      Create a pimple to be handled by a handle
      \tparam T pimple type (must be derived from PaintedWidgetsOfWRATHBase)
      \tparam S template parameter to handle (see PaintedWidgetsOfWRATHHandleT for details)
      \tparam R must be publically derived from S (or can be S itself).
      
      \param owner pointer to object that creates the pimple
      \param handle handle that will hold the pimple

      Given a handle, checks if the handle already has a pimple
      that is associated to owner, if so returns that pimple casted
      to T*. Otherwise creates that pimple passing the owner
      to the ctor of T.
     */
    template<typename T, typename S, typename R>
    static
    T*
    objectPassOwnerToCtor(R *owner,
                          PaintedWidgetsOfWRATHHandleT<S> &handle);

    template<typename T, typename S, typename R>
    static
    T*
    objectPassOwnerToCtor(type_tag<T>, R *owner,
                          PaintedWidgetsOfWRATHHandleT<S> &handle)
    {
      return objectPassOwnerToCtor<T, S, R>(owner, handle);
    }

    /*
      Create a pimple to be handled by a handle
      \tparam T pimple type (must be derived from PaintedWidgetsOfWRATHBase)
      \tparam S template parameter to handle (see PaintedWidgetsOfWRATHHandleT for details)
      \tparam R must be publically derived from S (or can be S itself).
      
      \param owner pointer to object that creates the pimple
      \param handle handle that will hold the pimple

      Given a handle, checks if the handle already has a pimple
      that is associated to owner, if so returns that pimple casted
      to T*. Otherwise creates that pimple passing nothing
      to the ctor of T.
     */
    template<typename T, typename S, typename R>
    static
    T*
    objectNoArgCtor(R *owner,
                    PaintedWidgetsOfWRATHHandleT<S> &handle);

    template<typename T, typename S, typename R>
    static
    T*
    objectNoArgCtor(type_tag<T>, R *owner,
                    PaintedWidgetsOfWRATHHandleT<S> &handle)
    {
      return objectNoArgCtor<T, S, R>(owner, handle);
    }
  };

  /*
    A handle to a pimple (i.e. derived from
    PaintedWidgetsOfWRATHBase. The template
    type is the type that fills the pimple
    and when the object that filled the pimple
    goes out of scope, that pimple is deleted too.

    Moreover, the handle associates the pimple
    to an object, if the association changes
    the pimple is deleted.

    the type T must have the method connect_dtor
    that takes a boost::signals2::sig::slot_type
    where sig is signal taking and returning
    no arguments.
   */
  template<typename T> 
  class PaintedWidgetsOfWRATHHandleT:boost::noncopyable
  {
  public:
    PaintedWidgetsOfWRATHHandleT(void):
      m_data(0),
      m_associate(0),
      m_visible(true)
    {}

    ~PaintedWidgetsOfWRATHHandleT()
    {
      clear();
    }

    /*
      associating to an object O means
      that m_data will be deleted when the 
      that object O is deleted.
     */
    void
    associate(T *q)
    {
      if(m_associate==q) {
        return;
      }
      
      /*
        [WRATH-DANGER] is is really necessary 
        to force reconstruction on association
        change? For that matter, should re-association
        be considered a bug? Also should be
        track the paint phase of the associating too?
      */
      if(m_associate) {
        clear();
      }
      
      m_associate=q;
      if(m_associate) {
        m_connection=m_associate->connect_dtor(boost::bind(&PaintedWidgetsOfWRATHHandleT<T>::clear, this));
      }
    }

    T*
    associate(void)
    {
      return m_associate;
    }

    void
    clear(void)
    {
      if(m_data) {
        WRATHDelete(m_data);
        m_data=0;
        m_connection.disconnect();
      }
    }

    void
    visible(bool v)
    {
      m_visible=v;
      if(m_data) {
        m_data->visible(v);
      }
    }

    bool
    visible(void) const { return m_visible; }

    PaintedWidgetsOfWRATHBase *m_data;

  private:
    T *m_associate;
    boost::signals2::connection m_connection;
    bool m_visible;
    
  };

  template<typename T, typename S, typename R>
  T*
  PaintedWidgetsOfWRATHBase::
  objectPassOwnerToCtor(R *owner,
                        PaintedWidgetsOfWRATHHandleT<S> &handle)
  {
    handle.associate(owner);
    if(!handle.m_data) {
      handle.m_data=WRATHNew T(owner);
      handle.m_data->visible(handle.visible());
    }
    return handle.m_data->cast<T>();
  }

  template<typename T, typename S, typename R>
  T*
  PaintedWidgetsOfWRATHBase::
  objectNoArgCtor(R *owner,
                  PaintedWidgetsOfWRATHHandleT<S> &handle)
  {
    handle.associate(owner);
    if(!handle.m_data) {
      handle.m_data=WRATHNew T();
      handle.m_data->visible(handle.visible());
    }
    return handle.m_data->cast<T>();
  }
  


  /*
    Conveniance class providing the visible() method,
    via a node memeber variable m_root_node.
    Thus always push that node at start of ready widgets.
    The type T is the type argument for the handle type.
    The type S is the derived type. The usage pattern is this:
    \code
    class RenderBarDerived_Foo:
      public PaintedWidgetsOfWRATHT<RenderBar, RenderBarDerived_Foo>
    {
    public:
      RenderBarDerived_Foo(void)
      {

      {

      //custom members
    };

    //RenderBarDerived publically inherits from RenderBar
    void RenderBarDerived::readyWRATHFoo(PaintInfoOfWRATH &paintInfo, 
                                         PaintedWidgetsOfWRATHHandleT<RenderBar> &handle,
                                         other arguments)
    {
       RenderBarDerived_Foo *d;
       d=RenderBarDerived_Foo::object(this, handle);

       
       ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

       
    }
    \endcode
   */
  template<typename T, typename S>
  class PaintedWidgetsOfWRATHT:public PaintedWidgetsOfWRATHBase
  {
  public:

    virtual
    void
    visible(bool v)
    {
      if(m_root_node.widget()) {
        m_root_node.widget()->visible(v);
      }
    }

    static
    S*
    object(T *ptr,
           PaintedWidgetsOfWRATHHandleT<T> &handle)
    {
      return objectNoArgCtor(type_tag<S>(), ptr, handle);
    }

    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_root_node;    
  };

  /*
    Provided to lessen typing where the creating object
    type is derived from RenderObject
   */
  template<typename S>
  class PaintedWidgetsOfWRATH:public PaintedWidgetsOfWRATHT<RenderObject, S>
  {
  };

  /*
    Same use pattern as PaintedWidgetsOfWRATHT<T,S> except passes
    the creating object to the ctor:

    
    \code
    class RenderBarDerived_Foo:public PaintedWidgetsOfWRATHPassOwnerT<RenderBar, RenderBarDerived_Foo, RenderBarDerived>
    {
    public:
      RenderBarDerived_Foo(RenderBarDerived *creator)
      {

      {

      //custom members
    };

    //RenderBarDerived publically inherits from RenderBar
    void RenderBarDerived::readyWRATHFoo(PaintInfoOfWRATH &paintInfo, 
                                         PaintedWidgetsOfWRATHHandleT<RenderBar> &handle,
                                         other arguments)
    {
       RenderBarDerived_Foo *d;
       d=RenderBarDerived_Foo::object(this, handle);

       
       ContextOfWRATH::AutoPushNode autoPushRoot(paintInfo.wrath_context, d->m_root_node);

       
    }
    \endcode
   */
  template<typename T, typename S, typename R=T>
  class PaintedWidgetsOfWRATHPassOwnerT:public PaintedWidgetsOfWRATHBase
  {
  public:

    virtual
    void
    visible(bool v)
    {
      if(m_root_node.widget()) {
        m_root_node.widget()->visible(v);
      }
    }

    static
    S*
    object(R *ptr,
           PaintedWidgetsOfWRATHHandleT<T> &handle)
    {
      return objectPassOwnerToCtor(type_tag<S>(), ptr, handle);
    }

    WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_root_node;    
  };
  
  /*
    Provided as a convenaince where the handle template
    arguemnt is RenderObject and the creating type
    is defaulted to RenderObject.
   */
  template<typename S, typename R=RenderObject>
  class PaintedWidgetsOfWRATHPassOwner:public PaintedWidgetsOfWRATHPassOwnerT<RenderObject, S, R>
  {
  };

  /*
    most handles are for RenderObject, so lets make some typedefs
   */
  typedef PaintedWidgetsOfWRATHHandleT<RenderObject> PaintedWidgetsOfWRATHHandle;



  /*
    enumeration values for different passes for drawing transparent stuff:
   */
  const int TextPassEnumerationOfWRATH=INT_MAX; //make sure text is drawn last
  const int TransparentImagePassEnumerationOfWRATH=1;

  
}

#endif 
