# BatchImageProcessor

## DESCRIPTION
A small tool (executable *batchIP*) to study Image Processing algorithms.

BatchImageProcessor is mostly written as template library to support rapid
prototyping for different image and pixel formats, processing algorithms, etc.

VERSION=v0.4

### Directory Structure
This software currently has the following directory structure:

    ├── batchIP       - Top-level source directory for libbatchIPTools.a library
    │   ├── cppTools  - C++ platform independence and template programming support headers
    │   │   └── ...   - <C++ platform/compiler support source files>
    │   ├── image     - Contains all Image types and Image Processing algorithms
    │   │   └── ...   - <main image support source files>
    │   ├── utility   - Support utilities such as string parsing and error handling
    │   │   └── ...   - <utility source files>
    │   └── Makefile  - Build support for batchIPTool library
    │
    ├── lib           - Build location for libbatchIPTools.a static archive
    ├── project       - Directory that has source code for the main batchIP program and a unit test
    │   ├── ...       - <program/executable source files>
    │   ├── bin       - Build location of batchIP and testImage executables
    │   └── Makefile  - Build support for batchIP program and unit-test testImage
    │
    ├── LICENSE
    ├── README.md
    └── TODO.txt


## BUILDING

Requires: Gnu Make, C++ compiler (such as g++ or clang++), and OpenCV4!

Because of the dependency upon OpenCV4, the build procedure has changed slightly
since v0.3. The Makefiles will attempt to use the tool pkg-config to load compiler and
linker flags. However, a new build script (build.sh) is now incorporated to make this
new requirement transparent.

    $ cd project
    $ build.sh

Default target (all) builds:
* static library: lib/libbatchIPTools.a
* unit test executable: project/bin/testImage
* main tool executable: project/bin/batchIP


Notes: Makefile build flags are all set in the toplevel Settings.mak file.
Options in that file can turn on/off debug settings, compiler optimizations,
warning levels, and advanced options such as compiling with AddressSanetizer.



## RUNNING (BatchImageProcessor tool)

From top level directory

    $ cd project/bin   # or place project/bin in the shell's ${PATH}
    $ batchIP <parametersFile.txt>

where the format of a parametersFile.txt is immediately below.




## PARAMETERS FILE

Briefly the parameters file may be expressed informally (ANTLR syntax) as:

    parameterfile      : (commentline|emptyline|operation)* EOF ;
    commentline        : "#" .* NL ;
    emptyline          : NL ;
    operation          : STRING STRING STRING (STRING)* (roi)* NL ;
    roi                : "ROI:" UNUM UNUM UNUM UNUM (STRING)* ;


In human terms, a parameters file may include multiple operation lines, where each line has

    inputFile outputFile functionName <default function parameters>* [roi]*

Each operation line can end with an arbitrary number of "Regions Of Interest" (or ROIs),
which has the following format (starting with the literal string "ROI:"):

    ROI: RowCoordinate ColCoordinate NumRows NumCols <function parameters>*

The fields RowCoordinate, ColCoordinate NumRows NumCols must all be unsigned integers.

The number of function parameters depends on the selected function (see below in FUNCTIONS)
and a default set must be provided after the functionName, as well as a set for at the end
of each ROI. There is no restriction on the number of ROIs that are supported. However,
currently, they must all be placed on a single line.

Note that the literal "ROI:" serves as a useful aid to the human reader/composer to easily 
identify multiple ROI sections and parameters.



## FUNCTIONS

*batchIP* currently supports the functions as indicated in the tables below.


### Grayscale images (.pgm)

| Short Description      | Function Name | # Params |          Parameters         |              Long Description
|------------------------|---------------|----------|-----------------------------|-----------------------------------------------
| Intensity              | add           |        1 | <amount     (int)>          | brighten or darken a grayscale image.
| Binarization           | binarize      |        1 | <threshold  (unsigned)>     | binarize the pixels with the threshold.
| Crop[^2]               | crop          |        4 | <rowBegin   (unsigned)>     | crop an image based on region
|                        |               |          | <colBegin   (unsigned)>     | 
|                        |               |          | <rows       (unsigned)>     | 
|                        |               |          | <cols       (unsigned)>     | 
| OptimalBinarization    | optBinarize   |        0 |                             | binarize the image using optimal threshold.
| OtsuBinarization       | otsuBinarize  |        0 |                             | binarize the image using Otsu threshold.
| BinarizationRange      | binarizeDT    |        2 | <threshLow  (unsigned)>     | binarize the pixels with 2 thresholds.
|                        |               |          | <threshHigh (unsigned)>     | 
| Histogram[^1]          | hist          |        1 | <type       (unsigned 0,2)> | compute histogram of grayscale intensity; type is 0-linear, 2-log
| HistogramModify        | histMod       |        2 | <low        (unsigned)>     | histogram stretch values between low and high.
|                        |               |          | <high       (unsigned)>     | 
| Resize[^2]             | scale         |        1 | <0.5 or 2.0 (float)>        | double or halve the size of an image.
| Smooth                 | uniformSmooth |        1 | <windowSize (odd,unsigned)> | smooth an image using uniform box.
| Histogram EQ           | histEQCV      |        0 |                             | histogram equalizes (OpenCV) an image.
| Thresh. Histogram EQ   | thresholdEQCV |        1 | <region (0-fg,1-bg,2-both)> | Otsu threshold, then histogramEQ foreground or background.
| OtsuBinarization (OCV) | otsuBinarizeCV|        0 |                             | binarize the image with Otsu threshold (OpenCV).
| EdgeGradientAmplitude  | edgeGradient  |        1 | <windowSize (unsigned 3,5)> | Sobel edge gradient magnitude.
| EdgeGradientDetect     | edgeDetect    |        1 | <windowSize (unsigned 3,5>  | thresholded (Otsu) Sobel edge detection. 
| OrientedEdgeGradient   | orientedEdgeGradient | 3 | <windowSize (unsigned 3,5)> | oriented Sobel edge gradient
|                        |               |          | <angle0 (float -180:180)>   |    if angle0 < angle1: angle0< edge < angle1
|                        |               |          | <angle1 (float -180:180)>   |    if angle0 > angle1: edge < angle1 or angle0 < edge (disjoint compare)
| OrientedEdgeDetect     | orientedEdgeDetect |   3 | <windowSize (unsigned 3,5)> | thresholded oriented Sobel edge detect
|                        |               |          | <angle0 (float -180:180)>   |    if angle0 < angle1: angle0 < edge < angle1
|                        |               |          | <angle1 (float -180:180)>   |    if angle0 > angle1: edge < angle1 or angle0 < edge (disjoint compare)
| EdgeSobel              | edgeSobelCV   |        1 | <windowSize (unsig 1,3,5,7)>| OpenCV Sobel edge magnitude
| EdgeCanny              | edgeCannyCV   |        3 | <windowSize (unsig 1,3,5,7)>| OpenCV Canny edge detector
|                        |               |          | <lowThresh  (double)>       |
|                        |               |          | <highThresh (double)>       |
| QRDecode               | qrDecodeCV    |        1 | <histEQ     (bool)>         | OpenCV implementation of QRDecoder
| PowerSpectrum (DFT)    | powerSpectrum |        0 |                             | log of frequency-domain magnitude or power spectrum
| LowPassFilterResponse  | lpFilterResp  |        1 | <cutoffFreq (float 0:1)>    | log power spectrum of frequency response after lowpass filter is applied
| HighPassFilterResponse | hpFilterResp  |        1 | <cutoffFreq (float 0:1)>    | log power spectrum of frequency response after lowpass filter is applied
| BandPassFilterResponse | bpFilterResp  |        2 | <lowCutoffFreq (float 0:1)> | log power spectrum of frequency response after bandpass filter is applied
|                        |               |          | <highCutoffFreq (float 0:1)>|     Note: if lowCutoffFreq > highCutoffFreq, filter acts as bandstop
| LowPassFilter          | lpFilter      |        1 | <cutoffFreq (float 0:1)>    | lowpass filter an image.
| HighPassFilter         | hpFilter      |        1 | <cutoffFreq (float 0:1)>    | highpass filter an image. 
| BandPassFilter         | bpFilter      |        2 | <lowCutoffFreq (float 0:1)> | bandpass (or bandstop) filter an image.
|                        |               |          | <highCutoffFreq (float 0:1)>|     Note: if lowCutoffFreq > highCutoffFreq, filter acts as bandstop
| FilterResponse         | filterResp    |        4 | <lowCutoff_1 (float 0:1)>   | log power spectrum of image or ROI after dual band filter is applied
|                        |               |          | <highCutoff_1 (float 0:1)>  |     Combines two separate bandpass/bandstop filters
|                        |               |          | <lowCutoff_2 (float 0:1)>   |     Band1: lowCutoff_1:highCutoff_1
|                        |               |          | <highCutoff_2 (float 0:1)>  |     Band2: lowCutoff_2:highCutoff_2
| (Dual-band) Filter     | filter        |        4 | <lowCutoff_1 (float 0:1)>   | dual band filter an image.
|                        |               |          | <highCutoff_1 (float 0:1)>  |     Combines two separate bandpass/bandstop filters
|                        |               |          | <lowCutoff_2 (float 0:1)>   |     Band1: lowCutoff_1:highCutoff_1
|                        |               |          | <highCutoff_2 (float 0:1)>  |     Band2: lowCutoff_2:highCutoff_2


### Color images (.ppm)  

| Short Description      | Function Name | # Params |         Parameters          |              Long Description
|------------------------|---------------|----------|-----------------------------|----------------------------------------------------------------
| Intensity              | add           |        1 | <amount     (int)>          | brighten or darken a color image.
| Binarization           | binarizeColor |        4 | <threshold  (float)>        | binarize the pixels with threshold distance from coordinate RGB
|                        |               |          | <red        (unsigned)>     | 
|                        |               |          | <green      (unsigned)>     | 
|                        |               |          | <blue       (unsigned)>     | 
| Crop[^2]               | crop          |        4 | <rowBegin   (unsigned)>     | crop an image based on region
|                        |               |          | <colBegin   (unsigned)>     | 
|                        |               |          | <rows       (unsigned)>     | 
|                        |               |          | <cols       (unsigned)>     | 
| Histogram[^1][^3]      | histChan      |        2 | <type       (unsigned 0,2)> | compute histogram of the RGB channel intensity; type is 0-linear, 2-log
|                        |               |          | <channel    (unsigned 0-2)> | 
| HistogramModify        | histMod       |        2 | <low        (unsigned)>     | histogram stretch all RGB values between low and high.
|                        |               |          | <high       (unsigned)>     | 
| HistogramModIntensity  | histModI      |        2 | <low        (unsigned)>     | histogram stretch intensity of color file between low and high.
|                        |               |          | <high       (unsigned)>     | 
| HistogramModifyRGB     | histModAnyRGB |        3 | <low        (unsigned)>     | histogram stretch intensity of any RGB channel
|                        |               |          | <high       (unsigned)>     | 
|                        |               |          | <channel    (unsigned 0-2)> | 
| HistogramModifyHSI     | histModAnyHSI |        6 | <lowI       (unsigned)>     | histogram stretch hue, saturation and value all at once
|                        |               |          | <highI      (unsigned)>     | 
|                        |               |          | <lowS       (unsigned)>     |    low/high values may be 0,255 to do nothing
|                        |               |          | <highS      (unsigned)>     | 
|                        |               |          | <lowH       (unsigned)>     | 
|                        |               |          | <highH      (unsigned)>     | 
| SelectColor[^2][^3]    | selectColor   |        1 | <channel    (unsigned 0-2)> | output one channel of RGB to gray image
| SelectHSI[^2][^3]      | selectHSI     |        1 | <channel    (unsigned 0-2)> | output one channel of HSI to gray image
| AfixAnyHSI             | afixAnyHSI    |        2 | <value      (unsigned)>     | Afix all pixels to a single value for one of H-S-I
|                        |               |          | <channel    (unsigned 0-2)> | 
| PowerSpectrum (DFT)    | powerSpectrum |        0 |                             | log of frequency-domain magnitude or power spectrum
| LowPassFilterResponse  | lpFilterResp  |        1 | <cutoffFreq (float 0:1)>    | log power spectrum of frequency response after lowpass filter is applied
| HighPassFilterResponse | hpFilterResp  |        1 | <cutoffFreq (float 0:1)>    | log power spectrum of frequency response after lowpass filter is applied
| BandPassFilterResponse | bpFilterResp  |        2 | <lowCutoffFreq (float 0:1)> | log power spectrum of frequency response after bandpass filter is applied
|                        |               |          | <highCutoffFreq (float 0:1)>|     Note: if lowCutoffFreq > highCutoffFreq, filter acts as bandstop
| LowPassFilter          | lpFilter      |        1 | <cutoffFreq (float 0:1)>    | lowpass filter an image.
| HighPassFilter         | hpFilter      |        1 | <cutoffFreq (float 0:1)>    | highpass filter an image. 
| BandPassFilter         | bpFilter      |        2 | <lowCutoffFreq (float 0:1)> | bandpass (or bandstop) filter an image.
|                        |               |          | <highCutoffFreq (float 0:1)>|     Note: if lowCutoffFreq > highCutoffFreq, filter acts as bandstop
| FilterResponse         | filterResp    |        4 | <lowCutoff_1 (float 0:1)>   | log power spectrum of image or ROI after dual band filter is applied
|                        |               |          | <highCutoff_1 (float 0:1)>  |     Combines two separate bandpass/bandstop filters
|                        |               |          | <lowCutoff_2 (float 0:1)>   |     Band1: lowCutoff_1:highCutoff_1
|                        |               |          | <highCutoff_2 (float 0:1)>  |     Band2: lowCutoff_2:highCutoff_2
| (Dual-band) Filter     | filter        |        4 | <lowCutoff_1 (float 0:1)>   | dual band filter an image.
|                        |               |          | <highCutoff_1 (float 0:1)>  |     Combines two separate bandpass/bandstop filters
|                        |               |          | <lowCutoff_2 (float 0:1)>   |     Band1: lowCutoff_1:highCutoff_1
|                        |               |          | <highCutoff_2 (float 0:1)>  |     Band2: lowCutoff_2:highCutoff_2


[^1]: Note, Histogram (hist,histChan) can use the ROI feature, but support just one region.

[^2]: Note, Resize, Crop, SelectColor, and SelectHSI functions do not support the ROI feature.

[^3]: Note, color histogram, selectColor and selectHSI functions require the output to be a grayscale file with suffix .pgm

