#include "filter.hpp"

namespace dsptb
{
    signal fft_convolve(const signal& input, const signal& kernel) {
        size_t nConv = input.size() + kernel.size() - 1; // Convolved Length = len_input + len_kernel - 1
        size_t N = (size_t)roundUpToNextPowerOfTwo((unsigned int)nConv); // PFFFT wants powers of two for efficiency
        PFFFT_Setup* fftSetup = pffft_new_setup((int)N, pffft_transform_t::PFFFT_REAL);
        float* paddedSignal = new float[N];
        float* paddedKernel = new float[N];
        float* workSpace    = new float[N];
        float* convolution  = new float[N];
        for (size_t i = 0; i < N; i++) {
            // Zero padding inputs
            paddedKernel[i] = i < kernel.size() ? kernel[i] : 0.0f;
            paddedSignal[i] = i < input.size() ? input[i] : 0.0f;
            convolution[i] = 0;
        }	
        pffft_transform(fftSetup, paddedSignal, paddedSignal, workSpace, pffft_direction_t::PFFFT_FORWARD);
        pffft_transform(fftSetup, paddedKernel, paddedKernel, workSpace, pffft_direction_t::PFFFT_FORWARD);
        // Always use zconvolve_accumulate to multiply spectrums together as PFFFT uses some satanic frequency ordering.
        pffft_zconvolve_accumulate(fftSetup, paddedSignal, paddedKernel, convolution, 1.0);
        pffft_transform(fftSetup, convolution, convolution, workSpace, pffft_direction_t::PFFFT_BACKWARD);
        signal out = signal(nConv, 0);
        for (size_t i = 0; i < nConv; i++)
            out[i] = (sample)((float)convolution[i] / (float)N); // Rescaling as PFFFT_BACKWARD(PFFFT_FORWARD(x)) = N*x
        delete[] paddedSignal;
        delete[] paddedKernel;
        delete[] workSpace;
        delete[] convolution;
        pffft_destroy_setup(fftSetup);
        return out; 
    }
    
    void normalise(signal& input, float scaling) {
        sample value = 0.0, tempValue;
		for (const sample& sample : input) {
			tempValue = abs(sample);
			if (tempValue > value) value = tempValue;
		}
		for (sample& sample : input) {
			sample = sample / value * scaling;
		}
    }

    // ------------------------------------------------------------------------
    //  Filter
    // ------------------------------------------------------------------------

	signal designLPF(unsigned int M, float fc)
	{
		// Windowed-sinc FIR low-pass using Blackman
		// ---  Smith S. W. The Scientist and Engineer's guide to DSP --- 
		signal h = signal((size_t)M, sample());
		float n, sum = 0;
		for (unsigned int i = 0; i < M; i++) {
			n = (sample)i - (sample)M / 2;
			if (n == 0.0)
				h[i] = 2 * (sample)pi * fc;
			else
				h[i] = sinf(2.0f * (sample)pi * fc * n) / n;
			// the filter is already built at this stage. The next line applies a Blackman window to it.
			h[i] = h[i] * (0.42f - 0.5f * cosf(2.0f * (sample)pi * (sample)i / (sample)M) + 0.08f * cosf(4.0f * (sample)pi * (sample)i / (sample)M));
			sum += h[i];
		}
		// DC normalisation
		for (unsigned int i = 0; i < M; i++)
			h[i] = h[i] / sum;
		return h;
	}


	signal designHPF(unsigned int M, float fc)
	{
		signal h = designLPF(M, fc);
		// high-pass FIR using spectral inversion.
		for (unsigned int i = 0; i < M; i++)
			h[i] = -h[i];
		h[(size_t)(M / 2)] += 1;
		return h;
	}


	signal designBPF(unsigned int M, float lower_fc, float upper_fc)
	{
		signal h = designLPF(M, upper_fc), hpf = designHPF(M, lower_fc);
		// Band-pass FIR combining the above.
		for (unsigned int i = 0; i < M; i++) {
			h[i] = h[i] + hpf[i];
			h[i] = -h[i];
		}
		h[(size_t)(M / 2)] += 1;
		return h;
	}


	Filter::Filter(filterType filt_t, float cutoff_hz)
	{
		lower_fc = cutoff_hz / fs;
        
		if (!(lower_fc > 0 && lower_fc < 0.5)) 
            DSPTB_ERROR("[DSP ERROR]: Invalid parameter cutoff_hz to Filter!");
		if (fs < 0)
            DSPTB_ERROR("[DSP ERROR]: Invalid Sample Rate");
		if (filt_t == filterType::BPF)
            DSPTB_ERROR("[DSP ERROR]: Invalid parameter filterType to Filter!");
		if (filt_t == filterType::LPF)	h = designLPF(M, lower_fc);
		else							h = designHPF(M, lower_fc);
	}


	Filter::Filter(filterType filt_t, float lower_cutoff_hz, float upper_cutoff_hz)
	{
		lower_fc = lower_cutoff_hz / fs;
		upper_fc = upper_cutoff_hz / fs;
		if (!(lower_fc > 0 && lower_fc < 0.5))
            DSPTB_ERROR("[DSP ERROR]: Invalid parameter cutoff_hz to Filter!");
		if (!(upper_fc > 0 && upper_fc < 0.5))
            DSPTB_ERROR("[DSP ERROR]: Invalid parameter cutoff_hz to Filter!");
		if (fs < 0)
            DSPTB_ERROR("[DSP ERROR]: Invalid Sample Rate");
		if (!(filt_t == filterType::BPF))
            DSPTB_ERROR("[DSP ERROR]: Invalid parameter filterType to Filter!");
		h = designBPF(M, lower_fc, upper_fc);
	}
	void Filter::convolve(signal& inputSignal) const
	{
		signal convolution = fft_convolve(inputSignal, h);
		convolution.erase(convolution.begin(), convolution.begin() + h.size() / 2);
		convolution.erase(convolution.begin() + inputSignal.size(), convolution.end());
		inputSignal.swap(convolution);
	}

} // namespace dsptb
