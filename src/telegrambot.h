#ifndef TELEGRAMBOT_H
#define TELEGRAMBOT_H

#include <QDebug>

#include <QObject>
#include <QVariant>

#include <QUrlQuery>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QNetworkReply>
#include <QNetworkAccessManager>

struct TelegramKeyboardButton
{
    // global
    QString text;

    // inline keyboard
    QString url;
    QString callBackData;
    QString switchInlineQuery;
    QString switchInlineQueryCurrentChat;

    // normal keyboard
    bool requestContact;
    bool requestLocation;
};

typedef QList<QList<TelegramKeyboardButton>> TelegramKeyboard;

class TelegramBot : public QObject
{
    Q_OBJECT
    public:
        enum TelegramFlags : long long
        {
            NoFlag                      = 0,

            // Message
            Markdown                     = 1 << 0,
            Html                         = 1 << 1,
            DisableWebPagePreview        = 1 << 2,
            DisableNotfication           = 1 << 3,

            // Keyboard Global
            Selective                    = 1 << 4,

            // ReplyKeyboardMarkup
            ReplyKeyboardMarkup          = 1 << 5,
            ResizeKeyboard               = 1 << 6,
            OneTimeKeyboard              = 1 << 7,

            // ReplyKeyboardRemove
            ReplyKeyboardRemove          = 1 << 8,

            // ForceReply
            ForceReply                   = 1 << 9
        };

        TelegramBot(QString apikey, QObject *parent = 0);
        void sendMessage(QVariant chatId, QString text, TelegramFlags flags = TelegramFlags::NoFlag, int replyToMessageId = 0, TelegramKeyboard keyboard = TelegramKeyboard());

    private:
        // helper
        QNetworkReply* callApi(QString method, QUrlQuery params = QUrlQuery());

        QNetworkAccessManager aManager;
        QString apiKey;
};

inline TelegramBot::TelegramFlags operator|(TelegramBot::TelegramFlags a, TelegramBot::TelegramFlags b)
{return static_cast<TelegramBot::TelegramFlags>(static_cast<int>(a) | static_cast<int>(b));}

// WARNING: override standard behavior: operator && returns true if bit is set, and not if complete equal!
// Note: this improves code readibility a lot!
inline bool operator&&(TelegramBot::TelegramFlags a, TelegramBot::TelegramFlags b)
{return (static_cast<int>(a) & static_cast<int>(b)) == static_cast<int>(b);}

#endif // TELEGRAMBOT_H
