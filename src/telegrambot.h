#ifndef TELEGRAMBOT_H
#define TELEGRAMBOT_H

#include <QDebug>
#include <QTimer>

#include <QObject>
#include <QVariant>

#include <QUrlQuery>

#include <QNetworkReply>
#include <QNetworkAccessManager>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>

#include "jsonhelper.h"
#include "telegramdatastructs.h"

class TelegramBot : public QObject
{
    Q_OBJECT
    signals:
        void newMessage(TelegramBotMessage message);
        void newInlineQuery(TelegramBotInlineQuery inlineQuery);
        void newChoosenInlineResult(TelegramBotChosenInlineResult choosenInlineResult);
        void newCallbackQuery(TelegramBotCallbackQuery callbackQuery);

    public:
        enum TelegramPollMessageTypes
        {
            All                 = 0,
            Message             = 1 << 0,
            EditedMessage       = 1 << 1,
            ChannelPost         = 1 << 2,
            EditedChannelPost   = 1 << 3,
            InlineQuery         = 1 << 4,
            ChoosenInlineQuery  = 1 << 5,
            CallbackQuery       = 1 << 6
        };

        enum TelegramFlags : long long
        {
            NoFlag                       = 0,

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
        void sendMessage(QVariant chatId, QString text, TelegramFlags flags = TelegramFlags::NoFlag, int replyToMessageId = 0, TelegramKeyboardRequest keyboard = TelegramKeyboardRequest());
        void startMessagePulling(uint timeout = 10, uint limit = 100, TelegramPollMessageTypes messageTypes = TelegramPollMessageTypes::All, long offset = 0);
        void stopMessagePulling(bool instantly = false);

    private slots:
        void pull();
        void handlePullResponse();

    private:
        // helper
        QNetworkReply* callApi(QString method, QUrlQuery params = QUrlQuery(), bool deleteOnFinish = true);

        QNetworkAccessManager aManager;
        QString apiKey;

        // message poller
        QNetworkReply* replyPull = 0;
        QUrlQuery pullParams;
        long updateId = 0;
};

/*
 * TelegramPollMessageTypes - operators
 */
inline TelegramBot::TelegramPollMessageTypes operator|(TelegramBot::TelegramPollMessageTypes a, TelegramBot::TelegramPollMessageTypes b)
{return static_cast<TelegramBot::TelegramPollMessageTypes>(static_cast<int>(a) | static_cast<int>(b));}

// WARNING: override standard behavior: operator && returns true if bit is set, and not if complete equal!
// Note: this improves code readibility a lot!
inline bool operator&&(TelegramBot::TelegramPollMessageTypes a, TelegramBot::TelegramPollMessageTypes b)
{return (static_cast<int>(a) & static_cast<int>(b)) == static_cast<int>(b);}

/*
 * TelegramFlags - operators
 */
inline TelegramBot::TelegramFlags operator|(TelegramBot::TelegramFlags a, TelegramBot::TelegramFlags b)
{return static_cast<TelegramBot::TelegramFlags>(static_cast<int>(a) | static_cast<int>(b));}

// WARNING: override standard behavior: operator && returns true if bit is set, and not if complete equal!
// Note: this improves code readibility a lot!
inline bool operator&&(TelegramBot::TelegramFlags a, TelegramBot::TelegramFlags b)
{return (static_cast<int>(a) & static_cast<int>(b)) == static_cast<int>(b);}

#endif // TELEGRAMBOT_H
