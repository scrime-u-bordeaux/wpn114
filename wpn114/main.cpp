#include <iostream>

#include <wpn114/audio/backend/backend.hpp>
#include <wpn114/audio/backend/context.hpp>
#include <wpn114/audio/plugins/vst/vst.hpp>
#include <wpn114/control/plugins/push_1/push_controller.hpp>

using namespace std;

// a vst host for kaivo
// vst host for absynth
// vst host for altiverb

// custom granular / multifunction sampler
// libossia for communication
// ableton push support?

// dependencies:
// - vst3sdk
// - libossia
// - portaudio
// - libsndfile

// vst take float** inputs and outputs
//

using namespace wpn114;

int main(int argc, char* argv[])
{
    wpn114::audio::context.blocksize = 512;
    wpn114::audio::context.sample_rate = 44100;
    wpn114::audio::context.num_inputs = 0;
    wpn114::audio::context.num_outputs = 2;
    wpn114::audio::context.master_tempo = 120.f;

    //wpn114::audio::backend audio_backend(2);

    //wpn114::audio::units::plugin_handler kaivo_1("Kaivo.vst");
    //wpn114::audio::units::plugin_handler absynth_1("Absynth.vst");
    //wpn114::audio::units::plugin_handler altiverb("Altiverb.vst");
    //wpn114::audio::units::plugin_handler amplitube("Amplitube.vst");

    //wpn114::audio::units::fields("/path/to/soundfile.wav");

    //audio_backend.initialize_io();

    // init controller
    wpn114::control::midi::device_factory push_device_factory;
    std::string push_port_name = "Ableton Push User Port";

    auto push_hdl = push_device_factory.make_device_hdl(push_port_name,
                                 wpn114::control::device_io_type::IN_OUT);

    wpn114::control::midi::push_controller push(std::move(push_hdl));

    // init view (qml or command line)

    return 0;
}
