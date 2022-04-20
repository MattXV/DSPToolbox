#include <iostream>
#include <sndfile.h>
#include <dsptb.h>


int main(int argc, const char* argv[]) {
    dsptbSETTINGS settings;
    settings.dspKernelSize = 4096;
    settings.mixChunkSize = 1024;
    settings.sampleRate = 44100;

    if (dsptbInit(&settings) < 0) return 1; 

    const float* dirac;
    int len;
    int ok = dsptbGeneratePoissonDiracSequence(settings.sampleRate, 3 * 2.5 * 3, &dirac, &len);

    std::cout << len  << ' ' << ok << std::endl;

    for (int i = 0; i < 10; i++)
        std::cout << dirac[len - i] << std::endl;
    
    SF_INFO info;
    info.channels = 1;
    info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_24 | SF_ENDIAN_LITTLE;
    info.samplerate = settings.sampleRate;
    SNDFILE* wav = sf_open("dirac.wav", SFM_WRITE, &info);
    std::cout << sf_strerror(wav) << std::endl;
    std::cout << "Wrote " << sf_write_float(wav, dirac, len) << std::endl;
    std::cout << sf_strerror(wav) << std::endl;
    sf_close(wav);
    

    delete[] dirac;

    dsptbQuit();
    std::cout << "finished." << std::endl;
    return 0;
}