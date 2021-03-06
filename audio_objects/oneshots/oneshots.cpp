#include "oneshots.hpp"

Oneshots::Oneshots() : m_pos(0), m_buf(0)
{
    SETN_OUT    ( 0 );
    SET_OFFSET  ( 0 );
}

Oneshots::~Oneshots()
{
    if ( m_buf ) delete m_buf;
}

void Oneshots::classBegin() {}
void Oneshots::componentComplete()
{
    m_buf = new sndbuf ( m_path.toStdString(), 0 );
    SETN_OUT    ( m_buf->nchannels );

    INITIALIZE_AUDIO_OUTPUTS;
    emit bufferLoaded();
}

float** Oneshots::process(const quint16 nsamples)
{
    auto pos            = m_pos;
    auto bufnsamples    = m_buf->nsamples;
    auto bufnframes     = m_buf->nframes;
    auto nch            = m_buf->nchannels;
    auto bufdata        = m_buf->data;
    auto out            = OUT;

    bufdata             += pos*nch;

    for ( int s = 0; s < nsamples; ++s )
    {
        if ( pos == bufnsamples )
        {
            //  fill with zeroes, stop activity for next callback
            for ( int ch = 0; ch < nch; ++ ch )
                out[ch][s] = 0.f;

            emit playbackEnd();
        }

        else if ( pos > bufnsamples )
        {
            //  fill the rest of the buffer with zeroes
            for ( int ch = 0; ch < nch; ++ ch )
                out[ch][s] = 0.f;
        }

        else
        {
            // normal behaviour
            for ( int ch = 0; ch < nch; ++ch )
            out[ch][s] = *bufdata++;
        }

        pos++;
    }

    // buffer is filled, we can deactivate and reset pos
    if ( pos >= bufnsamples )
    {
        setActive(false);
        pos = 0;
    }

    m_pos = pos;

    return m_outputs;
}

void Oneshots::setActive(const bool active)
{
    if      ( active && m_active ) m_pos = 0;
    else    m_active = active;
}

void Oneshots::setPath(const QString path)
{
    m_path = path;
}

QString Oneshots::path() const
{
    return m_path;
}
