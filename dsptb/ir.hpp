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
            
            const signal& getIR() const { return ir; }

            void setFrequencyDependentIRs(const signal& ir, DSPTB_ERB_BAND irBand) {
                frequencyDependentIRs[irBand] = ir;
            }
            const signal& getFrequencyDependentIRs(DSPTB_ERB_BAND irBand) const { return frequencyDependentIRs[irBand]; }
        private:
            int checkIRs();
            int generateDiracDeltas();

            Filter filter_125, filter_250, filter_500, filter_1000, filter_2000, filter_4000;
            std::array<signal, 6> frequencyDependentIRs;
            std::array<signal, 6> diracDeltaSequences;
            signal ir;
            float volume;
        DISABLE_COPY_ASSIGN(FilterBank)
    };


    void poisson_dirac_sequence(float* out_sequence, size_t length, unsigned int fs, float volume);
    void dirac_sequence_weighting(signal& energy_hist, const signal& dirac_sequence);

}
#endif

