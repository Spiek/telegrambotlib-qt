#include <QCoreApplication>
#include "telegrambot.h"

int main(int argc, char *argv[])
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
