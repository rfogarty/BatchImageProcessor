/************************************************************
 *															*
 * This sample project include three functions:				*
 * 1. Add intensity for gray-level image.					*
 *    Input: source image, output image name, value			*
 *															*
 * 2. Image thresholding: pixels will become black if the	*
 *    intensity is below the threshold, and white if above	*
 *    or equal the threhold.								*
 *    Input: source image, output image name, threshold		*
 *															*
 * 3. Image scaling: reduction/expansion of 2 for 			*
 *    the width and length. This project uses averaging 	*
 *    technique for reduction and pixel replication			*
 *    technique for expansion.								*
 *    Input: source image, output image name, scale factor	*
 *															*
 ************************************************************/

#include "image/NetpbmImage.h"
#include "image/ImageOperation.h"
#include "image/ImageAlgorithm.h"
#include "utility/StringParse.h"
#include <fstream>
#include <iostream>
#include <sstream>


template<typename OperationT>
void parseROIs(OperationT& op,std::stringstream& ss) {

   while(!ss.eof()) {
      try {
         std::string regionID = parseWord<std::string>(ss);
         if(regionID.size() == 0) break;
         if(regionID == "ROI:") {
            unsigned row = parseWord<unsigned>(ss);
            unsigned col = parseWord<unsigned>(ss);
            unsigned rows = parseWord<unsigned>(ss);
            unsigned cols = parseWord<unsigned>(ss);
            op.addRegion(RegionOfInterest(rows,cols,row,col));
         }
         else {
            // Log error and break
            break;
         }
      }
      catch(const ParseError& pe) {
         // TODO: log error
      }
   }
}

void parseOperation(const std::string& line) {
   std::stringstream ss;
   ss << line;

   // All Operations should be on a single line of the form:
   // 1) inputfile
   // 2) outputfile
   // 3) operation
   // 4) operation parameters
   // 5) [ROI: rowBegin, colBegin, rowSize, colSize]*

   std::string inputfile = parseWord<std::string>(ss);
   std::string outputfile = parseWord<std::string>(ss);
   std::string operation = parseWord<std::string>(ss);

   using namespace algorithm;
   
   if(endsWith(inputfile,".pgm")) {
      assert(endsWith(outputfile,".pgm"));

      typedef GrayAlphaPixel<unsigned char> PixelT;
      typedef Image<PixelT> ImageT;
      typedef Action<ImageT> ActionT;
      typedef Operation<ActionT> OperationT;

      ActionT* action = 0;

      if(operation == "add") {
         int amount = parseWord<int>(ss);
         action = new Intensity<ImageT>(amount);
      }
      else if(operation == "binarize") {
         unsigned threshold = parseWord<unsigned>(ss);
         action = new Binarize<ImageT>(threshold);
      }
      else if(operation == "binarizeDT") {
         unsigned thresholdLow = parseWord<unsigned>(ss);
         unsigned thresholdHigh = parseWord<unsigned>(ss);
         action = new BinarizeDT<ImageT>(thresholdLow,thresholdHigh);
      }
      else {
         std::cerr << "Unknown operation: " << operation << std::endl;
         return;
      }
      OperationT op(action);
      parseROIs(op,ss);

      // Lastly, run the Operation!
      ImageT tgt = op.run(readPGMFile<PixelT>(inputfile));
      writePGMFile(outputfile,tgt);
   }
   else if(endsWith(inputfile,".ppm")) {
      assert(endsWith(outputfile,".ppm"));

      typedef ColorPixel<unsigned char> PixelT;
      typedef Image<PixelT> ImageT;
      typedef Action<ImageT> ActionT;
      typedef Operation<ActionT> OperationT;

      ActionT* action = 0;

      if(operation == "add") {
         int amount = parseWord<int>(ss);
         action = new Intensity<ImageT>(amount);
      }
      else if(operation == "binarizeColor") {
         float threshold = parseWord<float>(ss);
         PixelT fromPos;
         fromPos.namedColor.red = parseWord<unsigned>(ss);
         fromPos.namedColor.green = parseWord<unsigned>(ss);
         fromPos.namedColor.blue = parseWord<unsigned>(ss);
         action = new BinarizeColor<ImageT>(threshold,fromPos);
      }
      else {
         std::cerr << "Unknown operation: " << operation << std::endl;
         return;
      }
      OperationT op(action);
      parseROIs(op,ss);

      // Lastly, run the Operation!
      ImageT tgt = op.run(readPPMFile<PixelT>(inputfile));
      writePPMFile(outputfile,tgt);
   }
   else {
      // TODO: report an error?
   }
}

void printHelpAndExit(const char* execname) {
   std::cerr << "Usage: " << execname << " <operations_file>" << std::endl;
   exit(1);
}

int main (int argc, char** argv) {

   if(argc != 2) printHelpAndExit(argv[0]);

   std::fstream opsFile;
   opsFile.open(argv[1], std::ios_base::in);
   if(opsFile.is_open()) {
      for(std::string line; std::getline(opsFile, line);) parseOperation(line);
   }
   else {
      std::cerr << "File: " << argv[1] << " could not be found." << std::endl;
      printHelpAndExit(argv[0]);
   }

	return 0;
}

