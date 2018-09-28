#pragma once

#include <QObject>
#include <QQmlListProperty>

#include <QQmlParserStatus>
#include <QAudioFormat>
#include <QAudioInput>
#include <QAudioOutput>
#include <QIODevice>
#include <QVector>

struct StreamProperties
{
    quint32 sample_rate;
    quint16 block_size;
};

/*class ControlNode : public QObject
{
    Q_OBJECT
    Q_PROPERTY  ( bool audioRate READ audioRate WRITE setAudioRate )
    Q_PROPERTY  ( qreal from READ from WRITE setFrom )

};*/

class StreamNode : public QObject
{
    Q_OBJECT
    Q_CLASSINFO ( "DefaultProperty", "subnodes" )

    Q_PROPERTY  ( bool mute READ mute WRITE setMute NOTIFY muteChanged )
    Q_PROPERTY  ( bool active READ active WRITE setActive NOTIFY activeChanged )
    Q_PROPERTY  ( int numInputs READ numInputs WRITE setNumInputs NOTIFY numInputsChanged )
    Q_PROPERTY  ( int numOutputs READ numOutputs WRITE setNumOutputs NOTIFY numOutputsChanged )
    Q_PROPERTY  ( QVariant parentChannels READ parentChannels WRITE setParentChannels )
    Q_PROPERTY  ( qreal level READ level WRITE setLevel NOTIFY levelChanged )
    Q_PROPERTY  ( qreal dBlevel READ dBlevel WRITE setDBlevel )
    Q_PROPERTY  ( QQmlListProperty<StreamNode> subnodes READ subnodes )

    public:
    StreamNode();
    static void allocateBuffer(float**& buffer, quint16 nchannels, quint16 nsamples );
    static void resetBuffer(float**& buffer, quint16 nchannels, quint16 nsamples );

    virtual float** userProcess ( float** buf, qint64 le ) = 0;
    virtual void userInitialize(qint64) = 0;

    virtual void initialize     ( StreamProperties properties);
    virtual float** process     ( float** buf, qint64 le );

    QQmlListProperty<StreamNode>  subnodes();
    const QVector<StreamNode*>&   getSubnodes() const { return m_subnodes; }

    void appendSubnode      ( StreamNode* );
    int subnodesCount       ( ) const;
    StreamNode* subnode     ( int ) const;
    void clearSubnodes      ( );

    float** inputBuffer  ( ) const { return m_in; }
    uint16_t numInputs   ( ) const { return m_num_inputs; }
    uint16_t numOutputs  ( ) const { return m_num_outputs; }
    uint16_t maxOutputs  ( ) const { return m_max_outputs; }
    qreal level          ( ) const { return m_level; }
    bool mute            ( ) const { return m_mute; }
    bool active          ( ) const { return m_active; }
    qreal dBlevel        ( ) const { return m_db_level; }

    void setNumInputs    ( uint16_t num_inputs );
    void setNumOutputs   ( uint16_t num_outputs );
    void setMaxOutputs   ( uint16_t max_outputs );
    void setMute         ( bool mute );
    void setActive       ( bool active );

    QVector<quint16> parentChannelsVec ( ) const;
    QVariant parentChannels ( ) const { return m_parent_channels; }
    void setParentChannels  ( QVariant pch );

    void setLevel   ( qreal level );
    void setDBlevel ( qreal db );

    signals:
    void muteChanged        ( );
    void activeChanged      ( );
    void numInputsChanged   ( );
    void numOutputsChanged  ( );
    void levelChanged       ( );

    protected:
    static void appendSubnode     ( QQmlListProperty<StreamNode>*, StreamNode* );
    static int subnodesCount      ( QQmlListProperty<StreamNode>* );
    static StreamNode* subnode    ( QQmlListProperty<StreamNode>*, int );
    static void clearSubnodes     ( QQmlListProperty<StreamNode>* );

    StreamProperties m_stream_properties;
    qreal m_level;
    qreal m_db_level;
    uint16_t m_num_inputs;
    uint16_t m_num_outputs;
    uint16_t m_max_outputs;
    bool m_mute;
    bool m_active;

    float** m_in;
    float** m_out;

    QVariant m_parent_channels;
    QVector<StreamNode*> m_subnodes;

    #define SAMPLERATE m_stream_properties.sample_rate
    #define SETN_OUT(n) setNumOutputs(n);
    #define SETN_IN(n) setNumInputs(n);
};

class WorldStream : public QIODevice, public QQmlParserStatus
{
    Q_OBJECT
    Q_CLASSINFO     ( "DefaultProperty", "inputs" )
    Q_INTERFACES    ( QQmlParserStatus )

    Q_PROPERTY      ( bool mute READ mute WRITE setMute NOTIFY muteChanged )
    Q_PROPERTY      ( bool active READ active WRITE setActive NOTIFY activeChanged )
    Q_PROPERTY      ( int numInputs READ numInputs WRITE setNumInputs NOTIFY numInputsChanged )
    Q_PROPERTY      ( int numOutputs READ numOutputs WRITE setNumOutputs NOTIFY numOutputsChanged )
    Q_PROPERTY      ( qreal level READ level WRITE setLevel NOTIFY levelChanged )
    Q_PROPERTY      ( int sampleRate READ sampleRate WRITE setSampleRate NOTIFY sampleRateChanged )
    Q_PROPERTY      ( int blockSize READ blockSize WRITE setBlockSize NOTIFY blockSizeChanged )
    Q_PROPERTY      ( QString inDevice READ inDevice WRITE setInDevice NOTIFY inDeviceChanged )
    Q_PROPERTY      ( QString outDevice READ outDevice WRITE setOutDevice NOTIFY outDeviceChanged )
    Q_PROPERTY      ( QQmlListProperty<StreamNode> inputs READ inputs )

    public:    
    WorldStream();

    virtual void classBegin()                           override {}
    virtual void componentComplete()                    override;
    virtual qint64 readData ( char*, qint64 )           override;
    virtual qint64 writeData ( const char*, qint64 )    override;
    virtual qint64 bytesAvailable ( )                   const override;

    Q_INVOKABLE void start  ();
    Q_INVOKABLE void stop   ();

    QQmlListProperty<StreamNode>  inputs();
    const QVector<StreamNode*>&   getInputs() const { return m_inputs; }

    void appendInput        ( StreamNode* );
    int inputCount          ( ) const;
    StreamNode* input       ( int ) const;
    void clearInputs        ( );

    uint16_t numInputs      ( ) const { return m_num_inputs; }
    uint16_t numOutputs     ( ) const { return m_num_outputs; }
    qreal level             ( ) const { return m_level; }
    bool mute               ( ) const { return m_mute; }
    bool active             ( ) const { return m_active; }
    uint32_t sampleRate     ( ) const { return m_sample_rate; }
    uint16_t blockSize      ( ) const { return m_block_size; }
    QString inDevice        ( ) const { return m_in_device; }
    QString outDevice       ( ) const { return m_out_device; }

    void setSampleRate   ( uint32_t sample_rate );
    void setBlockSize    ( uint16_t block_size );
    void setInDevice     ( QString device );
    void setOutDevice    ( QString device );
    void setNumInputs    ( uint16_t num_inputs );
    void setNumOutputs   ( uint16_t num_outputs );
    void setMute         ( bool mute );
    void setActive       ( bool active );
    void setLevel        ( qreal level );

    protected slots:
    void onAudioStateChanged ( QAudio::State ) const;

    signals:
    void sampleRateChanged  ( );
    void blockSizeChanged   ( );
    void inDeviceChanged    ( );
    void outDeviceChanged   ( );
    void muteChanged        ( );
    void activeChanged      ( );
    void numInputsChanged   ( );
    void numOutputsChanged  ( );
    void levelChanged       ( );

    private:
    static void appendInput     ( QQmlListProperty<StreamNode>*, StreamNode* );
    static int inputCount       ( QQmlListProperty<StreamNode>* );
    static StreamNode* input    ( QQmlListProperty<StreamNode>*, int );
    static void clearInputs     ( QQmlListProperty<StreamNode>* );

    qreal m_level;
    uint16_t m_num_inputs;
    uint16_t m_num_outputs;
    bool m_mute;
    bool m_active;
    uint32_t m_sample_rate;
    uint16_t m_block_size;
    QString m_in_device;
    QString m_out_device;
    QAudioFormat m_format;
    QAudioInput* m_input;
    QAudioOutput* m_output;
    float** m_pool;

    QVector<StreamNode*> m_inputs;
};
