/* adaptation of remaincalm.org Paranoia Mangler JS effect for Reaper
 * author: Daniel Arena (dan@remaincalm.org)
*/

#ifndef MANGLER_HPP
#define MANGLER_HPP

#include <source/audio/audio.hpp>

class Mangler : public StreamNode
{
    Q_OBJECT

    Q_PROPERTY  ( qreal inputGain READ inputGain WRITE setInputGain )
    Q_PROPERTY  ( qreal wetOut READ wetOut WRITE setWetOut )
    Q_PROPERTY  ( qreal dryOut READ dryOut WRITE setDryOut )
    Q_PROPERTY  ( qreal badResampler READ badResampler WRITE setBadResampler )
    Q_PROPERTY  ( int bitcrusher READ bitcrusher WRITE setBitcrusher )
    Q_PROPERTY  ( qreal thermonuclear READ thermonuclear WRITE setThermonuclear )
    Q_PROPERTY  ( int bitdepth READ bitdepth WRITE setBitdepth )
    Q_PROPERTY  ( qreal gate READ gate WRITE setGate )
    Q_PROPERTY  ( qreal love READ love WRITE setLove )
    Q_PROPERTY  ( qreal jive READ jive WRITE setJive )
    Q_PROPERTY  ( int attitude READ attitude WRITE setAttitude )

    public:
    Mangler();

    virtual void initialize ( qint64 ) override;
    virtual float** process ( float**, qint64 ) override;

    qreal inputGain         ( ) const { return m_input_gain; }
    qreal wetOut            ( ) const { return m_wet_out; }
    qreal dryOut            ( ) const { return m_dry_out; }
    qreal badResampler      ( ) const { return m_bad_resampler; }
    int bitcrusher          ( ) const { return m_bitcrusher; }
    qreal thermonuclear     ( ) const { return m_thermonuclear; }
    int bitdepth            ( ) const { return m_bitdepth; }
    qreal gate              ( ) const { return m_gate; }
    qreal love              ( ) const { return m_love; }
    qreal jive              ( ) const { return m_jive; }
    int attitude            ( ) const { return m_attitude; }

    void setInputGain       ( qreal input_gain ) { m_input_gain = input_gain; }
    void setDryOut          ( qreal dry_out ) { m_dry_out = dry_out; }
    void setWetOut          ( qreal wet_out ) { m_wet_out = wet_out; }
    void setBadResampler    ( qreal bad_resampler ) { m_bad_resampler = bad_resampler; }
    void setBitcrusher      ( int bitcrusher ) { m_bitcrusher = bitcrusher; }
    void setThermonuclear   ( qreal thermonuclear ) { m_thermonuclear = thermonuclear; }
    void setBitdepth        ( int bitdepth ) { m_bitdepth = bitdepth; }
    void setGate            ( qreal gate ) { m_gate = gate; }
    void setLove            ( qreal love ) { m_love = love; }
    void setJive            ( qreal jive ) { m_jive = jive; }
    void setAttitude        ( int attitude ) { m_attitude = attitude; }

    private:
    qreal m_input_gain      = 0.0;
    qreal m_dry_out         = -3.0;
    qreal m_wet_out         = -3.0;
    qreal m_bad_resampler   = 12000.0;
    int m_bitcrusher        = 0;
    qreal m_thermonuclear   = 0.0;
    int m_bitdepth          = 8;
    qreal m_gate            = 0.0;
    qreal m_love            = 75.0;
    qreal m_jive            = 15.0;
    int m_attitude          = 1;

    void setBitPattern ( quint16 index,
    qint8 b1, qint8 b2, qint8 b3, qint8 b4, qint8 b5, qint8 b6, qint8 b7, qint8 b8 );

    // -----
    float itm1 = 0, itm2 = 0, otm1 = 0, otm2 = 0;
    float dcshift = 1;
    float relgain[ 17 ];
    qint8 lut[ 136 ];
    float shaper_amt = 0.857f;
    float shaper_amt_2 = 0.9f;

    //-------

    float sample_csr = 0;
    float per_sample = 0;
    float last_spl0 = 0;
    float last_spl1 = 0;
    float next_sample = 0;

    // -------

    float v0L = 0.f, v0R = 0.f;
    float v1L = 0.f, v1R = 0.F;
    float hv0L = 0.f, hv0R = 0.f;
    float hv1L = 0.f, hv1R = 0.f;

};

#endif // MANGLER_HPP
