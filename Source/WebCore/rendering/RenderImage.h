/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2006 Allan Sandfeld Jensen (kde@carewolf.com) 
 *           (C) 2006 Samuel Weinig (sam.weinig@gmail.com)
 * Copyright (C) 2004, 2005, 2006, 2007, 2009, 2010, 2011 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef RenderImage_h
#define RenderImage_h

#include "RenderImageResource.h"
#include "RenderReplaced.h"

#if USE(WRATH)
#include "PaintInfoOfWRATHForwardDeclare.h"
#endif

namespace WebCore {

class HTMLAreaElement;
class HTMLMapElement;

class RenderImage : public RenderReplaced {
public:
#if USE(WRATH)
    enum MemberChange
    {
        E_altText,
        E_imageResource,
        E_needsToSetSizeForAltText,
        E_numberOfChanges
    };

    typedef Signal<RenderImage> SignalType;
    SignalType::connection connect(const SignalType::slot_type & slot) { return m_signal.connect(slot); }

    typedef boost::signals2::signal<void () > focus_change_type;
    boost::signals2::connection connect_focus_changed(const focus_change_type::slot_type & slot) { return m_focus_changed.connect(slot); }
#endif

    RenderImage(Node*);
    virtual ~RenderImage();

    void setImageResource(PassOwnPtr<RenderImageResource>);

    RenderImageResource* imageResource() { return m_imageResource.get(); }
    const RenderImageResource* imageResource() const { return m_imageResource.get(); }
    CachedImage* cachedImage() const { return m_imageResource ? m_imageResource->cachedImage() : 0; }

    bool setImageSizeForAltText(CachedImage* newImage = 0);

    void updateAltText();

    HTMLMapElement* imageMap() const;
    void areaElementFocusChanged(HTMLAreaElement*);

    void highQualityRepaintTimerFired(Timer<RenderImage>*);

protected:
    virtual void styleDidChange(StyleDifference, const RenderStyle*);

    virtual void imageChanged(WrappedImagePtr, const IntRect* = 0);

    virtual void paintIntoRect(GraphicsContext*, const IntRect&);
    virtual void paint(PaintInfo&, int tx, int ty);

#if USE(WRATH)
    virtual void readyWRATHWidgetIntoRect(PaintedWidgetsOfWRATHHandle&,
					  ContextOfWRATH*, const IntRect&);

    virtual void readyWRATHWidgets(PaintedWidgetsOfWRATHHandle&,
                                   PaintInfoOfWRATH &paintInfo, int tx, int ty);
#endif

    bool isLogicalWidthSpecified() const;
    bool isLogicalHeightSpecified() const;

    virtual void intrinsicSizeChanged()
    {
        if (m_imageResource)
            imageChanged(m_imageResource->imagePtr());
    }

private:
    virtual const char* renderName() const { return "RenderImage"; }

    virtual bool isImage() const { return true; }
    virtual bool isRenderImage() const { return true; }

    virtual void paintReplaced(PaintInfo&, int tx, int ty);

#if USE(WRATH)
    virtual void readyWRATHWidgetReplaced(PaintedWidgetsOfWRATHHandle&,
					  PaintInfoOfWRATH&, int, int);
#endif

    virtual int minimumReplacedHeight() const;

    virtual void notifyFinished(CachedResource*);
    virtual bool nodeAtPoint(const HitTestRequest&, HitTestResult&, int x, int y, int tx, int ty, HitTestAction);

    virtual int computeReplacedLogicalWidth(bool includeMaxWidth = true) const;
    virtual int computeReplacedLogicalHeight() const;

    IntSize imageSizeForError(CachedImage*) const;
    void imageDimensionsChanged(bool imageSizeChanged, const IntRect* = 0);

    int calcAspectRatioLogicalWidth() const;
    int calcAspectRatioLogicalHeight() const;

    void paintAreaElementFocusRing(PaintInfo&);

#if USE(WRATH)
    void readyWRATHWidgetAreaElementFocusRing(PaintedWidgetsOfWRATHHandle&, PaintInfoOfWRATH&);
#endif

    void setNeedsToSetSizeForAltText(bool t=true)
    {
        SET_VALUE_AND_EMIT(m_needsToSetSizeForAltText, t, m_signal, E_needsToSetSizeForAltText);
    }

    // Text to display as long as the image isn't available.
    String m_altText;
    OwnPtr<RenderImageResource> m_imageResource;
    bool m_needsToSetSizeForAltText;

    friend class RenderImageScaleObserver;

#if USE(WRATH)
private:
    SignalType m_signal;
    focus_change_type m_focus_changed;
#endif
};

inline RenderImage* toRenderImage(RenderObject* object)
{
    ASSERT(!object || object->isRenderImage());
    return static_cast<RenderImage*>(object);
}

inline const RenderImage* toRenderImage(const RenderObject* object)
{
    ASSERT(!object || object->isRenderImage());
    return static_cast<const RenderImage*>(object);
}

// This will catch anyone doing an unnecessary cast.
void toRenderImage(const RenderImage*);

} // namespace WebCore

#endif // RenderImage_h
