#ifndef TELEGRAMDATAINTERFACE_H
#define TELEGRAMDATAINTERFACE_H

#include "jsonhelper.h"

struct TelegramBotObject
{
    virtual void fromJson(QJsonObject& object) = 0;
    virtual ~TelegramBotObject() {}
};

template<typename T>
class JsonHelperT<T, typename std::enable_if<std::is_base_of<TelegramBotObject, T>::value>::type >
{
    public:
        static bool jsonPathGet(QJsonValue data, QString path, T& target, bool showWarnings = true)
        {
            QJsonObject object = showWarnings ? JsonHelper::jsonPathGet(data, path).toJsonObject() : JsonHelper::jsonPathGetSilent(data, path).toJsonObject();
            if(object.isEmpty()) return false;
            target.fromJson(object);
            return true;
        }

        static bool jsonPathGetArray(QJsonValue data, QString path, QList<T>& target, bool showWarnings = true)
        {
            QJsonArray array = showWarnings ? JsonHelper::jsonPathGet(data, path).toJsonArray() : JsonHelper::jsonPathGetSilent(data, path).toJsonArray();
            if(array.isEmpty()) return false;

            for(auto itr = array.begin(); itr != array.end(); itr++) {
                JsonHelperT::jsonPathGet(*itr, "", *target.insert(target.end(), T{}));
            }
            return true;
        }

        static inline bool jsonPathGetArrayArray(QJsonValue data, QString path, QList<QList<T>>& target, bool showWarnings = true)
        {
            QJsonArray array = showWarnings ? JsonHelper::jsonPathGet(data, path).toJsonArray() : JsonHelper::jsonPathGetSilent(data, path).toJsonArray();
            if(array.isEmpty()) return false;

            for(auto itr = array.begin(); itr != array.end(); itr++) {
                JsonHelperT::jsonPathGetArray(*itr, "", *target.insert(target.end(), QList<T>()));
            }
            return true;
        }
};


#endif // TELEGRAMDATAINTERFACE_H
