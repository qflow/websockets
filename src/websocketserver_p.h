#ifndef WEBSOCKETSERVER_P_H
#define WEBSOCKETSERVER_P_H

#include <QTcpServer>

#include <websocketpp/config/core.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/error.hpp>
typedef websocketpp::server<websocketpp::config::core> WebsocketppServer;
typedef WebsocketppServer::message_ptr message_ptr;

namespace QFlow{

class WebSocketServer;
class WebSocketConnection;
class WebSocketServerPrivate : public QTcpServer
{
public:
    WebSocketServerPrivate(WebSocketServer* parent);
    ~WebSocketServerPrivate();
    void incomingConnection(qintptr socketDescriptor);
    void on_message(websocketpp::connection_hdl hdl, message_ptr msg);
    int _port;
    QString _host;
    std::shared_ptr<WebsocketppServer> _server;
    void newConnection(WebSocketConnection* con);
private:
    WebSocketServer* q_ptr;
    Q_DECLARE_PUBLIC(WebSocketServer)
};
}
#endif // WEBSOCKETSERVER_P_H
