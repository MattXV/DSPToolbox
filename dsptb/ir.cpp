#include "ir.hpp"

namespace dsptb {
    FilterBank::FilterBank() :
        filter_125(Filter::filterType::HPF, 25.0f),
        filter_250(Filter::filterType::BPF, 25.0f, 250.0f),
        filter_500(Filter::filterType::BPF, 250.0f, 500.0f),
        filter_1000(Filter::filterType::BPF, 500.0f, 1000.0f),
        filter_2000(Filter::filterType::BPF, 1000.0f, 2000.0f),
        filter_4000(Filter::filterType::LPF, 20000.0f)
    { }
    int FilterBank::generateIR(float _volume) {

        size_t ir_size = frequencyDependentIRs[F_125].size();
        if (checkIRs() != DSPTB_SUCCESS) return DSPTB_FAILURE;
        if (volume < 0) {
            DSPTB_ERROR("FilterBank::generateIR: invalid volume parameter!");
            return DSPTB_FAILURE;
        }
        volume = _volume;
        if (generateDiracDeltas() != DSPTB_SUCCESS) return DSPTB_FAILURE;

        // Sum into broadband
        ir.resize(ir_size);
        for (size_t i = 0; i < ir_size; i++) {
            ir[i] = 
                frequencyDependentIRs[F_125 ][i] +
                frequencyDependentIRs[F_250 ][i] +
                frequencyDependentIRs[F_500 ][i] +
                frequencyDependentIRs[F_1000][i] +
                frequencyDependentIRs[F_2000][i] +
                frequencyDependentIRs[F_4000][i];
        }
        normalise(ir);
        DSPTB_LOG("Generated IR of len " << ir.size());
        return DSPTB_SUCCESS;
    }

    int FilterBank::checkIRs() {
        size_t ir_size = frequencyDependentIRs[F_125].size();
        if (ir_size == 0 || 
            (frequencyDependentIRs[F_125].size()  != ir_size) ||
            (frequencyDependentIRs[F_250].size()  != ir_size) ||
            (frequencyDependentIRs[F_500].size()  != ir_size) ||
            (frequencyDependentIRs[F_1000].size() != ir_size) ||
            (frequencyDependentIRs[F_2000].size() != ir_size) ||
            (frequencyDependentIRs[F_4000].size() != ir_size)) {
            DSPTB_ERROR("Invalid dsptb_erb_band parameter! Aborting FilterBank::generateIR.");
            return DSPTB_FAILURE;
        }
        return DSPTB_SUCCESS;
    }

    int FilterBank::generateDiracDeltas() {
        size_t length = frequencyDependentIRs[F_125].size();
        signal dirac = signal(length, 0);
        poisson_dirac_sequence(dirac.data(), length, dsptb::settings.sampleRate, volume);
        for (int i = 0; i < 6; i++) {
            diracDeltaSequences[i] = dirac;
        }
        filter_125.convolveToSignal( diracDeltaSequences[F_125]);
        filter_250.convolveToSignal( diracDeltaSequences[F_250]);
        filter_500.convolveToSignal( diracDeltaSequences[F_500]);
        filter_1000.convolveToSignal(diracDeltaSequences[F_1000]);
        filter_2000.convolveToSignal(diracDeltaSequences[F_2000]);
        filter_4000.convolveToSignal(diracDeltaSequences[F_4000]);

        for (int i = 0; i < 6; i++) {
            dirac_sequence_weighting(frequencyDependentIRs[i], diracDeltaSequences[i]);
        }
        return DSPTB_SUCCESS;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Poisson Dirac Delta generation
    // Dirk Schroder
    ///////////////////////////////////////////////////////////////////////////



    inline float delta_t(float uniform_random_z, float simulation_time, float volume) {
        float u = (4.0f * dsptb::pi * powf(343, 3) * powf(simulation_time, 2)) / volume;
        return logf(1.0f / uniform_random_z) / u;
    }

    inline float interval_t0(float volume) {
        return powf((2.0f * volume * logf(2)) / (4.0f * dsptb::pi * powf(343, 3)), 1.0f / 3.0f);
    }

    void poisson_dirac_sequence(float* out_sequence, size_t length, unsigned int fs, float volume) {
        
        std::random_device                       rand_dev;
        std::mt19937                             generator(rand_dev());
        std::uniform_real_distribution<float>    distr(0.001f, 1.0f);

        float value = 1.0f;
        size_t td = static_cast<size_t>(interval_t0(volume) * static_cast<float>(fs));
        while (td < length) {
            out_sequence[td] = value;
            td += std::max(static_cast<size_t>(delta_t(distr(generator), static_cast<float>(td) / static_cast<float>(fs), volume) * fs), size_t(2));
            value *= -1.0f;
        }
    }

    void dirac_sequence_weighting(signal& energy_hist, const signal& dirac_sequence) {
        size_t len = energy_hist.size();

        for (size_t i = 0; i < len; i++) {
            float dirac = dirac_sequence[i];
            float weight = fabsf(energy_hist[i]) / (powf(std::max(fabsf(dirac), 0.001f), 1.0f));
            energy_hist[i] = dirac * sqrtf(weight);
        }
    }


}