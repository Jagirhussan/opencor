/*******************************************************************************

Copyright (C) The University of Auckland

OpenCOR is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

OpenCOR is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*******************************************************************************/

//==============================================================================
// Graph panel plot widget
//==============================================================================

#pragma once

//==============================================================================

#include "commonwidget.h"
#include "graphpanelwidgetglobal.h"

//==============================================================================

#include "qwtbegin.h"
    #include "qwt_legend.h"
    #include "qwt_plot.h"
    #include "qwt_plot_curve.h"
    #include "qwt_scale_draw.h"
    #include "qwt_scale_widget.h"
    #include "qwt_symbol.h"
    #include "qwt_text.h"
#include "qwtend.h"

//==============================================================================

class QMenu;

//==============================================================================

class QwtPlotDirectPainter;
class QwtPlotGrid;

//==============================================================================

namespace OpenCOR {
namespace GraphPanelWidget {

//==============================================================================

static const double DefaultMinAxis    =    0.0;
static const double DefaultMinLogAxis =    1.0;
static const double DefaultMaxAxis    = 1000.0;

//==============================================================================

class GRAPHPANELWIDGET_EXPORT GraphPanelPlotGraphProperties
{
public:
    explicit GraphPanelPlotGraphProperties(const QString &pTitle = QString(),
                                           const Qt::PenStyle &pLineStyle = Qt::SolidLine,
                                           const int &pLineWidth = 1,
                                           const QColor &pLineColor = Qt::darkBlue,
                                           const QwtSymbol::Style &pSymbolStyle = QwtSymbol::NoSymbol,
                                           const int &pSymbolSize = 8,
                                           const QColor &pSymbolColor = Qt::darkBlue,
                                           const bool &pSymbolFilled = true,
                                           const QColor &pSymbolFillColor = Qt::white);

    QString title() const;

    Qt::PenStyle lineStyle() const;
    int lineWidth() const;
    QColor lineColor() const;

    QwtSymbol::Style symbolStyle() const;
    int symbolSize() const;
    QColor symbolColor() const;
    bool symbolFilled() const;
    QColor symbolFillColor() const;

private:
    QString mTitle;

    Qt::PenStyle mLineStyle;
    int mLineWidth;
    QColor mLineColor;

    QwtSymbol::Style mSymbolStyle;
    int mSymbolSize;
    QColor mSymbolColor;
    bool mSymbolFilled;
    QColor mSymbolFillColor;
};

//==============================================================================

class GRAPHPANELWIDGET_EXPORT GraphPanelPlotGraph : public QwtPlotCurve
{
public:
    explicit GraphPanelPlotGraph(void *pParameterX = 0, void *pParameterY = 0);

    bool isValid() const;

    bool isSelected() const;
    void setSelected(const bool &pSelected);

    QString fileName() const;
    void setFileName(const QString &pFileName);

    void * parameterX() const;
    void setParameterX(void *pParameterX);

    void * parameterY() const;
    void setParameterY(void *pParameterY);

    void setData(double *pDataX, double *pDataY, const int &pSize);

    QRectF boundingLogRect();

private:
    bool mSelected;

    QString mFileName;

    void *mParameterX;
    void *mParameterY;

    QRectF mBoundingLogRect;
};

//==============================================================================

typedef QList<GraphPanelPlotGraph *> GraphPanelPlotGraphs;

//==============================================================================

class GraphPanelPlotWidget;

//==============================================================================

class GraphPanelPlotOverlayWidget : public QWidget
{
public:
    explicit GraphPanelPlotOverlayWidget(GraphPanelPlotWidget *pParent);

    void setOriginPoint(const QPoint &pOriginPoint);
    void setPoint(const QPoint &pPoint);

    QRect zoomRegion() const;

protected:
    virtual void paintEvent(QPaintEvent *pEvent);

private:
    enum Position {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };

    GraphPanelPlotWidget *mOwner;

    QPoint mOriginPoint;
    QPoint mPoint;

    QPoint optimisedPoint(const QPoint &pPoint) const;

    void drawCoordinates(QPainter *pPainter, const QPoint &pPoint,
                         const QColor &pBackgroundColor,
                         const QColor &pForegroundColor, const int &pLineWidth,
                         const Position &pPosition,
                         const bool &pCanMovePosition = true);
};

//==============================================================================

class GraphPanelPlotScaleDraw : public QwtScaleDraw
{
public:
    void retranslateUi();

protected:
    virtual QwtText label(double pValue) const;
};

//==============================================================================

class GraphPanelPlotScaleWidget : public QwtScaleWidget
{
public:
    void updateLayout();
};

//==============================================================================

class GraphPanelPlotLegendWidget : public QwtLegend
{
    Q_OBJECT

public:
    explicit GraphPanelPlotLegendWidget(GraphPanelPlotWidget *pParent);

    bool isActive() const;
    void setActive(const bool &pActive);

    virtual bool isEmpty() const;

    void setFontSize(const int &pFontSize);
    void setForegroundColor(const QColor &pForegroundColor);

    virtual QSize sizeHint() const;

protected:
    virtual void updateWidget(QWidget *pWidget,
                              const QwtLegendData &pLegendData);

private:
    GraphPanelPlotWidget *mOwner;

    bool mActive;

    int mFontSize;
    QColor mForegroundColor;
};

//==============================================================================

typedef QList<GraphPanelPlotWidget *> GraphPanelPlotWidgets;

//==============================================================================

class GRAPHPANELWIDGET_EXPORT GraphPanelPlotWidget : public QwtPlot,
                                                     public Core::CommonWidget
{
    Q_OBJECT

public:
    enum Action {
        None,
        Pan,
        ShowCoordinates,
        Zoom,
        ZoomRegion
    };

    explicit GraphPanelPlotWidget(const GraphPanelPlotWidgets &pNeighbors,
                                  QAction *pSynchronizeXAxisAction,
                                  QAction *pSynchronizeYAxisAction,
                                  QWidget *pParent);
    ~GraphPanelPlotWidget();

    virtual void retranslateUi();

    GraphPanelPlotGraphs graphs() const;

    bool addGraph(GraphPanelPlotGraph *pGraph);
    bool removeGraph(GraphPanelPlotGraph *pGraph);

    bool hasData() const;

    bool dataRect(QRectF &pDataRect) const;
    bool dataLogRect(QRectF &pDataLogRect) const;

    void optimiseAxis(double &pMin, double &pMax) const;

    double minX() const;
    double maxX() const;
    double minY() const;
    double maxY() const;

    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &pBackgroundColor);

    int fontSize() const;
    void setFontSize(const int &pFontSize, const bool &pForceSetting = false);

    QColor foregroundColor() const;
    void setForegroundColor(const QColor &pForegroundColor);

    Qt::PenStyle gridLinesStyle() const;
    void setGridLinesStyle(const Qt::PenStyle &pGridLinesStyle);

    int gridLinesWidth() const;
    void setGridLinesWidth(const int &pGridLinesWidth);

    QColor gridLinesColor() const;
    void setGridLinesColor(const QColor &pGridLinesColor);

    bool legend() const;
    void setLegend(const bool &pLegend);

    void setLegendWidth(const int &pLegendWidth);

    Qt::PenStyle pointCoordinatesStyle() const;
    void setPointCoordinatesStyle(const Qt::PenStyle &pPointCoordinatesStyle);

    int pointCoordinatesWidth() const;
    void setPointCoordinatesWidth(const int &pPointCoordinatesWidth);

    QColor pointCoordinatesColor() const;
    void setPointCoordinatesColor(const QColor &pPointCoordinatesColor);

    QColor pointCoordinatesFontColor() const;
    void setPointCoordinatesFontColor(const QColor &pPointCoordinatesFontColor);

    void setTitle(const QString &pTitle);

    bool logAxisX() const;
    void setLogAxisX(const bool &pLogAxisX);

    QString titleAxisX() const;
    void setTitleAxisX(const QString &pTitleAxisX);

    bool logAxisY() const;
    void setLogAxisY(const bool &pLogAxisY);

    QString titleAxisY() const;
    void setTitleAxisY(const QString &pTitleAxisY);

    Qt::PenStyle zoomRegionStyle() const;
    void setZoomRegionStyle(const Qt::PenStyle &pZoomRegionStyle);

    int zoomRegionWidth() const;
    void setZoomRegionWidth(const int &pZoomRegionWidth);

    QColor zoomRegionColor() const;
    void setZoomRegionColor(const QColor &pZoomRegionColor);

    QColor zoomRegionFontColor() const;
    void setZoomRegionFontColor(const QColor &pZoomRegionFontColor);

    bool zoomRegionFilled() const;
    void setZoomRegionFilled(const bool &pZoomRegionFilled);

    QColor zoomRegionFillColor() const;
    void setZoomRegionFillColor(const QColor &pZoomRegionFillColor);

    void setDefaultAxesValues(const double &pDefaultMinX,
                              const double &pDefaultMaxX,
                              const double &pDefaultMinLogX,
                              const double &pDefaultMaxLogX,
                              const double &pDefaultMinY,
                              const double &pDefaultMaxY,
                              const double &pDefaultMinLogY,
                              const double &pDefaultMaxLogY);

    bool setAxes(double pMinX, double pMaxX, double pMinY, double pMaxY,
                 const bool &pSynchronizeAxes = true,
                 const bool &pCanReplot = true, const bool &pEmitSignal = true,
                 const bool &pForceXAxisSetting = false,
                 const bool &pForceYAxisSetting = false);

    bool drawGraphFrom(GraphPanelPlotGraph *pGraph,
                       const qulonglong &pFrom);

    GraphPanelPlotWidgets neighbors() const;

    void addNeighbor(GraphPanelPlotWidget *pPlot);
    void removeNeighbor(GraphPanelPlotWidget *pPlot);

    void alignWithNeighbors(const bool &pCanReplot,
                            const bool &pForceAlignment = false);
    void forceAlignWithNeighbors();

    Action action() const;

    bool canZoomInX() const;
    bool canZoomOutX() const;
    bool canZoomInY() const;
    bool canZoomOutY() const;

    QPointF canvasPoint(const QPoint &pPoint) const;

protected:
    virtual bool eventFilter(QObject *pObject, QEvent *pEvent);
    virtual void mouseMoveEvent(QMouseEvent *pEvent);
    virtual void mousePressEvent(QMouseEvent *pEvent);
    virtual void mouseReleaseEvent(QMouseEvent *pEvent);
    virtual void paintEvent(QPaintEvent *pEvent);
    virtual void resizeEvent(QResizeEvent *pEvent);
    virtual void wheelEvent(QWheelEvent *pEvent);

private:
    enum Scaling {
        BigScalingIn,
        ScalingIn,
        NoScaling,
        ScalingOut,
        BigScalingOut
    };

    QwtPlotDirectPainter *mDirectPainter;

    QColor mBackgroundColor;
    QColor mForegroundColor;

    Qt::PenStyle mPointCoordinatesStyle;
    int mPointCoordinatesWidth;
    QColor mPointCoordinatesColor;
    QColor mPointCoordinatesFontColor;

    Qt::PenStyle mZoomRegionStyle;
    int mZoomRegionWidth;
    QColor mZoomRegionColor;
    QColor mZoomRegionFontColor;
    bool mZoomRegionFilled;
    QColor mZoomRegionFillColor;

    bool mLogAxisX;
    bool mLogAxisY;

    GraphPanelPlotGraphs mGraphs;

    Action mAction;

    QPoint mOriginPoint;
    QPoint mPoint;

    GraphPanelPlotOverlayWidget *mOverlayWidget;

    GraphPanelPlotLegendWidget *mLegend;

    bool mCanDirectPaint;
    bool mCanReplot;

    bool mCanZoomInX;
    bool mCanZoomOutX;
    bool mCanZoomInY;
    bool mCanZoomOutY;

    bool mNeedContextMenu;
    QMenu *mContextMenu;

    bool mCanUpdateActions;

    QAction *mExportToAction;
    QAction *mCopyToClipboardAction;
    QAction *mSynchronizeXAxisAction;
    QAction *mSynchronizeYAxisAction;
    QAction *mGraphPanelSettingsAction;
    QAction *mGraphsSettingsAction;
    QAction *mCustomAxesAction;
    QAction *mZoomInAction;
    QAction *mZoomOutAction;
    QAction *mResetZoomAction;

    GraphPanelPlotScaleDraw *mAxisX;
    GraphPanelPlotScaleDraw *mAxisY;

    QwtPlotGrid *mGrid;

    double mDefaultMinX;
    double mDefaultMaxX;
    double mDefaultMinY;
    double mDefaultMaxY;

    double mDefaultMinLogX;
    double mDefaultMaxLogX;
    double mDefaultMinLogY;
    double mDefaultMaxLogY;

    GraphPanelPlotWidgets mNeighbors;

    void checkAxisValues(const bool &pLogAxis, double &pMin, double &pMax);

    void updateActions();

    void resetAction();

    QRectF realDataRect() const;

    void setAxis(const int &pAxisId, double pMin, double pMax);

    bool resetAxes();

    bool scaleAxis(const Scaling &pScaling, const bool &pCanZoomIn,
                   const bool &pCanZoomOut, const QwtScaleMap &pCanvasMap,
                   const double &pPoint, double &pMin, double &pMax);
    void scaleAxes(const QPoint &pPoint, const Scaling &pScalingX,
                   const Scaling &pScalingY);

    void setTitleAxis(const int &pAxisId, const QString &pTitleAxis);

signals:
    void axesChanged(const double &pMinX, const double &pMaxX,
                     const double &pMinY, const double &pMaxY);

    void graphPanelSettingsRequested();
    void graphsSettingsRequested();

private slots:
    void cannotUpdateActions();

    void exportTo();
    void copyToClipboard();
    void customAxes();
    void zoomIn();
    void zoomOut();
    void resetZoom();
};

//==============================================================================

}   // namespace GraphPanelWidget
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
