/*
 * Copyright (C) 2008 Holger Hans Peter Freyther
 *
 * All rights reserved.
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
#include "StillImageQt.h"

#include "ContextShadow.h"
#include "GraphicsContext.h"
#include "IntSize.h"

#include <QPainter>

#if USE(WRATH)
#include "NativeImageWRATH.h"
#include "WRATHPaintHelpers.h"

namespace {
    class StillImage_readyWRATHWidgets : 
      public WebCore::PaintedWidgetsOfWRATHT<WebCore::Image, StillImage_readyWRATHWidgets>
    {
    public:
      WebCore::ImageRectOfWRATH m_image_rect_item;
    };
}
#endif

namespace WebCore {

#if USE(WRATH)
StillImage::StillImage(const NativeImageWRATH& image)
    : m_image(new NativeImageWRATH(image))
    , m_ownsPixmap(true)
{}

StillImage::StillImage(NativeImageWRATH* image)
    : m_image(image)
    , m_ownsPixmap(false)
{}
#else

StillImage::StillImage(const QPixmap& pixmap)
    : m_pixmap(new QPixmap(pixmap))
    , m_ownsPixmap(true)
{}

StillImage::StillImage(const QPixmap* pixmap)
    : m_pixmap(pixmap)
    , m_ownsPixmap(false)
{}
#endif

StillImage::~StillImage()
{
    if (m_ownsPixmap)
#if USE(WRATH)
        delete m_image;
#else
        delete m_pixmap;
#endif
}

IntSize StillImage::size() const
{
#if USE(WRATH)
    QPixmap* m_pixmap = m_image->getPixmap();
#endif
    return IntSize(m_pixmap->width(), m_pixmap->height());
}

NativeImagePtr StillImage::nativeImageForCurrentFrame()
{
#if USE(WRATH)
    return const_cast<NativeImagePtr>(m_image);
#else
    return const_cast<NativeImagePtr>(m_pixmap);
#endif
}

void StillImage::draw(GraphicsContext* ctxt, const FloatRect& dst,
                      const FloatRect& src, ColorSpace, CompositeOperator op)
{
#if USE(WRATH)
    QPixmap* m_pixmap = m_image->getPixmap();
#endif 
    if (m_pixmap->isNull())
        return;

    FloatRect normalizedSrc = src.normalized();
    FloatRect normalizedDst = dst.normalized();

    CompositeOperator previousOperator = ctxt->compositeOperation();
    ctxt->setCompositeOperation(op);

    ContextShadow* shadow = ctxt->contextShadow();
    if (shadow->m_type != ContextShadow::NoShadow) {
        QPainter* shadowPainter = shadow->beginShadowLayer(ctxt, normalizedDst);
        if (shadowPainter) {
            shadowPainter->setOpacity(static_cast<qreal>(shadow->m_color.alpha()) / 255);
            shadowPainter->drawPixmap(normalizedDst, *m_pixmap, normalizedSrc);
            shadow->endShadowLayer(ctxt);
        }
    }

    ctxt->platformContext()->drawPixmap(normalizedDst, *m_pixmap, normalizedSrc);
    ctxt->setCompositeOperation(previousOperator);
}

#if USE(WRATH)
void StillImage::readyWRATHWidgets(PaintedWidgetsOfWRATHHandleT<Image>& handle, ContextOfWRATH *ctx,
                                   const FloatRect& dst, const FloatRect& src,
                                   ColorSpace, CompositeOperator op)
{
    StillImage_readyWRATHWidgets *d(StillImage_readyWRATHWidgets::object(this, handle));

    ContextOfWRATH::AutoPushNode autoPushRoot(ctx, d->m_root_node);

    if (d->m_image_rect_item.widget())
        d->m_image_rect_item.widget()->visible(false);


    /*
    QPixmap* m_pixmap = m_image->getPixmap();
    if (m_pixmap->isNull())
        return;
    */

    FloatRect normalizedSrc = src.normalized();
    FloatRect normalizedDst = dst.normalized();


    /*
    CompositeOperator previousOperator = ctxt->compositeOperation();
    ctxt->setCompositeOperation(op);
    */

    /*
      [WRATH-DANGER]: Shadows not implemented

    ContextShadow* shadow = ctxt->contextShadow();
    if (shadow->m_type != ContextShadow::NoShadow) {
        QPainter* shadowPainter = shadow->beginShadowLayer(ctxt, normalizedDst);
        if (shadowPainter) {
            shadowPainter->setOpacity(static_cast<qreal>(shadow->m_color.alpha()) / 255);
            shadowPainter->drawPixmap(normalizedDst, *m_pixmap, normalizedSrc);
            shadow->endShadowLayer(ctxt);
        }
    }
    */

    /* [WRATH-DANGER]: Possible rounding errors from converting to IntRect */
    d->m_image_rect_item.update(ctx, m_image,
                                normalizedDst, IntRect(normalizedSrc),
                                op);
    d->m_image_rect_item.widget()->visible(true);

    /*
    ctxt->platformContext()->drawPixmap(normalizedDst, *m_pixmap, normalizedSrc);
    ctxt->setCompositeOperation(previousOperator);
    */
}
#endif

}
