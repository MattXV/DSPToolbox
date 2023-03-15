#include "processing.hpp"

namespace dsptb {
    OverlapAdd::OverlapAdd(int _block_length, int _ir_length, int _channels)
        : block_length(_block_length), ir_length(_ir_length), channels(_channels)
    {
        prepare();
    }
    void OverlapAdd::prepare() {
        // ir = std::vector<float>(1024, float(0));
        // ir[1] = 1.0f;
        
        fft_length = roundUpToNextPowerOfTwo((unsigned int)(ir_length * channels + block_length * channels - 1));

        if (fft_length <= maxStackFFTSize) {
            fftWorkspace = nullptr;          
        } 
        else {
            fftWorkspace = (float*)pffft_aligned_malloc(fft_length * sizeof(float));
        }
        fftIn  = (float*)pffft_aligned_malloc(fft_length * sizeof(float));
        fftOut = (float*)pffft_aligned_malloc(fft_length * sizeof(float));
        fftIr  = (float*)pffft_aligned_malloc(fft_length * sizeof(float));

        for (int i = 0; i < fft_length; i++) {
            fftIr[i] = 0;
        }
        fftIr[1] = 1.0f;
        tranformIR();
        fft_setup = pffft_new_setup((int)fft_length, pffft_transform_t::PFFFT_REAL);
        DSPTB_LOG("FFT size: " << fft_length << std::endl);
    }
    
    void OverlapAdd::tranformIR() {
        pffft_transform(fft_setup, fftIr, fftIr, fftWorkspace, pffft_direction_t::PFFFT_FORWARD);
    }

    void OverlapAdd::processBlock(float* block) {
        for (int i = 0; i < block_length * channels; i++) {
            fftIn[i] = block[i];
            block[i] = 0;
            fftOut[i] = 0; 
        }
        for (int i = block_length * channels; i < fft_length; i++) {
            fftIn[i] = 0;
            fftOut[i] = 0;
        }
        pffft_transform(fft_setup, fftIn, fftIn, fftWorkspace, pffft_direction_t::PFFFT_FORWARD);
        pffft_zconvolve_accumulate(fft_setup, fftIn, fftIr, fftOut, 1.0f);
        pffft_transform(fft_setup, fftOut, fftOut, fftWorkspace, pffft_direction_t::PFFFT_BACKWARD);
        previousResults.emplace_back(fftOut, fftOut + fft_length);

        for (std::deque<float>& previousResult : previousResults) {
            for (int sample = 0; !previousResult.empty() && sample < block_length * channels; sample++) {
                block[sample] += previousResult.front() / (float)fft_length;
                previousResult.pop_front();
            }
        }
        previousResults.remove_if(emptyResults);
    }


    OverlapAdd::~OverlapAdd() {
        pffft_destroy_setup(fft_setup);

        pffft_aligned_free(fftIn);
        pffft_aligned_free(fftOut);
        pffft_aligned_free(fftIr);

        if (fft_length > maxStackFFTSize) {
            pffft_aligned_free(fftWorkspace);
        }

    }

    ///////////////////////////////////////////////////////////////////////////
    // HRTF Class
    ///////////////////////////////////////////////////////////////////////////
    int HRTF::load() {
        hrtf = mysofa_open(path.c_str(), settings.sampleRate, &filter_length, &error);
        if (!hrtf) {
            DSPTB_ERROR("[HRTF]: Could not load file" << path << std::endl);
            return DSPTB_FAILURE;
        }
        return DSPTB_SUCCESS;
    }

    int HRTF::getFilters(float x, float y, float z, float *leftIR, float *rightIR, float *leftDelaySeconds, float *rightDelaySeconds)
    {
        mysofa_getfilter_float(hrtf, x, y, z, leftIR, rightIR, leftDelaySeconds, rightDelaySeconds);
        if (error != 0) return DSPTB_FAILURE;
        return DSPTB_SUCCESS;
    }

    int HRTF::setFiltersToOverlapAdd(OverlapAdd& overlapAdd, float x, float y, float z)
    {
        assert(filter_length == overlapAdd.ir_length);
        assert(overlapAdd.channels == 2);
        mysofa_getfilter_float(hrtf, x, y, z, leftIR, rightIR, delayLeft, delayRight);
        int leftDelaySamples = (int)std::roundf(std::floorf(*delayLeft * settings.sampleRate));
        int rightDelaySamples = (int)std::roundf(std::floorf(*delayRight * settings.sampleRate));
        
        // Reset FFT IR to 0 and copy new HRTF data over
        float* fftIr = overlapAdd.fftIr;
        int fftLength = (int)overlapAdd.fft_length;
        // Reset to zeroes
        for (int i = 0; i <= std::max(leftDelaySamples, rightDelaySamples) * 2; i++) {
            *(fftIr + i) = 0.0f;
        }
        for (int i = filter_length * 2; i < fftLength; i++) {
            *(fftIr + i) = 0.0f;
        }
        // Copy data from left and right filters to the interleaved FFT IR buffer
        int itMax = std::max(leftDelaySamples, rightDelaySamples) + filter_length;
        for (int i = leftDelaySamples, int j = rightDelaySamples; itMax < fftLength && (i < filter_length || j < filter_length); i++, j++) {
            *(fftIr + i * 2) = leftIR[i - leftDelaySamples];
            *(fftIr + (j * 2 + 1)) = rightIR[j - rightDelaySamples];
        }
        overlapAdd.tranformIR();
        return DSPTB_SUCCESS;
    }

    HRTF::~HRTF() {
        if (hrtf) {
            mysofa_close(hrtf);
        }
    }

}