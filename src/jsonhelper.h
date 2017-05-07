#ifndef JSONHELPER_H
#define JSONHELPER_H

#include <QVariant>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

class JsonHelper
{
    public:
        // main json parser
        template<typename T>
        static bool jsonPathGet(QJsonValue data, QString path, T& target, bool showWarnings = true)
        {
            // get value and exit if value is invalid, or is not convertable to T
            QVariant jPathValue = JsonHelper::jsonPathGetImpl(data, path, showWarnings);
            target = jPathValue.value<T>();

            // return true if convert was successfull otherwise false
            return jPathValue != QJsonValue(QJsonValue::Undefined) && !jPathValue.canConvert<T>();
        }
        static inline QVariant jsonPathGet(QJsonValue data, QString path) { return JsonHelper::jsonPathGetImpl(data, path, true); }
        static inline QVariant jsonPathGetSilent(QJsonValue data, QString path) { return JsonHelper::jsonPathGetImpl(data, path, false); }

    private:
        static QVariant jsonPathGetImpl(QJsonValue data, QString path, bool showWarnings);
};

template<typename T, class Enable = void>
class JsonHelperT
{
    public:
        static inline bool jsonPathGet(QJsonValue data, QString path, T& target, bool showWarnings = true)
        {
            return JsonHelper::jsonPathGet(data, path, target, showWarnings);
        }
        static bool jsonPathGetArray(QJsonValue data, QString path, QList<T>& target, bool showWarnings = true)
        {
            QJsonValue value = showWarnings ? JsonHelper::jsonPathGet(data, path).toJsonValue() : JsonHelper::jsonPathGetSilent(data, path).toJsonValue();
            if(value.isArray()) {
                QJsonArray jArray = value.toArray();
                for(auto itr = jArray.begin(); itr != jArray.end(); itr++) {
                    JsonHelper::jsonPathGet(*itr, QString::number(itr.i), *target.insert(target.end(), T{}), showWarnings);
                }
            } else if(value.isObject()) {
                QJsonObject jObject = value.toObject();
                for(auto itr = jObject.begin(); itr != jObject.end(); itr++) {
                    JsonHelper::jsonPathGet(*itr, itr.key(), *target.insert(target.end(), T{}), showWarnings);
                }
            } else {
                return false;
            }
            return true;
        }
        static bool jsonPathGetArrayArray(QJsonValue data, QString path, QList<QList<T>>& target, bool showWarnings = true)
        {
            QJsonValue value = showWarnings ? JsonHelper::jsonPathGet(data, path).toJsonValue() : JsonHelper::jsonPathGetSilent(data, path).toJsonValue();
            if(value.isArray()) {
                QJsonArray jArray = value.toArray();
                for(auto itr = jArray.begin(); itr != jArray.end(); itr++) {
                    JsonHelperT::jsonPathGetArray(*itr, QString::number(itr.i), *target.insert(target.end(), QList<T>()), showWarnings);
                }
            } else if(value.isObject()) {
                QJsonObject jObject = value.toObject();
                for(auto itr = jObject.begin(); itr != jObject.end(); itr++) {
                    JsonHelperT::jsonPathGetArray(*itr, itr.key(), *target.insert(target.end(), QList<T>()), showWarnings);
                }
            } else {
                return false;
            }
            return true;
        }
        static inline QVariant jsonPathGet(QJsonValue data, QString path)
        {
            return JsonHelper::jsonPathGet(data, path);
        }
        static inline QVariant jsonPathGetSilent(QJsonValue data, QString path)
        {
            return JsonHelper::jsonPathGetSilent(data, path);
        }
};

#endif // JSONHELPER_H
