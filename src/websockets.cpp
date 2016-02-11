#include "websockets.h"
#include "qqml.h"
#include "websocketserver.h"
#include "websocketconnection.h"

namespace QFlow{

void Websockets::registerTypes(const char *uri)
{
    // @uri QFlow.Websockets
    qmlRegisterType<WebSocketServer>(uri, 1, 0, "WebSocketServer");
    qmlRegisterType<WebSocketConnection>(uri, 1, 0, "WebSocketConnection");
}
}
