#ifndef Generator_readyWRATHWidgetArguments_h
#define Generator_readyWRATHWidgetArguments_h

#include "config.h"
#include "PaintInfoOfWRATH.h"

namespace WebCore {

  class Generator_readyWRATHWidgetsArguments
  {
  public:
    Generator_readyWRATHWidgetsArguments(PaintedWidgetsOfWRATHHandleT<Generator> &ph,
                                         ContextOfWRATH *ptx,
                                         const FloatRect &pdstRect,
                                         const FloatRect &psrcRect,
                                         ColorSpace pstyleColorSpace,
                                         CompositeOperator pcompositeOp):
      handle(ph),
      ctx(ptx),
      dstRect(pdstRect),
      srcRect(psrcRect),
      styleColorSpace(pstyleColorSpace),
      compositeOp(pcompositeOp)
    {}

    PaintedWidgetsOfWRATHHandleT<Generator> &handle;
    ContextOfWRATH *ctx;
    const FloatRect &dstRect;
    const FloatRect &srcRect;
    ColorSpace styleColorSpace;
    CompositeOperator compositeOp;
  };

  class Generator_readyWRATHWidgetPatternArguments
  {
  public:
    Generator_readyWRATHWidgetPatternArguments(PaintedWidgetsOfWRATHHandleT<Generator>& phandle, 
                                               ContextOfWRATH *pctx,
                                               const FloatRect& psrcRect, 
                                               const AffineTransform& ppatternTransform,
                                               const FloatPoint& pphase, ColorSpace pstyleColorSpace,
                                               CompositeOperator pop, 
                                               const FloatRect &pdstRect):
      handle(phandle),
      ctx(pctx),
      srcRect(psrcRect),
      dstRect(pdstRect),
      patternTransform(ppatternTransform),
      phase(pphase),
      styleColorSpace(pstyleColorSpace),
      op(pop)
    {}

    PaintedWidgetsOfWRATHHandleT<Generator>& handle;
    ContextOfWRATH *ctx;
    const FloatRect& srcRect;
    const FloatRect& dstRect;
    const AffineTransform& patternTransform;
    const FloatPoint& phase;
    ColorSpace styleColorSpace;
    CompositeOperator op;      
  };
}

#endif
