#include "operator_api/operator.h"
#include "operator_api/audio_operator.h"
#include "operator_api/drum_dsp.h"
#include "operator_api/midi_types.h"
#include "operator_api/type_id.h"

// ---------------------------------------------------------------------------
// DrumTom: Pitched sine + harmonics with pitch bend envelope
//
// SVF bandpass for body resonance. Pitch drops from (pitch * (1+bend))
// down to base pitch over bend_time.
// ---------------------------------------------------------------------------

struct DrumTom : vivid::AudioOperatorBase {
    static constexpr const char* kName   = "DrumTom";
    static constexpr bool kTimeDependent = true;

    vivid::Param<float> phase     {"phase",     0.0f,   0.0f,   1.0f};
    vivid::Param<float> pitch     {"pitch",   120.0f,  40.0f, 400.0f};
    vivid::Param<float> bend      {"bend",      0.3f,   0.0f,   1.0f};
    vivid::Param<float> bend_time {"bend_time", 0.08f,  0.01f,  0.3f};
    vivid::Param<float> color     {"color",     0.3f,   0.0f,   1.0f};
    vivid::Param<float> tone      {"tone",      0.5f,   0.0f,   1.0f};
    vivid::Param<float> decay     {"decay",     0.3f,   0.1f,   2.0f};
    vivid::Param<float> volume    {"volume",    0.8f,   0.0f,   1.0f};
    vivid::Param<int>   note      {"note",      45,     0,     127};

    drum::DecayEnvelope amp_env_;
    drum::DecayEnvelope bend_env_;
    drum::SVF           body_filter_;
    double              osc_phase_ = 0.0;
    float               prev_phase_ = 0.0f;

    DrumTom() {
        vivid::semantic_tag(phase, "phase_01");
        vivid::semantic_shape(phase, "scalar");

        vivid::semantic_tag(pitch, "frequency_hz");
        vivid::semantic_shape(pitch, "scalar");
        vivid::semantic_unit(pitch, "Hz");

        vivid::semantic_tag(bend_time, "time_seconds");
        vivid::semantic_shape(bend_time, "scalar");
        vivid::semantic_unit(bend_time, "s");

        vivid::semantic_tag(decay, "time_seconds");
        vivid::semantic_shape(decay, "scalar");
        vivid::semantic_unit(decay, "s");

        vivid::semantic_tag(volume, "amplitude_linear");
        vivid::semantic_shape(volume, "scalar");
    }

    void collect_params(std::vector<vivid::ParamBase*>& out) override {
        out.push_back(&phase);
        out.push_back(&pitch);
        out.push_back(&bend);
        out.push_back(&bend_time);
        out.push_back(&color);
        out.push_back(&tone);
        out.push_back(&decay);
        out.push_back(&volume);
        out.push_back(&note);
    }

    void collect_ports(std::vector<VividPortDescriptor>& out) override {
        out.push_back({"output", VIVID_PORT_AUDIO, VIVID_PORT_OUTPUT});
        out.push_back(VIVID_HANDLE_PORT("midi_in", VIVID_PORT_INPUT, VividMidiBuffer));
    }

    void process_audio(const VividAudioContext* ctx) override {
        float* out = ctx->output_buffers[0];
        double sr  = ctx->sample_rate;
        double inv_sr = 1.0 / sr;

        float p_base   = pitch.value;
        float bnd      = bend.value;
        float bnd_time = bend_time.value;
        float col      = color.value;
        float tn       = tone.value;
        float dec      = decay.value;
        float vol      = volume.value;
        float cur_phase = phase.value;

        // Body filter: bandpass centered on pitch, amount controlled by tone
        float filter_cutoff = p_base * (1.0f + tn * 2.0f);
        float filter_reso   = 0.2f + tn * 0.4f;

        // Check for MIDI trigger
        bool midi_triggered = false;
        float midi_vel_scale = 1.0f;
        if (ctx->input_handles && ctx->input_handle_count > 0 && ctx->input_handles[0]) {
            auto* midi = static_cast<const VividMidiBuffer*>(ctx->input_handles[0]);
            uint8_t target_note = static_cast<uint8_t>(note.int_value());
            for (uint32_t m = 0; m < midi->count; ++m) {
                const auto& msg = midi->messages[m];
                if ((msg.status & 0xF0) == 0x90 && msg.data2 > 0 && msg.data1 == target_note) {
                    midi_triggered = true;
                    midi_vel_scale = msg.data2 / 127.0f;
                    break;
                }
            }
        }

        for (uint32_t i = 0; i < ctx->buffer_size; i++) {
            bool trig = (i == 0) && (midi_triggered || drum::detect_trigger(cur_phase, prev_phase_));
            if (trig) {
                amp_env_.trigger();
                bend_env_.trigger();
                osc_phase_ = 0.0;
            }

            float amp  = amp_env_.value(dec);
            float benv = bend_env_.value(bnd_time);

            // Pitch bend: starts high, drops to base
            double freq = p_base * (1.0 + bnd * benv);

            // Sine + harmonics
            double body = audio_dsp::harmonics_3(osc_phase_, col);

            // Bandpass filter for body resonance
            float sample = static_cast<float>(body) * amp;
            if (tn > 0.0f) {
                sample = body_filter_.process(sample, filter_cutoff, filter_reso,
                                               static_cast<float>(sr), drum::SVF::BP);
            }

            out[i] = sample * vol * (midi_triggered ? midi_vel_scale : 1.0f);

            osc_phase_ += freq * inv_sr;
            if (osc_phase_ >= 1.0) osc_phase_ -= 1.0;
            amp_env_.advance(inv_sr);
            bend_env_.advance(inv_sr);
        }

        prev_phase_ = cur_phase;
    }
};

VIVID_REGISTER(DrumTom)
