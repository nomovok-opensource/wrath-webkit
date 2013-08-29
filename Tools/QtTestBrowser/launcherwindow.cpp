/*
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2011 University of Szeged
 * Copyright (C) 2011 Kristof Kosztyo <Kosztyo.Kristof@stud.u-szeged.hu>
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

#include <qwebkitwrath.h>

#if defined(WTF_USE_WRATH) && WTF_USE_WRATH
#include "vectorGL.hpp"
#endif

#include <QtGui>
#include <QtCore>

#include "launcherwindow.h"
#include "urlloader.h"


#if !defined(QT_NO_FILEDIALOG) && !defined(QT_NO_MESSAGEBOX)
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QNetworkReply>
#endif


namespace
{
  void set_boolean_if(bool &out_value,
                      QKeyEvent *qev)
  {
    if(!qev->isAutoRepeat())
      {
        out_value=qev->type()==QEvent::KeyPress;
        qev->accept();
      }
  }
}

const int gExitClickArea = 80;
QVector<int> LauncherWindow::m_zoomLevels;



LauncherWindow::LauncherWindow(WindowOptions* data, 
                               QGraphicsScene* sharedScene)
    : MainWindow()
    , m_currentZoom(100)
    , m_currentScale(100)
    , m_urlLoader(0)
    , m_view(0)
    , m_inspector(0)
    , m_formatMenuAction(0)
    , m_zoomAnimation(0)
#if !defined(QT_NO_FILEDIALOG) && !defined(QT_NO_MESSAGEBOX)
    , m_reply(0)
#endif
#ifndef QT_NO_LINEEDIT
    , m_findFlag(0)
#endif
#if defined(WTF_USE_WRATH) && WTF_USE_WRATH
    , upKeyIsPressed(false)
    , downKeyIsPressed(false)
    , leftKeyIsPressed(false)
    , rightKeyIsPressed(false)
    , zoomInKeyIsPressed(false)
    , zoomOutKeyIsPressed(false)
    , m_key_smooth_scroll_active(false)
    , frameZoomInKeyIsPressed(false)
    , frameZoomOutKeyIsPressed(false)
    , frameUpKeyIsPressed(false)
    , frameDownKeyIsPressed(false)
    , frameLeftKeyIsPressed(false)
    , frameRightKeyIsPressed(false)
    , m_zoom_frame(0)
#endif
{
    setAttribute(Qt::WA_AcceptTouchEvents, true);
    if (data)
        m_windowOptions = *data;

    init();
    if (sharedScene && data->useGraphicsView)
        static_cast<QGraphicsView*>(m_view)->setScene(sharedScene);

    createChrome();
#if !defined(QT_NO_FILEDIALOG) && !defined(QT_NO_MESSAGEBOX)
    connect(page(), SIGNAL(downloadRequested(const QNetworkRequest&)), this, SLOT(downloadRequest(const QNetworkRequest&)));
#endif
}

LauncherWindow::~LauncherWindow()
{
    grabZoomKeys(false);
    delete m_urlLoader;
}

void LauncherWindow::init()
{
    QSplitter* splitter = new QSplitter(Qt::Vertical, this);
    setCentralWidget(splitter);

    if (m_windowOptions.startMaximized)
        setWindowState(windowState() | Qt::WindowMaximized);
    else
        resize(800, 600);

    m_inspector = new WebInspector;
#ifndef QT_NO_PROPERTIES
    if (!m_windowOptions.inspectorUrl.isEmpty())
        m_inspector->setProperty("_q_inspectorUrl", m_windowOptions.inspectorUrl);
#endif
    connect(this, SIGNAL(destroyed()), m_inspector, SLOT(deleteLater()));

    // the zoom values are chosen to be like in Mozilla Firefox 3
    if (!m_zoomLevels.count()) {
        m_zoomLevels << 30 << 50 << 67 << 80 << 90;
        m_zoomLevels << 100;
        m_zoomLevels << 110 << 120 << 133 << 150 << 170 << 200 << 240 << 300;
        m_zoomLevels << 400 << 500 << 600 << 700 << 800 << 900;
    }

    grabZoomKeys(true);

    initializeView();
}

void LauncherWindow::initializeView()
{
    delete m_view;

    m_inputUrl = addressUrl();
    QUrl url = page()->mainFrame()->url();
    setPage(new WebPage(this));

    QSplitter* splitter = static_cast<QSplitter*>(centralWidget());

    if (!m_windowOptions.useGraphicsView) {
      WebViewTraditional* view = new WebViewTraditional(splitter);
        view->setPage(page());

        view->installEventFilter(this);
        view->setAttribute(Qt::WA_AcceptTouchEvents);

        m_view = view;
// Class depends on methods defined when Wrath is in use.
#if defined(WTF_USE_WRATH) && WTF_USE_WRATH
        m_tmhandler.setView(view);
#endif
    } else {
        WebViewGraphicsBased* view = new WebViewGraphicsBased(splitter);
        m_view = view;
#if defined(QT_CONFIGURED_WITH_OPENGL)
        toggleQGLWidgetViewport(m_windowOptions.useQGLWidgetViewport);
#endif
        view->setPage(page());

        connect(view, SIGNAL(currentFPSUpdated(int)), this, SLOT(updateFPS(int)));

        view->installEventFilter(this);
        view->setAttribute(Qt::WA_AcceptTouchEvents);
        // The implementation of QAbstractScrollArea::eventFilter makes us need
        // to install the event filter also on the viewport of a QGraphicsView.
        view->viewport()->installEventFilter(this);
        view->viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
        //m_tmhandler.setView(view);
    }

    m_touchMocking = false;

    connect(page(), SIGNAL(loadStarted()), this, SLOT(loadStarted()));
    connect(page(), SIGNAL(loadFinished(bool)), this, SLOT(loadFinished()));
    connect(page(), SIGNAL(linkHovered(const QString&, const QString&, const QString&)),
            this, SLOT(showLinkHover(const QString&, const QString&)));
    connect(this, SIGNAL(enteredFullScreenMode(bool)), this, SLOT(toggleFullScreenMode(bool)));

    if (m_windowOptions.printLoadedUrls)
        connect(page()->mainFrame(), SIGNAL(urlChanged(QUrl)), this, SLOT(printURL(QUrl)));

    applyPrefs();

    splitter->addWidget(m_inspector);
    m_inspector->setPage(page());
    m_inspector->hide();

    if (m_windowOptions.remoteInspectorPort)
        page()->setProperty("_q_webInspectorServerPort", m_windowOptions.remoteInspectorPort);

    if (url.isValid())
        page()->mainFrame()->load(url);
    else  {
        setAddressUrl(m_inputUrl);
        m_inputUrl = QString();
    }
}

void LauncherWindow::applyPrefs()
{
    QWebSettings* settings = page()->settings();
    settings->setAttribute(QWebSettings::AcceleratedCompositingEnabled, m_windowOptions.useCompositing);
    settings->setAttribute(QWebSettings::TiledBackingStoreEnabled, m_windowOptions.useTiledBackingStore);
    settings->setAttribute(QWebSettings::FrameFlatteningEnabled, m_windowOptions.useFrameFlattening);
    settings->setAttribute(QWebSettings::WebGLEnabled, m_windowOptions.useWebGL);

    if (!isGraphicsBased())
        return;

    WebViewGraphicsBased* view = static_cast<WebViewGraphicsBased*>(m_view);
    view->setViewportUpdateMode(m_windowOptions.viewportUpdateMode);
    view->setFrameRateMeasurementEnabled(m_windowOptions.showFrameRate);
    view->setItemCacheMode(m_windowOptions.cacheWebView ? QGraphicsItem::DeviceCoordinateCache : QGraphicsItem::NoCache);

    if (m_windowOptions.resizesToContents)
        toggleResizesToContents(m_windowOptions.resizesToContents);
}

void LauncherWindow::createChrome()
{
#ifndef QT_NO_SHORTCUT
    QMenu* fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("New Window", this, SLOT(newWindow()), QKeySequence::New);
    fileMenu->addAction(tr("Open File..."), this, SLOT(openFile()), QKeySequence::Open);
    fileMenu->addAction(tr("Open Location..."), this, SLOT(openLocation()), QKeySequence(Qt::CTRL | Qt::Key_L));
    fileMenu->addAction("Close Window", this, SLOT(close()), QKeySequence::Close);
    fileMenu->addSeparator();
    fileMenu->addAction("Take Screen Shot...", this, SLOT(screenshot()));
#ifndef QT_NO_PRINTER
    fileMenu->addAction(tr("Print..."), this, SLOT(print()), QKeySequence::Print);
#endif
    fileMenu->addSeparator();
    fileMenu->addAction("Quit", QApplication::instance(), SLOT(closeAllWindows()), QKeySequence(Qt::CTRL | Qt::Key_Q));

    QMenu* editMenu = menuBar()->addMenu("&Edit");
    editMenu->addAction(page()->action(QWebPage::Undo));
    editMenu->addAction(page()->action(QWebPage::Redo));
    editMenu->addSeparator();
    editMenu->addAction(page()->action(QWebPage::Cut));
    editMenu->addAction(page()->action(QWebPage::Copy));
    editMenu->addAction(page()->action(QWebPage::Paste));
    editMenu->addSeparator();
#ifndef QT_NO_LINEEDIT
    editMenu->addAction("&Find", this, SLOT(showFindBar()), QKeySequence(Qt::CTRL | Qt::Key_F));
    editMenu->addSeparator();
#endif
    QAction* setEditable = editMenu->addAction("Set Editable", this, SLOT(setEditable(bool)));
    setEditable->setCheckable(true);

    QMenu* viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction(page()->action(QWebPage::Stop));
    viewMenu->addAction(page()->action(QWebPage::Reload));
    viewMenu->addSeparator();
    QAction* zoomIn = viewMenu->addAction("Zoom &In", this, SLOT(zoomIn()));
    QAction* zoomOut = viewMenu->addAction("Zoom &Out", this, SLOT(zoomOut()));
    QAction* resetZoom = viewMenu->addAction("Reset Zoom", this, SLOT(resetZoom()));
    QAction* zoomTextOnly = viewMenu->addAction("Zoom Text Only", this, SLOT(toggleZoomTextOnly(bool)));

     viewMenu->addAction("Scale In", this, SLOT(scaleIn()));
     viewMenu->addAction("Scale Out", this, SLOT(scaleOut()));
     viewMenu->addAction("Reset Scale", this, SLOT(resetScale()));


    zoomTextOnly->setCheckable(true);
    zoomTextOnly->setChecked(false);
    viewMenu->addSeparator();
    viewMenu->addAction("Dump HTML", this, SLOT(dumpHtml()));
    viewMenu->addAction("Dump render tree", this, SLOT(dumpRenderTree()));
    // viewMenu->addAction("Dump plugins", this, SLOT(dumpPlugins()));

    zoomIn->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));
    zoomOut->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));
    resetZoom->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));

    QMenu* formatMenu = new QMenu("F&ormat", this);
    m_formatMenuAction = menuBar()->addMenu(formatMenu);
    m_formatMenuAction->setVisible(false);
    formatMenu->addAction(page()->action(QWebPage::ToggleBold));
    formatMenu->addAction(page()->action(QWebPage::ToggleItalic));
    formatMenu->addAction(page()->action(QWebPage::ToggleUnderline));
    QMenu* writingMenu = formatMenu->addMenu(tr("Writing Direction"));
    writingMenu->addAction(page()->action(QWebPage::SetTextDirectionDefault));
    writingMenu->addAction(page()->action(QWebPage::SetTextDirectionLeftToRight));
    writingMenu->addAction(page()->action(QWebPage::SetTextDirectionRightToLeft));

    QMenu* windowMenu = menuBar()->addMenu("&Window");
    QAction* toggleFullScreen = windowMenu->addAction("Toggle FullScreen", this, SIGNAL(enteredFullScreenMode(bool)));
    toggleFullScreen->setShortcut(Qt::Key_F11);
    toggleFullScreen->setCheckable(true);
    toggleFullScreen->setChecked(false);
    // When exit fullscreen mode by clicking on the exit area (bottom right corner) we must
    // uncheck the Toggle FullScreen action.
    toggleFullScreen->connect(this, SIGNAL(enteredFullScreenMode(bool)), SLOT(setChecked(bool)));

    QWebSettings* settings = page()->settings();

    QMenu* toolsMenu = menuBar()->addMenu("&Develop");
    QMenu* graphicsViewMenu = toolsMenu->addMenu("QGraphicsView");
    QAction* toggleGraphicsView = graphicsViewMenu->addAction("Toggle use of QGraphicsView", this, SLOT(toggleWebView(bool)));
    toggleGraphicsView->setCheckable(true);
    toggleGraphicsView->setChecked(isGraphicsBased());

    QAction* toggleWebGL = toolsMenu->addAction("Toggle WebGL", this, SLOT(toggleWebGL(bool)));
    toggleWebGL->setCheckable(true);
    toggleWebGL->setChecked(settings->testAttribute(QWebSettings::WebGLEnabled));

    QAction* spatialNavigationAction = toolsMenu->addAction("Toggle Spatial Navigation", this, SLOT(toggleSpatialNavigation(bool)));
    spatialNavigationAction->setCheckable(true);
    spatialNavigationAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));

    QAction* toggleFrameFlattening = toolsMenu->addAction("Toggle Frame Flattening", this, SLOT(toggleFrameFlattening(bool)));
    toggleFrameFlattening->setCheckable(true);
    toggleFrameFlattening->setChecked(settings->testAttribute(QWebSettings::FrameFlatteningEnabled));

    QAction* touchMockAction = toolsMenu->addAction("Toggle touch mocking", this, SLOT(setTouchMocking(bool)));
    touchMockAction->setCheckable(true);
    touchMockAction->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_T));

    toolsMenu->addSeparator();

    QAction* toggleLocalStorage = toolsMenu->addAction("Enable Local Storage", this, SLOT(toggleLocalStorage(bool)));
    toggleLocalStorage->setCheckable(true);
    toggleLocalStorage->setChecked(m_windowOptions.useLocalStorage);

    QAction* toggleOfflineStorageDatabase = toolsMenu->addAction("Enable Offline Storage Database", this, SLOT(toggleOfflineStorageDatabase(bool)));
    toggleOfflineStorageDatabase->setCheckable(true);
    toggleOfflineStorageDatabase->setChecked(m_windowOptions.useOfflineStorageDatabase);

    QAction* toggleOfflineWebApplicationCache = toolsMenu->addAction("Enable Offline Web Application Cache", this, SLOT(toggleOfflineWebApplicationCache(bool)));
    toggleOfflineWebApplicationCache->setCheckable(true);
    toggleOfflineWebApplicationCache->setChecked(m_windowOptions.useOfflineWebApplicationCache);

    QAction* offlineStorageDefaultQuotaAction = toolsMenu->addAction("Set Offline Storage Default Quota Size", this, SLOT(setOfflineStorageDefaultQuota()));
    offlineStorageDefaultQuotaAction->setCheckable(true);
    offlineStorageDefaultQuotaAction->setChecked(m_windowOptions.offlineStorageDefaultQuotaSize);

    toolsMenu->addSeparator();

    QAction* userAgentAction = toolsMenu->addAction("Change User Agent", this, SLOT(showUserAgentDialog()));
    userAgentAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_U));

    toolsMenu->addAction("Select Elements...", this, SLOT(selectElements()));

    QAction* showInspectorAction = toolsMenu->addAction("Show Web Inspector", m_inspector, SLOT(setVisible(bool)), QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_I));
    showInspectorAction->setCheckable(true);
    showInspectorAction->connect(m_inspector, SIGNAL(visibleChanged(bool)), SLOT(setChecked(bool)));
    toolsMenu->addSeparator();
    toolsMenu->addAction("Load URLs from file", this, SLOT(loadURLListFromFile()));

    // GraphicsView sub menu.
    QAction* toggleAcceleratedCompositing = graphicsViewMenu->addAction("Toggle Accelerated Compositing", this, SLOT(toggleAcceleratedCompositing(bool)));
    toggleAcceleratedCompositing->setCheckable(true);
    toggleAcceleratedCompositing->setChecked(settings->testAttribute(QWebSettings::AcceleratedCompositingEnabled));
    toggleAcceleratedCompositing->setEnabled(isGraphicsBased());
    toggleAcceleratedCompositing->connect(toggleGraphicsView, SIGNAL(toggled(bool)), SLOT(setEnabled(bool)));

    QAction* toggleResizesToContents = graphicsViewMenu->addAction("Toggle Resizes To Contents Mode", this, SLOT(toggleResizesToContents(bool)));
    toggleResizesToContents->setCheckable(true);
    toggleResizesToContents->setChecked(m_windowOptions.resizesToContents);
    toggleResizesToContents->setEnabled(isGraphicsBased());
    toggleResizesToContents->connect(toggleGraphicsView, SIGNAL(toggled(bool)), SLOT(setEnabled(bool)));

    QAction* toggleTiledBackingStore = graphicsViewMenu->addAction("Toggle Tiled Backing Store", this, SLOT(toggleTiledBackingStore(bool)));
    toggleTiledBackingStore->setCheckable(true);
    toggleTiledBackingStore->setChecked(m_windowOptions.useTiledBackingStore);
    toggleTiledBackingStore->setEnabled(isGraphicsBased());
    toggleTiledBackingStore->connect(toggleGraphicsView, SIGNAL(toggled(bool)), SLOT(setEnabled(bool)));

#if defined(QT_CONFIGURED_WITH_OPENGL)
    QAction* toggleQGLWidgetViewport = graphicsViewMenu->addAction("Toggle use of QGLWidget Viewport", this, SLOT(toggleQGLWidgetViewport(bool)));
    toggleQGLWidgetViewport->setCheckable(true);
    toggleQGLWidgetViewport->setChecked(m_windowOptions.useQGLWidgetViewport);
    toggleQGLWidgetViewport->setEnabled(isGraphicsBased());
    toggleQGLWidgetViewport->connect(toggleGraphicsView, SIGNAL(toggled(bool)), SLOT(setEnabled(bool)));
#endif

    QMenu* viewportUpdateMenu = graphicsViewMenu->addMenu("Change Viewport Update Mode");
    viewportUpdateMenu->setEnabled(isGraphicsBased());
    viewportUpdateMenu->connect(toggleGraphicsView, SIGNAL(toggled(bool)), SLOT(setEnabled(bool)));

    QAction* fullUpdate = viewportUpdateMenu->addAction("FullViewportUpdate");
    fullUpdate->setCheckable(true);
    fullUpdate->setChecked((m_windowOptions.viewportUpdateMode == QGraphicsView::FullViewportUpdate) ? true : false);

    QAction* minimalUpdate = viewportUpdateMenu->addAction("MinimalViewportUpdate");
    minimalUpdate->setCheckable(true);
    minimalUpdate->setChecked((m_windowOptions.viewportUpdateMode == QGraphicsView::MinimalViewportUpdate) ? true : false);

    QAction* smartUpdate = viewportUpdateMenu->addAction("SmartViewportUpdate");
    smartUpdate->setCheckable(true);
    smartUpdate->setChecked((m_windowOptions.viewportUpdateMode == QGraphicsView::SmartViewportUpdate) ? true : false);

    QAction* boundingRectUpdate = viewportUpdateMenu->addAction("BoundingRectViewportUpdate");
    boundingRectUpdate->setCheckable(true);
    boundingRectUpdate->setChecked((m_windowOptions.viewportUpdateMode == QGraphicsView::BoundingRectViewportUpdate) ? true : false);

    QAction* noUpdate = viewportUpdateMenu->addAction("NoViewportUpdate");
    noUpdate->setCheckable(true);
    noUpdate->setChecked((m_windowOptions.viewportUpdateMode == QGraphicsView::NoViewportUpdate) ? true : false);

    QSignalMapper* signalMapper = new QSignalMapper(viewportUpdateMenu);
    signalMapper->setMapping(fullUpdate, QGraphicsView::FullViewportUpdate);
    signalMapper->setMapping(minimalUpdate, QGraphicsView::MinimalViewportUpdate);
    signalMapper->setMapping(smartUpdate, QGraphicsView::SmartViewportUpdate);
    signalMapper->setMapping(boundingRectUpdate, QGraphicsView::BoundingRectViewportUpdate);
    signalMapper->setMapping(noUpdate, QGraphicsView::NoViewportUpdate);

    connect(fullUpdate, SIGNAL(triggered()), signalMapper, SLOT(map()));
    connect(minimalUpdate, SIGNAL(triggered()), signalMapper, SLOT(map()));
    connect(smartUpdate, SIGNAL(triggered()), signalMapper, SLOT(map()));
    connect(boundingRectUpdate, SIGNAL(triggered()), signalMapper, SLOT(map()));
    connect(noUpdate, SIGNAL(triggered()), signalMapper, SLOT(map()));

    connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(changeViewportUpdateMode(int)));

    QActionGroup* viewportUpdateModeActions = new QActionGroup(viewportUpdateMenu);
    viewportUpdateModeActions->addAction(fullUpdate);
    viewportUpdateModeActions->addAction(minimalUpdate);
    viewportUpdateModeActions->addAction(smartUpdate);
    viewportUpdateModeActions->addAction(boundingRectUpdate);
    viewportUpdateModeActions->addAction(noUpdate);

    graphicsViewMenu->addSeparator();

    QAction* flipAnimated = graphicsViewMenu->addAction("Animated Flip");
    flipAnimated->connect(toggleGraphicsView, SIGNAL(toggled(bool)), SLOT(setEnabled(bool)));
    flipAnimated->setEnabled(isGraphicsBased());
    connect(flipAnimated, SIGNAL(triggered()), SLOT(animatedFlip()));

    QAction* flipYAnimated = graphicsViewMenu->addAction("Animated Y-Flip");
    flipYAnimated->connect(toggleGraphicsView, SIGNAL(toggled(bool)), SLOT(setEnabled(bool)));
    flipYAnimated->setEnabled(isGraphicsBased());
    connect(flipYAnimated, SIGNAL(triggered()), SLOT(animatedYFlip()));

    QAction* cloneWindow = graphicsViewMenu->addAction("Clone Window", this, SLOT(cloneWindow()));
    cloneWindow->connect(toggleGraphicsView, SIGNAL(toggled(bool)), SLOT(setEnabled(bool)));
    cloneWindow->setEnabled(isGraphicsBased());

    graphicsViewMenu->addSeparator();

    QAction* showFPS = graphicsViewMenu->addAction("Show FPS", this, SLOT(showFPS(bool)));
    showFPS->setCheckable(true);
    showFPS->setEnabled(isGraphicsBased());
    showFPS->connect(toggleGraphicsView, SIGNAL(toggled(bool)), SLOT(setEnabled(bool)));
    showFPS->setChecked(m_windowOptions.showFrameRate);

    QMenu* settingsMenu = menuBar()->addMenu("&Settings");

    QAction* toggleAutoLoadImages = settingsMenu->addAction("Disable Auto Load Images", this, SLOT(toggleAutoLoadImages(bool)));
    toggleAutoLoadImages->setCheckable(true);
    toggleAutoLoadImages->setChecked(false);

    QAction* togglePlugins = settingsMenu->addAction("Disable Plugins", this, SLOT(togglePlugins(bool)));
    togglePlugins->setCheckable(true);
    togglePlugins->setChecked(false);

    QAction* toggleInterruptingJavaScripteEnabled = settingsMenu->addAction("Enable interrupting js scripts", this, SLOT(toggleInterruptingJavaScriptEnabled(bool)));
    toggleInterruptingJavaScripteEnabled->setCheckable(true);
    toggleInterruptingJavaScripteEnabled->setChecked(false);

    QAction* toggleJavascriptCanOpenWindows = settingsMenu->addAction("Enable js popup windows", this, SLOT(toggleJavascriptCanOpenWindows(bool)));
    toggleJavascriptCanOpenWindows->setCheckable(true);
    toggleJavascriptCanOpenWindows->setChecked(false);

#if defined(WTF_USE_WRATH) && WTF_USE_WRATH
    QMenu* wrathMenu = menuBar()->addMenu("&WRATH");
    toggleUseWRATHaction=wrathMenu->addAction("Use WRATH to draw", this,
                                              SLOT(toggleUseWRATH(bool)));
    
    toggleUseWRATHaction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    toggleUseWRATHaction->setCheckable(true);

    toggleKeySmoothAction=wrathMenu->addAction("Keyboard SmoothScrollZoom", 
                                               this,
                                               SLOT(toggleKeySmoothScroll(bool)));
    toggleKeySmoothAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    toggleKeySmoothAction->setCheckable(true);
    toggleKeySmoothAction->setChecked(m_key_smooth_scroll_active);
    
    toggleMouseTouchSmoothAction=wrathMenu->addAction("Mouse SmoothScrollZoom", 
                                                      this,
                                                      SLOT(toggleMouseTouchSmoothScroll(bool)));
    toggleMouseTouchSmoothAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
    toggleMouseTouchSmoothAction->setCheckable(true);
    toggleMouseTouchSmoothAction->setChecked(m_tmhandler.active());

    wrathMenu->addAction("Reset SmoothScrollZoom",
                         this,
                         SLOT(resetSmoothScroll()));


    toggleRotate=wrathMenu->addAction("Rotate 90 degress",
                                      this,
                                      SLOT(rotate90degrees(bool)));
    toggleRotate->setCheckable(true);
    toggleRotate->setChecked(false);
    toggleRotate->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_A));

    QAction* clearWRATH=wrathMenu->addAction("Clear WRATH widgets", this,
                                             SLOT(clearWRATH()));
    clearWRATH->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));

    QAction *markWidgetsDirty=wrathMenu->addAction("Mark WRATH Widgets dirty", this,
                                                   SLOT(markWRATHWidgetsDirty()) );
    markWidgetsDirty->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_X));

   
#endif
#ifndef QT_NO_LINEEDIT
    m_findBar = new QToolBar("Find", this);
    addToolBar(Qt::BottomToolBarArea, m_findBar);

    QToolButton* findClose = new QToolButton(m_findBar);
    findClose->setText("X");
    m_lineEdit = new QLineEdit(m_findBar);
    m_lineEdit->setMaximumWidth(200);
    QToolButton* findPrevious = new QToolButton(m_findBar);
    findPrevious->setArrowType(Qt::LeftArrow);
    QToolButton* findNext = new QToolButton(m_findBar);
    findNext->setArrowType(Qt::RightArrow);
    QCheckBox* findCaseSensitive = new QCheckBox("Case Sensitive", m_findBar);
    QCheckBox* findWrapAround = new QCheckBox("Wrap Around", m_findBar);
    QCheckBox* findHighLightAll = new QCheckBox("HighLight All", m_findBar);

    QSignalMapper* findSignalMapper = new QSignalMapper(m_findBar);
    findSignalMapper->setMapping(m_lineEdit, s_findNormalFlag);
    findSignalMapper->setMapping(findPrevious, QWebPage::FindBackward);
    findSignalMapper->setMapping(findNext, s_findNormalFlag);
    findSignalMapper->setMapping(findCaseSensitive, QWebPage::FindCaseSensitively);
    findSignalMapper->setMapping(findWrapAround, QWebPage::FindWrapsAroundDocument);
    findSignalMapper->setMapping(findHighLightAll, QWebPage::HighlightAllOccurrences);

    connect(findClose, SIGNAL(clicked()), this, SLOT(showFindBar()));
    connect(m_lineEdit, SIGNAL(textChanged(const QString &)), findSignalMapper, SLOT(map()));
    connect(findPrevious, SIGNAL(pressed()), findSignalMapper, SLOT(map()));
    connect(findNext, SIGNAL(pressed()), findSignalMapper, SLOT(map()));
    connect(findCaseSensitive, SIGNAL(stateChanged(int)), findSignalMapper, SLOT(map()));
    connect(findWrapAround, SIGNAL(stateChanged(int)), findSignalMapper, SLOT(map()));
    connect(findHighLightAll, SIGNAL(stateChanged(int)), findSignalMapper, SLOT(map()));

    connect(findSignalMapper, SIGNAL(mapped(int)), this, SLOT(find(int)));

    m_findBar->addWidget(findClose);
    m_findBar->addWidget(m_lineEdit);
    m_findBar->addWidget(findPrevious);
    m_findBar->addWidget(findNext);
    m_findBar->addWidget(findCaseSensitive);
    m_findBar->addWidget(findWrapAround);
    m_findBar->addWidget(findHighLightAll);
    m_findBar->setMovable(false);
    m_findBar->setVisible(false);
#endif
#endif
    //toggleFullScreenMode(true);
}

bool LauncherWindow::isGraphicsBased() const
{
    return bool(qobject_cast<QGraphicsView*>(m_view));
}

void LauncherWindow::keyPressEvent(QKeyEvent* event)
{
  //#ifdef Q_WS_MAEMO_5
    switch (event->key()) {
    case Qt::Key_F7:
        zoomIn();
        event->accept();
        break;
    case Qt::Key_F8:
        zoomOut();
        event->accept();
        break;
    case Qt::Key_F9:
        scaleIn();
        event->accept();
        break;
    case Qt::Key_F10:
        scaleOut();
        event->accept();
        break;
    }
    //#endif
    MainWindow::keyPressEvent(event);
}

void LauncherWindow::grabZoomKeys(bool grab)
{
#ifdef Q_WS_MAEMO_5
    if (!winId()) {
        qWarning("Can't grab keys unless we have a window id");
        return;
    }

    Atom atom = XInternAtom(QX11Info::display(), "_HILDON_ZOOM_KEY_ATOM", False);
    if (!atom) {
        qWarning("Unable to obtain _HILDON_ZOOM_KEY_ATOM");
        return;
    }

    unsigned long val = (grab) ? 1 : 0;
    XChangeProperty(QX11Info::display(), winId(), atom, XA_INTEGER, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&val), 1);
#endif
}

void LauncherWindow::sendTouchEvent()
{
    if (m_touchPoints.isEmpty())
        return;

    QEvent::Type type = QEvent::TouchUpdate;
    if (m_touchPoints.size() == 1) {
        if (m_touchPoints[0].state() == Qt::TouchPointReleased)
            type = QEvent::TouchEnd;
        else if (m_touchPoints[0].state() == Qt::TouchPointPressed)
            type = QEvent::TouchBegin;
    }

    QTouchEvent touchEv(type);
    touchEv.setTouchPoints(m_touchPoints);
    QCoreApplication::sendEvent(page(), &touchEv);

    // After sending the event, remove all touchpoints that were released
    if (m_touchPoints[0].state() == Qt::TouchPointReleased)
        m_touchPoints.removeAt(0);
    if (m_touchPoints.size() > 1 && m_touchPoints[1].state() == Qt::TouchPointReleased)
        m_touchPoints.removeAt(1);
}

bool LauncherWindow::eventFilter(QObject* obj, QEvent* event)
{
#if defined(WTF_USE_WRATH) && WTF_USE_WRATH
  WebViewTraditional *p;
  p=qobject_cast<WebViewTraditional*>(m_view);

  if(p)
    {
      toggleUseWRATHaction->setChecked(p->useWRATHToPaint());
      toggleRotate->setChecked(p->rotate90degrees());
    }

  if(p && m_key_smooth_scroll_active)
    {
      bool return_value;
      
      return_value=handleEventDuringSmooth(event);
      if(return_value)
        {
          return return_value;
        }
    }
  if (m_tmhandler.handle(event)) return true;
#endif

    // If click pos is the bottom right corner (square with size defined by gExitClickArea)
    // and the window is on FullScreen, the window must return to its original state.
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* ev = static_cast<QMouseEvent*>(event);
        if (windowState() == Qt::WindowFullScreen
            && ev->pos().x() > (width() - gExitClickArea)
            && ev->pos().y() > (height() - gExitClickArea)) {

            // This does not appear to work on Iconia.
            emit enteredFullScreenMode(false);
            return true;
        }
        // Top right corner (below end of toolbar) toggles menu bar visibility.
        if (ev->pos().x() > (width() - gExitClickArea)
            && ev->pos().y() < gExitClickArea)
        {
            if (menuBar()->isHidden()) menuBar()->show();
            else menuBar()->hide();
            return true;
        }
    }

    if (!m_touchMocking)
        return QObject::eventFilter(obj, event);

    if (event->type() == QEvent::MouseButtonPress
        || event->type() == QEvent::MouseButtonRelease
        || event->type() == QEvent::MouseButtonDblClick
        || event->type() == QEvent::MouseMove) {

        QMouseEvent* ev = static_cast<QMouseEvent*>(event);
        if (ev->type() == QEvent::MouseMove
            && !(ev->buttons() & Qt::LeftButton))
            return false;

        QTouchEvent::TouchPoint touchPoint;
        touchPoint.setState(Qt::TouchPointMoved);
        if ((ev->type() == QEvent::MouseButtonPress
             || ev->type() == QEvent::MouseButtonDblClick))
            touchPoint.setState(Qt::TouchPointPressed);
        else if (ev->type() == QEvent::MouseButtonRelease)
            touchPoint.setState(Qt::TouchPointReleased);

        touchPoint.setId(0);
        touchPoint.setScreenPos(ev->globalPos());
        touchPoint.setPos(ev->pos());
        touchPoint.setPressure(1);

        // If the point already exists, update it. Otherwise create it.
        if (m_touchPoints.size() > 0 && !m_touchPoints[0].id())
            m_touchPoints[0] = touchPoint;
        else if (m_touchPoints.size() > 1 && !m_touchPoints[1].id())
            m_touchPoints[1] = touchPoint;
        else
            m_touchPoints.append(touchPoint);

        sendTouchEvent();
    } else if (event->type() == QEvent::KeyPress
        && static_cast<QKeyEvent*>(event)->key() == Qt::Key_F
        && static_cast<QKeyEvent*>(event)->modifiers() == Qt::ControlModifier) {

        // If the keyboard point is already pressed, release it.
        // Otherwise create it and append to m_touchPoints.
        if (m_touchPoints.size() > 0 && m_touchPoints[0].id() == 1) {
            m_touchPoints[0].setState(Qt::TouchPointReleased);
            sendTouchEvent();
        } else if (m_touchPoints.size() > 1 && m_touchPoints[1].id() == 1) {
            m_touchPoints[1].setState(Qt::TouchPointReleased);
            sendTouchEvent();
        } else {
            QTouchEvent::TouchPoint touchPoint;
            touchPoint.setState(Qt::TouchPointPressed);
            touchPoint.setId(1);
            touchPoint.setScreenPos(QCursor::pos());
            touchPoint.setPos(m_view->mapFromGlobal(QCursor::pos()));
            touchPoint.setPressure(1);
            m_touchPoints.append(touchPoint);
            sendTouchEvent();

            // After sending the event, change the touchpoint state to stationary
            m_touchPoints.last().setState(Qt::TouchPointStationary);
        }
    }

    return false;
}

void LauncherWindow::loadStarted()
{
    m_view->setFocus(Qt::OtherFocusReason);
}

void LauncherWindow::loadFinished()
{
    QUrl url = page()->mainFrame()->url();
    addCompleterEntry(url);
    if (m_inputUrl.isEmpty())
        setAddressUrl(url.toString(QUrl::RemoveUserInfo));
    else {
        setAddressUrl(m_inputUrl);
        m_inputUrl = QString();
    }

#if defined(WTF_USE_WRATH) && WTF_USE_WRATH
    WebViewTraditional *p;
    p=qobject_cast<WebViewTraditional*>(m_view);
    if(p)
      {
        /*
          the true argument indicates to also make
          the WRATH widgets ready for even the contents
          of the frame that are not visible.
         */
        p->markWRATHWidgetsDirty(true);
      }
#endif

  std::cout << "\n\nFinish Load "
            << page()->mainFrame()->url().toString().toLatin1().constData()
            << "\n----------------------------------------\n\n";
}

void LauncherWindow::showLinkHover(const QString &link, const QString &toolTip)
{
#ifndef Q_WS_MAEMO_5
    statusBar()->showMessage(link);
#endif
#ifndef QT_NO_TOOLTIP
    if (!toolTip.isEmpty())
        QToolTip::showText(QCursor::pos(), toolTip);
#endif
}

void LauncherWindow::zoomAnimationFinished()
{
    if (!isGraphicsBased())
        return;
    QGraphicsWebView* view = static_cast<WebViewGraphicsBased*>(m_view)->graphicsWebView();
    view->setTiledBackingStoreFrozen(false);
}

void LauncherWindow::applyZoom()
{
#ifndef QT_NO_ANIMATION
    if (isGraphicsBased() && page()->settings()->testAttribute(QWebSettings::TiledBackingStoreEnabled)) {
        QGraphicsWebView* view = static_cast<WebViewGraphicsBased*>(m_view)->graphicsWebView();
        view->setTiledBackingStoreFrozen(true);
        if (!m_zoomAnimation) {
            m_zoomAnimation = new QPropertyAnimation(view, "scale");
            m_zoomAnimation->setStartValue(view->scale());
            connect(m_zoomAnimation, SIGNAL(finished()), this, SLOT(zoomAnimationFinished()));
        } else {
            m_zoomAnimation->stop();
            m_zoomAnimation->setStartValue(m_zoomAnimation->currentValue());
        }

        m_zoomAnimation->setDuration(300);
        m_zoomAnimation->setEndValue(qreal(m_currentZoom) / 100.);
        m_zoomAnimation->start();
        return;
    }
#endif
    page()->mainFrame()->setZoomFactor(qreal(m_currentZoom) / 100.0);
}

void LauncherWindow::applyScale()
{
  page()->mainFrame()->setScaleFactor( static_cast<float>(m_currentScale)/100.0f);
}

void LauncherWindow::zoomIn()
{
    int i = m_zoomLevels.indexOf(m_currentZoom);
    Q_ASSERT(i >= 0);
    if (i < m_zoomLevels.count() - 1)
        m_currentZoom = m_zoomLevels[i + 1];

    applyZoom();
}

void LauncherWindow::zoomOut()
{
    int i = m_zoomLevels.indexOf(m_currentZoom);
    Q_ASSERT(i >= 0);
    if (i > 0)
        m_currentZoom = m_zoomLevels[i - 1];

    applyZoom();
}

void LauncherWindow::resetZoom()
{
    m_currentZoom = 100;
    applyZoom();
}

//

void LauncherWindow::scaleIn()
{
    int i = m_zoomLevels.indexOf(m_currentScale);
    Q_ASSERT(i >= 0);
    if (i < m_zoomLevels.count() - 1)
        m_currentScale = m_zoomLevels[i + 1];

    applyScale();
}

void LauncherWindow::scaleOut()
{
    int i = m_zoomLevels.indexOf(m_currentScale);
    Q_ASSERT(i >= 0);
    if (i > 0)
        m_currentScale = m_zoomLevels[i - 1];

    applyScale();
}

void LauncherWindow::resetScale()
{
    m_currentScale = 100;
    applyScale();
}


//




void LauncherWindow::toggleZoomTextOnly(bool b)
{
    page()->settings()->setAttribute(QWebSettings::ZoomTextOnly, b);
}

void LauncherWindow::print()
{
#if !defined(QT_NO_PRINTER)
    QPrintPreviewDialog dlg(this);
    connect(&dlg, SIGNAL(paintRequested(QPrinter*)),
            page()->mainFrame(), SLOT(print(QPrinter*)));
    dlg.exec();
#endif
}

void LauncherWindow::screenshot()
{
    QPixmap pixmap = QPixmap::grabWidget(m_view);
    QLabel* label = 0;
#if !defined(Q_OS_SYMBIAN)
    label = new QLabel;
    label->setAttribute(Qt::WA_DeleteOnClose);
    label->setWindowTitle("Screenshot - Preview");
    label->setPixmap(pixmap);
    label->show();
#endif

#ifndef QT_NO_FILEDIALOG
    QString fileName = QFileDialog::getSaveFileName(label, "Screenshot");
    if (!fileName.isEmpty()) {
        pixmap.save(fileName, "png");
        if (label)
            label->setWindowTitle(QString("Screenshot - Saved at %1").arg(fileName));
    }
#endif

#if defined(QT_CONFIGURED_WITH_OPENGL)
    toggleQGLWidgetViewport(m_windowOptions.useQGLWidgetViewport);
#endif
}

void LauncherWindow::setEditable(bool on)
{
    page()->setContentEditable(on);
    m_formatMenuAction->setVisible(on);
}

/*
void LauncherWindow::dumpPlugins() {
    QList<QWebPluginInfo> plugins = QWebSettings::pluginDatabase()->plugins();
    foreach (const QWebPluginInfo plugin, plugins) {
        qDebug() << "Plugin:" << plugin.name();
        foreach (const QWebPluginInfo::MimeType mime, plugin.mimeTypes()) {
            qDebug() << "   " << mime.name;
        }
    }
}
*/

void LauncherWindow::dumpHtml()
{
    qDebug() << "HTML: " << page()->mainFrame()->toHtml();
}

void LauncherWindow::dumpRenderTree()
{
    qDebug() << "Render tree: " << page()->mainFrame()->renderTreeDump();
}

void LauncherWindow::selectElements()
{
#ifndef QT_NO_INPUTDIALOG
    bool ok;
    QString str = QInputDialog::getText(this, "Select elements", "Choose elements",
                                        QLineEdit::Normal, "a", &ok);

    if (ok && !str.isEmpty()) {
        QWebElementCollection result =  page()->mainFrame()->findAllElements(str);
        foreach (QWebElement e, result)
            e.setStyleProperty("background-color", "yellow");
#ifndef Q_WS_MAEMO_5
        statusBar()->showMessage(QString("%1 element(s) selected").arg(result.count()), 5000);
#endif
    }
#endif
}

void LauncherWindow::setTouchMocking(bool on)
{
    m_touchMocking = on;
}

void LauncherWindow::toggleWebView(bool graphicsBased)
{
    m_windowOptions.useGraphicsView = graphicsBased;
    initializeView();
    menuBar()->clear();
    createChrome();
}

void LauncherWindow::toggleAcceleratedCompositing(bool toggle)
{
    m_windowOptions.useCompositing = toggle;
    page()->settings()->setAttribute(QWebSettings::AcceleratedCompositingEnabled, toggle);
}

void LauncherWindow::toggleTiledBackingStore(bool toggle)
{
    page()->settings()->setAttribute(QWebSettings::TiledBackingStoreEnabled, toggle);
}

void LauncherWindow::toggleResizesToContents(bool toggle)
{
    m_windowOptions.resizesToContents = toggle;
    static_cast<WebViewGraphicsBased*>(m_view)->setResizesToContents(toggle);
}

void LauncherWindow::toggleWebGL(bool toggle)
{
    m_windowOptions.useWebGL = toggle;
    page()->settings()->setAttribute(QWebSettings::WebGLEnabled, toggle);
}

void LauncherWindow::animatedFlip()
{
    qobject_cast<WebViewGraphicsBased*>(m_view)->animatedFlip();
}

void LauncherWindow::animatedYFlip()
{
    qobject_cast<WebViewGraphicsBased*>(m_view)->animatedYFlip();
}
void LauncherWindow::toggleSpatialNavigation(bool b)
{
    page()->settings()->setAttribute(QWebSettings::SpatialNavigationEnabled, b);
}

void LauncherWindow::toggleFullScreenMode(bool enable)
{
    bool alreadyEnabled = windowState() & Qt::WindowFullScreen;
    if (enable ^ alreadyEnabled)
        setWindowState(windowState() ^ Qt::WindowFullScreen);
}

void LauncherWindow::toggleFrameFlattening(bool toggle)
{
    m_windowOptions.useFrameFlattening = toggle;
    page()->settings()->setAttribute(QWebSettings::FrameFlatteningEnabled, toggle);
}

void LauncherWindow::toggleInterruptingJavaScriptEnabled(bool enable)
{
    page()->setInterruptingJavaScriptEnabled(enable);
}

void LauncherWindow::toggleJavascriptCanOpenWindows(bool enable)
{
    page()->settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, enable);
}

#if defined(WTF_USE_WRATH) && WTF_USE_WRATH
void LauncherWindow::rotate90degrees(bool v)
{
  WebViewTraditional *p;
  p=static_cast<WebViewTraditional*>(m_view);
  if(p)
    {
      p->rotate90degrees(v);
    }
}


bool LauncherWindow::handleEventDuringSmooth(QEvent * ev)
{
  WebViewTraditional *p;
  bool is_key_event, return_value(false);

  p=static_cast<WebViewTraditional*>(m_view);

  if(ev->type()==QEvent::MouseMove)
    {
      QMouseEvent *mev(static_cast<QMouseEvent*>(ev));
      m_zoom_frame=p->page()->frameAt(mev->pos());
      if(m_zoom_frame==p->page()->mainFrame())
        {
          m_zoom_frame=0;
        }
    }

  is_key_event=ev->type()==QEvent::KeyPress
    or ev->type()==QEvent::KeyRelease;

  if(is_key_event)
    {
      QKeyEvent *qev(static_cast<QKeyEvent*>(ev));

      switch (qev->key()) {
      case Qt::Key_U:
      case Qt::Key_VolumeDown:
        set_boolean_if(zoomInKeyIsPressed, qev);
        return_value=true;
        break;
        
      case Qt::Key_O:
      case Qt::Key_VolumeUp:
        set_boolean_if(zoomOutKeyIsPressed, qev);
        return_value=true;
        break;
        
      case Qt::Key_J:
        set_boolean_if(leftKeyIsPressed, qev);
        return_value=true;
        break;
        
      case Qt::Key_L:
        set_boolean_if(rightKeyIsPressed, qev);
        return_value=true;
        break;
        
      case Qt::Key_I:
        set_boolean_if(upKeyIsPressed, qev);
        return_value=true;
        break;
        
      case Qt::Key_K:
        set_boolean_if(downKeyIsPressed, qev);
        return_value=true;
        break;

      case Qt::Key_R:
        set_boolean_if(frameZoomOutKeyIsPressed, qev);
        return_value=true;
        break;

      case Qt::Key_Y:
        set_boolean_if(frameZoomInKeyIsPressed, qev);
        return_value=true;
        break;

      case Qt::Key_T:
        set_boolean_if(frameUpKeyIsPressed, qev);
        return_value=true;
        break;

      case Qt::Key_G:
        set_boolean_if(frameDownKeyIsPressed, qev);
        return_value=true;
        break;

      case Qt::Key_F:
        set_boolean_if(frameLeftKeyIsPressed, qev);
        return_value=true;
        break;

      case Qt::Key_H:
        set_boolean_if(frameRightKeyIsPressed, qev);
        return_value=true;
        break;
      }
    }


  bool shoud_be_in_smooth_scroll;

  shoud_be_in_smooth_scroll=
    (zoomInKeyIsPressed xor zoomOutKeyIsPressed)
    or (leftKeyIsPressed xor rightKeyIsPressed) 
    or (upKeyIsPressed xor downKeyIsPressed);
  
  if(shoud_be_in_smooth_scroll!=p->inSmoothScrollZoom())
    {
      if(shoud_be_in_smooth_scroll)
        {
          p->beginSmoothScrollZoom();
          smoothScrollTimer.restart();
        }
      else
        {
          p->endSmoothScrollZoom();
        }
    }
  
  bool should_be_in_frame_zoom;

  should_be_in_frame_zoom=
    frameZoomInKeyIsPressed xor frameZoomOutKeyIsPressed
    or (frameUpKeyIsPressed xor frameDownKeyIsPressed)
    or (frameLeftKeyIsPressed xor frameRightKeyIsPressed);

  if(m_zoom_frame && m_zoom_frame->inSmoothScrollZoom()!=should_be_in_frame_zoom)
    {
      if(should_be_in_frame_zoom)
        {
          m_zoom_frame->beginSmoothScrollZoom();
          smoothZoomFrameTimer.restart();
        }
      else
        {
          m_zoom_frame->endSmoothScrollZoom();
        }
      
    }

  if(shoud_be_in_smooth_scroll)
    {
      doSmoothScroll(p);
    }

  if(m_zoom_frame && should_be_in_frame_zoom)
    {
      doSmoothFrame(p);
    }
  
  return return_value;
}

void LauncherWindow::paintEvent(QPaintEvent *event)
{
  MainWindow::paintEvent(event);

  WebViewTraditional *p;
  p=qobject_cast<WebViewTraditional*>(m_view);
  if(p)
    {
      bool scroll, zoom;

      zoom=zoomInKeyIsPressed xor zoomOutKeyIsPressed;
      scroll=(upKeyIsPressed xor downKeyIsPressed) or (leftKeyIsPressed xor rightKeyIsPressed);
      if(zoom or scroll)
        {
          p->update();
        }
    }
}

void LauncherWindow::doSmoothFrame(QWebView *p)
{
  float elapsed_time;
  int elapsed_timeI;
  vec2 pixels_per_ms(0.3f, 0.3f);
  vec2 delta_value(0.0f, 0.0f);
  vec2 pixels_to_advance;

  elapsed_timeI=smoothZoomFrameTimer.restart();
  elapsed_time=static_cast<float>(elapsed_timeI);
  pixels_to_advance=elapsed_time*pixels_per_ms;

  if(frameLeftKeyIsPressed)
    {
      delta_value.x()-=pixels_to_advance.x();
    }

  if(frameRightKeyIsPressed)
    {
      delta_value.x()+=pixels_to_advance.x();
    }

  if(frameUpKeyIsPressed)
    {
      delta_value.y()+=pixels_to_advance.y();
    }

  if(frameDownKeyIsPressed)
    {
      delta_value.y()-=pixels_to_advance.y();
    }
  m_zoom_frame->smoothScroll(delta_value.x(), delta_value.y());

  if(frameZoomInKeyIsPressed xor frameZoomOutKeyIsPressed)
    {
      float zoom_factor;

      zoom_factor=powf(1.001, elapsed_time);
      if(frameZoomOutKeyIsPressed)
        {
          zoom_factor=1.0f/zoom_factor;
        }
      QPointF pivot(0.0f, 0.0f); //m_zoom_frame->width()/2, m_zoom_frame->height()/2);
      m_zoom_frame->smoothZoom(zoom_factor, pivot);
    }

  p->update();
}

void LauncherWindow::doSmoothScroll(QWebView *p)
{
  float elapsed_time;
  int elapsed_timeI;
  vec2 pixels_per_ms(0.3f, 0.3f);
  vec2 delta_value(0.0f, 0.0f);
  vec2 pixels_to_advance;

  elapsed_timeI=smoothScrollTimer.restart();
  elapsed_time=static_cast<float>(elapsed_timeI);
  pixels_to_advance=elapsed_time*pixels_per_ms;

  /*
  if(elapsed_timeI>0)
    {
      std::cout << "delta_t=" << elapsed_time
                << " ms (" << 1000.0f/elapsed_time
                << " FPS)\n" << std::flush;
    }
  */

  if(leftKeyIsPressed)
    {
      delta_value.x()-=pixels_to_advance.x();
    }

  if(rightKeyIsPressed)
    {
      delta_value.x()+=pixels_to_advance.x();
    }

  if(upKeyIsPressed)
    {
      delta_value.y()+=pixels_to_advance.y();
    }

  if(downKeyIsPressed)
    {
      delta_value.y()-=pixels_to_advance.y();
    }
  p->smoothScroll(delta_value.x(), delta_value.y());

  if(zoomInKeyIsPressed xor zoomOutKeyIsPressed)
    {
      float zoom_factor;

      zoom_factor=powf(1.001, elapsed_time);
      if(zoomOutKeyIsPressed)
        {
          zoom_factor=1.0f/zoom_factor;
        }
      QPointF pivot(p->width()/2, p->height()/2);
      p->smoothZoom(zoom_factor, pivot);
    }
}


void LauncherWindow::resetSmoothScroll()
{
  WebViewTraditional *p;

  p=qobject_cast<WebViewTraditional*>(m_view);
  if(p)
    {
      p->resetScrollZoom();
    }
}

void LauncherWindow::toggleUseWRATH(bool v)
{
  WebViewTraditional *p;

  p=qobject_cast<WebViewTraditional*>(m_view);
  if(p)
    {
      if(v)
        {
          markWRATHWidgetsDirty();
          std::cout << "\n------------------------------\nUse WRATH to paint\n\n";
        }
      else
        {
          std::cout << "\n------------------------------\nUse Qt to paint\n\n";
        }
      p->useWRATHToPaint(v);
      p->update();
    }
}

void LauncherWindow::toggleKeySmoothScroll(bool b)
{
  m_key_smooth_scroll_active=b;
}

void LauncherWindow::toggleMouseTouchSmoothScroll(bool b)
{
  m_tmhandler.setActive(b);
}



void LauncherWindow::markWRATHWidgetsDirty()
{
  WebViewTraditional *p;

  p=qobject_cast<WebViewTraditional*>(m_view);
  if(p)
    {
      p->markWRATHWidgetsDirty();
    }
}


void LauncherWindow::clearWRATH()
{
  WebViewTraditional *p;

  p=qobject_cast<WebViewTraditional*>(m_view);
  if(p)
    {
      p->clearWRATHWidgetOnNextPaint();
    }
}



#endif

void LauncherWindow::toggleAutoLoadImages(bool enable)
{
    page()->settings()->setAttribute(QWebSettings::AutoLoadImages, !enable);
}

void LauncherWindow::togglePlugins(bool enable)
{
    page()->settings()->setAttribute(QWebSettings::PluginsEnabled, !enable);
}

#if defined(QT_CONFIGURED_WITH_OPENGL)
void LauncherWindow::toggleQGLWidgetViewport(bool enable)
{
    if (!isGraphicsBased())
        return;

    m_windowOptions.useQGLWidgetViewport = enable;
    WebViewGraphicsBased* view = static_cast<WebViewGraphicsBased*>(m_view);

    view->setViewport(enable ? new QGLWidget() : 0);
}
#endif

void LauncherWindow::changeViewportUpdateMode(int mode)
{
    m_windowOptions.viewportUpdateMode = QGraphicsView::ViewportUpdateMode(mode);

    if (!isGraphicsBased())
        return;

    WebViewGraphicsBased* view = static_cast<WebViewGraphicsBased*>(m_view);
    view->setViewportUpdateMode(m_windowOptions.viewportUpdateMode);
}

void LauncherWindow::showFPS(bool enable)
{
    if (!isGraphicsBased())
        return;

    m_windowOptions.showFrameRate = enable;
    WebViewGraphicsBased* view = static_cast<WebViewGraphicsBased*>(m_view);
    view->setFrameRateMeasurementEnabled(enable);

    if (!enable) {
#if defined(Q_WS_MAEMO_5) && defined(Q_OS_SYMBIAN)
        setWindowTitle("");
#else
        statusBar()->clearMessage();
#endif
    }
}

void LauncherWindow::showUserAgentDialog()
{
    QStringList items;
    QFile file(":/useragentlist.txt");
    if (file.open(QIODevice::ReadOnly)) {
         while (!file.atEnd())
            items << file.readLine().trimmed();
        file.close();
    }

    QSettings settings;
    QString customUserAgent = settings.value("CustomUserAgent").toString();
    if (!items.contains(customUserAgent) && !customUserAgent.isEmpty())
        items << customUserAgent;

    QDialog* dialog = new QDialog(this);
    dialog->resize(size().width() * 0.7, dialog->size().height());
    dialog->setWindowTitle("Change User Agent");

    QVBoxLayout* layout = new QVBoxLayout(dialog);
    dialog->setLayout(layout);

#ifndef QT_NO_COMBOBOX
    QComboBox* combo = new QComboBox(dialog);
    combo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    combo->setEditable(true);
    combo->insertItems(0, items);
    layout->addWidget(combo);

    int index = combo->findText(page()->userAgentForUrl(QUrl()));
    combo->setCurrentIndex(index);
#endif

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
            | QDialogButtonBox::Cancel, Qt::Horizontal, dialog);
    connect(buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));
    layout->addWidget(buttonBox);

#ifndef QT_NO_COMBOBOX
    if (dialog->exec() && !combo->currentText().isEmpty()) {
        page()->setUserAgent(combo->currentText());
        if (!items.contains(combo->currentText()))
            settings.setValue("CustomUserAgent", combo->currentText());
    }
#endif

    delete dialog;
}

void LauncherWindow::loadURLListFromFile()
{
    QString selectedFile;
#ifndef QT_NO_FILEDIALOG
    selectedFile = QFileDialog::getOpenFileName(this, tr("Load URL list from file")
                                                       , QString(), tr("Text Files (*.txt);;All Files (*)"));
#endif
    if (selectedFile.isEmpty())
       return;

    m_urlLoader = new UrlLoader(this->page()->mainFrame(), selectedFile, 0, 0);
    m_urlLoader->loadNext();
}

void LauncherWindow::printURL(const QUrl& url)
{
    QTextStream output(stdout);
    output << "Loaded: " << url.toString() << endl;
}

#if !defined(QT_NO_FILEDIALOG) && !defined(QT_NO_MESSAGEBOX)
void LauncherWindow::downloadRequest(const QNetworkRequest &request)
{
    QNetworkAccessManager* manager = new QNetworkAccessManager(this);
    m_reply = manager->get(request);
    connect(m_reply, SIGNAL(finished()), this, SLOT(fileDownloadFinished()));
}

void LauncherWindow::fileDownloadFinished()
{
    QFileInfo fileInf(m_reply->request().url().toString());
    QString requestFileName = QDir::homePath() + "/" + fileInf.fileName();
    QString fileName = QFileDialog::getSaveFileName(this, "Save as...", requestFileName, "All Files (*)");

    if (fileName.isEmpty())
        return;
    if (m_reply->error() != QNetworkReply::NoError)
        QMessageBox::critical(this, QString("Download"), QString("Download failed."));
    else {
        QFile file(fileName);
        file.open(QIODevice::WriteOnly);
        file.write(m_reply->readAll());
        file.close();
        QMessageBox::information(this, QString("Download"), fileName + QString(" downloaded successfully."));
    }
}
#endif

void LauncherWindow::updateFPS(int fps)
{
    QString fpsStatusText = QString("Current FPS: %1").arg(fps);

#if defined(Q_WS_MAEMO_5) && defined(Q_OS_SYMBIAN)
    setWindowTitle(fpsStatusText);
#else
    statusBar()->showMessage(fpsStatusText);
#endif
}

void LauncherWindow::toggleLocalStorage(bool toggle)
{
    m_windowOptions.useLocalStorage = toggle;
    page()->settings()->setAttribute(QWebSettings::LocalStorageEnabled, toggle);
}

void LauncherWindow::toggleOfflineStorageDatabase(bool toggle)
{
    m_windowOptions.useOfflineStorageDatabase = toggle;
    page()->settings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, toggle);
}

void LauncherWindow::toggleOfflineWebApplicationCache(bool toggle)
{
    m_windowOptions.useOfflineWebApplicationCache = toggle;
    page()->settings()->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, toggle);
}

void LauncherWindow::setOfflineStorageDefaultQuota()
{
    // For command line execution, quota size is taken from command line.   
    if (m_windowOptions.offlineStorageDefaultQuotaSize)
        page()->settings()->setOfflineStorageDefaultQuota(m_windowOptions.offlineStorageDefaultQuotaSize);
    else {
#ifndef QT_NO_INPUTDIALOG
        bool ok;
        // Maximum size is set to 25 * 1024 * 1024.
        int quotaSize = QInputDialog::getInt(this, "Offline Storage Default Quota Size" , "Quota Size", 0, 0, 26214400, 1, &ok);
        if (ok) 
            page()->settings()->setOfflineStorageDefaultQuota(quotaSize);
#endif
    }
}

LauncherWindow* LauncherWindow::newWindow()
{
    LauncherWindow* mw = new LauncherWindow(&m_windowOptions);
    mw->show();
    return mw;
}

LauncherWindow* LauncherWindow::cloneWindow()
{
    LauncherWindow* mw = new LauncherWindow(&m_windowOptions, qobject_cast<QGraphicsView*>(m_view)->scene());
    mw->show();
    return mw;
}

#ifndef QT_NO_LINEEDIT
void LauncherWindow::showFindBar()
{
    if (!m_findBar->isVisible()) {
        m_findBar->setVisible(true);
        m_lineEdit->setText(page()->selectedText());
        m_lineEdit->setFocus(Qt::PopupFocusReason);
    } else {
        m_findBar->setVisible(false);
        page()->findText("", QWebPage::HighlightAllOccurrences);
    }
}

void LauncherWindow::find(int mode = s_findNormalFlag)
{
    QPalette palette;
    bool found;
    palette.setColor(m_lineEdit->backgroundRole(), Qt::white);
    page()->findText("", QFlag(QWebPage::HighlightAllOccurrences));

    m_findFlag = m_findFlag ^ mode;
    if (mode == s_findNormalFlag || mode == QWebPage::FindBackward) {
        found = page()->findText(m_lineEdit->text(), QFlag(m_findFlag & ~QWebPage::HighlightAllOccurrences));
        m_findFlag = m_findFlag ^ mode;

        if (found || m_lineEdit->text().isEmpty())
            m_lineEdit->setPalette(palette);
        else {
            palette.setColor(m_lineEdit->backgroundRole(), QColor(255, 0, 0, 127));
            m_lineEdit->setPalette(palette);
        }
    }

    if (m_findFlag & QWebPage::HighlightAllOccurrences)
        page()->findText(m_lineEdit->text(), QFlag(m_findFlag));
}
#endif
