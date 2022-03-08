#include "dsptb.hpp"

namespace dsptb {
    std::string dsptb_error = std::string();
    std::stringstream dsptb_logs = std::stringstream();
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

    int dsptbSetFrequencyDependentIRs(float* data, int length, int dsptb_erb_band) {
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