/*
 * Copyright (C) 2006 Samuel Weinig (sam.weinig@gmail.com)
 * Copyright (C) 2004, 2005, 2006 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "Image.h"

#include "AffineTransform.h"
#include "BitmapImage.h"
#include "GraphicsContext.h"
#include "IntRect.h"
#include "MIMETypeRegistry.h"
#include "SharedBuffer.h"
#include <math.h>
#include <wtf/StdLibExtras.h>

#if USE(CG)
#include <CoreFoundation/CoreFoundation.h>
#endif

#if USE(WRATH)
#include "ImageObserver.h"
#include "WRATHPaintHelpers.h"
#include "NativeImageWRATH.h"

namespace {
    

    class Image_SolidColor: 
      public WebCore::PaintedWidgetsOfWRATHT<WebCore::Image, Image_SolidColor>
    {
    public:
      WebCore::FilledFloatRectOfWRATH m_fill;
    };

    class Image_readyWRATHWidgetTiled : 
      public WebCore::PaintedWidgetsOfWRATHT<WebCore::Image, Image_readyWRATHWidgetTiled>
    {
    public:
        WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::Image> m_solid_color;
        WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::Image> m_widgets;
        WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::Image> m_pattern;
    };

    class Image_readyWRATHWidgetPattern : 
      public WebCore::PaintedWidgetsOfWRATHT<WebCore::Image, Image_readyWRATHWidgetPattern>
    {
    public:
        WebCore::ContextOfWRATH::NodeHandle::AutoDelete m_clip_node;
        WebCore::PaintedWidgetsOfWRATHHandleT<WebCore::Image> m_image_item;
    };
}




#endif

namespace WebCore {

Image::Image(ImageObserver* observer)
    : m_imageObserver(observer)
{
}

Image::~Image()
{
#if USE(WRATH)
    m_dtor_signal();
#endif
}

Image* Image::nullImage()
{
    ASSERT(isMainThread());
    DEFINE_STATIC_LOCAL(RefPtr<Image>, nullImage, (BitmapImage::create()));;
    return nullImage.get();
}

bool Image::supportsType(const String& type)
{
    return MIMETypeRegistry::isSupportedImageResourceMIMEType(type); 
} 

bool Image::setData(PassRefPtr<SharedBuffer> data, bool allDataReceived)
{
    m_data = data;
    if (!m_data.get())
        return true;

    int length = m_data->size();
    if (!length)
        return true;
    
    return dataChanged(allDataReceived);
}

void Image::fillWithSolidColor(GraphicsContext* ctxt, const FloatRect& dstRect, const Color& color, ColorSpace styleColorSpace, CompositeOperator op)
{
    if (color.alpha() <= 0)
        return;
    
    CompositeOperator previousOperator = ctxt->compositeOperation();
    ctxt->setCompositeOperation(!color.hasAlpha() && op == CompositeSourceOver ? CompositeCopy : op);
    ctxt->fillRect(dstRect, color, styleColorSpace);
    ctxt->setCompositeOperation(previousOperator);
}

#if USE(WRATH)
void Image::readyWRATHWidgetSolidColor(PaintedWidgetsOfWRATHHandleT<Image>& handle, ContextOfWRATH *ctx,
                                       Image* image,
                                       const FloatRect& dstRect, const Color& color,
                                       ColorSpace styleColorSpace, CompositeOperator op)
{
    Image_SolidColor *d(Image_SolidColor::object(image, handle));

    ContextOfWRATH::AutoPushNode autoPushRoot(ctx, d->m_root_node);

    if (d->m_fill.widget())
        d->m_fill.widget()->visible(false);

    if (color.alpha() <= 0)
        return;
    
    /*
    CompositeOperator previousOperator = ctxt->compositeOperation();
    ctxt->setCompositeOperation(!color.hasAlpha() && op == CompositeSourceOver ? CompositeCopy : op);
    ctxt->fillRect(dstRect, color, styleColorSpace);
    ctxt->setCompositeOperation(previousOperator);
    */
    if(!color.hasAlpha() && op == CompositeSourceOver)
      {
        op=CompositeCopy;
      }

    d->m_fill.update(ctx, FloatRect(dstRect), color, op);
    d->m_fill.widget()->visible(true);
}
#endif

static inline FloatSize calculatePatternScale(const FloatRect& dstRect, const FloatRect& srcRect, Image::TileRule hRule, Image::TileRule vRule)
{
    float scaleX = 1.0f, scaleY = 1.0f;
    
    if (hRule == Image::StretchTile)
        scaleX = dstRect.width() / srcRect.width();
    if (vRule == Image::StretchTile)
        scaleY = dstRect.height() / srcRect.height();
    
    if (hRule == Image::RepeatTile)
        scaleX = scaleY;
    if (vRule == Image::RepeatTile)
        scaleY = scaleX;
    
    return FloatSize(scaleX, scaleY);
}

void Image::drawTiled(GraphicsContext* ctxt, const FloatRect& destRect, const FloatPoint& srcPoint, const FloatSize& scaledTileSize, ColorSpace styleColorSpace, CompositeOperator op)
{    
    if (mayFillWithSolidColor()) {
        fillWithSolidColor(ctxt, destRect, solidColor(), styleColorSpace, op);
        return;
    }

    // See <https://webkit.org/b/59043>.
#if !PLATFORM(WX)
    ASSERT(!isBitmapImage() || static_cast<BitmapImage*>(this)->notSolidColor());
#endif

    FloatSize intrinsicTileSize = size();
    if (hasRelativeWidth())
        intrinsicTileSize.setWidth(scaledTileSize.width());
    if (hasRelativeHeight())
        intrinsicTileSize.setHeight(scaledTileSize.height());

    FloatSize scale(scaledTileSize.width() / intrinsicTileSize.width(),
                    scaledTileSize.height() / intrinsicTileSize.height());

    FloatRect oneTileRect;
    oneTileRect.setX(destRect.x() + fmodf(fmodf(-srcPoint.x(), scaledTileSize.width()) - scaledTileSize.width(), scaledTileSize.width()));
    oneTileRect.setY(destRect.y() + fmodf(fmodf(-srcPoint.y(), scaledTileSize.height()) - scaledTileSize.height(), scaledTileSize.height()));
    oneTileRect.setSize(scaledTileSize);
    
    // Check and see if a single draw of the image can cover the entire area we are supposed to tile.    
    if (oneTileRect.contains(destRect)) {
        FloatRect visibleSrcRect;
        visibleSrcRect.setX((destRect.x() - oneTileRect.x()) / scale.width());
        visibleSrcRect.setY((destRect.y() - oneTileRect.y()) / scale.height());
        visibleSrcRect.setWidth(destRect.width() / scale.width());
        visibleSrcRect.setHeight(destRect.height() / scale.height());
        draw(ctxt, destRect, visibleSrcRect, styleColorSpace, op);
        return;
    }

    AffineTransform patternTransform = AffineTransform().scaleNonUniform(scale.width(), scale.height());
    FloatRect tileRect(FloatPoint(), intrinsicTileSize);    
    drawPattern(ctxt, tileRect, patternTransform, oneTileRect.location(), styleColorSpace, op, destRect);
    
    startAnimation();
}

// FIXME: Merge with the other drawTiled eventually, since we need a combination of both for some things.
void Image::drawTiled(GraphicsContext* ctxt, const FloatRect& dstRect, const FloatRect& srcRect, TileRule hRule, TileRule vRule, ColorSpace styleColorSpace, CompositeOperator op)
{    
    if (mayFillWithSolidColor()) {
        fillWithSolidColor(ctxt, dstRect, solidColor(), styleColorSpace, op);
        return;
    }
    
    // FIXME: We do not support 'round' yet.  For now just map it to 'repeat'.
    if (hRule == RoundTile)
        hRule = RepeatTile;
    if (vRule == RoundTile)
        vRule = RepeatTile;

    FloatSize scale = calculatePatternScale(dstRect, srcRect, hRule, vRule);
    AffineTransform patternTransform = AffineTransform().scaleNonUniform(scale.width(), scale.height());

    // We want to construct the phase such that the pattern is centered (when stretch is not
    // set for a particular rule).
    float hPhase = scale.width() * srcRect.x();
    float vPhase = scale.height() * srcRect.y();
    if (hRule == Image::RepeatTile)
        hPhase -= fmodf(dstRect.width(), scale.width() * srcRect.width()) / 2.0f;
    if (vRule == Image::RepeatTile)
        vPhase -= fmodf(dstRect.height(), scale.height() * srcRect.height()) / 2.0f;
    FloatPoint patternPhase(dstRect.x() - hPhase, dstRect.y() - vPhase);
    
    drawPattern(ctxt, srcRect, patternTransform, patternPhase, styleColorSpace, op, dstRect);

    startAnimation();
}

#if USE(WRATH)
void Image::readyWRATHWidgetTiled(PaintedWidgetsOfWRATHHandleT<Image>& handle, ContextOfWRATH *ctx,
                                  const FloatRect& destRect, const FloatPoint& srcPoint,
                                  const FloatSize& scaledTileSize, ColorSpace styleColorSpace, CompositeOperator op)
{
    Image_readyWRATHWidgetTiled *d(Image_readyWRATHWidgetTiled::object(this, handle));

    ContextOfWRATH::AutoPushNode autoPushRoot(ctx, d->m_root_node);

    d->m_solid_color.visible(false);
    d->m_widgets.visible(false);
    d->m_pattern.visible(false);

    if (mayFillWithSolidColor()) {
        d->m_solid_color.visible(true);
        readyWRATHWidgetSolidColor(d->m_solid_color, ctx, this, destRect, solidColor(), styleColorSpace, op);
        return;
    }

    // See <https://webkit.org/b/59043>.
#if !PLATFORM(WX)
    ASSERT(!isBitmapImage() || static_cast<BitmapImage*>(this)->notSolidColor());
#endif

    FloatSize intrinsicTileSize = size();
    if (hasRelativeWidth())
        intrinsicTileSize.setWidth(scaledTileSize.width());
    if (hasRelativeHeight())
        intrinsicTileSize.setHeight(scaledTileSize.height());

    FloatSize scale(scaledTileSize.width() / intrinsicTileSize.width(),
                    scaledTileSize.height() / intrinsicTileSize.height());

    FloatRect oneTileRect;
    oneTileRect.setX(destRect.x() + fmodf(fmodf(-srcPoint.x(), scaledTileSize.width()) - scaledTileSize.width(), scaledTileSize.width()));
    oneTileRect.setY(destRect.y() + fmodf(fmodf(-srcPoint.y(), scaledTileSize.height()) - scaledTileSize.height(), scaledTileSize.height()));
    oneTileRect.setSize(scaledTileSize);
    
    // Check and see if a single draw of the image can cover the entire area we are supposed to tile.    
    if (oneTileRect.contains(destRect)) {
        FloatRect visibleSrcRect;
        visibleSrcRect.setX((destRect.x() - oneTileRect.x()) / scale.width());
        visibleSrcRect.setY((destRect.y() - oneTileRect.y()) / scale.height());
        visibleSrcRect.setWidth(destRect.width() / scale.width());
        visibleSrcRect.setHeight(destRect.height() / scale.height());
        d->m_widgets.visible(true);
        readyWRATHWidgets(d->m_widgets, ctx, destRect, visibleSrcRect, styleColorSpace, op);
        return;
    }

    AffineTransform patternTransform = AffineTransform().scaleNonUniform(scale.width(), scale.height());
    FloatRect tileRect(FloatPoint(), intrinsicTileSize);
    d->m_pattern.visible(true);
    readyWRATHWidgetPattern(d->m_pattern, ctx, tileRect, patternTransform, oneTileRect.location(), styleColorSpace, op, destRect);
    
    startAnimation();
}

// FIXME: Merge with the other drawTiled eventually, since we need a combination of both for some things.
void Image::readyWRATHWidgetTiled(PaintedWidgetsOfWRATHHandleT<Image>& handle, ContextOfWRATH *ctx,
                                  const FloatRect& dstRect, const FloatRect& srcRect, TileRule hRule, TileRule vRule,
                                  ColorSpace styleColorSpace, CompositeOperator op)
{    
    // Note: This is the same class as in the above function.
    // The only difference is that m_widgets is not used at all in this overload.
    Image_readyWRATHWidgetTiled *d(Image_readyWRATHWidgetTiled::object(this, handle));

    ContextOfWRATH::AutoPushNode autoPushRoot(ctx, d->m_root_node);

    d->m_solid_color.visible(false);
    d->m_widgets.visible(false);
    d->m_pattern.visible(false);

    if (mayFillWithSolidColor()) {
        d->m_solid_color.visible(true);
        readyWRATHWidgetSolidColor(d->m_solid_color, ctx, this, dstRect, solidColor(), styleColorSpace, op);
        return;
    }
    
    // FIXME: We do not support 'round' yet.  For now just map it to 'repeat'.
    if (hRule == RoundTile)
        hRule = RepeatTile;
    if (vRule == RoundTile)
        vRule = RepeatTile;

    FloatSize scale = calculatePatternScale(dstRect, srcRect, hRule, vRule);
    AffineTransform patternTransform = AffineTransform().scaleNonUniform(scale.width(), scale.height());

    // We want to construct the phase such that the pattern is centered (when stretch is not
    // set for a particular rule).
    float hPhase = scale.width() * srcRect.x();
    float vPhase = scale.height() * srcRect.y();
    if (hRule == Image::RepeatTile)
        hPhase -= fmodf(dstRect.width(), scale.width() * srcRect.width()) / 2.0f;
    if (vRule == Image::RepeatTile)
        vPhase -= fmodf(dstRect.height(), scale.height() * srcRect.height()) / 2.0f;
    FloatPoint patternPhase(dstRect.x() - hPhase, dstRect.y() - vPhase);
    
    d->m_pattern.visible(true);
    readyWRATHWidgetPattern(d->m_pattern, ctx, srcRect, patternTransform, patternPhase, styleColorSpace, op, dstRect);

    startAnimation();
}
#endif

#if USE(WRATH)
NativeImagePtr Image::nativeImageForWRATH()
{
    return nativeImageForCurrentFrame();
}


void Image::readyWRATHWidgetPattern(PaintedWidgetsOfWRATHHandleT<Image>& handle, ContextOfWRATH *ctx,
                                    const FloatRect& tileRect, const AffineTransform& patternTransform,
                                    const FloatPoint& phase, ColorSpace, CompositeOperator op, const FloatRect& destRect)
{
    Image_readyWRATHWidgetPattern *d(Image_readyWRATHWidgetPattern::object(this, handle));

    ContextOfWRATH::AutoPushNode autoPushRoot(ctx, d->m_root_node);
    ContextOfWRATH::AutoPushNode autoPushClip(ctx, d->m_clip_node);

    d->m_clip_node.widget()->visible(false);

    NativeImagePtr nativeImage = nativeImageForCurrentFrame();
    if(!nativeImage)
        return;

    //if the source rect or destination rect is empty, short circuit to not draw.
    FloatRect dr = destRect.normalized();
    IntRect tr = IntRect(tileRect.normalized());
    if (!dr.width() || !dr.height() || !tr.width() || !tr.height())
        return;
    
    /*
    CompositeOperator previousOperator = ctxt->compositeOperation();
    ctxt->setCompositeOperation(!pixmap.hasAlpha() && op == CompositeSourceOver ? CompositeCopy : op);
    */
    if(!nativeImage->hasAlpha() && op == CompositeSourceOver)
      {
        op=CompositeCopy;
      }

    /*
      What the arguments mean, since WebKit developers do not seem to
      enjoy commenting and explaining the arguments.

      A rectangle will be draw, that rectangle is exactly at dr.
      The image data from this WebCore::Image comes exactly from tr.

      The icky part is how that tr rectangle is mapped to dr and
      if it tiles or stretches (in each dimension seperately).

      Let SubSrc = image resticted to tr. Denote:

        SubSrc(i,j) = texel at (i,j) = image(i+tr.x(), j+tr.y()) for 0<=i<tr.width(), 0<=j<tr.height()
        
      Define

        PhasedSubSrc(i,j) = texel at ( (i+phase.x())%tr.width(), (j+phase.y())%tr.height())

      extend PhasedSubSrc(i,j) as follows: 

         PhasedSubSrc(i + n*tr.width(), j + m*tr.height()) = PhasedSubSrc(i,j)

      i.e. repeat in each dimension. Note that extenting SubSrc similarly yeilds that

         PhasedSubSrc(i,j) == SubSrc(i+phase.x(), j+phase.y())

      PatternTransform is _always_ an affine mapping whose matrix is diagnol,
      i.e. it is just a scale in x and a scale in y and a translation

      Let TrSrc be PhasedSubSrc mapped by patternTransform (viewing
      PhasedSubSrc as a repeated image on the entire plane).

      We are to draw the rectangle dr filled exactly by TrSrc.

     */
    
    FloatRect tileRectInTargetCoords;
    tileRectInTargetCoords = patternTransform.mapRect(FloatRect(tr));

    /*
      we need to "texture coordinates" of what the corners
      of the destination rectangle would be. We compute
      by seeing how many "times" (fractional ok) that
      we repeat the rectangle tileRectInTargetCoords in
      the plane and then repeat the original texture
      coordinates (given by tr).
     */
    vec2 oneRectTexelCoordsLocation(tileRectInTargetCoords.x(), tileRectInTargetCoords.y());
    vec2 oneRectTexelCoordsSize(tileRectInTargetCoords.width(), tileRectInTargetCoords.height());
    vec2 srcRectLocation(tr.x(), tr.y()), srcRectSize(tr.width(), tr.height());
    vec2 min_p, max_p, delta_p, count_before, count_after;

    count_before= ( vec2(dr.x(), dr.y()) - oneRectTexelCoordsLocation )/oneRectTexelCoordsSize;
    count_after=  ( vec2(dr.maxX(), dr.maxY()) - oneRectTexelCoordsLocation)/oneRectTexelCoordsSize;
    
    min_p= srcRectLocation + count_before*srcRectSize;
    max_p= srcRectLocation + count_after*srcRectSize;
    delta_p=max_p - min_p;

    FloatRect texelCoords(FloatPoint(min_p.x(), min_p.y()),
                          FloatSize(delta_p.x(), delta_p.y()) );

    /*
      slide texelCoords by phase now, phase is in coordinates
      after patternTransformation is applied, so we
      need to undo the scaling it did:
     */
    texelCoords.move(-(phase.x())/static_cast<float>(patternTransform.a()), 
                     -(phase.y())/static_cast<float>(patternTransform.d()));


    ContextOfWRATH::set_clipping(d->m_clip_node, IntRect(dr));
    d->m_clip_node.widget()->visible(true);


      
    d->m_image_item.visible(true);
    ImageRectOfWRATH::update_tiled_through_handle(ctx, d->m_image_item, this, nativeImage,
                                                  dr,
                                                  tr, 
                                                  op, 
                                                  texelCoords);
    


    if (imageObserver())
        imageObserver()->didDraw(this);
}
#endif


}
