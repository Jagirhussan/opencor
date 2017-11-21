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

#include "coreguiutils.h"
#include "graphpanelplotwidget.h"
#include "graphpanelwidgetcustomaxesdialog.h"
#include "i18ninterface.h"

//==============================================================================

#include <QApplication>
#include <QClipboard>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QImageWriter>
#include <QMenu>
#include <QPaintEvent>

//==============================================================================

#include <float.h>

//==============================================================================

#include "qwtbegin.h"
    #include "qwt_legend_label.h"
    #include "qwt_painter.h"
    #include "qwt_plot_canvas.h"
    #include "qwt_plot_directpainter.h"
    #include "qwt_plot_grid.h"
    #include "qwt_plot_layout.h"
    #include "qwt_plot_renderer.h"
    #include "qwt_scale_engine.h"
    #include "qwt_text_label.h"
#include "qwtend.h"

//==============================================================================

namespace OpenCOR {
namespace GraphPanelWidget {

//==============================================================================

GraphPanelPlotGraphProperties::GraphPanelPlotGraphProperties(const QString &pTitle,
                                                             const Qt::PenStyle &pLineStyle,
                                                             const int &pLineWidth,
                                                             const QColor &pLineColor,
                                                             const QwtSymbol::Style &pSymbolStyle,
                                                             const int &pSymbolSize,
                                                             const QColor &pSymbolColor,
                                                             const bool &pSymbolFilled,
                                                             const QColor &pSymbolFillColor) :
    mTitle(pTitle),
    mLineStyle(pLineStyle),
    mLineWidth(pLineWidth),
    mLineColor(pLineColor),
    mSymbolStyle(pSymbolStyle),
    mSymbolSize(pSymbolSize),
    mSymbolColor(pSymbolColor),
    mSymbolFilled(pSymbolFilled),
    mSymbolFillColor(pSymbolFillColor)
{
}

//==============================================================================

QString GraphPanelPlotGraphProperties::title() const
{
    // Return our title

    return mTitle;
}

//==============================================================================

Qt::PenStyle GraphPanelPlotGraphProperties::lineStyle() const
{
    // Return our line style

    return mLineStyle;
}

//==============================================================================

int GraphPanelPlotGraphProperties::lineWidth() const
{
    // Return our line width

    return mLineWidth;
}

//==============================================================================

QColor GraphPanelPlotGraphProperties::lineColor() const
{
    // Return our line colour

    return mLineColor;
}

//==============================================================================

QwtSymbol::Style GraphPanelPlotGraphProperties::symbolStyle() const
{
    // Return our symbol style

    return mSymbolStyle;
}

//==============================================================================

int GraphPanelPlotGraphProperties::symbolSize() const
{
    // Return our symbol size

    return mSymbolSize;
}

//==============================================================================

QColor GraphPanelPlotGraphProperties::symbolColor() const
{
    // Return our symbol colour

    return mSymbolColor;
}

//==============================================================================

bool GraphPanelPlotGraphProperties::symbolFilled() const
{
    // Return our symbol filled status

    return mSymbolFilled;
}

//==============================================================================

QColor GraphPanelPlotGraphProperties::symbolFillColor() const
{
    // Return our symbol fill colour

    return mSymbolFillColor;
}

//==============================================================================

static const QRectF NoBoundingLogRect = QRectF(0.0, 0.0, -1.0, -1.0);

//==============================================================================

GraphPanelPlotGraph::GraphPanelPlotGraph(void *pParameterX, void *pParameterY) :
    QwtPlotCurve(),
    mSelected(true),
    mFileName(QString()),
    mParameterX(pParameterX),
    mParameterY(pParameterY),
    mBoundingLogRect(NoBoundingLogRect)
{
    // Customise ourselves a bit

    setLegendAttribute(LegendShowLine);
    setLegendAttribute(LegendShowSymbol);
    setPen(QPen(Qt::darkBlue, 1.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    setRenderHint(RenderAntialiased);
}

//==============================================================================

bool GraphPanelPlotGraph::isValid() const
{
    // Return whether we are valid

    return   !mFileName.isEmpty()
           && mParameterX && mParameterY;
}

//==============================================================================

bool GraphPanelPlotGraph::isSelected() const
{
    // Return whether we are selected

    return mSelected;
}

//==============================================================================

void GraphPanelPlotGraph::setSelected(const bool &pSelected)
{
    // Set our selected state

    mSelected = pSelected;
}

//==============================================================================

QString GraphPanelPlotGraph::fileName() const
{
    // Return our file name

    return mFileName;
}

//==============================================================================

void GraphPanelPlotGraph::setFileName(const QString &pFileName)
{
    // Set our file name

    mFileName = pFileName;
}

//==============================================================================

void * GraphPanelPlotGraph::parameterX() const
{
    // Return our parameter X

    return mParameterX;
}

//==============================================================================

void GraphPanelPlotGraph::setParameterX(void *pParameterX)
{
    // Set our parameter X

    mParameterX = pParameterX;
}

//==============================================================================

void * GraphPanelPlotGraph::parameterY() const
{
    // Return our parameter Y

    return mParameterY;
}

//==============================================================================

void GraphPanelPlotGraph::setParameterY(void *pParameterY)
{
    // Set our parameter Y

    mParameterY = pParameterY;
}

//==============================================================================

void GraphPanelPlotGraph::setData(double *pDataX, double *pDataY,
                                  const int &pSize)
{
    // Set our data, i.e. raw samples

    setRawSamples(pDataX, pDataY, pSize);

    // Reset our cache version of our log data rectangle

    mBoundingLogRect = NoBoundingLogRect;
}

//==============================================================================

QRectF GraphPanelPlotGraph::boundingLogRect()
{
    // Return the cached version of our bounding log rectangle, if we have one,
    // or compute it and return it

    if (mBoundingLogRect == NoBoundingLogRect) {
        const QwtSeriesData<QPointF> *crtData = data();
        bool needInitMinX = true;
        bool needInitMaxX = true;
        bool needInitMinY = true;
        bool needInitMaxY = true;
        double minX = 1.0;
        double maxX = 1.0;
        double minY = 1.0;
        double maxY = 1.0;

        for (size_t i = 0, iMax = crtData->size(); i < iMax; ++i) {
            QPointF sample = crtData->sample(i);

            if (sample.x() > 0.0) {
                if (needInitMinX) {
                    minX = sample.x();

                    needInitMinX = false;
                } else if (sample.x() < minX) {
                    minX = sample.x();
                }

                if (needInitMaxX) {
                    maxX = sample.x();

                    needInitMaxX = false;
                } else if (sample.x() > maxX) {
                    maxX = sample.x();
                }
            }

            if (sample.y() > 0.0) {
                if (needInitMinY) {
                    minY = sample.y();

                    needInitMinY = false;
                } else if (sample.y() < minY) {
                    minY = sample.y();
                }

                if (needInitMaxY) {
                    maxY = sample.y();

                    needInitMaxY = false;
                } else if (sample.y() > maxY) {
                    maxY = sample.y();
                }
            }
        }

        if (!needInitMinX && !needInitMaxX && !needInitMinY && !needInitMaxY)
            mBoundingLogRect = QRectF(minX, minY, maxX-minX, maxY-minY);
    }

    return mBoundingLogRect;
}

//==============================================================================

GraphPanelPlotOverlayWidget::GraphPanelPlotOverlayWidget(GraphPanelPlotWidget *pParent) :
    QWidget(pParent),
    mOwner(pParent),
    mOriginPoint(QPoint()),
    mPoint(QPoint())
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents);

    setFocusPolicy(Qt::NoFocus);
}

//==============================================================================

QPoint GraphPanelPlotOverlayWidget::optimisedPoint(const QPoint &pPoint) const
{
    // Optimise the given point so that it fits within our owner's ranges

    QPoint realPoint = pPoint-mOwner->plotLayout()->canvasRect().topLeft().toPoint();

    QwtScaleMap canvasMapX = mOwner->canvasMap(QwtPlot::xBottom);
    QwtScaleMap canvasMapY = mOwner->canvasMap(QwtPlot::yLeft);

    return QPoint(qMin(qRound(canvasMapX.transform(mOwner->maxX())),
                       qMax(qRound(canvasMapX.transform(mOwner->minX())),
                            realPoint.x())),
                  qMin(qRound(canvasMapY.transform(mOwner->minY())),
                       qMax(qRound(canvasMapY.transform(mOwner->maxY())),
                            realPoint.y())));
}

//==============================================================================

void GraphPanelPlotOverlayWidget::paintEvent(QPaintEvent *pEvent)
{
    // Accept the event

    pEvent->accept();

    // Check whether an action is to be carried out

    if (mOwner->action() == GraphPanelPlotWidget::None)
        return;

    // Paint the overlay, if any is needed

    QPainter painter(this);

    QRectF canvasRect = mOwner->plotLayout()->canvasRect();

    painter.translate(canvasRect.x(), canvasRect.y());

    switch (mOwner->action()) {
    case GraphPanelPlotWidget::ShowCoordinates: {
        // Draw the lines that show the coordinates

        QPen pen = painter.pen();

        pen.setColor(mOwner->pointCoordinatesColor());
        pen.setStyle(mOwner->pointCoordinatesStyle());
        pen.setWidth(mOwner->pointCoordinatesWidth());

        painter.setPen(pen);

        QPoint point = optimisedPoint(mPoint);
        // Note: see drawCoordinates() as why we use QPoint rather than
        //       QPointF...

        painter.drawLine(0, point.y(), canvasRect.width(), point.y());
        painter.drawLine(point.x(), 0, point.x(), canvasRect.height());

        // Draw the coordinates of our point

        drawCoordinates(&painter, point, mOwner->pointCoordinatesColor(),
                        mOwner->pointCoordinatesFontColor(),
                        (mOwner->pointCoordinatesStyle() == Qt::NoPen)?
                            0:
                            mOwner->pointCoordinatesWidth(),
                        BottomRight);

        break;
    }
    case GraphPanelPlotWidget::ZoomRegion: {
        // Draw the region to be zoomed in

        QRect zoomRegionRect = zoomRegion();
        // Note: see drawCoordinates() as why we use QRect rather than QRectF...

        if (mOwner->zoomRegionFilled())
            painter.fillRect(zoomRegionRect, mOwner->zoomRegionFillColor());

        QPen pen = painter.pen();

        pen.setJoinStyle(Qt::RoundJoin);
        pen.setColor(mOwner->zoomRegionColor());
        pen.setStyle(mOwner->zoomRegionStyle());
        pen.setWidth(mOwner->zoomRegionWidth());

        painter.setPen(pen);

        painter.drawRect(zoomRegionRect);

        // Draw the two sets of coordinates

        drawCoordinates(&painter, zoomRegionRect.topLeft(),
                        mOwner->zoomRegionColor(),
                        mOwner->zoomRegionFontColor(),
                        (mOwner->zoomRegionStyle() == Qt::NoPen)?
                            0:
                            mOwner->zoomRegionWidth(),
                        BottomRight, false);
        drawCoordinates(&painter, zoomRegionRect.topLeft()+QPoint(zoomRegionRect.width(), zoomRegionRect.height()),
                        mOwner->zoomRegionColor(),
                        mOwner->zoomRegionFontColor(),
                        (mOwner->zoomRegionStyle() == Qt::NoPen)?
                            0:
                            mOwner->zoomRegionWidth(),
                        TopLeft, false);

        break;
    }
    default:
        // Either no action or not an action we know how to handle

        ;
    }
}

//==============================================================================

void GraphPanelPlotOverlayWidget::setOriginPoint(const QPoint &pOriginPoint)
{
    // Set our point of origin

    mOriginPoint = pOriginPoint;
    // Note: unlike for setPoint(), we must not repaint ourselves since a call
    //       to setOriginPoint() should always be followed by one to setPoint(),
    //       in which case we don't want to repaint ourselves twice, not least
    //       because the first time we may not have a correct mPoint value...
}

//==============================================================================

void GraphPanelPlotOverlayWidget::setPoint(const QPoint &pPoint)
{
    // Set our point

    mPoint = pPoint;

    update();
}

//==============================================================================

QRect GraphPanelPlotOverlayWidget::zoomRegion() const
{
    // Return the region to be zoomed based on mOriginPoint and mPoint
    // Note: by default, we assume that we are already fully zoomed in in both
    //       directions...

    QwtScaleMap canvasMapX = mOwner->canvasMap(QwtPlot::xBottom);
    QwtScaleMap canvasMapY = mOwner->canvasMap(QwtPlot::yLeft);

    int minX = canvasMapX.transform(mOwner->minX());
    int maxX = canvasMapX.transform(mOwner->maxX());
    int minY = canvasMapY.transform(mOwner->maxY());
    int maxY = canvasMapY.transform(mOwner->minY());

    if (mOwner->canZoomInX() || mOwner->canZoomInY()) {
        QPoint originPoint = optimisedPoint(mOriginPoint);
        QPoint point = optimisedPoint(mPoint);

        if (mOwner->canZoomInX()) {
            minX = qMin(originPoint.x(), point.x());
            maxX = qMax(originPoint.x(), point.x());
        }

        if (mOwner->canZoomInY()) {
            minY = qMin(originPoint.y(), point.y());
            maxY = qMax(originPoint.y(), point.y());
        }
    }

    return QRect(minX, minY, maxX-minX, maxY-minY);
}

//==============================================================================

void GraphPanelPlotOverlayWidget::drawCoordinates(QPainter *pPainter,
                                                  const QPoint &pPoint,
                                                  const QColor &pBackgroundColor,
                                                  const QColor &pForegroundColor,
                                                  const int &pLineWidth,
                                                  const Position &pPosition,
                                                  const bool &pCanMovePosition)
{
    // Retrieve the size of coordinates as they will appear on the screen,
    // which means using the same font as the one used for the axes
    // Note #1: normally, pPoint would be a QPointF, but we want the coordinates
    //          to be drawn relative to something (see paintEvent()) and the
    //          only way to guarantee that everything will be painted as
    //          expected is to use QPoint. Indeed, if we were to use QPointF,
    //          then QPainter would have to do some rounding and though
    //          everything should be fine (since we always add/subtract a
    //          rounded number), it happens that it's not always the case.
    //          Indeed, we should always have a gap of one pixel between the
    //          coordinates and pPoint, but it could happen that we have either
    //          no gap, or one or two pixels...
    // Note #2: we set the painter's pen's style to a solid line otherwise
    //          coordinatesRect will be empty if there is no style for showing
    //          the coordinates of a point or zooming in a region...

    pPainter->setFont(mOwner->axisFont(QwtPlot::xBottom));

    QPointF point = mOwner->canvasPoint(pPoint);
    QString coordinates = QString("X: %1\nY: %2").arg(QLocale().toString(point.x(), 'g', 15),
                                                      QLocale().toString(point.y(), 'g', 15));
    QPen pen = pPainter->pen();

    pen.setStyle(Qt::SolidLine);

    pPainter->setPen(pen);

    QRect coordinatesRect = pPainter->boundingRect(qApp->desktop()->availableGeometry(), 0, coordinates);

    // Determine where the coordinates and its background should be drawn

    int shift = (pLineWidth >> 1)+pLineWidth%2;
    int plusShift = shift+1;
    int minusShift = shift+!(pLineWidth%2);

    switch (pPosition) {
    case TopLeft:
        coordinatesRect.moveTo(pPoint.x()-coordinatesRect.width()-minusShift,
                               pPoint.y()-coordinatesRect.height()-minusShift);

        break;
    case TopRight:
        coordinatesRect.moveTo(pPoint.x()+plusShift,
                               pPoint.y()-coordinatesRect.height()-minusShift);

        break;
    case BottomLeft:
        coordinatesRect.moveTo(pPoint.x()-coordinatesRect.width()-minusShift,
                               pPoint.y()+plusShift);

        break;
    case BottomRight:
        coordinatesRect.moveTo(pPoint.x()+plusShift,
                               pPoint.y()+plusShift);

        break;
    }

    if (pCanMovePosition) {
        QwtScaleMap canvasMapX = mOwner->canvasMap(QwtPlot::xBottom);
        QwtScaleMap canvasMapY = mOwner->canvasMap(QwtPlot::yLeft);

        QPoint topLeftPoint = QPoint(canvasMapX.transform(mOwner->minX()),
                                     canvasMapY.transform(mOwner->maxY()));
        QPoint bottomRightPoint = QPoint(canvasMapX.transform(mOwner->maxX()),
                                         canvasMapY.transform(mOwner->minY()));

        if (coordinatesRect.top() < topLeftPoint.y())
            coordinatesRect.moveTop(pPoint.y()+plusShift);
        else if (coordinatesRect.top()+coordinatesRect.height()-1 > bottomRightPoint.y())
            coordinatesRect.moveTop(pPoint.y()-coordinatesRect.height()-minusShift);

        if (coordinatesRect.left() < topLeftPoint.x())
            coordinatesRect.moveLeft(pPoint.x()+plusShift);
        else if (coordinatesRect.left()+coordinatesRect.width()-1 > bottomRightPoint.x())
            coordinatesRect.moveLeft(pPoint.x()-coordinatesRect.width()-minusShift);

        // Note: the -1 for the else-if tests is because fillRect() below works
        //       on (0, 0; width-1, height-1)...
    }

    // Draw a filled rectangle to act as the background for the coordinates
    // we are to show

    pPainter->fillRect(coordinatesRect, pBackgroundColor);

    // Draw the text for the coordinates, using a white pen

    pen.setColor(pForegroundColor);

    pPainter->setPen(pen);

    pPainter->drawText(coordinatesRect, coordinates);
}

//==============================================================================

void GraphPanelPlotScaleDraw::retranslateUi()
{
    // Retranslate ourselves by invalidating our cache

    invalidateCache();
}

//==============================================================================

QwtText GraphPanelPlotScaleDraw::label(double pValue) const
{
    // Return pValue as a string, keeping in mind the current locale

    QString label = QLocale().toString(pValue);
    QString fullLabel = QLocale().toString(pValue, 'g', 15);

    return fullLabel.startsWith(label)?fullLabel:label;
}

//==============================================================================

void GraphPanelPlotScaleWidget::updateLayout()
{
    // Our layout has changed, so update our internals

    layoutScale();
}

//==============================================================================

GraphPanelPlotLegendWidget::GraphPanelPlotLegendWidget(GraphPanelPlotWidget *pParent) :
    QwtLegend(pParent),
    mOwner(pParent),
    mFontSize(pParent->fontSize()),
    mForegroundColor(pParent->foregroundColor())
{
}

//==============================================================================

void GraphPanelPlotLegendWidget::setFontSize(const int &pFontSize)
{
    // Set our font size

    if (pFontSize != mFontSize) {
        mFontSize = pFontSize;

        mOwner->updateLegend();
    }
}

//==============================================================================

void GraphPanelPlotLegendWidget::setForegroundColor(const QColor &pForegroundColor)
{
    // Set our foreground color

    if (pForegroundColor != mForegroundColor) {
        mForegroundColor = pForegroundColor;

        mOwner->updateLegend();
    }
}

//==============================================================================

void GraphPanelPlotLegendWidget::updateWidget(QWidget *pWidget,
                                              const QwtLegendData &pLegendData)
{
    // Default handling

    QwtLegend::updateWidget(pWidget, pLegendData);

    // Update our font size and foreground colour

    QwtLegendLabel *legendLabel = dynamic_cast<QwtLegendLabel *>(pWidget);
    QFont newFont = legendLabel->font();

    newFont.setPointSize(mFontSize);

    legendLabel->setFont(newFont);

    QPalette newPalette = legendLabel->palette();

    newPalette.setColor(QPalette::Text, mForegroundColor);

    legendLabel->setPalette(newPalette);

    // Make sure that updates are enabled
    // Note: indeed, when setting its data, QwtLegendLabel (which used by
    //       QwtLegend) disables itself from updating, and then reenables itself
    //       if it was originally enabled. Now, the problem is that if one of
    //       our ancestors decides to temporarily disable updates (e.g. our
    //       simulation experiment view) then QwtLegendLabel won't reenable
    //       itself, which means that the legend may not actually be visible in
    //       some cases (e.g. when opening a SED-ML file)...

    pWidget->setUpdatesEnabled(true);
}

//==============================================================================

static const double DblMinAxis = 1000.0*DBL_MIN;
// Note: normally, we would use DBL_MIN, but this would cause problems with
//       QwtPlot (e.g. to create ticks), so instead we use a value that results
//       in a range that we know works...
static const double DblMaxAxis = 0.3*DBL_MAX;
// Note: normally, we would use DBL_MAX, but this means that our maximum axis
//       range would be 2*DBL_MAX, which would cause problems with QwtPlot (e.g.
//       to create ticks), so instead we use a value that results in a range
//       that we know works...

static const double MinAxis    = -DblMaxAxis;
static const double MinLogAxis =  DblMinAxis ;
static const double MaxAxis    =  DblMaxAxis;

static const double MaxAxisRange    = MaxAxis-MinAxis;
static const double MaxLogAxisRange = MaxAxis-MinLogAxis;
static const double MinAxisRange    = DblMinAxis;

//==============================================================================

GraphPanelPlotWidget::GraphPanelPlotWidget(const GraphPanelPlotWidgets &pNeighbors,
                                           QAction *pSynchronizeXAxisAction,
                                           QAction *pSynchronizeYAxisAction,
                                           QWidget *pParent) :
    QwtPlot(pParent),
    Core::CommonWidget(this),
    mBackgroundColor(QColor()),
    mForegroundColor(QColor()),
    mPointCoordinatesStyle(Qt::DashLine),
    mPointCoordinatesWidth(1),
    mPointCoordinatesColor(QColor()),
    mPointCoordinatesFontColor(Qt::white),
    mZoomRegionStyle(Qt::SolidLine),
    mZoomRegionWidth(1),
    mZoomRegionColor(QColor()),
    mZoomRegionFontColor(Qt::white),
    mZoomRegionFilled(true),
    mZoomRegionFillColor(QColor()),
    mLogAxisX(false),
    mLogAxisY(false),
    mGraphs(GraphPanelPlotGraphs()),
    mAction(None),
    mOriginPoint(QPoint()),
    mPoint(QPoint()),
    mCanDirectPaint(true),
    mCanReplot(true),
    mCanZoomInX(true),
    mCanZoomOutX(true),
    mCanZoomInY(true),
    mCanZoomOutY(true),
    mNeedContextMenu(false),
    mCanUpdateActions(true),
    mSynchronizeXAxisAction(pSynchronizeXAxisAction),
    mSynchronizeYAxisAction(pSynchronizeYAxisAction),
    mDefaultMinX(DefaultMinAxis),
    mDefaultMaxX(DefaultMaxAxis),
    mDefaultMinY(DefaultMinAxis),
    mDefaultMaxY(DefaultMaxAxis),
    mDefaultMinLogX(DefaultMinLogAxis),
    mDefaultMaxLogX(DefaultMaxAxis),
    mDefaultMinLogY(DefaultMinLogAxis),
    mDefaultMaxLogY(DefaultMaxAxis),
    mNeighbors(pNeighbors)
{
    // Keep track of when our grand parent (i.e. a GraphPanelsWidget object)
    // gets destroyed
    // Note: indeed, when that happens, all of its children (i.e.
    //       GraphPanelWidget objects) will tell their plot's neighbours (i.e.
    //       objects like this one) that they are not going to be their
    //       neighbour anymore (see removeNeighbor()). This will, in turn,
    //       involve updating our actions (see updateActions()), something that
    //       we cannot do when our grand parent gets destroyed...

    connect(pParent->parent(), SIGNAL(destroyed(QObject *)),
            this, SLOT(cannotUpdateActions()));

    // Get ourselves a direct painter

    mDirectPainter = new QwtPlotDirectPainter(this);

    mDirectPainter->setAttribute(QwtPlotDirectPainter::CopyBackingStore, true);

    // Speedup painting on X11 systems
    // Note: this can only be done on X11 systems...

    if (QwtPainter::isX11GraphicsSystem())
        canvas()->setAttribute(Qt::WA_PaintOnScreen, true);

    // We don't want a frame around ourselves

    qobject_cast<QwtPlotCanvas *>(canvas())->setFrameShape(QFrame::NoFrame);

    // Customise ourselves a bit

    setAutoFillBackground(true);

    setBackgroundColor(Qt::white);
    setFontSize(10, true);
    setForegroundColor(Qt::black);

    QColor pointCoordinatesColor = Qt::darkCyan;

    pointCoordinatesColor.setAlphaF(0.69);

    setPointCoordinatesColor(pointCoordinatesColor);

    QColor zoomRegionColor = Qt::darkRed;
    QColor zoomRegionFillColor = Qt::yellow;

    zoomRegionColor.setAlphaF(0.69);
    zoomRegionFillColor.setAlphaF(0.19);

    setZoomRegionColor(zoomRegionColor);
    setZoomRegionFillColor(zoomRegionFillColor);

    // Add some axes to ourselves

    mAxisX = new GraphPanelPlotScaleDraw();
    mAxisY = new GraphPanelPlotScaleDraw();

    setAxisScaleDraw(QwtPlot::xBottom, mAxisX);
    setAxisScaleDraw(QwtPlot::yLeft, mAxisY);

    // Attach a grid to ourselves

    mGrid = new QwtPlotGrid();

    mGrid->setMajorPen(Qt::gray, 1, Qt::DotLine);

    mGrid->attach(this);

    // Create our overlay widget

    mOverlayWidget = new GraphPanelPlotOverlayWidget(this);

    // Create our context menu

    mContextMenu = new QMenu(this);

    mExportToAction = Core::newAction(this);
    mCopyToClipboardAction = Core::newAction(this);
    mCustomAxesAction = Core::newAction(this);
    mGraphPanelSettingsAction = Core::newAction(this);
    mGraphsSettingsAction = Core::newAction(this);
    mZoomInAction = Core::newAction(this);
    mZoomOutAction = Core::newAction(this);
    mResetZoomAction = Core::newAction(this);

    connect(mExportToAction, SIGNAL(triggered(bool)),
            this, SLOT(exportTo()));
    connect(mCopyToClipboardAction, SIGNAL(triggered(bool)),
            this, SLOT(copyToClipboard()));
    connect(mCustomAxesAction, SIGNAL(triggered(bool)),
            this, SLOT(customAxes()));
    connect(mGraphPanelSettingsAction, SIGNAL(triggered(bool)),
            this, SIGNAL(graphPanelSettingsRequested()));
    connect(mGraphsSettingsAction, SIGNAL(triggered(bool)),
            this, SIGNAL(graphsSettingsRequested()));
    connect(mZoomInAction, SIGNAL(triggered(bool)),
            this, SLOT(zoomIn()));
    connect(mZoomOutAction, SIGNAL(triggered(bool)),
            this, SLOT(zoomOut()));
    connect(mResetZoomAction, SIGNAL(triggered(bool)),
            this, SLOT(resetZoom()));

    mContextMenu->addAction(mExportToAction);
    mContextMenu->addSeparator();
    mContextMenu->addAction(mCopyToClipboardAction);

    if (pSynchronizeXAxisAction && pSynchronizeYAxisAction) {
        mContextMenu->addSeparator();
        mContextMenu->addAction(pSynchronizeXAxisAction);
        mContextMenu->addAction(pSynchronizeYAxisAction);
    }

    mContextMenu->addSeparator();
    mContextMenu->addAction(mGraphPanelSettingsAction);
    mContextMenu->addAction(mGraphsSettingsAction);
    mContextMenu->addSeparator();
    mContextMenu->addAction(mCustomAxesAction);
    mContextMenu->addSeparator();
    mContextMenu->addAction(mZoomInAction);
    mContextMenu->addAction(mZoomOutAction);
    mContextMenu->addSeparator();
    mContextMenu->addAction(mResetZoomAction);

    // Set our axes' values
    // Note: we are not all initialised yet, so we don't want setAxes() to
    //       replot ourselves...

    setAxes(DefaultMinAxis, DefaultMaxAxis, DefaultMinAxis, DefaultMaxAxis, false, false, false);

    // Some further initialisations that are done as part of retranslating the
    // GUI (so that they can be updated when changing languages)

    retranslateUi();
}

//==============================================================================

GraphPanelPlotWidget::~GraphPanelPlotWidget()
{
    // Delete some internal objects

    delete mDirectPainter;

    foreach (GraphPanelPlotGraph *graph, mGraphs)
        delete graph;
}

//==============================================================================

void GraphPanelPlotWidget::retranslateUi()
{
    // Retranslate our actions

    I18nInterface::retranslateAction(mExportToAction, tr("Export To..."),
                                     tr("Export the contents of the graph panel to a PDF, PNG, SVG, etc. file"));
    I18nInterface::retranslateAction(mCopyToClipboardAction, tr("Copy To Clipboard"),
                                     tr("Copy the contents of the graph panel to the clipboard"));
    I18nInterface::retranslateAction(mCustomAxesAction, tr("Custom Axes..."),
                                     tr("Specify custom axes for the graph panel"));
    I18nInterface::retranslateAction(mGraphPanelSettingsAction, tr("Graph Panel Settings..."),
                                     tr("Customise the graph panel"));
    I18nInterface::retranslateAction(mGraphsSettingsAction, tr("Graphs Settings..."),
                                     tr("Customise the graphs"));
    I18nInterface::retranslateAction(mZoomInAction, tr("Zoom In"),
                                     tr("Zoom in the graph panel"));
    I18nInterface::retranslateAction(mZoomOutAction, tr("Zoom Out"),
                                     tr("Zoom out the graph panel"));
    I18nInterface::retranslateAction(mResetZoomAction, tr("Reset Zoom"),
                                     tr("Reset the zoom level of the graph panel"));

    // Replot ourselves
    // Note: we do this because we want to display numbers using digit grouping,
    //       this respecting the current locale...

    mAxisX->retranslateUi();
    mAxisY->retranslateUi();
}

//==============================================================================

bool GraphPanelPlotWidget::eventFilter(QObject *pObject, QEvent *pEvent)
{
    // Default handling of the event

    bool res = QwtPlot::eventFilter(pObject, pEvent);

    // We want to handle a double mouse click, but for some reasons to override
    // mouseDoubleClickEvent() doesn't work, so we do it ourselves

    if (pEvent->type() == QEvent::MouseButtonDblClick) {
        // Reset the zoom level (i.e. our axes), in case we double-clicked using
        // the left mouse button with no modifiers

        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(pEvent);

        if (   !(mouseEvent->modifiers() & Qt::ShiftModifier)
            && !(mouseEvent->modifiers() & Qt::ControlModifier)
            && !(mouseEvent->modifiers() & Qt::AltModifier)
            && !(mouseEvent->modifiers() & Qt::MetaModifier)
            &&  (mouseEvent->button() == Qt::LeftButton)) {
            resetAxes();
        }
    }

    return res;
}

//==============================================================================

void GraphPanelPlotWidget::updateActions()
{
    // Leave straightaway if we cannot update our actions anymore

    if (!mCanUpdateActions)
        return;

    // Update our actions

    double crtMinX = minX();
    double crtMaxX = maxX();
    double crtMinY = minY();
    double crtMaxY = maxY();

    double crtRangeX = crtMaxX-crtMinX;
    double crtRangeY = crtMaxY-crtMinY;

    mCanZoomInX = crtRangeX > MinAxisRange;
    mCanZoomOutX = crtRangeX < (logAxisX()?MaxLogAxisRange:MaxAxisRange);

    mCanZoomInY = crtRangeY > MinAxisRange;
    mCanZoomOutY = crtRangeY < (logAxisY()?MaxLogAxisRange:MaxAxisRange);

    // Update the enabled status of our actions

    mSynchronizeXAxisAction->setVisible(!mNeighbors.isEmpty());
    mSynchronizeYAxisAction->setVisible(!mNeighbors.isEmpty());

    mZoomInAction->setEnabled(mCanZoomInX || mCanZoomInY);
    mZoomOutAction->setEnabled(mCanZoomOutX || mCanZoomOutY);

    QRectF dRect = realDataRect();

    mResetZoomAction->setEnabled(   (crtMinX != dRect.left())
                                 || (crtMaxX != dRect.left()+dRect.width())
                                 || (crtMinY != dRect.top())
                                 || (crtMaxY != dRect.top()+dRect.height()));
}

//==============================================================================

void GraphPanelPlotWidget::checkAxisValues(const bool &pLogAxis, double &pMin,
                                           double &pMax)
{
    // Make sure that our axis' values have finite values

    double minAxis = pLogAxis?MinLogAxis:MinAxis;

    if (!qIsFinite(pMin))
        pMin = minAxis;

    if (!qIsFinite(pMax))
        pMax = MaxAxis;

    // Make sure that our axis' values are valid

    double range = pMax-pMin;

    if (range > (pLogAxis?MaxLogAxisRange:MaxAxisRange)) {
        // The range is too big, so reset our values

        pMin = minAxis;
        pMax = MaxAxis;
    } else if (range < MinAxisRange) {
        // The range is too small, so reset our values

        pMin = qMax(minAxis, 0.5*(pMin+pMax-MinAxisRange));
        pMax = qMin(MaxAxis, pMin+MinAxisRange);
        pMin = pMax-MinAxisRange;
        // Note: the last statement is in case pMax was set to MaxAxis, in which
        //       case pMin has to be re-reset...
    } else if (pMin < minAxis) {
        // The minimum value is too small, so reset it

        pMin = minAxis;
        pMax = pMin+range;
    } else if (pMax > MaxAxis) {
        // The maximum value is too big, so reset it

        pMax = MaxAxis;
        pMin = pMax-range;
    }
}

//==============================================================================

GraphPanelPlotWidget::Action GraphPanelPlotWidget::action() const
{
    // Return our action

    return mAction;
}

//==============================================================================

void GraphPanelPlotWidget::resetAction()
{
    // Reset our action and our overlay widget, by updating it

    mAction = None;

    mOverlayWidget->update();
}

//==============================================================================

QColor GraphPanelPlotWidget::backgroundColor() const
{
    // Return our background colour

    return mBackgroundColor;
}

//==============================================================================

void GraphPanelPlotWidget::setBackgroundColor(const QColor &pBackgroundColor)
{
    // Set our background colour
    // Note: setCanvasBackground() doesn't handle semi-transparent colours and,
    //       even if it did, it might take into account the grey background that
    //       is used by the rest of our object while we want a semi-transparent
    //       colour to rely on a white background, which we do here by computing
    //       an opaque colour from an opaque white colour and the given
    //       (potentially semi-transparent) colour...

    if (pBackgroundColor != mBackgroundColor) {
        mBackgroundColor = pBackgroundColor;

        // Set our canvas background colour

        static const QColor White = Qt::white;

        QBrush brush = canvasBackground();
        double ratio = pBackgroundColor.alpha()/256.0;
        QColor color;

        color.setRedF((1.0-ratio)*White.redF()+ratio*pBackgroundColor.redF());
        color.setGreenF((1.0-ratio)*White.greenF()+ratio*pBackgroundColor.greenF());
        color.setBlueF((1.0-ratio)*White.blueF()+ratio*pBackgroundColor.blueF());

        brush.setColor(color);

        setCanvasBackground(brush);

        // Set the colour of the background surrounding our canvas, which we
        // make slightly darker than that of our canvas background colour, based
        // on the typical colour used for a widget's background

        QPalette pal = palette();
        QColor backgroundColor = Core::windowColor();

        backgroundColor.setRedF(backgroundColor.redF()*color.redF());
        backgroundColor.setGreenF(backgroundColor.greenF()*color.greenF());
        backgroundColor.setBlueF(backgroundColor.blueF()*color.blueF());

        pal.setColor(QPalette::Window, backgroundColor);

        setPalette(pal);

        replot();
    }
}

//==============================================================================

int GraphPanelPlotWidget::fontSize() const
{
    // Return our font size

    return font().pointSize();
}

//==============================================================================

void GraphPanelPlotWidget::setFontSize(const int &pFontSize,
                                       const bool &pForceSetting)
{
    // Set our font size

    if (pForceSetting || (pFontSize != fontSize())) {
        QFont newFont = font();

        newFont.setPointSize(pFontSize);

        setFont(newFont);

        // Legend

        GraphPanelPlotLegendWidget *legend = static_cast<GraphPanelPlotLegendWidget *>(QwtPlot::legend());

        if (legend)
            legend->setFontSize(pFontSize);

        // Title

        setTitle(title().text());

        // X axis

        newFont = axisFont(QwtPlot::xBottom);

        newFont.setPointSize(pFontSize);

        setAxisFont(QwtPlot::xBottom, newFont);
        setTitleAxisX(titleAxisX());

        // Y axis

        newFont = axisFont(QwtPlot::yLeft);

        newFont.setPointSize(pFontSize);

        setAxisFont(QwtPlot::yLeft, newFont);
        setTitleAxisY(titleAxisY());
    }
}

//==============================================================================

QColor GraphPanelPlotWidget::foregroundColor() const
{
    // Return our foreground colour

    return mForegroundColor;
}

//==============================================================================

void GraphPanelPlotWidget::setForegroundColor(const QColor &pForegroundColor)
{
    // Set our foreground colour

    if (pForegroundColor != mForegroundColor) {
        mForegroundColor = pForegroundColor;

        // Legend

        GraphPanelPlotLegendWidget *legend = static_cast<GraphPanelPlotLegendWidget *>(QwtPlot::legend());

        if (legend)
            legend->setForegroundColor(pForegroundColor);

        // Title

        setTitle(title().text());

        // X axis

        QwtScaleWidget *axis = axisWidget(QwtPlot::xBottom);
        QPalette newPalette = axis->palette();

        newPalette.setColor(QPalette::Text, pForegroundColor);
        newPalette.setColor(QPalette::WindowText, pForegroundColor);

        axis->setPalette(newPalette);

        setTitleAxisX(titleAxisX());

        // Y axis

        axis = axisWidget(QwtPlot::yLeft);
        newPalette = axis->palette();

        newPalette.setColor(QPalette::Text, pForegroundColor);
        newPalette.setColor(QPalette::WindowText, pForegroundColor);

        axis->setPalette(newPalette);

        setTitleAxisY(titleAxisY());
    }
}

//==============================================================================

Qt::PenStyle GraphPanelPlotWidget::gridLinesStyle() const
{
    // Return our grid lines style

    return mGrid->majorPen().style();
}

//==============================================================================

void GraphPanelPlotWidget::setGridLinesStyle(const Qt::PenStyle &pGridLinesStyle)
{
    // Set our grid lines style

    if (pGridLinesStyle != gridLinesStyle()) {
        QPen newPen = mGrid->majorPen();

        newPen.setStyle(pGridLinesStyle);

        mGrid->setMajorPen(newPen);

        replot();
    }
}

//==============================================================================

int GraphPanelPlotWidget::gridLinesWidth() const
{
    // Return our grid lines width

    return mGrid->majorPen().width();
}

//==============================================================================

void GraphPanelPlotWidget::setGridLinesWidth(const int &pGridLinesWidth)
{
    // Set our grid lines width

    if (pGridLinesWidth != gridLinesWidth()) {
        QPen newPen = mGrid->majorPen();

        newPen.setWidth(pGridLinesWidth);

        mGrid->setMajorPen(newPen);

        replot();
    }
}

//==============================================================================

QColor GraphPanelPlotWidget::gridLinesColor() const
{
    // Return our grid lines colour

    return mGrid->majorPen().color();
}

//==============================================================================

void GraphPanelPlotWidget::setGridLinesColor(const QColor &pGridLinesColor)
{
    // Set our grid lines colour

    if (pGridLinesColor != gridLinesColor()) {
        QPen newPen = mGrid->majorPen();

        newPen.setColor(pGridLinesColor);

        mGrid->setMajorPen(newPen);

        replot();
    }
}

//==============================================================================

bool GraphPanelPlotWidget::legend() const
{
    // Return whether we show our legend

    return QwtPlot::legend();
}

//==============================================================================

void GraphPanelPlotWidget::setLegend(const bool &pLegend)
{
    // Show/hide our legend

    if (pLegend != legend())
        insertLegend(pLegend?new GraphPanelPlotLegendWidget(this):0);
}

//==============================================================================

Qt::PenStyle GraphPanelPlotWidget::pointCoordinatesStyle() const
{
    // Return our point coordinates style

    return mPointCoordinatesStyle;
}

//==============================================================================

void GraphPanelPlotWidget::setPointCoordinatesStyle(const Qt::PenStyle &pPointCoordinatesStyle)
{
    // Set our point coordinates style

    if (pPointCoordinatesStyle != mPointCoordinatesStyle)
        mPointCoordinatesStyle = pPointCoordinatesStyle;
}

//==============================================================================

int GraphPanelPlotWidget::pointCoordinatesWidth() const
{
    // Return our point coordinates width

    return mPointCoordinatesWidth;
}

//==============================================================================

void GraphPanelPlotWidget::setPointCoordinatesWidth(const int &pPointCoordinatesWidth)
{
    // Set our point coordinates width

    if (pPointCoordinatesWidth != mPointCoordinatesWidth)
        mPointCoordinatesWidth = pPointCoordinatesWidth;
}

//==============================================================================

QColor GraphPanelPlotWidget::pointCoordinatesColor() const
{
    // Return our point coordinates colour

    return mPointCoordinatesColor;
}

//==============================================================================

void GraphPanelPlotWidget::setPointCoordinatesColor(const QColor &pPointCoordinatesColor)
{
    // Set our point coordinates colour

    if (pPointCoordinatesColor != mPointCoordinatesColor)
        mPointCoordinatesColor = pPointCoordinatesColor;
}

//==============================================================================

QColor GraphPanelPlotWidget::pointCoordinatesFontColor() const
{
    // Return our point coordinates font colour

    return mPointCoordinatesFontColor;
}

//==============================================================================

void GraphPanelPlotWidget::setPointCoordinatesFontColor(const QColor &pPointCoordinatesFontColor)
{
    // Set our point coordinates font colour

    if (pPointCoordinatesFontColor != mPointCoordinatesFontColor)
        mPointCoordinatesFontColor = pPointCoordinatesFontColor;
}

//==============================================================================

void GraphPanelPlotWidget::setTitle(const QString &pTitle)
{
    // Set our title

    if (pTitle.isEmpty()) {
        QwtPlot::setTitle(QString());
    } else {
        QwtText title = QwtText(pTitle);
        QFont newFont = title.font();

        newFont.setPointSize(2*fontSize());

        title.setColor(mForegroundColor);
        title.setFont(newFont);

        QwtPlot::setTitle(title);
    }
}

//==============================================================================

bool GraphPanelPlotWidget::logAxisX() const
{
    // Return whether our X axis uses a logarithmic scale

    return mLogAxisX;
}

//==============================================================================

void GraphPanelPlotWidget::setLogAxisX(const bool &pLogAxisX)
{
    // Specify whether our X axis should use a logarithmic scale

    if (pLogAxisX != mLogAxisX) {
        mLogAxisX = pLogAxisX;

        setAxisScaleEngine(QwtPlot::xBottom,
                           pLogAxisX?
                               static_cast<QwtScaleEngine *>(new QwtLogScaleEngine()):
                               static_cast<QwtScaleEngine *>(new QwtLinearScaleEngine()));

        resetAxes();

        replot();
    }
}

//==============================================================================

QString GraphPanelPlotWidget::titleAxisX() const
{
    // Return the title for our X axis

    return axisTitle(QwtPlot::xBottom).text();
}

//==============================================================================

void GraphPanelPlotWidget::setTitleAxisX(const QString &pTitleAxisX)
{
    // Set the title for our X axis

    setTitleAxis(QwtPlot::xBottom, pTitleAxisX);
}

//==============================================================================

bool GraphPanelPlotWidget::logAxisY() const
{
    // Return whether our Y axis uses a logarithmic scale

    return mLogAxisY;
}

//==============================================================================

void GraphPanelPlotWidget::setLogAxisY(const bool &pLogAxisY)
{
    // Specify whether our Y axis should use a logarithmic scale

    if (pLogAxisY != mLogAxisY) {
        mLogAxisY = pLogAxisY;

        setAxisScaleEngine(QwtPlot::yLeft,
                           pLogAxisY?
                               static_cast<QwtScaleEngine *>(new QwtLogScaleEngine()):
                               static_cast<QwtScaleEngine *>(new QwtLinearScaleEngine()));

        resetAxes();

        replot();
    }
}

//==============================================================================

QString GraphPanelPlotWidget::titleAxisY() const
{
    // Return the title for our Y axis

    return axisTitle(QwtPlot::yLeft).text();
}

//==============================================================================

void GraphPanelPlotWidget::setTitleAxisY(const QString &pTitleAxisY)
{
    // Set the title for our Y axis

    setTitleAxis(QwtPlot::yLeft, pTitleAxisY);
}

//==============================================================================

Qt::PenStyle GraphPanelPlotWidget::zoomRegionStyle() const
{
    // Return our zoom region style

    return mZoomRegionStyle;
}

//==============================================================================

void GraphPanelPlotWidget::setZoomRegionStyle(const Qt::PenStyle &pZoomRegionStyle)
{
    // Set our zoom region style

    if (pZoomRegionStyle != mZoomRegionStyle)
        mZoomRegionStyle = pZoomRegionStyle;
}

//==============================================================================

int GraphPanelPlotWidget::zoomRegionWidth() const
{
    // Return our zoom region width

    return mZoomRegionWidth;
}

//==============================================================================

void GraphPanelPlotWidget::setZoomRegionWidth(const int &pZoomRegionWidth)
{
    // Set our zoom region width

    if (pZoomRegionWidth != mZoomRegionWidth)
        mZoomRegionWidth = pZoomRegionWidth;
}

//==============================================================================

QColor GraphPanelPlotWidget::zoomRegionColor() const
{
    // Return our zoom region colour

    return mZoomRegionColor;
}

//==============================================================================

void GraphPanelPlotWidget::setZoomRegionColor(const QColor &pZoomRegionColor)
{
    // Set our zoom region colour

    if (pZoomRegionColor != mZoomRegionColor)
        mZoomRegionColor = pZoomRegionColor;
}

//==============================================================================

QColor GraphPanelPlotWidget::zoomRegionFontColor() const
{
    // Return our zoom region font colour

    return mZoomRegionFontColor;
}

//==============================================================================

void GraphPanelPlotWidget::setZoomRegionFontColor(const QColor &pZoomRegionFontColor)
{
    // Set our zoom region font colour

    if (pZoomRegionFontColor != mZoomRegionFontColor)
        mZoomRegionFontColor = pZoomRegionFontColor;
}

//==============================================================================

bool GraphPanelPlotWidget::zoomRegionFilled() const
{
    // Return our zoom region filled status

    return mZoomRegionFilled;
}

//==============================================================================

void GraphPanelPlotWidget::setZoomRegionFilled(const bool &pZoomRegionFilled)
{
    // Set our zoom region filled status

    if (pZoomRegionFilled != mZoomRegionFilled)
        mZoomRegionFilled = pZoomRegionFilled;
}

//==============================================================================

QColor GraphPanelPlotWidget::zoomRegionFillColor() const
{
    // Return our zoom region fill colour

    return mZoomRegionFillColor;
}

//==============================================================================

void GraphPanelPlotWidget::setZoomRegionFillColor(const QColor &pZoomRegionFillColor)
{
    // Set our zoom region fill colour

    if (pZoomRegionFillColor != mZoomRegionFillColor)
        mZoomRegionFillColor = pZoomRegionFillColor;
}

//==============================================================================

double GraphPanelPlotWidget::minX() const
{
    // Return our minimum X value

    return axisScaleDiv(QwtPlot::xBottom).lowerBound();
}

//==============================================================================

double GraphPanelPlotWidget::maxX() const
{
    // Return our maximum X value

    return axisScaleDiv(QwtPlot::xBottom).upperBound();
}

//==============================================================================

double GraphPanelPlotWidget::minY() const
{
    // Return our minimum Y value

    return axisScaleDiv(QwtPlot::yLeft).lowerBound();
}

//==============================================================================

double GraphPanelPlotWidget::maxY() const
{
    // Return our maximum Y value

    return axisScaleDiv(QwtPlot::yLeft).upperBound();

}

//==============================================================================

bool GraphPanelPlotWidget::canZoomInX() const
{
    // Return whether we can zoom in on the X axis

    return mCanZoomInX;
}

//==============================================================================

bool GraphPanelPlotWidget::canZoomOutX() const
{
    // Return whether we can zoom out on the X axis

    return mCanZoomOutX;
}

//==============================================================================

bool GraphPanelPlotWidget::canZoomInY() const
{
    // Return whether we can zoom in on the Y axis

    return mCanZoomInY;
}

//==============================================================================

bool GraphPanelPlotWidget::canZoomOutY() const
{
    // Return whether we can zoom out on the Y axis

    return mCanZoomOutY;
}

//==============================================================================

GraphPanelPlotGraphs GraphPanelPlotWidget::graphs() const
{
    // Return all our graphs

    return mGraphs;
}

//==============================================================================

void GraphPanelPlotWidget::optimiseAxis(double &pMin, double &pMax) const
{
    // Make sure that the given values are different

    if (pMin == pMax) {
        // The given values are the same, so update them so that we can properly
        // optimise them below

        double powerValue = pMin?std::floor(log10(qAbs(pMin)))-1.0:0.0;

        pMin = pMin-pow(10.0, powerValue);
        pMax = pMax+pow(10.0, powerValue);
    }
}

//==============================================================================

bool GraphPanelPlotWidget::hasData() const
{
    // Determine and return whether at least one of the graphs, which are valid
    // and selected, has some data

    foreach (GraphPanelPlotGraph *graph, mGraphs) {
        if (graph->isValid() && graph->isSelected() && graph->dataSize())
            return true;
    }

    return false;
}

//==============================================================================

bool GraphPanelPlotWidget::dataRect(QRectF &pDataRect) const
{
    // Determine and return the rectangle within which all the graphs, which are
    // valid, selected and have some data, can fit

    bool res = false;

    pDataRect = QRectF();

    foreach (GraphPanelPlotGraph *graph, mGraphs) {
        if (graph->isValid() && graph->isSelected() && graph->dataSize()) {
            pDataRect |= graph->boundingRect();

            res = true;
        }
    }

    return res;
}

//==============================================================================

bool GraphPanelPlotWidget::dataLogRect(QRectF &pDataLogRect) const
{
    // Determine and return the log rectangle within which all the graphs, which
    // are valid, selected and have some data, can fit

    bool res = false;

    pDataLogRect = QRectF();

    foreach (GraphPanelPlotGraph *graph, mGraphs) {
        if (graph->isValid() && graph->isSelected() && graph->dataSize()) {
            QRectF boundingLogRect = graph->boundingLogRect();

            if (boundingLogRect != NoBoundingLogRect) {
                pDataLogRect |= boundingLogRect;

                res = true;
            }
        }
    }

    return res;
}

//==============================================================================

QRectF GraphPanelPlotWidget::realDataRect() const
{
    // Return an optimised version of dataRect() or a default rectangle, if no
    // dataRect() exists

    QRectF dRect = QRectF();
    QRectF dLogRect = QRectF();

    if (dataRect(dRect) && dataLogRect(dLogRect)) {
        // Optimise our axes' values

        double minX = logAxisX()?dLogRect.left():dRect.left();
        double maxX = minX+(logAxisX()?dLogRect.width():dRect.width());
        double minY = logAxisY()?dLogRect.top():dRect.top();
        double maxY = minY+(logAxisY()?dLogRect.height():dRect.height());

        optimiseAxis(minX, maxX);
        optimiseAxis(minY, maxY);

        return QRectF(minX, minY, maxX-minX, maxY-minY);
    } else {
        double minX = logAxisX()?mDefaultMinLogX:mDefaultMinX;
        double maxX = logAxisX()?mDefaultMaxLogX:mDefaultMaxX;
        double minY = logAxisY()?mDefaultMinLogY:mDefaultMinY;
        double maxY = logAxisY()?mDefaultMaxLogY:mDefaultMaxY;

        return QRectF(minX, minY, maxX-minX, maxY-minY);
    }
}

//==============================================================================

void GraphPanelPlotWidget::setAxis(const int &pAxisId, double pMin, double pMax)
{
    // Set our axis
    // Note: to use setAxisScale() on its own is not sufficient unless we were
    //       to replot ourselves immediately after, but we don't want to do
    //       that, so instead we also use setAxisScaleDiv() to make sure that
    //       our axis is indeed taken into account (i.e. we can retrieve them
    //       using minX(), maxX(), minY() and maxY()). Also, we must call
    //       setAxisScaleDiv() before setAxisScale() to make sure that the axis
    //       data is not considered as valid, which is important when it comes
    //       to plotting ourselves...

    setAxisScaleDiv(pAxisId, QwtScaleDiv(pMin, pMax));
    setAxisScale(pAxisId, pMin, pMax);
}

//==============================================================================

void GraphPanelPlotWidget::setDefaultAxesValues(const double &pDefaultMinX,
                                                const double &pDefaultMaxX,
                                                const double &pDefaultMinLogX,
                                                const double &pDefaultMaxLogX,
                                                const double &pDefaultMinY,
                                                const double &pDefaultMaxY,
                                                const double &pDefaultMinLogY,
                                                const double &pDefaultMaxLogY)
{
    // Set the default axes values

    mDefaultMinX = pDefaultMinX;
    mDefaultMaxX = pDefaultMaxX;
    mDefaultMinLogX = pDefaultMinLogX;
    mDefaultMaxLogX = pDefaultMaxLogX;

    mDefaultMinY = pDefaultMinY;
    mDefaultMaxY = pDefaultMaxY;
    mDefaultMinLogY = pDefaultMinLogY;
    mDefaultMaxLogY = pDefaultMaxLogY;
}

//==============================================================================

bool GraphPanelPlotWidget::setAxes(double pMinX, double pMaxX, double pMinY,
                                   double pMaxY, const bool &pSynchronizeAxes,
                                   const bool &pCanReplot,
                                   const bool &pEmitSignal,
                                   const bool &pForceXAxisSetting,
                                   const bool &pForceYAxisSetting)
{
    // Keep track of our axes' old values

    double oldMinX = minX();
    double oldMaxX = maxX();
    double oldMinY = minY();
    double oldMaxY = maxY();

    // Make sure that the given axes' values are fine

    checkAxisValues(logAxisX(), pMinX, pMaxX);
    checkAxisValues(logAxisY(), pMinY, pMaxY);

    // Update our axes' values, if needed

    bool xAxisValuesChanged = false;
    bool yAxisValuesChanged = false;

    if (pForceXAxisSetting || (pMinX != oldMinX) || (pMaxX != oldMaxX)) {
        setAxis(QwtPlot::xBottom, pMinX, pMaxX);

        xAxisValuesChanged = true;
    }

    if (pForceYAxisSetting || (pMinY != oldMinY) || (pMaxY != oldMaxY)) {
        setAxis(QwtPlot::yLeft, pMinY, pMaxY);

        yAxisValuesChanged = true;
    }

    // Update our actions and align ourselves with our neighbours (which will
    // result in ourselves, and maybe its neighbours, bein replotted, if
    // allowed), in case the axes' values have changed

    if (xAxisValuesChanged || yAxisValuesChanged) {
        mCanDirectPaint = false;

        if (xAxisValuesChanged || yAxisValuesChanged)
            updateActions();

        if (pSynchronizeAxes) {
            if (   mSynchronizeXAxisAction->isChecked()
                && mSynchronizeYAxisAction->isChecked()) {
                foreach (GraphPanelPlotWidget *neighbor, mNeighbors)
                    neighbor->setAxes(pMinX, pMaxX, pMinY, pMaxY, false, false, false);
            } else if (xAxisValuesChanged && mSynchronizeXAxisAction->isChecked()) {
                foreach (GraphPanelPlotWidget *neighbor, mNeighbors)
                    neighbor->setAxes(pMinX, pMaxX, neighbor->minY(), neighbor->maxY(), false, false, false);
            } else if (yAxisValuesChanged && mSynchronizeYAxisAction->isChecked()) {
                foreach (GraphPanelPlotWidget *neighbor, mNeighbors)
                    neighbor->setAxes(neighbor->minX(), neighbor->maxX(), pMinY, pMaxY, false, false, false);
            }

            alignWithNeighbors(pCanReplot, true);
        }

        if (pEmitSignal)
            emit axesChanged(pMinX, pMaxX, pMinY, pMaxY);

        return pCanReplot;
    } else {
        return false;
    }
}

//==============================================================================

bool GraphPanelPlotWidget::resetAxes()
{
    // Reset our axes by setting their values to either default ones or to some
    // that allow us to see all the graphs

    QRectF dRect = realDataRect();

    return setAxes(dRect.left(), dRect.left()+dRect.width(),
                   dRect.top(), dRect.top()+dRect.height());
}

//==============================================================================

bool GraphPanelPlotWidget::scaleAxis(const Scaling &pScaling,
                                     const bool &pCanZoomIn,
                                     const bool &pCanZoomOut,
                                     const QwtScaleMap &pCanvasMap,
                                     const double &pPoint, double &pMin,
                                     double &pMax)
{
    // Check whether we can scale the axis and, if so, determine what its new
    // values should be

    if (   ((pScaling < NoScaling) && pCanZoomIn)
        || ((pScaling > NoScaling) && pCanZoomOut)) {
        static const double ScalingInFactor     = 0.9;
        static const double ScalingOutFactor    = 1.0/ScalingInFactor;
        static const double BigScalingInFactor  = 0.5*ScalingInFactor;
        static const double BigScalingOutFactor = 1.0/BigScalingInFactor;

        double min = pCanvasMap.transform(pMin);
        double range = pCanvasMap.transform(pMax)-min;
        double factor = qMin(1.0, qMax(0.0, (pPoint-min)/range));
        // Note: QwtPlot puts some extra space around the area we want to show,
        //       which means that we could end up with a factor which is either
        //       smaller than zero or bigger than one, hence we have to make
        //       sure that it is clamped within the [0; 1] range...

        switch (pScaling) {
        case BigScalingIn:
            range *= BigScalingInFactor;

            break;
        case ScalingIn:
            range *= ScalingInFactor;

            break;
        case NoScaling:
            break;
        case ScalingOut:
            range *= ScalingOutFactor;

            break;
        case BigScalingOut:
            range *= BigScalingOutFactor;

            break;
        }

        pMin = qMax(MinAxis, pCanvasMap.invTransform(pPoint-factor*range));
        pMax = qMin(MaxAxis, pCanvasMap.invTransform(pCanvasMap.transform(pMin)+range));
        pMin = pCanvasMap.invTransform(pCanvasMap.transform(pMax)-range);
        // Note: the last statement is in case pMax has been set to MaxAxis, in
        //       which case we need to re-update pMin...

        return true;
    } else {
        return false;
    }
}

//==============================================================================

void GraphPanelPlotWidget::scaleAxes(const QPoint &pPoint,
                                     const Scaling &pScalingX,
                                     const Scaling &pScalingY)
{
    // Rescale our X axis, but only if zooming in/out is possible on that axis

    QPointF point = pPoint-plotLayout()->canvasRect().topLeft();

    double newMinX = minX();
    double newMaxX = maxX();
    double newMinY = minY();
    double newMaxY = maxY();

    bool scaledAxisX = scaleAxis(pScalingX, mCanZoomInX, mCanZoomOutX,
                                 canvasMap(QwtPlot::xBottom), point.x(),
                                 newMinX, newMaxX);
    bool scaledAxisY = scaleAxis(pScalingY, mCanZoomInY, mCanZoomOutY,
                                 canvasMap(QwtPlot::yLeft), point.y(),
                                 newMinY, newMaxY);
    // Note: we want to make both calls to scaleAxis(), hence they are not part
    //       of the if() statement below...

    if (scaledAxisX || scaledAxisY)
        setAxes(newMinX, newMaxX, newMinY, newMaxY);
}

//==============================================================================

void GraphPanelPlotWidget::setTitleAxis(const int &pAxisId,
                                        const QString &pTitleAxis)
{
    // Set the title for our axis

    if (pTitleAxis.isEmpty()) {
        setAxisTitle(pAxisId, QString());
    } else {
        QwtText axisTitle = QwtText(pTitleAxis);
        QFont axisTitleFont = axisTitle.font();

        axisTitleFont.setPointSizeF(1.25*fontSize());

        axisTitle.setColor(mForegroundColor);
        axisTitle.setFont(axisTitleFont);

        setAxisTitle(pAxisId, axisTitle);
    }

    forceAlignWithNeighbors();
}

//==============================================================================

QPointF GraphPanelPlotWidget::canvasPoint(const QPoint &pPoint) const
{
    // Return the mouse position using canvas coordinates, making sure that they
    // are within our ranges

    return QPointF(qMin(maxX(), qMax(minX(), canvasMap(QwtPlot::xBottom).invTransform(pPoint.x()))),
                   qMin(maxY(), qMax(minY(), canvasMap(QwtPlot::yLeft).invTransform(pPoint.y()))));
}

//==============================================================================

void GraphPanelPlotWidget::mouseMoveEvent(QMouseEvent *pEvent)
{
    // Default handling of the event

    QwtPlot::mouseMoveEvent(pEvent);

    // Carry out the action

    switch (mAction) {
    case None:
        // No action, so do nothing

        break;
    case Pan: {
        // Determine the X/Y shifts for our panning

        int shiftX = pEvent->pos().x()-mPoint.x();
        int shiftY = pEvent->pos().y()-mPoint.y();

        mPoint = pEvent->pos();

        // Set our axes' new values

        QwtScaleMap canvasMapX = canvasMap(QwtPlot::xBottom);
        QwtScaleMap canvasMapY = canvasMap(QwtPlot::yLeft);

        setAxes(canvasMapX.invTransform(canvasMapX.transform(minX())-shiftX),
                canvasMapX.invTransform(canvasMapX.transform(maxX())-shiftX),
                canvasMapY.invTransform(canvasMapY.transform(minY())-shiftY),
                canvasMapY.invTransform(canvasMapY.transform(maxY())-shiftY));

        break;
    }
    case ShowCoordinates:
        // Update the point of our overlay widget

        mOverlayWidget->setPoint(pEvent->pos());

        break;
    case Zoom: {
        // Determine our X/Y delta values

        int deltaX = pEvent->pos().x()-mPoint.x();
        int deltaY = pEvent->pos().y()-mPoint.y();

        mPoint = pEvent->pos();

        // Rescale ourselves
        // Note: this will automatically replot ourselves...

        scaleAxes(mOriginPoint,
                  deltaX?(deltaX > 0)?ScalingIn:ScalingOut:NoScaling,
                  deltaY?(deltaY < 0)?ScalingIn:ScalingOut:NoScaling);

        break;
    }
    case ZoomRegion:
        // Update our zoom region by updating the point of our overlay widget

        mOverlayWidget->setPoint(pEvent->pos());

        break;
    }

    // The mouse has moved, so we definitely won't need to show our context menu

    mNeedContextMenu = false;
}

//==============================================================================

void GraphPanelPlotWidget::mousePressEvent(QMouseEvent *pEvent)
{
    // Default handling of the event

    QwtPlot::mousePressEvent(pEvent);

    // Check that the position of the mouse is over our canvas

    if (!plotLayout()->canvasRect().contains(pEvent->pos()))
        return;

    // Make sure that we are not already carrying out a action (e.g. we were
    // zooming in/out and then pressed on the left mouse button) and if so, then
    // cancel it by resetting our action

    if (mAction != None) {
        resetAction();

        return;
    }

    // Check which action to can carry out

    bool noModifiers =    !(pEvent->modifiers() & Qt::ShiftModifier)
                       && !(pEvent->modifiers() & Qt::ControlModifier)
                       && !(pEvent->modifiers() & Qt::AltModifier)
                       && !(pEvent->modifiers() & Qt::MetaModifier);

    if (noModifiers && (pEvent->button() == Qt::LeftButton)) {
        // We want to pan, but only do this if we are not completely zoomed out

        if (mCanZoomOutX || mCanZoomOutY) {
            mAction = Pan;

            mPoint = pEvent->pos();
        }
    } else if (    (pEvent->modifiers() & Qt::ShiftModifier)
               && !(pEvent->modifiers() & Qt::ControlModifier)
               && !(pEvent->modifiers() & Qt::AltModifier)
               && !(pEvent->modifiers() & Qt::MetaModifier)
               && (pEvent->button() == Qt::LeftButton)) {
        // We want to show the coordinates

        mAction = ShowCoordinates;

        mOverlayWidget->setPoint(pEvent->pos());
    } else if (noModifiers && (pEvent->button() == Qt::RightButton)) {
        // We want to zoom in/out

        mAction = Zoom;

        mOriginPoint = pEvent->pos();
        mPoint = pEvent->pos();
    } else if (   !(pEvent->modifiers() & Qt::ShiftModifier)
               &&  (pEvent->modifiers() & Qt::ControlModifier)
               && !(pEvent->modifiers() & Qt::AltModifier)
               && !(pEvent->modifiers() & Qt::MetaModifier)
               && (pEvent->button() == Qt::RightButton)) {
        // We want to zoom a region, but only allow it if we are not already
        // fully zoomed in

        if (mCanZoomInX || mCanZoomInY) {
            mAction = ZoomRegion;

            mOverlayWidget->setOriginPoint(pEvent->pos());
            mOverlayWidget->setPoint(pEvent->pos());
        }
    }

    // Check whether we might need to show our context menu

    mNeedContextMenu = pEvent->button() == Qt::RightButton;
}

//==============================================================================

void GraphPanelPlotWidget::mouseReleaseEvent(QMouseEvent *pEvent)
{
    // Default handling of the event

    QwtPlot::mouseReleaseEvent(pEvent);

    // Check whether we need to carry out a action

    if (mAction == None)
        return;

    // Finish carrying out the action, if needed

    switch (mAction) {
    case ZoomRegion: {
        // Retrieve our zoom region

        QRect zoomRegionRect = mOverlayWidget->zoomRegion();

        // Reset our action

        resetAction();

        // Effectively zoom our region, if possible, by updating our axes

        QRectF zoomRegion = QRectF(canvasPoint(zoomRegionRect.topLeft()),
                                   canvasPoint(zoomRegionRect.topLeft()+QPoint(zoomRegionRect.width(), zoomRegionRect.height())));

        if (zoomRegion.width() && zoomRegion.height()) {
            setAxes(zoomRegion.left(), zoomRegion.left()+zoomRegion.width(),
                    zoomRegion.top()+zoomRegion.height(), zoomRegion.top());
        }

        break;
    }
    default:
        // An action that doesn't require anything specific to be done, except
        // being reset

        resetAction();
    }

    // Show our context menu, if still needed

    if (mNeedContextMenu) {
        mOriginPoint = mapFromGlobal(QCursor::pos());

        mContextMenu->exec(QCursor::pos());
    }
}

//==============================================================================

void GraphPanelPlotWidget::paintEvent(QPaintEvent *pEvent)
{
    // Default handling of the event

    QwtPlot::paintEvent(pEvent);

    // We have just (re)painted ourselves, which means that we can (re)allow
    // direct painting and replotting

    mCanDirectPaint = true;
    mCanReplot = true;
}

//==============================================================================

void GraphPanelPlotWidget::resizeEvent(QResizeEvent *pEvent)
{
    // Default handling of the event

    QwtPlot::resizeEvent(pEvent);

    // Update the size of our overlay widget

    mOverlayWidget->resize(pEvent->size());
}

//==============================================================================

void GraphPanelPlotWidget::wheelEvent(QWheelEvent *pEvent)
{
    // Handle the wheel mouse button for zooming in/out

    if (   !(pEvent->modifiers() & Qt::ShiftModifier)
        && !(pEvent->modifiers() & Qt::ControlModifier)
        && !(pEvent->modifiers() & Qt::AltModifier)
        && !(pEvent->modifiers() & Qt::MetaModifier)) {
        // Make sure that we are not already carrying out a action

        if (mAction == None) {
            int delta = pEvent->delta();
            Scaling scaling = NoScaling;

            if (delta > 0)
                scaling = ScalingIn;
            else if (delta < 0)
                scaling = ScalingOut;

            if (scaling)
                scaleAxes(pEvent->pos(), scaling, scaling);
        }

        pEvent->accept();
    } else {
        // Not the modifier we were expecting, so call the default handling of
        // the event

        QwtPlot::wheelEvent(pEvent);
    }
}

//==============================================================================

bool GraphPanelPlotWidget::addGraph(GraphPanelPlotGraph *pGraph)
{
    // Make sure that the given graph is not already attached to us

    if (mGraphs.contains(pGraph))
        return false;

    // Attach the given graph to ourselves and keep track of it

    pGraph->attach(this);

    mGraphs << pGraph;

    return true;
}

//==============================================================================

bool GraphPanelPlotWidget::removeGraph(GraphPanelPlotGraph *pGraph)
{
    // Check that the given graph is attached to us

    if (!mGraphs.contains(pGraph))
        return false;

    // Detach the given graph from ourselves, stop tracking it and delete it

    pGraph->detach();

    mGraphs.removeOne(pGraph);

    delete pGraph;

    return true;
}

//==============================================================================

bool GraphPanelPlotWidget::drawGraphFrom(GraphPanelPlotGraph *pGraph,
                                         const qulonglong &pFrom)
{
    // Direct paint our graph from the given point unless we can't direct paint
    // (due to the axes having been changed), in which case we replot ourselves

    if (mCanDirectPaint) {
        mDirectPainter->drawSeries(pGraph, pFrom, -1);

        return false;
    } else {
        if (mCanReplot) {
            replot();

            mCanReplot = false;
        }

        return true;
    }
}

//==============================================================================

GraphPanelPlotWidgets GraphPanelPlotWidget::neighbors() const
{
    // Return our neighbours

    return mNeighbors;
}

//==============================================================================

void GraphPanelPlotWidget::addNeighbor(GraphPanelPlotWidget *pPlot)
{
    // Add the plot as a neighbour and make sure our actions are up to date

    if ((pPlot != this) && !mNeighbors.contains(pPlot)) {
        mNeighbors << pPlot;

        updateActions();
    }
}

//==============================================================================

void GraphPanelPlotWidget::removeNeighbor(GraphPanelPlotWidget *pPlot)
{
    // Remove the plot from our neighbours and make sure our actions are up to
    // date

    mNeighbors.removeOne(pPlot);

    updateActions();
}

//==============================================================================

void GraphPanelPlotWidget::alignWithNeighbors(const bool &pCanReplot,
                                              const bool &pForceAlignment)
{
    // Align ourselves with our neighbours by taking into account the size it
    // takes to draw the Y axis and, if any, its corresponding title (including
    // the gap between the Y axis and its corresponding title)

    GraphPanelPlotWidgets selfPlusNeighbors = GraphPanelPlotWidgets() << this << mNeighbors;
    double oldMinExtent = axisWidget(QwtPlot::yLeft)->scaleDraw()->minimumExtent();
    double newMinExtent = 0;

    foreach (GraphPanelPlotWidget *plot, selfPlusNeighbors) {
        QwtScaleWidget *scaleWidget = plot->axisWidget(QwtPlot::yLeft);
        QwtScaleDraw *scaleDraw = scaleWidget->scaleDraw();

        scaleDraw->setMinimumExtent(0.0);

        plot->updateAxes();
        // Note: this ensures that our major ticks (which are used to compute
        //       the extent) are up to date...

        double minExtent =  scaleDraw->extent(scaleWidget->font())
                           +(plot->titleAxisY().isEmpty()?
                                 0:
                                 scaleWidget->spacing()+scaleWidget->title().textSize().height());

        if (minExtent > newMinExtent)
            newMinExtent = minExtent;
    }

    foreach (GraphPanelPlotWidget *plot, selfPlusNeighbors) {
        GraphPanelPlotScaleWidget *scaleWidget = static_cast<GraphPanelPlotScaleWidget *>(plot->axisWidget(QwtPlot::yLeft));

        scaleWidget->scaleDraw()->setMinimumExtent( newMinExtent
                                                   -(plot->titleAxisY().isEmpty()?
                                                         0:
                                                         scaleWidget->spacing()+scaleWidget->title().textSize().height()));

        if (pCanReplot) {
            if (pForceAlignment || (newMinExtent != oldMinExtent)) {
                scaleWidget->updateLayout();

                plot->replot();
            } else if (plot == this) {
                replot();
            }
        }
    }
}

//==============================================================================

void GraphPanelPlotWidget::forceAlignWithNeighbors()
{
    // Force the re-alignment with our neighbours

    alignWithNeighbors(true, true);
}

//==============================================================================

void GraphPanelPlotWidget::cannotUpdateActions()
{
    // Keep track of the fact that we cannot update our actions anymore

    mCanUpdateActions = false;
}

//==============================================================================

void GraphPanelPlotWidget::exportTo()
{
    // Export our contents to a PDF, SVG, etc. file
    // Note: if no file extension is given, then we export our contents to a PDF
    //       file...

    QString pdfFilter = tr("PDF File - Portable Document Format (*.pdf)");
    QStringList filters = QStringList() << pdfFilter
                                        << tr("SVG File - Scalable Vector Graphics (*.svg)");

    foreach (const QString &supportedMimeType, QImageWriter::supportedMimeTypes()) {
        if (!supportedMimeType.compare("image/bmp"))
            filters << tr("BMP File - Windows Bitmap (*.bmp)");
        else if (!supportedMimeType.compare("image/jpeg"))
            filters << tr("JPEG File - Joint Photographic Experts Group (*.jpg)");
        else if (!supportedMimeType.compare("image/png"))
            filters << tr("PNG File - Portable Network Graphics (*.png)");
        else if (!supportedMimeType.compare("image/x-portable-bitmap"))
            filters << tr("PBM File - Portable Bitmap (*.pbm)");
        else if (!supportedMimeType.compare("image/x-portable-graymap"))
            filters << tr("PGM File - Portable Graymap (*.pgm)");
        else if (!supportedMimeType.compare("image/x-portable-pixmap"))
            filters << tr("PPM File - Portable Pixmap (*.ppm)");
        else if (!supportedMimeType.compare("image/x-xbitmap"))
            filters << tr("XBM File - X11 Bitmap (*.xbm)");
        else if (!supportedMimeType.compare("image/x-xpixmap"))
            filters << tr("XPM File - X11 Pixmap (*.xpm)");
    }

    std::sort(filters.begin(), filters.end());

    QString fileName = Core::getSaveFileName(tr("Export To"), filters, &pdfFilter);

    if (!fileName.isEmpty()) {
        static double InToMm = 25.4;
        static double Dpi = 85.0;

        if (QFileInfo(fileName).completeSuffix().isEmpty())
            QwtPlotRenderer().renderDocument(this, fileName, "pdf", QSizeF(width()*InToMm/Dpi, height()*InToMm/Dpi), Dpi);
        else
            QwtPlotRenderer().renderDocument(this, fileName, QSizeF(width()*InToMm/Dpi, height()*InToMm/Dpi), Dpi);
    }

    // QwtPlotRenderer::renderDocument() changes and then invalidates our
    // layout, so we need to update it
    // Note: indeed, the plot layout's canvas rectangle is, among other things,
    //       used to determine whether we can interact with our plot...

    updateLayout();
}

//==============================================================================

void GraphPanelPlotWidget::copyToClipboard()
{
    // Copy our contents to the clipboard

    QApplication::clipboard()->setPixmap(grab());
}

//==============================================================================

void GraphPanelPlotWidget::customAxes()
{
    // Specify custom axes for the graph panel

    double oldMinX = minX();
    double oldMaxX = maxX();
    double oldMinY = minY();
    double oldMaxY = maxY();

    GraphPanelWidgetCustomAxesDialog customAxesDialog(oldMinX, oldMaxX, oldMinY, oldMaxY, this);

    customAxesDialog.exec();

    // Update our axes' values, if requested and only if they have actually
    // changed

    if (customAxesDialog.result() == QMessageBox::Accepted) {
        double newMinX = customAxesDialog.minX();
        double newMaxX = customAxesDialog.maxX();
        double newMinY = customAxesDialog.minY();
        double newMaxY = customAxesDialog.maxY();

        if (   (newMinX != oldMinX) || (newMaxX != oldMaxX)
            || (newMinY != oldMinY) || (newMaxY != oldMaxY)) {
            setAxes(newMinX, newMaxX, newMinY, newMaxY);
        }
    }
}

//==============================================================================

void GraphPanelPlotWidget::zoomIn()
{
    // Zoom in by scaling our two axes around the point where the context menu
    // was shown

    scaleAxes(mOriginPoint, BigScalingIn, BigScalingIn);
}

//==============================================================================

void GraphPanelPlotWidget::zoomOut()
{
    // Zoom out by scaling our two axes around the point where the context menu
    // was shown

    scaleAxes(mOriginPoint, BigScalingOut, BigScalingOut);
}

//==============================================================================

void GraphPanelPlotWidget::resetZoom()
{
    // Reset the zoom level by resetting our axes

    resetAxes();
}

//==============================================================================

}   // namespace GraphPanelWidget
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
