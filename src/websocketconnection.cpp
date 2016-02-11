#include "websocketconnection.h"
#include "websocketworker.h"
#include "websocketserver_p.h"
#include <QTcpSocket>
#include <QUrl>

namespace QFlow{

WebSocketConnection::WebSocketConnection(QObject* parent) : QThread(parent), _worker(new WebSocketWorker(this))
{
    _worker->moveToThread(this);
    QObject::connect(_worker, SIGNAL(opened()), this, SIGNAL(opened()));
    QObject::connect(_worker, SIGNAL(closed()), this, SIGNAL(closed()));
    QObject::connect(_worker, SIGNAL(messageReceived(QByteArray)), this, SIGNAL(messageReceived(QByteArray)));
    WebSocketServerPrivate* wsp = static_cast<WebSocketServerPrivate*>(parent);
    if(wsp)
    {
        _worker->_con = wsp->_server->get_connection();
        _worker->connectHandlers();
        _worker->_con->start();
    }
}
WebSocketConnection::~WebSocketConnection()
{
    QMetaObject::invokeMethod(_worker, "deleteLater", Qt::BlockingQueuedConnection);
    exit();
    wait();
}
void WebSocketConnection::close()
{
    QMetaObject::invokeMethod(_worker, "close", Qt::BlockingQueuedConnection);
}

void WebSocketConnection::connect()
{
    start();
    QMetaObject::invokeMethod(_worker, "connect", Qt::QueuedConnection);
}

void WebSocketConnection::run()
{
    exec();
}
void WebSocketConnection::handleConnection(qintptr socketDescriptor)
{
    qRegisterMetaType<qintptr>("qintptr");
    QMetaObject::invokeMethod(_worker, "handleConnection", Qt::QueuedConnection, Q_ARG(qintptr, socketDescriptor));
}
void WebSocketConnection::selectSubprotocol(const QString& value)
{
    _worker->_con->select_subprotocol(value.toStdString());
    Q_EMIT subprotocolChanged();
}
QStringList WebSocketConnection::requestedSubprotocols() const
{
    return _worker->_requestedSubprotocols;
}
void WebSocketConnection::setRequestedSubprotocols(const QStringList &list)
{
    _worker->_requestedSubprotocols = list;
}
QString WebSocketConnection::uri() const
{
    return _worker->_uri;
}
void WebSocketConnection::setUri(const QString &value)
{
    _worker->_uri = value;
}

void WebSocketConnection::accept(bool accepted)
{
    QMutexLocker lock(&_worker->_mutex);
    _worker->_accepted = accepted;
    _worker->_pendingValidation.wakeAll();
}
QHostAddress WebSocketConnection::peerAddress() const
{
    return _worker->_socket->peerAddress();
}
void WebSocketConnection::sendBinary(const QByteArray &data)
{
    QMetaObject::invokeMethod(_worker, "sendBinary", Qt::QueuedConnection, Q_ARG(QByteArray, data));
}
void WebSocketConnection::sendText(const QString &msg)
{
    QMetaObject::invokeMethod(_worker, "sendText", Qt::QueuedConnection, Q_ARG(QString, msg));
}
QString WebSocketConnection::subprotocol() const
{
    if(isServer()) return QString::fromStdString( _worker->_con->get_subprotocol());
    return  responseHeader("Sec-WebSocket-Protocol");
}
QString WebSocketConnection::responseHeader(const QString &key) const
{
    return QString::fromStdString(_worker->_con->get_response_header(key.toStdString()));
}
bool WebSocketConnection::isServer() const
{
    return _worker->_con->is_server();
}
}
