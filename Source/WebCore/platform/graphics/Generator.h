/*
 * Copyright (C) 2008 Apple Computer, Inc.  All rights reserved.
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

#ifndef Generator_h
#define Generator_h

#include <wtf/RefCounted.h>

#if defined(WTF_USE_WRATH) && WTF_USE_WRATH
#include <boost/signals2.hpp>
#include <boost/bind.hpp>

namespace WebCore {
  template<typename T>
  class PaintedWidgetsOfWRATHHandleT;
  class Generator_readyWRATHWidgetsArguments;
  class Generator_readyWRATHWidgetPatternArguments;
}
#endif

namespace WebCore {

class FloatRect;
class GraphicsContext;

class Generator : public RefCounted<Generator> {
public:
    virtual ~Generator() 
    {
#if defined(WTF_USE_WRATH) && WTF_USE_WRATH
      m_dtor_signal();
#endif
    };
    
    virtual void fill(GraphicsContext*, const FloatRect&) = 0;
    virtual void adjustParametersForTiledDrawing(IntSize& /* size */, FloatRect& /* srcRect */) { }

#if defined(WTF_USE_WRATH) && WTF_USE_WRATH
  virtual void readyWRATHWidgets(const Generator_readyWRATHWidgetsArguments&)=0;
  virtual void readyWRATHWidgetPattern(const Generator_readyWRATHWidgetPatternArguments&)=0;

  typedef boost::signals2::signal<void () >::slot_type on_dtor_slot_type;
  boost::signals2::connection connect_dtor(const on_dtor_slot_type &S)
  {
    return m_dtor_signal.connect(S);
  }

private:
  boost::signals2::signal<void () > m_dtor_signal;

#endif

};

} //namespace

#endif
