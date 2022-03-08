#ifndef DSPTB_H
#define DSPTB_H


#if defined(_MSC_VER)
    //  Microsoft 
    #define EXPORT __declspec(dllexport)
    #define IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
    //  GCC
    #define EXPORT __attribute__((visibility("default")))
    #define IMPORT
#else
    //  do nothing and hope for the best?
    #define EXPORT
    #define IMPORT
    #pragma warning Unknown dynamic link import/export semantics.
#endif

#define DSPTB_SUCCESS 0
#define DSPTB_FAILURE -1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int sampleRate;
    int mixChunkSize;
    int dspKernelSize;
} dsptbSETTINGS;

enum DSPTB_ERB_BAND {
    F_125, F_250, F_500, F_1000, F_2000, F_4000
};

EXPORT int dsptbInit(dsptbSETTINGS* settings);
EXPORT void dsptbQuit();


/**
 * Set Impulse Response for a specific ERB frequency @param dsptb_erb_band.
 * All bands must be set before convolution operations.
 * @param data is a pointer to a float array of a specified @param length.
**/ 
EXPORT int dsptbSetFrequencyDependentIRs(float* data, int length, int dsptb_erb_band);

/**
 * Convolve filter bank to all frequency-dependent Impulse Responses.
 * All ERB bands must have a signal of the same length.
**/ 
EXPORT int dsptbConvolveFilterBankToIRs(void);


#ifdef __cplusplus
}      // extern "C"
#endif 
#endif // DSPTB_H