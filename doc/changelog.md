# Sound Enhancement Toolkit Changelog #

## List of versions ##

Here is the list of all changes in the application. Further improvements may be done later, but from now on they will be tracked only as commits in this git repo.

### Version 0.10 (04/10/11) ###

- Fixed serious bug in iterative Wiener filter, which caused that the output audio was distorted

### Version 0.9 (03/04/11) ###

- Added MCRA 2 noise estimation algorithm
- Added samplerate parameter in noise estimation and sound enhancement function prototypes
- Code cleanup

### Version 0.8 (03/03/11) ###

- Added MCRA noise estimation algorithm

### Version 0.7 (03/02/11) ###

- Added option residual output - estimated noise is saved into output file instead of enhanced speech, useful when testing noise estimation algorithms efficiency
- Added doblinger noise estimation algorithm
- Improvements and optimization in doblinger noise est.

### Version 0.6 (02/27/11) ###

- Fixed bug in wiener-iter

### Version 0.5 (02/26/11) ###

- Added basic iterative wiener filter (wiener-iter)
- Improved wiener-as
- Optimized processing

### Version 0.4 (02/25/11) ###

- Changes in file_info() - show track duration, samplerate and channel number
- Added function show_time() - calculates elapsed time in seconds based on frames and samplerate
- Fixes and improvements in specsub
- Simple progress bar
- Fixed wiener-as algorithm

### Version 0.3 (02/24/11) ###

- Fixed bugs in MMSE which caused MMSE to not work properly
- MMSE and SPECSUB require less memory
- Code cleanup in MMSE and Specsub

### Version 0.2 (02/23/11) ###

- Added MMSE sound enhancement algorithm
- Minor fixes and improvements

### Version 0.1 ###

- Initial release
- Implemented spectral substractive algorithm
- Implemented wiener filer with a priori snr estimation, has bugs
- Implemented seven types of window function
- Fully functional menu
- Ability to select noise estimation and sound enhancement algorithm
- Implemented simple VAD detection based noise estimation
- Implemented hirsch method of noise estimation
