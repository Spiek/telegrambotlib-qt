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

The Webhook system tells the Telegram Server to call a secure public Web Url to deliver the user message   
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

The Tegrambot library gives you also the possibility to communicate with chats

<details><summary>Send Message - sends a message to a chat</summary>

Represent [sendMessage](https://core.telegram.org/bots/api#sendmessage) function

```c++
#include <QCoreApplication>
#include "telegrambot.h"

int main(int argc, char** argv)
{
    QCoreApplication a(argc, argv);

    TelegramBot bot("APIKEY");
    bot.sendMessage("Chat_ID", "Please <b>choose</b>", TelegramBot::Html, 0, {
                            // row 1
                            {
                                // columns
                                { "Google", "https://www.google.com", "", "", "", false, false },
                                { "Reply with data", "", "MYDATA1", "", "", false, false },
                            },

                            // row 2
                            {
                                // columns
                                { "Reply with data", "", "MYDATA2", "", "", false, false },
                                { "Wikipedia", "https://en.wikipedia.org", "", "", "", false, false },
                            }
                        }
                    );
                    
   return a.exec();
}
```
</details>