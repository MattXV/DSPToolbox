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

            int generateIR();
            
            const signal& getIR() const { return ir; }

            void setFrequencyDependentIRs(const signal& ir, DSPTB_ERB_BAND irBand) {
                frequencyDependentIRs[irBand] = ir;
            }
            const signal& getFrequencyDependentIRs(DSPTB_ERB_BAND irBand) const { return frequencyDependentIRs[irBand]; }
        private:
            int checkIRs();

            Filter filter_125, filter_250, filter_500, filter_1000, filter_2000, filter_4000;
            std::array<signal, 6> frequencyDependentIRs;
            signal ir;
        DISABLE_COPY_ASSIGN(FilterBank)
    };


    signal poisson_dirac_sequence(const size_t& energy_hist, unsigned int hist_fs, float volume);
    signal dirac_sequence_weighting(const signal& energy_hist, const signal& dirac_sequence);

}
#endif

