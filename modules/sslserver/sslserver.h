#ifndef SSLSERVER_H
#define SSLSERVER_H

#include <QTcpServer>
#include <QSslSocket>
#include <QFile>
#include <QSslCertificate>
#include <QSslKey>
#include <QSslConfiguration>

class SSLServer : public QTcpServer
{
    Q_OBJECT
    signals:
        void connectionReady();

    public:
        SSLServer(QObject* parent = 0);
        bool setPrivateKey(QString keyFile, QSsl::KeyAlgorithm algorithmn = QSsl::KeyAlgorithm::Rsa);
        bool setPrivateKeyRaw(QByteArray& keyFileContent, QSsl::KeyAlgorithm algorithmn = QSsl::KeyAlgorithm::Rsa);
        bool isSamePrivateKey(QString keyFile, QSsl::KeyAlgorithm algorithmn = QSsl::KeyAlgorithm::Rsa);
        bool isSamePrivateKey(QByteArray& keyFileContent, QSsl::KeyAlgorithm algorithmn = QSsl::KeyAlgorithm::Rsa);
        QSslCertificate addCert(QString certFile, QSsl::EncodingFormat format = QSsl::Pem);
        QSslCertificate addCertRaw(QByteArray& certData, QSsl::EncodingFormat format = QSsl::Pem);
        inline QSslCertificate getCert(int index = 0) { return this->sslCerts.value(index); }

        // host whitelist
        void addWhiteListHost(QString address);
        void addWhiteListHostSubnet(QString subnet);

    protected:
        virtual void incomingConnection(qintptr socketDescriptor);

    private:
        void handleNewConnection();
        bool isConnectionAllowed(QTcpSocket* socket);

    private:
        // private helpers
        static QByteArray getFileContent(QString file);

        // intern data
        QList<QSslCertificate> sslCerts;
        QSslKey sslKey;

        // white list
        QList<QPair<QHostAddress, int>> whiteListHosts;
};

#endif // SSLSERVER_H
