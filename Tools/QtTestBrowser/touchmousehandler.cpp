#if defined(WTF_USE_WRATH) && WTF_USE_WRATH

#include "touchmousehandler.h"
#include <QDateTime>
#include <QApplication>



TouchMouseHandler::TouchMouseHandler(QObject* a_parent)
    : QObject(a_parent),
    m_view(NULL), m_frame(NULL), 
    m_state(None),
    m_moveLimit(2),
    m_mouseZoomDelay(500), m_zoomShiftScale(0.01f),
    m_active(true)
{
}


void TouchMouseHandler::setView(QWebView* a_view) {
    m_view = a_view;
}


void TouchMouseHandler::setMouseZoomDelay(int a_delay) {
    if (a_delay > 0) m_mouseZoomDelay = a_delay;
    else m_mouseZoomDelay = 1;
}


bool TouchMouseHandler::handle(QEvent* a_event) {
    if (!m_active || m_view == NULL) return false; // No view, no work...
    if (m_state == PassThrough) { // For passing "clicks" to underlying view.
        return false;
    }

    // Check if this is a mouse event. Return true if handled.
    if (a_event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* e = static_cast<QMouseEvent*>(a_event);
        // We only handle left mouse button events.
        // For touch as mouse event, left button is down here.
        if (e->button() != Qt::LeftButton) return false;
        m_state = Immobile;
        m_mouseHaltTime = QDateTime::currentMSecsSinceEpoch();
        m_mousePrev = e->pos();
        
        /*
          Get the game at the position
         */
        QWebPage *pg;
        pg=m_view->page();
        if(pg) {
          m_frame=pg->frameAt(e->pos());
          if(m_frame && !m_frame->hasScrollbars()) {
            m_frame=NULL;
          }
        } else {
          m_frame=NULL;
        }


        // Start in zoom state if shift is held.
        if (e->modifiers() == Qt::ShiftModifier) m_state = Zoom;
        return true;
    } else if (a_event->type() == QEvent::MouseMove && m_state != None) {
        QMouseEvent* e = static_cast<QMouseEvent*>(a_event);
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        // There are no buttons labeled as pressed even though a button is down.
        // (That holds for mouse and actual touch received as mouse event.)
        if (m_state == Immobile) { // Has not moved yet.
            if ((e->pos() - m_mousePrev).manhattanLength() < m_moveLimit)
                return true; // Is considered not to have really moved.
            if (m_mouseZoomDelay <= now - m_mouseHaltTime) {
                m_zoomPivot = m_mousePrev;
                m_state = Zoom;
            } else m_state = Scroll;
        }
        if (m_state == Zoom) {
            float appliedZoom = m_view->getZoomValue();
            float zoomShift;
            // This is inverse of factor formula below.
            if (appliedZoom < 1.0f) zoomShift = -1.0f / appliedZoom + 1.0f;
            else zoomShift = appliedZoom - 1.0f;
            zoomShift = -zoomShift; // Set slide up to zoom out.
            float shift;
            if (m_view->rotate90degrees())
                shift = m_mousePrev.x() - e->pos().x();
            else shift = m_mousePrev.y() - e->pos().y();
            zoomShift += shift * m_zoomShiftScale;
            zoomShift = -zoomShift; // Set slide up to zoom out.
            float factor;
            if (zoomShift >= 0) factor = zoomShift + 1.0f;
            else factor = 1.0f / (-zoomShift + 1.0f);
            m_view->smoothZoom(factor / appliedZoom, m_zoomPivot);
        } else { // Scrolling.
            m_mouseHaltTime = now; // Used in button release to check swipe.
            QPoint d = m_mousePrev - e->pos();
            if (m_view->rotate90degrees()) {
              d=QPoint(-d.y(), -d.x());
            } else {
              d=QPoint(d.x(), -d.y());
            }



            if(m_frame) {
              m_frame->smoothScroll(d.x(), d.y());
              m_view->update();
            } else {
              m_view->smoothScroll(d.x(), d.y());
            }
        }
        m_mousePrev = e->pos();
        return true;
    } else if (a_event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* e = static_cast<QMouseEvent*>(a_event);
        qint64 now = QDateTime::currentMSecsSinceEpoch();
	    // We only handle left mouse button events.
        // For touch as mouse event, left button is down here.
	    if (e->button() != Qt::LeftButton) return false;
        m_view->endSmoothScrollZoom();
        if(m_frame) {
          m_frame->endSmoothScrollZoom();
          m_view->update();
          m_frame=NULL;
        }

        // Check if we should fake a click.
        if (m_state == Immobile) {
            // This might eventually display a menu. Depends on how long the
            // press was. Then again. We should eventually get real touch
            // events (in which case the test moves elsewhere).
            m_state = PassThrough;
            QMouseEvent down(QEvent::MouseButtonPress, m_mousePrev,
                Qt::LeftButton, 0, 0);
            QApplication::sendEvent(m_view, &down);
            QMouseEvent up(QEvent::MouseButtonRelease, m_mousePrev,
                Qt::LeftButton, 0, 0);
            QApplication::sendEvent(m_view, &up);
        }
        m_state = None;
	    return true;
    }
    // Check if this is a touch event. Return true if handled.
    return false; // Return false so someone else can handle this.
}


#endif // WRATH

