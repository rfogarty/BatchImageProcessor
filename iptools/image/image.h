#ifndef IMAGE_H
#define IMAGE_H

#include <cassert>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

template<typename ChannelT>
struct ColorPixel {
   
   typedef ChannelT Channels[4];
   
   struct RGBA {
      ChannelT red;
      ChannelT green;
      ChannelT blue;
      ChannelT alpha;
      RGBA() :
         red(0),
         green(0),
         blue(0),
         alpha(0)
      {}
   };
   
   union {
      Channels indexedColor;
      RGBA namedColor;
   };

   enum {
      MAX_CHANNELS = 4
   };

   ColorPixel() :
      namedColor()
   {} 
};

template<typename ChannelT>
struct GrayAlphaPixel {
   
   typedef ChannelT Channels[2];
   
   struct GA {
      ChannelT gray;
      ChannelT alpha;
      GA() :
         gray(0),
         alpha(0)
      {}
   };
   
   union {
      Channels indexedColor;
      GA namedColor;
   };

   enum {
      MAX_CHANNELS = 2
   };

   GrayAlphaPixel() :
      namedColor()
   {}
};

template<typename ChannelT>
struct MonochromePixel {
   
   typedef ChannelT Channels[1];
   
   struct M {
      ChannelT mono;
      M() :
         mono(0)
      {}
   };
   
   union {
      Channels indexedColor;
      M        namedColor;
   };

   enum {
      MAX_CHANNELS = 1
   };

   MonochromePixel() :
      namedColor()
   {}
};


//template<typename ChannelT>
//class Pixel {
//public:
//   virtual ~Pixel() {}
//
//   virtual ChannelT getChannel(int n) = 0;
//};

//
//template<typename ChannelT>
//class ColorPixel //: public Pixel<ChannelT> // going to do only static polymorphism to ensure pixel size is optimal
//                                            // and unaffected by vtable
//{
//private:
//   typedef ColorChannel<ChannelT> value_type;
//   value_type data;
//public:
//   //virtual ~ColorPixel() {}
//   
//   //virtual 
//   ChannelT getChannel(unsigned n) {
//      assert(n < value_type::MAX_CHANNELS);
//      return data.indexedColor[n];
//   }
//
//   ChannelT getRed() { return data.namedColor.red; }
//   ChannelT getGreen() { return data.namedColor.green; }
//   ChannelT getBlue() { return data.namedColor.blue; }
//   ChannelT getAlpha() { return data.namedColor.alpha; }
//};
//
//
//template<typename ChannelT>
//class GrayAlphaPixel { //: public Pixel<ChannelT> {
//private:
//   typedef GrayAlphaChannel<ChannelT> value_type;
//   value_type data;
//public:
//   //virtual ~GrayAlphaPixel() {}
//
//   //virtual 
//   ChannelT getChannel(unsigned n) {
//      assert(n < value_type::MAX_CHANNELS);
//      return data.indexedColor[n];
//   }
//
//   ChannelT getGray() { return data.namedColor.gray; }
//   ChannelT getAlpha() { return data.namedColor.alpha; }
//};
//
//
//template<typename ChannelT>
//class MonochromePixel { //: public Pixel<ChannelT> {
//private:
//   typedef MonochromeChannel<ChannelT> value_type;
//   value_type data;
//public:
//   //virtual ~MonochromePixel() {}
//
//   //virtual 
//   ChannelT getChannel(unsigned n) {
//      assert(n < value_type::MAX_CHANNELS);
//      return data.indexedColor[n];
//   }
//
//   ChannelT getMono() { return data.namedColor.mono; }
//};

static const unsigned PRESUMED_CACHELINE_SIZE = 64;

template<typename PixelT>
unsigned computeCacheFriendlyRowSize(unsigned cols) {
   return cols + (PRESUMED_CACHELINE_SIZE - ((sizeof(PixelT)*cols) % PRESUMED_CACHELINE_SIZE))/sizeof(PixelT);
}

template<typename PixelT>
class ImageStore {
private:
   typedef std::vector<PixelT> PixelStore;

   unsigned mAllocatedRows;
   unsigned mAllocatedCols;
   unsigned mPadding;
   PixelStore pixels;
public:
   ImageStore(unsigned rows, unsigned cols,unsigned padding = 0) :
      mAllocatedRows(rows+2*padding),
      // The following assumes images are stored in row-major order
      // which is the normal convention in C/C++
      mAllocatedCols(computeCacheFriendlyRowSize<PixelT>(cols + 2*padding)),
      mPadding(padding),
      pixels(mAllocatedRows*mAllocatedCols)
   {} // rounding up cols here to make row processing
      // always cache friendly (which usually is 64 byte aligned)

   void resize(unsigned rows,unsigned cols,unsigned padding) {
      mPadding = padding;
      mAllocatedRows = rows + 2*padding;
      mAllocatedCols = computeCacheFriendlyRowSize<PixelT>(cols + 2*padding);
      pixels.resize(mAllocatedRows*mAllocatedCols);
   }

   void resize(unsigned rows,unsigned cols) {
      // Assume we are maintaining padding.
      resize(rows,cols,mPadding);
   }

   unsigned rows() const { return mAllocatedRows; }

   unsigned cols() const { return mAllocatedCols; }

   const PixelT& pixel(unsigned row,unsigned col) const {
      // TODO: perhaps use vector operation
      // to do asserting.
      assert(row < mAllocatedRows);
      assert(col < mAllocatedCols);

      unsigned index = mAllocatedCols * row + col;
      return pixels[index];
   }

   PixelT& pixel(unsigned row,unsigned col) {
      typedef ImageStore<PixelT> ThisType;
      return const_cast<PixelT&>(static_cast<const ThisType&>(*this).pixel(row,col));
   }

};


template<typename PixelT,typename ImageStoreT = ImageStore<PixelT> >
class ImageWindow {
private:
   unsigned mRows;
   unsigned mCols;
   unsigned mRowBegin;
   unsigned mColBegin;
   ImageStoreT* mStore;
public:
   //virtual ~ImageArea();
   ImageWindow(unsigned rows,unsigned cols,
               ImageStoreT* store,
               unsigned beginRow = 0, unsigned beginCol = 0) :
      mRows(rows),
      mCols(cols),
      mRowBegin(beginRow),
      mColBegin(beginCol),
      mStore(store)
   {}

   const PixelT& pixel(unsigned row, unsigned col) const {
      return mStore->pixel(row+mRowBegin,col+mColBegin);
   }

   PixelT& pixel(unsigned row, unsigned col) {
      typedef ImageWindow<PixelT,ImageStoreT> ThisType;
      return const_cast<PixelT&>(static_cast<const ThisType&>(*this).pixel(row,col));
   }

   void resize(unsigned rows,unsigned cols) {
      assert(rows <= mStore->rows());
      assert(cols <= mStore->cols());
      mRows = rows;
      mCols = cols;
   }

   unsigned rows() const { return mRows; }

   unsigned cols() const { return mCols; }

   void move(unsigned rowBegin,unsigned colBegin) {
      assert(rowBegin+mRows <= mStore->rows());
      assert(colBegin+mCols <= mStore->cols());
   }

   unsigned rowBegin() const { return mRowBegin; }

   unsigned colBegin() const { return mColBegin; }
};

template<typename PixelT,typename ImageWindowT = ImageWindow<PixelT> >
class ImageViewIterator {
private:
   // This is a pointer in lieu of a reference
   // so that iterators can be properly assignable.
   ImageWindowT* mImageWindow;
   unsigned rowPos;
   unsigned colPos;

public:
   typedef std::ptrdiff_t difference_type;
   typedef PixelT value_type;
   typedef PixelT& reference;
   typedef PixelT* pointer;
   typedef std::forward_iterator_tag iterator_category;

   static ImageViewIterator end(ImageWindowT* window) {
      return ImageViewIterator(window,window->rows());
   }

   ImageViewIterator(ImageWindowT* window,unsigned row = 0) :
      mImageWindow(window),
      rowPos(row),
      colPos(0)
   {}

   const PixelT& operator*() const {
      assert(rowPos < mImageWindow->rows());
      assert(colPos < mImageWindow->cols());
      return mImageWindow->pixel(rowPos,colPos);
   }

   PixelT& operator*() {
      typedef ImageViewIterator<PixelT,ImageWindowT> ThisType;
      return const_cast<PixelT&>(static_cast<const ThisType&>(*this).operator*());
   }

   const PixelT* operator->() const {
      return &operator*();
   }

   PixelT* operator->() {
      typedef ImageViewIterator<PixelT,ImageWindowT> ThisType;
      return &const_cast<PixelT&>(static_cast<const ThisType&>(*this).operator*());
   }

   ImageViewIterator& operator++() {
      // If colPos wraps to zero (aka false)
      // the rowPos will also be evaluated.
      if(0 == (++colPos %= mImageWindow->cols())) ++rowPos;
      return *this;
   }

   ImageViewIterator operator++(int) {
      ImageViewIterator prior(*this);
      ++(*this);
      return prior;
   }


   bool operator==(const ImageViewIterator& that) {
      return rowPos == that.rowPos && colPos == that.colPos;
   }

   bool operator!=(const ImageViewIterator& that) {
      return !(*this == that);
   }
};


template<typename PixelT,typename ImageStoreT = ImageStore<PixelT> >
class ImageView : public ImageWindow<PixelT> {
public:
   typedef ImageWindow<PixelT> ImageWindowT;
   typedef ImageViewIterator<PixelT,ImageWindowT> iterator;
   typedef ImageViewIterator<const PixelT,const ImageWindowT> const_iterator;

   ImageView(unsigned rows,unsigned cols,
             ImageStoreT* store,
             unsigned beginRow = 0, unsigned beginCol = 0) :
      ImageWindowT(rows,cols,store,beginRow,beginCol)
   {}

   iterator begin() { return iterator(this); }
   
   iterator end() { return iterator::end(this); }

   const_iterator begin() const { return const_iterator(this); }
   
   const_iterator end() const { return const_iterator::end(this); }
};


template<typename PixelT>
class Image {
public:
   typedef ImageStore<PixelT> ImageStoreT;
   typedef ImageView<PixelT> ImageViewT;
   typedef typename ImageViewT::iterator iterator;
   typedef typename ImageViewT::const_iterator const_iterator;
private:
   ImageStoreT mStore;
   ImageViewT mDefaultView;
public:
   Image(unsigned rows,unsigned cols,unsigned padding = 0) :
      mStore(rows,cols,padding),
      mDefaultView(rows,cols,&mStore,padding,padding)
   {}

   void resize(unsigned rows,unsigned cols,unsigned padding) {
      mStore.resize(rows,cols,padding);
      mDefaultView.resize(rows,cols);
   }

   void resize(unsigned rows,unsigned cols) {
      mStore.resize(rows,cols);
      mDefaultView.resize(rows,cols);
   }

   unsigned rows() const { return mDefaultView.rows(); }

   unsigned cols() const { return mDefaultView.cols(); }

   const PixelT& pixel(unsigned row, unsigned col) const {
      return mDefaultView.pixel(row,col);
   }

   PixelT& pixel(unsigned row, unsigned col) {
      return mDefaultView.pixel(row,col);
   }

   ImageViewT view(unsigned rows,unsigned cols,
                   unsigned rowBegin = 0,unsigned colBegin = 0) {
      return ImageViewT(rows,cols,&mStore,rowBegin,colBegin);
   }

   iterator begin() { return mDefaultView.begin(); }
   
   iterator end() { return mDefaultView.end(); }

   const_iterator begin() const { return mDefaultView.begin(); }
   
   const_iterator end() const { return mDefaultView.end(); }
};


inline bool endsWith(const std::string& str, const std::string& suffix)
{
   return str.size() >= suffix.size() && 
          0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

// This is a little ugly to try and be resilient to all the PNM formats; 
// this would be much easier in ANTLR!
bool readNext2Integers(std::istream& ins,unsigned& val1,unsigned& val2) {
   // Note: a successful read of val2 implies a successful read of val1
   bool readVal2 = false;
   do {
      std::string line;
      std::getline(ins,line);
      std::stringstream ss(line);
      if(!(ss >> val1).fail()) {
         if(!ss.eof()) readVal2 = !(ss >> val2).fail();
         break;
      }
   } while(ins);

   if(ins && !readVal2)
   do {
      std::string line;
      std::getline(ins,line);
      std::stringstream ss(line);
      if(readVal2 = !(ss >> val2).fail()) break;
   } while(ins);

   return readVal2;
}

void readPNMHeader(std::istream& ifs,const std::string& filename,unsigned& rows, unsigned& cols,const char* pnmTypeID,const char* extType) {
   std::string str;
   std::getline(ifs, str);

   if ((str.substr(0,2) != pnmTypeID)) {
      std::stringstream ss;
      ss << "Image file \"" << filename << "\" not in " << extType << " format..." << str;
      throw std::invalid_argument(ss.str().c_str());
   }

   /* read the next two ints in a file to grab the rows and cols*/
   if(!readNext2Integers(ifs,cols,rows)) {
      std::stringstream ss;
      ss << "Image file \"" << filename << "\" size could not be deduced perhaps bad PNM format...";
      throw std::invalid_argument(ss.str().c_str());
   }

   // This bit seems a little troublesome (especially if there are extra comment lines)
   std::string line;
   std::getline(ifs,line);
}


template<typename PixelT> 
Image<PixelT> readPPMFile(const std::string& filename) {
   /* PPM Color Image */
   if (!endsWith(filename, ".ppm"))
      throw std::invalid_argument("Incorrect File Type, expecting .ppm");

   std::fstream pgm_file;
   pgm_file.open(filename.c_str(), std::ios_base::in | std::ios_base::binary);
   if (!pgm_file.is_open()) {
      std::stringstream ss;
      ss << "Filename \"" << filename << "\" could not be opened correctly";
      throw std::invalid_argument(ss.str().c_str());
   }

   unsigned cols = 0, rows = 0;
   readPNMHeader(pgm_file,filename,rows,cols,"P6","PPM");

   // Read color bytes
   unsigned byteSize = rows*cols*3;
   typedef std::vector<unsigned char> BufferT;
   BufferT buffer(byteSize);
   pgm_file.read(reinterpret_cast<char*>(&buffer[0]), byteSize);

   // Copy all the image buffer data into the Image<PixelT> object
   Image<PixelT> image(rows,cols);
   BufferT::const_iterator pos = buffer.begin();
   for(unsigned r = 0; r < rows; ++r) {
      for(unsigned c = 0; c < cols;++c) {
         PixelT pixel;
         pixel.namedColor.red = *pos;
         ++pos;
         pixel.namedColor.green = *pos;
         ++pos;
         pixel.namedColor.blue = *pos;
         ++pos;
         image.pixel(r,c) = pixel;
      }
   }

   pgm_file.close();

   return image;
}

template<typename PixelT>
Image<PixelT> readPGMFile(const std::string& filename) {
   /* PGM Gray-level Image */ 
   if (!endsWith(filename, ".pgm"))
      throw std::invalid_argument("Incorrect File Type, expecting .pgm");

   std::fstream pgm_file;

   pgm_file.open(filename.c_str(), std::ios_base::in | std::ios_base::binary);
   if (!pgm_file.is_open()) {
      std::stringstream ss;
      ss << "Filename \"" << filename << "\" could not be opened correctly";
      throw std::invalid_argument(ss.str().c_str());
   }

   unsigned cols = 0, rows = 0;
   readPNMHeader(pgm_file,filename,rows,cols,"P5","PPM");

   // Read grayscale bytes
   unsigned byteSize = rows*cols;
   typedef std::vector<unsigned char> BufferT;
   BufferT buffer(byteSize);
   pgm_file.read(reinterpret_cast<char*>(&buffer[0]), byteSize);
   pgm_file.close();

   // Copy all the image buffer data into the Image<PixelT> object
   Image<PixelT> image(rows,cols);
   BufferT::const_iterator pos = buffer.begin();
   for(unsigned r = 0; r < rows; ++r) {
      for(unsigned c = 0; c < cols;++c) {
         PixelT pixel;
         pixel.namedColor.gray = *pos;
         ++pos;
         image.pixel(r,c) = pixel;
      }
   }

   return image;
}


template<typename PixelT>
void writePPMFile(const std::string& filename,const Image<PixelT>& image) {
   if (!endsWith(filename, ".ppm"))
      throw std::invalid_argument("Incorrect file extension, expecting .ppm");

   std::ofstream outfile;
   outfile.open(filename.c_str(), std::ios_base::out | std::ios_base::binary);
   if (!outfile.is_open()) {
      std::stringstream ss;
      ss << "Filename \"" << filename << "\" could not write to correctly";
      throw std::invalid_argument(ss.str().c_str());
   }

   outfile << "P6\n";
   outfile << image.cols() << " " << image.rows() << "\n";
   outfile << "255\n";

   typename Image<PixelT>::const_iterator pos = image.begin();
   typename Image<PixelT>::const_iterator end = image.end();

   for(;pos != end;++pos) {
      // TODO: note Pixel ChannelT in this type has to be unsigned char
      // At some point may need to have Pixel conversion functions.
      outfile << pos->namedColors.red;
      outfile << pos->namedColors.blue;
      outfile << pos->namedColors.green;
   }

   outfile.close();
}

template<typename PixelT>
void writePGMFile(const std::string& filename,const Image<PixelT>& image) {
   if (!endsWith(filename, ".pgm"))
      throw std::invalid_argument("Incorrect file extension, expecting .pgm");

   std::fstream outfile;
   outfile.open(filename.c_str(), std::ios_base::out | std::ios_base::binary);
   if (!outfile.is_open()) {
      std::stringstream ss;
      ss << "Filename \"" << filename << "\" could not write to correctly";
      throw std::invalid_argument(ss.str().c_str());
   }

   outfile << "P5\n";
   outfile << image.cols() << " " << image.rows() << "\n";
   outfile << "255\n";

   typename Image<PixelT>::const_iterator pos = image.begin();
   typename Image<PixelT>::const_iterator end = image.end();

   for(;pos != end;++pos) {
      // TODO: note Pixel ChannelT in this type has to be unsigned char
      // At some point may need to have Pixel conversion functions.
      outfile << pos->namedColor.gray;
   }

   outfile.close();
}


#endif

