#include "audio.hpp"
#include <QtDebug>
#include <qendian.h>
#include <cmath>
#include <src/oscquery/node.hpp>
#include <memory>

static const QStringList g_ignore = {
    "parentStream", "subnodes", "exposeDevice", "objectName", "exposePath"
};

static const QStringList g_stream =
{
    "active", "mute", "numInputs", "numOutputs", "parentChannels", "level", "dBlevel"
};

StreamNode::StreamNode() : m_level(1.0), m_db_level(0.0),
    m_num_inputs(0), m_num_outputs(0), m_max_outputs(0), m_parent_channels(0),
    m_mute(false), m_active(false),
    m_in(nullptr), m_out(nullptr),
    m_exp_device(nullptr), m_parent_stream(nullptr)
{

}

void StreamNode::setNumInputs(uint16_t num_inputs)
{
    if ( m_num_inputs != num_inputs ) emit numInputsChanged();
    m_num_inputs = num_inputs;
}

void StreamNode::setNumOutputs(uint16_t num_outputs)
{
    if ( m_num_outputs != num_outputs ) emit numOutputsChanged();
    m_num_outputs = num_outputs;

    QVariantList list;

    for ( quint16 ch = 0; ch < num_outputs; ++ch)
    {
        list.push_back(QVariant(ch));
        m_parent_channels = list;
    }

    if ( m_subnodes.isEmpty() ) setMaxOutputs(num_outputs);
}

void StreamNode::setMaxOutputs(uint16_t max_outputs)
{
    m_max_outputs = max_outputs;

    QVariantList list;
    for ( quint16 i = 0; i < m_max_outputs; ++i ) list << i;

    m_parent_channels = list;
}

void StreamNode::setMute(bool mute)
{
    if ( mute != m_mute ) emit muteChanged();
    m_mute = mute;
}

void StreamNode::setActive(bool active)
{
    if ( active != m_active ) emit activeChanged();
    m_active = active;
}

void StreamNode::setLevel(qreal level)
{
    if ( level != m_level ) emit levelChanged();
    m_level = level;
    m_db_level = std::log10(level)*(qreal)20.f;
}

void StreamNode::setDBlevel(qreal db)
{
    m_db_level = db;
    m_level = std::pow(10.f, db*.05);
}

void StreamNode::setExposePath(QString path)
{
    m_exp_path = path;

    if ( m_exp_device )
         m_exp_node = WPNDevice::findOrCreateNode(m_exp_device, path);

    else if ( auto dev = WPNDevice::instance() )
    {
        m_exp_device = dev;
        m_exp_node = WPNDevice::findOrCreateNode(dev, path);
    }

    else return;

    auto pcount = metaObject()->propertyCount();

    auto stream     = m_exp_node->createSubnode("control");
    auto parameters = m_exp_node->createSubnode("parameters");

    for ( quint16 i = 0; i < pcount; ++i )
    {
        auto property = metaObject()->property(i);
        QString name = property.name();

        WPNNode* node;

        if      ( g_ignore.contains(name) ) continue;
        else if ( g_stream.contains(name) )
             node = stream->createSubnode(property.name());
        else node = parameters->createSubnode(property.name());

        node->setTarget ( this, property );
    }
}

void StreamNode::setExposeDevice(WPNDevice* device)
{
    m_exp_device = device;
}

void StreamNode::setParentStream(StreamNode* stream)
{
    m_parent_stream = stream;
    m_parent_stream->appendSubnode(this);
}

//-------------------------------------------------------------------------------------------

QVector<quint16> StreamNode::parentChannelsVec() const
{
    QVector<quint16> res;

    if ( m_parent_channels.type() == QMetaType::QVariantList )
    {
        for ( const auto& ch : m_parent_channels.toList() )
            res << ch.toInt();
    }

    else if ( m_parent_channels.type() == QMetaType::Int )
        res << m_parent_channels.toInt();

    return res;

}

void StreamNode::setParentChannels(QVariant pch)
{
    m_parent_channels = pch;
}

QQmlListProperty<StreamNode> StreamNode::subnodes()
{
    return QQmlListProperty<StreamNode>( this, this,
                           &StreamNode::appendSubnode,
                           &StreamNode::subnodesCount,
                           &StreamNode::subnode,
                           &StreamNode::clearSubnodes );
}

StreamNode* StreamNode::subnode(int index) const
{
    return m_subnodes.at(index);
}

void StreamNode::appendSubnode(StreamNode* subnode)
{
    m_subnodes.append(subnode);
    if ( !m_num_inputs ) setMaxOutputs(subnode->maxOutputs());
}

int StreamNode::subnodesCount() const
{
    return m_subnodes.count();
}

void StreamNode::clearSubnodes()
{
    m_subnodes.clear();
}

// statics --

void StreamNode::appendSubnode(QQmlListProperty<StreamNode>* list, StreamNode* subnode)
{
    reinterpret_cast<StreamNode*>(list->data)->appendSubnode(subnode);
}

void StreamNode::clearSubnodes(QQmlListProperty<StreamNode>* list )
{
    reinterpret_cast<StreamNode*>(list->data)->clearSubnodes();
}

StreamNode* StreamNode::subnode(QQmlListProperty<StreamNode>* list, int i)
{
    return reinterpret_cast<StreamNode*>(list->data)->subnode(i);
}

int StreamNode::subnodesCount(QQmlListProperty<StreamNode>* list)
{
    return reinterpret_cast<StreamNode*>(list->data)->subnodesCount();
}

//-------------------------------------------------------------------------------------------

inline void StreamNode::allocateBuffer(float**& buffer, quint16 nchannels, quint16 nsamples )
{
    buffer = new float* [ nchannels ];
    for ( uint16_t ch = 0; ch < nchannels; ++ch )
        buffer[ch] = (float*) std::calloc(nsamples, sizeof(float));
}

void StreamNode::resetBuffer(float**& buffer, quint16 nchannels, quint16 nsamples )
{
    for ( uint16_t ch = 0; ch < nchannels; ++ch )
        memset(buffer[ch], 0.f, sizeof(float)*nsamples);
}

void StreamNode::mergeBuffers(float**& lhs, float** rhs, quint16 lnchannels,
                              quint16 rnchannels, quint16 nsamples )
{
    for ( quint16 ch = 0; ch < rnchannels; ++ch )
        for ( quint16 s = 0; s < nsamples; ++s )
            lhs[ch][s] += rhs[ch][s];
}

void StreamNode::initialize(StreamProperties properties)
{
    m_stream_properties = properties;
    StreamNode::allocateBuffer(m_in, m_num_inputs, properties.block_size);
    StreamNode::allocateBuffer(m_out, m_num_outputs, properties.block_size);

    userInitialize(properties.block_size);

    for ( const auto& subnode : m_subnodes )
        subnode->initialize(properties);
}

float** StreamNode::process(float** buf, qint64 le)
{
    if ( !m_num_inputs ) // if generator, pass the buffer down the chain
    {
        float** ubuf = userProcess(buf, le);

        for ( const auto& subnode : m_subnodes )
            if ( subnode->active() && subnode->numInputs() == m_num_outputs )
                ubuf = subnode->process(ubuf, le);

        return ubuf;
    }

    else
    {
        // mix all sources down to an array of channels
        float** ubuf = inputBuffer();

        for ( const auto& subnode : m_subnodes )
        {
            if ( subnode->active() )
            {
                auto pch     = subnode->parentChannelsVec();
                auto genbuf  = subnode->process(nullptr, le);

                for ( quint16 ch = 0; ch < pch.size(); ++ch )
                    for ( quint16 s = 0; s < le; ++s )
                        ubuf[pch[ch]][s] += genbuf[ch][s];
            }
        }
    }
}

//-----------------------------------------------------------------------------------------------

WorldStream::WorldStream() : m_sample_rate(44100), m_block_size(512)
{

}

void WorldStream::setSampleRate(uint32_t sample_rate)
{
    if ( sample_rate != m_sample_rate ) emit sampleRateChanged();
    m_sample_rate = sample_rate;
}

void WorldStream::setBlockSize(uint16_t block_size)
{
    if ( block_size != m_block_size ) emit blockSizeChanged();
    m_block_size = block_size;
}

void WorldStream::setInDevice(QString device)
{
    if ( device != m_in_device ) emit inDeviceChanged();
    m_in_device = device;
}

void WorldStream::setOutDevice(QString device)
{
    if ( device != m_out_device ) emit outDeviceChanged();
    m_out_device = device;
}

void WorldStream::componentComplete()
{
    QAudioFormat format;

    format.setCodec           ( "audio/pcm" );
    format.setByteOrder       ( QAudioFormat::LittleEndian );
    format.setSampleType      ( QAudioFormat::SignedInt );
    format.setSampleSize      ( 16 );
    format.setSampleRate      ( m_sample_rate );
    format.setChannelCount    ( m_num_outputs );

    auto device_info = QAudioDeviceInfo::defaultOutputDevice();

    if ( !m_out_device.isEmpty() || m_out_device != "default" )
    {
        auto devices = device_info.availableDevices(QAudio::AudioOutput);
        for ( const auto& device : devices )
        {
            if ( device.deviceName() == m_out_device )
            {
                device_info = device;
                break;
            }
        }
    }

    if ( !device_info.isFormatSupported(format) )
        qDebug() << "[AUDIO] Format not supported";

    QAudioOutput* outp = new QAudioOutput(device_info, format);
    m_stream = new AudioStream(*this, format, nullptr, outp);

    m_stream->moveToThread  ( &m_stream_thread );
    m_stream_thread.start   ( QThread::TimeCriticalPriority );

    QObject::connect ( outp, SIGNAL(stateChanged(QAudio::State)),
                      this, SLOT(onAudioStateChanged(QAudio::State)));

    QObject::connect ( this, SIGNAL(startStream()), m_stream, SLOT(start()));
    QObject::connect ( this, SIGNAL(stopStream()), m_stream, SLOT(stop()));
}

void WorldStream::onAudioStateChanged(QAudio::State state) const
{
    qDebug() << "[AUDIO]" << state;
}

void WorldStream::start()
{
    emit startStream();
}

void WorldStream::stop()
{
    emit stopStream();
}

// -----------------------------------------------------------------------------------------

AudioStream::AudioStream(
        const WorldStream& world, QAudioFormat format, QAudioInput* input, QAudioOutput* output) :
    m_world(world), m_format(format), m_input(input), m_output(output)
{

}

AudioStream::~AudioStream()
{
    delete m_input;
    delete m_output;
    delete m_pool;
}

void AudioStream::start()
{
    for ( const auto& input : m_world.m_subnodes )
        input->initialize({m_world.m_sample_rate, m_world.m_block_size});

    StreamNode::allocateBuffer(m_pool, m_world.m_num_outputs, m_world.m_block_size);

    open(QIODevice::ReadOnly);
    m_output->start(this);
}

void AudioStream::stop()
{
    m_output->stop();
}

qint64 AudioStream::readData(char* data, qint64 maxlen)
{
    auto inputs     = m_world.m_subnodes;
    quint16 nout    = m_world.m_num_outputs;
    quint16 bsize   = m_world.m_block_size;
    float** buf     = m_pool;
    qreal level     = m_world.m_level;

    StreamNode::resetBuffer(m_pool, nout, bsize);

    for ( const auto& input : inputs )
    {
        if ( !input->active() ) continue;

        float** cdata   = input->process ( nullptr, bsize );
        auto pch        = input->parentChannelsVec();

        for ( quint16 s = 0; s < bsize; ++s )
            for ( quint16 ch = 0; ch < pch.size(); ++ch )
                buf[pch[ch]][s] += ( cdata[ch][s] *level );
    }

        for ( quint16 s = 0; s < bsize; ++s )
        {
            for ( quint16 ch = 0; ch < nout; ++ch )
            {
                // convert to interleaved little endian int16
                qint16 sdata = static_cast<qint16>(buf[ch][s] * 32767);
                qToLittleEndian<qint16>(sdata, data);
                data += 2;
            }
        }


    // i.e. block_size * 2bytes per value * numChannels
    return ( bsize*2*nout );

}

qint64 AudioStream::writeData(const char* data, qint64 len)
{
    Q_UNUSED ( data );
    Q_UNUSED ( len );

    return 0;
}

qint64 AudioStream::bytesAvailable() const
{
    return 0;
}
