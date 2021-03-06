#include "kernel.h"

#include "uicontextmodel.h"

quint64 UIContextModel::_objectCounter = 0;
std::unique_ptr<UIContextModel> UIContextModel::_uicontext;

UIContextModel::UIContextModel(QObject *parent) :
    QObject(parent)
{
}

VisualizationManager *UIContextModel::createVisualizationManager(const QString& objectName)
{
    QObject *object =_qmlcontext->findChild<QObject *>(objectName);
    QObject *newparent = object == 0 ? this : object;
    VisualizationManager *manager =  new VisualizationManager(newparent, this);

    return manager;
}

QString UIContextModel::uniqueName()
{
    return "ilwis_ui_object_" + QString::number(_objectCounter++);
}

QList<PropertyEditor *> UIContextModel::propertyEditors(quint64 objecttype)
{
    QList<PropertyEditor *> editors;
    for ( const auto& ilwtype : _propertyEditors){
        if ( hasType(ilwtype.first, objecttype)){
            for(auto editor : _propertyEditors[ilwtype.first]) {
                editors.append(editor.second());
            }
        }
    }
    return editors;
}

void UIContextModel::qmlContext(QQmlContext *ctx)
{
    _qmlcontext = ctx;
}

int UIContextModel::activeSplit() const
{
    return _activeSplit;
}

void UIContextModel::setActiveSplit(int index)
{
    if ( index == 1 || index == 2){
        _activeSplit = index;
    }
}

int UIContextModel::addPropertyEditor(quint64 objecttype, const QString &propertyName, CreatePropertyEditor func)
{
    _propertyEditors[objecttype][propertyName] = func;

    return 0;
}





