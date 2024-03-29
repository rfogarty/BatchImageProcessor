#pragma once

#include "cppTools/TemplateMetaprogramming.h"
#include "utility/Error.h"
#include <iterator>
#include <vector>

namespace batchIP {
namespace types {

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
   // rowEnd is exclusive (not inclusive of range).
   unsigned rowEnd() const { return rowBegin() + rows(); }
   // colEnd is exclusive (not inclusive of range).
   unsigned colEnd() const { return colBegin() + cols(); }

   //std::size_t pixelSize() { return sizeof(PixelT); }
};

template<typename PixelT>
class SavedImageBounds : public ImageBounds<PixelT> {
private:
   unsigned mRows;
   unsigned mCols;
   unsigned mRowBegin;
   unsigned mColBegin;

public:
   SavedImageBounds() :
      mRows(0u),
      mCols(0u),
      mRowBegin(0u),
      mColBegin(0u)
   {}

   template<typename ImageBoundsT>
   SavedImageBounds(const ImageBoundsT& bounds) :
      mRows(bounds.rows()),
      mCols(bounds.cols()),
      mRowBegin(bounds.rowBegin()),
      mColBegin(bounds.colBegin())
   {}

   virtual ~SavedImageBounds() {}

   virtual unsigned rows() const { return mRows; }
   virtual unsigned cols() const { return mCols; }
   virtual unsigned rowBegin() const { return mRowBegin; }
   virtual unsigned colBegin() const { return mColBegin; }
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
template<typename PixelT,
         typename ImageBoundsT = ImageBounds<typename std::remove_const<PixelT>::type> >
class ImageStore : public ImageBoundsT {
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
      pixels(mAllocatedRows*mAllocatedCols,pixel_type())
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

   virtual unsigned rows() const { return mAllocatedRows; }

   virtual unsigned cols() const { return mAllocatedCols; }

   virtual unsigned rowBegin() const { return 0u; }

   virtual unsigned colBegin() const { return 0u; }

   unsigned padding() const { return mPadding; }

   const pixel_type& pixel(unsigned row,unsigned col) const {
      utility::reportIfNotLessThan("row",row,mAllocatedRows);
      utility::reportIfNotLessThan("col",col,mAllocatedCols);
      unsigned index = mAllocatedCols * row + col;
      return pixels[index];
   }

   pixel_type& pixel(unsigned row,unsigned col) {
      return const_cast<pixel_type&>(static_cast<const this_type&>(*this).pixel(row,col));
   }
};

// Prototype for Image class
template<typename PixelT> class Image;
template<typename PixelT,
         typename ImageStoreT,
         typename ImageBoundsT> class ImageView;

///////////////////////////////////////////////////////////////////////////////
// ImageWindow - base class for setting up a view of Image data.
//
// This class cannot be constructed directly but rather is intended to be
// used as a base class. Its primary derivative is ImageView.
//
template<typename PixelT,
         typename ImageStoreT  = ImageStore<typename std::remove_const<PixelT>::type>,
         typename ImageBoundsT = ImageBounds<typename std::remove_const<PixelT>::type> >
class ImageWindow : public ImageBoundsT {
   // Images are friends of ImageWindows, to allow for Image assignment
   //   which requires the private update of bounds.
   template<typename PixelTT> friend class Image;
   template<typename T,typename U,typename V> friend class ImageWindow;

public:
   typedef typename std::remove_const<PixelT>::type pixel_type;
   typedef ImageWindow<PixelT,ImageStoreT>          this_type;
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
   ImageWindow(unsigned rows,unsigned cols,
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

   // This conversion constructor is used to convert between non-const
   // and const versions of PixelT.
   template<typename ImageWindowT>
   explicit ImageWindow(const ImageWindowT& that) :
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
   ImageWindow& operator=(const ImageWindow&); // deleted

   void updateBounds(const image_bounds& newBounds) {
      mBounds = saved_image_bounds(newBounds);
   }

public:
   const PixelT& pixel(unsigned row, unsigned col) const {
      // Verify that the requested pixel is within the bounds of the ImageWindow
      utility::reportIfNotLessThan("mRowBegin+row",row,mRows);
      utility::reportIfNotLessThan("mColBegin+col",col,mCols);
      return mStore->pixel(row+mRowBegin,col+mColBegin);
   }

   PixelT& pixel(unsigned row, unsigned col) {
      // Note: if ImageWindow PixelT is a const type, the below const_cast is a no-op.
      return const_cast<PixelT&>(static_cast<const this_type&>(*this).pixel(row,col));
   }

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

   void move(unsigned rowPos,unsigned colPos) {
      utility::reportIfNotLessThan("rowBegin+mRows",rowPos + mRows,mBounds.rowEnd()+1);
      utility::reportIfNotLessThan("colBegin+mCols",colPos + mCols,mBounds.colEnd()+1);
      mRowBegin = rowPos + mBounds.rowBegin();
      mColBegin = colPos + mBounds.colBegin();
   }

   void shiftCol() {
      unsigned colPos = mColBegin + 1;
      utility::reportIfNotLessThan("colBegin+mCols",colPos + mCols,mBounds.colEnd()+1);
      mColBegin = colPos;
   }

   void shiftRow() {
      unsigned rowPos = mRowBegin + 1;
      utility::reportIfNotLessThan("rowBegin+mRows",rowPos + mRows,mBounds.rowEnd()+1);
      mRowBegin = rowPos;
   }
};

///////////////////////////////////////////////////////////////////////////////
// ImageViewIterator - a forward iterator and input and output iterator.
//
// Iterates the ImageView from which it is requested. Iteration order
// is as expected, across (columns) then down (rows).
//
template<typename PixelT,typename ImageWindowT = ImageWindow<PixelT> >
class ImageViewIterator {
   template<typename PixelTT,typename ImageWindowTT> friend class ImageViewIterator;
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

   explicit ImageViewIterator(ImageWindowT* window,unsigned row = 0) :
      mImageWindow(window),
      rowPos(row),
      colPos(0)
   {}

public:
   // ImageViewIterators must be constructed via these static begin/end functions or copy-constructed.
   static ImageViewIterator begin(ImageWindowT* window) { return ImageViewIterator(window); }
   static ImageViewIterator end(ImageWindowT* window) { return ImageViewIterator(window,window->rows()); }

   ImageViewIterator(const ImageViewIterator& that) :
      mImageWindow(that.mImageWindow),
      rowPos(that.rowPos),
      colPos(that.colPos)
   {}

   template<typename PixelTT,typename ImageWindowTT>
   explicit ImageViewIterator(const ImageViewIterator<PixelTT,ImageWindowTT>& that) :
      mImageWindow(that.mImageWindow),
      rowPos(that.rowPos),
      colPos(that.colPos)
   {}

   ImageViewIterator& operator=(const ImageViewIterator& that) {
      if(this != &that) {
         this->mImageWindow = that.mImageWindow;
         this->rowPos = that.rowPos;
         this->colPos = that.colPos;
      }
      return *this;
   }

   template<typename PixelTT,typename ImageWindowTT>
   ImageViewIterator& operator=(const ImageViewIterator<PixelTT,ImageWindowTT>& that) {
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
// ElasticImageView - a view of an Image that is given a window size, but that
//                    window size automatically grows and shrinks as it approaches
//                    the border of the ImageBounds that contains it.
//
// Supports:
// 1) Iteration of the "elastic window".
// 2) Indexed access to pixels for read or write.
// 3) Moving operations moveRight and moveDown (note, no general move or resize supported)
// 4) Listen callbacks during moveRight and moveDown operations to track which pixels
//    have departed the elastic window, and which pixels have entered it.
//
template<typename PixelT,
         typename ImageStoreT  = ImageStore<typename std::remove_const<PixelT>::type>,
         typename ImageBoundsT = ImageBounds<typename std::remove_const<PixelT>::type> >
class ElasticImageView {
public:
   typedef ElasticImageView<PixelT,ImageStoreT,ImageBoundsT>   this_type;
   typedef typename std::remove_const<PixelT>::type            pixel_type;
   typedef ImageStoreT                                         image_store;
   typedef ImageBoundsT                                        image_bounds;
   typedef SavedImageBounds<pixel_type>                        saved_image_bounds;
   typedef ImageViewIterator<PixelT,this_type>                 iterator;
   typedef ImageViewIterator<const pixel_type,const this_type> const_iterator;

private:
   unsigned           mHalfWindowRows; // The largest size rows our elastic view can grow
   unsigned           mHalfWindowCols; // The largest size cols our elastic view can grow
   unsigned           mElasticHalfWindowRows;
   unsigned           mElasticHalfWindowCols;
   // mRowPos and mColPos are offsets within a "bounded" view
   // Additionally, together (mRowPos,mColPos) represents the "center"
   // of the ElasticImageView (not the upper corner of the window), while
   // an ImageView's rowBegin and colBegin represent its window's top-left corner.
   unsigned           mRowPos;
   unsigned           mColPos;
   image_store*       mStore;
   saved_image_bounds mBounds;
   

   template<typename DepartedListener,typename EnteredListener>
   void shiftCol(DepartedListener& departedListener,EnteredListener& enteredListener) {
      
      unsigned oldStart = mColPos - mElasticHalfWindowCols; // inclusive
      unsigned oldEnd   = mColPos + mElasticHalfWindowCols; // also inclusive

      ++mColPos;

      unsigned halfCols = (mBounds.cols()-1)/2;

      if(mColPos < mHalfWindowCols) mElasticHalfWindowCols = std::min(mColPos,halfCols);
      else {
         unsigned rColPos = mBounds.cols()-1-mColPos;
         if(rColPos < mHalfWindowCols) mElasticHalfWindowCols = std::min(rColPos,halfCols);
         else {
            // We can only get here if Window is smaller or equal to bounds
            // so no need to do std::min(mHalfWindowCols,halfCols)
            mElasticHalfWindowCols = mHalfWindowCols;
         }
      }

      unsigned newStart = mColPos - mElasticHalfWindowCols; // inclusive
      unsigned newEnd   = mColPos + mElasticHalfWindowCols; // also inclusive

      // Now we just have to determine, what has moved outside
      // and inside the window.
      
      // Send departed
      unsigned rowBegin = mRowPos - mElasticHalfWindowRows;
      unsigned rowEnd   = mRowPos + mElasticHalfWindowRows;

      for(unsigned r = rowBegin;r <= rowEnd;++r) {
         for(unsigned c = oldStart;c < newStart;++c) {
            departedListener(mStore->pixel(mBounds.rowBegin()+r,mBounds.colBegin()+c));
         }
      }

      // Send entered
      for(unsigned r = rowBegin;r <= rowEnd;++r) {
         for(unsigned c = oldEnd + 1;c <= newEnd;++c) {
            enteredListener(mStore->pixel(mBounds.rowBegin()+r,mBounds.colBegin()+c));
         }
      }
   }

   template<typename DepartedListener,typename EnteredListener>
   void shiftRow(DepartedListener& departedListener,EnteredListener& enteredListener) {
      
      unsigned oldStart = mRowPos - mElasticHalfWindowRows; // inclusive
      unsigned oldEnd   = mRowPos + mElasticHalfWindowRows; // also inclusive

      ++mRowPos;

      unsigned halfRows = (mBounds.rows()-1)/2;

      if(mRowPos < mHalfWindowRows) mElasticHalfWindowRows = std::min(mRowPos,halfRows);
      else {
         unsigned rRowPos = mBounds.rows()-1-mRowPos;
         if(rRowPos < mHalfWindowRows) mElasticHalfWindowRows = std::min(rRowPos,halfRows);
         else {
            // We can only get here if Window is smaller or equal to bounds
            // so no need to do std::min(mHalfWindowRows,halfRows)
            mElasticHalfWindowRows = mHalfWindowRows;
         }
      }

      unsigned newStart = mRowPos - mElasticHalfWindowRows; // inclusive
      unsigned newEnd   = mRowPos + mElasticHalfWindowRows; // also inclusive

      // Now we just have to determine, what has moved outside
      // and inside the window.
      
      // Send departed
      unsigned colBegin = mColPos - mElasticHalfWindowCols;
      unsigned colEnd   = mColPos + mElasticHalfWindowCols;

      for(unsigned r = oldStart;r < newStart;++r) {
         for(unsigned c = colBegin;c <= colEnd;++c) {
            departedListener(mStore->pixel(mBounds.rowBegin()+r,mBounds.colBegin()+c));
         }
      }

      for(unsigned r = oldEnd + 1;r <= newEnd;++r) {
         for(unsigned c = colBegin;c <= colEnd;++c) {
            enteredListener(mStore->pixel(mBounds.rowBegin()+r,mBounds.colBegin()+c));
         }
      }
   }

public:

   ElasticImageView(unsigned rows,unsigned cols,image_store* store,image_bounds* bounds) :
      // The below subract by 1 and divide by 2 enforces odd size windows.
      mHalfWindowRows((rows-1)/2),
      mHalfWindowCols((cols-1)/2),
      mElasticHalfWindowRows(0),
      mElasticHalfWindowCols(0),
      mRowPos(0),
      mColPos(0),
      mStore(store),
      mBounds(*bounds) {
      // Ensure rows and columns is at least 1 or greater!
      utility::reportIfNotLessThan("rows",0u,rows);
      utility::reportIfNotLessThan("cols",0u,cols);
   }

   const PixelT& pixel(unsigned row, unsigned col) const {
      // Verify that the requested pixel is within the bounds of the ImageWindow
      utility::reportIfNotLessThan("row",row,rows());
      utility::reportIfNotLessThan("col",col,cols());
      unsigned rowOffset = mRowPos - mElasticHalfWindowRows + row;
      unsigned colOffset = mColPos - mElasticHalfWindowCols + col;
      return mStore->pixel(rowOffset+mBounds.rowBegin(),colOffset+mBounds.colBegin());
   }

   PixelT& pixel(unsigned row, unsigned col) {
      // Note: if ImageWindow PixelT is a const type, the below const_cast is a no-op.
      return const_cast<PixelT&>(static_cast<const this_type&>(*this).pixel(row,col));
   }

   template<typename DepartedListener,typename EnteredListener>
   void moveRight(DepartedListener& departedListener,EnteredListener& enteredListener) {
      utility::reportIfNotLessThan("cols",mColPos+1,mBounds.cols());
      shiftCol(departedListener,enteredListener);
   }

   template<typename DepartedListener,typename EnteredListener>
   void moveDown(DepartedListener& departedListener,EnteredListener& enteredListener) {
      utility::reportIfNotLessThan("rows",mRowPos+1,mBounds.rows());
      shiftRow(departedListener,enteredListener);
   }

   unsigned rows() const { return mElasticHalfWindowRows*2 + 1; }

   unsigned cols() const { return mElasticHalfWindowCols*2 + 1; }

   // TODO: this doesn't seem to compensate for "elasticity" when the view is pushed
   //       up against a corner or edge
   unsigned size() const { return rows() * cols(); }

   unsigned rowPosition() const { return mRowPos; }

   unsigned colPosition() const { return mColPos; }

   iterator begin() { return iterator::begin(this); }
   
   iterator end() { return iterator::end(this); }

   const_iterator begin() const { return const_iterator::begin(this); }
   
   const_iterator end() const { return const_iterator::end(this); }
};



///////////////////////////////////////////////////////////////////////////////
// ImageView - a view of an Image is a bounded box that is valid anywhere
//             within the bounds of an image or a parent ImageView, 
//             and is useful to operate on regions of an Image.
//
// Supports:
// 1) Iteration of the ImageView.
// 2) Subviews (views of views).
// 3) Indexed access to pixels for read or write.
// 4) Copy construction of views.
// 5) Assignment of views of equal size.
// 6) Moving and Resizing of the View (as long as the ImageView remains
//    within the ImageBounds of its parent).
// 7) An ImageView's parent can be an Image itself or another ImageView.
// 8) Can also create an ElastiveImageViews with itself as its parent.
//
template<typename PixelT,
         typename ImageStoreT  = ImageStore<typename std::remove_const<PixelT>::type>,
         typename ImageBoundsT = ImageBounds<typename std::remove_const<PixelT>::type> >
class ImageView : public ImageWindow<PixelT> {
   // ImageViews are friends of all other ImageView types
   // for the purpose of self-assignment checks.
   template<typename PixelTT,
            typename ImageStoreTT,
            typename ImageBoundsTT> friend class ImageView;

public:
   typedef typename std::remove_const<PixelT>::type               pixel_type;
   typedef ImageStoreT                                            image_store;
   typedef ImageBoundsT                                           image_bounds;
   typedef ImageWindow<PixelT>                                    image_window;
   typedef ImageView<PixelT,ImageStoreT,ImageBoundsT>             image_view;
   typedef const ImageView<const pixel_type>                      const_image_view;
   typedef ImageViewIterator<PixelT,image_window>                 iterator;
   typedef ImageViewIterator<const pixel_type,const image_window> const_iterator;
   typedef ElasticImageView<pixel_type>                           elastic_image_view;
   typedef ElasticImageView<const pixel_type>                     const_elastic_image_view;

   ImageView(unsigned rows,unsigned cols,
             image_store* store,
             image_bounds* bounds,
             unsigned rowPos = 0, unsigned colPos = 0) :
      image_window(rows,cols,store,bounds,rowPos,colPos)
   {}

   // This conversion constructor is used to convert between non-const
   // and const versions of PixelT.
   template<typename ImageViewT>
   explicit ImageView(const ImageViewT& that) :
      image_window(that)
   {}

   ImageView& operator=(const ImageView& that) {
      if(this != &that) {
         utility::reportIfNotEqual("mRows",this->mRows,that.mRows);
         utility::reportIfNotEqual("mCols",this->mCols,that.mCols);
         if(this->mStore == that.mStore) {
            // TODO: this should be optimized, however,
            // if these are the same store, we need to check
            // that the regions are either non-overlapping or
            // that the target succeeds the source.
            Image<pixel_type> copy(that);
            *this = copy.defaultView();
         }
         else {
            iterator tpos = begin();
            iterator tend = end();
            const_iterator spos = that.begin();
            for(;tpos != tend;++tpos,++spos) *tpos = *spos;
         }
      }
      return *this;
   }

   template<typename PixelTT,typename ImageStoreTT,typename ImageBoundsTT>
   ImageView& operator=(const ImageView<PixelTT,ImageStoreTT,ImageBoundsTT>& that) {
      const unsigned char* thisStore = reinterpret_cast<const unsigned char*>(this->mStore);
      const unsigned char* thatStore = reinterpret_cast<const unsigned char*>(that.mStore);
      utility::reportIfNotEqual("mRows",this->mRows,that.rows());
      utility::reportIfNotEqual("mCols",this->mCols,that.cols());
      if(thisStore != thatStore) {
         iterator tpos = begin();
         iterator tend = end();
         typename ImageView<PixelTT,ImageStoreTT,ImageBoundsTT>::const_iterator spos = that.begin();
         // The below allows, conversion between two image pixel types.
         // However, the Pixels must be implicitly convertible.
         for(;tpos != tend;++tpos,++spos) *tpos = *spos;
      }
      else {
         // Stores are the same, so for now copy pixels
         // out before copying.
         // TODO: this can definitely be optimized.
         Image<pixel_type> copy(that);
         *this = copy.defaultView();
      }
      return *this;
   }

   template<typename ImageT>
   ImageView& operator=(const ImageT& that) {
      const void* thisStore = reinterpret_cast<const void*>(this->mStore);
      const void* thatStore = that.store();
      utility::reportIfNotEqual("mRows",this->mRows,that.rows());
      utility::reportIfNotEqual("mCols",this->mCols,that.cols());
      if(thisStore != thatStore) {
         iterator tpos = begin();
         iterator tend = end();
         typename ImageT::const_iterator spos = that.begin();
         // The below allows, conversion between two image pixel types.
         // However, the Pixels must be implicitly convertible.
         for(;tpos != tend;++tpos,++spos) *tpos = *spos;
      }
      else {
         // Stores are the same, so for now copy pixels
         // out before copying.
         // TODO: this can definitely be optimized.
         Image<pixel_type> copy(that);
         *this = copy.defaultView();
      }
      return *this;
   }



   const_image_view view(unsigned rows,unsigned cols,
                         unsigned rowPos = 0,unsigned colPos = 0) const {
      return const_image_view(rows,cols,
                             // Although Views can be made const, it does not make sense to have
                             // a const ImageStore or ImageBounds type.
                             const_cast<typename std::remove_const<image_store>::type*>(this->mStore),
                             const_cast<typename std::remove_const<image_bounds>::type*>(static_cast<const image_bounds*>(this)),
                             rowPos,colPos);
   }


   // take a subview of the view
   image_view view(unsigned rows,unsigned cols,
                   unsigned rowPos = 0,unsigned colPos = 0) {
      return image_view(rows,cols,this->mStore,this,
                        rowPos,colPos);
   }

   const_elastic_image_view elastic_view(unsigned rows,unsigned cols) const {
      return const_elastic_image_view(rows,cols,
                             // Although Views can be made const, it does not make sense to have
                             // a const ImageStore or ImageBounds type.
                             const_cast<typename std::remove_const<image_store>::type*>(this->mStore),
                             const_cast<typename std::remove_const<image_bounds>::type*>(static_cast<const image_bounds*>(this)));
   }


   // take a subview of the view
   elastic_image_view elastic_view(unsigned rows,unsigned cols) { return elastic_image_view(rows,cols,this->mStore,this); }

   iterator begin() { return iterator::begin(this); }
   
   iterator end() { return iterator::end(this); }

   const_iterator begin() const { return const_iterator::begin(this); }
   
   const_iterator end() const { return const_iterator::end(this); }
};


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
   typedef ElasticImageView<pixel_type>             elastic_image_view;
   typedef ElasticImageView<const pixel_type>       const_elastic_image_view;
   typedef typename image_view::iterator            iterator;
   typedef typename image_view::const_iterator      const_iterator;

private:
   image_store mStore;
   image_view  mDefaultView;

public:
   explicit Image(unsigned rows = 0,unsigned cols = 0,unsigned padding = 0) :
      mStore(rows,cols,padding),
      mDefaultView(rows,cols,&mStore,&mStore,padding,padding)
   {}

   // Need to override copy-constructor and assignment operator
   // to ensure mDefaultView points to the proper ImageStoreT
   Image(const Image& that) :
      mStore(that.mStore),
      mDefaultView(that.rows(),that.cols(),&mStore,&mStore,mStore.padding(),mStore.padding())
   {}

   template<typename PixelTT>
   Image(const Image<PixelTT>& that) :
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
   Image(const ImageView<PixelTT>& that) :
      mStore(that.rows(),that.cols(),0),
      mDefaultView(that.rows(),that.cols(),&mStore,&mStore,0,0) {
      iterator tpos = begin();
      iterator tend = end();
      typename ImageView<PixelTT>::const_iterator spos = that.begin();
      // The below allows, conversion between two image pixel types.
      // However, the Pixels must be implicitly convertible.
      for(;tpos != tend;++tpos,++spos) *tpos = *spos;
   }

   Image& operator=(const Image& that) {
      if(this != &that) {
         // Note: that the ImageStore assignment is what copies all of the data
         mStore = that.mStore;
         mDefaultView.updateBounds(mStore);
         mDefaultView.resizeAndMove(that.rows(),that.cols(),that.padding(),that.padding());
      }
      return *this;
   }

   template<typename PixelTT>
   Image& operator=(const Image<PixelTT>& that) {
      const void* utThis = this;
      const void* utThat = &that;
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
   Image& operator=(const ImageView<PixelTT>& that) {
      const void* utThisStore = &this->mStore;
      const void* utThatStore = that.mStore;
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
         Image clonedView(that);
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

   unsigned size() const { return mDefaultView.size(); }

   unsigned padding() const { return mStore.padding(); }

   const pixel_type& pixel(unsigned row, unsigned col) const { return mDefaultView.pixel(row,col); }

   pixel_type& pixel(unsigned row, unsigned col) { return mDefaultView.pixel(row,col); }

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


   const void* store() const { return &mStore; }
};

} // namespace types
} // namespace batchIP
