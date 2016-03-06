#ifndef WEBSOCKETWORKER_H
#define WEBSOCKETWORKER_H

#include <QObject>
#include <websocketpp/config/core.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/client.hpp>
#include <QWaitCondition>
#include <QMutex>
#include <atomic>
#include <QTcpSocket>
#include <QPointer>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::placeholders::_3;
using websocketpp::lib::bind;
typedef websocketpp::server<websocketpp::config::core> WebsocketppServer;
typedef websocketpp::connection<websocketpp::config::core> connection_type;
typedef connection_type::message_ptr message_ptr;
typedef websocketpp::client<websocketpp::config::core> WebsocketppClient;
typedef websocketpp::endpoint<connection_type, websocketpp::config::core> WebsocketppEndpoint;

class QTcpSocket;

namespace QFlow{

enum ConnectionState {CLOSED = 0, CONNECTING = 1, VALIDATING = 2, OPENED = 3};

class WebSocketConnection;
class WebSocketWorker : public QObject
{
    Q_OBJECT
public:
    QPointer<QTcpSocket> _socket;
    WebSocketConnection* _connection;
    connection_type::ptr _con;
    std::shared_ptr<WebsocketppEndpoint> _endpoint;
    QStringList _requestedSubprotocols;
    QMutex _mutex;
    QWaitCondition _pendingValidation;
    bool _accepted;
    QString _uri;
    std::atomic<ConnectionState> _state;
    WebSocketWorker(WebSocketConnection* thread);
    ~WebSocketWorker();
    websocketpp::lib::error_code write(websocketpp::connection_hdl, char const *, size_t);
    void on_message(websocketpp::connection_hdl hdl, message_ptr msg);
    bool on_validate(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);
    void on_open(websocketpp::connection_hdl hdl);
    void on_fail(websocketpp::connection_hdl hdl);
    void connectHandlers();
    void connectSocketSignals();
public Q_SLOTS:
    void handleConnection(qintptr socketDescriptor);
    void readyRead();
    void disconnected();
    void clientConnected();
    void connect();
    void error(QAbstractSocket::SocketError socketError);
    void sendText(const QString &msg);
    void sendBinary(const QByteArray& data);
    void close();
Q_SIGNALS:
    void opened();
    void closed();
    void messageReceived(const QByteArray& message);

};
}
#endif // WEBSOCKETWORKER_H
