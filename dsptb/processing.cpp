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
        
        fft_length = roundUpToNextPowerOfTwo((unsigned int)(ir_length + block_length - 1));

        if (fft_length <= maxStackFFTSize) {
            fftWorkspace = nullptr;          
        } 
        else {
            fftWorkspace = (float*)pffft_aligned_malloc(fft_length * sizeof(float));
        }
        fftInL  = (float*)pffft_aligned_malloc(fft_length * sizeof(float));
        fftInR  = (float*)pffft_aligned_malloc(fft_length * sizeof(float));
        fftOutL = (float*)pffft_aligned_malloc(fft_length * sizeof(float));
        fftOutR = (float*)pffft_aligned_malloc(fft_length * sizeof(float));
        fftIrL  = (float*)pffft_aligned_malloc(fft_length * sizeof(float));
        fftIrR  = (float*)pffft_aligned_malloc(fft_length * sizeof(float));


        for (int i = 0; i < fft_length; i++) {
            fftIrL[i] = 0;
            fftIrR[i] = 0;
        }
        fftIrL[1] = 1.0f;
        fftIrR[1] = 1.0f;

        fft_setup = pffft_new_setup((int)fft_length, pffft_transform_t::PFFFT_REAL);
        tranformIR();
        DSPTB_LOG("FFT size: " << fft_length << std::endl);
    }
    
    void OverlapAdd::tranformIR() {
        pffft_transform(fft_setup, fftIrL, fftIrL, fftWorkspace, pffft_direction_t::PFFFT_FORWARD);
        pffft_transform(fft_setup, fftIrR, fftIrR, fftWorkspace, pffft_direction_t::PFFFT_FORWARD);
    }
    void OverlapAdd::setIRData(float* leftIr, float* rightIR) {
        assert(channels == 2);
        for (int i = 0; i < fft_length; i++) {
            fftIrL[i] = 0.0f;
            fftIrR[i] = 0.0f;
        }
        for (int i = 0; i < ir_length; i++) {
            fftIrL[i] = leftIr[i];
            fftIrR[i] = rightIR[i];
        }
        tranformIR();
    }

    void OverlapAdd::processBlock(float* block) {
        for (int i = 0; i < block_length; i++) {
            fftInL[i] = block[i * channels];
            fftInR[i] = block[i * channels + 1];
            block[i * channels] = 0;
            block[i * channels + 1] = 0;
            fftOutL[i] = 0; 
            fftOutR[i] = 0; 
        }
        for (int i = block_length; i < fft_length; i++) {
            fftInL[i] = 0;
            fftInR[i] = 0;
            fftOutL[i] = 0;
            fftOutR[i] = 0;
        }
        pffft_transform(fft_setup, fftInL, fftInL, fftWorkspace, pffft_direction_t::PFFFT_FORWARD);
        pffft_transform(fft_setup, fftInR, fftInR, fftWorkspace, pffft_direction_t::PFFFT_FORWARD);
        pffft_zconvolve_accumulate(fft_setup, fftInL, fftIrL, fftOutL, 1.0f);
        pffft_zconvolve_accumulate(fft_setup, fftInR, fftIrR, fftOutR, 1.0f);
        pffft_transform(fft_setup, fftOutL, fftOutL, fftWorkspace, pffft_direction_t::PFFFT_BACKWARD);
        pffft_transform(fft_setup, fftOutR, fftOutR, fftWorkspace, pffft_direction_t::PFFFT_BACKWARD);
        previousResultsL.emplace_back(fftOutL, fftOutL + fft_length);
        previousResultsR.emplace_back(fftOutR, fftOutR + fft_length);

        for (std::deque<float>& previousResult : previousResultsL) {
            for (int sample = 0; !previousResult.empty() && sample < block_length; sample++) {
                block[sample* channels] += previousResult.front() / (float)fft_length;
                previousResult.pop_front();
            }
        }
        for (std::deque<float>& previousResult : previousResultsR) {
            for (int sample = 0; !previousResult.empty() && sample < block_length; sample++) {
                block[sample * channels + 1] += previousResult.front() / (float)fft_length;
                previousResult.pop_front();
            }
        }
        previousResultsL.remove_if(emptyResults);
        previousResultsR.remove_if(emptyResults);
    }


    OverlapAdd::~OverlapAdd() {
        pffft_destroy_setup(fft_setup);

        pffft_aligned_free(fftInL);
        pffft_aligned_free(fftInR);
        pffft_aligned_free(fftOutL);
        pffft_aligned_free(fftOutR);
        pffft_aligned_free(fftIrL);
        pffft_aligned_free(fftIrR);

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
        if (filter_length <= maxStackSize) {
            leftIR = &leftStackBuffer[0];
            rightIR = &rightStackBuffer[0];
        } else {
            leftIR = new float[filter_length];
            rightIR = new float[filter_length];
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
        mysofa_getfilter_float(hrtf, x, y, z, leftIR, rightIR, &delayLeft, &delayRight);
        int leftDelaySamples = (int)std::roundf(std::floorf(delayLeft * settings.sampleRate));
        int rightDelaySamples = (int)std::roundf(std::floorf(delayRight * settings.sampleRate));
        
        float* leftFFTIR = overlapAdd.fftIrL;
        for (int i = 0; i < overlapAdd.fft_length; i++) { ;leftFFTIR [i] = 0; }
        for (int i = leftDelaySamples; i < filter_length; i++) {
            leftFFTIR[i] = leftIR[i];
        }

        float* rightFFTIR = overlapAdd.fftIrR;
        for (int i = 0; i < overlapAdd.fft_length; i++) { rightFFTIR[i] = 0; }
        for (int i = rightDelaySamples; i < filter_length; i++) {
            rightFFTIR[i] = rightIR[i];
        }

        //// Reset FFT IR to 0 and copy new HRTF data over
        //float* fftIr = overlapAdd.fftIrL;
        //int fftLength = (int)overlapAdd.fft_length;
        //// Reset to zeroes
        //for (int i = 0; i <= std::max(leftDelaySamples, rightDelaySamples) * 2; i++) {
        //    *(fftIr + i) = 0.0f;
        //}
        //for (int i = filter_length * 2; i < fftLength; i++) {
        //    *(fftIr + i) = 0.0f;
        //}
        //// Copy data from left and right filters to the interleaved FFT IR buffer
        //int itMax = std::max(leftDelaySamples, rightDelaySamples) + filter_length;
        //for (int i = leftDelaySamples, j = rightDelaySamples; itMax < fftLength && (i < filter_length || j < filter_length); i++, j++) {
        //    *(fftIr + i * 2) = leftIR[i - leftDelaySamples];
        //    *(fftIr + (j * 2 + 1)) = rightIR[j - rightDelaySamples];
        //}
        overlapAdd.tranformIR();
        return DSPTB_SUCCESS;
    }

    HRTF::~HRTF() {
        if (filter_length > maxStackSize) {
            delete[] leftIR;
            delete[] rightIR;
        }
        if (hrtf) {
            mysofa_close(hrtf);
        }
    }

}