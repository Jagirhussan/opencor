//==============================================================================
// Single cell view information parameters widget
//==============================================================================

#ifndef SINGLECELLVIEWINFORMATIONPARAMETERSWIDGET_H
#define SINGLECELLVIEWINFORMATIONPARAMETERSWIDGET_H

//==============================================================================

#include "commonwidget.h"

//==============================================================================

#include <QStackedWidget>

//==============================================================================

namespace OpenCOR {

//==============================================================================

namespace Core {
    class Property;
    class PropertyEditorWidget;
}   // namespace Core

//==============================================================================

namespace CellMLSupport {
    class CellmlFileRuntime;
    class CellmlFileRuntimeModelParameter;
}   // namespace CellMLSupport

//==============================================================================

namespace SingleCellView {

//==============================================================================

class SingleCellViewSimulationData;

//==============================================================================

class SingleCellViewInformationParametersWidget : public QStackedWidget,
                                                  public Core::CommonWidget
{
    Q_OBJECT

public:
    explicit SingleCellViewInformationParametersWidget(QWidget *pParent = 0);

    virtual void retranslateUi();

    virtual void loadSettings(QSettings *pSettings);
    virtual void saveSettings(QSettings *pSettings) const;

    void initialize(const QString &pFileName,
                    CellMLSupport::CellmlFileRuntime *pRuntime,
                    SingleCellViewSimulationData *pSimulationData);
    void finalize(const QString &pFileName);

    void finishPropertyEditing();

private:
    QMap<QString, Core::PropertyEditorWidget *> mPropertyEditors;

    QMap<Core::Property *, CellMLSupport::CellmlFileRuntimeModelParameter *> mModelParameters;

    QList<int> mColumnWidths;

    SingleCellViewSimulationData *mSimulationData;

    void populateModel(Core::PropertyEditorWidget *pPropertyEditor,
                       CellMLSupport::CellmlFileRuntime *pRuntime);

    void updateModelParametersToolTips();

public Q_SLOTS:
    void updateParameters();

private Q_SLOTS:
    void propertyEditorContextMenu(const QPoint &pPosition) const;

    void propertyEditorSectionResized(const int &pLogicalIndex,
                                      const int &pOldSize, const int &pNewSize);

    void propertyChanged(Core::Property *pProperty);
};

//==============================================================================

}   // namespace SingleCellView
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
