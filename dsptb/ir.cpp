#include "ir.hpp"

namespace dsptb {
    FilterBank::FilterBank() :
        filter_125(Filter::filterType::LPF, 250.0f),
        filter_250(Filter::filterType::BPF, 250.0f, 500.0f),
        filter_500(Filter::filterType::BPF, 500.0f, 1000.0f),
        filter_1000(Filter::filterType::BPF, 1000.0f, 2000.0f),
        filter_2000(Filter::filterType::BPF, 2000.0f, 4000.0f),
        filter_4000(Filter::filterType::HPF, 4000.0f)
    { }
    int FilterBank::generateIR(float _volume) {
        size_t n_samples = histograms[F_125].size();

        if (check_hists_length() != DSPTB_SUCCESS) return DSPTB_FAILURE;
        if (_volume < 0) {
            std::string error = "FilterBank::generateIR: invalid volume parameter! volume: " + std::to_string(_volume);
            DSPTB_ERROR(error);
            return DSPTB_FAILURE;
        }
        volume = _volume;

        signal dirac_sequence(n_samples, 0);
        poisson_dirac_sequence(dirac_sequence, n_samples, dsptb::settings.sampleRate, volume);

        Filter* filters[] = { &filter_125, &filter_250, &filter_500, &filter_1000, &filter_2000, &filter_4000 };

        for (int i = 0; i < 6; i++) {
            dirac_sequences[i] = signal(n_samples, 0);
            std::copy(dirac_sequence.begin(), dirac_sequence.end(), dirac_sequences[i].begin());

            dirac_sequence_weighting(histograms[i], dirac_sequence);
            filters[i]->convolve(histograms[i]);
        }
        monoaural_ir = signal(n_samples, 0);
        
        // Sum into broadband
        for (size_t i = 0; i < n_samples; i++) {
            float broadband = 0;
            for (int f = 0; f < 6; f++) {
                broadband += histograms[f][i];
            }
            monoaural_ir[i] = broadband;
        }
        //normalise(ir);
        DSPTB_LOG("Generated IR of len " << monoaural_ir.size() << " Volume: " << volume);
        return DSPTB_SUCCESS;
    }

    int FilterBank::check_hists_length() {
        size_t n_samples = histograms[F_125].size();

        if (n_samples == 0 || 
            (histograms[F_125].size()  != n_samples) ||
            (histograms[F_250].size()  != n_samples) ||
            (histograms[F_500].size()  != n_samples) ||
            (histograms[F_1000].size() != n_samples) ||
            (histograms[F_2000].size() != n_samples) ||
            (histograms[F_4000].size() != n_samples)) {
            DSPTB_ERROR("Filterbank::generateIR: All histograms must have the same length!");
            return DSPTB_FAILURE;
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

    void poisson_dirac_sequence(signal& out_sequence, size_t length, unsigned int fs, float volume) {
        std::random_device                       rand_dev;
        std::mt19937                             generator(rand_dev());
        std::uniform_real_distribution<float>    distr(0.001f, 1.0f);

        float value = 1.0f;
        size_t td = static_cast<size_t>(interval_t0(volume) * static_cast<float>(fs));
        while (td < length) {
            out_sequence[td] = value;
            size_t interval = static_cast<size_t>(delta_t(distr(generator), static_cast<float>(td) / static_cast<float>(fs), volume) * fs);
            td += std::max(interval, size_t(1));

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