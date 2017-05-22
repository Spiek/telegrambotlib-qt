#include "telegrambot.h"

QMap<qint16, HttpServer*> TelegramBot::webHookWebServers = QMap<qint16, HttpServer*>();

TelegramBot::TelegramBot(QString apikey, QObject *parent) : QObject(parent), apiKey(apikey) { }

void TelegramBot::sendMessage(QVariant chatId, QString text, TelegramFlags flags, int replyToMessageId, TelegramKeyboardRequest keyboard)
{
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    params.addQueryItem("text", text);

    // handle flags
    if(flags && TelegramFlags::Markdown) params.addQueryItem("parse_mode", "Markdown");
    else if(flags && TelegramFlags::Html) params.addQueryItem("parse_mode", "HTML");

    if(flags && TelegramFlags::DisableWebPagePreview) params.addQueryItem("disable_web_page_preview", "true");

    if(flags && TelegramFlags::DisableNotfication) params.addQueryItem("disable_notification", "true");

    if(replyToMessageId) params.addQueryItem("reply_to_message_id", QString::number(replyToMessageId));

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
                    if(!column.requestContact) keyboardContent += QString(",\"request_contact\":%1").arg(column.requestContact ? "true" : "false");
                    if(!column.requestLocation) keyboardContent += QString(",\"request_location\":%1").arg(column.requestLocation ? "true" : "false");
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

    // call api
    this->callApi("sendMessage", params);
}


/*
 * Message Puller
 */
void TelegramBot::startMessagePulling(uint timeout, uint limit, TelegramPollMessageTypes messageTypes, long offset)
{
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
    this->pullParams.addQueryItem("offset", QString::number(++this->updateId));

    // continue pulling
    this->pull();
}

/*
 *  Message Poller
 */
void TelegramBot::setHttpServerWebhook(qint16 port, QString pathCert, QString pathPrivateKey, int maxConnections, TelegramPollMessageTypes messageTypes)
{
    // try to acquire httpServer
    HttpServer* httpServer = 0;
    QSslCertificate cert;
    if(this->webHookWebServers.contains(port)) {
        // if existing webhook contains not the same privateKey, inform user and exit
        if(this->webHookWebServers.find(port).value()->isSamePrivateKey(pathPrivateKey)) {
            return (void)qWarning("TelegramBot::setHttpServerWebhook - It's not possible to set multiple private keys for one webserver, webhook installation failed...");
        }
        httpServer = this->webHookWebServers.find(port).value();

        // add new cert
        cert = httpServer->addCert(pathCert);
        if(cert.isNull()) {
            return (void)qWarning("TelegramBot::setHttpServerWebhook - Cert file %s is invalid, webhook installation failed...", qPrintable(pathCert));
        }
        if(cert.subjectInfo(QSslCertificate::CommonName).isEmpty()) {
            return (void)qWarning("TelegramBot::setHttpServerWebhook - Cert don't contain a Common Name (CN), webhook installation failed...");
        }
    }

    // if no webserver exist, create it
    else {
        httpServer = this->webHookWebServers.insert(port, new HttpServer()).value();
        cert = httpServer->addCert(pathCert);
        if(cert.isNull()) {
            delete this->webHookWebServers.take(port);
            return (void)qWarning("TelegramBot::setHttpServerWebhook - Cert file %s is invalid, webhook installation failed...", qPrintable(pathCert));
        }
        if(cert.subjectInfo(QSslCertificate::CommonName).isEmpty()) {
            delete this->webHookWebServers.take(port);
            return (void)qWarning("TelegramBot::setHttpServerWebhook - Cert don't contain a Common Name (CN), webhook installation failed...");
        }
        if(!httpServer->setPrivateKey(pathPrivateKey)) {
            delete this->webHookWebServers.take(port);
            return (void)qWarning("TelegramBot::setHttpServerWebhook - Private Key file %s is invalid, webhook installation failed...", qPrintable(pathPrivateKey));
        }

        // permit only telegram connections
        httpServer->addWhiteListHostSubnet("149.154.164.0/22");

        // start listener
        if(!httpServer->listen(QHostAddress::Any, port)) {
            delete this->webHookWebServers.take(port);
            return (void)qWarning("TelegramBot::setHttpServerWebhook - Cannot listen on port %i, webhook installation failed...", port);
        }
    }


    // simplify data
    QString host = cert.subjectInfo(QSslCertificate::CommonName).first();

    // add rewrite rule
    httpServer->addRewriteRule(host, "/" + this->apiKey, {this, &TelegramBot::handleServerPollResponse});

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
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart textPart;
    textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"certificate\"; filename=\"cert.pem\""));
    textPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    textPart.setBody(cert.toPem());
    multiPart->append(textPart);

    // call api
    this->callApi("setWebhook", query, true, multiPart);
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
    if(jError.error != QJsonParseError::NoError || (!singleMessage && !JsonHelper::jsonPathGet(oUpdate, "ok").toBool())) {
        return (void)qDebug("TelegramBot::parseMessage - Parse Error: %s", qPrintable(jError.errorString()));
    }

    // loop results
    for(QJsonValue result : singleMessage ? QJsonArray({oUpdate}) : oUpdate.value("result").toArray()) {
        QJsonObject update = result.toObject();
        for(auto itr = update.begin(); itr != update.end(); itr++) {
            // handle update id
            if(itr.key() == "update_id") {
                JsonHelper::jsonPathGet(update, "update_id", this->updateId);
                continue;
            }

            // simplify object
            QJsonObject oMessage = update.value(itr.key()).toObject();

            // handle message fields
            if(itr.key().endsWith("message") || itr.key().endsWith("post")) {
                TelegramBotMessage message;
                message.fromJson(oMessage);
                message.type = itr.key() == "message"             ? TelegramBotMessage::Message :
                               itr.key() == "edited_message"      ? TelegramBotMessage::EditedMessage :
                               itr.key() == "channel_post"        ? TelegramBotMessage::ChannelPost :
                               itr.key() == "edited_channel_post" ? TelegramBotMessage::EditedChannelPost :
                                                                    TelegramBotMessage::Undefined;
                emit this->newMessage(message);
            }

            // handle inline Query
            else if(itr.key() == "inline_query") {
                TelegramBotInlineQuery query;
                query.fromJson(oMessage);
                emit this->newInlineQuery(query);
            }

            // handle choosen inline result
            else if(itr.key() == "chosen_inline_result") {
                TelegramBotChosenInlineResult inlineResult;
                inlineResult.fromJson(oMessage);
                emit this->newChoosenInlineResult(inlineResult);
            }

            // handle callback Query
            else if(itr.key() == "callback_query") {
                TelegramBotCallbackQuery query;
                query.fromJson(oMessage);
                emit this->newCallbackQuery(query);
            }
        }
    }
}

void TelegramBot::handleServerPollResponse(HttpServerRequest request, HttpServerResponse response)
{
	// parse response
    this->parseMessage(request->content, true);

	// reply to server with status OK
    response->status = HttpServerResponsePrivate::OK;
}


/*
 * Helpers
 */
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
