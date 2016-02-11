import qbs
import qbs.Probes as Probes

QmlPlugin
{
    name: "websockets"
    cpp.cxxLanguageVersion: "c++11"
    files: [
        "websocketconnection.cpp",
        "websocketconnection.h",
        "websockets.cpp",
        "websockets.h",
        "websockets_global.h",
        "websocketserver.cpp",
        "websocketserver.h",
        "websocketserver_p.h",
        "websocketworker.cpp",
        "websocketworker.h",
    ]

    pluginNamespace: "QFlow.WebSockets"
    pluginRootPath: "bin/plugins"
    cpp.includePaths: ["."]

    Depends{name: "websocketpp"}

    Depends{name: "Qt"; submodules: ["network", "websockets"]}
    Depends{name: "core"}
}
