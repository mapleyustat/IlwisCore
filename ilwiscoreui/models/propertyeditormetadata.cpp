#include "propertyeditormetadata.h"

PropertyEditorMetaData::PropertyEditorMetaData(QObject *parent) : QObject(parent)
{

}

PropertyEditorMetaData::PropertyEditorMetaData(const QString& name, const QUrl &Url, QObject *parent) : QObject(parent), Ilwis::Identity(name)
{

}

PropertyEditorMetaData::PropertyEditorMetaData(const PropertyEditorMetaData& metadata, QObject *parent) : QObject(parent), Ilwis::Identity(metadata.name())
{
    _qmlUrl = metadata._qmlUrl;
}

QUrl PropertyEditorMetaData::qmlUrl() const
{
    return _qmlUrl;
}

QString PropertyEditorMetaData::editorName() const
{
    return name();
}


