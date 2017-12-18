#include "websocketworker.h"
#include "websocketconnection.h"
#include "websocketserver_p.h"
#include "websocketserver.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QUrl>
#include <QCoreApplication>

#if defined(Q_OS_LINUX) | defined(Q_OS_MACOS)
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

#ifdef Q_OS_WIN
#include <Winsock2.h>
#include <ws2tcpip.h>
#include "mstcpip.h"
#endif

namespace QFlow{

WebSocketWorker::WebSocketWorker(WebSocketConnection* thread) : QObject(), _connection(thread), _accepted(false), _state(ConnectionState::CLOSED)
{
    WebSocketServerPrivate* wsp = static_cast<WebSocketServerPrivate*>(thread->parent());
    if(wsp)
    {
        //is server
        _endpoint = wsp->_server;
    }
    else
    {
        //is client
        _endpoint.reset(new WebsocketppClient());
        _endpoint->clear_access_channels(websocketpp::log::alevel::all);
        //_client.get_elog().set_ostream(&std::cout);
    }
}
WebSocketWorker::~WebSocketWorker()
{
    close();
}
void WebSocketWorker::close()
{
    QCoreApplication::processEvents();
    ConnectionState expState = ConnectionState::OPENED;
    if(_state.compare_exchange_strong(expState, ConnectionState::CLOSED))
    {
        disconnectSocketSignals();
        _con->close(websocketpp::close::status::normal, "Web socket disposed by application");
        _con = nullptr;
        _socket->deleteLater();
        _socket = nullptr;
        _endpoint.reset(new WebsocketppClient());
        _endpoint->clear_access_channels(websocketpp::log::alevel::all);
        Q_EMIT closed();
    }
}

void WebSocketWorker::connectHandlers()
{
    _con->set_write_handler(bind(&WebSocketWorker::write,this,::_1,::_2,::_3));
    _con->set_message_handler(bind(&WebSocketWorker::on_message,this,::_1,::_2));
    _con->set_validate_handler(bind(&WebSocketWorker::on_validate,this,::_1));
    _con->set_close_handler(bind(&WebSocketWorker::on_close,this,::_1));
    _con->set_open_handler(bind(&WebSocketWorker::on_open,this,::_1));
     _con->set_fail_handler(bind(&WebSocketWorker::on_fail,this,::_1));
}
void WebSocketWorker::connectSocketSignals()
{
    QObject::connect(_socket, SIGNAL(connected()), this, SLOT(clientConnected()));
    QObject::connect(_socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    QObject::connect(_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    QObject::connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
}
void WebSocketWorker::disconnectSocketSignals()
{
    QObject::disconnect(_socket, SIGNAL(connected()), this, SLOT(clientConnected()));
    QObject::disconnect(_socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    QObject::disconnect(_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    QObject::disconnect(_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
}
void setKeepAlive(int socketHandle)
{
#ifdef Q_OS_WIN
    struct tcp_keepalive {
        u_long  onoff;
        u_long  keepalivetime;
        u_long  keepaliveinterval;
    } alive;
    alive.onoff = TRUE;
    alive.keepalivetime = 6000;
    alive.keepaliveinterval = 3000;
    DWORD dwBytesRet=0;
    int res = WSAIoctl(socketHandle, SIO_KEEPALIVE_VALS, &alive, sizeof(alive),
                NULL, 0, &dwBytesRet, NULL, NULL);
#endif
#ifdef Q_OS_LINUX
    int enableKeepAlive = 1;
    int res = setsockopt(socketHandle, SOL_SOCKET, SO_KEEPALIVE, &enableKeepAlive, sizeof(enableKeepAlive));

    int maxIdle = 10; /* seconds */
    res = setsockopt(socketHandle, IPPROTO_TCP, TCP_KEEPIDLE, &maxIdle, sizeof(maxIdle));

    int count = 3;  // send up to 3 keepalive packets out, then disconnect if no response
    res = setsockopt(socketHandle, SOL_TCP, TCP_KEEPCNT, &count, sizeof(count));

    int interval = 2;   // send a keepalive packet out every 2 seconds (after the 5 second idle period)
    res = setsockopt(socketHandle, SOL_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
#endif
#ifdef Q_OS_MACOS
	
	int enableKeepAlive = 1;
	int res = setsockopt(socketHandle, SOL_SOCKET, SO_KEEPALIVE, &enableKeepAlive, sizeof(enableKeepAlive));
	
	int maxIdle = 10; /* seconds */
	res = setsockopt(socketHandle, IPPROTO_TCP, TCP_KEEPALIVE, &maxIdle, sizeof(maxIdle));
	
#endif
}

void WebSocketWorker::handleConnection(qintptr socketDescriptor)
{
    _socket = new QTcpSocket(this);
    _socket->setSocketDescriptor(socketDescriptor);
    connectSocketSignals();
    setKeepAlive(_socket->socketDescriptor());
}
void WebSocketWorker::readyRead()
{
    QByteArray bytes = _socket->readAll();
    _con->read_all(bytes.data(), bytes.length());
}
websocketpp::lib::error_code WebSocketWorker::write(websocketpp::connection_hdl, char const * msg, size_t size)
{
    if (_socket->isOpen())
    {
    	qint64 bytesWritten = _socket->write(msg, size);
        if(bytesWritten != (qint64)size)
        {
            return websocketpp::error::make_error_code(websocketpp::error::invalid_state);
        }
        return websocketpp::lib::error_code();
    }
    else
    {
        return websocketpp::error::make_error_code(websocketpp::error::bad_connection);
    }
}
void WebSocketWorker::connect()
{
    _socket = new QTcpSocket(this);
    QUrl url(_uri);
    connectSocketSignals();

    _state.store(ConnectionState::CONNECTING);
	int defaultPort = 80;
	if (url.scheme() == "wss")
	{
		defaultPort = 443;
	}
    _socket->connectToHost(url.host(), url.port(defaultPort));

}
void WebSocketWorker::clientConnected()
{
    setKeepAlive(_socket->socketDescriptor());

    websocketpp::lib::error_code ec;
    std::string uri = _uri.toStdString();
    WebsocketppClient* client = (WebsocketppClient*)_endpoint.data();
    _con = client->get_connection(uri, ec);
    connectHandlers();
    for(QString subprotocol: _requestedSubprotocols)
    {
        _con->add_subprotocol(subprotocol.toStdString());
    }

    client->connect(_con);
}
QAbstractSocket::SocketState WebSocketWorker::state() const
{
	return !_socket.isNull() ? _socket->state() : QAbstractSocket::UnconnectedState;
}

void WebSocketWorker::error(QAbstractSocket::SocketError socketError)
{
    qDebug() << socketError;
    ConnectionState expState = ConnectionState::OPENED;
    if(_state.compare_exchange_strong(expState, ConnectionState::CLOSED)) Q_EMIT closed();
    expState = ConnectionState::CONNECTING;
    if(_state.compare_exchange_strong(expState, ConnectionState::CLOSED)) Q_EMIT closed();
}
void WebSocketWorker::disconnected()
{
    qDebug() << "disconnected";
    ConnectionState opened = ConnectionState::OPENED;
    if(_state.compare_exchange_strong(opened, ConnectionState::CLOSED)) Q_EMIT closed();
}
void WebSocketWorker::on_message(websocketpp::connection_hdl /*hdl*/, message_ptr msg)
{
   std::string s = msg->get_payload();
   QByteArray result(s.data(), s.length());
   Q_EMIT messageReceived(result);
}
void WebSocketWorker::on_close(websocketpp::connection_hdl /*hdl*/)
{
    ConnectionState opened = ConnectionState::OPENED;
    if(_state.compare_exchange_strong(opened, ConnectionState::CLOSED)) Q_EMIT closed();
}
void WebSocketWorker::on_fail(websocketpp::connection_hdl /*hdl*/)
{
    ConnectionState opened = ConnectionState::VALIDATING;
    if(_state.compare_exchange_strong(opened, ConnectionState::CLOSED)) Q_EMIT closed();
}

void WebSocketWorker::on_open(websocketpp::connection_hdl /*hdl*/)
{
    _state.store(ConnectionState::OPENED);
    Q_EMIT opened();
}

bool WebSocketWorker::on_validate(websocketpp::connection_hdl hdl)
{
    WebSocketServerPrivate* server = (WebSocketServerPrivate*)_connection->parent();
    WebsocketppServer::connection_ptr con = _endpoint->get_con_from_hdl(hdl);
    const std::vector<std::string> & subp_requests = con->get_requested_subprotocols();
    std::vector<std::string>::const_iterator it;
    QStringList list;
    for (it = subp_requests.begin(); it != subp_requests.end(); ++it) {
            QString subprotocol = QString::fromStdString(*it);
            list << subprotocol;
        }
    _requestedSubprotocols = list;
    _state.store(ConnectionState::VALIDATING);
    QMutexLocker lock(&_mutex);
    server->newConnection(_connection);
    _pendingValidation.wait(&_mutex);
    return _accepted;
}
void WebSocketWorker::sendText(const QString &msg)
{
    ConnectionState state = _state.load();
    if(state != ConnectionState::OPENED)
    {
        qDebug() << "FATAL: connection not in opened state";
    }
    else
    {
        _con->send(msg.toStdString(), websocketpp::frame::opcode::text);
    }
}
void WebSocketWorker::sendBinary(const QByteArray& data)
{
    ConnectionState state = _state.load();
    if(state != ConnectionState::OPENED)
    {
        qDebug() << "FATAL: connection not in opened state";
    }
    else
    {
        _con->send(data.toStdString(), websocketpp::frame::opcode::binary);
    }
}
}
