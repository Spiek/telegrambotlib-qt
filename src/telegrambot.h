#ifndef TELEGRAMBOT_H
#define TELEGRAMBOT_H

#include <QDebug>
#include <QTimer>

#include <QObject>
#include <QVariant>
#include <QEventLoop>
#include <QMimeDatabase>

#include <QUrlQuery>

#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QHttpMultiPart>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>

#include "jsonhelper.h"
#include "telegramdatastructs.h"

#include "httpserver.h"

// Helper Macros
#define EXIT_FAILED(...) qWarning(__VA_ARGS__); return false;

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

        // Info Functions
        TelegramBotUser getMe();

        // Message Functions
        void sendMessage(QVariant chatId, QString text, TelegramFlags flags = TelegramFlags::NoFlag, int replyToMessageId = 0, TelegramKeyboardRequest keyboard = TelegramKeyboardRequest());
        void forwardMessage(QVariant chatId, QVariant fromChatId, qint32 messageId, TelegramFlags flags = TelegramFlags::NoFlag);

        // Message Puller
        void startMessagePulling(uint timeout = 10, uint limit = 100, TelegramPollMessageTypes messageTypes = TelegramPollMessageTypes::All, long offset = 0);
        void stopMessagePulling(bool instantly = false);

        // Webhook Functions
        bool setHttpServerWebhook(qint16 port, QString pathCert, QString pathPrivateKey, int maxConnections = 10, TelegramPollMessageTypes messageTypes = TelegramPollMessageTypes::All);
        void deleteWebhook();
        TelegramBotOperationResult deleteWebhookResult();
        TelegramBotWebHookInfo getWebhookInfo();

    private slots:
        // pull functions
        void pull();
        void handlePullResponse();

        // parser functions
        void parseMessage(QByteArray &data, bool singleMessage = false);

        // webhook functions
        void handleServerWebhookResponse(HttpServerRequest request, HttpServerResponse response);

    private:
        // helper
        QNetworkReply* callApi(QString method, QUrlQuery params = QUrlQuery(), bool deleteOnFinish = true, QHttpMultiPart* multiPart = 0);
        QJsonObject callApiJson(QString method, QUrlQuery params = QUrlQuery(), QHttpMultiPart* multiPart = 0);
        QHttpMultiPart* generateFile(QString name, QString fileName, QByteArray& content, bool detectMimeType = false, QHttpMultiPart* multiPart = 0);
        void hanldeReplyMarkup(QUrlQuery& params, TelegramFlags flags, TelegramKeyboardRequest& keyboard);

        // global data
        QNetworkAccessManager aManager;
        QString apiKey;
        long updateId = 0;

        // message puller
        QNetworkReply* replyPull = 0;
        QUrlQuery pullParams;

        // httpserver webhook
        static QMap<qint16, HttpServer*> webHookWebServers;
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
