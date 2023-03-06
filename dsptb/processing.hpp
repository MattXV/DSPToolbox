#ifndef DSPTB_PROCESSING
#define DSPTB_PROCESSING
#include "filter.hpp"
#include <list>

namespace dsptb {

    class OverlapSave {
        public:
            OverlapSave(int block_length, int channels, std::vector<float>&& ir);
            
            ~OverlapSave();
            
            void processBlock(float* block);
        private:
            void prepare();

            PFFFT_Setup* fft_setup = nullptr;
            float* fftIn;
            float* fftOut;
            float* fftIr;
            float* fftWorkspace;

            std::vector<float> ir;
            std::list<std::vector<float>> previousResults;

            size_t channels, block_length, fft_length;
            DISABLE_COPY_ASSIGN(OverlapSave)
    };
}

#endif