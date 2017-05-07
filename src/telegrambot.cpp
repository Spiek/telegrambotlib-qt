#include "telegrambot.h"

TelegramBot::TelegramBot(QString apikey, QObject *parent) : QObject(parent), apiKey(apikey) { }

void TelegramBot::sendMessage(QVariant chatId, QString text, TelegramFlags flags, int replyToMessageId, TelegramKeyboard keyboard)
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
        for(QList<TelegramBotKeyboardButton>& row : keyboard) {
            if(!firstRow) keyboardContent += ",";
            keyboardContent += "[";
            bool firstColumn = true;
            for(TelegramBotKeyboardButton& column : row) {
                keyboardContent += QString("%1{\"text\":\"%2\"").arg(firstColumn ? "" : ",", column.text);
                if(flags && TelegramFlags::ReplyKeyboardMarkup) {
                    if(!column.requestContact) keyboardContent += QString(",\"request_contact\":%1").arg(column.requestContact ? "true" : "false");
                    if(!column.requestLocation) keyboardContent += QString(",\"request_location\":%1").arg(column.requestLocation ? "true" : "false");
                } else {
                    if(!column.url.isEmpty()) keyboardContent += QString(",\"url\":\"%1\"").arg(column.url);
                    if(!column.callBackData.isEmpty()) keyboardContent += QString(",\"callback_data\":\"%1\"").arg(column.callBackData);
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
    // parse result
    QJsonParseError jError;
    QJsonObject oUpdate = QJsonDocument::fromJson(this->replyPull->readAll(), &jError).object();

    // handle parse error
    if(jError.error != QJsonParseError::NoError || !JsonHelper::jsonPathGet(oUpdate, "ok").toBool()) {
        return (void)qDebug("TelegramBot::handlePullResponse - Parse Error: %s", qPrintable(jError.errorString()));
    }

    // remove update id from request
    this->pullParams.removeQueryItem("offset");

    // loop results
    for(QJsonValue result : oUpdate.value("result").toArray()) {
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
                qDebug() << message.messageId << message.from.firstName << message.from.username << message.text;
                emit this->newMessage(message);
            }

            // handle inline Query
            else if(itr.key() == "callback_query") {
                TelegramBotCallbackQuery query;
                query.fromJson(oMessage);
                emit this->newCallbackQuery(query);
            }
        }
    }

    // add update id to request
    this->pullParams.addQueryItem("offset", QString::number(++this->updateId));

    // continue pulling
    this->pull();
}

QNetworkReply* TelegramBot::callApi(QString method, QUrlQuery params, bool deleteOnFinish)
{
    // build url
    QUrl url(QString("https://api.telegram.org/bot%1/%2").arg(this->apiKey, method));
    url.setQuery(params);

    qDebug() << url;
    qDebug() << params.toString(QUrl::FullyDecoded);

    // execute
    QNetworkReply* reply = this->aManager.get(QNetworkRequest(url));
    if(deleteOnFinish) QObject::connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    return reply;
}
