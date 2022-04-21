#include <iostream>
#include <sndfile.h>
#include <dsptb.h>
#include <string>
#include <vector>
#include <array>

void test_ir_generation(const std::string& path_to_histograms) {
    std::array<std::vector<float>, 6> hists;
    int fs;
    for (int i = 0; i < 6; i++) {
        std::string filename = path_to_histograms + "/" + "RT_IR_bin_" + std::to_string(i) + ".wav";
        SF_INFO wav_info;
        SNDFILE* wav = sf_open(filename.c_str(), SFM_READ, &wav_info);
        std::cout << wav_info.frames << std::endl;
        std::cout << wav_info.samplerate << std::endl;
        
        fs = wav_info.samplerate;
        sf_close(wav);
    }
    dsptbSETTINGS newSettings;
    newSettings.dspKernelSize = 4096;
    newSettings.mixChunkSize = 1024;
    newSettings.sampleRate = fs;
    dsptbInit(&newSettings);

}


int main(int argc, const char* argv[]) {
    dsptbSETTINGS settings;
    settings.dspKernelSize = 4096;
    settings.mixChunkSize = 1024;
    settings.sampleRate = 44100;

    if (dsptbInit(&settings) < 0) return 1; 

    const float* dirac;
    int len = settings.sampleRate;
    int ok = dsptbGeneratePoissonDiracSequence(len, 3 * 2.5 * 3, &dirac);

    std::cout << len  << ' ' << ok << std::endl;

    for (int i = 0; i < 10; i++)
        std::cout << dirac[len - i] << std::endl;
    
    SF_INFO info;
    info.channels = 1;
    info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_24 | SF_ENDIAN_LITTLE;
    info.samplerate = settings.sampleRate;
    SNDFILE* wav = sf_open("dirac.wav", SFM_WRITE, &info);
    std::cout << "Wrote " << sf_write_float(wav, dirac, len);
    std::cout << " " << sf_strerror(wav) << std::endl;
    sf_close(wav);
    delete[] dirac;
    
    test_ir_generation("test_hists");


    dsptbQuit();
    std::cout << "finished." << std::endl;
    return 0;
}