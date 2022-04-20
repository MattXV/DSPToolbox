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

    int dsptbSetFrequencyDependentIRs(const float* data, int length, int dsptb_erb_band) {
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

    int dsptbGetIR(const float** data, int* len) {
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

    int dsptbGeneratePoissonDiracSequence(int n_samples, float volume, const float** data, int* len) {
        if (!dsptb::dsptbInitOK) {
            DSPTB_ERROR("DSPTB not initialised correctly. Aborting dsptbGetIR.");
            return DSPTB_FAILURE;
        }

        dsptb::signal dirac_sequence = dsptb::poisson_dirac_sequence(static_cast<size_t>(n_samples), dsptb::settings.sampleRate, volume);
        float* unmanaged = new float[dirac_sequence.size()];
        std::memcpy(static_cast<void*>(unmanaged), dirac_sequence.data(), dirac_sequence.size() * sizeof(float));
        *data = unmanaged;
        *len = dirac_sequence.size();

        return DSPTB_SUCCESS;

    }


    int dsptbConvolveFilterBankToIRs(void) {
        if (!dsptb::dsptbInitOK) {
            DSPTB_ERROR("DSPTB not initialised correctly. Aborting dsptbSetFrequencyDependentIRs.");
            return DSPTB_FAILURE;
        }
        int ok = dsptb::filterBank->generateIR();
        return ok;
    }

}