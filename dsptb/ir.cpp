#include "ir.hpp"

namespace dsptb {
    FilterBank::FilterBank() :
        filter_125(Filter::filterType::HPF, 25.0f),
        filter_250(Filter::filterType::BPF, 25.0f, 250.0f),
        filter_500(Filter::filterType::BPF, 250.0f, 500.0f),
        filter_1000(Filter::filterType::BPF, 500.0f, 1000.0f),
        filter_2000(Filter::filterType::BPF, 1000.0f, 2000.0f),
        filter_4000(Filter::filterType::LPF, 20000.0f)
    { }
    int FilterBank::generateIR() {

        size_t ir_size = frequencyDependentIRs[F_125].size();
        if (checkIRs() != DSPTB_SUCCESS) return DSPTB_FAILURE;

        filter_125.convolveToSignal(frequencyDependentIRs[F_125]);
        filter_250.convolveToSignal(frequencyDependentIRs[F_250]);
        filter_500.convolveToSignal(frequencyDependentIRs[F_500]);
        filter_1000.convolveToSignal(frequencyDependentIRs[F_1000]);
        filter_2000.convolveToSignal(frequencyDependentIRs[F_2000]);
        filter_4000.convolveToSignal(frequencyDependentIRs[F_4000]);

        ir.resize(ir_size);
        for (size_t i = 0; i < ir_size; i++) {
            ir[i] = 
                frequencyDependentIRs[F_125 ][i] +
                frequencyDependentIRs[F_250 ][i] +
                frequencyDependentIRs[F_500 ][i] +
                frequencyDependentIRs[F_1000][i] +
                frequencyDependentIRs[F_2000][i] +
                frequencyDependentIRs[F_4000][i];
        }
        normalise(ir);
        DSPTB_LOG("Generated IR of len " << ir.size());
        return DSPTB_SUCCESS;
    }

    int FilterBank::checkIRs() {
        size_t ir_size = frequencyDependentIRs[F_125].size();
        if (ir_size == 0 || 
            (frequencyDependentIRs[F_125].size()  != ir_size) ||
            (frequencyDependentIRs[F_250].size()  != ir_size) ||
            (frequencyDependentIRs[F_500].size()  != ir_size) ||
            (frequencyDependentIRs[F_1000].size() != ir_size) ||
            (frequencyDependentIRs[F_2000].size() != ir_size) ||
            (frequencyDependentIRs[F_4000].size() != ir_size)) {
            DSPTB_ERROR("Invalid dsptb_erb_band parameter! Aborting FilterBank::generateIR.");
            return DSPTB_FAILURE;
        }
        return DSPTB_SUCCESS;
    }

}