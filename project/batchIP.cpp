/************************************************************
 * iptool.cpp - a simple batch processing tool for 
 *              experimenting with Image Processing
 *
 ************************************************************/

#include "image/NetpbmImage.h"
#include "image/ImageOperation.h"
#include "image/ImageActions.h"
#include "image/RegionOfInterest.h"
#include "utility/StringParse.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>


namespace batchIP {
namespace {

void printHelpAndExit(const char* execname) {
   std::cerr << "Usage: " << execname << " <operations_file>" << std::endl;
   exit(1);
}


template<typename ImageT>
ImageT readImage(const std::string& inputfile,const std::string& line,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<types::is_grayscale<typename ImageT::pixel_type>::value,int>::type* = 0) {
   if(!utility::endsWith(inputfile,".pgm")) {
      std::stringstream ss;
      ss << "ERROR: on line: " << line << "\n"
         << "ERROR: outputfile is expected to be .pgm (grayscale)" << std::endl;
      throw std::invalid_argument(ss.str());
   }
   return io::readPGMFile<typename ImageT::pixel_type>(inputfile);
}

template<typename ImageT>
ImageT readImage(const std::string& inputfile,const std::string& line,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<types::is_rgba<typename ImageT::pixel_type>::value,int>::type* = 0) {
   if(!utility::endsWith(inputfile,".ppm")) {
      std::stringstream ss;
      ss << "ERROR: on line: " << line << "\n"
         << "ERROR: outputfile is expected to be .pgm (grayscale)" << std::endl;
      throw std::invalid_argument(ss.str());
   }
   return io::readPPMFile<typename ImageT::pixel_type>(inputfile);
}

template<typename ImageT>
void saveImage(const ImageT& image, const std::string& outputfile,const std::string& line,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<types::is_grayscale<typename ImageT::pixel_type>::value,int>::type* = 0) {
   if(!utility::endsWith(outputfile,".pgm")) {
      std::stringstream ss;
      ss << "ERROR: on line: " << line << "\n"
         << "ERROR: outputfile is expected to be .pgm (grayscale)" << std::endl;
      throw std::invalid_argument(ss.str());
   }
   io::writePGMFile<ImageT::pixel_type::GRAY_CHANNEL>(outputfile,image);
}

template<typename ImageT>
void saveImage(const ImageT& image, const std::string& outputfile,const std::string& line,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<types::is_rgba<typename ImageT::pixel_type>::value,int>::type* = 0) {
   if(!utility::endsWith(outputfile,".ppm")) {
      std::stringstream ss;
      ss << "ERROR: on line: " << line << "\n"
         << "ERROR: outputfile is expected to be .ppm (color)" << std::endl;
      throw std::invalid_argument(ss.str());
   }
   io::writePPMFile(outputfile,image);
}


template<typename ActionT>
void process(const std::string& inputfile,
             const std::string& outputfile,
             const std::string& operation,
             const std::string& line,
             std::istream& ins,
             ActionT* action) {

   typedef typename ActionT::src_image_type ImageSrc;
   typedef typename ActionT::tgt_image_type ImageTgt;
   typedef operation::Operation<ActionT,ImageSrc,ImageTgt> OperationT;

   OperationT op(action);
   parseROIsAndParameters(op,action->numParameters(),ins);

   // Lastly, run the Operation!
   if(operation == "hist" || operation == "histChan") { 
      ImageTgt tgt(ImageTgt::pixel_type::traits::max()+1u,
                   ImageTgt::pixel_type::traits::max()+1u);
      op.run(readImage<ImageSrc>(inputfile,line),tgt);
      saveImage(tgt,outputfile,line);
   }
   else {
      ImageTgt tgt = op.run(readImage<ImageSrc>(inputfile,line));
      saveImage(tgt,outputfile,line);
   }
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
      inputfile = utility::parseWord<std::string>(ss);
      if(utility::startsWith(inputfile,"#")) return;
      std::cout << "Processing: " << (line.size() > 80 ? line.substr(0,80) + "..." : line) << std::endl;
      outputfile = utility::parseWord<std::string>(ss);
      operation = utility::parseWord<std::string>(ss);
   }
   catch(const utility::ParseError& pe) {
      std::cerr << "ERROR: parsing line: " << line << "\n"
                << "ERROR: " << pe.what() << std::endl;
      return;
   }

   using namespace ::batchIP::operation;
   
   if(utility::endsWith(inputfile,".pgm")) {

      typedef types::GrayAlphaPixel<uint8_t> PixelT;
      typedef types::Image<PixelT> ImageT;
      try {
         if     (operation == "add")           process(inputfile,outputfile,operation,line,ss,Intensity<ImageT>::make(ss));
         else if(operation == "crop")          process(inputfile,outputfile,operation,line,ss,Crop<ImageT>::make(ss));
         else if(operation == "hist")          process(inputfile,outputfile,operation,line,ss,Histogram<ImageT>::make(ss));
         else if(operation == "histMod")       process(inputfile,outputfile,operation,line,ss,HistogramModify<ImageT>::make(ss));
         else if(operation == "histEQCV")      process(inputfile,outputfile,operation,line,ss,HistogramEqualizeOCV<ImageT>::make(ss));
         else if(operation == "thresholdEQCV") process(inputfile,outputfile,operation,line,ss,ThresholdEqualizeOCV<ImageT>::make(ss));
         else if(operation == "scale")         process(inputfile,outputfile,operation,line,ss,Scale<ImageT>::make(ss));
         else if(operation == "binarize")      process(inputfile,outputfile,operation,line,ss,Binarize<ImageT>::make(ss));
         else if(operation == "optBinarize")   process(inputfile,outputfile,operation,line,ss,OptimalBinarize<ImageT>::make(ss));
         else if(operation == "otsuBinarize")  process(inputfile,outputfile,operation,line,ss,OtsuBinarize<ImageT>::make(ss));
         else if(operation == "otsuBinarizeCV") process(inputfile,outputfile,operation,line,ss,OtsuBinarizeOCV<ImageT>::make(ss));
         else if(operation == "binarizeDT")    process(inputfile,outputfile,operation,line,ss,BinarizeDT<ImageT>::make(ss));
         else if(operation == "uniformSmooth") process(inputfile,outputfile,operation,line,ss,UniformSmooth<ImageT>::make(ss));
         else if(operation == "edgeGradient")  process(inputfile,outputfile,operation,line,ss,EdgeGradient<ImageT>::make(ss));
         else if(operation == "edgeDetect")    process(inputfile,outputfile,operation,line,ss,EdgeDetect<ImageT>::make(ss));
         else if(operation == "orientedEdgeGradient")  process(inputfile,outputfile,operation,line,ss,OrientedEdgeGradient<ImageT>::make(ss));
         else if(operation == "orientedEdgeDetect")  process(inputfile,outputfile,operation,line,ss,OrientedEdgeDetect<ImageT>::make(ss));
         else if(operation == "edgeSobelCV")   process(inputfile,outputfile,operation,line,ss,EdgeSobelOCV<ImageT>::make(ss));
         else if(operation == "edgeCannyCV")   process(inputfile,outputfile,operation,line,ss,EdgeCannyOCV<ImageT>::make(ss));
#ifdef SUPPORT_QRCODE_DETECT
         else if(operation == "qrDecodeCV")   process(inputfile,outputfile,operation,line,ss,QRDecodeOCV<ImageT>::make(ss));
#endif
         else {
            std::cerr << "Unknown operation: " << operation << std::endl;
            return;
         }
      }
      catch(const utility::ParseError& pe) {
         std::cerr << "ERROR: parsing operation on line: " << line << "\n"
                   << "ERROR: " << pe.what() << std::endl; 
      }
      catch(const std::exception& e) {
         std::cerr << "ERROR: processing operation: " << e.what() << std::endl;
      }
   }
   else if(utility::endsWith(inputfile,".ppm")) {

      typedef types::RGBAPixel<uint8_t> PixelT;
      typedef types::Image<PixelT> ImageT;

      try {
         if     (operation == "add")           process(inputfile,outputfile,operation,line,ss,Intensity<ImageT>::make(ss));
         else if(operation == "crop")          process(inputfile,outputfile,operation,line,ss,Crop<ImageT>::make(ss));
         else if(operation == "binarizeColor") process(inputfile,outputfile,operation,line,ss,BinarizeColor<ImageT>::make(ss));
         else if(operation == "histChan")      process(inputfile,outputfile,operation,line,ss,HistogramChannel<ImageT>::make(ss));
         else if(operation == "histMod")       process(inputfile,outputfile,operation,line,ss,HistogramModifyRGB<ImageT>::make(ss));
         else if(operation == "histModI")      process(inputfile,outputfile,operation,line,ss,HistogramModifyIntensity<ImageT>::make(ss));
         else if(operation == "histModAnyRGB") process(inputfile,outputfile,operation,line,ss,HistogramModifyAnyRGB<ImageT>::make(ss));
         else if(operation == "histModAnyHSI") process(inputfile,outputfile,operation,line,ss,HistogramModifyAnyHSI<ImageT>::make(ss));
         else if(operation == "selectColor")   process(inputfile,outputfile,operation,line,ss,SelectColor<ImageT>::make(ss));
         else if(operation == "selectHSI")     process(inputfile,outputfile,operation,line,ss,SelectHSI<ImageT>::make(ss));
         else if(operation == "afixAnyHSI")    process(inputfile,outputfile,operation,line,ss,AfixAnyHSI<ImageT>::make(ss));
         else {
            std::cerr << "Unknown operation: " << operation << std::endl;
            return;
         }
      }
      catch(const utility::ParseError& pe) {
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

} // unnamed namespace
} // namespace batchIP

int main (int argc, char** argv) {

   using namespace batchIP;

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

