#include "telegrambot.h"

QMap<qint16, HttpServer*> TelegramBot::webHookWebServers = QMap<qint16, HttpServer*>();


TelegramKeyboardRequest TelegramBot::constructInlineMenu(QList<QString> menu, QString dataPattern, int page, int columns, int limit, QString lastPage)
{
    // generate time zone keyboard for continent
    TelegramKeyboardRequest keyboard;
    int column = 0;
    for(QString menuEntry : !limit ? menu : menu.mid(!page ? 0 : (page * limit) - 1, limit)) {
        if(!column || column == columns) {
            keyboard.append(QList<TelegramBotKeyboardButtonRequest>());
            column = 0;
        }
        keyboard.last().append(TelegramBot::constructInlineButton(menuEntry, QString(dataPattern).arg(menuEntry)));
        column++;
    }

    // add back/forward button?
    QString currentPage = !dataPattern.contains(".") ? QString(dataPattern).arg("") : dataPattern.left(dataPattern.lastIndexOf("."));
    bool hasBackButton = page > 0;
    bool hasForwardButton = limit && (page * limit) + limit < menu.length();

    // construct menu row
    if(!lastPage.isEmpty() || hasBackButton || hasForwardButton) {
        keyboard.append(QList<TelegramBotKeyboardButtonRequest>{});
    }
    if(!lastPage.isEmpty()) {
        keyboard.last().append({TelegramBot::constructInlineButton("Back", lastPage)});
    }
    if(hasBackButton) {
        keyboard.last().append({TelegramBot::constructInlineButton("<", QString("%1..%2").arg(currentPage).arg(page - 1))});
    }
    if(hasForwardButton) {
        keyboard.last().append({TelegramBot::constructInlineButton(">", QString("%1..%2").arg(currentPage).arg(page + 1))});
    }

    // return keyboard
    return keyboard;
}

TelegramBot::TelegramBot(QString apikey, QObject *parent) : QObject(parent), apiKey(apikey) { }

TelegramBot::~TelegramBot()
{
    qDeleteAll(this->messageRoutes);
}

/*
 * Bot Functions
 */
TelegramBotUser TelegramBot::getMe()
{
    return TelegramBotUser(this->callApiJson("getMe").value("result").toObject());
}

void TelegramBot::sendChatAction(QVariant chatId, TelegramBotChatAction action, bool *response)
{
    return this->sendChatAction(chatId, action == TelegramBotChatAction::Typing            ? "typing" :
                                        action == TelegramBotChatAction::UploadPhoto       ? "upload_photo" :
                                        action == TelegramBotChatAction::RecordVideo       ? "record_video" :
                                        action == TelegramBotChatAction::UploadVideo       ? "upload_video" :
                                        action == TelegramBotChatAction::RecordAudio       ? "record_audio" :
                                        action == TelegramBotChatAction::UploadAudio       ? "upload_audio" :
                                        action == TelegramBotChatAction::UploadDocument    ? "upload_document" :
                                        action == TelegramBotChatAction::FindLocation      ? "find_location" :
                                        action == TelegramBotChatAction::RecordVideoNote   ? "record_video_note" :
                                        action == TelegramBotChatAction::UploadVideoNote   ? "upload_video_note" : "", response);
}

void TelegramBot::sendChatAction(QVariant chatId, QString action, bool *response)
{
    // param check
    if(action.isEmpty()) return;

    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    params.addQueryItem("action", action);

    // call api
    this->callApiTemplate("sendChatAction", params, response);
}

TelegramBotFile TelegramBot::getFile(QString fileId, bool generateAbsoluteLink)
{
    // prepare
    QDateTime validUntil = QDateTime::currentDateTime().addSecs(3600);

    // build params
    QUrlQuery params;
    params.addQueryItem("file_id", fileId);

    // construct TelegramBotFile
    TelegramBotFile file(this->callApiJson("getFile", params).value("result").toObject());
    file.validUntil = validUntil;
    if(generateAbsoluteLink && !file.filePath.isEmpty()) file.link = QString("https://api.telegram.org/file/bot%1/%2").arg(this->apiKey, file.filePath);
    return file;
}

/*
 * User Functions
 */
TelegramBotUserProfilePhotos TelegramBot::getUserProfilePhotos(qint32 userId, int offset, int limit)
{
    QUrlQuery params;
    params.addQueryItem("user_id", QString::number(userId));
    if(offset > 0) params.addQueryItem("offset", QString::number(offset));
    if(limit > 0) params.addQueryItem("limit", QString::number(limit));

    // call api and return constructed data
    return TelegramBotUserProfilePhotos(this->callApiJson("getUserProfilePhotos", params).value("result").toObject());
}

/*
 * Chat Functions
 */
void TelegramBot::kickChatMember(QVariant chatId, qint32 userId, bool *response)
{
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    params.addQueryItem("user_id", QString::number(userId));

    this->callApiTemplate("kickChatMember", params, response);
}

void TelegramBot::unbanChatMember(QVariant chatId, qint32 userId, bool *response)
{
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    params.addQueryItem("user_id", QString::number(userId));

    this->callApiTemplate("unbanChatMember", params, response);
}

void TelegramBot::leaveChat(QVariant chatId, bool *response)
{
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());

    this->callApiTemplate("leaveChat", params, response);
}

TelegramBotChat TelegramBot::getChat(QVariant chatId)
{
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());

    return TelegramBotChat(this->callApiJson("getChat", params).value("result").toObject());
}

QList<TelegramBotChatMember> TelegramBot::getChatAdministrators(QVariant chatId)
{
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());

    // call api and parse result
    QList<TelegramBotChatMember> chatMemebers;
    JsonHelperT<TelegramBotChatMember>::jsonPathGetArray(this->callApiJson("getChatAdministrators", params), "result", chatMemebers);
    return chatMemebers;
}

int TelegramBot::getChatMembersCount(QVariant chatId)
{
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    return this->callApiJson("getChatMembersCount", params).value("result").toInt();
}

TelegramBotChatMember TelegramBot::getChatMember(QVariant chatId, qint32 userId)
{
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    params.addQueryItem("user_id", QString::number(userId));

    return TelegramBotChatMember(this->callApiJson("getChatMember", params).value("result").toObject());
}

/*
 * Callback Query Functions
 */
void TelegramBot::answerCallbackQuery(QString callbackQueryId, QString text, bool showAlert, int cacheTime, QString url, bool *response)
{
    QUrlQuery params;
    params.addQueryItem("callback_query_id", callbackQueryId);
    if(!text.isNull()) params.addQueryItem("text", text);
    if(showAlert) params.addQueryItem("show_alert", "true");
    if(!url.isNull()) params.addQueryItem("url", url);
    if(cacheTime > 0) params.addQueryItem("cache_time", QString::number(cacheTime));

    this->callApiTemplate("answerCallbackQuery", params, response);
}


/*
 * Message Functions
 */
void TelegramBot::sendMessage(QVariant chatId, QString text, int replyToMessageId, TelegramFlags flags, TelegramKeyboardRequest keyboard, TelegramBotMessage *response)
{
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    params.addQueryItem("text", text);
    if(flags && TelegramFlags::Markdown) params.addQueryItem("parse_mode", "Markdown");
    else if(flags && TelegramFlags::Html) params.addQueryItem("parse_mode", "HTML");
    if(flags && TelegramFlags::DisableWebPagePreview) params.addQueryItem("disable_web_page_preview", "true");
    if(flags && TelegramFlags::DisableNotfication) params.addQueryItem("disable_notification", "true");
    if(replyToMessageId) params.addQueryItem("reply_to_message_id", QString::number(replyToMessageId));

    // handle reply markup
    this->hanldeReplyMarkup(params, flags, keyboard);

    // call api
    return this->callApiTemplate("sendMessage", params, response);
}

void TelegramBot::editMessageText(QVariant chatId, QVariant messageId, QString text, TelegramFlags flags, TelegramKeyboardRequest keyboard, bool *response)
{
    // if we have a null messageId, and user don't request a response, call send Message
    if(!response && messageId.isNull()) {
        return this->sendMessage(chatId, text, 0, flags, keyboard);
    }

    // determine message id type
    bool isInlineMessageId = messageId.type() == QVariant::String;

    QUrlQuery params;
    if(!isInlineMessageId && !chatId.isNull()) params.addQueryItem("chat_id", chatId.toString());
    params.addQueryItem(isInlineMessageId ? "inline_message_id" : "message_id", messageId.toString());
    params.addQueryItem("text", text);
    if(flags && TelegramFlags::Markdown) params.addQueryItem("parse_mode", "Markdown");
    else if(flags && TelegramFlags::Html) params.addQueryItem("parse_mode", "HTML");
    if(flags && TelegramFlags::DisableWebPagePreview) params.addQueryItem("disable_web_page_preview", "true");

    // only build inline keyboard
    if(!(flags && TelegramFlags::ReplyKeyboardMarkup) && !(flags && TelegramFlags::ForceReply) && !(flags && TelegramFlags::ReplyKeyboardRemove)) {
        this->hanldeReplyMarkup(params, flags, keyboard);
    }

    // call api
    this->callApiTemplate("editMessageText", params, response);
}

void TelegramBot::editMessageCaption(QVariant chatId, QVariant messageId, QString caption, TelegramKeyboardRequest keyboard, bool *response)
{
    // determine message id type
    bool isInlineMessageId = messageId.type() == QVariant::String;

    QUrlQuery params;
    if(!isInlineMessageId && !chatId.isNull()) params.addQueryItem("chat_id", chatId.toString());
    params.addQueryItem(isInlineMessageId ? "inline_message_id" : "message_id", messageId.toString());
    if(!caption.isNull()) params.addQueryItem("caption", caption);

    // only build inline keyboard
    this->hanldeReplyMarkup(params, TelegramFlags(), keyboard);

    // call api
    this->callApiTemplate("editMessageCaption", params, response);
}

void TelegramBot::editMessageReplyMarkup(QVariant chatId, QVariant messageId, TelegramKeyboardRequest keyboard, bool *response)
{
    // determine message id type
    bool isInlineMessageId = messageId.type() == QVariant::String;

    QUrlQuery params;
    if(!isInlineMessageId && !chatId.isNull()) params.addQueryItem("chat_id", chatId.toString());
    params.addQueryItem(isInlineMessageId ? "inline_message_id" : "message_id", messageId.toString());

    // only build inline keyboard
    this->hanldeReplyMarkup(params, TelegramFlags(), keyboard);

    // call api
    this->callApiTemplate("editMessageReplyMarkup", params, response);
}

void TelegramBot::forwardMessage(QVariant targetChatId, QVariant fromChatId, qint32 fromMessageId, TelegramFlags flags, TelegramBotMessage *response)
{
    QUrlQuery params;
    params.addQueryItem("chat_id", targetChatId.toString());
    params.addQueryItem("from_chat_id", fromChatId.toString());
    params.addQueryItem("message_id", QString::number(fromMessageId));
    if(flags && TelegramFlags::DisableNotfication) params.addQueryItem("disable_notification", "true");

    this->callApiTemplate("forwardMessage", params, response);
}

void TelegramBot::deleteMessage(QVariant chatId, qint32 messageId, bool *response)
{
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    params.addQueryItem("message_id", QString::number(messageId));

    this->callApiTemplate("deleteMessage", params, response);
}

/*
 * Content Functions
 */
void TelegramBot::sendPhoto(QVariant chatId, QVariant photo, QString caption, int replyToMessageId, TelegramFlags flags, TelegramKeyboardRequest keyboard, TelegramBotMessage *response)
{
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    if(!caption.isNull()) params.addQueryItem("caption", caption);
    if(flags && TelegramFlags::DisableNotfication) params.addQueryItem("disable_notification", "true");
    if(replyToMessageId) params.addQueryItem("reply_to_message_id", QString::number(replyToMessageId));

    // handle reply markup
    this->hanldeReplyMarkup(params, flags, keyboard);

    // handle file
    QHttpMultiPart* multiPart = this->handleFile("photo", photo, params);

    // call api
    this->callApiTemplate("sendPhoto", params, response, multiPart);
}

void TelegramBot::sendAudio(QVariant chatId, QVariant audio, QString caption, QString performer, QString title, int duration, int replyToMessageId, TelegramFlags flags, TelegramKeyboardRequest keyboard, TelegramBotMessage *response)
{
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    if(!caption.isNull()) params.addQueryItem("caption", caption);
    if(duration >= 0) params.addQueryItem("duration", QString::number(duration));
    if(!performer.isNull()) params.addQueryItem("performer", performer);
    if(!title.isNull()) params.addQueryItem("title", title);
    if(flags && TelegramFlags::DisableNotfication) params.addQueryItem("disable_notification", "true");
    if(replyToMessageId) params.addQueryItem("reply_to_message_id", QString::number(replyToMessageId));

    // handle reply markup
    this->hanldeReplyMarkup(params, flags, keyboard);

    // handle file
    QHttpMultiPart* multiPart = this->handleFile("audio", audio, params);

    // call api
    this->callApiTemplate("sendAudio", params, response, multiPart);
}

void TelegramBot::sendDocument(QVariant chatId, QVariant document, QString caption, int replyToMessageId, TelegramFlags flags, TelegramKeyboardRequest keyboard, TelegramBotMessage *response)
{
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    if(!caption.isNull()) params.addQueryItem("caption", caption);
    if(flags && TelegramFlags::DisableNotfication) params.addQueryItem("disable_notification", "true");
    if(replyToMessageId) params.addQueryItem("reply_to_message_id", QString::number(replyToMessageId));

    // handle reply markup
    this->hanldeReplyMarkup(params, flags, keyboard);

    // handle file
    QHttpMultiPart* multiPart = this->handleFile("document", document, params);

    // call api
    this->callApiTemplate("sendDocument", params, response, multiPart);
}

void TelegramBot::sendSticker(QVariant chatId, QVariant sticker, int replyToMessageId, TelegramFlags flags, TelegramKeyboardRequest keyboard, TelegramBotMessage *response)
{
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    if(flags && TelegramFlags::DisableNotfication) params.addQueryItem("disable_notification", "true");
    if(replyToMessageId) params.addQueryItem("reply_to_message_id", QString::number(replyToMessageId));

    // handle reply markup
    this->hanldeReplyMarkup(params, flags, keyboard);

    // handle file
    QHttpMultiPart* multiPart = this->handleFile("sticker", sticker, params);

    // call api
    this->callApiTemplate("sendSticker", params, response, multiPart);
}

void TelegramBot::sendVideo(QVariant chatId, QVariant video, QString caption, int duration, int width, int height, int replyToMessageId, TelegramFlags flags, TelegramKeyboardRequest keyboard, TelegramBotMessage *response)
{
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    if(!caption.isNull()) params.addQueryItem("caption", caption);
    if(duration >= 0) params.addQueryItem("duration", QString::number(duration));
    if(width >= 0) params.addQueryItem("width", QString::number(width));
    if(height >= 0) params.addQueryItem("height", QString::number(height));
    if(flags && TelegramFlags::DisableNotfication) params.addQueryItem("disable_notification", "true");
    if(replyToMessageId) params.addQueryItem("reply_to_message_id", QString::number(replyToMessageId));

    // handle reply markup
    this->hanldeReplyMarkup(params, flags, keyboard);

    // handle file
    QHttpMultiPart* multiPart = this->handleFile("video", video, params);

    // call api
    this->callApiTemplate("sendVideo", params, response, multiPart);
}

void TelegramBot::sendVoice(QVariant chatId, QVariant voice, QString caption, int duration, int replyToMessageId, TelegramFlags flags, TelegramKeyboardRequest keyboard, TelegramBotMessage *response)
{
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    if(!caption.isNull()) params.addQueryItem("caption", caption);
    if(duration >= 0) params.addQueryItem("duration", QString::number(duration));
    if(flags && TelegramFlags::DisableNotfication) params.addQueryItem("disable_notification", "true");
    if(replyToMessageId) params.addQueryItem("reply_to_message_id", QString::number(replyToMessageId));

    // handle reply markup
    this->hanldeReplyMarkup(params, flags, keyboard);

    // handle file
    QHttpMultiPart* multiPart = this->handleFile("voice", voice, params);

    // call api
    this->callApiTemplate("sendVoice", params, response, multiPart);
}

void TelegramBot::sendVideoNote(QVariant chatId, QVariant videoNote, int length, int duration, int replyToMessageId, TelegramFlags flags, TelegramKeyboardRequest keyboard, TelegramBotMessage *response)
{
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    if(length >= 0) params.addQueryItem("length", QString::number(length));
    if(duration >= 0) params.addQueryItem("duration", QString::number(duration));
    if(flags && TelegramFlags::DisableNotfication) params.addQueryItem("disable_notification", "true");
    if(replyToMessageId) params.addQueryItem("reply_to_message_id", QString::number(replyToMessageId));

    // handle reply markup
    this->hanldeReplyMarkup(params, flags, keyboard);

    // handle file
    QHttpMultiPart* multiPart = this->handleFile("video_note", videoNote, params);

    // call api
    this->callApiTemplate("sendVideoNote", params, response, multiPart);
}

void TelegramBot::sendLocation(QVariant chatId, double latitude, double longitude, int replyToMessageId, TelegramFlags flags, TelegramKeyboardRequest keyboard, TelegramBotMessage *response)
{
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    params.addQueryItem("latitude", QString::number(latitude));
    params.addQueryItem("longitude", QString::number(longitude));
    if(flags && TelegramFlags::DisableNotfication) params.addQueryItem("disable_notification", "true");
    if(replyToMessageId) params.addQueryItem("reply_to_message_id", QString::number(replyToMessageId));

    // handle reply markup
    this->hanldeReplyMarkup(params, flags, keyboard);

    // call api
    this->callApiTemplate("sendLocation", params, response);
}

void TelegramBot::sendVenue(QVariant chatId, double latitude, double longitude, QString title, QString address, QString foursquareId, int replyToMessageId, TelegramFlags flags, TelegramKeyboardRequest keyboard, TelegramBotMessage *response)
{
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    params.addQueryItem("latitude", QString::number(latitude));
    params.addQueryItem("longitude", QString::number(longitude));
    params.addQueryItem("title", title);
    params.addQueryItem("address", address);
    if(!foursquareId.isNull()) params.addQueryItem("foursquare_id", foursquareId);
    if(flags && TelegramFlags::DisableNotfication) params.addQueryItem("disable_notification", "true");
    if(replyToMessageId) params.addQueryItem("reply_to_message_id", QString::number(replyToMessageId));

    // handle reply markup
    this->hanldeReplyMarkup(params, flags, keyboard);

    // call api
    this->callApiTemplate("sendVenue", params, response);
}

void TelegramBot::sendContact(QVariant chatId, QString phoneNumber, QString firstName, QString lastName, int replyToMessageId, TelegramFlags flags, TelegramKeyboardRequest keyboard, TelegramBotMessage *response)
{
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    params.addQueryItem("phone_number", phoneNumber);
    params.addQueryItem("first_name", firstName);
    if(!lastName.isNull()) params.addQueryItem("last_name", lastName);
    if(flags && TelegramFlags::DisableNotfication) params.addQueryItem("disable_notification", "true");
    if(replyToMessageId) params.addQueryItem("reply_to_message_id", QString::number(replyToMessageId));

    // handle reply markup
    this->hanldeReplyMarkup(params, flags, keyboard);

    // call api
    this->callApiTemplate("sendContact", params, response);
}

/*
 * Message Puller
 */
void TelegramBot::startMessagePulling(uint timeout, uint limit, TelegramPollMessageTypes messageTypes, long offset)
{
    // remove webhook
    this->deleteWebhookResult();

    // build url params
    this->pullParams.clear();
    if(offset) this->pullParams.addQueryItem("offset", QString::number(offset));
    else if(this->updateId) this->pullParams.addQueryItem("offset", QString::number(this->updateId));
    this->pullParams.addQueryItem("limit",   QString::number(limit));
    this->pullParams.addQueryItem("timeout", QString::number(timeout));

    // allowed updates
    QStringList allowedUpdates;
    if(static_cast<int>(messageTypes) > 0) {
        if(messageTypes && TelegramPollMessageTypes::Message) allowedUpdates += "message";
        if(messageTypes && TelegramPollMessageTypes::EditedMessage) allowedUpdates += "edited_message";
        if(messageTypes && TelegramPollMessageTypes::ChannelPost) allowedUpdates += "channel_post";
        if(messageTypes && TelegramPollMessageTypes::EditedChannelPost) allowedUpdates += "edited_channel_post";
        if(messageTypes && TelegramPollMessageTypes::InlineQuery) allowedUpdates += "inline_query";
        if(messageTypes && TelegramPollMessageTypes::ChoosenInlineQuery) allowedUpdates += "chosen_inline_result";
        if(messageTypes && TelegramPollMessageTypes::CallbackQuery) allowedUpdates += "callback_query";
    }
    if(!allowedUpdates.isEmpty()) this->pullParams.addQueryItem("allowed_updates", "[\"" + allowedUpdates.join("\",\"") + "\"]");

    // start pulling
    this->pull();
}

void TelegramBot::stopMessagePulling(bool instantly)
{
    this->pullParams.clear();
    if(instantly && this->replyPull) this->replyPull->abort();
}

void TelegramBot::pull()
{
    // if we pull is disabled, exit
    if(this->pullParams.isEmpty()) return;

    // cleanup
    if(this->replyPull) this->replyPull->deleteLater();

    // call api
    this->replyPull = this->callApi("getUpdates", this->pullParams, false);
    QObject::connect(this->replyPull, &QNetworkReply::finished, this, &TelegramBot::handlePullResponse);
}

void TelegramBot::handlePullResponse()
{
    // remove update id from request
    this->pullParams.removeQueryItem("offset");

    // parse response
    QByteArray data = this->replyPull->readAll();
    this->parseMessage(data);

    // add update id to request
    if(this->updateId) this->pullParams.addQueryItem("offset", QString::number(this->updateId + 1));

    // continue pulling
    this->pull();
}

/*
 *  Webhook Functions
 */
bool TelegramBot::setHttpServerWebhook(qint16 port, QString pathCert, QString pathPrivateKey, int maxConnections, TelegramPollMessageTypes messageTypes)
{
    // try to acquire httpServer
    HttpServer* httpServer = 0;
    QSslCertificate cert;
    if(this->webHookWebServers.contains(port)) {
        // if existing webhook contains not the same privateKey, inform user and exit
        if(this->webHookWebServers.find(port).value()->isSamePrivateKey(pathPrivateKey)) {
            EXIT_FAILED("TelegramBot::setHttpServerWebhook - It's not possible to set multiple private keys for one webserver, webhook installation failed...")
        }
        httpServer = this->webHookWebServers.find(port).value();

        // add new cert
        cert = httpServer->addCert(pathCert);
        if(cert.isNull()) {
            EXIT_FAILED("TelegramBot::setHttpServerWebhook - Cert file %s is invalid, webhook installation failed...", qPrintable(pathCert))
        }
        if(cert.subjectInfo(QSslCertificate::CommonName).isEmpty()) {
            EXIT_FAILED("TelegramBot::setHttpServerWebhook - Cert don't contain a Common Name (CN), webhook installation failed...");
        }
    }

    // if no webserver exist, create it
    else {
        // create new http server and register it for auto scope deletion if an error occours
        httpServer = new HttpServer;
        QScopedPointer<HttpServer> scopedHttpServer(httpServer);

        // handle certificates
        cert = httpServer->addCert(pathCert);
        if(cert.isNull()) {
            EXIT_FAILED("TelegramBot::setHttpServerWebhook - Cert file %s is invalid, webhook installation failed...", qPrintable(pathCert))
        }
        if(cert.subjectInfo(QSslCertificate::CommonName).isEmpty()) {
            EXIT_FAILED("TelegramBot::setHttpServerWebhook - Cert don't contain a Common Name (CN), webhook installation failed...")
        }
        if(!httpServer->setPrivateKey(pathPrivateKey)) {
            EXIT_FAILED("TelegramBot::setHttpServerWebhook - Private Key file %s is invalid, webhook installation failed...", qPrintable(pathPrivateKey))
        }

        // permit only telegram connections
        httpServer->addWhiteListHostSubnet("149.154.164.0/22");

        // start listener
        if(!httpServer->listen(QHostAddress::Any, port)) {
            EXIT_FAILED("TelegramBot::setHttpServerWebhook - Cannot listen on port %i, webhook installation failed...", port)
        }

        // everything is okay, so register http server
        this->webHookWebServers.insert(port, scopedHttpServer.take());
    }

    // simplify data
    QString host = cert.subjectInfo(QSslCertificate::CommonName).first();

    // add rewrite rule
    httpServer->addRewriteRule(host, "/" + this->apiKey, {this, &TelegramBot::handleServerWebhookResponse});

    // build server webhook request
    QUrlQuery query;
    query.addQueryItem("url", "https://" + host + ":" + QString::number(port) + "/" + this->apiKey);
    if(maxConnections) query.addQueryItem("max_connections", QString::number(maxConnections));

    // allowed updates
    QStringList allowedUpdates;
    if(static_cast<int>(messageTypes) > 0) {
        if(messageTypes && TelegramPollMessageTypes::Message) allowedUpdates += "message";
        if(messageTypes && TelegramPollMessageTypes::EditedMessage) allowedUpdates += "edited_message";
        if(messageTypes && TelegramPollMessageTypes::ChannelPost) allowedUpdates += "channel_post";
        if(messageTypes && TelegramPollMessageTypes::EditedChannelPost) allowedUpdates += "edited_channel_post";
        if(messageTypes && TelegramPollMessageTypes::InlineQuery) allowedUpdates += "inline_query";
        if(messageTypes && TelegramPollMessageTypes::ChoosenInlineQuery) allowedUpdates += "chosen_inline_result";
        if(messageTypes && TelegramPollMessageTypes::CallbackQuery) allowedUpdates += "callback_query";
    }
    if(!allowedUpdates.isEmpty()) query.addQueryItem("allowed_updates", "[\"" + allowedUpdates.join("\",\"") + "\"]");

    // build multipart
    QByteArray certContent = cert.toPem();
    QHttpMultiPart *multiPart = this->createUploadFile("certificate", "cert.pem", certContent);

    // call api
    return this->callApiJson("setWebhook", query, multiPart).value("result").toBool();
}

void TelegramBot::deleteWebhook()
{
    this->callApi("deleteWebhook");
}

TelegramBotOperationResult TelegramBot::deleteWebhookResult()
{
    return TelegramBotOperationResult(this->callApiJson("deleteWebhook"));
}

TelegramBotWebHookInfo TelegramBot::getWebhookInfo()
{
    return TelegramBotWebHookInfo(this->callApiJson("getWebhookInfo").value("result").toObject());
}

/*
 *  Message Router functions
 */
void TelegramBot::messageRouterRegister(QString startWith, QDelegate<bool(TelegramBotUpdate)> delegate, TelegramBotMessageType type)
{
    // save message route
    this->messageRoutes.append(new MessageRoute {
                                   type,
                                   startWith,
                                   delegate
                               });
}

/*
 * Reponse Parser
 */
void TelegramBot::parseMessage(QByteArray &data, bool singleMessage)
{
    // parse result
    QJsonParseError jError;
    QJsonObject oUpdate = QJsonDocument::fromJson(data, &jError).object();

    // handle parse error
    if(jError.error != QJsonParseError::NoError) {
        return (void)qDebug("TelegramBot::parseMessage - Parse Error: %s", qPrintable(jError.errorString()));
    }

    if (!singleMessage && !JsonHelper::jsonPathGet(oUpdate, "ok").toBool()) {
        return (void)qDebug("TelegramBot::parseMessage - Receive Error: %i - %s",
                            JsonHelper::jsonPathGet(oUpdate, "error_code").toInt(),
                            qPrintable(JsonHelper::jsonPathGet(oUpdate, "description").toString()));
    }

    // loop results
    for(QJsonValue result : singleMessage ? QJsonArray({oUpdate}) : oUpdate.value("result").toArray()) {
        QJsonObject update = result.toObject();

        // parse result
        TelegramBotUpdate updateMessage(new TelegramBotUpdatePrivate);
        updateMessage->fromJson(update);

        // save update id
        this->updateId = updateMessage->updateId;

        // send Message to outside world
        emit this->newMessage(updateMessage);

        // call message routes
        QString routeData = updateMessage->inlineQuery          ? updateMessage->inlineQuery->query :
                            updateMessage->chosenInlineResult   ? updateMessage->chosenInlineResult->query :
                            updateMessage->callbackQuery        ? updateMessage->callbackQuery->data :
                            updateMessage->message              ? updateMessage->message->text : QString();
        if(routeData.isNull()) continue;
        for(auto itrRoute = this->messageRoutes.begin(); itrRoute != this->messageRoutes.end(); itrRoute++) {
            MessageRoute* route = *itrRoute;
            if(route->type && updateMessage->type != updateMessage->type) continue;
            if(!routeData.startsWith(route->startWith)) continue;
            if(!route->delegate.invoke(updateMessage).first()) break;
        }
    }
}

void TelegramBot::handleServerWebhookResponse(HttpServerRequest request, HttpServerResponse response)
{
	// parse response
    this->parseMessage(request->content, true);

	// reply to server with status OK
    response->status = HttpServerResponsePrivate::OK;
}


/*
 * Call Api Helpers
 */
template<typename T>
typename std::enable_if<std::is_base_of<TelegramBotObject, T>::value>::type TelegramBot::callApiTemplate(QString method, QUrlQuery params, T* response, QHttpMultiPart* multiPart)
{
    // if no response was provided, just call the api
    if(!response) return (void)this->callApi(method, params, true, multiPart);

    // get result and parse it
    QJsonObject object = QJsonObject(this->callApiJson(method, params, multiPart)).value("result").toObject();
    response->fromJson(object);
}

template<typename T>
typename std::enable_if<!std::is_base_of<TelegramBotObject, T>::value>::type TelegramBot::callApiTemplate(QString method, QUrlQuery params, T* response, QHttpMultiPart* multiPart)
{
    // if no response was provided, just call the api
    if(!response) return (void)this->callApi(method, params, true, multiPart);

    // get result and (if possible) convert it to T
    QVariant result = QJsonObject(this->callApiJson(method, params, multiPart)).value("result").toVariant();
    if(result.canConvert<T>()) *response = result.value<T>();
}


QNetworkReply* TelegramBot::callApi(QString method, QUrlQuery params, bool deleteOnFinish, QHttpMultiPart *multiPart)
{
    // build url
    QUrl url(QString("https://api.telegram.org/bot%1/%2").arg(this->apiKey, method));
    url.setQuery(params);

    qDebug() << url;

    // execute
    QNetworkRequest request(url);
    QNetworkReply* reply = multiPart ? this->aManager.post(request, multiPart) : this->aManager.get(request);
    if(multiPart) multiPart->setParent(reply);
    if(deleteOnFinish) QObject::connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    return reply;
}

QJsonObject TelegramBot::callApiJson(QString method, QUrlQuery params, QHttpMultiPart *multiPart)
{
    // exec request
    QNetworkReply* reply = this->callApi(method, params, true, multiPart);

    // wait async for answer
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    // parse answer
    return QJsonDocument::fromJson(reply->readAll()).object();
}

QHttpMultiPart* TelegramBot::createUploadFile(QString name, QString fileName, QByteArray &content, bool detectMimeType, QHttpMultiPart *multiPart)
{
    // construct instance if not provided
    if(!multiPart) multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    // append multipart multipart
    QHttpPart contentPart;
    contentPart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data; name=\"%1\"; filename=\"%2\"").arg(name, fileName));
    contentPart.setHeader(QNetworkRequest::ContentTypeHeader, detectMimeType ? QMimeDatabase().mimeTypeForData(content).name() : QString("application/octet-stream"));
    contentPart.setBody(content);
    multiPart->append(contentPart);

    return multiPart;
}

void TelegramBot::hanldeReplyMarkup(QUrlQuery& params, TelegramFlags flags, TelegramKeyboardRequest &keyboard)
{
    // handle types
    QString replyMarkup;
    if(flags && TelegramFlags::ForceReply) {
        replyMarkup = "{\"force_reply\":true";
        if(flags && TelegramFlags::Selective) replyMarkup += ",\"selective\":true";
        replyMarkup += "}";
    }

    else if(flags && TelegramFlags::ReplyKeyboardRemove) {
        replyMarkup = "{\"remove_keyboard\":true";
        if(flags && TelegramFlags::Selective) replyMarkup += ",\"selective\":true";
        replyMarkup += "}";
    }

    // build keyboard
    else if(!keyboard.isEmpty()) {
        QString keyboardContent = "[";
        bool firstRow = true;
        for(QList<TelegramBotKeyboardButtonRequest>& row : keyboard) {
            if(!firstRow) keyboardContent += ",";
            keyboardContent += "[";
            bool firstColumn = true;
            for(TelegramBotKeyboardButtonRequest& column : row) {
                keyboardContent += QString("%1{\"text\":\"%2\"").arg(firstColumn ? "" : ",", column.text);
                if(flags && TelegramFlags::ReplyKeyboardMarkup) {
                    if(column.requestContact) keyboardContent += ",\"request_contact\":true";
                    if(column.requestLocation) keyboardContent += ",\"request_location\":true";
                } else {
                    if(!column.url.isEmpty()) keyboardContent += QString(",\"url\":\"%1\"").arg(column.url);
                    if(!column.callbackData.isEmpty()) keyboardContent += QString(",\"callback_data\":\"%1\"").arg(column.callbackData);
                    if(!column.switchInlineQuery.isEmpty()) keyboardContent += QString(",\"switch_inline_query\":\"%1\"").arg(column.switchInlineQuery);
                    if(!column.switchInlineQueryCurrentChat.isEmpty()) keyboardContent += QString(",\"switch_inline_query_current_chat\":\"%1\"").arg(column.switchInlineQueryCurrentChat);
                }
                keyboardContent += "}";
                firstColumn = false;
            }
            keyboardContent += "]";
            firstRow = false;
        }
        keyboardContent += "]";

        if(flags && TelegramFlags::ReplyKeyboardMarkup) {
            replyMarkup += "{\"keyboard\":" + keyboardContent;
            if(flags && TelegramFlags::ResizeKeyboard) replyMarkup += ",\"resize_keyboard\":true";
            if(flags && TelegramFlags::OneTimeKeyboard) replyMarkup += ",\"one_time_keyboard\":true";
            if(flags && TelegramFlags::Selective) replyMarkup += ",\"selective\":true";
        } else {
            replyMarkup += "{\"inline_keyboard\":" + keyboardContent;
        }
        replyMarkup += "}";
    }
    if(!replyMarkup.isEmpty()) params.addQueryItem("reply_markup", replyMarkup);
}

QHttpMultiPart* TelegramBot::handleFile(QString fieldName, QVariant file, QUrlQuery &params, QHttpMultiPart* multiPart)
{
    // handle content
    if(file.type() == QVariant::ByteArray) {
        QByteArray content = file.value<QByteArray>();
        multiPart = this->createUploadFile(fieldName, fieldName, content, true, multiPart);
    }

    // handle url
    else if(file.type() == QVariant::String) {
        QUrl url = QUrl::fromUserInput(file.toString());

        // upload the local file to telegram
        if(url.isLocalFile() || url.isRelative()) {
            QFile fFile(file.toString());
            if(!fFile.open(QFile::ReadOnly)) {
                qWarning("TelegramBot::handleFile - Cannot open file \"%s\"", qPrintable(file.toString()));
                return multiPart;
            }
            QByteArray content = fFile.readAll();
            QFileInfo fInfo(fFile);
            multiPart = this->createUploadFile(fieldName, fInfo.fileName(), content, true, multiPart);
        }

        // we have a link given, so just set it
        else {
            params.addQueryItem(fieldName, file.toString());
        }
    }

    // otherwise we interpret it as telegram file id
    else {
        params.addQueryItem(fieldName, file.toString());
    }

    return multiPart;
}
