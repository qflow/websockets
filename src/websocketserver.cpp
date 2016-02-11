#include "websocketserver.h"
#include "websocketserver_p.h"
#include "websocketconnection.h"

#include <iostream>
#include <fstream>
#include <QTcpServer>
#include <QThread>
#include <QTcpSocket>
#include <QDebug>


namespace QFlow{

WebSocketServerPrivate::WebSocketServerPrivate(WebSocketServer* parent) : QTcpServer(), _server(new WebsocketppServer()),
    q_ptr(parent)
{
    _server->clear_access_channels(websocketpp::log::alevel::all);
}
WebSocketServerPrivate::~WebSocketServerPrivate()
{

}
void WebSocketServerPrivate::incomingConnection(qintptr socketDescriptor)
{
    WebSocketConnection* con = new WebSocketConnection(this);
    con->start();
    con->handleConnection(socketDescriptor);
}


WebSocketServer::WebSocketServer(QObject *parent) : QObject(parent), d_ptr(new WebSocketServerPrivate(this))
{

}
WebSocketServer::~WebSocketServer()
{

}
int WebSocketServer::port() const
{
    Q_D(const WebSocketServer);
    return d->_port;
}
void WebSocketServer::setPort(int value)
{
    Q_D(WebSocketServer);
    d->_port = value;
}
QString WebSocketServer::host() const
{
    Q_D(const WebSocketServer);
    return d->_host;
}
void WebSocketServer::setHost(QString value)
{
    Q_D(WebSocketServer);
    d->_host = value;
}

ErrorInfo WebSocketServer::init()
{
    Q_D(WebSocketServer);
    d->listen(QHostAddress(d->_host), d->_port);
    return ErrorInfo();
}
void WebSocketServerPrivate::newConnection(WebSocketConnection *con)
{
    Q_Q(WebSocketServer);
    Q_EMIT q->newConnection(con);
}
}
