#pragma once

#include <src/audio/audio.hpp>

class Sharpen : public StreamNode
{
    Q_OBJECT
    Q_PROPERTY      ( qreal distortion READ distortion WRITE setDistortion )

public:
    Sharpen();

    virtual void userInitialize(qint64) override;
    virtual float** userProcess(float**, qint64) override;

    qreal   distortion() const { return m_distortion; }
    void    setDistortion(qreal dist) { m_distortion = dist; }

    private:
    qreal   m_distortion;

};


