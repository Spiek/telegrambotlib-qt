#ifndef TELEGRAMDATASTRUCTS_H
#define TELEGRAMDATASTRUCTS_H

#include <QString>
#include <QList>

#include "jsonhelper.h"
#include "telegramdatainterface.h"

struct TelegramBotKeyboardButton
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

typedef QList<QList<TelegramBotKeyboardButton>> TelegramKeyboard;

// TelegramBotUser - This object represents a Telegram user or bot.
struct TelegramBotUser : public TelegramBotObject {
    qint32 id; // Unique identifier for this user or bot
    QString firstName; // User‘s or bot’s first name
    QString lastName; // Optional. User‘s or bot’s last name
    QString username; // Optional. User‘s or bot’s username

    // parse logic
    virtual void fromJson(QJsonObject& object) {
        JsonHelperT<qint32>::jsonPathGet(object, "id", this->id);
        JsonHelperT<QString>::jsonPathGet(object, "first_name", this->firstName);
        JsonHelperT<QString>::jsonPathGet(object, "last_name", this->lastName, false);
        JsonHelperT<QString>::jsonPathGet(object, "username", this->username, false);
    }
};

// TelegramBotChat - This object represents a chat.
struct TelegramBotChat : public TelegramBotObject {
    qint32 id; // Unique identifier for this chat. This number may be greater than 32 bits and some programming languages may have difficulty/silent defects in interpreting it. But it is smaller than 52 bits, so a signed 64 bit integer or double-precision float type are safe for storing this identifier.
    QString type; // Type of chat, can be either “private”, “group”, “supergroup” or “channel”
    QString title; // Optional. Title, for supergroups, channels and group chats
    QString username; // Optional. Username, for private chats, supergroups and channels if available
    QString firstName; // Optional. First name of the other party in a private chat
    QString lastName; // Optional. Last name of the other party in a private chat
    bool allMembersAreAdministrators; // Optional. True if a group has ‘All Members Are Admins’ enabled.

    // parse logic
    virtual void fromJson(QJsonObject& object) {
        JsonHelperT<qint32>::jsonPathGet(object, "id", this->id);
        JsonHelperT<QString>::jsonPathGet(object, "type", this->type);
        JsonHelperT<QString>::jsonPathGet(object, "title", this->title, false);
        JsonHelperT<QString>::jsonPathGet(object, "username", this->username, false);
        JsonHelperT<QString>::jsonPathGet(object, "first_name", this->firstName, false);
        JsonHelperT<QString>::jsonPathGet(object, "last_name", this->lastName, false);
        JsonHelperT<bool>::jsonPathGet(object, "all_members_are_administrators", this->allMembersAreAdministrators, false);
    }
};

// TelegramBotMessageEntity - This object represents one special entity in a text message. For example, hashtags, usernames, URLs, etc.
struct TelegramBotMessageEntity : public TelegramBotObject {
    QString type; // Type of the entity. Can be mention (@username), hashtag, bot_command, url, email, bold (bold text), italic (italic text), code (monowidth string), pre (monowidth block), text_link (for clickable text URLs), text_mention (for users without usernames)
    qint32 offset; // Offset in UTF-16 code units to the start of the entity
    qint32 length; // Length of the entity in UTF-16 code units
    QString url; // Optional. For “text_link” only, url that will be opened after user taps on the text
    TelegramBotUser user; // Optional. For “text_mention” only, the mentioned user

    // parse logic
    virtual void fromJson(QJsonObject& object) {
        JsonHelperT<QString>::jsonPathGet(object, "type", this->type);
        JsonHelperT<qint32>::jsonPathGet(object, "offset", this->offset);
        JsonHelperT<qint32>::jsonPathGet(object, "length", this->length);
        JsonHelperT<QString>::jsonPathGet(object, "url", this->url, false);
        JsonHelperT<TelegramBotUser>::jsonPathGet(object, "user", this->user, false);
    }
};

// TelegramBotPhotoSize - This object represents one size of a photo or a file / sticker thumbnail.
struct TelegramBotPhotoSize : public TelegramBotObject {
    QString fileId; // Unique identifier for this file
    qint32 width; // Photo width
    qint32 height; // Photo height
    qint32 fileSize; // Optional. File size

    // parse logic
    virtual void fromJson(QJsonObject& object) {
        JsonHelperT<QString>::jsonPathGet(object, "file_id", this->fileId);
        JsonHelperT<qint32>::jsonPathGet(object, "width", this->width);
        JsonHelperT<qint32>::jsonPathGet(object, "height", this->height);
        JsonHelperT<qint32>::jsonPathGet(object, "file_size", this->fileSize, false);
    }
};

// TelegramBotAudio - This object represents an audio file to be treated as music by the Telegram clients.
struct TelegramBotAudio : public TelegramBotObject {
    QString fileId; // Unique identifier for this file
    qint32 duration; // Duration of the audio in seconds as defined by sender
    QString performer; // Optional. Performer of the audio as defined by sender or by audio tags
    QString title; // Optional. Title of the audio as defined by sender or by audio tags
    QString mimeType; // Optional. MIME type of the file as defined by sender
    qint32 fileSize; // Optional. File size

    // parse logic
    virtual void fromJson(QJsonObject& object) {
        JsonHelperT<QString>::jsonPathGet(object, "file_id", this->fileId);
        JsonHelperT<qint32>::jsonPathGet(object, "duration", this->duration);
        JsonHelperT<QString>::jsonPathGet(object, "performer", this->performer, false);
        JsonHelperT<QString>::jsonPathGet(object, "title", this->title, false);
        JsonHelperT<QString>::jsonPathGet(object, "mime_type", this->mimeType, false);
        JsonHelperT<qint32>::jsonPathGet(object, "file_size", this->fileSize, false);
    }
};

// TelegramBotDocument - This object represents a general file (as opposed to photos, voice messages and audio files).
struct TelegramBotDocument : public TelegramBotObject {
    QString fileId; // Unique file identifier
    TelegramBotPhotoSize thumb; // Optional. Document thumbnail as defined by sender
    QString fileName; // Optional. Original filename as defined by sender
    QString mimeType; // Optional. MIME type of the file as defined by sender
    qint32 fileSize; // Optional. File size

    // parse logic
    virtual void fromJson(QJsonObject& object) {
        JsonHelperT<QString>::jsonPathGet(object, "file_id", this->fileId);
        JsonHelperT<TelegramBotPhotoSize>::jsonPathGet(object, "thumb", this->thumb, false);
        JsonHelperT<QString>::jsonPathGet(object, "file_name", this->fileName, false);
        JsonHelperT<QString>::jsonPathGet(object, "mime_type", this->mimeType, false);
        JsonHelperT<qint32>::jsonPathGet(object, "file_size", this->fileSize, false);
    }
};

// TelegramBotSticker - This object represents a sticker.
struct TelegramBotSticker : public TelegramBotObject {
    QString fileId; // Unique identifier for this file
    qint32 width; // Sticker width
    qint32 height; // Sticker height
    TelegramBotPhotoSize thumb; // Optional. Sticker thumbnail in .webp or .jpg format
    QString emoji; // Optional. Emoji associated with the sticker
    qint32 fileSize; // Optional. File size

    // parse logic
    virtual void fromJson(QJsonObject& object) {
        JsonHelperT<QString>::jsonPathGet(object, "file_id", this->fileId);
        JsonHelperT<qint32>::jsonPathGet(object, "width", this->width);
        JsonHelperT<qint32>::jsonPathGet(object, "height", this->height);
        JsonHelperT<TelegramBotPhotoSize>::jsonPathGet(object, "thumb", this->thumb, false);
        JsonHelperT<QString>::jsonPathGet(object, "emoji", this->emoji, false);
        JsonHelperT<qint32>::jsonPathGet(object, "file_size", this->fileSize, false);
    }
};

// TelegramBotVideo - This object represents a video file.
struct TelegramBotVideo : public TelegramBotObject {
    QString fileId; // Unique identifier for this file
    qint32 width; // Video width as defined by sender
    qint32 height; // Video height as defined by sender
    qint32 duration; // Duration of the video in seconds as defined by sender
    TelegramBotPhotoSize thumb; // Optional. Video thumbnail
    QString mimeType; // Optional. Mime type of a file as defined by sender
    qint32 fileSize; // Optional. File size

    // parse logic
    virtual void fromJson(QJsonObject& object) {
        JsonHelperT<QString>::jsonPathGet(object, "file_id", this->fileId);
        JsonHelperT<qint32>::jsonPathGet(object, "width", this->width);
        JsonHelperT<qint32>::jsonPathGet(object, "height", this->height);
        JsonHelperT<qint32>::jsonPathGet(object, "duration", this->duration);
        JsonHelperT<TelegramBotPhotoSize>::jsonPathGet(object, "thumb", this->thumb, false);
        JsonHelperT<QString>::jsonPathGet(object, "mime_type", this->mimeType, false);
        JsonHelperT<qint32>::jsonPathGet(object, "file_size", this->fileSize, false);
    }
};

// TelegramBotVoice - This object represents a voice note.
struct TelegramBotVoice : public TelegramBotObject {
    QString fileId; // Unique identifier for this file
    qint32 duration; // Duration of the audio in seconds as defined by sender
    QString mimeType; // Optional. MIME type of the file as defined by sender
    qint32 fileSize; // Optional. File size

    // parse logic
    virtual void fromJson(QJsonObject& object) {
        JsonHelperT<QString>::jsonPathGet(object, "file_id", this->fileId);
        JsonHelperT<qint32>::jsonPathGet(object, "duration", this->duration);
        JsonHelperT<QString>::jsonPathGet(object, "mime_type", this->mimeType, false);
        JsonHelperT<qint32>::jsonPathGet(object, "file_size", this->fileSize, false);
    }
};

// TelegramBotContact - This object represents a phone contact.
struct TelegramBotContact : public TelegramBotObject {
    QString phoneNumber; // Contact's phone number
    QString firstName; // Contact's first name
    QString lastName; // Optional. Contact's last name
    qint32 userId; // Optional. Contact's user identifier in Telegram

    // parse logic
    virtual void fromJson(QJsonObject& object) {
        JsonHelperT<QString>::jsonPathGet(object, "phone_number", this->phoneNumber);
        JsonHelperT<QString>::jsonPathGet(object, "first_name", this->firstName);
        JsonHelperT<QString>::jsonPathGet(object, "last_name", this->lastName, false);
        JsonHelperT<qint32>::jsonPathGet(object, "user_id", this->userId, false);
    }
};

// TelegramBotLocation - This object represents a point on the map.
struct TelegramBotLocation : public TelegramBotObject {
    double longitude; // Longitude as defined by sender
    double latitude; // Latitude as defined by sender

    // parse logic
    virtual void fromJson(QJsonObject& object) {
        JsonHelperT<double>::jsonPathGet(object, "longitude", this->longitude);
        JsonHelperT<double>::jsonPathGet(object, "latitude", this->latitude);
    }
};

// TelegramBotVenue - This object represents a venue.
struct TelegramBotVenue : public TelegramBotObject {
    TelegramBotLocation location; // Venue location
    QString title; // Name of the venue
    QString address; // Address of the venue
    QString foursquareId; // Optional. Foursquare identifier of the venue

    // parse logic
    virtual void fromJson(QJsonObject& object) {
        JsonHelperT<TelegramBotLocation>::jsonPathGet(object, "location", this->location);
        JsonHelperT<QString>::jsonPathGet(object, "title", this->title);
        JsonHelperT<QString>::jsonPathGet(object, "address", this->address);
        JsonHelperT<QString>::jsonPathGet(object, "foursquare_id", this->foursquareId, false);
    }
};

// This object represent a user's profile pictures.
struct UserProfilePhotos : public TelegramBotObject {
    qint32 totalCount; // Total number of profile pictures the target user has
    QList<QList<TelegramBotPhotoSize>> photos; // Requested profile pictures (in up to 4 sizes each)

    // parse logic
    virtual void fromJson(QJsonObject& object) {
        JsonHelperT<qint32>::jsonPathGet(object, "total_count", this->totalCount);
        JsonHelperT<TelegramBotPhotoSize>::jsonPathGetArrayArray(object, "photos", this->photos);
    }
};

// TelegramBotFile - This object represents a file ready to be downloaded. The file can be downloaded via the link https://api.telegram.org/file/bot<token>/<file_path>. It is guaranteed that the link will be valid for at least 1 hour. When the link expires, a new one can be requested by calling getFile. Maximum file size to download is 20 MB
struct TelegramBotFile : public TelegramBotObject {
    QString fileId; // Unique identifier for this file
    qint32 fileSize; // Optional. File size, if known
    QString filePath; // Optional. File path. Use https://api.telegram.org/file/bot<token>/<file_path> to get the file.

    // parse logic
    virtual void fromJson(QJsonObject& object) {
        JsonHelperT<QString>::jsonPathGet(object, "file_id", this->fileId);
        JsonHelperT<qint32>::jsonPathGet(object, "file_size", this->fileSize, false);
        JsonHelperT<QString>::jsonPathGet(object, "file_path", this->filePath, false);
    }
};

// TelegramBotChatMember - This object contains information about one member of the chat.
struct TelegramBotChatMember : public TelegramBotObject {
    TelegramBotUser user; // Information about the user
    QString status; // The member's status in the chat. Can be “creator”, “administrator”, “member”, “left” or “kicked”

    // parse logic
    virtual void fromJson(QJsonObject& object) {
        JsonHelperT<TelegramBotUser>::jsonPathGet(object, "user", this->user);
        JsonHelperT<QString>::jsonPathGet(object, "status", this->status);
    }
};

// TelegramBotResponseParameters - Contains information about why a request was unsuccessfull.
struct TelegramBotResponseParameters : public TelegramBotObject {
    qint32 migrateToChatId; // Optional. The group has been migrated to a supergroup with the specified identifier. This number may be greater than 32 bits and some programming languages may have difficulty/silent defects in interpreting it. But it is smaller than 52 bits, so a signed 64 bit integer or double-precision float type are safe for storing this identifier.
    qint32 retryAfter; // Optional. In case of exceeding flood control, the number of seconds left to wait before the request can be repeated

    // parse logic
    virtual void fromJson(QJsonObject& object) {
        JsonHelperT<qint32>::jsonPathGet(object, "migrate_to_chat_id", this->migrateToChatId, false);
        JsonHelperT<qint32>::jsonPathGet(object, "retry_after", this->retryAfter, false);
    }
};

#define TELEGRAMBOTMESSAGE_FIELDS \
    qint32 messageId; /* Unique message identifier inside this chat */\
    TelegramBotUser from; /* Optional. Sender, can be empty for messages sent to channels */\
    qint32 date; /* Date the message was sent in Unix time */\
    TelegramBotChat chat; /* Conversation the message belongs to */\
    TelegramBotUser forwardFrom; /* Optional. For forwarded messages, sender of the original message */\
    TelegramBotChat forwardFromChat; /* Optional. For messages forwarded from a channel, information about the original channel */\
    qint32 forwardFromMessageId; /* Optional. For forwarded channel posts, identifier of the original message in the channel */\
    qint32 forwardDate; /* Optional. For forwarded messages, date the original message was sent in Unix time */\
    qint32 editDate; /* Optional. Date the message was last edited in Unix time */\
    QString text; /* Optional. For text messages, the actual UTF-8 text of the message, 0-4096 characters. */\
    QList<TelegramBotMessageEntity> entities; /* Optional. For text messages, special entities like usernames, URLs, bot commands, etc. that appear in the text */\
    TelegramBotAudio audio; /* Optional. Message is an audio file, information about the file */\
    TelegramBotDocument document; /* Optional. Message is a general file, information about the file */\
    /*Game game; // Optional. Message is a game, information about the game. More about games */\
    QList<TelegramBotPhotoSize> photo; /* Optional. Message is a photo, available sizes of the photo */\
    TelegramBotSticker sticker; /* Optional. Message is a sticker, information about the sticker */\
    TelegramBotVideo video; /* Optional. Message is a video, information about the video */\
    TelegramBotVoice voice; /* Optional. Message is a voice message, information about the file */\
    QString caption; /* Optional. Caption for the document, photo or video, 0-200 characters */\
    TelegramBotContact contact; /* Optional. Message is a shared contact, information about the contact */\
    TelegramBotLocation location; /* Optional. Message is a shared location, information about the location */\
    TelegramBotVenue venue; /* Optional. Message is a venue, information about the venue */\
    TelegramBotUser newChatMember; /* Optional. A new member was added to the group, information about them (this member may be the bot itself) */\
    TelegramBotUser leftChatMember; /* Optional. A member was removed from the group, information about them (this member may be the bot itself) */\
    QString newChatTitle; /* Optional. A chat title was changed to this value */\
    QList<TelegramBotPhotoSize> newChatPhoto; /* Optional. A chat photo was change to this value */\
    bool deleteChatPhoto = false; /* Optional. Service message: the chat photo was deleted */\
    bool groupChatCreated = false; /* Optional. Service message: the group has been created */\
    bool supergroupChatCreated = false; /* Optional. Service message: the supergroup has been created. This field can‘t be received in a message coming through updates, because bot can’t be a member of a supergroup when it is created. It can only be found in reply_to_message if someone replies to a very first message in a directly created supergroup. */\
    bool channelChatCreated = false; /* Optional. Service message: the channel has been created. This field can‘t be received in a message coming through updates, because bot can’t be a member of a channel when it is created. It can only be found in reply_to_message if someone replies to a very first message in a channel. */\
    qint32 migrateToChatId; /* Optional. The group has been migrated to a supergroup with the specified identifier. This number may be greater than 32 bits and some programming languages may have difficulty/silent defects in interpreting it. But it is smaller than 52 bits, so a signed 64 bit integer or double-precision float type are safe for storing this identifier. */\
    qint32 migrateFromChatId; /* Optional. The supergroup has been migrated from a group with the specified identifier. This number may be greater than 32 bits and some programming languages may have difficulty/silent defects in interpreting it. But it is smaller than 52 bits, so a signed 64 bit integer or double-precision float type are safe for storing this identifier. */\

// TelegramBotMessageSingle - This object represents a message (without any recursive fields which references to same class, see TelegramBotMessage for complete message)
struct TelegramBotMessageSingle : public TelegramBotObject {
    TELEGRAMBOTMESSAGE_FIELDS

    // parse logic
    virtual void fromJson(QJsonObject& object) {
        JsonHelperT<qint32>::jsonPathGet(object, "message_id", this->messageId);
        JsonHelperT<TelegramBotUser>::jsonPathGet(object, "from", this->from, false);
        JsonHelperT<qint32>::jsonPathGet(object, "date", this->date);
        JsonHelperT<TelegramBotChat>::jsonPathGet(object, "chat", this->chat, false);
        JsonHelperT<TelegramBotUser>::jsonPathGet(object, "forward_from", this->forwardFrom, false);
        JsonHelperT<TelegramBotChat>::jsonPathGet(object, "forward_from_chat", this->forwardFromChat, false);
        JsonHelperT<qint32>::jsonPathGet(object, "forward_from_message_id", this->forwardFromMessageId, false);
        JsonHelperT<qint32>::jsonPathGet(object, "forward_date", this->forwardDate, false);
        JsonHelperT<qint32>::jsonPathGet(object, "edit_date", this->editDate, false);
        JsonHelperT<QString>::jsonPathGet(object, "text", this->text, false);
        JsonHelperT<TelegramBotMessageEntity>::jsonPathGetArray(object, "entities", this->entities, false);
        JsonHelperT<TelegramBotAudio>::jsonPathGet(object, "audio", this->audio, false);
        JsonHelperT<TelegramBotDocument>::jsonPathGet(object, "document", this->document, false);
        //JsonHelperT<Game>::jsonPathGet(object, "game", this->game);
        JsonHelperT<TelegramBotPhotoSize>::jsonPathGetArray(object, "photo", this->photo, false);
        JsonHelperT<TelegramBotSticker>::jsonPathGet(object, "sticker", this->sticker, false);
        JsonHelperT<TelegramBotVideo>::jsonPathGet(object, "video", this->video, false);
        JsonHelperT<TelegramBotVoice>::jsonPathGet(object, "voice", this->voice, false);
        JsonHelperT<QString>::jsonPathGet(object, "caption", this->caption, false);
        JsonHelperT<TelegramBotContact>::jsonPathGet(object, "contact", this->contact, false);
        JsonHelperT<TelegramBotLocation>::jsonPathGet(object, "location", this->location, false);
        JsonHelperT<TelegramBotVenue>::jsonPathGet(object, "venue", this->venue, false);
        JsonHelperT<TelegramBotUser>::jsonPathGet(object, "new_chat_member", this->newChatMember, false);
        JsonHelperT<TelegramBotUser>::jsonPathGet(object, "left_chat_member", this->leftChatMember, false);
        JsonHelperT<QString>::jsonPathGet(object, "new_chat_title", this->newChatTitle, false);
        JsonHelperT<TelegramBotPhotoSize>::jsonPathGetArray(object, "new_chat_photo", this->newChatPhoto, false);
        JsonHelperT<bool>::jsonPathGet(object, "delete_chat_photo = false", this->deleteChatPhoto, false);
        JsonHelperT<bool>::jsonPathGet(object, "group_chat_created = false", this->groupChatCreated, false);
        JsonHelperT<bool>::jsonPathGet(object, "supergroup_chat_created = false", this->supergroupChatCreated, false);
        JsonHelperT<bool>::jsonPathGet(object, "channel_chat_created = false", this->channelChatCreated, false);
        JsonHelperT<qint32>::jsonPathGet(object, "migrate_to_chat_id", this->migrateToChatId, false);
        JsonHelperT<qint32>::jsonPathGet(object, "migrate_from_chat_id", this->migrateFromChatId, false);
    }
};

// TelegramBotMessage - This object represents a message.
struct TelegramBotMessage : public TelegramBotObject {
    TELEGRAMBOTMESSAGE_FIELDS
    enum Type {
        Undefined,
        Message,
        EditedMessage,
        ChannelPost,
        EditedChannelPost,
        InlineQuery,
        ChosenInlineResult,
        CallbackQuery
    } type = Undefined;
    TelegramBotMessageSingle replyToMessage; // Optional. For replies, the original message. Note that the Message object in this field will not contain further reply_to_message fields even if it itself is a reply.
    TelegramBotMessageSingle pinnedMessage; // Optional. Specified message was pinned. Note that the Message object in this field will not contain further reply_to_message fields even if it is itself a reply.

    // parse logic
    virtual void fromJson(QJsonObject& object) {
        // Message
        JsonHelperT<qint32>::jsonPathGet(object, "message_id", this->messageId);
        JsonHelperT<TelegramBotUser>::jsonPathGet(object, "from", this->from, false);
        JsonHelperT<qint32>::jsonPathGet(object, "date", this->date);
        JsonHelperT<TelegramBotChat>::jsonPathGet(object, "chat", this->chat, false);
        JsonHelperT<TelegramBotUser>::jsonPathGet(object, "forward_from", this->forwardFrom, false);
        JsonHelperT<TelegramBotChat>::jsonPathGet(object, "forward_from_chat", this->forwardFromChat, false);
        JsonHelperT<qint32>::jsonPathGet(object, "forward_from_message_id", this->forwardFromMessageId, false);
        JsonHelperT<qint32>::jsonPathGet(object, "forward_date", this->forwardDate, false);
        JsonHelperT<qint32>::jsonPathGet(object, "edit_date", this->editDate, false);
        JsonHelperT<QString>::jsonPathGet(object, "text", this->text, false);
        JsonHelperT<TelegramBotMessageEntity>::jsonPathGetArray(object, "entities", this->entities, false);
        JsonHelperT<TelegramBotAudio>::jsonPathGet(object, "audio", this->audio, false);
        JsonHelperT<TelegramBotDocument>::jsonPathGet(object, "document", this->document, false);
        //JsonHelperT<Game>::jsonPathGet(object, "game", this->game);
        JsonHelperT<TelegramBotPhotoSize>::jsonPathGetArray(object, "photo", this->photo, false);
        JsonHelperT<TelegramBotSticker>::jsonPathGet(object, "sticker", this->sticker, false);
        JsonHelperT<TelegramBotVideo>::jsonPathGet(object, "video", this->video, false);
        JsonHelperT<TelegramBotVoice>::jsonPathGet(object, "voice", this->voice, false);
        JsonHelperT<QString>::jsonPathGet(object, "caption", this->caption, false);
        JsonHelperT<TelegramBotContact>::jsonPathGet(object, "contact", this->contact, false);
        JsonHelperT<TelegramBotLocation>::jsonPathGet(object, "location", this->location, false);
        JsonHelperT<TelegramBotVenue>::jsonPathGet(object, "venue", this->venue, false);
        JsonHelperT<TelegramBotUser>::jsonPathGet(object, "new_chat_member", this->newChatMember, false);
        JsonHelperT<TelegramBotUser>::jsonPathGet(object, "left_chat_member", this->leftChatMember, false);
        JsonHelperT<QString>::jsonPathGet(object, "new_chat_title", this->newChatTitle, false);
        JsonHelperT<TelegramBotPhotoSize>::jsonPathGetArray(object, "new_chat_photo", this->newChatPhoto, false);
        JsonHelperT<bool>::jsonPathGet(object, "delete_chat_photo = false", this->deleteChatPhoto, false);
        JsonHelperT<bool>::jsonPathGet(object, "group_chat_created = false", this->groupChatCreated, false);
        JsonHelperT<bool>::jsonPathGet(object, "supergroup_chat_created = false", this->supergroupChatCreated, false);
        JsonHelperT<bool>::jsonPathGet(object, "channel_chat_created = false", this->channelChatCreated, false);
        JsonHelperT<qint32>::jsonPathGet(object, "migrate_to_chat_id", this->migrateToChatId, false);
        JsonHelperT<qint32>::jsonPathGet(object, "migrate_from_chat_id", this->migrateFromChatId, false);

        // own data
        JsonHelperT<TelegramBotMessageSingle>::jsonPathGet(object, "reply_to_message", this->replyToMessage, false);
        JsonHelperT<TelegramBotMessageSingle>::jsonPathGet(object, "pinned_message", this->pinnedMessage, false);
    }
};

// TelegramBotCallbackQuery - This object represents an incoming callback query from a callback button in an inline keyboard. If the button that originated the query was attached to a message sent by the bot, the field message will be present. If the button was attached to a message sent via the bot (in inline mode), the field inline_message_id will be present. Exactly one of the fields data or game_short_name will be present.
struct TelegramBotCallbackQuery : public TelegramBotObject {
    QString id; // Unique identifier for this query
    TelegramBotUser from; // Sender
    TelegramBotMessage message; // Optional. Message with the callback button that originated the query. Note that message content and message date will not be available if the message is too old
    QString inlineMessageId; // Optional. Identifier of the message sent via the bot in inline mode, that originated the query.
    QString chatInstance; // Global identifier, uniquely corresponding to the chat to which the message with the callback button was sent. Useful for high scores in games.
    QString data; // Optional. Data associated with the callback button. Be aware that a bad client can send arbitrary data in this field.
    QString gameShortName; // Optional. Short name of a Game to be returned, serves as the unique identifier for the game

    // parse logic
    virtual void fromJson(QJsonObject& object) {
        JsonHelperT<QString>::jsonPathGet(object, "id", this->id);
        JsonHelperT<TelegramBotUser>::jsonPathGet(object, "from", this->from);
        JsonHelperT<TelegramBotMessage>::jsonPathGet(object, "message", this->message, false);
        JsonHelperT<QString>::jsonPathGet(object, "inline_message_id", this->inlineMessageId, false);
        JsonHelperT<QString>::jsonPathGet(object, "chat_instance", this->chatInstance);
        JsonHelperT<QString>::jsonPathGet(object, "data", this->data, false);
        JsonHelperT<QString>::jsonPathGet(object, "game_short_name", this->gameShortName, false);
    }
};

#endif // TELEGRAMDATASTRUCTS_H
