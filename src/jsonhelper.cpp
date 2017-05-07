#include "jsonhelper.h"

QVariant JsonHelper::jsonPathGetImpl(QJsonValue data, QString path, bool showWarnings)
{
    // json parse
    QStringList splittedPath = path.split('.', QString::SkipEmptyParts);
    QStringList processed;
    while(!splittedPath.isEmpty()) {
        QString pathElement = splittedPath.takeFirst();
        processed += pathElement;

        // if we have an invalid value, return it
        if(data.isUndefined()) return data;

        // handle QJsonArray
        if(data.type() == QJsonValue::Array) {
            bool isInt;
            int intPath = pathElement.toInt(&isInt);
            QJsonArray jArray = data.toArray();
            if(!isInt) {
                if(showWarnings) qWarning("JsonHelper::jsonPath: pos: %s -> QJsonArray path element is non numeric",  qPrintable(processed.join(".")));
                intPath = -1;
            }
            else if(showWarnings && jArray.count() <= intPath) {
                if(jArray.count() >= 1) {
                    qWarning("JsonHelper::jsonPath: pos: %s -> QJsonArray with length %i, is smaller than requested",  qPrintable(processed.join(".")), jArray.count());
                } else {
                    qWarning("JsonHelper::jsonPath: pos: %s -> QJsonArray is empty so cannot access requested index",  qPrintable(processed.join(".")));
                }
            }
            data = jArray.at(intPath);
        }

        // handle QJsonObject
        else if(data.type() == QJsonValue::Object) {
            QJsonObject jObject = data.toObject();
            if(showWarnings && !jObject.contains(pathElement)) {
                qWarning("JsonHelper::jsonPath: pos: %s -> QJsonObject path element not found",  qPrintable(processed.join(".")));
            }
            data = jObject.value(pathElement);
        }

        // handle all other data types
        else {
            if(showWarnings && !splittedPath.isEmpty()) {
                qWarning("JsonHelper::jsonPath: pos: %s -> Reached data end, but still path data available (%s)", qPrintable(processed.join(".")), qPrintable(splittedPath.join(".")));
            }
            return data.toVariant();
        }
    }

    // if we reach this point we have not reached the tail of the json object, so we just return the current position (correct converted to the right type!), so that the user can continue process
    return data.isObject() ? QVariant::fromValue(data.toObject()) :
           data.isArray()  ? QVariant::fromValue(data.toArray()) :
                             QVariant::fromValue(data);
}
