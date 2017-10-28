#include <wpn114/audio/backend/unit_base.hpp>
#include <wpn114/audio/backend/sndfile_support.hpp>

namespace wpn114 {
namespace audio {
namespace plugins {

class oneshots final : public wpn114::audio::unit_base
{

public:

#ifdef WPN_OSSIA
    void net_expose(ossia::net::device_base* application_node)
    {
        auto root = application_node->get_root_node().create_child(m_name);
        auto play_node = root->create_child("play");
        auto level_node = root->create_child("level");

        //auto play_node = ossia::net::find_or_create_node(root_node, "/oneshots/play");
        //auto play_node = ossia::net::find_or_create_node(root_node, "/oneshots/level");

        auto play_param = play_node->create_parameter(ossia::val_type::IMPULSE);
        auto level_param = level_node->create_parameter(ossia::val_type::FLOAT);

        play_param->add_callback([&](const ossia::value& v) {
            m_active = true;
        });

        level_param->add_callback([&](const ossia::value& v) {
            m_level = v.get<float>();
        });


    }
#endif

    oneshots(const char* name, const char* sfpath, float default_level) :
        m_name(name), m_sf_path(sfpath),
        m_level(default_level),
        m_sample_pos(0), m_active(false)
    {
        SETN_INPUTS     (0);
        SET_UTYPE       (unit_type::GENERATOR_UNIT);

        load_soundfile  (m_sf_buffer, m_sf_path);

        SETN_OUTPUTS    (m_sf_buffer.num_channels);
    }

    void initialize(uint16_t samples_per_buffer) override {}
    void process_audio(uint16_t samples_per_buffer) override
    {
        for(int i = 0; i < samples_per_buffer; ++i)
        {
            for(int j = 0; i < N_OUTPUTS; ++i)
            {
                OUT[j][i] = *m_sf_buffer.data++;
            }
        }
    }

private:

    std::string             m_name;
    std::string             m_sf_path;
    sndbuf_t                m_sf_buffer;
    uint32_t                m_sample_pos;
    float                   m_level;
};

}
}
}
