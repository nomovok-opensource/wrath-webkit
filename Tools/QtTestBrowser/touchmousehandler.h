

#if !defined(TOUCHMOUSEHANDLER_H)
#define TOUCHMOUSEHANDLER_H


#if defined(WTF_USE_WRATH) && WTF_USE_WRATH


#include <QWebView>
#include <QWebPage>
#include <QWebFrame>
#include <QEvent>
#include <QMouseEvent>
#include <QTouchEvent>
#include <QPoint>
#include <QPointF>



class TouchMouseHandler : public QObject {
    Q_OBJECT

public:
    TouchMouseHandler(QObject* a_parent = NULL);

    // Set the object to control.
    void setView(QWebView* a_view);

    // Receive mouse and touch events. Returns true if event was handled.
    bool handle(QEvent* a_event);

    int mouseZoomDelay() const { return m_mouseZoomDelay; }
    void setMouseZoomDelay(int a_delay);

    bool active(void) const { return m_active; }
    void setActive(bool v) { m_active=v; m_state=None; }
private:
    enum TouchState { None = 0, Immobile, Zoom, Scroll, PassThrough };
    QWebView* m_view;
    QWebFrame *m_frame;

    TouchState m_state; // State of the touch or mouse drag.
    int m_moveLimit; // Limit for small movements to be ignored.
    qint64 m_mouseHaltTime; // Time when last mouse movement was detected.
    QPoint m_mousePrev; // Previous mouse location.
    int m_mouseZoomDelay; // How many milliseconds to wait before zoom.
    QPointF m_zoomPivot; // Point where zoom started.
    float m_zoomShiftScale; // Multiplies distance on screen when zooming.
    bool m_active; //if false, then do nothing always
};


#endif // WRATH

#endif

