#include "httpserver.h"

HttpServer::HttpServer(QObject *parent) : SSLServer(parent)
{
    QObject::connect(this, &HttpServer::connectionReady, this, &HttpServer::handleNewConnection);
}

void HttpServer::addRewriteRule(QString host, QString path, QDelegate<void(HttpServerRequest, HttpServerResponse)> delegate)
{
    if(!this->rewriteRules.contains(host)) this->rewriteRules.insert(host, QMultiMap<QString, QDelegate<void(HttpServerRequest,HttpServerResponse)>>());
    this->rewriteRules.find(host).value().insert(path, delegate);
}

void HttpServer::handleNewConnection()
{
    QObject::connect(this->nextPendingConnection(), &QTcpSocket::readyRead, this, &HttpServer::handleNewData);
}

void HttpServer::handleNewData()
{
    // get socket where data are available
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(this->sender());
    if(!socket) return;

    // parse result
    HttpServerRequest request = this->pendingRequests.contains(socket) ?
                                    this->pendingRequests.find(socket).value() :
                                    this->pendingRequests.insert(socket, HttpServerRequest(new HttpServerRequestPrivate())).value();
    if(!this->parseRequest(*socket, request)) return;

    // remove pending request
    this->pendingRequests.remove(socket);

    // exit if we have no route for host
    if(!this->rewriteRules.contains(request->host)) return;

    // invoke routes
    HttpServerResponse response(new HttpServerResponsePrivate);
    auto& hostRoutes = this->rewriteRules.find(request->host).value();
    for(auto itr = hostRoutes.begin(); itr != hostRoutes.end(); itr++) {
        if(itr.key().startsWith(request->url)) itr.value().invoke(request, response);
    }

    // exit if user don't want to send a response back
    if(!response->status) return;

    // build response
    QByteArray responseContent;
    responseContent += (response->version.isEmpty() ? "HTTP/1.1" : response->version) + " "; // Version
    responseContent += QByteArray::number((qint32)response->status) + " " + response->StatusNames.value((qint32)response->status, "") + "\r\n"; // Status

    // add headers
    qint64 contentLength = 0;
    for(auto itr = response->headers.begin(); itr != response->headers.end(); itr++) {
        if(itr.key().toLower() == "content-length") contentLength = itr.value().toLongLong();
        responseContent += itr.key() + ": " + itr.value() + "\r\n";
    }

    // add content length if not available
    if(contentLength && !response->content.isEmpty()) responseContent += "Content-Length: " + QByteArray::number(contentLength) + "\r\n";
    responseContent += "\r\n";

    // add content
    if(!response->content.isEmpty()) responseContent += response->content;

    // send response
    socket->write(responseContent);
}

bool HttpServer::parseRequest(QTcpSocket &device, HttpServerRequest& request)
{
    // parse content
    QByteArray content = device.readAll();
    for(auto itr = content.begin(); itr != content.end(); itr++) {
        // parse method
        if(request->parseState == HttpServerRequestPrivate::Method) {
            if(*itr == ' ') request->parseState = HttpServerRequestPrivate::Url;
            else request->method += *itr;
        }

        // parse url
        else if(request->parseState == HttpServerRequestPrivate::Url) {
            if(*itr == ' ') request->parseState = HttpServerRequestPrivate::Version;
            else request->url += *itr;
        }

        // parse version
        else if(request->parseState == HttpServerRequestPrivate::Version) {
            if(*itr == '\r') {
                itr++;
                request->parseState = HttpServerRequestPrivate::Headers;
            }
            else request->version += *itr;
        }

        // parse headers
        else if(request->parseState == HttpServerRequestPrivate::Headers) {
            // switch from key to value
            if(*itr == ':') {
                request->currentHeaderValue = &request->headers.insert(request->currentHeaderKey, QString("")).value();
                itr++;
            } else {
                // end header
                if(*itr == '\r') {
                    itr++;
                    if(*(itr + 1) == '\r') {
                        itr++;
                        itr++;
                        request->parseState = HttpServerRequestPrivate::Content;
                    }
                    request->currentHeaderKey = QString();
                    request->currentHeaderValue = 0;
                }

                // append to value
                else if(request->currentHeaderValue) *request->currentHeaderValue += *itr;

                // append to key
                else request->currentHeaderKey += *itr;
            }
        }

        // parse content
        else if(request->parseState == HttpServerRequestPrivate::Content) {
            if(request->contentLength == -1) request->contentLength = request->headers.value("Content-Length").toInt();
            if(request->contentLength <= 1) request->parseState = HttpServerRequestPrivate::Done;

            // append to content
            if(request->contentLength) {
                request->content += *itr;
                request->contentLength--;
            }
        }
    }

    // demoralize data
    request->host = request->headers.value("Host");

    // if we are done return true, otherwise false
    return request->parseState == HttpServerRequestPrivate::Done;
}
