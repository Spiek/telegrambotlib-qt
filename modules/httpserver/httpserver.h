#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QObject>
#include <QNetworkRequest>
#include <QSharedPointer>
#include <QMap>
#include <QString>
#include "sslserver.h"
#include "qdelegate.h"

struct HttpServerResponsePrivate
{
    QString version;
    const QMap<qint32, QString> StatusNames {
        {100, "Continue"},{101, "Switching Protocols"},{102, "Processing"},{200, "OK"},{201, "Created"},{202, "Accepted"},{203, "Non-Authoritative Information"},{204, "No Content"},{205, "Reset Content"},{206, "Partial Content"},{207, "Multi-Status"},{208, "Already Reported"},{226, "IM Used"},{300, "Multiple Choices"},{301, "Moved Permanently"},{302, "Found"},{303, "See Other"},{304, "Not Modified"},{305, "Use Proxy"},{306, "Switch Proxy"},{307, "Temporary Redirect"},{308, "Permanent Redirect"},{400, "Bad Request"},{401, "Unauthorized"},{402, "Payment Required"},{403, "Forbidden"},{404, "Not Found"},{405, "Method Not Allowed"},{406, "Not Acceptable"},{407, "Proxy Authentication Required"},{408, "Request Timeout"},{409, "Conflict"},{410, "Gone"},{411, "Length Required"},{412, "Precondition Failed"},{413, "Payload Too Large"},{414, "URI Too Long"},{415, "Unsupported Media Type"},{416, "Range Not Satisfiable"},{417, "Expectation Failed"},{418, "I'm a teapot"},{421, "Misdirected Request"},{422, "Unprocessable Entity"},{423, "Locked"},{424, "Failed Dependency"},{426, "Upgrade Required"},{428, "Precondition Required"},{429, "Too Many Requests"},{431, "Request Header Fields Too Large"},{451, "Unavailable For Legal Reasons"},{500, "Internal Server Error"},{501, "Not Implemented"},{502, "Bad Gateway"},{503, "Service Unavailable"},{504, "Gateway Timeout"},{505, "HTTP Version Not Supported"},{506, "Variant Also Negotiates"},{507, "Insufficient Storage"},{508, "Loop Detected"},{510, "Not Extended"},{511, "Network Authentication Required"},{103, "Checkpoint"},{103, "Early Hints"},{420, "Method Failure"},{420, "Enhance Your Calm"},{450, "Blocked by Windows Parental Controls"},{498, "Invalid Token"},{499, "Token Required"},{509, "Bandwidth Limit Exceeded"},{530, "Site is frozen"},{598, "Network read timeout error"},{599, "Network connect timeout error"},{440, "Login Time-out"},{449, "Retry With"},{451, "Redirect"},{444, "No Response"},{495, "SSL Certificate Error"},{496, "SSL Certificate Required"},{497, "HTTP Request Sent to HTTPS Port"},{499, "Client Closed Request"},{520, "Unknown Error"},{521, "Web Server Is Down"},{522, "Connection Timed Out"},{523, "Origin Is Unreachable"},{524, "A Timeout Occurred"},{525, "SSL Handshake Failed"},{526, "Invalid SSL Certificate"},{527, "Railgun Error"}
    };
    enum Status : qint32 {
        NoResponse = 0,

        /*
         * 1xx Informational responses
         * An informational response indicates that the request was received and understood. It is issued on a provisional basis while request processing continues. It alerts the client to wait for a final response. The message consists only of the status line and optional header fields, and is terminated by an empty line. As the HTTP/1.0 standard did not define any 1xx status codes, servers must not[note 1] send a 1xx response to an HTTP/1.0 compliant client except under experimental conditions.[4]
         */
        Continue = 100, // The server has received the request headers and the client should proceed to send the request body (in the case of a request for which a body needs to be sent; for example, a POST request). Sending a large request body to a server after a request has been rejected for inappropriate headers would be inefficient. To have a server check the request's headers, a client must send Expect: 100-continue as a header in its initial request and receive a 100 Continue status code in response before sending the body. The response 417 Expectation Failed indicates the request should not be continued.[2]
        Switching_Protocols = 101, // The requester has asked the server to switch protocols and the server has agreed to do so.[5]
        Processing = 102, // A WebDAV request may contain many sub-requests involving file operations, requiring a long time to complete the request. This code indicates that the server has received and is processing the request, but no response is available yet.[6] This prevents the client from timing out and assuming the request was lost.

        /*
         * 2xx Success
         * This class of status codes indicates the action requested by the client was received, understood, accepted, and processed successfully.[2]
         */
        OK = 200, // Standard response for successful HTTP requests. The actual response will depend on the request method used. In a GET request, the response will contain an entity corresponding to the requested resource. In a POST request, the response will contain an entity describing or containing the result of the action.[7]
        Created = 201, // The request has been fulfilled, resulting in the creation of a new resource.[8]
        Accepted = 202, // The request has been accepted for processing, but the processing has not been completed. The request might or might not be eventually acted upon, and may be disallowed when processing occurs.[9]
        NonAuthoritative_Information = 203, // The server is a transforming proxy (e.g. a Web accelerator) that received a 200 OK from its origin, but is returning a modified version of the origin's response.[10][11]
        No_Content = 204, // The server successfully processed the request and is not returning any content.[12]
        Reset_Content = 205, // The server successfully processed the request, but is not returning any content. Unlike a 204 response, this response requires that the requester reset the document view.[13]
        Partial_Content = 206, // The server is delivering only part of the resource (byte serving) due to a range header sent by the client. The range header is used by HTTP clients to enable resuming of interrupted downloads, or split a download into multiple simultaneous streams.[14]
        MultiStatus = 207, // The message body that follows is an XML message and can contain a number of separate response codes, depending on how many sub-requests were made.[15]
        Already_Reported = 208, // The members of a DAV binding have already been enumerated in a preceding part of the (multistatus) response, and are not being included again.
        IM_Used = 226, // The server has fulfilled a request for the resource, and the response is a representation of the result of one or more instance-manipulations applied to the current instance.[16]

        /*
         * 3xx Redirection
         * This class of status code indicates the client must take additional action to complete the request. Many of these status codes are used in URL redirection.[2]
         *
         * A user agent may carry out the additional action with no user interaction only if the method used in the second request is GET or HEAD. A user agent may automatically redirect a request. A user agent should detect and intervene to prevent cyclical redirects.[17]
         */
        Multiple_Choices = 300, // Indicates multiple options for the resource from which the client may choose (via agent-driven content negotiation). For example, this code could be used to present multiple video format options, to list files with different filename extensions, or to suggest word-sense disambiguation.[18]
        Moved_Permanently = 301, // This and all future requests should be directed to the given URI.[19]
        Found = 302, // This is an example of industry practice contradicting the standard. The HTTP/1.0 specification (RFC 1945) required the client to perform a temporary redirect (the original describing phrase was "Moved Temporarily"),[20] but popular browsers implemented 302 with the functionality of a 303 See Other. Therefore, HTTP/1.1 added status codes 303 and 307 to distinguish between the two behaviours.[21] However, some Web applications and frameworks use the 302 status code as if it were the 303.[22]
        See_Other = 303, // The response to the request can be found under another URI using a GET method. When received in response to a POST (or PUT/DELETE), the client should presume that the server has received the data and should issue a redirect with a separate GET message.[23]
        Not_Modified = 304, // Indicates that the resource has not been modified since the version specified by the request headers If-Modified-Since or If-None-Match. In such case, there is no need to retransmit the resource since the client still has a previously-downloaded copy.[24]
        Use_Proxy = 305, // The requested resource is available only through a proxy, the address for which is provided in the response. Many HTTP clients (such as Mozilla[25] and Internet Explorer) do not correctly handle responses with this status code, primarily for security reasons.[26]
        Switch_Proxy = 306, // No longer used. Originally meant "Subsequent requests should use the specified proxy."[27]
        Temporary_Redirect = 307, // In this case, the request should be repeated with another URI; however, future requests should still use the original URI. In contrast to how 302 was historically implemented, the request method is not allowed to be changed when reissuing the original request. For example, a POST request should be repeated using another POST request.[28]
        Permanent_Redirect = 308, // The request and all future requests should be repeated using another URI. 307 and 308 parallel the behaviors of 302 and 301, but do not allow the HTTP method to change. So, for example, submitting a form to a permanently redirected resource may continue smoothly.[29]

        /*
         * 4xx Client errors
         * The 4xx class of status codes is intended for situations in which the client seems to have erred. Except when responding to a HEAD request, the server should include an entity containing an explanation of the error situation, and whether it is a temporary or permanent condition. These status codes are applicable to any request method. User agents should display any included entity to the user.[30]
         */
        Bad_Request = 400, // The server cannot or will not process the request due to an apparent client error (e.g., malformed request syntax, too large size, invalid request message framing, or deceptive request routing).[31]
        Unauthorized = 401, // Similar to 403 Forbidden, but specifically for use when authentication is required and has failed or has not yet been provided. The response must include a WWW-Authenticate header field containing a challenge applicable to the requested resource. See Basic access authentication and Digest access authentication.[32] 401 semantically means "unauthenticated",[33] i.e. the user does not have the necessary credentials.
        Payment_Required = 402, // Reserved for future use. The original intention was that this code might be used as part of some form of digital cash or micropayment scheme, but that has not happened, and this code is not usually used. Google Developers API uses this status if a particular developer has exceeded the daily limit on requests.[34]
        Forbidden = 403, // The request was valid, but the server is refusing action. The user might not have the necessary permissions for a resource.
        Not_Found = 404, // The requested resource could not be found but may be available in the future. Subsequent requests by the client are permissible.[35]
        Method_Not_Allowed = 405, // A request method is not supported for the requested resource; for example, a GET request on a form that requires data to be presented via POST, or a PUT request on a read-only resource.
        Not_Acceptable = 406, // The requested resource is capable of generating only content not acceptable according to the Accept headers sent in the request.[36] See Content negotiation.
        Proxy_Authentication_Required = 407, // The client must first authenticate itself with the proxy.[37]
        Request_Timeout = 408, // The server timed out waiting for the request. According to HTTP specifications: "The client did not produce a request within the time that the server was prepared to wait. The client MAY repeat the request without modifications at any later time."[38]
        Conflict = 409, // Indicates that the request could not be processed because of conflict in the request, such as an edit conflict between multiple simultaneous updates.
        Gone = 410, // Indicates that the resource requested is no longer available and will not be available again. This should be used when a resource has been intentionally removed and the resource should be purged. Upon receiving a 410 status code, the client should not request the resource in the future. Clients such as search engines should remove the resource from their indices.[39] Most use cases do not require clients and search engines to purge the resource, and a "404 Not Found" may be used instead.
        Length_Required = 411, // The request did not specify the length of its content, which is required by the requested resource.[40]
        Precondition_Failed = 412, // The server does not meet one of the preconditions that the requester put on the request.[41]
        Payload_Too_Large = 413, // The request is larger than the server is willing or able to process. Previously called "Request Entity Too Large".[42]
        URI_Too_Long = 414, // The URI provided was too long for the server to process. Often the result of too much data being encoded as a query-string of a GET request, in which case it should be converted to a POST request.[43] Called "Request-URI Too Long" previously.[44]
        Unsupported_Media_Type = 415, // The request entity has a media type which the server or resource does not support. For example, the client uploads an image as image/svg+xml, but the server requires that images use a different format.
        Range_Not_Satisfiable = 416, // The client has asked for a portion of the file (byte serving), but the server cannot supply that portion. For example, if the client asked for a part of the file that lies beyond the end of the file.[45] Called "Requested Range Not Satisfiable" previously.[46]
        Expectation_Failed = 417, // The server cannot meet the requirements of the Expect request-header field.[47]
        Im_a_teapot = 418, // This code was defined in 1998 as one of the traditional IETF April Fools' jokes, in RFC 2324, Hyper Text Coffee Pot Control Protocol, and is not expected to be implemented by actual HTTP servers. The RFC specifies this code should be returned by teapots requested to brew coffee.[48] This HTTP status is used as an Easter egg in some websites, including Google.com.[49]
        Misdirected_Request = 421, // The request was directed at a server that is not able to produce a response (for example because a connection reuse).[50]
        Unprocessable_Entity = 422, // The request was well-formed but was unable to be followed due to semantic errors.[15]
        Locked = 423, // The resource that is being accessed is locked.[15]
        Failed_Dependency = 424, // The request failed due to failure of a previous request (e.g., a PROPPATCH).[15]
        Upgrade_Required = 426, // The client should switch to a different protocol such as TLS/1.0, given in the Upgrade header field.[51]
        Precondition_Required = 428, // The origin server requires the request to be conditional. Intended to prevent "the 'lost update' problem, where a client GETs a resource's state, modifies it, and PUTs it back to the server, when meanwhile a third party has modified the state on the server, leading to a conflict."[52]
        Too_Many_Requests = 429, // The user has sent too many requests in a given amount of time. Intended for use with rate-limiting schemes.[52]
        Request_Header_Fields_Too_Large = 431, // The server is unwilling to process the request because either an individual header field, or all the header fields collectively, are too large.[52]
        Unavailable_For_Legal_Reasons = 451, // A server operator has received a legal demand to deny access to a resource or to a set of resources that includes the requested resource.[53] The code 451 was chosen as a reference to the novel Fahrenheit 451.

        /*
         * 5xx Server error
         * The server failed to fulfil an apparently valid request.[54]
         * Response status codes beginning with the digit "5" indicate cases in which the server is aware that it has encountered an error or is otherwise incapable of performing the request. Except when responding to a HEAD request, the server should include an entity containing an explanation of the error situation, and indicate whether it is a temporary or permanent condition. Likewise, user agents should display any included entity to the user. These response codes are applicable to any request method.[55]
         */
        Internal_Server_Error = 500, // A generic error message, given when an unexpected condition was encountered and no more specific message is suitable.[56]
        Not_Implemented = 501, // The server either does not recognize the request method, or it lacks the ability to fulfil the request. Usually this implies future availability (e.g., a new feature of a web-service API).[57]
        Bad_Gateway = 502, // The server was acting as a gateway or proxy and received an invalid response from the upstream server.[58]
        Service_Unavailable = 503, // The server is currently unavailable (because it is overloaded or down for maintenance). Generally, this is a temporary state.[59]
        Gateway_Timeout = 504, // The server was acting as a gateway or proxy and did not receive a timely response from the upstream server.[60]
        HTTP_Version_Not_Supported = 505, // The server does not support the HTTP protocol version used in the request.[61]
        Variant_Also_Negotiates = 506, // Transparent content negotiation for the request results in a circular reference.[62]
        Insufficient_Storage = 507, // The server is unable to store the representation needed to complete the request.[15]
        Loop_Detected = 508, // The server detected an infinite loop while processing the request (sent in lieu of 208 Already Reported).
        Not_Extended = 510, // Further extensions to the request are required for the server to fulfil it.[63]
        Network_Authentication_Required = 511, // The client needs to authenticate to gain network access. Intended for use by intercepting proxies used to control access to the network (e.g., "captive portals" used to require agreement to Terms of Service before granting full Internet access via a Wi-Fi hotspot).[52]

        /*
         * Unofficial codes
         * The following codes are not specified by any standard.
         */
        Checkpoint = 103, // Used in the resumable requests proposal to resume aborted PUT or POST requests.[64]
        Early_Hints = 103, // Used to return some response headers before entire HTTP response.[65][66]
        Method_Failure = 420, // A deprecated response used by the Spring Framework when a method has failed.[67]
        Enhance_Your_Calm = 420, // Returned by version 1 of the Twitter Search and Trends API when the client is being rate limited; versions 1.1 and later use the 429 Too Many Requests response code instead.[68]
        Blocked_by_Windows_Parental_Controls = 450, // The Microsoft extension code indicated when Windows Parental Controls are turned on and are blocking access to the given webpage.[69]
        Invalid_Token = 498, // Returned by ArcGIS for Server. Code 498 indicates an expired or otherwise invalid token.[70]
        Token_Required = 499, // Returned by ArcGIS for Server. Code 499 indicates that a token is required but was not submitted.[70]
        Bandwidth_Limit_Exceeded = 509, // The server has exceeded the bandwidth specified by the server administrator; this is often used by shared hosting providers to limit the bandwidth of customers.[71]
        Site_is_frozen = 530, // Used by the Pantheon web platform to indicate a site that has been frozen due to inactivity.[72]
        Network_read_timeout_error = 598, // Used by some HTTP proxies to signal a network read timeout behind the proxy to a client in front of the proxy.[73][74]
        Network_connect_timeout_error = 599, // Used to indicate when the connection to the network times out.[75][citation needed]

        /*
         * Internet Information Services
         * The Internet Information Services expands the 4xx error space to signal errors with the client's request.
         */
        Login_Timeout = 440, // The client's session has expired and must log in again.[76]
        Retry_With = 449, // The server cannot honour the request because the user has not provided the required information.[77]
        Redirect = 451, // Used in Exchange ActiveSync when either a more efficient server is available or the server cannot access the users' mailbox.[78] The client is expected to re-run the HTTP AutoDiscover operation to find a more appropriate server.[79]

        /*
         * nginx
         * The nginx web server software expands the 4xx error space to signal issues with the client's request.[80][81] These are only used for logging purposes, no actual response is sent with these codes.
         */
        No_Response = 444, // Used to indicate that the server has returned no information to the client and closed the connection.
        SSL_Certificate_Error = 495, // An expansion of the 400 Bad Request response code, used when the client has provided an invalid client certificate.
        SSL_Certificate_Required = 496, // An expansion of the 400 Bad Request response code, used when a client certificate is required but not provided.
        HTTP_Request_Sent_to_HTTPS_Port = 497, // An expansion of the 400 Bad Request response code, used when the client has made a HTTP request to a port listening for HTTPS requests.
        Client_Closed_Request = 499, // Used when the client has closed the request before the server could send a response.

        /*
         * Cloudflare
         * Cloudflare's reverse proxy service expands the 5xx series of errors space to signal issues with the origin server.[82]
         */
        Unknown_Error = 520, // The 520 error is used as a "catch-all response for when the origin server returns something unexpected", listing connection resets, large headers, and empty or invalid responses as common triggers.
        Web_Server_Is_Down = 521, // The origin server has refused the connection from Cloudflare.
        Connection_Timed_Out = 522, // Cloudflare could not negotiate a TCP handshake with the origin server.
        Origin_Is_Unreachable = 523, // Cloudflare could not reach the origin server; for example, if the DNS records for the origin server are incorrect.
        A_Timeout_Occurred = 524, // Cloudflare was able to complete a TCP connection to the origin server, but did not receive a timely HTTP response.
        SSL_Handshake_Failed = 525, // Cloudflare could not negotiate a SSL/TLS handshake with the origin server.
        Invalid_SSL_Certificate = 526, // Cloudflare could not validate the SSL/TLS certificate that the origin server presented.
        Railgun_Error = 527, // Error 527 indicates that the requests timeout or failed after the WAN connection has been established.[83]
    } status = NoResponse;
    QMap<QString, QString> headers;
    QByteArray content;
};
typedef QSharedPointer<HttpServerResponsePrivate> HttpServerResponse;

struct HttpServerRequestPrivate
{
    QString method;
    QString host;
    QString url;
    QString version;
    QMap<QString, QString> headers;
    QByteArray content;

    private:
        enum ParseState {
            Method,
            Url,
            Version,
            Headers,
            Content,
            Done
        } parseState = ParseState::Method;
        QString currentHeaderKey;
        QString* currentHeaderValue = 0;
        int contentLength = -1;

   friend class HttpServer;
};
typedef QSharedPointer<HttpServerRequestPrivate> HttpServerRequest;

class HttpServer : public SSLServer
{
    Q_OBJECT
    public:
        HttpServer(QObject *parent = 0);
        void addRewriteRule(QString host, QString path, QDelegate<void(HttpServerRequest,HttpServerResponse)> delegate);

    private:
        void handleNewConnection();
        void handleNewData();

        // Helper
        bool parseRequest(QTcpSocket& device, HttpServerRequest& request);

        // routing
        QMap<QString, QMultiMap<QString, QDelegate<void(HttpServerRequest,HttpServerResponse)>>> rewriteRules;
        QMap<QTcpSocket*, HttpServerRequest> pendingRequests;
};

#endif // HTTPSERVER_H
