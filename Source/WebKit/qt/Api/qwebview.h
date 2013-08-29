/*
    Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
    Copyright (C) 2007 Staikos Computing Services Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef QWEBVIEW_H
#define QWEBVIEW_H


#if defined(WTF_USE_WRATH) && WTF_USE_WRATH
#include "WRATHConfig.hpp"
#include "WRATHTripleBufferEnabler.hpp"
#include <QtOpenGL/qgl.h>
#include <boost/signals2.hpp>

#endif

#include "qwebkitglobal.h"
#include "qwebpage.h"
#include <QtGui/qwidget.h>
#include <QtGui/qicon.h>
#include <QtGui/qpainter.h>
#include <QtCore/qurl.h>
#include <QtNetwork/qnetworkaccessmanager.h>

QT_BEGIN_NAMESPACE
class QNetworkRequest;
class QPrinter;
QT_END_NAMESPACE

class QWebView;
class QWebPage;
class QWebViewPrivate;
class QWebNetworkRequest;

#if defined(WTF_USE_WRATH) && WTF_USE_WRATH
class QWebViewScrollZoomValuePrivate;
class QWebViewScrollZoomValue
{
public:
  QWebViewScrollZoomValue(void);
  QWebViewScrollZoomValue(const QWebViewScrollZoomValue &obj);
  ~QWebViewScrollZoomValue();

  const QWebViewScrollZoomValue&
  operator=(const QWebViewScrollZoomValue &obj);

private:
  QWebViewScrollZoomValuePrivate *d;
  friend class QWebView;
};
#endif

class QWEBKIT_EXPORT QWebView :
#if defined(WTF_USE_WRATH) && WTF_USE_WRATH
 public QGLWidget
#else
 public QWidget
#endif
 {
    Q_OBJECT
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(QUrl url READ url WRITE setUrl)
    Q_PROPERTY(QIcon icon READ icon)
    Q_PROPERTY(QString selectedText READ selectedText)
    Q_PROPERTY(QString selectedHtml READ selectedHtml)
    Q_PROPERTY(bool hasSelection READ hasSelection)
    Q_PROPERTY(bool modified READ isModified)
    //Q_PROPERTY(Qt::TextInteractionFlags textInteractionFlags READ textInteractionFlags WRITE setTextInteractionFlags)
    Q_PROPERTY(qreal textSizeMultiplier READ textSizeMultiplier WRITE setTextSizeMultiplier DESIGNABLE false)
    Q_PROPERTY(qreal zoomFactor READ zoomFactor WRITE setZoomFactor)

    Q_PROPERTY(QPainter::RenderHints renderHints READ renderHints WRITE setRenderHints)
    Q_FLAGS(QPainter::RenderHints)
public:
#if defined(WTF_USE_WRATH) && WTF_USE_WRATH
      /*
        WRATH assumes a common GL-context share group.
        the argument shareWidget is another QGLWidget
        whose GL-context is in that share group.
        The GL context that will be used for the QWebView
        will then be made to be in the same share group.
       */
   QWebView(const WRATHTripleBufferEnabler::handle &tr, 
            QWidget* parent, QGLWidget *shareWidget);

    /*
      WRATH assumes a common GL-context share group.
      The argument ctx will be the GL context used
      by the QWebView and it must be in that GL context
      share group.
     */
  QWebView(const WRATHTripleBufferEnabler::handle &tr, 
           QWidget* parent, QGLContext *ctx);
    
  /*
    if there are other QWebView's or GL contexts
    that WRATH is active in, make sure they are in the
    same share group as the QWebView!
   */
  explicit
  QWebView(const WRATHTripleBufferEnabler::handle &tr, 
           QWidget* parent=0);
  

   const WRATHTripleBufferEnabler::handle& tripleBufferEnabler(void);

   void setIsWRATHUpdateDisabled(bool);
   bool isWRATHUpdateDisabled(void) const;
#endif

   explicit QWebView(QWidget* parent = 0);


   virtual ~QWebView();

    QWebPage* page() const;
    void setPage(QWebPage* page);

    void load(const QUrl& url);
    void load(const QNetworkRequest& request,
              QNetworkAccessManager::Operation operation = QNetworkAccessManager::GetOperation,
              const QByteArray &body = QByteArray());
    void setHtml(const QString& html, const QUrl& baseUrl = QUrl());
    void setContent(const QByteArray& data, const QString& mimeType = QString(), const QUrl& baseUrl = QUrl());

    QWebHistory* history() const;
    QWebSettings* settings() const;

    QString title() const;
    void setUrl(const QUrl &url);
    QUrl url() const;
    QIcon icon() const;

    bool hasSelection() const;
    QString selectedText() const;
    QString selectedHtml() const;

#ifndef QT_NO_ACTION
    QAction* pageAction(QWebPage::WebAction action) const;
#endif
    void triggerPageAction(QWebPage::WebAction action, bool checked = false);

    bool isModified() const;

    /*
    Qt::TextInteractionFlags textInteractionFlags() const;
    void setTextInteractionFlags(Qt::TextInteractionFlags flags);
    void setTextInteractionFlag(Qt::TextInteractionFlag flag);
    */

    QVariant inputMethodQuery(Qt::InputMethodQuery property) const;

    QSize sizeHint() const;

    qreal zoomFactor() const;
    void setZoomFactor(qreal factor);

    void setTextSizeMultiplier(qreal factor);
    qreal textSizeMultiplier() const;

    QPainter::RenderHints renderHints() const;
    void setRenderHints(QPainter::RenderHints hints);
    void setRenderHint(QPainter::RenderHint hint, bool enabled = true);

    bool findText(const QString& subString, QWebPage::FindFlags options = 0);

    virtual bool event(QEvent*);

#if defined(WTF_USE_WRATH) && WTF_USE_WRATH
    bool inSmoothScrollZoom(void);
    float getZoomValue() const;

    /*
      called jsut before the qwebview dtor finishes,
      so the GL context is active when it is called.
     */
    boost::signals2::connection 
    connect_dtor(const boost::signals2::signal<void () >::slot_type &slot);
#endif

public Q_SLOTS:
    void stop();
    void back();
    void forward();
    void reload();

    void print(QPrinter*) const;

#if defined(WTF_USE_WRATH) && WTF_USE_WRATH
    void useWRATHToPaint(bool);
    bool useWRATHToPaint(void);
    void clearWRATHWidgetOnNextPaint();

    /*
      if ready_all is true, even those widgets outside
      the viewing region are made ready
     */
    void markWRATHWidgetsDirty(bool ready_all=false);
    void markWRATHWidgetsDirtyForFun(void);

    void hookWRATHWidgets(QWebFrame *frame);


    void rotate90degrees(bool);
    bool rotate90degrees(void) const;

    /*
      smoothScrollZoom interface:
       - does not refresh wrath widgets on update
       - smoothScroll arguments units are in displayed pixels
       - smoothZoom multiplies the current zoom factor by factor
         zooming so that the point passed is
         in on the screen (in pixel coordinates) is fixed
       - at endSmoothScrollZoom, WebKit is informed of the scrolling
       - WebKit is NOT informed of the zoom factor, rather that is applied
         during drawing
       - For zoom factor > 1, there is additional offset applied to painting
         that webkit does not know about (for example a zoom factor of 10, scrolling
         by 5 units is half a pixel as far as webkit is concenered so is not known
         be WebKit) That left over scrolling is applied at drawing
       - resetScrollZoom resets the "draw" transformation to identity, essentially
         sets the zoom factor to 1.
       - 
     */
    void beginSmoothScrollZoom(void);
    void smoothScroll(qreal dx, qreal dy);
    void smoothZoom(qreal factor, QPointF zoomPivot);
    void endSmoothScrollZoom(void);
    void resetScrollZoom(void);

#endif

Q_SIGNALS:
    void loadStarted();
    void loadProgress(int progress);
    void loadFinished(bool);
    void titleChanged(const QString& title);
    void statusBarMessage(const QString& text);
    void linkClicked(const QUrl&);
    void selectionChanged();
    void iconChanged();
    void urlChanged(const QUrl&);

protected:
    void resizeEvent(QResizeEvent*);

#if defined(WTF_USE_WRATH) && WTF_USE_WRATH
    void paintGL();
    void resizeGL(int, int);
#endif

    void paintEvent(QPaintEvent*);

    virtual QWebView *createWindow(QWebPage::WebWindowType type);

    virtual void changeEvent(QEvent*);
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseDoubleClickEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent*);
#ifndef QT_NO_CONTEXTMENU
    virtual void contextMenuEvent(QContextMenuEvent*);
#endif
#ifndef QT_NO_WHEELEVENT
    virtual void wheelEvent(QWheelEvent*);
#endif
    virtual void keyPressEvent(QKeyEvent*);
    virtual void keyReleaseEvent(QKeyEvent*);
    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dragLeaveEvent(QDragLeaveEvent*);
    virtual void dragMoveEvent(QDragMoveEvent*);
    virtual void dropEvent(QDropEvent*);
    virtual void focusInEvent(QFocusEvent*);
    virtual void focusOutEvent(QFocusEvent*);
    virtual void inputMethodEvent(QInputMethodEvent*);

    virtual bool focusNextPrevChild(bool next);

private:

#if defined(WTF_USE_WRATH) && WTF_USE_WRATH
    void init(const WRATHTripleBufferEnabler::handle &h);
#else
    void init(void);
#endif

    friend class QWebPage;
    QWebViewPrivate* d;
    Q_PRIVATE_SLOT(d, void _q_pageDestroyed())
};

#endif // QWEBVIEW_H
