#include "sslserver.h"

SSLServer::SSLServer(QObject *parent) : QTcpServer(parent)
{
    QObject::connect(this, &SSLServer::newConnection, this, &SSLServer::handleNewConnection);
}

bool SSLServer::setPrivateKey(QString keyFile, QSsl::KeyAlgorithm algorithmn)
{
    QByteArray content = SSLServer::getFileContent(keyFile);
    return this->setPrivateKeyRaw(content, algorithmn);
}

bool SSLServer::setPrivateKeyRaw(QByteArray &keyFileContent, QSsl::KeyAlgorithm algorithmn)
{
    if(keyFileContent.isEmpty()) {
        this->sslKey = QSslKey();
        return true;
    } else {
        this->sslKey = QSslKey(keyFileContent, algorithmn);
    }
    return !this->sslKey.isNull();
}

bool SSLServer::isSamePrivateKey(QString keyFile, QSsl::KeyAlgorithm algorithmn)
{
    auto data = this->getFileContent(keyFile);
    return this->isSamePrivateKey(data, algorithmn);
}

bool SSLServer::isSamePrivateKey(QByteArray &keyFileContent, QSsl::KeyAlgorithm algorithmn)
{
    return QSslKey(keyFileContent, algorithmn) == this->sslKey;
}

QSslCertificate SSLServer::addCert(QString certFile, QSsl::EncodingFormat format)
{
    QByteArray content = SSLServer::getFileContent(certFile);
    return this->addCertRaw(content, format);
}

QSslCertificate SSLServer::addCertRaw(QByteArray &certData, QSsl::EncodingFormat format)
{
    // param check
    if(certData.isEmpty()) return QSslCertificate();
    QSslCertificate cert(certData, format);
    if(cert.isNull() || cert.subjectInfo(QSslCertificate::CommonName).isEmpty()) return cert;
    this->sslCerts.append(cert);
    return cert;
}

void SSLServer::addWhiteListHost(QString address)
{
    this->whiteListHosts.append(qMakePair(QHostAddress(address), -1));
}

void SSLServer::addWhiteListHostSubnet(QString subnet)
{
    this->whiteListHosts.append(QHostAddress::parseSubnet(subnet));
}

void SSLServer::handleNewConnection()
{
    if(this->sslKey.isNull()) emit this->connectionReady();
}

QByteArray SSLServer::getFileContent(QString file)
{
    // get key content
    QFile ffile(file);
    if(!ffile.open(QFile::ReadOnly)) return QByteArray();
    return ffile.readAll();
}

void SSLServer::incomingConnection(qintptr socketDescriptor)
{
    // use default QTcpServer implementation if we have no present private key
    if(this->sslKey.isNull()) {
        QTcpSocket *socket = new QTcpSocket(this);
        if(!socket->setSocketDescriptor(socketDescriptor) || !this->isConnectionAllowed(socket)) delete socket;
        else this->addPendingConnection(socket);
        return;
    }

    // create ssl connection
    QSslSocket* sslSocket = new QSslSocket(this);
    QObject::connect(sslSocket, &QSslSocket::disconnected, sslSocket, &QSslSocket::deleteLater);
    QObject::connect(sslSocket, SIGNAL(error(QAbstractSocket::SocketError)), sslSocket, SLOT(deleteLater()));
    if(sslSocket->setSocketDescriptor(socketDescriptor) && this->isConnectionAllowed(sslSocket)) {
        QObject::connect(sslSocket, &QSslSocket::encrypted, this, &SSLServer::connectionReady);
        sslSocket->setPeerVerifyMode(QSslSocket::VerifyNone);
        sslSocket->setLocalCertificateChain(this->sslCerts);
        sslSocket->setPrivateKey(this->sslKey);
        sslSocket->setProtocol(QSsl::TlsV1_2OrLater);
        sslSocket->startServerEncryption();
        this->addPendingConnection(sslSocket);
    } else {
        delete sslSocket;
    }
}

bool SSLServer::isConnectionAllowed(QTcpSocket *socket)
{
    // param check
    if(!socket) return false;
    if(this->whiteListHosts.isEmpty()) return true;

    // check if ip is whitelisted, if not return false
    QHostAddress addressIpv4(socket->peerAddress().toIPv4Address());
    for(QPair<QHostAddress, int>& host : this->whiteListHosts) {
        if(host.second == -1 && addressIpv4.isEqual(host.first)) return true;
        else if(addressIpv4.isInSubnet(host)) return true;
    }

    qDebug("SSLServer::isConnectionAllowed: Host %s Rejected", qPrintable(addressIpv4.toString()));
    return false;
}
