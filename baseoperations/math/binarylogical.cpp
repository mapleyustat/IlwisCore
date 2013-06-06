#include <functional>
#include <future>
#include "kernel.h"
#include "raster.h"
#include "ilwisoperation.h"
#include "pixeliterator.h"
#include "binarylogical.h"

using namespace Ilwis;
using namespace BaseOperations;


OperationImplementation *BinaryLogical::create(quint64 metaid, const Ilwis::OperationExpression &expr)
{
    return new BinaryLogical( metaid, expr);
}

BinaryLogical::BinaryLogical() : _coveragecoverage(false)
{
}

BinaryLogical::BinaryLogical(quint64 metaid,const Ilwis::OperationExpression &expr) : OperationImplementation(metaid, expr) , _coveragecoverage(false)
{
}

bool BinaryLogical::setOutput(ExecutionContext *ctx) {
    if ( ctx) {
        QVariant value;
        value.setValue<IGridCoverage>(_outputGC);

        ctx->_results.push_back(value);
    }
    return _outputGC.isValid();
}

bool BinaryLogical::executeCoverageNumber(ExecutionContext *ctx) {

    auto BinaryLogical = [&](const Box3D<qint32> box ) -> bool {
        PixelIterator iterIn(_inputGC1, box);
        PixelIterator iterOut(_outputGC, Box3D<qint32>(box.size()));

        for_each(iterOut, iterOut.end(), [&](double& v){
            double v_in1 = *iterIn;
            if ( v_in1 != rUNDEF && _number != rUNDEF) {
                switch(_operator) {
                case otAND:
                case otEQ:
                    v = v_in1 == _number; break;
                case otOR:
                    v = ((bool)v_in1) || ((bool)_number); break;
                case otXOR:
                    v = ((bool)v_in1) ^ ((bool)_number); break;
                case otLESS:
                    v = v_in1 < _number; break;
                case otLESSEQ:
                    v = v_in1 <= _number; break;
                case otNEQ:
                    v = v_in1 != _number; break;
                case otGREATER:
                    v = v_in1 > _number; break;
                case otGREATEREQ:
                    v = v_in1 >= _number; break;
                default:
                    v = rUNDEF;
                    return false;
                }
            }
            ++iterIn;
        });
        return true;
    };

    std::vector<Box3D<qint32>> boxes;

    int cores = OperationHelper::subdivideTasks(_outputGC,boxes);

    if ( cores == iUNDEF)
        return false;

    std::vector<std::future<bool>> futures(cores);
    bool res = true;

    for(int i =0; i < cores; ++i) {
        futures[i] = std::async(std::launch::async, BinaryLogical, boxes[i]);
    }

    for(int i =0; i < cores; ++i) {
        res &= futures[i].get();
    }


    return setOutput(ctx);

}

bool BinaryLogical::executeCoverageCoverage(ExecutionContext *ctx) {
    std::function<bool(const Box3D<qint32>&)> binaryLogical = [&](const Box3D<qint32>& box ) -> bool {
        PixelIterator iterIn1(_inputGC1, box);
        PixelIterator iterIn2(_inputGC2, box);
        PixelIterator iterOut(_outputGC, Box3D<qint32>(box.size()));

        double v_in1 = 0;
        double v_in2 = 0;
        for_each(iterOut, iterOut.end(), [&](double& v){
            v_in1 = *iterIn1;
            v_in2 = *iterIn2;
            if ( v_in1 != rUNDEF && v_in2 != rUNDEF) {
                switch(_operator) {
                case otAND:
                case otEQ:
                    v = v_in1 == v_in2; break;
                case otOR:
                    v = ((bool)v_in1) || ((bool)v_in2); break;
                case otXOR:
                    v = ((bool)v_in1) ^ ((bool)v_in2); break;
                case otLESS:
                    v = v_in1 < v_in2; break;
                case otLESSEQ:
                    v = v_in1 <= v_in2; break;
                case otNEQ:
                    v = v_in1 != v_in2; break;
                case otGREATER:
                    v = v_in1 > v_in2; break;
                case otGREATEREQ:
                    v = v_in1 >= v_in2; break;
                default:
                    v = rUNDEF;
                    return false;
                }
            }
            ++iterIn1;
            ++iterIn2;
        });
        return true;
    };

   bool res = OperationHelper::execute(binaryLogical, _outputGC);

    if (res && ctx)
        return setOutput(ctx);

    return false;
}

bool BinaryLogical::execute(ExecutionContext *ctx)
{
    if (_prepState == sNOTPREPARED)
        if((_prepState = prepare()) != sPREPARED)
            return false;

    if ( _coveragecoverage) {
        return executeCoverageCoverage(ctx);

    } else  {
        return executeCoverageNumber(ctx);
    }
    return true;
}

bool BinaryLogical::prepareCoverageCoverage() {
    QString gc =  _expression.parm(0).value();
    if (!_inputGC1.prepare(gc)) {
        kernel()->issues()->log(TR(ERR_COULD_NOT_LOAD_2).arg(gc, ""));
        return false;
    }
    gc =  _expression.parm(1).value();
    if (!_inputGC2.prepare(gc)) {
        kernel()->issues()->log(TR(ERR_COULD_NOT_LOAD_2).arg(gc, ""));
        return false;
    }
    bool isNumeric = _inputGC1->datadef().domain()->ilwisType() == itNUMERICDOMAIN && _inputGC2->datadef().domain()->ilwisType() == itNUMERICDOMAIN;
    if (!isNumeric ){
        return ERROR2(ERR_INVALID_PROPERTY_FOR_2,"domain","logical operation");
    }

    OperationHelper helper;
    _box = helper.initialize(_inputGC1, _outputGC, _expression.parm(0),
                                itGRIDSIZE | itENVELOPE | itCOORDSYSTEM | itGEOREF);

    IDomain dom;
    dom.prepare("bool");
    _outputGC->datadef().domain(dom);
    _coveragecoverage = true;
    return true;
}

bool BinaryLogical::prepareCoverageNumber(IlwisTypes ptype1, IlwisTypes ptype2) {

    int mindex = (ptype1 & itNUMERIC) == 0 ? 0 : 1;
    int nindex = mindex ? 0 : 1;

    QString gc =  _expression.parm(mindex).value();
    if (!_inputGC1.prepare(gc)) {
        kernel()->issues()->log(TR(ERR_COULD_NOT_LOAD_2).arg(gc, ""));
        return false;
    }
    if(_inputGC1->datadef().domain()->ilwisType() != itNUMERICDOMAIN)
        return false;

    _number = _expression.parm(nindex).value().toDouble();

    OperationHelper helper;
    _box = helper.initialize(_inputGC1, _outputGC, _expression.parm(mindex),
                                itGRIDSIZE | itENVELOPE | itCOORDSYSTEM | itGEOREF);

     IDomain dom;
    dom.prepare("boolean");
    _outputGC->datadef().domain(dom);

    return true;
}

OperationImplementation::State BinaryLogical::prepare() {
    if ( _expression.parameterCount() != 3){
        return sPREPAREFAILED;
    }

    IlwisTypes ptype1 = _expression.parm(0).valuetype();
    IlwisTypes ptype2 = _expression.parm(1).valuetype();

    QString oper = _expression.parm(2).value();
    oper = oper.toLower();
    if ( oper == "and")
        _operator = otAND;
    else if ( oper == "or")
        _operator = otOR;
    else if ( oper == "xor")
        _operator = otXOR;
    else if ( oper == "less")
        _operator = otLESS;
    else if ( oper == "lesseq")
        _operator = otLESSEQ;
    else if ( oper == "neq")
        _operator = otNEQ;
    else if ( oper == "eq")
        _operator = otEQ;
    else if ( oper == "greater")
        _operator = otGREATER;
    else if ( oper == "greatereq")
        _operator = otGREATEREQ;

    if ( ((ptype1 | ptype2) & (itGRIDCOVERAGE | itNUMERIC)) ) {
        if(!prepareCoverageNumber(ptype1, ptype2))
            return sPREPAREFAILED;

    } else if ( ptype1 & ptype2 & itGRIDCOVERAGE ) {
        if(!prepareCoverageCoverage())
            return sPREPAREFAILED;
    }

    return sPREPARED;
}

quint64 BinaryLogical::createMetadata()
{
    QString url = QString("ilwis://operations/binarylogicalraster");
    Resource res(QUrl(url), itOPERATIONMETADATA);
    res.addProperty("namespace","ilwis");
    res.addProperty("longname","binarylogicalraster");
    res.addProperty("syntax","binarylogicalraster(gridcoverage1,gridcoverage2|number|boolean,and|or|xor|less|lesseq|neq|eq|greater|greatereq)");
    res.addProperty("inparameters","3");
    res.addProperty("pin_1_type", itGRIDCOVERAGE | itNUMERIC);
    res.addProperty("pin_1_name", TR("input gridcoverage or number/boolean"));
    res.addProperty("pin_1_domain","value");
    res.addProperty("pin_1_desc",TR("input gridcoverage with a numerical/boolean domain or number"));
    res.addProperty("pin_2_type", itGRIDCOVERAGE | itNUMERIC);
    res.addProperty("pin_2_name", TR("input gridcoverage or number"));
    res.addProperty("pin_2_domain","value");
    res.addProperty("pin_2_desc",TR("input gridcoverage with a numerical/boolean domain or number"));
    res.addProperty("pin_3_type", itSTRING);
    res.addProperty("pin_3_name", TR("Operator"));
    res.addProperty("pin_3_domain","string");
    res.addProperty("pin_3_desc",TR("operator applied to the other 2 input parameters"));
    res.addProperty("outparameters",1);
    res.addProperty("pout_1_type", itGRIDCOVERAGE);
    res.addProperty("pout_1_name", TR("output gridcoverage"));
    res.addProperty("pout_1_domain","value");
    res.addProperty("pout_1_desc",TR("output gridcoverage with a boolean domain"));

    res.prepare();
    url += "=" + QString::number(res.id());
    res.setUrl(url);

    mastercatalog()->addItems({res});
    return res.id();
}