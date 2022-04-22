#ifndef DSPTB_FILTER_HPP
#define DSPTB_FILTER_HPP
#pragma once
#include "utils.hpp"
#include <pffft.h>

namespace dsptb {
    signal fft_convolve(const signal& input, const signal& kernel);
    void normalise(signal& input, float scaling = 0.9f);

    inline float sinc(const float& x) { return (x == 0) ? 1 : sin(x) / x; }

    class Filter {
        public:
            enum class filterType { LPF, HPF, BPF };
            Filter(filterType fType, float cutoff);
            Filter(filterType fType, float lowerCutoff, float upperCutoff);
            ~Filter() = default;

            void convolveToSignal(signal& signal);

            const signal& getKernel() { return h; }
            unsigned int getSampleRate() const { return fs; }
            unsigned int getKernelSize() const { return M; }

        private:
            signal h;
            float lower_fc = 0.0f, upper_fc = 0.0f;
            const unsigned int& M = settings.dspKernelSize;
            const unsigned int& fs = settings.sampleRate;
        DISABLE_COPY_ASSIGN(Filter)
    };
}

#endif