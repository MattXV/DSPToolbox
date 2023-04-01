# DSPToolbox
C++ shared library of DSP utilities I use frequently. WIP. Adding tools as I need them.

## Current Features 
- Full convolution and real-time Overlap-Add block convolution;
- Geometrical acoustics impulse response processing, implementing Schroder's methods (2011);
- HRTF spatialiser system using [libmysofa](https://github.com/hoene/libmysofa), with custom hot-swappable SOFA files.

They are reasonably fast implementations, with minimal dependencies, that integrate with Unity by importing a built DLL. **Pre-built binaries with Unity package and example project coming soon**.

I mainly implemented these features myself because I needed full control over them in Hololens 2 devices, which currently don't offer much pre-existing DSP tools. Hence, this project builds an equally optimal UWP ARM64 DLL as a Unity plugin for the Hololens 2. **This will be included in the release too**.

I'm working on this in my spare time, slowly writing unit tests and error-handling codes. If you're interested any of these features for your project, feel free to contact me (section below).

### Internal DSP Tools
- Filters: FIR Low-pass, band-pass and high-pass. band and high are just inversions and combinations of low-pass filters. Implementations from Smith's book (1997).
- FFT Convolution based on [PFFFT](https://bitbucket.org/jpommier/pffft.git).
- Filterbank to transform frequency-dependent IRs to a single IR.
- Overlap-Add block convolution.

## References
- Schröder, D., 2011. Physically based real-time auralization of interactive virtual environments (Vol. 11). Logos Verlag Berlin GmbH.
- (1997) Smith, S.W., Linearity, S., Fidelity, S., Decompositions, C., Notation, P., Nuisances, P., High-Pass, B.P., Convolution, F.F.T., Overshoot, S.R., Hearing, H. and Transforms, G., The Scientist and Engineer's Guide to Digital Signal Processing By Steven W. Smith, Ph. D.

## Contacts
- [mattia.clmb@gmail.com](mailto:mattia.clmb@gmail.com)
- [About me](https://mattxv.github.io)
