#ifndef WEBSOCKETS_H
#define WEBSOCKETS_H

#include <QObject>
#include <QtPlugin>
#include <QQmlExtensionPlugin>

namespace QFlow{

class Websockets: public QQmlExtensionPlugin
{
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "Wamp" FILE "websockets_metadata.json")
public:
        void registerTypes(const char *uri);
};
}

#endif // WEBSOCKETS_H
