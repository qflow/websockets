#ifndef WEBSOCKETCONNECTION_H
#define WEBSOCKETCONNECTION_H

#include "websockets_global.h"
#include <QThread>
#include <QPointer>
#include <QAbstractSocket>

class QHostAddress;

namespace QFlow{

class WebSocketWorker;
class WEBSOCKETS_EXPORT WebSocketConnection : public QThread
{
    Q_OBJECT
    Q_PROPERTY(QStringList requestedSubprotocols READ requestedSubprotocols WRITE setRequestedSubprotocols)
    Q_PROPERTY(QString uri READ uri WRITE setUri)
    Q_PROPERTY(QString subprotocol READ subprotocol NOTIFY subprotocolChanged)
public:
    explicit WebSocketConnection(QObject* parent = NULL);
    ~WebSocketConnection();
    void handleConnection(qintptr socketDescriptor);
    QStringList requestedSubprotocols() const;
    void setRequestedSubprotocols(const QStringList& list);
    QString uri() const;
    void setUri(const QString& value);
    QString subprotocol() const;
    QHostAddress peerAddress() const;
    QAbstractSocket::SocketState state() const;
public Q_SLOTS:
    void connect();
    void sendBinary(const QByteArray& data);
    void sendText(const QString& msg);
    void accept(bool accepted = true);
    void selectSubprotocol(const QString& value);
    QString responseHeader(const QString& key) const;
    bool isServer() const;
    void close();
Q_SIGNALS:
    void closed();
    void opened();
    void messageReceived(const QByteArray& message);
    void subprotocolChanged();
private:
    void run();
    WebSocketWorker* _worker;
};
}
#endif // WEBSOCKETCONNECTION_H
