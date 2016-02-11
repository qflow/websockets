#ifndef WEBSOCKETSERVER_H
#define WEBSOCKETSERVER_H


#include "websockets_global.h"
#include "initiable.h"
#include "websocketconnection.h"
#include <QObject>

class QTcpSocket;
namespace QFlow{
class WebSocketServerPrivate;
class WEBSOCKETS_EXPORT WebSocketServer : public QObject, public QmlInitiable
{
    Q_OBJECT
    Q_PROPERTY(int port READ port WRITE setPort)
    Q_PROPERTY(QString host READ host WRITE setHost)
public:
    explicit WebSocketServer(QObject *parent = 0);
    ~WebSocketServer();
    int port() const;
    void setPort(int value);
    ErrorInfo init();
    QString host() const;
    void setHost(QString value);
Q_SIGNALS:
    void newConnection(WebSocketConnection* con);
public Q_SLOTS:
private:
    const QScopedPointer<WebSocketServerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(WebSocketServer)
};
}
#endif // WEBSOCKETSERVER_H
