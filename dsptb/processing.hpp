#ifndef DSPTB_PROCESSING
#define DSPTB_PROCESSING
#include "filter.hpp"

namespace dsptb {
    class AudioBuffer { 
        public:
            AudioBuffer() {}
            AudioBuffer(size_t _channels, size_t _block_length) {
                setSize(_channels, _block_length);
            } 
            void setSize(size_t _channels, size_t _block_length) {
                channels = _channels;
                block_length = _block_length;
                buffers.resize(channels);
                for (auto& buffer : buffers) buffer.resize(block_length);
            }
            std::vector<float>& getBuffer(size_t channel) { return buffers[channel]; }
            const std::vector<float>& getBuffer(size_t channel) const { return buffers[channel]; }
            size_t getBlockLength() const { return block_length; }
        private:
            size_t channels = 0;
            size_t block_length = 0;
            std::vector<std::vector<float>> buffers;
            DISABLE_COPY_ASSIGN(AudioBuffer)
    };

    class OverlapSave {
        public:
            OverlapSave(int block_length, int channels, std::vector<float>&& ir);
            
            ~OverlapSave();
            
            void processBlock(float* block);
        private:
            void prepare();
            void performConvolution();

            PFFFT_Setup* fft_setup = nullptr;
            float* fftIn;
            float* fftOut;
            float* fftIr;
            float* fftWorkspace;

            std::vector<float> ir;
            std::vector<float> leftInputBuffer, rightInputBuffer;
            std::vector<float> leftOutputBuffer, rightOutputBuffer;
            int inTap = 0, outTap = 0;
            int overlap = 0, stepSize = 0;
            bool once = true;
            size_t channels, block_length, fft_length;
            DISABLE_COPY_ASSIGN(OverlapSave)
    };
}

#endif