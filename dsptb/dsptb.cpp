#include "dsptb.hpp"

namespace dsptb {
    std::string dsptb_error = std::string();
    std::stringstream dsptb_logs = std::stringstream();
    std::string currentLogs = std::string();
    dsptbSETTINGS settings;
}

void DSPTB_SetError(const std::string& error_string) {
    dsptb::dsptb_error = error_string;
}
void DSPTB_Log(const std::string& log) {
    dsptb::dsptb_logs << log;
}

extern "C" {
    int dsptbInit(dsptbSETTINGS* _settings) {
        if (!_settings) {
            DSPTB_ERROR("Invalid dsptbSettings pointer. Initilialisation failure.");
            return DSPTB_FAILURE;
        }
        int sr = _settings->sampleRate;
        int mcs = _settings->mixChunkSize;
        int ks = _settings->dspKernelSize;

        if (sr < 0 || sr > 192000) {
            DSPTB_ERROR("Invalid sample rate. Initilialisation failure.");
            return DSPTB_FAILURE;
        }
        if (mcs < 0 || mcs > 32768) {
            DSPTB_ERROR("Invalid mix chunk size. Initilialisation failure.");
            return DSPTB_FAILURE;
        }
        if (ks < 0 || ks > 32768) {
            DSPTB_ERROR("Invalid dsp kernel size. Initilialisation failure.");
            return DSPTB_FAILURE;
        }

        dsptb::settings.sampleRate = sr;
        dsptb::settings.mixChunkSize = mcs;
        dsptb::settings.dspKernelSize = ks;

        dsptb::filterBank = std::make_unique<dsptb::FilterBank>();
        dsptb::dsptbInitOK = true;
        return DSPTB_SUCCESS;
    }
    void dsptbQuit() {
        dsptb::outConvolution.clear();
        dsptb::filterBank.reset();
        dsptb::blockProcessingObjects.clear();
    }

    const char* dsptbGetError(void) {
        return dsptb::dsptb_error.c_str();
    }
    void dsptbClearLogs(void) {
        dsptb::dsptb_logs.clear();
        dsptb::currentLogs.clear();
    }
    void dsptbClearError(void) {
        dsptb::dsptb_error.clear();
    }


    const char* dsptbGetLogs(void) {
        dsptb::currentLogs = dsptb::dsptb_logs.str();
        return dsptb::currentLogs.c_str();
    }

    int dsptbSetEnergyHistograms(const float* data, int length, int dsptb_erb_band) {
        if (!dsptb::dsptbInitOK) {
            DSPTB_ERROR("DSPTB not initialised correctly. Aborting dsptbSetFrequencyDependentIRs.");
            return DSPTB_FAILURE;
        }
        if (!data) { 
            DSPTB_ERROR("Invalid data pointer! Aborting dsptbSetFrequencyDependentIRs.");
            return DSPTB_FAILURE;
        }
        if (length < 0) {
            DSPTB_ERROR("Invalid length parameter! Aborting dsptbSetFrequencyDependentIRs.");
            return DSPTB_FAILURE;
        }
        if (dsptb_erb_band < 0 || dsptb_erb_band > static_cast<int>(F_4000)) {
            DSPTB_ERROR("Invalid dsptb_erb_band parameter! Aborting dsptbSetFrequencyDependentIRs.");
            return DSPTB_FAILURE;
        }

        dsptb::signal ir = dsptb::signal(data, data + length);
        dsptb::filterBank->setFrequencyDependentIRs(ir, static_cast<DSPTB_ERB_BAND>(dsptb_erb_band));
        DSPTB_LOG("Set Frequency-dependent ir. ERB band: " << dsptb_erb_band << " length: " << length << " \n");
        return DSPTB_SUCCESS;
    }

    int dsptbGetGeneratedDiracSequence(const float** data, int* len, DSPTB_ERB_BAND band) {
        if (!dsptb::dsptbInitOK) {
            DSPTB_ERROR("DSPTB not initialised correctly. Aborting dsptbGetIR.");
            return DSPTB_FAILURE;
        }
        const dsptb::signal& ir = dsptb::filterBank->getIR();
        if (ir.size() == 0) {
            DSPTB_ERROR("IR not generated. Aborting dsptbGetIR.");
            return DSPTB_FAILURE;
        }

        const dsptb::signal& sequence = dsptb::filterBank->getDiracSequence(band);
        *data = sequence.data();
        *len = static_cast<int>(sequence.size());
        return DSPTB_SUCCESS;
    }

    int dsptbGetIRComponent(const float** data, int* len, DSPTB_ERB_BAND band) {
        if (!dsptb::dsptbInitOK) {
            DSPTB_ERROR("DSPTB not initialised correctly. Aborting dsptbGetIRComponent.");
            return DSPTB_FAILURE;
        }
        const dsptb::signal& ir = dsptb::filterBank->getIR();
        if (ir.size() == 0) {
            DSPTB_ERROR("IR not generated. Aborting dsptbGetIRComponent.");
            return DSPTB_FAILURE;
        }

        const dsptb::signal& component = dsptb::filterBank->getFrequencyDependentIRs(band);
        *data = component.data();
        *len = component.size();
        return DSPTB_SUCCESS;
    }

    int dsptbGetMonoauralIR(const float** data, int* len) {
        if (!dsptb::dsptbInitOK) {
            DSPTB_ERROR("DSPTB not initialised correctly. Aborting dsptbGetIR.");
            return DSPTB_FAILURE;
        }
        const dsptb::signal& ir = dsptb::filterBank->getIR();
        if (ir.size() == 0) {
            DSPTB_ERROR("IR not generated. Aborting dsptbGetIR.");
            return DSPTB_FAILURE;
        }

        *data = ir.data();
        *len = ir.size();
        return DSPTB_SUCCESS;
    }

    int dsptbGeneratePoissonDiracSequence(int n_samples, float volume, const float** data) {
        if (!dsptb::dsptbInitOK) {
            DSPTB_ERROR("DSPTB not initialised correctly. Aborting dsptbGeneratePoissonDiracSequence.");
            return DSPTB_FAILURE;
        }
        size_t length = static_cast<size_t>(n_samples);
        if (length < 1) {
            DSPTB_ERROR("Invalid n_samples parameter. Aborting dsptbGeneratePoissonDiracSequence");
            return DSPTB_FAILURE;
        }
        
        float* unmanaged = new float[n_samples];
        dsptb::signal temp(n_samples, 0);
        dsptb::poisson_dirac_sequence(temp, length, dsptb::settings.sampleRate, volume);
        memcpy(unmanaged, temp.data(), sizeof(float) * n_samples);
        *data = unmanaged;
        return DSPTB_SUCCESS;
    }


    int dsptbGenerateMonouralIR(float volume) {
        if (!dsptb::dsptbInitOK) {
            DSPTB_ERROR("DSPTB not initialised correctly. Aborting dsptbSetFrequencyDependentIRs.");
            return DSPTB_FAILURE;
        }
        int ok = dsptb::filterBank->generateIR(volume);
        return ok;
    }

    int dsptbFFTConvolve(const float* a, int lenA, const float* kernel, int lenKernel, const float** outConvolution, int* outLenConvolution) {
        if (!dsptb::dsptbInitOK) {
            DSPTB_ERROR("DSPTB not initialised correctly. Aborting dsptbFFTConvolve.");
            return DSPTB_FAILURE;
        }
        if (!a) {
            DSPTB_ERROR("Invalid pointer: a. Aborting dsptbFFTConvolve.");
            return DSPTB_FAILURE;
        }
        if (!kernel) {
            DSPTB_ERROR("Invalid pointer: kernel. Aborting dsptbFFTConvolve.");
            return DSPTB_FAILURE;
        }
        dsptb::signal _signal = dsptb::signal(a, a + lenA);
        dsptb::signal _kernel = dsptb::signal(kernel, kernel + lenKernel);

        dsptb::outConvolution.clear();
        dsptb::outConvolution = dsptb::fft_convolve(_signal, _kernel);
        *outConvolution = dsptb::outConvolution.data();
        *outLenConvolution = dsptb::outConvolution.size();
        return DSPTB_SUCCESS;
    }


    int dsptbInitBlockProcessing(int blockLength, int irLength, int channels) {
        if (!dsptb::dsptbInitOK) {
            DSPTB_ERROR("DSPTB not initialised correctly. Aborting dsptbInitBlockProcessing.");
            return DSPTB_FAILURE;
        }
        if (blockLength <= 0) {
            DSPTB_ERROR("Invalid block length. Aborting dsptbInitBlockProcessing.");
            return DSPTB_FAILURE; 
        }
        if (channels != 2) {
            DSPTB_ERROR("Unsupported channels. Aborting dsptbInitBlockProcessing.");
            return DSPTB_FAILURE; 
        }

        dsptb::blockProcessingObjects.emplace_back(new dsptb::OverlapAdd(blockLength, irLength, channels));
        
        return dsptb::blockProcessingObjects.size() - 1;
    }

    int dsptbProcessBlock(int blockProcessingObject, float* data) {
        dsptb::blockProcessingObjects[blockProcessingObject]->processBlock(data);
        return DSPTB_SUCCESS;
    }

    int dsptbLoadHRTF(const char* path) {
        if (!dsptb::dsptbInitOK) {
            DSPTB_ERROR("DSPTB not initialised correctly. Aborting dsptbLoadHRTF.");
            return DSPTB_FAILURE;
        }

        std::unique_ptr<dsptb::HRTF> newHrtf = std::unique_ptr<dsptb::HRTF>(new dsptb::HRTF(path));
        if (newHrtf->load() == DSPTB_FAILURE) {
            return DSPTB_FAILURE;
        }
        dsptb::hrtfObjects.push_back(newHrtf);
        return dsptb::hrtfObjects.size() - 1;
    }

    int dsptbSetHRTFToBlockProcessing(int hrtfObject, int blockProcessingObject, float x, float y, float z) {
        dsptb::hrtfObjects[hrtfObject]->setFiltersToOverlapAdd(*dsptb::blockProcessingObjects[blockProcessingObject], x, y, z);
    }

} 