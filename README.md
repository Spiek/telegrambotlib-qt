# Qt Telegram Bot Library

A very easy to use Qt library for Accessing the Telegram Bot Api.

----------

### Receiving Updates
The Telegrambot library gives you multiple possibilities to receive messages which was sent to your bot 

<details><summary>Message Pulling</summary>

The Message pull system connects to Telegram Server and pull for new messages:  
* If a new Message is available, the Telegram Server push a new message directly over this connection to us,  
  then the Telegrambot processes this message(s) and send for every message a SIGNAL to the outside world.
* If the Telegrambot don't receive a message in a given time, the pull system disconnects from the telegram server.
* This Process repeats endless until the user call stopMessagePulling().

Advantages over Webhook:
* No administrative Task needed to make it work
* Client only need the possibility to connect to a remote server
* No Certificate needed
* No open Port needed 
* No NAT port forwarding needed (if client is in NAT-Network)

Disadvantages over Webhook:
* Only one simultaneously connection to telegram to handle messages

Here an example 
```c++
#include <QCoreApplication>
#include "telegrambot.h"

int main(int argc, char** argv)
{
    QCoreApplication a(argc, argv);
    
    TelegramBot bot("APIKEY");
    QObject::connect(&bot, &TelegramBot::newMessage, [](TelegramBotMessage message) {
        qDebug("New Message from: %s %s (%s) -> %s", 
               qPrintable(message.from.firstName), 
               qPrintable(message.from.lastName), 
               qPrintable(message.from.username), 
               qPrintable(message.text));
    });
    bot.startMessagePulling();
    
    return a.exec();
}
```
</details>

<details><summary>Message Polling (Webhook)</summary>

The Webhook system tells the Telegram Server to call a secure public Web Url to deliver the user message.   
To make this possible the public url needs a valid certificate, either a valid public accpeted one or a self signed one.

Here an easy example installation instruction which should work nearly for all kinds of setups (including NAT):
* First we need a public domain, so let us make one using this awesome nip.io service: telegram.[our ip address](https://api.ipify.org/).nip.io
* Now we generate an ssl cert for this domain using the following command line (we need the openssl tool for it):   
  ```openssl req -newkey rsa:2048 -sha256 -nodes -keyout telegram.key -x509 -days 365 -out telegram.crt -subj "//C=DE\CN=telegram.[our ip address].nip.io"```
* Put the generated files (telegram.key and telegram.crt) into the working directory of your application.
* If in Nat, forward external port 8443 to port 8443 on local PC.

Then use the following Qt Code:
```c++
#include <QCoreApplication>
#include "telegrambot.h"

int main(int argc, char** argv)
{
    QCoreApplication a(argc, argv);
    
    TelegramBot bot("APIKEY");
    QObject::connect(&bot, &TelegramBot::newMessage, [](TelegramBotMessage message) {
        qDebug("New Message from: %s %s (%s) -> %s", 
               qPrintable(message.from.firstName), 
               qPrintable(message.from.lastName), 
               qPrintable(message.from.username), 
               qPrintable(message.text));
    });
    bot.setHttpServerWebhook(8443, "telegram.crt", "telegram.key");
    
    return a.exec();
}
```
</details>


----------


### Communication

The Tegrambot library gives you also the possibility to communicate with chats.  
Here is an example which should explain the basic use cases:

```c++
#include <QCoreApplication>
#include "telegrambot.h"

int main(int argc, char** argv)
{
    QCoreApplication a(argc, argv);

    TelegramBot bot("APIKEY");
    QObject::connect(&bot, &TelegramBot::newMessage, [&bot](TelegramBotUpdate update) {
        // only handle Messages
        if(update->type != TelegramBotMessageType::Message) return;
        
        // simplify message access
        TelegramBotMessage& message = *update->message;		
        
        // send message (Format: Normal)
        TelegramBotMessage msgSent;
        bot.sendMessage(message.chat.id,
                        "This is a Testmessage",
                        0,
                        TelegramBot::NoFlag,
                        TelegramKeyboardRequest(),
                        &msgSent);

        // edit text of sent message (Format: Markdown)
        bot.editMessageText(message.chat.id,
                            msgSent.messageId,
                            "This is an edited *Testmessage*",
                            TelegramBot::Markdown);

        // send message (Format: HTML, Keyboard: Inline (2 Rows, 1 Column), Reply to Message: Yes)
        bot.sendMessage(message.chat.id,
                        "Please <b>choose</b>",
                        0,
                        TelegramBot::Html,
                        {
                            // Keyboard
                            {
                                TelegramBot::constructInlineButton("Google", "", "https://www.google.com"),
                            }, {
                                TelegramBot::constructInlineButton("Reply with data", "MYDATA1"),
                            }
                        });

        // send photo (file location: web)
        bot.sendPhoto(message.chat.id,
                      "https://www.kernel.org/theme/images/logos/tux.png",
                      "This is the Linux Tux");

        // send audio (file location: local)
        bot.sendAudio(message.chat.id,
                      "Maktone - Fluke01.mp3",
                      "Listen to this great art :-)",
                      "Maktone",
                      "Fluke01");

        // send video chat note (file location: QByteArray)
        QFile file("testvideo.mp4");
        file.open(QFile::ReadOnly);
        QByteArray content = file.readAll();
        bot.sendVideoNote(msgSent.chat.id, content);

        // send sticker (file location: telegram server)
        bot.sendSticker(message.chat.id, "CXXXXXXXXXXXXXXXXXXXXXXX");
    });
    bot.startMessagePulling();
                    
   return a.exec();
}
```

This example produces the following Telgram messages:  
![result](https://raw.githubusercontent.com/Spiek/telegrambotlib-qt/master/doc/readme-example-result.png)

----------

### File Handing
The library provides four different opportunities for sending a file to telegram:
*  Web: just provide an web link as string
*  Local file: just provide a local file path as string
*  QBytearray: just call with a QBytearray which contains the data (The library also try to detect its content type!)
*  Telegram Server: just call with an telegram file id as string   

Note: file uploades are handled asynyron!   
A possible use case for all types are also available in the example above.

----------

### Message Routing
In Addition the Library contains a message routing system.   
This system allows you to route any kind of message to your own functions (using a [QDelegate](https://github.com/Spiek/QDelegate)) 

The following Code Example demonstrate this message routing, by handing the first /start-message a user send to a bot:
```c++
#include <QCoreApplication>
#include "telegrambot.h"

int main(int argc, char** argv)
{
    QCoreApplication a(argc, argv);

    TelegramBot bot("APIKEY");
    bot.messageRouterRegister("/start", {[&bot](TelegramBotUpdate update) {
        bot.sendMessage(update->message->chat.id, "Hi, i'am a Test bot");
		return true;
    }}, TelegramBotMessageType::Message);
    
    bot.startMessagePulling();
}
```
Additional Notes:
* The example above only handles the /start message all other messages send to the bot are silently ignored.
* The message Router match field depends on the received message type:

Message Type  | Fieldname | 
:-------- | :------- 
TelegramBotMessageType::Message | [Message](https://core.telegram.org/bots/api#message).text | 
TelegramBotMessageType::EditedMessage | [Message](https://core.telegram.org/bots/api#message).text | 
TelegramBotMessageType::ChannelPost | [Message](https://core.telegram.org/bots/api#message).text | 
TelegramBotMessageType::EditedChannelPost | [Message](https://core.telegram.org/bots/api#message).text | 
TelegramBotMessageType::InlineQuery | [InlineQuery](https://core.telegram.org/bots/api#inlinequery).query | 
TelegramBotMessageType::ChosenInlineResult | [ChosenInlineResult](https://core.telegram.org/bots/api#choseninlineresult).query | 
TelegramBotMessageType::CallbackQuery | [CallbackQuery](https://core.telegram.org/bots/api#callbackquery).data | 

----------

### Library Dependings
The telegrambotlib-qt depends on [QDelegate](https://github.com/Spiek/QDelegate) which are allready included as submodule,  
so we have to make sure that the submodules are checked out, too:
```bash
git clone --recursive <path>
```
Or if repo allready exists
```bash
git submodule update --init --recursive
```

----------

### Compile Staticly:  
Just add the following to your Qt-Project file:
```qmake
include(telegrambotlib-qt.pri)
```
**Include project syntax:**  
#include "telegrambot.h"

----------

### Compile Dynamicly:   
Note: The **make install** installation pathes, are printed to you during qmake!
```
qmake telegrambotlib-qt.pro
make
make install
```
add the following to your pro file:
```qmake
LIBS += -ltelegrambotlib-qt
```
**Include project syntax:**   
#include <telegrambotlib-qt/telegrambot.h>

----------

### Licence
The [telegrambotlib-qt licence](https://github.com/Spiek/telegrambotlib-qt/blob/master/LICENCE) is a modified version of the [LGPL](http://www.gnu.org/licenses/lgpl.html) licence, with a static linking exception.
