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

#include "image/netpbmImage.h"
#include "utility/utility.h"
#include <fstream>
#include <iostream>
#include <sstream>
//#include <cstdlib>
//#include <cstdio>

using namespace std;

#define MAXLEN 256

struct RegionOfInterest {
   unsigned mRows;
   unsigned mCols;
   unsigned mRowBegin;
   unsigned mColBegin;

   RegionOfInterest(unsigned rows = 0,unsigned cols = 0,
                    unsigned rowBegin = 0,unsigned colBegin = 0) :
      mRows(rows),
      mCols(cols),
      mRowBegin(rowBegin),
      mColBegin(colBegin)
   {}
};


template<typename ImageT>
typename ImageT::ImageViewT roi2view(ImageT& image,const RegionOfInterest& roi) {
   return image.view(roi.mRows,roi.mCols,roi.mRowBegin,roi.mColBegin);
}

template<typename ImageT>
typename ImageT::ConstImageViewT roi2view(const ImageT& image,const RegionOfInterest& roi) {
   return image.view(roi.mRows,roi.mCols,roi.mRowBegin,roi.mColBegin);
}

template<typename ImageViewT>
RegionOfInterest view2roi(ImageViewT& view) {
  return RegionOfInterest(view.rows(),view.cols(),view.rowBegin(),view.colBegin()); 
}

enum ActionOperation {
   INTENSITY,
   BINARIZE,
   BINARIZE_DT,
   SCALE
};


template<typename ImageT>
class AbstractAction {
public:
   typedef ImageT image_type;

   virtual ~AbstractAction() {}

   virtual ActionOperation op() = 0;

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) = 0;
};

// TODO: these classe, their implementations and 
// their parsing logic should probably be all collocated
template<typename ImageT>
class Intensity : public AbstractAction<ImageT> {
public:
   typedef Intensity<ImageT> ThisT;
private:
   int mAmount;
public:
   Intensity(int amount) : mAmount(amount) {}
   virtual ~Intensity() {}

   virtual ActionOperation op() { return INTENSITY; }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) {
      typename ImageT::ImageViewT tgtview = roi2view(tgt,roi);
      utility::add(roi2view(src,roi),tgtview,mAmount);
   }
};

template<typename ImageT>
class Binarize : public AbstractAction<ImageT> {
public:
   typedef Intensity<ImageT> ThisT;
private:
   unsigned mThreshold;
public:
   Binarize(unsigned threshold) : mThreshold(threshold) {}
   virtual ~Binarize() {}

   virtual ActionOperation op() { return BINARIZE; }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) {
      typename ImageT::ImageViewT tgtview = roi2view(tgt,roi);
      utility::binarize(roi2view(src,roi),tgtview,mThreshold);
   }
};

template<typename ImageT>
class BinarizeDT : public AbstractAction<ImageT> {
public:
   typedef AbstractAction<ImageT> SuperT;
   typedef Intensity<ImageT> ThisT;
private:
   unsigned mThresholdLow;
   unsigned mThresholdHigh;
public:
   BinarizeDT(unsigned thresholdLow,unsigned thresholdHigh) : 
      mThresholdLow(thresholdLow),
      mThresholdHigh(thresholdHigh)
   {}

   virtual ~BinarizeDT() {}

   virtual ActionOperation op() { return BINARIZE; }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) {
      typename ImageT::ImageViewT tgtview = roi2view(tgt,roi);
      utility::binarizeDouble(roi2view(src,roi),tgtview,mThresholdLow,mThresholdHigh);
   }
};


template<typename ImageT>
class BinarizeColor : public AbstractAction<ImageT> {
public:
   typedef AbstractAction<ImageT> SuperT;
   typedef Intensity<ImageT> ThisT;
   typedef typename ImageT::pixel_type pixel_type;
private:
   float mThreshold;
   pixel_type mFromPos;
public:

   BinarizeColor(float threshold,const pixel_type& fromPos) : mThreshold(0), mFromPos(fromPos) {}
   virtual ~BinarizeColor() {}

   virtual ActionOperation op() { return BINARIZE; }

   virtual void run(const ImageT& src,ImageT& tgt,const RegionOfInterest& roi) {
      typename ImageT::ImageViewT tgtview = roi2view(tgt,roi);
      utility::binarizeColor(roi2view(src,roi),tgtview,mThreshold,mFromPos);
   }
};




template<typename AbstractActionT,typename ImageT = typename AbstractActionT::image_type>
class Operation {
private:
   //std::string mInputImage;
   //std::string mOutputImage;
   AbstractActionT* mAction;
   typedef std::vector<RegionOfInterest> RegionsT;
   RegionsT mRegions;

   void operateOnRegions(const ImageT& src,ImageT& tgt) {
      // If no regions defined, assume operation is to operate
      // on entire image
      if(0 == mRegions.size()) {
         mAction->run(src,tgt,view2roi(src.defaultView()));
      }
      // O.w. iterate all of the given regions
      else {
         RegionsT::const_iterator pos = mRegions.begin();
         RegionsT::const_iterator end = mRegions.end();
         for(;pos != end;++pos) {
            mAction->run(src,tgt,*pos);
         }
      }
   }

public:
   Operation(//const std::string& inputfile,
             //const std::string& outputfile,
             AbstractActionT* action) : 
      //mInputImage(inputfile),
      //mOutputImage(outputfile),
      mAction(action) {}

   ~Operation() {
      delete mAction;
      mAction = 0;
   }

   void addRegion(const RegionOfInterest& roi) {
      mRegions.push_back(roi);
   }

   ImageT run(const ImageT& src) {
	   ImageT tgt(src);
      operateOnRegions(src,tgt);
      return tgt;
   }
};


class ParseError : public exception {
private:
   std::string mMessage;
public:
   /** Takes a character string describing the error.  */
   explicit ParseError(const string& msg) : mMessage(msg) {}

   virtual ~ParseError() throw() {}

   virtual const char* what() const throw() { return mMessage.c_str(); }
};


template<typename ReturnT>
ReturnT parseWord(std::stringstream& ss) {
   ReturnT retval;
   ss >> retval;
   if(ss.fail()) throw ParseError("Reading string from stream failed");
   return retval;
}

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

   if(endsWith(inputfile,".pgm")) {
      assert(endsWith(outputfile,".pgm"));

      typedef GrayAlphaPixel<unsigned char> PixelT;
      typedef Image<PixelT> ImageT;
      typedef AbstractAction<ImageT> AbstractActionT;
      typedef Operation<AbstractActionT> OperationT;

      AbstractActionT* action = 0;

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
      typedef AbstractAction<ImageT> AbstractActionT;
      typedef Operation<AbstractActionT> OperationT;

      AbstractActionT* action = 0;

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

