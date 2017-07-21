#ifndef TELEGRAMBOT_H
#define TELEGRAMBOT_H

#include <QDebug>
#include <QTimer>

#include <QObject>
#include <QVariant>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
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
        void newMessage(TelegramBotUpdate message);

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

		// Keyboard construction helpers
        static inline TelegramBotKeyboardButtonRequest constructTextButton(QString text, bool requestContact = false, bool requestLocation = false){
            return TelegramBotKeyboardButtonRequest { text, QString(), QString(), QString(), QString(), requestContact, requestLocation };
        }
        static inline TelegramBotKeyboardButtonRequest constructInlineButton(QString text, QString callbackData, QString url = QString(), QString inlineQueryData = QString(), QString inlineQueryDataCurrentChat = QString()) {
            return TelegramBotKeyboardButtonRequest { text, url, callbackData, inlineQueryData, inlineQueryDataCurrentChat, false, false };
        }
        static TelegramKeyboardRequest constructInlineMenu(QList<QString> menu, QString dataPattern, int page, int columns, int limit, QString lastPage = "");

        TelegramBot(QString apikey, QObject *parent = 0);
        ~TelegramBot();

        // Bot Functions
        TelegramBotUser getMe();
        void sendChatAction(QVariant chatId, TelegramBotChatAction action, bool* response = 0);
        void sendChatAction(QVariant chatId, QString action, bool* response = 0);
        TelegramBotFile getFile(QString fileId, bool generateAbsoluteLink = false);

        // User Functions
        TelegramBotUserProfilePhotos getUserProfilePhotos(qint32 userId, int offset = 0, int limit = 0);

        // Chat Functions
        void kickChatMember(QVariant chatId, qint32 userId, bool* response = 0);
        void unbanChatMember(QVariant chatId, qint32 userId, bool* response = 0);
        void leaveChat(QVariant chatId, bool* response = 0);
        TelegramBotChat getChat(QVariant chatId);
        QList<TelegramBotChatMember> getChatAdministrators(QVariant chatId);
        int getChatMembersCount(QVariant chatId);
        TelegramBotChatMember getChatMember(QVariant chatId, qint32 userId);

        // Callback Query Functions
        void answerCallbackQuery(QString callbackQueryId, QString text = QString(), bool showAlert = false, int cacheTime = 0, QString url = QString(), bool* response = 0);

        // Message Functions
        void sendMessage(QVariant chatId, QString text, int replyToMessageId = 0, TelegramFlags flags = TelegramFlags::NoFlag, TelegramKeyboardRequest keyboard = TelegramKeyboardRequest(), TelegramBotMessage* response = 0);
        void editMessageText(QVariant chatId, QVariant messageId, QString text, TelegramFlags flags = TelegramFlags::NoFlag, TelegramKeyboardRequest keyboard = TelegramKeyboardRequest(), bool* response = 0);
        void editMessageCaption(QVariant chatId, QVariant messageId, QString caption = QString(), TelegramKeyboardRequest keyboard = TelegramKeyboardRequest(), bool* response = 0);
        void editMessageReplyMarkup(QVariant chatId, QVariant messageId, TelegramKeyboardRequest keyboard = TelegramKeyboardRequest(), bool* response = 0);
        void forwardMessage(QVariant targetChatId, QVariant fromChatId, qint32 fromMessageId, TelegramFlags flags = TelegramFlags::NoFlag, TelegramBotMessage* response = 0);
        void deleteMessage(QVariant chatId, qint32 messageId, bool* response = 0);

        // Content Functions
        void sendPhoto(QVariant chatId, QVariant photo, QString caption = QString(), int replyToMessageId = 0, TelegramFlags flags = TelegramFlags::NoFlag, TelegramKeyboardRequest keyboard = TelegramKeyboardRequest(), TelegramBotMessage* response = 0);
        void sendAudio(QVariant chatId, QVariant audio, QString caption = QString(), QString performer = QString(), QString title = QString(), int duration = -1, int replyToMessageId = 0, TelegramFlags flags = TelegramFlags::NoFlag, TelegramKeyboardRequest keyboard = TelegramKeyboardRequest(), TelegramBotMessage* response = 0);
        void sendDocument(QVariant chatId, QVariant document, QString caption = QString(), int replyToMessageId = 0, TelegramFlags flags = TelegramFlags::NoFlag, TelegramKeyboardRequest keyboard = TelegramKeyboardRequest(), TelegramBotMessage* response = 0);
        void sendSticker(QVariant chatId, QVariant sticker, int replyToMessageId = 0, TelegramFlags flags = TelegramFlags::NoFlag, TelegramKeyboardRequest keyboard = TelegramKeyboardRequest(), TelegramBotMessage* response = 0);
        void sendVideo(QVariant chatId, QVariant video, QString caption = QString(), int duration = -1, int width = -1, int height = -1, int replyToMessageId = 0, TelegramFlags flags = TelegramFlags::NoFlag, TelegramKeyboardRequest keyboard = TelegramKeyboardRequest(), TelegramBotMessage* response = 0);
        void sendVoice(QVariant chatId, QVariant voice, QString caption = QString(), int duration = -1, int replyToMessageId = 0, TelegramFlags flags = TelegramFlags::NoFlag, TelegramKeyboardRequest keyboard = TelegramKeyboardRequest(), TelegramBotMessage* response = 0);
        void sendVideoNote(QVariant chatId, QVariant videoNote, int length = -1, int duration = -1, int replyToMessageId = 0, TelegramFlags flags = TelegramFlags::NoFlag, TelegramKeyboardRequest keyboard = TelegramKeyboardRequest(), TelegramBotMessage* response = 0);
        void sendLocation(QVariant chatId, double latitude, double longitude, int replyToMessageId = 0, TelegramFlags flags = TelegramFlags::NoFlag, TelegramKeyboardRequest keyboard = TelegramKeyboardRequest(), TelegramBotMessage* response = 0);
        void sendVenue(QVariant chatId, double latitude, double longitude, QString title, QString address, QString foursquareId = QString(), int replyToMessageId = 0, TelegramFlags flags = TelegramFlags::NoFlag, TelegramKeyboardRequest keyboard = TelegramKeyboardRequest(), TelegramBotMessage* response = 0);
        void sendContact(QVariant chatId, QString phoneNumber, QString firstName, QString lastName = QString(), int replyToMessageId = 0, TelegramFlags flags = TelegramFlags::NoFlag, TelegramKeyboardRequest keyboard = TelegramKeyboardRequest(), TelegramBotMessage* response = 0);

        // Message Puller
        void startMessagePulling(uint timeout = 10, uint limit = 100, TelegramPollMessageTypes messageTypes = TelegramPollMessageTypes::All, long offset = 0);
        void stopMessagePulling(bool instantly = false);

        // Webhook Functions
        bool setHttpServerWebhook(qint16 port, QString pathCert, QString pathPrivateKey, int maxConnections = 10, TelegramPollMessageTypes messageTypes = TelegramPollMessageTypes::All);
        void deleteWebhook();
        TelegramBotOperationResult deleteWebhookResult();
        TelegramBotWebHookInfo getWebhookInfo();

        // Message Router functions
        void messageRouterRegister(QString startWith, QDelegate<bool(TelegramBotUpdate)> delegate, TelegramBotMessageType type = TelegramBotMessageType::All);

    private slots:
        // pull functions
        void pull();
        void handlePullResponse();

        // parser functions
        void parseMessage(QByteArray &data, bool singleMessage = false);

        // webhook functions
        void handleServerWebhookResponse(HttpServerRequest request, HttpServerResponse response);

    private:
        // call Api Helpers
        template<typename T>
        typename std::enable_if<std::is_base_of<TelegramBotObject, T>::value>::type callApiTemplate(QString method, QUrlQuery params = QUrlQuery(), T* response = 0, QHttpMultiPart* multiPart = 0);
        template<typename T>
        typename std::enable_if<!std::is_base_of<TelegramBotObject, T>::value>::type callApiTemplate(QString method, QUrlQuery params = QUrlQuery(), T* response = 0, QHttpMultiPart* multiPart = 0);

        QNetworkReply* callApi(QString method, QUrlQuery params = QUrlQuery(), bool deleteOnFinish = true, QHttpMultiPart* multiPart = 0);
        QJsonObject callApiJson(QString method, QUrlQuery params = QUrlQuery(), QHttpMultiPart* multiPart = 0);

        // helpers
        QHttpMultiPart* createUploadFile(QString name, QString fileName, QByteArray& content, bool detectMimeType = false, QHttpMultiPart* multiPart = 0);
        void hanldeReplyMarkup(QUrlQuery& params, TelegramFlags flags, TelegramKeyboardRequest& keyboard);
        QHttpMultiPart* handleFile(QString fieldName, QVariant file, QUrlQuery& params, QHttpMultiPart* multiPart = 0);

        // global data
        QNetworkAccessManager aManager;
        QString apiKey;
        long updateId = 0;

        // message puller
        QNetworkReply* replyPull = 0;
        QUrlQuery pullParams;

        // httpserver webhook
        static QMap<qint16, HttpServer*> webHookWebServers;

        // message router
        struct MessageRoute
        {
            TelegramBotMessageType type;
            QString startWith;
            QDelegate<bool(TelegramBotUpdate)> delegate;
        };
        QList<MessageRoute*> messageRoutes;
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
