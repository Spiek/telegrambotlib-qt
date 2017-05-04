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
        for(QList<TelegramKeyboardButton>& row : keyboard) {
            if(!firstRow) keyboardContent += ",";
            keyboardContent += "[";
            bool firstColumn = true;
            for(TelegramKeyboardButton& column : row) {
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

QNetworkReply* TelegramBot::callApi(QString method, QUrlQuery params)
{
    // build url
    QUrl url(QString("https://api.telegram.org/bot%1/%2").arg(this->apiKey, method));
    url.setQuery(params);

    qDebug() << url;
    qDebug() << params.toString(QUrl::FullyDecoded);

    // execute
    return this->aManager.get(QNetworkRequest(url));
}
