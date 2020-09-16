# BatchImageProcessor

## Description
A small tool (executable *iptool*) to study Image Processing algorithms.

BatchImageProcessor is mostly written as template library to support rapid
prototyping for different image and pixel formats, processing algorithms, etc.

### Directory Structure
This software currently has the following directory structure:

    ├── iptools       - top-level source directory for libiptools.a library
    │   ├── cppTools  - C++ platform independence and template programming support headers
    │   ├── image     - contains all Image types and Image Processing algorithms
    │   └── utility   - support utilities such as string parsing and error handling
    ├── lib           - build location for libiptools.a static archive
    ├── project       - directory that has source code for the main iptool and a unit test
    │   └── bin       - build location of iptool and testImage executables
    ├── ...
    ├── ...
    └── readme.txt




## BUILDING

Requires: Gnu Make and C++ compiler (such as g++ or clang++)

    $ cd project
    $ make

Default target (all) builds:
* static library: lib/libiptools.a
* unit test executable: project/bin/testImage
* main tool executable: project/bin/iptool


Notes: Makefile CCFLAGS and CCC arguments may be tweaked to support C++11, different
debug flags and optimization modes or to use a different compiler.




## RUNNING (BatchImageProcessor tool)

From top level directory

    $ cd project/bin   # or place project/bin in the shell's ${PATH}
    $ iptool <parametersFile.txt>

where the format of a parametersFile.txt is immediately below.




## PARAMETERS FILE

Briefly the parameters file may be expressed formally (ANTLR syntax) as:

    parameterfile      : (commentline|operation)* EOF ;
    commentline        : "#" .* NL ;
    operation          : STRING STRING STRING (STRING)* (roi)* NL ;
    roi                : "ROI:" UNUM UNUM UNUM UNUM (STRING)* ;


In human speak, a parameters file may include multiple operation lines, where each line has

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

*iptool* currently supports the functions as indicated in the tables below.


### Grayscale images (.pgm)

| Short Description  | Function Name |                    Parameters                         |          Long Description
|--------------------|---------------|-------------------------------------------------------|----------------------------------------
| Intensity          | add           | <amount (int)>                                        | brighten or darken a grayscale image.
| Binarization       | binarize      | <threshold (unsigned)>                                | binarize the pixels with the threshold.
| BinarizationRange  | binarizeDT    | <thresholdLow (unsigned)> <thresholdHigh (unsigned)>  | binarize the pixels with 2 thresholds.
| Resize [^1]        | scale         | <0.5 or 2.0 (float)>                                  | double or halve the size of an image.
| Smooth             | uniformSmooth | <windowSize (odd,unsigned)>                           | smooth an image using uniform box.


[^1]: Note, Resize function does not support the ROI feature.


### Color images (.ppm)

| Short Description  | Function Name |                    Parameters                         |          Long Description
|--------------------|---------------|-------------------------------------------------------|----------------------------------------------------------------
| Intensity          | add           | <amount (int)>                                        | brighten or darken a color image.
| Binarization       | binarizeColor | <threshold (float)>  <red green blue (3x unsigned)>   | binarize the pixels with threshold distance from coordinate RGB


