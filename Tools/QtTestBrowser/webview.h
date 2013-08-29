/*
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2009 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2006 George Staikos <staikos@kde.org>
 * Copyright (C) 2006 Dirk Mueller <mueller@kde.org>
 * Copyright (C) 2006 Zack Rusin <zack@kde.org>
 * Copyright (C) 2006 Simon Hausmann <hausmann@kde.org>
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

#ifndef webview_h
#define webview_h

#include <qwebkitwrath.h>

#if defined(WTF_USE_WRATH) && WTF_USE_WRATH
#include "WRATHTripleBufferEnabler.hpp"
#endif

#include "fpstimer.h"
#include "webpage.h"
#include <qwebview.h>
#include <qgraphicswebview.h>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QTime>
#include <QSharedPointer>

QT_BEGIN_NAMESPACE
class QStateMachine;
QT_END_NAMESPACE

#if defined(WTF_USE_WRATH) && WTF_USE_WRATH
/*
    Qt is a giant steaming failure.
    A fixed QGLContext cannot be used in multiple QGLWidget
    Apparently it was too diffucult for Qt-developers.
    The correct thing to have here would be that
    m_ctx is a _shared_ pointer and the same QGLContext
    object is used for the entire process... but
    this makes all but the first window display 
    garbage, with spams to stderr of 
    "GLContext::makeCurrent(): Cannot make invalid context current."
   
    In raw GL/WGL/GLX one can have one GL context for multiple windows,

    see: http://orion.lcg.ufrj.br/opengl/glXMakeCurrent.html
    and the discussion at http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=237071
    
    but alas apparently Qt-people did not get the memo.

    A number of ..surreal moments in life.
    We need for all the LauncherWindow's
    to be in the same GL context share group
    AND they also need a a triple buffer
    enabler.
    So what we do is this:
    - HacksForQt holds a set of QGLWidgets
    - there is only one HacksForQt object
    - all QGLWidget objects will share to the GL context of the HacksForQt object
    - it should be created at startup and deleted at end of program.
   */
class HacksForQt:public QGLWidget
{
    Q_OBJECT

public:

  
  QGLWidget*
  share_widget(void) { return this; }

  static
  HacksForQt*
  getHacksForQt(void);

  static
  void
  shut_down(void);

  const WRATHTripleBufferEnabler::handle&
  triple_buffer_enabler(void) { return m_tr; }
    
private:
  HacksForQt(void);
  ~HacksForQt();

  WRATHTripleBufferEnabler::handle m_tr;
};
#endif



class WebViewTraditional : public QWebView {
    Q_OBJECT

public:
  WebViewTraditional(QWidget* parent);
  ~WebViewTraditional();

#if defined(WTF_USE_WRATH) && WTF_USE_WRATH
    QPointF lastMousePosition(void) { return m_lastMousePosition; }
#endif

protected:
    virtual void contextMenuEvent(QContextMenuEvent*);
    virtual void mousePressEvent(QMouseEvent*);
#if defined(WTF_USE_WRATH) && WTF_USE_WRATH
    virtual void mouseMoveEvent(QMouseEvent *event);
#endif
private:
  QPointF m_lastMousePosition;
};


class GraphicsWebView : public QGraphicsWebView {
    Q_OBJECT

public:
    GraphicsWebView(QGraphicsItem* parent = 0) : QGraphicsWebView(parent) {};

protected:
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent*);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent*);
};


class WebViewGraphicsBased : public QGraphicsView {
    Q_OBJECT
    Q_PROPERTY(qreal yRotation READ yRotation WRITE setYRotation)

public:
    WebViewGraphicsBased(QWidget* parent);
    void setPage(QWebPage* page);

    void setItemCacheMode(QGraphicsItem::CacheMode mode) { graphicsWebView()->setCacheMode(mode); }
    QGraphicsItem::CacheMode itemCacheMode() { return graphicsWebView()->cacheMode(); }

    void setFrameRateMeasurementEnabled(bool enabled);
    bool frameRateMeasurementEnabled() const { return m_measureFps; }

    virtual void resizeEvent(QResizeEvent*);
    virtual void paintEvent(QPaintEvent* event);

    void setResizesToContents(bool b);
    bool resizesToContents() const { return m_resizesToContents; }

    void setYRotation(qreal angle);
    qreal yRotation() const { return m_yRotation; }

    GraphicsWebView* graphicsWebView() const { return m_item; }

public slots:
    void updateFrameRate();
    void animatedFlip();
    void animatedYFlip();
    void contentsSizeChanged(const QSize&);
    void scrollRequested(int, int);

signals:
    void currentFPSUpdated(int fps);

private:
    GraphicsWebView* m_item;
    int m_numPaintsTotal;
    int m_numPaintsSinceLastMeasure;
    QTime m_startTime;
    QTime m_lastConsultTime;
    QTimer* m_updateTimer;
    bool m_measureFps;
    qreal m_yRotation;
    bool m_resizesToContents;
    QStateMachine* m_machine;
    FpsTimer m_fpsTimer;
};

inline void WebViewGraphicsBased::setYRotation(qreal angle)
{
    QRectF r = graphicsWebView()->boundingRect();
    graphicsWebView()->setTransform(QTransform()
        .translate(r.width() / 2, r.height() / 2)
        .rotate(angle, Qt::YAxis)
        .translate(-r.width() / 2, -r.height() / 2));
    m_yRotation = angle;
}

#endif
