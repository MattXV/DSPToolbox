#include <dsptb.h>
#include <iostream>
#include <sndfile.h>
#include <vector>
#include <cmath>
#include <algorithm>


std::vector<float> open_wav(const char* path) {
    SF_INFO info;
    SNDFILE* sndfile = sf_open(path, SFM_READ, &info);
    if (!sndfile) {
        std::cout << sf_strerror(sndfile) << std::endl;
        sf_close(sndfile);
        return std::vector<float>();
    }
    std::vector<float> data = std::vector<float>(info.frames * info.channels, float(0));
    sf_count_t read = sf_readf_float(sndfile, data.data(), info.frames);
    if (read != info.frames) {
        std::cout << sf_strerror(sndfile) << std::endl;
        sf_close(sndfile);
        return std::vector<float>();
    }
    sf_close(sndfile);
    return data;
}

void write_wav(const std::vector<float>& frames, const char* path, int channels, int samplerate) {
    SF_INFO info;
    info.samplerate = samplerate;
    info.channels = channels;
    info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_24 | SF_ENDIAN_FILE;

    SNDFILE* sndfile = sf_open(path, SFM_WRITE, &info);
    if (!sndfile) {
        std::cout << sf_strerror(sndfile) << std::endl;
        sf_close(sndfile);
        return;
    }
    sf_count_t written = sf_writef_float(sndfile, frames.data(), frames.size() / channels);
    if (written != frames.size() / channels) {
        std::cout << sf_strerror(sndfile) << std::endl;
        sf_close(sndfile);
        return;
    }
    sf_close(sndfile);
}


int main(int argc, const char* argv[]) {

    std::vector<float> testWav = open_wav("drums.wav");

    dsptbSETTINGS settings;
    settings.sampleRate = 48000;
    settings.mixChunkSize = 1024;
    settings.dspKernelSize = 2048;
    int ok = dsptbInit(&settings);
    if (!ok) {
        std::cout << dsptbGetError() << std::endl;
    }
    const int blockSize = 1024;
    const int channels = 2;
    const char* hrtfPath = "dtf las_nh4.sofa";
    int hrtfObject = dsptbLoadHRTF(hrtfPath);
    const int filterLength = dsptbGetHRTFFilterLength(hrtfObject);
    
    std::vector<float> silence = open_wav("silence.wav");
    std::vector<float> short_ir = open_wav("ir.wav");
    const int kernel_size = filterLength;
    const int blockProcessingObject = dsptbInitBlockProcessing(blockSize, kernel_size, 2);
    
    dsptbSetHRTFToBlockProcessing(hrtfObject, blockProcessingObject, 30.f, +30.0f, 2.0f);
    float hpf[2048];
    float lpf[2048];

    silence.erase(silence.begin() + kernel_size, silence.end());
    short_ir.erase(short_ir.begin() + kernel_size, short_ir.end());

//    dsptbDesignFilter(DSPTB_FILTER_TYPE::LPF, 1000, 0, silence.data());
  //  dsptbDesignFilter(DSPTB_FILTER_TYPE::HPF, 10000, 0, short_ir.data());

    //dsptbSetFIRtoBlockProcessing(blockProcessingObject, short_ir.data(), short_ir.data());
  
  
    int numBlocks = (int)floorf((float)testWav.size() / (float)(blockSize * channels));
    std::vector<float> out;
    float* block = new float[blockSize * channels];
    for (int i = 0; i < numBlocks; i++) {
        for (int j = 0; j < blockSize; j++) {       
            block[j * channels] = testWav[j * channels];
            block[j * channels + 1] = testWav[j * channels + 1];
        }
        dsptbProcessBlock(blockProcessingObject, block);
        out.insert(out.end(), block, block + blockSize * channels);
        testWav.erase(testWav.begin(), testWav.begin() + blockSize * channels);
    }
    for (int i = 0; i < blockSize; i++) {
        float leftSample = 0;
        float rightSample = 0;
        if (i * channels < testWav.size()) {
            leftSample = testWav[i * channels];
            rightSample = testWav[i * channels + 1];
        }
        block[i * channels] = leftSample;
        block[i * channels + 1] = rightSample;
        
    }
    dsptbProcessBlock(blockProcessingObject, block);
    out.insert(out.end(), block, block + blockSize * channels);
    delete[] block;

    float maxValue = std::abs(out[0]);
    //std::for_each(out.begin(), out.end(), [&](const float& value) { float x = std::abs(value); if (x > maxValue) maxValue = x; });
    //std::for_each(out.begin(), out.end(), [&](float& value) { value = value / maxValue; });



    write_wav(out, "test_write.wav", 2, 48000);

/*


    float testData[blockSize * channels];
    for (int i = 0; i < blockSize; i++)
        testData[i] = (float)i / blockSize;

    for (int i = 0; i < 2000; i++) {
        dsptbProcessBlock(&testData[0]);
        if (i == 1500) {
            __debugbreak();
        }
    }

    std::cout << "done" << std::endl;
    */
    dsptbQuit();
    return 0;
}