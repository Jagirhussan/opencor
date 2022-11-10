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

#include "bgporthamiltonianrenderer.h"
#include "thirdparty/qtaccordian/qaccordion.h"
#include <QDateTime>
#include <QDir>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QTableWidget>
#include <QScrollArea>
#include <QTimer>

#include <algorithm>
#include <fstream>
#include <sstream>


#include "bgprojectdetails.h"
#include "thirdparty/jkqtlatex/jkqtmathtext.h"
#include <QDebug>

namespace OpenCOR {
namespace BondGraphEditorWindow {

static QPixmap getPixMapForLatex(std::string str) {
  JKQTMathText mathText;
  mathText.useXITS();
  mathText.setFontSize(12);

  mathText.parse(str.c_str());

  // 3. here we do the painting
  QPainter painter;
  QSizeF lsize = mathText.getSize(painter);
  QPixmap pix(lsize.width(), lsize.height());
  pix.fill(Qt::transparent);
  painter.begin(&pix);
  mathText.draw(painter, Qt::AlignCenter,
                QRectF(-5, 0, pix.width(), pix.height()), false);
  painter.end();

  return pix;
}

static std::string getCorrectLatex(std::string sym) {
  // handles double subscripts
  std::string::difference_type n = std::count(sym.begin(), sym.end(), '_');
  if (n > 1) {
    // Find the first occurance
    size_t found = sym.find("_");
    std::string prefix = sym.substr(0, found);
    std::string suffix = sym.substr(found);
    suffix.erase(std::remove(suffix.begin(), suffix.end(), '_'), suffix.end());
    return prefix + "_{" + suffix + "}";
  }
  return sym;
}

static std::tuple<QPixmap, std::string> getImplicitDS(nlohmann::json &ds) {
  std::vector<std::string> latex;
  std::vector<std::string> keys = {"F", "f", "E", "e"};
  std::ostringstream gm;
  for (auto &k : keys) {
    auto jmat = ds[k];
    int rows = jmat["rows"];
    int cols = jmat["cols"];
    std::vector<std::string> elems = jmat["elements"];
    gm.clear();
    gm.str("");

    gm << "\\begin{bmatrix} ";
    int ctr = 0;
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols - 1; j++) {
        gm << getCorrectLatex(elems[ctr++]) << " & ";
      }
      gm << getCorrectLatex(elems[ctr++]);
      if (i < rows - 1)
        gm << " \\\\ ";
    }
    gm << "\\end{bmatrix} ";
    latex.push_back(gm.str());
  }
  gm.clear();
  gm.str("");
  gm << latex[0] << latex[1] << " + " << latex[2] << latex[3] << " = 0 ";

  return std::make_tuple(getPixMapForLatex(gm.str()), gm.str());
}

static std::tuple<QPixmap, std::string> getExplicitDS(nlohmann::json &ds) {
  std::vector<std::string> latex;
  std::vector<std::string> keys = {"lhs", "ds", "rhs"};
  std::ostringstream gm;
  for (auto &k : keys) {
    auto jmat = ds[k];
    int rows = jmat["rows"];
    int cols = jmat["cols"];
    std::vector<std::string> elems = jmat["elements"];
    gm.clear();
    gm.str("");

    gm << "\\begin{bmatrix} ";
    int ctr = 0;
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols - 1; j++) {
        gm << getCorrectLatex(elems[ctr++]) << " & ";
      }
      gm << getCorrectLatex(elems[ctr++]);
      if (i < rows - 1)
        gm << " \\\\ ";
    }
    gm << "\\end{bmatrix} ";
    latex.push_back(gm.str());
  }
  gm.clear();
  gm.str("");
  gm << latex[0] << " = " << latex[1] << latex[2];

  return std::make_tuple(getPixMapForLatex(gm.str()), gm.str());
}

static std::tuple<QPixmap, size_t, std::string>
getRenderedMatrix(nlohmann::json &jmat) {
  int rows = jmat["rows"];
  int cols = jmat["cols"];
  std::vector<std::string> elems = jmat["elements"];
  std::ostringstream gm;
  gm << "\\begin{bmatrix} ";
  int ctr = 0;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols - 1; j++) {
      gm << getCorrectLatex(elems[ctr++]) << " & ";
    }
    gm << getCorrectLatex(elems[ctr++]);
    if (i < rows - 1)
      gm << " \\\\ ";
  }
  gm << "\\end{bmatrix}";

  return std::make_tuple(getPixMapForLatex(gm.str()), elems.size(), gm.str());
}

BGPorthamiltonianRenderer::BGPorthamiltonianRenderer(QWidget *parent)
    : QWidget(parent) {
  // Horizontal splitter for rendering latex on left, bond descriptions on the
  // right
  stackedWidget = new QStackedWidget(this);
  auto widgetLayout = new QVBoxLayout();
  widgetLayout->addWidget(stackedWidget);
  setLayout(widgetLayout);
  // Create Infor widget that shows nothing

  {
    QWidget *dummy = new QWidget();
    // Set the background to be white
    // QPalette pal = QPalette();
    // pal.setColor(QPalette::Window, Qt::white);
    // dummy->setAutoFillBackground(true);
    // dummy->setPalette(pal);
    stackedWidget->addWidget(dummy);
  }
  // Main content widget
  QWidget *contentWidget = new QWidget();
  stackedWidget->addWidget(contentWidget);
  auto centralLayout = new QVBoxLayout();
  contentWidget->setLayout(centralLayout);
  QScrollArea* m_area = new QScrollArea();
  latexWidget = new QWidget();
  legendWidget = new QWidget();
  latex = new QVBoxLayout(latexWidget);
  // Set the background to be white
  QPalette pal = QPalette();
  pal.setColor(QPalette::Window, Qt::white);
  latexWidget->setAutoFillBackground(true);
  latexWidget->setPalette(pal);
  latexWidget->setMinimumSize(QSize(1200,600));

  legend = new QVBoxLayout(legendWidget);
  auto hs = new QSplitter();
  m_area->setWidget(latexWidget);
  //hs->addWidget(latexWidget);
  hs->addWidget(m_area);
  hs->addWidget(legendWidget);
  centralLayout->addWidget(hs);
  stackedWidget->setCurrentIndex(0);
}

void BGPorthamiltonianRenderer::clear(){
  //Hide results widget, only show it when it is ready
  stackedWidget->setCurrentIndex(0);  
  qDeleteAll(latexWidget->findChildren<QWidget *>(QString(),
                                                  Qt::FindDirectChildrenOnly));
  qDeleteAll(legendWidget->findChildren<QWidget *>(QString(),
                                                   Qt::FindDirectChildrenOnly));
}

void BGPorthamiltonianRenderer::setPorthamiltonian(nlohmann::json &ph) {
  // Remove all children
  clear();
  if(ph.contains("clear")){
    return;
  }
  std::ostringstream documentString;
  success = false;
  phs = ph;
  int ns = 0;
  int nr = 0;
  int ne = 0;
  if (ph["success"]) {
    // Create the legend part
    auto m_controls = new QAccordion(this);
    m_controls->setMultiActive(
        true); // Enable multiple panes to be open simultaneously

    ns = phs["number of storages"];
    nr = phs["number of resistances"];
    ne = phs["number of sources"];
    documentString << " \\section*{Summary}" << std::endl
                   << "Number of Storage elements    : " << ns << "\\\\"
                   << std::endl
                   << "Number of Dissipative elements: " << nr << "\\\\"
                   << std::endl
                   << "Number of Source elements     : " << ne << "\\\\"
                   << std::endl;

    {
      auto smap = new QFormLayout();
      auto lens = new QLineEdit(QString("%1").arg(ns));
      lens->setReadOnly(true);
      smap->addRow(QString(tr("Storages")), lens);
      auto lenr = new QLineEdit(QString("%1").arg(nr));
      lenr->setReadOnly(true);
      smap->addRow(QString(tr("Resistances")), lenr);
      auto lene = new QLineEdit(QString("%1").arg(ne));
      lene->setReadOnly(true);
      smap->addRow(QString(tr("Sources")), lene);
      auto *frame = new QFrame();
      frame->setLayout(smap);
      m_controls->addContentPane(tr("Summary"), frame);
    }
    {
      QFormLayout *emap = new QFormLayout();
      documentString << "\\section*{Element name to dof id Mapping}"
                     << std::endl
                     << "\\begin{center}\\begin{tabular}{|c | c  |} \\hline "
                     << std::endl
                     << "Element name & Dof ID \\\\ [0.5ex] " << std::endl
                     << "\\hline " << std::endl;
      for (auto &c : phs["Elements"]) {
        int sufix = c["suffix"];
        std::string name = c["name"];
        auto le = new QLineEdit(QString("%1").arg(sufix));
        le->setReadOnly(true);
        emap->addRow(QString::fromStdString(name), le);
        documentString << "$" << getCorrectLatex(name) << "$&" << sufix
                       << "\\\\ \\hline " << std::endl;
      }
      documentString << "\\end{tabular}\\end{center}" << std::endl;

      auto *frame = new QFrame();
      frame->setLayout(emap);
      m_controls->addContentPane(tr("Elements"), frame);
    }

    {
      QTableWidget *bondMap = new QTableWidget();
      bondMap->setColumnCount(3);
      bondMap->verticalHeader()->setVisible(false);
      bondMap->setHorizontalHeaderLabels(
          QStringList() << tr("Bond  ") << tr("Start ") << tr("Finish"));
      std::map<std::string, std::string> bm = phs["BondIdMap"];
      documentString << "\\section*{Bond id to elements name mapping}"
                     << std::endl
                     << "\\begin{center}\\begin{tabular}{|c | c | c|} \\hline "
                     << std::endl
                     << "Bond ID & Start Element & Finish Element \\\\ [0.5ex] "
                     << std::endl
                     << "\\hline " << std::endl;
      for (auto &bnd : bm) {

        auto rc = bondMap->rowCount();
        bondMap->insertRow(rc);
        std::string bname = bnd.first;
        bname.erase(std::remove(bname.begin(), bname.end(), '_'), bname.end());
        auto pitem = new QTableWidgetItem(QString::fromStdString(bname));
        pitem->setTextAlignment(Qt::AlignCenter);
        bondMap->setItem(rc, 0, pitem);
        std::string mp = bnd.second;
        auto delim = mp.find("->");
        std::string start = mp.substr(0, delim);
        std::string finish = mp.substr(delim + 2);

        bondMap->setItem(rc, 1,
                         new QTableWidgetItem(QString::fromStdString(start)));

        bondMap->setItem(rc, 2,
                         new QTableWidgetItem(QString::fromStdString(finish)));

        documentString << "$" << getCorrectLatex(bname) << "$&$"
                       << getCorrectLatex(start) << "$&$"
                       << getCorrectLatex(finish) << "$"
                       << "\\\\ \\hline " << std::endl;
      }
      documentString << "\\end{tabular}\\end{center}" << std::endl;
      bondMap->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
      bondMap->setAlternatingRowColors(true);
      auto *frame = new QFrame();
      frame->setLayout(new QVBoxLayout());
      frame->layout()->addWidget(bondMap);
      dynamic_cast<QVBoxLayout *>(frame->layout())->addStretch();
      m_controls->addContentPane(tr("Bond id map"), frame);
    }

    legend->addWidget(m_controls);
    // Add control to save file
    legend->addItem(
        new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
    legend->insertStretch(-1, 1);
    QPushButton *btn = new QPushButton(tr("Save latex"));
    legend->addWidget(btn);
    btn->setVisible(false);
    connect(btn, &QPushButton::clicked, this,
            &BGPorthamiltonianRenderer::serialisetofile);

    documentString << "\\section*{Derivation}" << std::endl;
    {
      auto impDS = getImplicitDS(phs["ImplicitDS"]);
      QLabel *mat =
          new QLabel(tr("Implicit representation of the Dirac structure:"));
      latex->addWidget(mat);
      QLabel *lab = new QLabel();
      lab->setPixmap(std::get<0>(impDS));
      lab->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
      latex->addWidget(lab);
      documentString << "Implicit representation of the Dirac structure:\\\\ "
                     << std::endl
                     << "\\begin{center}" << std::endl
                     << "$" << std::get<1>(impDS) << "$" << std::endl
                     << "\\end{center}" << std::endl;
    }

    {
      auto expDS = getExplicitDS(phs["ExplicitDS"]);
      QLabel *mat =
          new QLabel(tr("Explicit representation of the Dirac structure:"));
      latex->addWidget(mat);
      QLabel *lab = new QLabel();
      lab->setPixmap(std::get<0>(expDS));
      lab->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
      latex->addWidget(lab);
      documentString << "Explicit representation of the Dirac structure:\\\\ "
                     << std::endl
                     << "\\begin{center}" << std::endl
                     << "$" << std::get<1>(expDS) << "$" << std::endl
                     << "\\end{center}" << std::endl;
    }
    {
      QLabel *mat = new QLabel(tr("State vector (<b>x</b>):"));
      latex->addWidget(mat);
      QLabel *lab = new QLabel();
      auto sv = getRenderedMatrix(phs["stateVector"]);
      lab->setPixmap(std::get<0>(sv));
      lab->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
      latex->addWidget(lab);
      documentString << "State Vector:\\\\" << std::endl
                     << "\\begin{center}" << std::endl
                     << "$ x = " << std::get<2>(sv) << "$" << std::endl
                     << "\\end{center}" << std::endl;
    }
    {
      QLabel *mat = new QLabel(tr("Hamiltonian:"));
      latex->addWidget(mat);
      QLabel *lab = new QLabel();
      std::string hm = phs["hamiltonian"];
      lab->setPixmap(getPixMapForLatex("H = " + hm));
      lab->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
      latex->addWidget(lab);
      // Get the symengine latex version
      hm = phs["hamiltonianLatex"];
      documentString << "Hamiltonian:\\\\" << std::endl
                     << "\\begin{center}" << std::endl
                     << "$H = " << hm << "$\\\\ " << std::endl
                     << "\\end{center}" << std::endl;
    }

    {
      // Input-output PHS form
      std::string phsString =
          "\\begin{align} \\dot{\\mathbold{x}} &= "
          "[J(\\mathbold{x})-R(\\mathbold{x})] \\frac{\\partial "
          "H(\\mathbold{x})}{\\partial\\mathbold{x}} + "
          "[G(\\mathbold{x})-P(\\mathbold{x})]\\mathbold{u}   \\\\ "
          "\\mathbold{y} &=  [G(\\mathbold{x})+P(\\mathbold{x})]^{T} "
          "\\frac{\\partial H(\\mathbold{x})}{\\partial\\mathbold{x}} + "
          "[M(\\mathbold{x})+S(\\mathbold{x})]\\mathbold{u} \\end{align}";
      QLabel *mat = new QLabel(
          "Explicit Input (<b>u</b>) Output (<b>y</b>) Port Hamiltonian:");
      latex->addWidget(mat);
      QLabel *lab = new QLabel();
      lab->setPixmap(getPixMapForLatex(phsString));
      lab->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
      latex->addWidget(lab);
      documentString << "\\section*{Explicit Input (u) Output "
                        "(y) Port Hamiltonian}"
                     << std::endl
                     << "\\begin{align*}" << std::endl
                     << "\\dot{x} = "
                     << "[J(x)-R(x)] \\frac{\\partial "
                     << "H(x)}{\\partial x} + "
                     << "[G(x)-P(x)]u   \\\\ " << std::endl
                     << "y =  [G(x)+P(x)]^{T} "
                     << "\\frac{\\partial H(x)}{\\partial x} + "
                     << "[M(x)+S(x)]u " << std::endl
                     << "\\end{align*}" << std::endl;
    }
    {
      QLabel *mat = new QLabel("Matrices:");
      latex->addWidget(mat);

      std::ostringstream topString;
      {
        auto maj = getRenderedMatrix(phs["matJ"]);
        if (std::get<1>(maj)) {
          topString << "J = " << std::get<2>(maj);
        }
        auto mar = getRenderedMatrix(phs["matR"]);
        if (std::get<1>(mar)) {
          if (std::get<1>(maj))
            topString << ", ";
          topString << "R = " << std::get<2>(mar);
        }
        auto mag = getRenderedMatrix(phs["matG"]);
        if (std::get<1>(mag)) {
          if (std::get<1>(mar))
            topString << ", ";
          topString << "G = " << std::get<2>(mag);
        };
      }
      bool matricestextAdded = false;
      if (topString.str().size()) {
        QLabel *lab = new QLabel();
        lab->setPixmap(getPixMapForLatex(topString.str()));
        lab->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        latex->addWidget(lab);
        documentString << "Matrices:\\\\" << std::endl
                       << "\\begin{center}" << std::endl
                       << "$" << topString.str() << "$" << std::endl
                       << "\\end{center}" << std::endl;
        matricestextAdded = true;
      }
      topString.clear();
      topString.str("");

      {
        auto matp = getRenderedMatrix(phs["matP"]);
        if (std::get<1>(matp)) {
          topString << "P = " << std::get<2>(matp);
        }
        auto mam = getRenderedMatrix(phs["matM"]);
        if (std::get<1>(mam)) {
          if (std::get<1>(matp))
            topString << ", ";
          topString << "M = " << std::get<2>(mam);
        }
        auto mas = getRenderedMatrix(phs["matS"]);
        if (std::get<1>(mas)) {
          if (std::get<1>(mam))
            topString << ", ";
          topString << "S = " << std::get<2>(mas);
        };
      }
      if (topString.str().size()) {
        QLabel *lab = new QLabel();
        lab->setPixmap(getPixMapForLatex(topString.str()));
        lab->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        latex->addWidget(lab);
        if (matricestextAdded)
          documentString << "\\\\" << std::endl
                         << "\\begin{center}" << std::endl
                         << "$" << topString.str() << "$" << std::endl
                         << "\\end{center}" << std::endl;
        else
          documentString << "Matrices:\\\\" << std::endl
                         << "\\begin{center}" << std::endl
                         << "$" << topString.str() << "$" << std::endl
                         << "\\end{center}" << std::endl;
      }
    }
    if (phs.contains("portHamiltonianMatrices")) {
      auto phsm = phs["portHamiltonianMatrices"];
      // Get the latex
      std::string matJR = std::get<2>(getRenderedMatrix(phsm["matJR"]));
      std::string matGP = std::get<2>(getRenderedMatrix(phsm["matGP"]));
      std::string matGtPt = std::get<2>(getRenderedMatrix(phsm["matGtPt"]));
      std::string matGt = std::get<2>(getRenderedMatrix(phsm["matGt"]));
      std::string matMS = std::get<2>(getRenderedMatrix(phsm["matMS"]));
      std::string matJ = std::get<2>(getRenderedMatrix(phs["matJ"]));
      std::string matG = std::get<2>(getRenderedMatrix(phs["matG"]));
      std::string matM = std::get<2>(getRenderedMatrix(phs["matM"]));
      std::string vecU = std::get<2>(getRenderedMatrix(phs["vecU"]));
      std::string vecX = std::get<2>(getRenderedMatrix(phs["stateVector"]));
      std::string vecY = std::get<2>(getRenderedMatrix(phs["vecY"]));

      std::ostringstream gm;
      if (ns != 0 && nr != 0 && ne != 0) {
        gm << "\\frac{d}{dt} " << vecX << " = " << matJR
           << "\\frac{\\partial H}{\\partial x} + " << matGP << vecU << "\\\\"
           << std::endl
           << vecY << " = " << matGtPt << "\\frac{\\partial H}{\\partial x} + "
           << matMS << vecU;
      } else if (ns == 0 && nr != 0 && ne != 0) {
        gm << vecY << " = " << matMS << vecU;
      } else if (ns != 0 && nr == 0 && ne != 0) {
        gm << "\\frac{d}{dt} " << vecX << " = " << matJ
           << "\\frac{\\partial H}{\\partial x} + " << matG << vecU << "\\\\"
           << std::endl
           << vecY << " = " << matGt << "\\frac{\\partial H}{\\partial x} + "
           << matM << vecU;
      } else if (ns == 0 && nr == 0 && ne != 0) {
        gm << vecY << " = " << matM << vecU;
      } else if (ns != 0 && nr != 0 && ne == 0) {
        gm << "\\frac{d}{dt} " << vecX << " = " << matJR
           << "\\frac{\\partial H}{\\partial x}";
      } else if (ns != 0 && nr == 0 && ne == 0) {
        gm << "\\frac{d}{dt} " << vecX << " = " << matJ
           << "\\frac{\\partial H}{\\partial x}";
      }
      QLabel *mat = new QLabel(tr("Port Hamiltonian:"));
      latex->addWidget(mat);
      if (gm.str().size()) {
        QLabel *lab = new QLabel();
        lab->setPixmap(getPixMapForLatex(gm.str()));
        lab->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        latex->addWidget(lab);
      } else {
        QLabel *lab = new QLabel(tr("PHS matrices are not available!!"));
        latex->addWidget(lab);
      }
      documentString << "\\section*{Port Hamiltonian}" << std::endl
                     << "\\begin{center}" << std::endl
                     << "$" << gm.str() << "$" << std::endl
                     << "\\end{center}" << std::endl;
    }

    // Tighten the layout, so that the latex items do not expand when the legend
    // items are expanded
    latex->addItem(
        new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
    latex->insertStretch(-1, 1);
    success = true;
    btn->setVisible(true);
    stackedWidget->setCurrentIndex(1);
    Q_EMIT phsRendered(true);
  } else {
    documentString.clear();
    documentString.str("");
    documentString << " Failed to generate latex from the input "<<std::endl;
    documentString << " Reported error "<<std::endl;
    std::string er = ph["error"];
    documentString << er;
    QMessageBox::critical(nullptr,"PHS generation failed",QString::fromStdString(ph["error"]));
    Q_EMIT phsRendered(false);
  }
  latexScript = QString::fromStdString(documentString.str());
  //resize for QScrollArea
//   int adjustedWidth = width();
//     int adjustedHeight = 0;
//     QLayout* currentLayout = layout();
//     for(QObject* obj : this->children())
//     {
//         QWidget* w = qobject_cast<QWidget*>(obj);
//         if(w == nullptr)
//             continue;
//         QSize hint = w->sizeHint();
//         adjustedWidth = std::max(adjustedWidth, hint.width());
//         adjustedHeight += hint.height() + currentLayout->spacing();
//     }
//     adjustedHeight += (latexWidget->sizeHint().height() + currentLayout->spacing());
//     auto msize = QSize(adjustedWidth, adjustedHeight);

// QTimer::singleShot(1, this, [this,&msize](){
//        this->resize(msize);
//     });
}

nlohmann::json BGPorthamiltonianRenderer::getCurrentJson() { return phs; }

bool BGPorthamiltonianRenderer::phsParsedSuccessfully() { return success; }

QString BGPorthamiltonianRenderer::getLatexScript(QString modelName,
                                                QString author, QString date,
                                                QString notes) {
  std::ostringstream documentString;
  documentString << "\\documentclass[12pt]{article}" << std::endl
                 << "\\usepackage{amsmath}" << std::endl
                 << "\\usepackage{graphicx}" << std::endl
                 << "\\usepackage{hyperref}" << std::endl
                 << "\\usepackage[latin1]{inputenc}" << std::endl;
  // Add title, provanance data etc
  if (modelName.size()) {
    documentString << " \\title{Port hamitonian derivation from Bondgraph "
                   << modelName.toStdString() << "}" << std::endl;
  } else {
    documentString << " \\title{Port hamitonian derivation from Bondgraph}"
                   << std::endl;
  }
  if (author.size()) {
    documentString << " \\author{" << author.toStdString() << "}" << std::endl;
  } else {
    documentString << " \\author{OpenCOR}" << std::endl;
  }
  if (date.size()) {
    documentString << " \\date{" << date.toStdString() << "}" << std::endl;
  }

  documentString << "\\begin{document}" << std::endl
                 << "\\maketitle" << std::endl
                 << latexScript.toStdString() << std::endl;
  if (notes.size()) {
    documentString << "\\section*{Notes}" << std::endl
                   << notes.toStdString() << std::endl;
  }
  documentString << "\\end{document}" << std::endl;
  std::string res = documentString.str();
  return QString::fromStdString(res);
}

void BGPorthamiltonianRenderer::serialisetofile() {
  BGProjectDetails *details = new BGProjectDetails(true, nullptr);
  details->setWindowTitle(QObject::tr("Project details"));
  if (details->exec() == QDialog::Accepted) {
    details->setCursor(Qt::WaitCursor);
    std::map<std::string, std::string> importFile;
    try {
      nlohmann::json jres = details->getJson();
      std::string pdir = jres["directory"];
      // Use the projectname from the popup, the user could have changed it
      QString projectName =
          QString::fromStdString(jres["projectname"]).replace(" ", "_");
      QDir projectDirectory(QString::fromStdString(pdir));
      QString serializedFileName =
          projectDirectory.filePath(projectName + ".tex");
      QDateTime date = QDateTime::currentDateTime();
      QString formattedTime = date.toString("dd/MM/yyyy hh:mm");

      QString latex =
          getLatexScript(QString::fromStdString(jres["projectname"]),
                         QString::fromStdString(jres["authors"]), formattedTime,
                         QString::fromStdString(jres["description"]));
      std::ofstream ofs;
      ofs.open(serializedFileName.toStdString());
      ofs << latex.toStdString();
      ofs.close();

    } catch (std::exception &e) {
      details->setCursor(Qt::ArrowCursor);
      QMessageBox::critical(
          nullptr, QObject::tr("Failed"),
          QObject::tr("Failed to generate CellML model for current "
                      "specification!!\n Bondgraph library returned %1")
              .arg(QString::fromStdString(e.what())));
    }
  }
  details->setCursor(Qt::ArrowCursor);
  delete details;
}

BGPorthamiltonianRenderer::~BGPorthamiltonianRenderer() {}

} // namespace BondGraphEditorWindow
} // namespace OpenCOR