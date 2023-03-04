#include <dsptb.h>
#include <iostream>

int main(int argc, const char* argv[]) {

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

    float testData[blockSize * channels];
    for (int i = 0; i < blockSize; i++)
        testData[i] = (float)i / blockSize;

    for (int i = 0; i < 2000; i++) {
        dsptbProcessBlock(&testData[0]);

    }

    dsptbQuit();
    std::cout << "done" << std::endl;
    return 0;
}