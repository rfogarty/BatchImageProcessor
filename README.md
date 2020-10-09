# BatchImageProcessor

## DESCRIPTION
A small tool (executable *batchIP*) to study Image Processing algorithms.

BatchImageProcessor is mostly written as template library to support rapid
prototyping for different image and pixel formats, processing algorithms, etc.

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

Requires: Gnu Make and C++ compiler (such as g++ or clang++)

    $ cd project
    $ make

Default target (all) builds:
* static library: lib/libbatchIPTools.a
* unit test executable: project/bin/testImage
* main tool executable: project/bin/batchIP


Notes: Makefile CCFLAGS and CCC arguments may be tweaked to support C++11, different
debug flags and optimization modes or to use a different compiler. Additionally,
compiler flags may be set for compiling with AddressSanetizer if available.




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

| Short Description     | Function Name | # Params |          Parameters         |              Long Description
|-----------------------|---------------|----------|-----------------------------|-----------------------------------------------
| Intensity             | add           |        1 | <amount     (int)>          | brighten or darken a grayscale image.
| Binarization          | binarize      |        1 | <threshold  (unsigned)>     | binarize the pixels with the threshold.
| OptimalBinarization   | optBinarize   |        0 |                             | binarize the image using optimal threshold.
| OtsuBinarization      | otsuBinarize  |        0 |                             | binarize the image using Otsu threshold.
| BinarizationRange     | binarizeDT    |        2 | <threshLow  (unsigned)>     | binarize the pixels with 2 thresholds.
|                       |               |          | <threshHigh (unsigned)>     | 
| Histogram             | hist          |        0 |                             | compute histogram of grayscale intensity.
| HistogramModify       | histMod       |        2 | <low        (unsigned)>     | histogram stretch values between low and high.
|                       |               |          | <high       (unsigned)>     | 
| Resize [^1]           | scale         |        1 | <0.5 or 2.0 (float)>        | double or halve the size of an image.
| Smooth                | uniformSmooth |        1 | <windowSize (odd,unsigned)> | smooth an image using uniform box.




### Color images (.ppm) 

| Short Description     | Function Name | # Params |         Parameters          |              Long Description
|-----------------------|---------------|----------|-----------------------------|----------------------------------------------------------------
| Intensity             | add           |        1 | <amount     (int)>          | brighten or darken a color image.
| Binarization          | binarizeColor |        4 | <threshold  (float)>        | binarize the pixels with threshold distance from coordinate RGB
|                       |               |          | <red        (unsigned)>     | 
|                       |               |          | <green      (unsigned)>     | 
|                       |               |          | <blue       (unsigned)>     | 
| Histogram[^2]         | histChan      |        1 | <channel    (unsigned 0-2)> | compute histogram of one of the RGB channel intensities.
| HistogramModify       | histMod       |        2 | <low        (unsigned)>     | histogram stretch all RGB values between low and high.
|                       |               |          | <high       (unsigned)>     | 
| HistogramModIntensity | histModI      |        2 | <low        (unsigned)>     | histogram stretch intensity of color file between low and high.
|                       |               |          | <high       (unsigned)>     | 
| HistogramModifyRGB    | histModAnyRGB |        3 | <low        (unsigned)>     | histogram stretch intensity of any RGB channel
|                       |               |          | <high       (unsigned)>     | 
|                       |               |          | <channel    (unsigned 0-2)> | 
| HistogramModifyHSI    | histModAnyHSI |        6 | <lowI       (unsigned)>     | histogram stretch hue, saturation and value all at once
|                       |               |          | <highI      (unsigned)>     | 
|                       |               |          | <lowS       (unsigned)>     |    low/high values may be 0,255 to do nothing
|                       |               |          | <highS      (unsigned)>     | 
|                       |               |          | <lowH       (unsigned)>     | 
|                       |               |          | <highH      (unsigned)>     | 
| SelectColor[^2]       | selectColor   |        1 | <channel    (unsigned 0-2)> | output one channel of RGB to gray image
| SelectHSI[^2]         | selectHSI     |        1 | <channel    (unsigned 0-2)> | output one channel of HSI to gray image
| AfixAnyHSI[^2]        | afixAnyHSI    |        2 | <value      (unsigned)>     | Afix all pixels to a single value for one of H-S-I
|                       |               |          | <channel    (unsigned 0-2)> | 


[^1]: Note, Resize function does not support the ROI feature.
[^2]: Note, color histogram, selectColor and selectHSI functions require the output to be a grayscale file with suffix .pgm
