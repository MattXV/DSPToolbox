#ifndef DSPTB_PROCESSING
#define DSPTB_PROCESSING
#include "filter.hpp"
#include <deque>
#include <list>
#include <mysofa.h>
#include <assert.h>

namespace dsptb {

    class OverlapAdd {
        friend class HRTF;
        public:
            OverlapAdd(int block_length, int ir_length, int channels);
            ~OverlapAdd();
            
            void processBlock(float* block);
            void setIRData(float* data);

        private:
            void prepare();

            PFFFT_Setup* fft_setup = nullptr;
            float* fftIn;
            float* fftOut;
            float* fftIr;
            float* fftWorkspace;

            const static int maxStackFFTSize = 16384;
            void tranformIR();

            std::list<std::deque<float>> previousResults;

            size_t channels, block_length, ir_length, fft_length;
            DISABLE_COPY_ASSIGN(OverlapAdd)

            static bool emptyResults(const std::deque<float>& result) { return result.empty(); }
    };

    
    class HRTF {
    public:
        HRTF(const char* _path) : path(_path) {}
        ~HRTF();

        int getFilters(float x, float y, float z, float* leftIR, float* rightIR, float* leftDelaySeconds, float* rightDelaySeconds);
        int setFiltersToOverlapAdd(OverlapAdd& overlapAdd, float x, float y, float z);
        int getFilterLength() const { return filter_length; }
        int load();

    private:
        int error;
        int filter_length;

        float* leftIR;
        float* rightIR;
        float* delayLeft;
        float *delayRight;
        std::string path;

        MYSOFA_EASY* hrtf;
        DISABLE_COPY_ASSIGN(HRTF)
    };

}

#endif