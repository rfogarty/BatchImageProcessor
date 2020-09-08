#pragma once

#include <cassert>
#include <iterator>
#include <vector>

template<typename ChannelT>
struct ColorPixel {
   
   typedef ChannelT Channels[4];
   
   struct RGBA {
      ChannelT red;
      ChannelT green;
      ChannelT blue;
      ChannelT alpha;
#if __cplusplus >= 201103L
      RGBA() :
         red(0),
         green(0),
         blue(0),
         alpha(0)
      {}
#endif
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
#if __cplusplus >= 201103L
      GA() :
         gray(0),
         alpha(0)
      {}
#endif
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
#if __cplusplus >= 201103L
      M() :
         mono(0)
      {}
#endif
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
protected:
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

   ImageViewIterator(ImageWindowT* window,unsigned row = 0) :
      mImageWindow(window),
      rowPos(row),
      colPos(0)
   {}

public:
   typedef std::ptrdiff_t difference_type;
   typedef PixelT value_type;
   typedef PixelT& reference;
   typedef PixelT* pointer;
   typedef std::forward_iterator_tag iterator_category;

   static ImageViewIterator begin(ImageWindowT* window) {
      return ImageViewIterator(window);
   }

   static ImageViewIterator end(ImageWindowT* window) {
      return ImageViewIterator(window,window->rows());
   }

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

   ImageView subview(unsigned rows,unsigned cols,
                     unsigned rowBegin = 0,unsigned colBegin = 0) {
      assert(rowBegin+rows <= this->mRows);
      assert(colBegin+cols <= this->mCols);
      return ImageView(rows,cols,this->mStore,rowBegin+this->mRowBegin,colBegin+this->mColBegin);
   }

   iterator begin() { return iterator::begin(this); }
   
   iterator end() { return iterator::end(this); }

   const_iterator begin() const { return const_iterator::begin(this); }
   
   const_iterator end() const { return const_iterator::end(this); }
};


// Image - an image type composed of a backend ImageStore and
//         a default ImageView.
//
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



