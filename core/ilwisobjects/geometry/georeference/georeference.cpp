#include <QString>

#include "kernel.h"
#include "ilwis.h"
#include "geometries.h"
#include "ilwisdata.h"
#include "coordinatesystem.h"
#include "georeference.h"
#include "georefimplementation.h"
#include "factory.h"
#include "abstractfactory.h"
#include "georefimplementationfactory.h"

using namespace Ilwis;



GeoReference::GeoReference()
{
}

GeoReference::GeoReference(const Resource& resource) : IlwisObject(resource)
{
}

GeoReference::~GeoReference()
{
//    if ( coordinateSystem().isValid())
//        qDebug() << "use count " << name() << " : " <<  mastercatalog()->usecount(coordinateSystem()->id());

}

GeoReference  *GeoReference::create(const QString& type,const Resource& resource) {
    GeoReference *georef = new GeoReference(resource);
    GeoRefImplementationFactory *grfFac = kernel()->factory<GeoRefImplementationFactory>("ilwis::georefimplementationfactory");
    GeoRefImplementation *impl = grfFac->create(type);
    if ( !impl) {
        ERROR1(ERR_COULDNT_CREATE_OBJECT_FOR_1,type);
        return 0;
    }
    georef->impl(impl);
    return georef;
}

Coordinate GeoReference::pixel2Coord(const Pixeld &pixel) const
{
    return _georefImpl->pixel2Coord(pixel);
}

Pixeld GeoReference::coord2Pixel(const Coordinate &crd) const
{
    // for performance reasons no isValid check here, haas tobe checked before hand
    return _georefImpl->coord2Pixel(crd);
}

double GeoReference::pixelSize() const
{
    // for performance reasons no isValid check here, haas tobe checked before hand
    return _georefImpl->pixelSize();
}

bool GeoReference::compute()
{
    if ( isValid())
        return _georefImpl->compute();
    return false;
}

ICoordinateSystem GeoReference::coordinateSystem() const
{
    if ( isValid())
        return _georefImpl->coordinateSystem();
    return ICoordinateSystem();
}

 void GeoReference::coordinateSystem(const ICoordinateSystem& csy)
{
     if ( isValid())
        _georefImpl->coordinateSystem(csy);

 }


Size<> GeoReference::size() const
{
    if ( isValid())
        return _georefImpl->size();
    return Size<>();

}

void GeoReference::size(const Size<> &sz)
{
    // size must always be positive or undefined
    if ( isValid() && sz.xsize() > 0 && sz.ysize() > 0)
        _georefImpl->size(sz);
}

bool GeoReference::centerOfPixel() const
{
    if ( isValid())
        return _georefImpl->centerOfPixel();
    return false;
}

void GeoReference::centerOfPixel(bool yesno)
{
    if ( isValid())
        _georefImpl->centerOfPixel(yesno);
}

bool GeoReference::isCompatible(const IGeoReference &georefOther) const
{
    if (!georefOther.isValid())
        return false;

    if (georefOther->id() == id())
        return true;

    if ( coordinateSystem() != georefOther->coordinateSystem())
        return false;

    if ( isValid())
        return _georefImpl->isCompatible(georefOther);

    return false;
}

void GeoReference::adapter(GeoRefAdapter *adapt)
{
    _adapter.reset(adapt);
}

const QScopedPointer<Ilwis::GeoRefAdapter> &GeoReference::adapter() const
{
    return _adapter;
}

Envelope GeoReference::pixel2Coord(const BoundingBox &box) const
{
    if ( !box.isValid()) {
        ERROR2(ERR_INVALID_PROPERTY_FOR_2,"size", "box");
        return Envelope();
    }
    Coordinate c1 = pixel2Coord(box.min_corner());
    Coordinate c2 = pixel2Coord(box.max_corner() + Pixel(1,1));
    return Envelope(c1,c2);
}

BoundingBox GeoReference::coord2Pixel(const Envelope &box) const
{
    if ( !box.isValid()) {
        ERROR2(ERR_INVALID_PROPERTY_FOR_2,"size", "box");
        return  BoundingBox();
    }
    Pixel p1 = coord2Pixel(box.min_corner());
    Pixel p2 = coord2Pixel(box.max_corner());
    return BoundingBox(p1,p2);
}

bool GeoReference::isValid() const
{
    return !_georefImpl.isNull();
}

void GeoReference::impl(GeoRefImplementation *impl)
{
    _georefImpl.reset(impl);
}

IlwisTypes GeoReference::ilwisType() const
{
    return itGEOREF;
}




