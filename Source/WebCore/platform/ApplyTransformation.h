#ifndef ApplyTransformation_h
#define ApplyTransformation_h

#include "IntPoint.h"
#include "FloatPoint.h"

namespace WebCore
{
  class ApplyTransformationToPoint
  {
  public:

    virtual
    ~ApplyTransformationToPoint(void)
    {}

    virtual
    void
    operator()(IntPoint &in_out) const=0; 
  };
} 


#endif
