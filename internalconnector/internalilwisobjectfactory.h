#ifndef INTERNALOBJECTFACTORY_H
#define INTERNALOBJECTFACTORY_H

namespace Ilwis {

class Coverage;
class GeodeticDatum;

namespace Internal {
/*!
 \brief factory used to create IlwisObjects that do not come from an external source

 There are a number of IlwisObject that are based on internal definitions. Many objects in the realm of
 spatial reference system are based on known defaults and thus are generated from a set of internal tables.
Furthermore there are a number of objects that are default to the system (e.g. domain Image). They are
also generated by the internal object factories.

*/
class InternalIlwisObjectFactory : public IlwisObjectFactory
{
public:
    InternalIlwisObjectFactory();

    virtual Ilwis::IlwisObject *create(const Resource & item) const;
    virtual IlwisObject *create(IlwisTypes type, const QString &subtype=sUNDEF) const;
    bool canUse(const Ilwis::Resource &item) const;

private:
    Resource property2Resource(const QVariant &property, IlwisTypes type) const;
    IlwisObject *createEllipsoid(const Resource &item) const;
    GeodeticDatum *createDatum(const Resource &item) const;
    IlwisObject *createProjection(const Resource &item) const;
    IlwisObject *createEllipsoidFromQuery(const QString &query, const Resource& resource) const;
    IlwisObject *createDomain(const Resource &item) const;
    IlwisObject *createCsyFromCode(const Resource &item) const;
    IlwisObject *createGridCoverage(const Resource &item) const;
    IlwisObject *createTable(const Resource &item) const;
    IlwisObject *createOperationMetaData(const Resource &item) const;
    IlwisObject *createGeoreference(const Resource &item) const;
    bool createCoverage(const Ilwis::Resource &item, Coverage *coverage) const;
    IlwisObject *createFeatureCoverage(const Resource &item) const;
};
}
}

#endif // INTERNALOBJECTFACTORY_H
