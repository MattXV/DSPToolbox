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
        overlap = ir.size() - 1;
        fft_length = roundUpToNextPowerOfTwo(static_cast<unsigned int>(8 * overlap));
        stepSize = static_cast<int>(fft_length) - overlap;

        leftInputBuffer.resize(fft_length, 0);
        rightInputBuffer.resize(fft_length, 0);
        inTap = 0;
        leftOutputBuffer.resize(stepSize, 0);
        rightOutputBuffer.resize(stepSize, 0);
        outTap = 0;
        fftIn = new float[fft_length];
        fftOut = new float[fft_length];
        fftIr = new float[fft_length];
        fftWorkspace = new float[fft_length];

        for (int i = 0; i < ir.size(); i++) {
            fftIr[i] = ir[i];
        }
        fft_setup = pffft_new_setup((int)fft_length, pffft_transform_t::PFFFT_REAL);

        pffft_transform(fft_setup, fftIr, fftIr, fftWorkspace, pffft_direction_t::PFFFT_FORWARD);

        DSPTB_LOG("[OverlapSave]: overlap len: " << overlap << ", input/out buffer len: " << leftInputBuffer.size() << "," << leftOutputBuffer.size() <<
            "fft len: " << fft_length << "fft setup" << fft_setup << std::endl
        );

    }
    
    void OverlapSave::processBlock(float* block) {

        for (int sample = 0; sample < block_length; sample++) {

            leftInputBuffer[inTap] = block[sample * channels];
            rightInputBuffer[inTap] = block[sample * channels + 1];
            inTap = (inTap + 1) % leftInputBuffer.size();

            block[sample * channels] = leftOutputBuffer[outTap];
            block[sample * channels + 1] = rightOutputBuffer[outTap];

            if (++outTap == leftOutputBuffer.size()) {
                performConvolution();
                outTap = 0;
            }
        }
    }

    void OverlapSave::performConvolution()
    {
        for (int i = 0, readPos = inTap; i < fft_length; ++i, readPos = (readPos + 1) % leftInputBuffer.size()) {
            const float& sample = leftInputBuffer[readPos];
            fftIn[i] = std::isnan(sample) ? 0.0f : clip(sample, -1.0f, 1.0f);
            fftOut[i] = 0;
        }

        pffft_transform(fft_setup, fftIn, fftOut, fftWorkspace, pffft_direction_t::PFFFT_FORWARD);
        if (once) {
            std::stringstream ss, ss1;
            for (int i = 0; i < fft_length; i++) {
                ss << fftIr[i] << ',';
                ss1 << fftOut[i] << ',';
            }
            DSPTB_LOG("forward fftin: " << ss.str() << std::endl);
            DSPTB_LOG("forward fftout: " << ss1.str() << std::endl);
            DSPTB_LOG("fft setup" << fft_setup << std::endl);
        }
        pffft_zconvolve_accumulate(fft_setup, fftOut, fftIr, fftOut, 1.0f / (float)fft_length);
        if (once) {
            std::stringstream ss, ss1;
            for (int i = 0; i < fft_length; i++) {
                ss << fftIr[i] << ',';
                ss1 << fftOut[i] << ',';
            }
            DSPTB_LOG("z acc fftin: " << ss.str() << std::endl);
            DSPTB_LOG("z acc fftout: " << ss1.str() << std::endl);
        }
        pffft_transform(fft_setup, fftOut, fftOut, fftWorkspace, pffft_direction_t::PFFFT_BACKWARD);
        if (once) {
            std::stringstream ss, ss1;
            for (int i = 0; i < fft_length; i++) {
                ss1 << fftOut[i] << ',';
            }
            DSPTB_LOG("backward fftout: " << ss1.str() << std::endl);
            once = false;
        }


        for (int i = 0; i < leftOutputBuffer.size(); i++) {
            leftOutputBuffer[i] = fftOut[i + overlap] * 0.0001f;
            rightOutputBuffer[i] = fftOut[i + overlap] * 0.0001f;
        }
    }

    OverlapSave::~OverlapSave() {
        pffft_destroy_setup(fft_setup);
        delete[] fftWorkspace;
        delete[] fftIr;
        delete[] fftOut;
        delete[] fftIn;
    }

}