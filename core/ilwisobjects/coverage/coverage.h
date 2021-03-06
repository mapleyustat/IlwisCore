#ifndef BASELAYER_H
#define BASELAYER_H

#include "kernel.h"
#include "kernel_global.h"
#include "geos/geom/Geometry.h"
#include "geometries.h"
#include "ilwisobject.h"
#include "ilwisdata.h"
#include "domain.h"
#include "datadefinition.h"
#include "columndefinition.h"
#include "attributedefinition.h"
#include "coordinatesystem.h"
#include "connectorinterface.h"
#include "containerstatistics.h"
#include "commandhandler.h"

namespace Ilwis {

class Table;
class ItemRange;

typedef IlwisData<Table> ITable;
/*!
 * \brief The Coverage class
 *
 * In Ilwis we have two kinds of coverages. Raster coverages for imagery and feature coverages for vector data.
 * A coverage controls the validity and meaning of data. A resource can be mapped to a coverage, and by this way be accessed
 */
class KERNELSHARED_EXPORT Coverage : public IlwisObject
{

public:
    /*!
     * The constructor for an empty coverage
     */
    Coverage();

    /*!
     * The constructor for a RasterCoverage with a a Resource that is registered in the mastercatalog.<br>
     * This resource must have a name and an Id.<br>
     * Code and description are optional.
     *
     * \sa IlwisObject
     * \sa Resource
     * \param source Resource that has to be used
     */
    Coverage(const Resource& source);

    virtual ~Coverage();

    /*!
     * Query for the coordinate system of this coverage
     *
     * \sa ICoordinateSystem
     * \return the coordinate system if it has one or else null
     */
    ICoordinateSystem coordinateSystem() const;

    /*!
     * Changes the coordinate system of this coverage into the one specified<br>
     * does not work if this coverage is readonly<br>
     * sets the changed flag
     *
     * \sa ICoordinateSystem
     * \param the new coordinate system
     */
    void coordinateSystem(const ICoordinateSystem& csy);

    /*!
     * Query for the envelope of this coverage, must fit in the coordinate system<br>
     * a envelope decides what domain within a coordinate system is valid, or in this case,<br>
     * which part of the coordinate system is covered by this coverage<br>
     * do note that the envelope should not be bigger than the coordinateSystem
     *
     * \return the envelope of this coverage if it has one or else null
     */
    Envelope envelope(bool tolatlon=false) const;

    /*!
     * Changes the envelope of this coverage to the one specified<br>
     * a envelope decides what domain within a coordinate system is valid, or in this case,<br>
     * which part of the coordinate system is covered by this coverage<br>
     * do note that the envelope should not be bigger than the coordinateSystem<br>
     * sets the changed flag
     *
     * \param the new envelope
     */
    void envelope(const Envelope &bnds);

    /*!
     * \brief Sets a new Coveragetable or Indextable
     *
     * When called without parameter, or with atCOVERAGE as parameter you will replace the Coveragetable<br>
     * When called with atINDEX as parameter it will replace the Indextable
     *
     * \param tbl the new table
     * \param attType the type of the new table
     */

    virtual NumericStatistics& statistics(int mode=0);


      //@override
    Resource source(int mode=cmINPUT) const;

    /**
     * Changes the name of this coverage<br>
     * only works if this coverage is not readonly<br>
     * will also adjust the name of the attribute and or indextable<br>
     * to respectively "name"_attributes and "name"_indexattributes
     *
     * @param nam the new name of this coverage
     */
    void name(const QString &nam);
    QString name() const { return IlwisObject::name(); }

    bool prepare();
protected:
    void copyTo(IlwisObject *obj) ;
private:

    ICoordinateSystem _coordinateSystem;
    Envelope _envelope;
    NumericStatistics _statistics;


};

typedef IlwisData<Coverage> ICoverage;

}

Q_DECLARE_METATYPE(Ilwis::ICoverage)



#endif // BASELAYER_H
