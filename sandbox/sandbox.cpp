#include <iostream>
#include <sndfile.h>
#include <dsptb.h>
#include <string>
#include <vector>
#include <array>
#include <sstream>

constexpr float VOLUME = 3.07f * 3.83f * 3.90f;

void write_wav(const std::string& filename, const std::vector<float>& data, const size_t& fs) {
    SF_INFO wav_info;
    wav_info.channels = 1;
    wav_info.frames = data.size();
    wav_info.samplerate = fs;
    wav_info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_24 | SF_ENDIAN_LITTLE;
    SNDFILE* wav = sf_open(filename.c_str(), SFM_WRITE, &wav_info);
    if (!wav) std::cerr << sf_strerror(wav) << std::endl;
    size_t wrote = sf_write_float(wav, data.data(), data.size());
    if (wrote == 0) std::cerr << sf_strerror(wav) << std::endl;
}





int main(int argc, const char* argv[]) {

    if (argc != 3) {
        std::cout << "Usage: sandbox V path_to_hists" << std::endl; 
        return 0;
    }

    float volume = std::stof(argv[1]);
    std::cout << "Generating Monoaural IR. Volume " << volume << std::endl;


    std::string filename;
    std::array<std::vector<float>, 6> hists;
    int fs;
    for (int i = 0; i < 6; i++) {
        std::stringstream sstream;
        sstream << argv[2] << "/RT_IR_bin_" << i << ".wav";
        filename = sstream.str();
  
        SF_INFO info;
        SNDFILE* wavfile = sf_open(filename.c_str(), SFM_READ, &info);
        if (!wavfile) { std::cout << "Could not read hists." << std::endl; break; }

        fs = info.samplerate;
        hists[i] = std::vector<float>(info.frames, float(0));
        sf_read_float(wavfile, hists[i].data(), info.frames);
        sf_close(wavfile);

    }
    dsptbSETTINGS settings;
    settings.dspKernelSize = 4096;
    settings.mixChunkSize = 1024;
    settings.sampleRate = fs;
    if (dsptbInit(&settings) < 0) return 1; 

    for (int i = 0; i < 6; i++) {
        if (dsptbSetEnergyHistograms(hists[i].data(), hists[i].size(), i) != DSPTB_SUCCESS) {
            std::cerr << dsptbGetError() << std::endl;
        }
    }

    if (dsptbGenerateMonouralIR(volume) != DSPTB_SUCCESS) {
        std::cerr << dsptbGetError() << std::endl;
    }
    
    const float* data;
    int len;
    if (dsptbGetMonoauralIR(&data, &len) != DSPTB_SUCCESS) {
        std::cerr << dsptbGetError() << std::endl;
    }
    std::vector<float> monoaural(data, data + len);

    
    for (int i = 0; i < 6; i++) {
        const float* data;
        int len;
        if (dsptbGetGeneratedDiracSequence(&data, &len, static_cast<DSPTB_ERB_BAND>(i)) != DSPTB_SUCCESS) {
            std::cerr << dsptbGetError() << std::endl;
        }
        filename = "processed/MA_bin_" + std::to_string(i) + ".wav";
        std::vector<float> part(data, data + len);
        write_wav(filename, part, fs);
    }

    write_wav("monoaural.wav", monoaural, fs);

    dsptbQuit();

    return 0;
}