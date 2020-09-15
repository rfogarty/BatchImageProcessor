/************************************************************
 * iptool.cpp - a simple batch processing tool for 
 *              experimenting with Image Processing
 *
 ************************************************************/

#include "image/NetpbmImage.h"
#include "image/ImageOperation.h"
#include "image/ImageAlgorithm.h"
#include "image/RegionOfInterest.h"
#include "utility/StringParse.h"
#include <fstream>
#include <iostream>
#include <sstream>


void printHelpAndExit(const char* execname) {
   std::cerr << "Usage: " << execname << " <operations_file>" << std::endl;
   exit(1);
}

void parseAndRunOperation(const std::string& line) {

   std::stringstream ss;
   ss << line;

   // All Operations should be on a single line of the form:
   // 1) inputfile
   // 2) outputfile
   // 3) operation
   // 4) operation parameters
   // 5) [ROI: rowBegin, colBegin, rowSize, colSize <parameters>]*
   //    Note: "ROI:" followed by a space is required
   //          before each region is specified.
  
   std::string inputfile;
   std::string outputfile;
   std::string operation;

   try {
      inputfile = parseWord<std::string>(ss);
      outputfile = parseWord<std::string>(ss);
      operation = parseWord<std::string>(ss);
   }
   catch(const ParseError& pe) {
      std::cerr << "ERROR: parsing line: " << line << "\n"
                << "ERROR: " << pe.what() << std::endl;
      return;
   }

   using namespace algorithm;
   
   if(startsWith(inputfile,"#")); // commented line, so do nothing
   else if(endsWith(inputfile,".pgm")) {
      if(!endsWith(outputfile,".pgm")) {
          std::cerr << "ERROR: on line: " << line << "\n"
                    << "ERROR: currently processing doesn't support different input and output file types" << std::endl;
          return;
      }

      typedef GrayAlphaPixel<unsigned char> PixelT;
      typedef Image<PixelT> ImageT;
      typedef Action<ImageT> ActionT;
      typedef Operation<ActionT> OperationT;

      ActionT* action = 0;

      try {
         if(operation == "add") {
            action = Intensity<ImageT>::make(ss);
         }
         else if(operation == "scale") {
            action = Scale<ImageT>::make(ss);
         }
         else if(operation == "binarize") {
            action = Binarize<ImageT>::make(ss);
         }
         else if(operation == "binarizeDT") {
            action = BinarizeDT<ImageT>::make(ss);
         }
         else if(operation == "uniformSmooth") {
            action = UniformSmooth<ImageT>::make(ss);
         }
         else {
            std::cerr << "Unknown operation: " << operation << std::endl;
            return;
         }
         OperationT op(action);
         parseROIsAndParameters(op,action->numParameters(),ss);

         // Lastly, run the Operation!
         ImageT tgt = op.run(readPGMFile<PixelT>(inputfile));
         writePGMFile(outputfile,tgt);
      }
      catch(const ParseError& pe) {
         std::cerr << "ERROR: parsing operation on line: " << line << "\n"
                   << "ERROR: " << pe.what() << std::endl; 
      }
      catch(const std::exception& e) {
         std::cerr << "ERROR: processing operation: " << e.what() << std::endl;
      }
   }
   else if(endsWith(inputfile,".ppm")) {
      if(!endsWith(outputfile,".ppm")) {
          std::cerr << "ERROR: on line: " << line << "\n"
                    << "ERROR: currently processing doesn't support different input and output file types" << std::endl;
          return;
      }

      typedef ColorPixel<unsigned char> PixelT;
      typedef Image<PixelT> ImageT;
      typedef Action<ImageT> ActionT;
      typedef Operation<ActionT> OperationT;

      ActionT* action = 0;

      try {
         if(operation == "add") {
            action = Intensity<ImageT>::make(ss);
         }
         else if(operation == "binarizeColor") {
            action = BinarizeColor<ImageT>::make(ss);
         }
         else {
            std::cerr << "Unknown operation: " << operation << std::endl;
            return;
         }
         OperationT op(action);
         parseROIsAndParameters(op,action->numParameters(),ss);

         // Lastly, run the Operation!
         ImageT tgt = op.run(readPPMFile<PixelT>(inputfile));
         writePPMFile(outputfile,tgt);
      }
      catch(const ParseError& pe) {
         std::cerr << "ERROR: parsing operation on line: " << line << "\n"
                   << "ERROR: " << pe.what() << std::endl; 
      }
      catch(const std::exception& e) {
         std::cerr << "ERROR: processing operation: " << e.what() << std::endl;
      }
   }
   else {
       std::cerr << "ERROR: on line: " << line << "\n"
                 << "ERROR: unknown input file type" << std::endl;
   }
}


int main (int argc, char** argv) {

   if(argc != 2) printHelpAndExit(argv[0]);

   // This main function iterates an input file
   // that describes operations to run on input images.
   std::fstream opsFile;
   opsFile.open(argv[1], std::ios_base::in);
   if(opsFile.is_open()) {
      for(std::string line; std::getline(opsFile, line);) {
         if(line.size() > 0) parseAndRunOperation(line);
      }
   }
   else {
      std::cerr << "File: " << argv[1] << " could not be found." << std::endl;
      printHelpAndExit(argv[0]);
   }

	return 0;
}

