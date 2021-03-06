#include "kernel.h"
#include "errorobject.h"
#include "complexdrawer.h"

using namespace Ilwis;
using namespace Geodrawer;

ComplexDrawer::ComplexDrawer(const QString &name, DrawerInterface* parentDrawer, RootDrawer *rootdrawer, QObject *parent) : BaseDrawer(name, parentDrawer, rootdrawer, parent)
{

}

bool ComplexDrawer::draw(QOpenGLContext *openglContext, const IOOptions &options)
{
    if (!isActive() || !isValid())
        return false;

    bool ok = drawSideDrawers(openglContext, _preDrawers, options)    ;

    for(const auto& drawer : _mainDrawers){
        if ( drawer){
            ok &= drawer->draw(openglContext, options);
        }
    }
    ok &= drawSideDrawers(openglContext, _postDrawers, options);

    return ok;
}

bool ComplexDrawer::prepare(DrawerInterface::PreparationType prepType, const IOOptions &options, QOpenGLContext *openglContext)
{
    for( auto& drawer : _preDrawers)    {
        if (!drawer.second->prepare(prepType, options,openglContext))
            return false;
    }
    for( auto& drawer : _mainDrawers){
        if (!drawer->prepare(prepType, options, openglContext))
            return false;
    }
    for( auto& drawer : _postDrawers)    {
        if (!drawer.second->prepare(prepType, options, openglContext))
            return false;
    }
    return true;

}

void ComplexDrawer::unprepare(DrawerInterface::PreparationType prepType)
{
    for( auto& drawer : _preDrawers)    {
        drawer.second->unprepare(prepType);
    }
    for( auto& drawer : _mainDrawers){
        drawer->unprepare(prepType);
    }
    for( auto& drawer : _postDrawers)    {
        drawer.second->unprepare(prepType);
    }
}

bool ComplexDrawer::prepareChildDrawers(DrawerInterface::PreparationType prepType, const IOOptions &options)
{
    for(auto& drawer : _mainDrawers){
        if ( drawer && drawer->isValid())
            drawer->prepare(prepType, options);
    }
    return true;
}

quint32 ComplexDrawer::drawerCount(ComplexDrawer::DrawerType tpe) const
{
    switch(tpe){
    case dtPRE:
        return _postDrawers.size();
    case dtMAIN:
        return _mainDrawers.size();
    case dtPOST:
        return _postDrawers.size();
    default:
        return _postDrawers.size() + _preDrawers.size() + _mainDrawers.size();;
    }
    return iUNDEF;
}

const UPDrawer &ComplexDrawer::drawer(quint32 order, ComplexDrawer::DrawerType drawerType) const
{
    if ( drawerType == dtPOST || drawerType == dtPRE){
        const DrawerMap& drawers = drawerType == dtPRE ? _preDrawers : _postDrawers;
        const auto& current = drawers.find(order);
        if ( current == drawers.end())
            throw VisualizationError(TR("Invalid drawer number used while drawing"));

        return current->second;
    } else if ( drawerType == dtMAIN){
        if ( order < _mainDrawers.size()){
            return _mainDrawers[order];
        }
    }
    throw VisualizationError(TR(QString("Drawer number %1 is not valid").arg(order)));
}

void ComplexDrawer::addDrawer(DrawerInterface *drawer, ComplexDrawer::DrawerType drawerType, quint32 order)
{
    if ( drawerType == dtPOST || drawerType == dtPRE){
        if ( order != iUNDEF){
            DrawerMap& drawers = drawerType == dtPRE ? _preDrawers : _postDrawers;
            drawers[order].reset(drawer);
        }
    }else if ( drawerType == dtMAIN){
        UPDrawer drawerp(drawer);
        _mainDrawers.push_back(std::move(drawerp));
    }
}

void ComplexDrawer::setDrawer(quint32 order, DrawerInterface *drawer, ComplexDrawer::DrawerType drawerType)
{
    if ( drawerType == dtPOST || drawerType == dtPRE){
        DrawerMap& drawers = drawerType == dtPRE ? _preDrawers : _postDrawers;
        const auto& current = drawers.find(order);
        if ( current == drawers.end()){
            return;
        }
        drawers[order].reset(drawer);
    } else if ( drawerType == dtMAIN){
        if ( order < _mainDrawers.size()) {
            _mainDrawers[order].reset(drawer);
        }
    }
}

void ComplexDrawer::removeDrawer(quint32 order, ComplexDrawer::DrawerType drawerType)
{
    if ( drawerType == dtPOST || drawerType == dtPRE){
        DrawerMap& drawers = drawerType == dtPRE ? _preDrawers : _postDrawers;
        auto current = drawers.find(order);
        if ( current != drawers.end()){
            drawers.erase(current)    ;

        }
    } else if ( drawerType == dtMAIN){
        if ( order < _mainDrawers.size()) {
           _mainDrawers.erase(_mainDrawers.begin() + order);
        }
    }
}

bool ComplexDrawer::isSimple() const
{
    return false;
}

void ComplexDrawer::cleanUp()
{
    for( auto& drawer : _preDrawers)    {
        drawer.second->cleanUp();
    }
    for( auto& drawer : _mainDrawers){
        drawer->cleanUp();
    }
    for( auto& drawer : _postDrawers)    {
        drawer.second->cleanUp();
    }

}

std::vector<VertexPosition> &ComplexDrawer::drawPositions()
{
    return _positions;
}

bool ComplexDrawer::drawSideDrawers(QOpenGLContext *openglContext, const DrawerMap& drawers, const IOOptions &options) const
{
    if (!isActive())
        return false;

    if ( drawers.size() > 0) {
        for(const auto& current : drawers) {
            const auto& drw = current.second;
            if ( drw)
                drw->draw(openglContext, options);
        }
    }
    return true;
}

std::vector<QVariant> ComplexDrawer::attributes(const QString &attrNames) const
{
    std::vector<QVariant> results = BaseDrawer::attributes(attrNames);
    QStringList parts = attrNames.split("|");
    for(QString& part : parts)
        part.toLower();

    for(const QString& part : parts){
        results.push_back(attribute(part));
    }
    return results;
}

QVariant ComplexDrawer::attribute(const QString &attrNme) const
{
    QString attrName = attrNme.toLower();
    QVariant var = BaseDrawer::attribute(attrName);
    if ( var.isValid())
        return var;

    if ( attrName == "maindrawercount")
        return drawerCount(ComplexDrawer::dtMAIN);
    if ( attrName == "predrawercount")
        return drawerCount(ComplexDrawer::dtPRE);
    if ( attrName == "postdrawercount")
        return drawerCount(ComplexDrawer::dtPOST);

    return QVariant();
}
