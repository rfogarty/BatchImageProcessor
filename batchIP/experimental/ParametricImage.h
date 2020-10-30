#pragma once

#include "cppTools/TemplateMetaprogramming.h"
#include "Image.h"

namespace batchIP {
namespace types {


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
template<typename PixelT,
      typename ImageBoundsT = ImageBounds<typename std::remove_const<PixelT>::type> >
class ParametricImageStore : public ImageBoundsT {
public:
   typedef typename std::remove_const<PixelT>::type pixel_type;
   typedef ParametricImageStore<PixelT> this_type;

private:
   // TODO: depending on what we want to do, we may want to make
   // this a list to allow for easy insertion and deletion
   // of values. Perhaps we could also make each row a list
   // and store the rows in a vector. We will let it be
   // for now.
   typedef std::vector<pixel_type> pixel_store;

   unsigned mRows;
   unsigned mCols;
   pixel_store pixels;

public:
   ParametricImageStore(unsigned rows, unsigned cols) :
         mRows(rows),
         // The following assumes images are stored in row-major order
         // which is the normal convention in C/C++
         mCols(cols)
   {}

   void resize(unsigned rows,unsigned cols) {
      mRows = rows;
      mCols = cols;
      // TODO: Now should we filter out the pixels to only
      // those that reside in the image?
   }

   virtual unsigned rows() const { return mRows; }

   virtual unsigned cols() const { return mCols; }

   virtual unsigned rowBegin() const { return 0u; }

   virtual unsigned colBegin() const { return 0u; }

};

// Prototype for Image class
template<typename PixelT> class ParametricImage;
template<typename PixelT,typename ImageStoreT,typename ImageBoundsT> class ParametricImageView;

///////////////////////////////////////////////////////////////////////////////
// ImageWindow - base class for setting up a view of Image data.
//
// This class cannot be constructed directly but rather is intended to be
// used as a base class. Its primary derivative is ImageView.
//
template<typename PixelT,
      typename ImageStoreT  = ParametricImageStore<typename std::remove_const<PixelT>::type>,
      typename ImageBoundsT = ImageBounds<typename std::remove_const<PixelT>::type> >
class ParametricImageWindow : public ImageBoundsT {
   // Images are friends of ImageWindows, to allow for Image assignment
   //   which requires the private update of bounds.
   template<typename PixelTT> friend class ParametricImage;
   template<typename T,typename U,typename V> friend class ParametricImageWindow;

public:
   typedef typename std::remove_const<PixelT>::type pixel_type;
   typedef ParametricImageWindow<PixelT,ImageStoreT>          this_type;
   typedef ImageStoreT                              image_store;
   typedef ImageBoundsT                             image_bounds;
   typedef SavedImageBounds<pixel_type>             saved_image_bounds;

protected:
   unsigned           mRows;
   unsigned           mCols;
   unsigned           mSize; // For optimization, compute only when resized
   unsigned           mRowBegin;
   unsigned           mColBegin;
   image_store*       mStore;
   saved_image_bounds mBounds;


   // Constructor is protected because this class is only meant to be
   // a base class.
   ParametricImageWindow(unsigned rows,unsigned cols,
                         image_store* store,
                         image_bounds* bounds,
                         unsigned rowPos = 0, unsigned colPos = 0) :
         mRows(rows),
         mCols(cols),
         mSize(rows*cols),
         mRowBegin(rowPos+bounds->rowBegin()),
         mColBegin(colPos+bounds->colBegin()),
         mStore(store),
         mBounds(*bounds) {
      // Ensure that this ImageWindow is valid, and throw exception if not
      utility::reportIfNotLessThan("rowPos+rows",rowPos + rows,mBounds.rows()+1);
      utility::reportIfNotLessThan("colPos+cols",colPos + cols,mBounds.cols()+1);
   }

   // Copy-constructible only via derivatives
   ParametricImageWindow(const ParametricImageWindow& that) :
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

   // This conversion constructor is used to convert between non-const
   // and const versions of PixelT.
   template<typename ParametricImageWindowT>
   explicit ParametricImageWindow(const ParametricImageWindowT& that) :
         mRows(that.rows()),
         mCols(that.cols()),
         mSize(mRows*mCols),
         mRowBegin(that.rowBegin()),
         mColBegin(that.colBegin()),
         mStore(that.mStore),
         mBounds(that.mBounds) {
      // Note: shouldn't need to check invariants here because all resize/move
      // operations ensure invariants are never violated.
   }

private:
   // ImageWindows themselves are not assignable, however,
   // their derivatives can be.
   ParametricImageWindow& operator=(const ParametricImageWindow&); // deleted

   void updateBounds(const image_bounds& newBounds) {
      mBounds = saved_image_bounds(newBounds);
   }

public:

   // In some cases (such as when assigning one image to another)
   // it is not safe to perform resize and move as separate functions.
   // Therefore, this function should be called in such a case.
   void resizeAndMove(unsigned rows,unsigned cols,unsigned rowPos,unsigned colPos) {
      utility::reportIfNotLessThan("rowBegin+mRows",rowPos + rows,mBounds.rowEnd()+1);
      utility::reportIfNotLessThan("colBegin+mCols",colPos + cols,mBounds.colEnd()+1);
      mRows = rows;
      mCols = cols;
      mSize = rows*cols;
      mRowBegin = rowPos + mBounds.rowBegin();
      mColBegin = colPos + mBounds.colBegin();
   }

   void resize(unsigned rows,unsigned cols) {
      utility::reportIfNotLessThan("rows",mRowBegin + rows,mBounds.rowEnd()+1);
      utility::reportIfNotLessThan("cols",mColBegin + cols,mBounds.colEnd()+1);
      mRows = rows;
      mCols = cols;
      mSize = rows*cols;
   }

   virtual unsigned rows() const { return mRows; }

   virtual unsigned cols() const { return mCols; }

   unsigned size() const { return mSize; }

   virtual unsigned rowBegin() const { return mRowBegin; }

   virtual unsigned colBegin() const { return mColBegin; }

   // TODO: not sure if it makes sense to support these yet. If needed, they
   // may need to be able to actively filter the pixels in the set (either that
   // or we never prune that set and let the iterators lazily filter them).
//   void move(unsigned rowPos,unsigned colPos) {
//      utility::reportIfNotLessThan("rowBegin+mRows",rowPos + mRows,mBounds.rowEnd()+1);
//      utility::reportIfNotLessThan("colBegin+mCols",colPos + mCols,mBounds.colEnd()+1);
//      mRowBegin = rowPos + mBounds.rowBegin();
//      mColBegin = colPos + mBounds.colBegin();
//   }
//
//   void shiftCol() {
//      unsigned colPos = mColBegin + 1;
//      utility::reportIfNotLessThan("colBegin+mCols",colPos + mCols,mBounds.colEnd()+1);
//      mColBegin = colPos;
//   }
//
//   void shiftRow() {
//      unsigned rowPos = mRowBegin + 1;
//      utility::reportIfNotLessThan("rowBegin+mRows",rowPos + mRows,mBounds.rowEnd()+1);
//      mRowBegin = rowPos;
//   }
};


///////////////////////////////////////////////////////////////////////////////
// ImageViewIterator - a forward iterator and input and output iterator.
//
// Iterates the ImageView from which it is requested. Iteration order
// is as expected, across (columns) then down (rows).
//
template<typename PixelT,typename ImageWindowT = ParametricImageWindow<PixelT> >
class ParametricImageViewIterator {
   template<typename PixelTT,typename ImageWindowTT> friend class ParametricImageViewIterator;
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

   explicit ParametricImageViewIterator(ImageWindowT* window,unsigned row = 0) :
         mImageWindow(window),
         rowPos(row),
         colPos(0)
   {}

public:
   // ImageViewIterators must be constructed via these static begin/end functions or copy-constructed.
   static ParametricImageViewIterator begin(ImageWindowT* window) { return ParametricImageViewIterator(window); }
   static ParametricImageViewIterator end(ImageWindowT* window) { return ParametricImageViewIterator(window,window->rows()); }

   ParametricImageViewIterator(const ParametricImageViewIterator& that) :
         mImageWindow(that.mImageWindow),
         rowPos(that.rowPos),
         colPos(that.colPos)
   {}

   template<typename PixelTT,typename ImageWindowTT>
   explicit ParametricImageViewIterator(const ParametricImageViewIterator<PixelTT,ImageWindowTT>& that) :
         mImageWindow(that.mImageWindow),
         rowPos(that.rowPos),
         colPos(that.colPos)
   {}

   ParametricImageViewIterator& operator=(const ParametricImageViewIterator& that) {
      if(this != &that) {
         this->mImageWindow = that.mImageWindow;
         this->rowPos = that.rowPos;
         this->colPos = that.colPos;
      }
      return *this;
   }

   template<typename PixelTT,typename ImageWindowTT>
   ParametricImageViewIterator& operator=(const ParametricImageViewIterator<PixelTT,ImageWindowTT>& that) {
      if(this != &that) {
         this->mImageWindow = that.mImageWindow;
         this->rowPos = that.rowPos;
         this->colPos = that.colPos;
      }
      return *this;
   }

   const PixelT& operator*() const {
      utility::reportIfNotLessThan("rowPos",rowPos,mImageWindow->rows());
      utility::reportIfNotLessThan("colPos",colPos,mImageWindow->cols());
      return mImageWindow->pixel(rowPos,colPos);
   }

   PixelT& operator*() { return const_cast<PixelT&>(static_cast<const this_type&>(*this).operator*()); }

   const PixelT* operator->() const { return &operator*(); }

   PixelT* operator->() { return &const_cast<PixelT&>(static_cast<const this_type&>(*this).operator*()); }

   ParametricImageViewIterator& operator++() {
      // Each time colPos wraps to zero
      // the rowPos will be incremented.
      if(0 == (++colPos %= mImageWindow->cols())) ++rowPos;
      return *this;
   }

   ParametricImageViewIterator operator++(int) {
      ParametricImageViewIterator prior(*this);
      ++(*this);
      return prior;
   }

   bool operator==(const ParametricImageViewIterator& that) {
      return rowPos == that.rowPos && colPos == that.colPos;
   }

   bool operator!=(const ParametricImageViewIterator& that) {
      return !(*this == that);
   }
};




//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// Experimental...
// Some ideas for a ParametricImage
//   * ParametricImages should be assignable to regular Images and ImageViews
//      ** Design decision: what happens when image size isn't the same size as ParametricImage
//      ** ImageViews should not be resizable (as always), so Parametric Images should be able
//         to completely write into the ImageView space.
//      ** We may need to support two types of assignment. One where we first zero out all
//         pixels, or secondly, where we only assign on the ParametricPixels
//      ** ParametricPixel types should thus be assignable to non-parametric Pixel types for
//         ease of implementation.
//  * ParametricImages should support ParametricImageViews - the PIVs can constrain the bounds
//     of a Parametric Image (and simply filter which pixels are considered as part of that view
//     A result of this is the number of PIV pixels will not be known without iterating them.
//  * PIVs should also support a ParametricViewIterator which iterates the PIV.
//  * As many algorithms as possible should operate on iterators instead of Pixel indexing to implicitly
//    support both non-parametric Images and ParametricImages (and their views).
//  * Note ParametricImages will NOT support Pixel indexing, e.g. image.pixel(i,j), as that does not
//    make sense for a parametric set of pixels (search is difficult, and data is sparse).
//  * ElasticImageViews are not supported as those are specifically used for Local-processing operations
//    such as moving average (justificaton: sparse sets of data aren't appropriate).
template<typename PixelT>
class ParametricImage {
public:
   typedef typename std::remove_const<PixelT>::type pixel_type;
   typedef ImageStore<pixel_type>                   image_store;
   typedef ImageBounds<pixel_type>                  image_bounds;
   typedef ImageView<pixel_type>                    image_view;
   typedef const ImageView<const pixel_type>        const_image_view;
   typedef ElasticImageView<pixel_type>             elastic_image_view;
   typedef ElasticImageView<const pixel_type>       const_elastic_image_view;
   typedef typename image_view::iterator            iterator;
   typedef typename image_view::const_iterator      const_iterator;

private:
   image_store mStore;
   image_view  mDefaultView;

public:
   ParametricImage(unsigned rows,unsigned cols,unsigned padding = 0) :
         mStore(rows,cols,padding),
         mDefaultView(rows,cols,&mStore,&mStore,padding,padding)
   {}

   // Need to override copy-constructor and assignment operator
   // to ensure mDefaultView points to the proper ImageStoreT
   ParametricImage(const ParametricImage& that) :
         mStore(that.mStore),
         mDefaultView(that.rows(),that.cols(),&mStore,&mStore,mStore.padding(),mStore.padding())
   {}

   template<typename PixelTT>
   ParametricImage(const Image<PixelTT>& that) :
         mStore(that.rows(),that.cols(),that.padding()),
         mDefaultView(that.rows(),that.cols(),&mStore,&mStore,that.padding(),that.padding()) {
      iterator tpos = begin();
      iterator tend = end();
      typename Image<PixelTT>::const_iterator spos = that.begin();
      // The below allows, conversion between two image pixel types.
      // However, the Pixels must be implicitly convertible.
      for(;tpos != tend;++tpos,++spos) *tpos = *spos;
   }


   template<typename PixelTT>
   ParametricImage(const ImageView<PixelTT>& that) :
         mStore(that.rows(),that.cols(),0),
         mDefaultView(that.rows(),that.cols(),&mStore,&mStore,0,0) {
      iterator tpos = begin();
      iterator tend = end();
      typename ImageView<PixelTT>::const_iterator spos = that.begin();
      // The below allows, conversion between two image pixel types.
      // However, the Pixels must be implicitly convertible.
      for(;tpos != tend;++tpos,++spos) *tpos = *spos;
   }

   ParametricImage& operator=(const ParametricImage& that) {
      if(this != &that) {
         // Note: that the ImageStore assignment is what copies all of the data
         mStore = that.mStore;
         mDefaultView.updateBounds(mStore);
         mDefaultView.resizeAndMove(that.rows(),that.cols(),that.padding(),that.padding());
      }
      return *this;
   }

   template<typename PixelTT>
   ParametricImage& operator=(const ParametricImage<PixelTT>& that) {
      void* utThis = this;
      void* utThat = &that;
      if(utThis != utThat) {
         resize(that.rows(),that.cols(),that.padding());
         iterator tpos = begin();
         iterator tend = end();
         typename Image<PixelTT>::const_iterator spos = that.begin();
         // The below allows, conversion between two image pixel types.
         // However, the Pixels must be implicitly convertible.
         for(;tpos != tend;++tpos,++spos) *tpos = *spos;
      }
      return *this;
   }

   template<typename PixelTT>
   ParametricImage& operator=(const ImageView<PixelTT>& that) {
      void* utThisStore = &this->mStore;
      void* utThatStore = that.mStore;
      if(utThisStore != utThatStore) {
         resize(that.rows(),that.cols());
         iterator tpos = begin();
         iterator tend = end();
         typename ImageView<PixelTT>::const_iterator spos = that.begin();
         // The below allows, conversion between two image pixel types.
         // However, the Pixels must be implicitly convertible.
         for(;tpos != tend;++tpos,++spos) *tpos = *spos;
      }
      else {
         // Apparently we are trying to assign a view of ourself to our own image
         // The only way to do this is to simply clone ourself and then assign
         // TODO: one caveat to this, is that for now we will lose padding. However,
         // we aren't even using this feature yet...
         ParametricImage clonedView(that);
         *this = clonedView;
      }
      return *this;
   }


   // ***************************************************************************
   // Note: despite the rule of three, in this case we don't need a destructor...
   // ***************************************************************************

   void resize(unsigned rows,unsigned cols,unsigned padding) {
      mStore.resize(rows,cols,padding);
      mDefaultView.updateBounds(mStore);
      mDefaultView.resize(rows,cols);
   }

   void resize(unsigned rows,unsigned cols) {
      mStore.resize(rows,cols);
      mDefaultView.updateBounds(mStore);
      mDefaultView.resize(rows,cols);
   }

   unsigned rows() const { return mDefaultView.rows(); }

   unsigned cols() const { return mDefaultView.cols(); }

   unsigned padding() const { return mStore.padding(); }

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

   const_elastic_image_view elastic_view(unsigned rows,unsigned cols) const {
      return const_elastic_image_view(rows,cols,
            // Although Views can be made const, it does not make sense to have
            // a const ImageStore or ImageBounds type.
                                      const_cast<typename std::remove_const<image_store>::type*>(&mStore),
                                      const_cast<typename std::remove_const<image_bounds>::type*>(static_cast<const image_bounds*>(&mStore)));
   }

   elastic_image_view elastic_view(unsigned rows,unsigned cols) {
      return elastic_image_view(rows,cols,&mStore,&mStore);
   }

   iterator begin() { return mDefaultView.begin(); }

   iterator end() { return mDefaultView.end(); }

   const_iterator begin() const { return mDefaultView.begin(); }

   const_iterator end() const { return mDefaultView.end(); }
};




} // namespace types
} // namespace batchIP
