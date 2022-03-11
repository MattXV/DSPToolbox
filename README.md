# DSPToolbox
C++ shared library of DSP utilities I use frequently. WIP. Adding tools as I need them.

## Tools 
- Filters: FIR Low-pass, band-pass and high-pass. band and high are just inversions and combinations of low-pass filters. Implementations from Smith's book (1997).
- FFT Convolution based on [PFFFT](https://bitbucket.org/jpommier/pffft.git).
- Filterbank to transform frequency-dependent IRs to a single IR.

## References
- (1997) Smith, S.W., Linearity, S., Fidelity, S., Decompositions, C., Notation, P., Nuisances, P., High-Pass, B.P., Convolution, F.F.T., Overshoot, S.R., Hearing, H. and Transforms, G., The Scientist and Engineer's Guide to Digital Signal Processing By Steven W. Smith, Ph. D.