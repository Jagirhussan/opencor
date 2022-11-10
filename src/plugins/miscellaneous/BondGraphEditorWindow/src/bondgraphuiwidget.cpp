/*******************************************************************************

Copyright (C) The University of Auckland

OpenCOR is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

OpenCOR is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://gnu.org/licenses>.

*******************************************************************************/
#include "bondgraphuiwidget.h"

#include <QApplication>
#include <QCoreApplication>

#include <QDesktopWidget>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QScreen>
#include <QSettings>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QList>

#include "bgannotationviewmetadataeditdetailsui.h"
#include "bgelementeditoruicontroller.h"
#include "bgporthamiltonianrenderer.h"

#include "thirdparty/qtaccordian/qaccordion.h"
#include "thirdparty/widgetbox/libraryitem.h"
#include "thirdparty/widgetbox/widgetbox.h"

#include "thirdparty/json.hpp"

#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wnewline-eof"
    #pragma clang diagnostic ignored "-Wold-style-cast"
    #pragma clang diagnostic ignored "-Wreserved-id-macro"
    #pragma clang diagnostic ignored "-Wdeprecated-dynamic-exception-spec"
#endif
#include "bondgraph.hpp"

#if defined(__clang__)
    #pragma clang diagnostic pop
#endif

namespace OpenCOR {
namespace BondGraphEditorWindow {

BondGraphUIWidget::BondGraphUIWidget(QWidget *parent, bool do_backup) :
    QWidget(parent), m_doBackup(do_backup)
{
    setupUi();
}

BondGraphUIWidget::~BondGraphUIWidget()
{
}

QSettings &BondGraphUIWidget::getApplicationSettings() const
{
    // static QSettings settings(QCoreApplication::organizationName(),
    // QCoreApplication::applicationName()); return settings;

    static QSettings *settings =
        new QSettings(QCoreApplication::organizationName(),
                      QCoreApplication::applicationName());
    return *settings;
}

QToolBar *BondGraphUIWidget::getToolBar() const
{
    return m_toolBar;
}

void BondGraphUIWidget::saveSettings(QSettings &settings) const
{
    settings.setValue("geometry", saveGeometry());
    settings.setValue("maximized", isMaximized());

    settings.setValue("lastPath", m_lastPath);
}

QString BondGraphUIWidget::getCurrentFileName() const
{
    return m_currentFileName;
}

QStatusBar *BondGraphUIWidget::statusBar() const
{
    return m_statusBar;
}

void BondGraphUIWidget::readSettings(QSettings &settings)
{
    settings.sync();
    showNormal();

    // window geometry
    const QByteArray geometry =
        settings.value("BondGraphEditorWindow/geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry =
            QApplication::desktop()->availableGeometry(this);
        resize(availableGeometry.width() - 200, availableGeometry.height() - 100);
        move(100, 50);
    } else {
        restoreGeometry(geometry);
    }

    QApplication::processEvents();

    // window state
    if (settings.value("BondGraphEditorWindow/maximized", true).toBool()) {
        //#ifdef Q_OS_WIN32
        showMaximized();
        //#else
        //		showNormal();
        //        QTimer::singleShot(0, this, SLOT(showMaximized()));
        //#endif
    } else
        showNormal();

    // path
    m_lastPath = settings.value("BondGraphEditorWindow/lastPath").toString();
}

QFrame *createSampleFrame()
{
    auto *frame = new QFrame();
    frame->setLayout(new QVBoxLayout());
    auto *ipsumLabel = new QLabel();
    frame->layout()->addWidget(ipsumLabel);
    dynamic_cast<QVBoxLayout *>(frame->layout())->addStretch();
    ipsumLabel->setTextFormat(Qt::TextFormat::RichText);
    ipsumLabel->setWordWrap(true);
    ipsumLabel->setText("This is a sample frame with populated contents");
    return frame;
}

void BondGraphUIWidget::showStatusMessage(const QString &msg, int timeout)
{
    m_statusBar->showMessage(msg, timeout);
}

void BondGraphUIWidget::setCentralWidget(QWidget *main_widget)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(main_widget);
    m_canvasParent->setLayout(layout);
}

void BondGraphUIWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    // Create tool bar
    m_toolBar = new QToolBar(this);
    mainLayout->addWidget(m_toolBar, 0);

    QSplitter *wSplitter = new QSplitter(this);
    m_widgetBox = new WidgetBox(this);
    // Get the elements supported by the bondgraph library
    nlohmann::json support = BG::getSupportedPhysicalDomainsAndFactoryMethods();
    // Set up icons
    nlohmann::json iconMaps;
    nlohmann::json iconfiles;
    iconfiles["Electrical"] = nlohmann::json::parse(
        R"(
        {
          "Resistance": ":/Icons/resistor",
          "Capacitance": ":/Icons/capacitor",
          "Inductance": ":/Icons/inductor",
          "PotentialSource": ":/Icons/effortsource",
          "FlowSource": ":/Icons/flowsource"
        }
        )");
    iconfiles["Generic"] = nlohmann::json::parse(
        R"(
      {
        "Resistance": ":/Icons/resistor",
        "Capacitance": ":/Icons/capacitor",
        "Inductance": ":/Icons/inductor",
        "PotentialSource": ":/Icons/effortsource",
        "FlowSource": ":/Icons/flowsource",
        "Gyrator": ":/Icons/gyrator",
        "Transformer": ":/Icons/transformer",
        "ZeroJunction": ":/Icons/zerojunction",
        "OneJunction": ":/Icons/onejunction"        
      }
      )");

    iconMaps["Icons"] = iconfiles;
    iconMaps["ElementDefinitions"] = nlohmann::json::parse(
        R"(
        {
      "Composition":{
        "location":3,
        "UserDefined":{
          "name":"userdefined",
          "description": "User Defined blackbox or BG",
          "shortname": "ud",
          "variableprefix": "UD",
          "class": "userdefined",
          "domain": "Composition",        
          "ports": {
            "limit": 1
          }
        }
      },
      "Annotation":{
        "location":4,
        "Region":{
          "name":"region",
          "description": "Spatial Annotation",
          "shortname": "Text",
          "variableprefix": "",
          "class": "annotation",
          "domain": "Annotation",        
          "ports": {
            "limit": 0
          }
        }
      }      
    }
    )");

    nlohmann::json &edf = support["ElementDefinitions"];
    if (iconMaps.contains("ElementDefinitions")) {
        nlohmann::json &jdf = iconMaps["ElementDefinitions"];
        for (auto &doj : jdf.items()) {
            // Update
            if (edf.contains(doj.key())) {
                doj.value().update(edf[doj.key()]);
            }
        }
        // Addin elements in support that are not in the iconMap
        for (auto &doj : edf.items()) {
            // Update
            if (!jdf.contains(doj.key())) {
                jdf[doj.key()] = doj.value();
                // Check if icon is available
            }
        }
    } else {
        // Update
        iconMaps["ElementDefinitions"] = edf;
    }
    // Add domain information to each element definition
    nlohmann::json &df = iconMaps["ElementDefinitions"];
    for (auto &ed : df.items()) {
        nlohmann::json &jdf = ed.value();
        std::string key = ed.key();
        for (auto &elem : jdf.items()) {
            nlohmann::json &j = elem.value();
            std::string k = elem.key();
            if (j.is_object() && k != "BondDimensions") {
                j["domain"] = key;
                j["type"] = k;
            }
        }
    }
    QString js = QString::fromStdString(iconMaps.dump());
    m_widgetBox->load(js);

    wSplitter->addWidget(m_widgetBox);

    QSplitter *phsSplitter = new QSplitter(this);
    phsSplitter->setOrientation(Qt::Vertical);
    m_canvasParent = new QWidget(this);
    m_canvasParent->setAcceptDrops(true);
    QSplitter *cSplitter = new QSplitter(this);

    phsSplitter->addWidget(m_canvasParent);
    phsWindow = new BGPorthamiltonianRenderer(this);
    phsSplitter->addWidget(phsWindow);
    phsSplitter->setCollapsible(0,false);
    phsSplitter->setCollapsible(1,true);
    //Enable disable handle depending on result

    // phsSplitter->handle(1)->setEnabled(false);
    // connect(phsWindow,&BGPorthamiltonianRenderer::phsRendered,[&phsSplitter,this](bool success){
    //     // if(!success){
    //     //     QList<int> sz = QList<int>(phsSplitter->sizes());
    //     //     sz[0] += sz[1];
    //     //     sz[1] = 0;
    //     //     phsSplitter->setSizes(sz);
    //     //     auto pw = this->phsWindow->width();
    //     //     this->phsWindow->resize(pw,1);
    //     // }
    //     phsSplitter->handle(1)->setEnabled(success);
    // });

    cSplitter->addWidget(phsSplitter);
    //cSplitter->addWidget(m_canvasParent);
    m_controls = new QAccordion(this);
    // Enable multiple panes to be open simultaneously
    m_controls->setMultiActive(true); 

    auto annotation = new BGAnnotationViewMetadataEditDetailsUI(this);
    m_annotation = annotation; //Maintain a handle, note m_annotation is of type QWidget
    auto controlsTab = new QTabWidget(this);
    controlsTab->addTab(m_controls, "Graphics");
    controlsTab->addTab(m_annotation, "Annotation");

    // Create status bar
    m_statusBar = new QStatusBar(this);

    // Setup the controller
    uiController = new BGElementEditorUIController(this, m_doBackup);
    uiController->setAnnotationEditor(annotation);
    uiController->setPHSRenderer(phsWindow);
    connect(uiController, &BGElementEditorUIController::serializedToCellML,
            [this](QString path) {
                Q_EMIT cellMLFileGenerated(path);
            });
    
    

    // cSplitter->addWidget(m_controls);
    cSplitter->addWidget(controlsTab);
    wSplitter->addWidget(cSplitter);
    wSplitter->setHandleWidth(5);
    // Setup splitter collapsing experience
    {
        auto handle = wSplitter->handle(1);
        auto layout = new QVBoxLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        auto mWidgetBoxButton = new QToolButton(handle);
        mWidgetBoxButton->setArrowType(Qt::RightArrow);
        mWidgetBoxButton->setVisible(false);
        mWidgetBoxButton->setIconSize(QSize(15, 15));
        connect(mWidgetBoxButton, &QToolButton::clicked,
                [wSplitter, mWidgetBoxButton]() {
                    int largeWidth = QGuiApplication::primaryScreen()->size().width();
                    wSplitter->setSizes(QList<int>(
                        { std::min(600, largeWidth / 4),
                          std::max(largeWidth - 600, (3 * largeWidth) / 4) }));
                    mWidgetBoxButton->setVisible(false);
                    wSplitter->setHandleWidth(5);
                });
        connect(wSplitter, &QSplitter::splitterMoved,
                [mWidgetBoxButton, wSplitter](int pos, int index) {
                    Q_UNUSED(index);
                    if (pos < 5) { // When widgetBox is closed
                        mWidgetBoxButton->setVisible(true);
                        wSplitter->setHandleWidth(25);
                    } else {
                        mWidgetBoxButton->setVisible(false);
                    }
                });
    }
    {
        auto handle = cSplitter->handle(1);
        auto layout = new QVBoxLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        auto mControlBoxButton = new QToolButton(handle);
        mControlBoxButton->setArrowType(Qt::LeftArrow);
        mControlBoxButton->setVisible(false);
        mControlBoxButton->setIconSize(QSize(15, 15));
        connect(mControlBoxButton, &QToolButton::clicked,
                [cSplitter, mControlBoxButton]() {
                    int largeWidth = QGuiApplication::primaryScreen()->size().width();
                    cSplitter->setSizes(
                        QList<int>({ (largeWidth * 3) / 4, largeWidth / 4 }));
                    mControlBoxButton->setVisible(false);
                    cSplitter->setHandleWidth(5);
                });
        connect(cSplitter, &QSplitter::splitterMoved,
                [mControlBoxButton, cSplitter](int pos, int index) {
                    Q_UNUSED(pos);
                    Q_UNUSED(index);
                    if (cSplitter->sizes().at(1) < 5) { // When controlBox is closed
                        mControlBoxButton->setVisible(true);
                        cSplitter->setHandleWidth(25);
                    } else {
                        mControlBoxButton->setVisible(false);
                    }
                });
    }
    uiController->doReadSettings(getApplicationSettings());
    int largeWidth = QGuiApplication::primaryScreen()->size().width();
    wSplitter->setSizes(
        QList<int>({ std::min(400, largeWidth / 4),
                     std::max(largeWidth - 400, (3 * largeWidth) / 4) }));

    mainLayout->addWidget(wSplitter, 10);

    mainLayout->addWidget(m_statusBar, 0);
    setLayout(mainLayout);

    resize(1600, 1000);
}

QString BondGraphUIWidget::iconURI(QString domain, QString element)
{
    return m_widgetBox->getIconURI(domain, element);
}

void BondGraphUIWidget::addControlPane(QString name, QWidget *widget)
{
    auto *frame = new QFrame();
    frame->setLayout(new QVBoxLayout());
    frame->layout()->addWidget(widget);
    dynamic_cast<QVBoxLayout *>(frame->layout())->addStretch();
    m_controls->addContentPane(name, frame);
}

void BondGraphUIWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (m_initialShowEvent) { // check for backups from previous crash, if there
                              // is one offer to load
        m_initialShowEvent = false;
        uiController->reloadBackup();
    }
}

void BondGraphUIWidget::retranslateUi()
{
}

void BondGraphUIWidget::closeEvent(QCloseEvent *event)
{
    // Remove backup file
    uiController->removeBackUp();
    // then call parent's procedure
    QWidget::closeEvent(event);
}

void BondGraphUIWidget::saveFile(const QString &p_old_file_name,
                                 const QString &p_new_file_name,
                                 bool &p_need_feedback)
{
    Q_UNUSED(p_old_file_name);
    Q_UNUSED(p_new_file_name);
    Q_UNUSED(p_need_feedback);
    // Ask user to us
    uiController->exportFile();
}
void BondGraphUIWidget::cut()
{
    uiController->cut();
}
void BondGraphUIWidget::copy()
{
    uiController->copy();
}
void BondGraphUIWidget::paste()
{
    uiController->paste();
}
void BondGraphUIWidget::del()
{
    uiController->del();
}
void BondGraphUIWidget::undo()
{
    uiController->undo();
}
void BondGraphUIWidget::redo()
{
    uiController->redo();
}

} // namespace BondGraphEditorWindow
} // namespace OpenCOR
