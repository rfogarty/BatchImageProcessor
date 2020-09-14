#pragma once

#include "cppTools/TemplateMetaprogramming.h"
#include "utility/Error.h"
#include <iterator>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
// ImageBounds - abstract base class that ImageStore and all ImageView
//                derive. Its data is used when taking "sub-views".
//
template<typename PixelT>
class ImageBounds {
public:
   virtual ~ImageBounds() {}
   virtual unsigned rows() const = 0;
   virtual unsigned cols() const = 0;
   virtual unsigned rowBegin() const = 0;
   virtual unsigned colBegin() const = 0;

   std::size_t pixelSize() { return sizeof(PixelT); }
};

///////////////////////////////////////////////////////////////////////////////
// ImageStore - class that manages the memory for an Image.
//
// Notes:
// 1) Data is stored in Row-Major order (conventional C/C++ order).
// 2) Rows are inflated to be cache-aligned for optimization purposes.
//    This means that multithreads can operate optimally on rows independently.
// 4) Padding may be added to an image in case algorithms need to operate in
//    a window or view on the boundary of an Image.
// 3) Actual size of the image is not stored within this class, as that data
//    is not needed.
//
template<typename PixelT>
class ImageStore : public ImageBounds<PixelT> {
public:
   typedef typename std::remove_const<PixelT>::type pixel_type;
   typedef ImageStore<PixelT> this_type;

private:
   typedef std::vector<pixel_type> pixel_store;

   unsigned mAllocatedRows;
   unsigned mAllocatedCols;
   unsigned mPadding;
   pixel_store pixels;

   static const unsigned PRESUMED_CACHELINE_SIZE = 64;
   
   unsigned computeCacheFriendlyRowSize(unsigned cols) {
      return cols + (PRESUMED_CACHELINE_SIZE - ((sizeof(PixelT)*cols) % PRESUMED_CACHELINE_SIZE))/sizeof(PixelT);
   }

public:
   ImageStore(unsigned rows, unsigned cols,unsigned padding = 0) :
      mAllocatedRows(rows+2*padding),
      // The following assumes images are stored in row-major order
      // which is the normal convention in C/C++
      mAllocatedCols(computeCacheFriendlyRowSize(cols + 2*padding)),
      mPadding(padding),
      pixels(mAllocatedRows*mAllocatedCols)
   {} // rounding up cols here to make row processing
      // always cache friendly (which usually is 64 byte aligned)

   void resize(unsigned rows,unsigned cols,unsigned padding) {
      mPadding = padding;
      mAllocatedRows = rows + 2*padding;
      mAllocatedCols = computeCacheFriendlyRowSize(cols + 2*padding);
      pixels.resize(mAllocatedRows*mAllocatedCols);
   }

   void resize(unsigned rows,unsigned cols) {
      // Assume we are maintaining padding.
      resize(rows,cols,mPadding);
   }

   unsigned rows() const { return mAllocatedRows; }

   unsigned cols() const { return mAllocatedCols; }

   unsigned rowBegin() const { return 0u; }

   unsigned colBegin() const { return 0u; }

   unsigned padding() const { return mPadding; }

   const pixel_type& pixel(unsigned row,unsigned col) const {
      reportIfNotLessThan("row",row,mAllocatedRows);
      reportIfNotLessThan("col",col,mAllocatedCols);
      unsigned index = mAllocatedCols * row + col;
      return pixels[index];
   }

   pixel_type& pixel(unsigned row,unsigned col) {
      return const_cast<pixel_type&>(static_cast<const this_type&>(*this).pixel(row,col));
   }
};


///////////////////////////////////////////////////////////////////////////////
// ImageWindow - base class for setting up a view of Image data.
//
// This class cannot be constructed directly but rather is intended to be
// used as a base class. Its primary derivative is ImageView.
//
template<typename PixelT,
         typename ImageStoreT  = ImageStore<typename std::remove_const<PixelT>::type>,
         typename ImageBoundsT = ImageBounds<typename std::remove_const<PixelT>::type> >
class ImageWindow : public ImageBounds<PixelT> {
public:
   typedef typename std::remove_const<PixelT>::type pixel_type;
   typedef ImageWindow<PixelT,ImageStoreT>          this_type;
   typedef ImageStoreT                              image_store;
   typedef ImageBoundsT                             image_bounds;

protected:
   unsigned      mRows;
   unsigned      mCols;
   unsigned      mSize; // For optimization, compute only when resized
   unsigned      mRowBegin;
   unsigned      mColBegin;
   image_store*  mStore;
   image_bounds* mBounds;


   // Constructor is protected because this class is only meant to be
   // a base class.
   ImageWindow(unsigned rows,unsigned cols,
               image_store* store,
               image_bounds* bounds,
               unsigned rowBegin = 0, unsigned colBegin = 0) :
      mRows(rows),
      mCols(cols),
      mSize(rows*cols),
      mRowBegin(rowBegin),
      mColBegin(colBegin),
      mStore(store),
      mBounds(bounds) {
      // Ensure that this ImageWindow is valid, and throw exception if not
      reportIfNotLessThan("rowBegin+rows",rowBegin + rows,mBounds->rows()+1);
      reportIfNotLessThan("colBegin+cols",colBegin + cols,mBounds->cols()+1);
   }

   // Copy-constructible only via derivatives
   ImageWindow(const ImageWindow& that) :
      mRows(that.mRows),
      mCols(that.mCols),
      mSize(mRows*mCols),
      mRowBegin(that.mRowBegin),
      mColBegin(that.mColBegin),
      mStore(that.mStore),
      mBounds(that.mBounds) {
      // Note: shouldn't need to check invariants here because all resize/move
      // operations ensure invariants are never violated.
   }

private:
   // ImageWindows themselves are not assignable, however,
   // their derivatives can be.
   ImageWindow& operator=(const ImageWindow&); // deleted

public:
   const PixelT& pixel(unsigned row, unsigned col) const { return mStore->pixel(row+mRowBegin,col+mColBegin); }

   PixelT& pixel(unsigned row, unsigned col) {
      // Note: if ImageWindow PixelT is a const type, the below const_cast is a no-op.
      return const_cast<PixelT&>(static_cast<const this_type&>(*this).pixel(row,col));
   }

   // In some cases (such as when assigning one image to another)
   // it is not safe to perform resize and move as separate functions.
   // Therefore, this function should be called in such a case.
   void resizeAndMove(unsigned rows,unsigned cols,unsigned rowBegin,unsigned colBegin) {
      reportIfNotLessThan("rowBegin+mRows",rowBegin + rows,mBounds->rows()+1);
      reportIfNotLessThan("colBegin+mCols",colBegin + cols,mBounds->cols()+1);
      mRows = rows;
      mCols = cols;
      mSize = rows*cols;
      mRowBegin = rowBegin + mBounds->rowBegin();
      mColBegin = colBegin + mBounds->colBegin();
   }

   void resize(unsigned rows,unsigned cols) {
      reportIfNotLessThan("rows",mRowBegin + rows,mBounds->rows()+1);
      reportIfNotLessThan("cols",mColBegin + cols,mBounds->cols()+1);
      mRows = rows;
      mCols = cols;
      mSize = rows*cols;
   }

   unsigned rows() const { return mRows; }

   unsigned cols() const { return mCols; }

   unsigned size() const { return mSize; }

   unsigned rowBegin() const { return mRowBegin; }

   unsigned colBegin() const { return mColBegin; }

   void move(unsigned rowBegin,unsigned colBegin) {
      reportIfNotLessThan("rowBegin+mRows",rowBegin + mRows,mBounds->rows()+1);
      reportIfNotLessThan("colBegin+mCols",colBegin + mCols,mBounds->cols()+1);
      mRowBegin = rowBegin + mBounds->rowBegin();
      mColBegin = colBegin + mBounds->colBegin();
   }
};

///////////////////////////////////////////////////////////////////////////////
// ImageViewIterator - a forward iterator and input and output iterator.
//
// Iterates the ImageView from which it is requested. Iteration order
// is as expected, across (columsn) then down (rows).
//
template<typename PixelT,typename ImageWindowT = ImageWindow<PixelT> >
class ImageViewIterator {
public:
   typedef ImageViewIterator<PixelT,ImageWindowT> this_type;
   typedef PixelT                                 value_type;
   typedef PixelT&                                reference;
   typedef PixelT*                                pointer;
   typedef std::forward_iterator_tag              iterator_category;

private:
   // This is a pointer in lieu of a reference
   // so that ImageViewIterators are assignable.
   ImageWindowT* mImageWindow;
   unsigned      rowPos;
   unsigned      colPos;

   ImageViewIterator(ImageWindowT* window,unsigned row = 0) :
      mImageWindow(window),
      rowPos(row),
      colPos(0)
   {}

public:
   // ImageViewIterators must be constructed via these static begin/end functions or copy-constructed.
   static ImageViewIterator begin(ImageWindowT* window) { return ImageViewIterator(window); }
   static ImageViewIterator end(ImageWindowT* window) { return ImageViewIterator(window,window->rows()); }

   const PixelT& operator*() const {
      reportIfNotLessThan("rowPos",rowPos,mImageWindow->rows());
      reportIfNotLessThan("colPos",colPos,mImageWindow->cols());
      return mImageWindow->pixel(rowPos,colPos);
   }

   PixelT& operator*() { return const_cast<PixelT&>(static_cast<const this_type&>(*this).operator*()); }

   const PixelT* operator->() const { return &operator*(); }

   PixelT* operator->() { return &const_cast<PixelT&>(static_cast<const this_type&>(*this).operator*()); }

   ImageViewIterator& operator++() {
      // Each time colPos wraps to zero
      // the rowPos will be incremented.
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


///////////////////////////////////////////////////////////////////////////////
// ImageView - a view of an Image is a bounded box that is valid anywhere
//             within the bounds of an image, and is useful to operate on
//             regions of an Image.
//
// Supports:
// 1) Iteration of the ImageView.
// 2) Subviews (views of views).
// 3) Indexed access to pixels for read or write.
//
template<typename PixelT,
         typename ImageStoreT  = ImageStore<typename std::remove_const<PixelT>::type>,
         typename ImageBoundsT = ImageBounds<typename std::remove_const<PixelT>::type> >
class ImageView : public ImageWindow<PixelT> {
public:
   typedef typename std::remove_const<PixelT>::type               pixel_type;
   typedef ImageStoreT                                            image_store;
   typedef ImageBoundsT                                           image_bounds;
   typedef ImageWindow<PixelT>                                    image_window;
   typedef ImageViewIterator<PixelT,image_window>                 iterator;
   typedef ImageViewIterator<const pixel_type,const image_window> const_iterator;

   ImageView(unsigned rows,unsigned cols,
             image_store* store,
             image_bounds* bounds,
             unsigned rowBegin = 0, unsigned colBegin = 0) :
      image_window(rows,cols,store,bounds,rowBegin,colBegin)
   {}

   ImageView& operator=(const ImageView& that) {
      if(this != &that) {
         reportIfNotEqual("mRows",this->mRows,that.mRows);
         reportIfNotEqual("mCols",this->mCols,that.mCols);
         iterator tpos = begin();
         iterator tend = end();
         const_iterator spos = that.begin();
         for(;tpos != tend;++tpos,++spos) *tpos = *spos;
      }
      return *this;
   }

   template<typename ImageViewT>
   ImageView& operator=(const ImageViewT& that) {
      if(this != &that) {
         reportIfNotEqual("mRows",this->mRows,that.mRows);
         reportIfNotEqual("mCols",this->mCols,that.mCols);
         iterator tpos = begin();
         iterator tend = end();
         typename ImageViewT::const_iterator spos = that.begin();
         // The below allows, conversion between two image pixel types.
         // However, the Pixels must be implicitly convertible.
         for(;tpos != tend;++tpos,++spos) *tpos = *spos;
      }
      return *this;
   }

   // take a subview of the view
   ImageView view(unsigned rows,unsigned cols,
                  unsigned rowBegin = 0,unsigned colBegin = 0) {
      return ImageView(rows,cols,this->mStore,this,
                       rowBegin+this->mRowBegin,colBegin+this->mColBegin);
   }

   iterator begin() { return iterator::begin(this); }
   
   iterator end() { return iterator::end(this); }

   const_iterator begin() const { return const_iterator::begin(this); }
   
   const_iterator end() const { return const_iterator::end(this); }
};

#if 0
template<typename PixelT,typename ImageStoreT = ImageStore<typename std::remove_const<PixelT>::type> >
class ElasticImageView //: public ImageWindow<PixelT,ImageStoreT> {
private:
   unsigned mMaxRows; // The largest size rows our elastic view can grow
   unsigned mMaxCols; // The largest size cols our elastic view can grow
   unsigned mRowBegin;
   unsigned mColBegin;
public:
   typedef ImageView<PixelT,ImageStoreT> super_type;
   using super_type::pixel_type;
   using super_type::image_store;
   using super_type::image_window;

//   template<typename ElasticImageViewT>
//   ImageView& operator=(const ElasticImageViewT& that) {
//      if(this != &that) {
//         static_cast<super_type&>(*this) = static_cast<typename ElasticImageViewT::super_type&>(that);
//      }
//      return *this;
//   }

   template<typename DepartedScopeAction,typename EnteredScopeAction>
   void moveRight(const DepartedScopeAction& departed,const EnteredScopeAction& entered) {
      // Implicitly need to know 
      //    bounds (original)
      //    current position
      //    window size
      //    current window size
      reportIfNotLessThan("cols",mColBegin+1,this->mMaxCols+1);
      
   }

   template<typename DepartedScopeAction,typename EnteredScopeAction>
   void moveDown(const DepartedScopeAction& departed,const EnteredScopeAction& entered) {
   }
};
#endif

///////////////////////////////////////////////////////////////////////////////
// Image - a generic image type composed of a backend ImageStore and
//         a default ImageView.
//
// Images may be specified with whatever Pixel type is desired,
// thus Images themselves are both agnostic and unaware of the
// underlying memory model for Pixels.
//
// Capabilities:
// 1) Images may be constructed, copy-constructed, and assigned from
//    like-type Images, or unlike-type Images if their Pixel types
//    are convertible.
// 2) Views within an Image are the standard mechanism to modify or
//    operate on an Image. Views are themselves assignable and iterable.
// 3) Every Image owns a "default" view, which is a view of the entire
//    Image.
//
template<typename PixelT>
class Image {
public:
   typedef typename std::remove_const<PixelT>::type pixel_type;
   typedef ImageStore<pixel_type>                   image_store;
   typedef ImageBounds<pixel_type>                  image_bounds;
   typedef ImageView<pixel_type>                    image_view;
   typedef const ImageView<const pixel_type>        const_image_view;
   typedef typename image_view::iterator            iterator;
   typedef typename image_view::const_iterator      const_iterator;

private:
   image_store mStore;
   image_view  mDefaultView;

public:
   Image(unsigned rows,unsigned cols,unsigned padding = 0) :
      mStore(rows,cols,padding),
      mDefaultView(rows,cols,&mStore,&mStore,padding,padding)
   {}

   // Need to override copy-constructor and assignment operator
   // to ensure mDefaultView points to the proper ImageStoreT
   Image(const Image& that) :
      mStore(that.mStore),
      mDefaultView(that.rows(),that.cols(),&mStore,&mStore,mStore.padding(),mStore.padding())
   {}

   Image& operator=(const Image& that) {
      if(this != &that) {
         // Note: that the ImageStore assignment is what copies all of the data
         mStore = that.mStore;
         mDefaultView.resizeAndMove(that.rows(),that.cols(),that.padding(),that.padding());
      }
      return *this;
   }

   template<typename ImageT>
   Image& operator=(const ImageT& that) {
      if(this != &that) {
         resize(that.rows(),that.cols(),that.padding());
         iterator tpos = begin();
         iterator tend = end();
         typename ImageT::const_iterator spos;
         // The below allows, conversion between two image pixel types.
         // However, the Pixels must be implicitly convertible.
         for(;tpos != tend;++tpos,++spos) *tpos = *spos;
      }
      return *this;
   }
  
   // ***************************************************************************
   // Note: despite the rule of three, in this case we don't need a destructor...
   // ***************************************************************************

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

   unsigned padding() const { return mStore.padding(); }

   const PixelT& pixel(unsigned row, unsigned col) const { return mDefaultView.pixel(row,col); }

   PixelT& pixel(unsigned row, unsigned col) { return mDefaultView.pixel(row,col); }

   const image_view& defaultView() const { return mDefaultView; }

   const_image_view view(unsigned rows,unsigned cols,
                         unsigned rowBegin = 0,unsigned colBegin = 0) const {
      return const_image_view(rows,cols,
                             // Although Views can be made const, it does not make sense to have
                             // a const ImageStore or ImageBounds type.
                             const_cast<typename std::remove_const<image_store>::type*>(&mStore),
                             const_cast<typename std::remove_const<image_bounds>::type*>(static_cast<const image_bounds*>(&mStore)),
                             rowBegin,colBegin);
   }

   image_view view(unsigned rows,unsigned cols,
                   unsigned rowBegin = 0,unsigned colBegin = 0) {
      return image_view(rows,cols,&mStore,&mStore,rowBegin,colBegin);
   }

   iterator begin() { return mDefaultView.begin(); }
   
   iterator end() { return mDefaultView.end(); }

   const_iterator begin() const { return mDefaultView.begin(); }
   
   const_iterator end() const { return mDefaultView.end(); }
};



