#ifndef DSPTB_IR_HPP
#define DSPTB_IR_HPP
#pragma once
#include "filter.hpp"
#include <array>
#include <cmath>
#include <random>
#include <algorithm>

namespace dsptb {
    class FilterBank {
        public:
            FilterBank();
            ~FilterBank() {}

            int generateIR(float volume);
            
            const signal& getIR() const { return monoaural_ir; }

            void setFrequencyDependentIRs(const signal& _ir, DSPTB_ERB_BAND irBand) {
                histograms[irBand] = _ir;
            }
            const signal& getFrequencyDependentIRs(DSPTB_ERB_BAND irBand) const { return histograms[irBand]; }
            const signal& getDiracSequence(DSPTB_ERB_BAND band) const { return dirac_sequences[band]; }

        private:

            int check_hists_length();

            Filter filter_125, filter_250, filter_500, filter_1000, filter_2000, filter_4000;
            
            std::array<signal, 6> histograms;
            std::array<signal, 6> dirac_sequences;

            signal monoaural_ir;
            float volume;
        DISABLE_COPY_ASSIGN(FilterBank)
    };


    void poisson_dirac_sequence(signal& out_sequence, size_t length, unsigned int fs, float volume);
    void dirac_sequence_weighting(signal& energy_hist, const signal& dirac_sequence);

}
#endif

