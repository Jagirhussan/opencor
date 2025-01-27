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
#include "bgannotationmiriamtermsui.h"

#include <QAbstractItemModel>
#include <QApplication>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkInterface>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QStandardItemModel>
#include <QVariant>
#include <QtNetwork>

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVBoxLayout>

#include "bgannotationviewmetadataeditdetailsui.h"
#include "miriamtermsjsonstring.h"
#include <sstream>

namespace OpenCOR {
namespace BondGraphEditorWindow {

DownloadManager::DownloadManager()
{
    connect(&mManager, &QNetworkAccessManager::finished, this,
            &DownloadManager::downloadFinished);
}

void DownloadManager::doDownload(const QUrl &url)
{
    QNetworkRequest request(url);
    QNetworkReply *reply = mManager.get(request);

#if QT_CONFIG(ssl)
    connect(reply, &QNetworkReply::sslErrors, this, &DownloadManager::sslErrors);
#endif

    mCurrentDownloads = reply;
}

bool DownloadManager::isHttpRedirect(QNetworkReply *reply)
{
    int statusCode =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return statusCode == 301 || statusCode == 302 || statusCode == 303 || statusCode == 305 || statusCode == 307 || statusCode == 308;
}

void DownloadManager::sslErrors(const QList<QSslError> &ssl_error)
{
#if QT_CONFIG(ssl)
    mSslErrors = ssl_error;
#else
    Q_UNUSED(sslError);
#endif
}

void DownloadManager::downloadFinished(QNetworkReply *reply)
{
    QUrl url = reply->url();
    if (reply->error()) {
        Q_EMIT downloadFailed(url, reply->errorString());
    } else {
        Q_EMIT downloadCompleted(url, reply->readAll());
    }

    mCurrentDownloads = nullptr;
    reply->deleteLater();
}

BGAnnotationMiriamtermsModel::BGAnnotationMiriamtermsModel(
    QStandardItemModel *model, QObject *parent) :
    QAbstractTableModel(parent),
    model(model)
{
}

int BGAnnotationMiriamtermsModel::rowCount(
    const QModelIndex & /*parent*/) const
{
    return model->rowCount();
}

int BGAnnotationMiriamtermsModel::columnCount(
    const QModelIndex & /*parent*/) const
{
    return 2; // Resource name and Id are passed
}

QVariant BGAnnotationMiriamtermsModel::headerData(int section,
                                                  Qt::Orientation orientation,
                                                  int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return QString("Description");
        case 1:
            return QString("FMA ID");
        }
    }
    return QVariant();
}

QVariant BGAnnotationMiriamtermsModel::data(const QModelIndex &index,
                                            int role) const
{
    if (role == Qt::DisplayRole) {
        int col = 0;
        if (index.column() != 0)
            col = 2;
        return model->data(model->index(index.row(), col));
    }

    return QVariant();
}

static const char *pmr2RicordoUrl =
    "https://models.physiomeproject.org/pmr2_ricordo/miriam_terms/";

QString resourceUrl(const QString &p_resource)
{
    // Return the URL for the given resource

    return "https://registry.identifiers.org/registry/" + p_resource;
}

QString idUrl(const QString &p_id)
{
    // Return the URL for the given resource

    return "https://identifiers.org/" + p_id;
}

bool hasInternetConnection()
{
    // Check whether an Internet connection is available, this by going through
    // all of our network interfaces and checking whether one of them is both
    // active and is not a loopback, and has at least one IPv4 IP address

    const QList<QNetworkInterface> networkInterfaces =
        QNetworkInterface::allInterfaces();

    for (const auto &networkInterface : networkInterfaces) {
        QNetworkInterface::InterfaceFlags interfaceFlags = networkInterface.flags();

        if (interfaceFlags.testFlag(QNetworkInterface::IsUp) && !interfaceFlags.testFlag(QNetworkInterface::IsLoopBack)) {
            const QList<QNetworkAddressEntry> addressEntries =
                networkInterface.addressEntries();

            for (const auto &addressEntry : addressEntries) {
                QAbstractSocket::NetworkLayerProtocol protocol =
                    addressEntry.ip().protocol();

                if ((protocol == QAbstractSocket::IPv4Protocol) || (protocol == QAbstractSocket::AnyIPProtocol)) {
                    return true;
                }
            }
        }
    }

    return false;
}

QString stringFromPercentEncoding(const QString &p_string)
{
    // Remove the percent encoding from the given string

    return QUrl::fromPercentEncoding(p_string.toUtf8());
}

bool decodeTerm(const QString &p_term, QString &p_resource, QString &p_Id)
{
    // Decode the term, based on whether it matches that of a MIRIAN URN or an
    // identifiers.org URI
    static const auto resourceRegEx =
        QStringLiteral("[0-9a-z]+((-|\\.)[0-9a-z]+)?");
    static const auto idRegEx =
        QStringLiteral("[0-9A-Za-z\\.%-_:]+(:[0-9A-Za-z\\.%-_:]+)?");
    static const QRegularExpression miriamUrnRegEx =
        QRegularExpression("^urn:miriam:" + resourceRegEx + ":" + idRegEx);
    static const QRegularExpression identifierDotOrgRegEx = QRegularExpression(
        "^http://identifiers.org/" + resourceRegEx + "/#?" + idRegEx);

    if (miriamUrnRegEx.match(p_term).hasMatch()) {
        // The term is a MIRIAM URN, so retrieve its corresponding resource and
        // id

        QStringList miriamUrnList = p_term.split(':');

        p_resource = stringFromPercentEncoding(miriamUrnList[2]);
        p_Id = stringFromPercentEncoding(miriamUrnList[3]);

        return true;
    }

    if (identifierDotOrgRegEx.match(p_term).hasMatch()) {
        // The term is an identifiers.org URI, so retrieve its corresponding
        // resource and id

        QString identifiersDotOrgUri = p_term;
        // Note: the above is because pTerm is a const, so we can't directly use
        //       QString::remove() on it...
        QStringList identifiersDotOrgUriList =
            identifiersDotOrgUri.remove("http://identifiers.org/").split('/');

        p_resource = stringFromPercentEncoding(identifiersDotOrgUriList[0]);
        p_Id = stringFromPercentEncoding(identifiersDotOrgUriList[1]);

        return true;
    }

    // Not a term we can recognise

    p_resource = "???";
    p_Id = "???";

    return false;
}

void addTerm(QAbstractItemModel *model, const QString &name,
             const QString &term)
{
    QString resource;
    QString id;
    if (decodeTerm(term, resource, id)) {
        model->insertRow(0);
        model->setData(model->index(0, 0), name);
        model->setData(model->index(0, 1), resource);
        model->setData(model->index(0, 2), id);
        model->setData(model->index(0, 3), term);
    }
}

BGAnnotationMiriamtermsModel *miriamDataModel(QObject *parent, QString result)
{
    QStandardItemModel *model = new QStandardItemModel(0, 4, parent);
    // Resource, Name, Id
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Name"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Resource"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Id"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("url"));

    QJsonParseError jsonParseError;
    QJsonDocument jsonDocument =
        QJsonDocument::fromJson(result.toUtf8(), &jsonParseError);

    if (jsonParseError.error == QJsonParseError::NoError) {
        // Retrieve the list of terms

        QVariantMap termMap;
        QString name = "";
        QString resource = "";
        QString id = "";

        for (const auto &terms :
             jsonDocument.object().toVariantMap()["results"].toList()) {
            termMap = terms.toMap();
            name = termMap["name"].toString();

            if (!name.isEmpty()) {
                // We have a name and we could decode the term, so add
                // the item to our list
                addTerm(model, name, termMap["identifiers_org_uri"].toString());
            }
        }
    } else {
        throw jsonParseError.errorString();
    }

    return new BGAnnotationMiriamtermsModel(model, parent);
}

BGAnnotationMiriamtermsUI::BGAnnotationMiriamtermsUI(QLineEdit *st,
                                                     QTableView *it,
                                                     QComboBox *rel,
                                                     QWidget *p_parent) :
    QObject(p_parent),
    m_SearchTerm(st), m_ItemsTable(it), m_Relationships(rel)
{
    auto *downloader = new DownloadManager();
    m_Proxy = new QSortFilterProxyModel(this);
    m_Empty = new QSortFilterProxyModel(this);
    m_ItemsTable->setModel(m_Empty);
    m_ItemsTable->setSortingEnabled(true);
    m_ItemsTable->setEnabled(false);
    m_Relationships->setEnabled(false);
    m_ItemsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    // Add the supported Annotations
    m_Relationships->addItem("");
    for (auto anot :
         BGAnnotationViewMetadataEditDetailsUI::m_supportedAnnotations_static) {
        m_Relationships->addItem(anot);
    }
    // searchTerm->setEnabled(false);
    connect(m_SearchTerm, SIGNAL(textChanged(QString)), this,
            SLOT(filterText(QString)));
    connect(m_ItemsTable, SIGNAL(clicked(const QModelIndex &)), this,
            SLOT(cellClicked(const QModelIndex &)));
    connect(m_Relationships, SIGNAL(currentIndexChanged(int)), this,
            SLOT(assignRelationship(int)));

    if (hasInternetConnection()) {
        connect(downloader, &DownloadManager::downloadCompleted,
                [this](QUrl &url, QString result) {
                    Q_UNUSED(url);
                    try {
                        m_MiriamTerms = miriamDataModel(this, result);
                        m_Proxy->setSourceModel(m_MiriamTerms);
                        m_Proxy->setFilterKeyColumn(0);
                    } catch (std::exception &e) {
                        m_qError = e.what();
                    }
                    Q_EMIT dataLoaded();
                    m_ItemsTable->setEnabled(true);
                    m_SearchTerm->setEnabled(true);
                });
        connect(downloader, &DownloadManager::downloadFailed,
                [this](QUrl &url, QString result) {
                    Q_UNUSED(url);
                    m_qError = result;
                    try {
                        loadMiriamTermsFromResources();
                        m_Proxy->setSourceModel(m_MiriamTerms);
                        m_Proxy->setFilterKeyColumn(0);
                    } catch (std::exception &e) {
                        m_qError = e.what();
                    }
                    Q_EMIT dataLoaded();
                    m_ItemsTable->setEnabled(true);
                    m_SearchTerm->setEnabled(true);
                });

        downloader->doDownload(QUrl(pmr2RicordoUrl));
    } else {
        m_qError = "No internet connection";
        try {
            loadMiriamTermsFromResources();
            m_Proxy->setSourceModel(m_MiriamTerms);
            m_Proxy->setFilterKeyColumn(0);
        } catch (std::exception &e) {
            m_qError = e.what();
        }
        Q_EMIT dataLoaded();
        m_ItemsTable->setEnabled(true);
        m_SearchTerm->setEnabled(true);
    }
}

void BGAnnotationMiriamtermsUI::loadMiriamTermsFromResources()
{
    std::ostringstream ss;
    ss << miriamtermsjsonstring;
    QString mjson = QString::fromStdString(ss.str());
    m_MiriamTerms = miriamDataModel(this, mjson);
}

void BGAnnotationMiriamtermsUI::assignRelationship(int index)
{
    if (index > 0) {
        auto rel = BGAnnotationViewMetadataEditDetailsUI::
                       m_supportedAnnotations_static[index - 1]
                           .toStdString();
        QModelIndexList selection = m_ItemsTable->selectionModel()->selectedRows();
        nlohmann::json relations = nlohmann::json::object();
        relations[rel] = nlohmann::json::array();

        for (int i = 0; i < selection.count(); i++) {
            int ind1x = selection.at(i).row();
            // Get the FMA term and text
            QString fma = m_Proxy->index(ind1x, 1).data().toString();
            QString txt = m_Proxy->index(ind1x, 0).data().toString();
            relations[rel].push_back((fma + " | " + txt).toStdString());
        }
        if (selection.count() > 0)
            Q_EMIT relationshipsAssigned(relations);
        m_Relationships->blockSignals(true);
        m_Relationships->setCurrentIndex(0);
        m_Relationships->blockSignals(false);
    } else {
        // Do not assign
    }
}

void BGAnnotationMiriamtermsUI::cellClicked(const QModelIndex &index)
{
    if (index.isValid()) {
        m_Relationships->setEnabled(true);
    }
}

void BGAnnotationMiriamtermsUI::filterText(QString term)
{
    if (term.length() > 0) {
        m_ItemsTable->setModel(m_Proxy);
        m_Proxy->setFilterFixedString(term);
    } else {
        m_Relationships->setEnabled(false);
        m_ItemsTable->setModel(m_Empty);
    }
}

void BGAnnotationMiriamtermsUI::termDeSelected(QString term)
{
    Q_UNUSED(term);
}

void BGAnnotationMiriamtermsUI::reset()
{
}

void BGAnnotationMiriamtermsUI::selectTerms(const QList<QString> &terms)
{
    Q_UNUSED(terms);
}

} // namespace BondGraphEditorWindow
} // namespace OpenCOR