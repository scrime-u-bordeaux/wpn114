#include "query-server.hpp"
#include <QDataStream>
#include <QJsonObject>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QDateTime>
#include <QRegExp>
#include <QBitArray>

QJsonObject HostExtensions::toJson() const
{
    QJsonObject ext
    {
        { "ACCESS", access },
        { "VALUE", value },
        { "RANGE", range },
        { "DESCRIPTION", description },
        { "TAGS", tags },
        { "EXTENDED_TYPE", extended_type },
        { "UNIT", unit },
        { "CRITICAL", critical },
        { "CLIPMODE", clipmode },
        { "LISTEN", listen },
        { "PATH_CHANGED", path_changed },
        { "PATH_REMOVED", path_removed },
        { "PATH_ADDED", path_added },
        { "PATH_RENAMED", path_renamed }
    };

    return ext;
}

QJsonObject HostSettings::toJson() const
{
    QJsonObject obj
    {
        { "NAME", name },
        { "OSC_PORT", osc_port },
        { "OSC_TRANSPORT", osc_transport },
        { "OSC_STREAMING", osc_streaming },
        { "EXTENSIONS", extensions.toJson() }
    };

    return obj;
}

OSCQueryServer::OSCQueryServer() : OSCQueryDevice (), m_ws_server(new WPNWebSocketServer(5678))
{
    // default settings & extensions

    m_settings.name             = "WPN-SERVER";
    m_settings.tcp_port         = 5678;
    m_settings.osc_port         = 1234;
    m_settings.osc_transport    = "UDP";
    m_settings.osc_streaming    = true;

    m_settings.extensions.access            = false;
    m_settings.extensions.clipmode          = false;
    m_settings.extensions.critical          = false;
    m_settings.extensions.description       = false;
    m_settings.extensions.extended_type     = false;
    m_settings.extensions.listen            = false;
    m_settings.extensions.path_added        = false;
    m_settings.extensions.path_changed      = false;
    m_settings.extensions.path_removed      = false;
    m_settings.extensions.path_renamed      = false;
    m_settings.extensions.range             = false;
    m_settings.extensions.tags              = false;
    m_settings.extensions.unit              = false;
    m_settings.extensions.value             = false;
}

void OSCQueryServer::componentComplete()
{
    QObject::connect( m_ws_server, SIGNAL(newConnection(WPNWebSocket*)),
                      this, SLOT(onNewConnection(WPNWebSocket*)));

    m_osc_hdl->setLocalPort     ( m_settings.osc_port );
    m_ws_server->setPort        ( m_settings.tcp_port );
    m_ws_server->start          ( );
}

void OSCQueryServer::setTcpPort(quint16 port)
{
    m_settings.tcp_port     = port;
    m_ws_server->setPort    ( port );
}

void OSCQueryServer::setUdpPort(quint16 port)
{
    m_settings.osc_port = port;
    m_osc_hdl->setLocalPort(port);
}

void OSCQueryServer::onNewConnection(WPNWebSocket* con)
{
    auto client = new OSCQueryClient(con);
    m_clients.push_back(client);

    QObject::connect ( client, SIGNAL(valueUpdate(QString,QVariantList)),
                      this, SLOT(onValueUpdate(QString,QVariantList)));

    QObject::connect ( client, SIGNAL(command(QString)), this, SLOT(onCommand(QString)));
}

void OSCQueryServer::onHttpRequest(WPNWebSocket* client, QString req)
{
    if ( req.contains("HOST_INFO") ) onHostInfoRequest();
    else if ( req.contains("GET /") )
    {
        auto spl = req.split(' ');
        onNamespaceRequest(spl[1]);
    }
}

void OSCQueryServer::onCommand(QString data)
{

}

void OSCQueryServer::onHostInfoRequest()
{
    for ( const auto& con : m_clients )
        con->writeWebSocket(m_settings.toJson());
}

void OSCQueryServer::onNamespaceRequest(QString method)
{

}


