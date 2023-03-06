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
        
        fft_length = roundUpToNextPowerOfTwo((unsigned int)(ir.size() + block_length - 1));

        fftIn = new float[fft_length];
        fftOut = new float[fft_length];
        fftIr = new float[fft_length];
        fftWorkspace = new float[fft_length];

        for (int i = 0; i < ir.size(); i++) {
            fftIr[i] = ir[i];
        }
        for (int i = ir.size(); i < fft_length; i++) {
            fftIr[i] = 0;
        }
        fft_setup = pffft_new_setup((int)fft_length, pffft_transform_t::PFFFT_REAL);
        pffft_transform(fft_setup, fftIr, fftIr, fftWorkspace, pffft_direction_t::PFFFT_FORWARD);

    }
    
    void OverlapSave::processBlock(float* block) {
        for (int i = 0; i < block_length; i++) {
            
            fftIn[i] = clip(block[i * channels], -1.0f, 1.0f);
            block[i * channels] = 0;
            block[i * channels + 1] = 0;
            fftOut[i] = 0;
        }
        for (int i = block_length; i < fft_length; i++) {
            fftIn[i] = 0;
            fftOut[i] = 0;
        }
        pffft_transform(fft_setup, fftIn, fftIn, fftWorkspace, pffft_direction_t::PFFFT_FORWARD);
        pffft_zconvolve_accumulate(fft_setup, fftIn, fftIr, fftOut, 1.0f);
        pffft_transform(fft_setup, fftOut, fftOut, fftWorkspace, pffft_direction_t::PFFFT_BACKWARD);

        std::vector<float> newResult = std::vector<float>(fftOut, fftOut + fft_length);
        previousResults.push_back(newResult);

        for (std::vector<float>& previousResult : previousResults) {
            for (int sample = 0; sample < block_length; sample++) {
                if (previousResult.empty()) {
                    break;
                }
                float resultValue = previousResult.front() / (float)fft_length;
                previousResult.erase(previousResult.begin(), previousResult.begin() + 1);
                block[sample * channels] += resultValue;
                block[sample * channels + 1] += resultValue;
            }
        }
        previousResults.remove_if([](const std::vector<float>& result) { return result.empty(); });
    }


    OverlapSave::~OverlapSave() {
        pffft_destroy_setup(fft_setup);
        delete[] fftWorkspace;
        delete[] fftIr;
        delete[] fftOut;
        delete[] fftIn;
    }

}