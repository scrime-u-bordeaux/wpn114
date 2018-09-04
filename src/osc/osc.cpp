#include "osc.hpp"
#include <QDataStream>
#include <QNetworkDatagram>

OSCHandler::OSCHandler() :
    m_remote_address("127.0.0.1"), m_remote_port(8889), m_local_port(8888),
    m_udpsocket(new QUdpSocket(this))
{
    QObject::connect(m_udpsocket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));
}

void OSCHandler::componentComplete()
{    
    m_udpsocket->bind(QHostAddress::LocalHost, m_local_port);
}

void OSCHandler::setLocalPort(uint16_t port)
{
    m_local_port = port;
    if ( m_udpsocket->isOpen() ) m_udpsocket->close();

    m_udpsocket->bind(QHostAddress::LocalHost, m_local_port);
}

void OSCHandler::setRemotePort(uint16_t port)
{
    m_remote_port = port;
}

void OSCHandler::setRemoteAddress(QString address)
{
    m_remote_address = address;
}

template<typename T>
inline void parseArgumentsFromStream(QVariantList& dest, QDataStream& stream)
{
    T value;

    stream  >> value;
    dest    << value;
}

void OSCHandler::readOSCBundle(QByteArray bundle)
{
    QDataStream stream(bundle);

    // skipping 16 first bytes, including time-tag, useless for now
    stream.skipRawData(16);

    while ( !stream.atEnd() )
    {
        quint32     element_sz;
        stream >>   element_sz;
        QByteArray  message(element_sz, '\0');

        stream.readRawData  ( message.data(), element_sz );
        readOSCMessage      ( message );
    }
}

void OSCHandler::readOSCMessage(QByteArray message)
{
    QString         address, typetag;
    QVariantList    arguments;
    QDataStream     stream(message);

    stream.setFloatingPointPrecision ( QDataStream::SinglePrecision );
    auto spl = message.split(',');

    address = spl[0];
    typetag = spl[1].split('\0')[0];

    uint8_t adpads = 4-(address.size()%4);
    uint8_t ttpads = 4-((typetag.size()+1)%4);
    uint32_t total = address.size()+adpads+typetag.size()+ttpads+1;

    stream.skipRawData(total);

    for ( const QChar& c : typetag )
    {
        if        ( c == 'i' ) parseArgumentsFromStream<int>(arguments, stream);
        else if   ( c == 'f' ) parseArgumentsFromStream<float>(arguments, stream);
        else if   ( c == 's' )
        {
            quint8 byte, padding;
            QString res;
            stream >> byte;

            while ( byte )
            {
                res.append(byte);
                stream >> byte;
            }

            // we add padding after string's end
            padding = 4-(res.size()%4);
            stream.skipRawData(padding-1);

            arguments << res;
        }
    }

    qDebug() << "OSCMessage received:" << address << arguments;
    emit messageReceived(address, arguments);

}

void OSCHandler::readPendingDatagrams()
{
    while ( m_udpsocket->hasPendingDatagrams() )
    {
        QNetworkDatagram datagram = m_udpsocket->receiveDatagram();
        QByteArray data = datagram.data();

        if       ( data.startsWith('#') ) readOSCBundle(data);
        else if  ( data.startsWith('/') ) readOSCMessage(data);
    }
}

void OSCHandler::sendMessage(QString address, QVariantList arguments)
{
    QString             typetag;
    QByteArray          data;
    QNetworkDatagram    datagram;

    typetag.append(',');
    for ( const auto& var : arguments )
    {
        switch(var.type())
        {
        case QMetaType::Void:       break;
        case QMetaType::Bool:       typetag.append('i'); break;
        case QMetaType::Int:        typetag.append('i'); break;
        case QMetaType::Float:      typetag.append('f'); break;
        case QMetaType::Double:     typetag.append('f'); break;
        case QMetaType::QString:    typetag.append('s'); break;
        }
    }

    data.append(address);

    auto    pads = 4-(address.size() % 4);
    while   ( pads-- ) data.append((char)0);

    data.append(typetag);

    pads    = 4-(typetag.size() % 4);
    while   ( pads-- ) data.append((char)0);

    for ( const auto& var : arguments )
    {
        switch(var.type())
        {
        case QMetaType::Void: break;
        case QMetaType::Bool:
        {
            QDataStream stream(&data, QIODevice::ReadWrite);
            stream.skipRawData(data.size());
            stream << var.toInt();
            break;
        }
        case QMetaType::Int:
        {
            QDataStream stream(&data, QIODevice::ReadWrite);
            stream.skipRawData(data.size());
            stream << var.toInt();
            break;
        }
        case QMetaType::Float:
        {
            QDataStream stream(&data, QIODevice::ReadWrite);
            stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
            stream.skipRawData(data.size());
            stream << (float) var.toFloat();
            break;
        }
        case QMetaType::Double:
        {
            QDataStream stream ( &data, QIODevice::ReadWrite );
            stream.setFloatingPointPrecision ( QDataStream::SinglePrecision );
            stream.skipRawData ( data.size() );
            stream << (float) var.toFloat();
            break;
        }
        case QMetaType::QString:
        {
            QString str = var.toString();
            pads = 4-(var.toString().size() % 4);
            while(pads--)
                str.append('\0');
            data.append(str);
            break;
        }
        }
    }

    datagram.setData            ( data );
    datagram.setDestination     ( QHostAddress(m_remote_address), m_remote_port );
    m_udpsocket->writeDatagram  ( datagram );

}
