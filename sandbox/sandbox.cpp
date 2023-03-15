#include <dsptb.h>
#include <iostream>


int main(int argc, const char* argv[]) {
    dsptbSETTINGS settings;
    settings.dspKernelSize = 1024;
    settings.sampleRate = 48000;
    settings.mixChunkSize = 1024;
    int err = dsptbInit(&settings);
    if (err == DSPTB_FAILURE) {
        std::cout << err;
    }

    


    dsptbQuit();
    return 0;
}