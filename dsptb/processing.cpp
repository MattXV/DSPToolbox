#include "processing.hpp"

namespace dsptb {
    OverlapSave::OverlapSave(int _block_length, int _channels, std::vector<float>&& _ir)
        : ir(_ir), channels(_channels), block_length(_block_length)
    {
        prepare();
    }
    void OverlapSave::prepare() {
        // ir = std::vector<float>(1024, float(0));
        // ir[1] = 1.0f;
        
        fft_length = roundUpToNextPowerOfTwo((unsigned int)(ir.size() * channels + block_length * channels - 1));

        fftIn = new float[fft_length];
        fftOut = new float[fft_length];
        fftIr = new float[fft_length];
        fftWorkspace = new float[fft_length];

        for (int i = 0; i < ir.size(); i++) {
            const float& value = ir[i];
            fftIr[i * channels] = value;
            fftIr[i * channels + 1] = value;
        }
        for (int i = ir.size(); i < fft_length; i++) {
            fftIr[i] = 0;
        }
        fft_setup = pffft_new_setup((int)fft_length, pffft_transform_t::PFFFT_REAL);
        pffft_transform(fft_setup, fftIr, fftIr, fftWorkspace, pffft_direction_t::PFFFT_FORWARD);
        DSPTB_LOG("FFT size: " << fft_length << std::endl);
    }
    
    void OverlapSave::processBlock(float* block) {
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


    OverlapSave::~OverlapSave() {
        pffft_destroy_setup(fft_setup);
        delete[] fftWorkspace;
        delete[] fftIr;
        delete[] fftOut;
        delete[] fftIn;

    }

}