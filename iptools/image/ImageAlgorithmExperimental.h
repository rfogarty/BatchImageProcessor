
template<typename PixelT,typename AccumulatorVariableTT>
struct PixelSquareSubtractor {
   AccumulatorVariableTT& mSquareAccumulator;
   PixelSquareSubtractor(AccumulatorVariableTT& squareAccumulator) : mSquareAccumulator(squareAccumulator) {}
   void operator()(const PixelT& pixel) { 
      mSquareAccumulator -= pixel.namedColor.gray * pixel.namedColor.gray; }
};


template<typename PixelT,typename AccumulatorVariableTT>
struct PixelSquareAdder {
   AccumulatorVariableTT& mSquareAccumulator;
   PixelSquareAdder(AccumulatorVariableTT& squareAccumulator) :  mSquareAccumulator(squareAccumulator) {}
   void operator()(const PixelT& pixel) { 
      mSquareAccumulator += pixel.namedColor.gray * pixel.namedColor.gray;
   }
};



template<typename ParametricImageT>
void initParametricImage(ParametricImageT& parImage) {
   
   typedef typename ParametricImageT::pixel_type PixelT;

   for(unsigned r = 0; r < parImage.rows();++r) {
      for(unsigned c = 0; c < parImage.cols();++c) {
         PixelT& rpix = parImage.pixel(r,c);
         rpix.row = r;
         rpix.col = c;
      }
   }
}



// Computing Variance using the "Sum of Squares" method. Note:
// that under some circumstances that this approach is numerically unstable.
// For more details on this see:
// https://www.johndcook.com/blog/2008/09/26/comparing-three-methods-of-computing-standard-deviation/
//
// It was not apparent how to apply the Welford method to a moving windowed average/variance.
// But for low-precision (uint8_t) grayscale, we'd need to have rather large
// window size (larger than 16x16) to exceed the precision of the accumulators.
//
// Furthermore, for the derivation of the Variance + Mean^2 term, if the window
// shrinks to 1 (namely in the corners or edges of the Window, an unbiased variance calculation
// leads to divide by zero terms in both the sum of squares and the square of average term. If
// the biased mean, on the other hand is used, we have numerous benefits:
//   1) The square of the average term completely falls away (so does not need to be calculate)
//   2) The sum of the square terms is simply divided by the window size without needing
//      a difference term with large precision.
//   3) Window size may be increased rather dramatically from 15x15 to 257x257 (for uint8_t
//   grayscale, and unsigned int accumulator).
// I.e. using the biased estimate for variance provides a stable, numerical and faster solution.
// We also expect that the bias is in the same direction for all samples, and we really
// only care about the relative order of M^2 + S^2 terms for histogram equalization (by sorting),
// so bias is of no concern.
template<typename ImageViewT,
         typename PixelT = typename ImageViewT::pixel_type,
         typename AccumulatorVariableT = typename AccumulatorVariableSelect<PixelT>::type >
class SumOfSquares {
private:
   typedef ElasticImageView<const PixelT>                     ConstElasticViewT;
   typedef PixelSquareSubtractor<PixelT,AccumulatorVariableT> SubtractorT;
   typedef PixelSquareAdder<PixelT,AccumulatorVariableT>      AdderT;

   typename ImageViewT::const_image_view mBoundingView;
   ConstElasticViewT                     mElasticView;
   AccumulatorVariableT                  mSquareAccumulator;
   SubtractorT                           mSubtractor;
   AdderT                                mAdder;

public:
   typedef typename PixelT::value_type value_type;

   SumOfSquares(const ImageViewT& imageView,unsigned windowSize) :
      mBoundingView(imageView),
      mElasticView(mBoundingView.elastic_view(windowSize,windowSize)),
      // Initialize mAccumulator with element 0,0
      mSquareAccumulator(mElasticView.pixel(0,0).namedColor.gray*mElasticView.pixel(0,0).namedColor.gray),
      mSubtractor(mSquareAccumulator),
      mAdder(mSquareAccumulator)
   {}

   value_type squareAverage() {
      return static_cast<value_type>(checkValue<PixelT>(static_cast<AccumulatorVariableT>((double) mSquareAccumulator / mElasticView.size())));
   }

   void operator++() {
      mElasticView.moveRight(mSubtractor,mAdder);
   }
};


// The concept with this histogram equalization is to find a more perfect uniform equalization.
// The idea is to rank all pixels in both intensity and another term such as gradient or 
// dispersion(variance). Then something like the sum of the squares of the terms will be calculated
// pixels are than sorted redistributed across the intensity channel. I believe a colleague of mine
// Dr. Ashwin Sarma suggested a solution like this back in about 2003 when I worked for the Navy.
template<typename SrcImageT,typename TgtImageT>
void histogramUnify(const SrcImageT& src, TgtImageT& tgt,unsigned windowSize,
         // This ugly bit is an unnamed argument with a default which means it neither           
         // contributes to the mangled declaration name nor requires an argument. So what is the 
         // point? It still participates in SFINAE to help select that this is an appropriate    
         // matching function given its arguments. Note, SFINAE techniques are incompatible with 
         // deduction so can't be applied to in parameter directly.                              
         typename std::enable_if<is_grayscale<typename SrcImageT::pixel_type>::value,int>::type* = 0) {

   typedef typename SrcImageT::pixel_type PixelT;
   typedef typename AccumulatorVariableSelect<PixelT>::type AccumulatorVariableT;
   typedef ParametricGrayAlphaPixel<AccumulatorVariableT> IntermediatePixelT;
   typedef Image<IntermediatePixelT> IntermediateImageT;

   reportIfNotLessThan("windowSize",2u,windowSize);
   reportIfNotEqual("windowSize (which should be odd)",windowSize-1,((windowSize >> 1u) << 1u));
   reportIfNotEqual("src.rows() != tgt.rows()",src.rows(),tgt.rows());
   reportIfNotEqual("src.cols() != tgt.cols()",src.cols(),tgt.cols());

   unsigned rows = src.rows();
   unsigned cols = src.cols();

   // We can write out result to iterator operating over entire passed Image or ImageView
   // since the iteration order is exactly the same as our loops.
   IntermediateImageT interImage(rows,cols);
   // Start by initializing parametric values;
   initParametricImage(interImage);

   // TODO: To Complete!!!
//   typename TgtImageT::iterator tpos = tgt.begin();
//
//   // Start by smoothing along X for each row
//   for(unsigned i = 0;i < rows;++i) {
//      typedef SumOfSquares<SrcImageT> SquareSmoothT;
//      typedef typename SrcImageT::image_view RowViewT;
//      
//      RowViewT rowView(src.view(rows-i,cols,i));
//      SquareSmoothT squareSmooth(rowView,windowSize);
//      // We always start with the first answer precomputed
//      // which also implies we will only iterate and shift cols-1 times.
//      tpos->namedColor.gray = smoothX.average();
//      ++tpos;
//      for(unsigned j = 1;j < cols;++j,++tpos) {
//         ++smoothX;
//         tpos->namedColor.gray = smoothX.average();
//      }
//   }
//
//   typedef typename TgtImageT::image_view ColumnViewT; // not strictly a Column type, but will be
//                                                       // parameterized below to operate as one.
//   typedef typename ColumnViewT::iterator ColumnIteratorT;
//   // In this case our iteration order is flipped, and since we don't have a column-first
//   // iterator type, we just need to create column views to write out our output.
//   for(unsigned j = 0; j < cols; ++j) {
//      typedef SmoothY<TgtImageT> SmoothYT;
//      ColumnViewT columnView(tgt.view(rows,1,0,j));
//      SmoothYT smoothY(columnView,windowSize);
//      ColumnIteratorT cpos = columnView.begin();
//      // As above, we always start with the first answer precomputed
//      // which also implies we will only iterate and shift rows-1 times.
//      cpos->namedColor.gray = smoothY.average();
//      ++cpos;
//      for(unsigned i = 1;i < rows;++i,++cpos) {
//         ++smoothY;
//         cpos->namedColor.gray = smoothY.average();
//      }
//   }
}


