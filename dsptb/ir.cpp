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
    int FilterBank::generateIR() {

        size_t ir_size = frequencyDependentIRs[F_125].size();
        if (checkIRs() != DSPTB_SUCCESS) return DSPTB_FAILURE;

        filter_125.convolveToSignal(frequencyDependentIRs[F_125]);
        filter_250.convolveToSignal(frequencyDependentIRs[F_250]);
        filter_500.convolveToSignal(frequencyDependentIRs[F_500]);
        filter_1000.convolveToSignal(frequencyDependentIRs[F_1000]);
        filter_2000.convolveToSignal(frequencyDependentIRs[F_2000]);
        filter_4000.convolveToSignal(frequencyDependentIRs[F_4000]);

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



    inline float delta_t(float uniform_random_z, float simulation_time, float volume) {
        float u = (4.0f * dsptb::pi * powf(343, 3) * powf(simulation_time, 2)) / volume;
        return logf(1.0f / uniform_random_z) / u;
    }

    inline float interval_t0(float volume) {
        return powf((2.0f * volume * logf(2)) / (4.0f * dsptb::pi * powf(343, 3)), 1.0f / 3.0f);
    }

    signal poisson_dirac_sequence(const size_t& n_samples, unsigned int hist_fs, float volume) {
        
        std::random_device                       rand_dev;
        std::mt19937                             generator(rand_dev());
        std::uniform_real_distribution<float>    distr(0.001f, 1.0f);

        signal dirac_sequence = signal(n_samples, 0.0f);
        float value = 1.0f;
        size_t td = static_cast<size_t>(interval_t0(volume) * static_cast<float>(hist_fs));
        while (td < n_samples) {
            dirac_sequence[td] = value;
            td += std::max(static_cast<size_t>(delta_t(distr(generator), static_cast<float>(td) / static_cast<float>(hist_fs), volume) * hist_fs), size_t(2));
            value *= -1.0f;
        }
        return dirac_sequence;
    }

    signal dirac_sequence_weighting(const signal& energy_hist, const signal& dirac_sequence) {
        size_t len = energy_hist.size();
        signal weighted_ir = signal(len, 0);
        for (size_t i = 0; i < len; i++) {
            float dirac = dirac_sequence[i];
            float weight = fabsf(energy_hist[i]) / (powf(std::max(fabsf(dirac), 0.001f), 1.0f));
            weighted_ir[i] = dirac * sqrtf(weight);
        }
        return weighted_ir; 
    }


}