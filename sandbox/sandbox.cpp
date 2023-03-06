#include <dsptb.h>
#include <iostream>
#include <sndfile.h>
#include <vector>
#include <cmath>

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

    std::vector<float> testWav = open_wav("test.wav");

    dsptbSETTINGS settings;
    settings.sampleRate = 48000;
    settings.mixChunkSize = 1024;
    settings.dspKernelSize = 4096;
    int ok = dsptbInit(&settings);
    if (!ok) {
        std::cout << dsptbGetError() << std::endl;
    }
    const int blockSize = 1024;
    const int channels = 2;
    ok = dsptbInitBlockProcessing(blockSize, channels);
    
    int numBlocks = (int)floorf((float)testWav.size() / (float)(blockSize * channels));
    std::vector<float> out;
    float* block = new float[blockSize * channels];
    for (int i = 0; i < numBlocks; i++) {
        for (int j = 0; j < blockSize; j++) {       
            block[j * channels] = testWav[j * channels];
            block[j * channels + 1] = testWav[j * channels + 1];
        }
        dsptbProcessBlock(block);
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
    dsptbProcessBlock(block);
    out.insert(out.end(), block, block + blockSize * channels);
    delete[] block;

    


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

    dsptbQuit();
    std::cout << "done" << std::endl;
    */
    return 0;
}