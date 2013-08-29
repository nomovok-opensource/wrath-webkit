#ifndef _WRATHPaintHelpers_
#define _WRATHPaintHelpers_

#include "config.h"

#include "Color.h"
#include "IntRect.h"
#include "PaintInfoOfWRATH.h"
#include "NativeImageWRATH.h"

namespace WebCore
{
#if USE(WRATH)
    /*
      =====================
      Widget adding helpers 
      =====================
    */
  class FilledIntRectOfWRATH:boost::noncopyable
  {
  public:
    typedef WebCore::ContextOfWRATH::ColorFamily::DrawnRect RectType;

    FilledIntRectOfWRATH(void);

    /*
      Set as color with named composite operator
     */
    void
    update(ContextOfWRATH *ctx,
           const IntRect& newrect,
           const Color& newcolor,
           CompositeOperator op);
    
    /*
      set as clear rect
     */
    void
    update(WebCore::ContextOfWRATH *ctx,
           const WebCore::IntRect& newrect);
    
    RectType::Widget*
    widget(void) 
    {
      return m_item.widget();
    }

    /*
      Adds a rectangle to the given shape's current outline, making
      the rectangle's position at the origin if the first bool is true.
     */
    static
    void 
    AddToShape_TranslateToOrigin(bool translate_to_origin,
                                 WRATHShapeF & shape,
                                 const WebCore::IntRect & rect);

    /*
      Adds a rectangle to the given shape's current outline
     */
    static
    inline
    void 
    AddToShape(WRATHShapeF & shape,
               const WebCore::IntRect & rect)
    {
      AddToShape_TranslateToOrigin(false, shape, rect);
    }
    
  private:
    IntSize m_rect_size;
    CompositeOperator m_op;
    ContextOfWRATH::CanvasNodeForTransparentSingleton m_node;
    RectType::AutoDelete m_item;
  };

  
  class FilledFloatRectOfWRATH:boost::noncopyable
  {
  public:
    typedef WebCore::ContextOfWRATH::ColorFamily::DrawnRect RectType;

    FilledFloatRectOfWRATH(void);

    /*
      Set as color with named composite operator
     */
    void
    update(WebCore::ContextOfWRATH *ctx,
           const WebCore::FloatRect& newrect,
           const WebCore::Color& newcolor,
           CompositeOperator op);
    
    /*
      set as clear rect
    */
    void
    update(WebCore::ContextOfWRATH *ctx,
           const WebCore::FloatRect& newrect);
    
    RectType::Widget*
    widget(void) 
    {
      return m_item.widget();
    }

    /*
      Adds a rectangle to the given shape's current outline, making
      the rectangle's position at the origin if the first bool is true.
     */
    static
    void 
    AddToShape_TranslateToOrigin(bool translate_to_origin,
                                 WRATHShapeF & shape,
                                 const WebCore::FloatRect & rect);

    /*
      Adds a rectangle to the given shape's current outline
     */
    static
    inline
    void 
    AddToShape(WRATHShapeF & shape,
               const WebCore::FloatRect & rect)
    {
      AddToShape_TranslateToOrigin(false, shape, rect);
    }

  private:
    FloatSize m_rect_size;
    CompositeOperator m_op;
    ContextOfWRATH::CanvasNodeForTransparentSingleton m_node;
    RectType::AutoDelete m_item;
    
  };

  /*
    TODO:
     we should make seperate classes to implement
     update_tiled() and update(), since the latter
     can save some complexity in the fragment
     shader by being SimpleXSimplyImageFamily
   */
  class ImageRectOfWRATH:boost::noncopyable
  {
  public:
    typedef WebCore::ContextOfWRATH::RepeatXRepeatYImageFamily::DrawnRect RectType;

    ImageRectOfWRATH(void);
    ~ImageRectOfWRATH();

    /*
      makes the image stretched/compressed to fit in dest
     */
    void
    update(ContextOfWRATH *ctx,
           NativeImageWRATH *image,
           FloatRect dest, 
           IntRect src,
           CompositeOperator);

    /*
      draws number_tiles of the image in dest.

      dest --> location to draw at
      src  --> location within image from which to take texel data
      TexelCoords --> Let subImage=sub-image specified at src, then tile 
                      that subimage on the plane, these TexelCoords indicate
                      the texel values to use at the two corners of the dest                     
     */
    void
    update_tiled(ContextOfWRATH *ctx,
                 NativeImageWRATH *image,
                 FloatRect dest, 
                 IntRect src, 
                 CompositeOperator,
                 FloatRect relativeTexelCoords); 

    /*
      the pimple behind handle is essentially just a 
      ImageRectOfWRATH, but by going through the handle,
      the item gets deleted if the WebCore::Image gets
      deleted.
     */
    static
    void
    update_through_handle(ContextOfWRATH *ctx,
                          PaintedWidgetsOfWRATHHandleT<Image>& handle,
                          Image* image,
                          NativeImageWRATH* newImage,
                          const FloatRect &dest, const IntRect &src,
                          CompositeOperator op);

    static
    void
    update_tiled_through_handle(ContextOfWRATH *ctx,
                                PaintedWidgetsOfWRATHHandleT<Image>& handle,
                                Image* image,
                                NativeImageWRATH* newImage,
                                const FloatRect &dest, 
                                const IntRect &src,
                                CompositeOperator op,
                                const FloatRect &relativeTexelCoords);

    RectType::Widget*
    widget(void) 
    {
      return m_item.widget();
    }

    void 
    clear(void);
    
  private:
    void
    update_common(ContextOfWRATH *ctx,
                  NativeImageWRATH *image,
                  FloatRect dest, 
                  IntRect src,
                  FloatRect corners,
                  CompositeOperator);

    WRATHImage *m_image;
    FloatSize m_dest_size;
    CompositeOperator m_op;
    bool m_hasInterestingAlpha;
    FloatRect m_corners;
    boost::signals2::connection m_image_ctor_connection;
    NativeImageWRATH::NonTrivialAlphaQuery m_query;
    ContextOfWRATH::CanvasNodeForTransparentSingleton m_node;

    RectType::AutoDelete m_item;
  };

  class RoundedFilledRectOfWRATH:boost::noncopyable
  {
  public:
    typedef WebCore::ContextOfWRATH::CColorFamily::DrawnShape RectType;

    RoundedFilledRectOfWRATH(void);

    void
    update(WebCore::ContextOfWRATH *ctx,
           const WebCore::RoundedIntRect &rect,
           const WebCore::Color &color,
           CompositeOperator op=CompositeCopy);

    RectType::Widget*
    widget(void) 
    {
      return m_item.widget();
    }

    /*
      some methods to add RoundedRect to WRATHShape:
     */

    /*
      Adds a rounded rectangle to the given shape's current outline, making
      the rectangle's position at the origin if the first bool is true.
    */
    static
    void 
    AddToShape_TranslateToOrigin(bool translate_to_origin,
                                 WRATHShapeF & shape,
                                 const WebCore::RoundedIntRect & rect);

    static
    void 
    AddToShape_TranslateToOrigin(bool translate_to_origin,
                                 WRATHShapeF & shape,
                                 const WebCore::FloatRect &frect,
                                 const float radius);
    
    
  
    /*
      Adds a rounded rectangle to the given shape's current outline
    */
    static
    inline
    void AddToShape(WRATHShapeF & shape,
                    const WebCore::RoundedIntRect & rect)
    {
      AddToShape_TranslateToOrigin(false, shape, rect);
    }

  private:
    WebCore::RoundedIntRect m_rect;
    CompositeOperator m_op;
    ContextOfWRATH::CanvasNodeForTransparentSingleton m_node;
    RectType::AutoDelete m_item;
  };
  
    /*
      WRATH versions of GraphicsContext::drawImage
    */

  void WRATH_drawImage(PaintedWidgetsOfWRATHHandleT<Image>& handle, ContextOfWRATH *ctx,
                       Image* image, ColorSpace styleColorSpace, const IntPoint&, CompositeOperator = CompositeSourceOver);
  void WRATH_drawImage(PaintedWidgetsOfWRATHHandleT<Image>& handle, ContextOfWRATH *ctx,
                       Image* image, ColorSpace styleColorSpace, const IntRect&, CompositeOperator = CompositeSourceOver, 
                       bool useLowQualityScale = false);
  void WRATH_drawImage(PaintedWidgetsOfWRATHHandleT<Image>& handle, ContextOfWRATH *ctx,
                       Image* image, ColorSpace styleColorSpace, const IntPoint& destPoint, 
                       const IntRect& srcRect, CompositeOperator = CompositeSourceOver);
  void WRATH_drawImage(PaintedWidgetsOfWRATHHandleT<Image>& handle, ContextOfWRATH *ctx,
                       Image* image, ColorSpace styleColorSpace, const IntRect& destRect, 
                       const IntRect& srcRect, CompositeOperator = CompositeSourceOver, bool useLowQualityScale = false);
  void WRATH_drawImage(PaintedWidgetsOfWRATHHandleT<Image>& handle, ContextOfWRATH *ctx,
                       Image* image, ColorSpace styleColorSpace, const FloatRect& dest, 
                       const FloatRect& src = FloatRect(0, 0, -1, -1),
                       CompositeOperator op = CompositeSourceOver, bool useLowQualityScale = false);
  
    /*
      WRATH versions of GraphicsContext::drawTiledImage
    */

  void WRATH_drawTiledImage(PaintedWidgetsOfWRATHHandleT<Image>& handle, ContextOfWRATH *ctx,
                            Image* image, ColorSpace styleColorSpace, 
                            const IntRect& destRect, const IntPoint& srcPoint, const IntSize& tileSize,
                            CompositeOperator = CompositeSourceOver, bool useLowQualityScale = false);
  void WRATH_drawTiledImage(PaintedWidgetsOfWRATHHandleT<Image>& handle, ContextOfWRATH *ctx,
                            Image*, ColorSpace styleColorSpace, 
                            const IntRect& destRect, const IntRect& srcRect,
                            Image::TileRule hRule = Image::StretchTile, Image::TileRule vRule = Image::StretchTile,
                            CompositeOperator = CompositeSourceOver, bool useLowQualityScale = false);
  

#endif
}

#endif
