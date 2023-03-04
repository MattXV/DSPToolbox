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

EXPORT const char* dsptbGetError(void);
EXPORT const char* dsptbGetLogs(void);
EXPORT void dsptbClearLogs(void);
EXPORT void dsptbClearError(void);

/**
 * Set Impulse Response for a specific ERB frequency @param dsptb_erb_band.
 * All 6 bands must be set before convolution operations, and must have equal length.
 * @param data is a pointer to a float array of a specified @param length.
**/ 
EXPORT int dsptbSetEnergyHistograms(const float* data, int length, int dsptb_erb_band);

/**
 * Convolve filter bank to all frequency-dependent Impulse Responses.
 * All ERB bands must have a signal of the same length.
 * @param volume indicates the volume of the environment for the 
 *  Poisson-distributed Dirac Delta sequences (Schroder, 2011) 
**/ 
EXPORT int dsptbGenerateMonouralIR(float volume);

/**
 * Obtain a pointer to the head of the IR array, @param data, and its length,
 * @param len.
 * Successful only if all 6 bands are set and the filterbank is applied.
**/
EXPORT int dsptbGetMonoauralIR(const float** data, int* len);

/**
 * FFT convolution to apply @param kernel of length @param lenKernel to @param a signal of length @param lenA.
 * The convolution results exists until dsptbQuit is called. 
**/
EXPORT int dsptbFFTConvolve(const float* a, int lenA, const float* kernel, int lenKernel, const float** outConvolution, int* outLenConvolution);

/**
 * Testing: Generates and returns a Poisson-distributed Dirac Delta sequence
 * Generates an unmanaged raw array.
**/
EXPORT int dsptbGeneratePoissonDiracSequence(int n_samples, float volume, const float** data);



EXPORT int dsptbInitBlockProcessing(int blockLength, int channels);
/**
 * Overlap save
*/
EXPORT int dsptbProcessBlock(float* data);




EXPORT int dsptbGetGeneratedDiracSequence(const float** data, int* len, DSPTB_ERB_BAND band);

EXPORT int dsptbGetIRComponent(const float** data, int* len, DSPTB_ERB_BAND band);

#ifdef __cplusplus
}      // extern "C"
#endif 
#endif // DSPTB_H