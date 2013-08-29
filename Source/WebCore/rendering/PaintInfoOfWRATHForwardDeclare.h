#ifndef _PaintInfoOfWRATHForwardDeclare_
#define _PaintInfoOfWRATHForwardDeclare_


#include "PaintInfo.h"
#include <boost/signals2.hpp>
#include <boost/bind.hpp>


/*
  C++ nested types and forward declare just is plain
  awful!

  One needs to guarantee (by hand) that
  
  ContextOfWRATHCanvas <---> underlying type of ContextOfWRATH::Canvas
 */
class LayerOfWRATH;

#include "WRATHBBoxForwardDeclare.hpp"

namespace WebCore {

  class RenderObject;
  class PaintInfoOfWRATH;
  class BranchVisibilityNodePair;
  class PaintedWidgetsOfWRATHBase;

  template<typename T>
  class PaintedWidgetsOfWRATHHandleT;

  typedef PaintedWidgetsOfWRATHHandleT<RenderObject> PaintedWidgetsOfWRATHHandle;

  class ContextOfWRATH;

  typedef LayerOfWRATH ContextOfWRATHCanvas;
};


#endif
