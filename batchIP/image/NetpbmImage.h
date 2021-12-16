#pragma once

#include "image/Image.h"
#include "image/Pixel.h"
#include "image/PixelReader.h"
#include "image/PixelWriter.h"
#include "utility/StringParse.h"
#include <fstream>
#include <string>

namespace batchIP {
namespace io {

void readPNMHeader(std::istream& ifs,const std::string& filename,
                   unsigned& rows, unsigned& cols, unsigned& numColors,
                   const char* pnmTypeID,const char* extType) {
   std::string str;
   std::getline(ifs, str);

   if ((str.substr(0,2) != pnmTypeID)) {
      std::stringstream ss;
      ss << "Image file \"" << filename << "\" not in " << extType << " format..." << str;
      throw std::invalid_argument(ss.str().c_str());
   }

   /* read the next two ints in a file to grab the rows and cols*/
   if(!utility::readNext2Integers(ifs,cols,rows)) {
      std::stringstream ss;
      ss << "Image file \"" << filename << "\" size could not be deduced perhaps bad PNM format...";
      throw std::invalid_argument(ss.str().c_str());
   }

   /* read the next int in a file to grab the numColors */
   if(!utility::readNextInteger(ifs,numColors)) {
      std::stringstream ss;
      ss << "Image file \"" << filename << "\" numColors could not be deduced perhaps bad PNM format...";
      throw std::invalid_argument(ss.str().c_str());
   }
//
//   // This bit seems a little troublesome (especially if there are extra comment lines)
//   std::string res;
//   std::getline(ifs,res);
}

template<typename PixelT> 
types::Image<PixelT> readPPMFile(const std::string& filename) {
   /* PPM Color Image */
   if (!utility::endsWith(filename, ".ppm"))
      throw std::invalid_argument("Incorrect File Type, expecting .ppm");

   std::fstream ppm_file;
   ppm_file.open(filename.c_str(), std::ios_base::in | std::ios_base::binary);
   if (!ppm_file.is_open()) {
      std::stringstream ss;
      ss << "Filename \"" << filename << "\" could not be read. Check path or permissions.";
      throw std::invalid_argument(ss.str().c_str());
   }

   unsigned cols = 0, rows = 0, numColors = 0;
   readPNMHeader(ppm_file,filename,rows,cols,numColors,"P6","PPM");

   // Copy all the image buffer data into the Image<PixelT> object
   typedef types::Image<PixelT> ImageT;

   ImageT image(rows,cols);

   // Read color bytes
   if( numColors == 255) {
      unsigned byteSize = rows*cols*3;
      typedef std::vector<uint8_t> BufferT;
      BufferT buffer(byteSize);
      ppm_file.read(reinterpret_cast<char*>(&buffer[0]), byteSize);
      ppm_file.close();

      PixelReader<ImageT>::readRGBPixels(image,buffer);
   }
   else if (numColors == 65535 ) {
      unsigned elemSize = rows*cols*3;
      unsigned byteSize = elemSize*2;
      typedef std::vector<uint16_t> BufferT;
      BufferT buffer(elemSize);
      ppm_file.read(reinterpret_cast<char*>(&buffer[0]), byteSize);
      ppm_file.close();

      // Copy all the image buffer data into the Image<PixelTT> object
      typedef types::RGBAPixel<uint16_t> PixelTT;
      typedef types::Image<PixelTT> ImageTT;
      ImageTT image16(rows,cols);
      PixelReader<ImageTT>::readRGBPixels(image16,buffer,true);

      // TODO: I thought I supported arbitrary bitdepths throughout, and processed with arbitrary precision
      //       but it looks like a quantize down to 8 bits regardless of the precision of the source.
      image = image16;
   }
   else {
      std::stringstream ss;
      ss << "Image file \"" << filename << "\" numColors=" << numColors << " unsupported";
      throw std::invalid_argument(ss.str().c_str());
   }

   return image;
}


template<typename PixelT>
types::Image<PixelT> readPGMFile(const std::string& filename) {
   /* PGM Gray-level Image */ 
   if (!utility::endsWith(filename, ".pgm"))
      throw std::invalid_argument("Incorrect File Type, expecting .pgm");

   std::fstream pgm_file;

   pgm_file.open(filename.c_str(), std::ios_base::in | std::ios_base::binary);
   if (!pgm_file.is_open()) {
      std::stringstream ss;
      ss << "Filename \"" << filename << "\" could not be read. Check path or permissions.";
      throw std::invalid_argument(ss.str().c_str());
   }

   unsigned cols = 0, rows = 0, numColors = 0;
   readPNMHeader(pgm_file,filename,rows,cols,numColors,"P5","PPM");

   typedef types::Image<PixelT> ImageT;
   ImageT image(rows,cols);

   // Read grayscale bytes
   if( numColors == 255) {
      unsigned byteSize = rows*cols;
      typedef std::vector<uint8_t> BufferT;
      BufferT buffer(byteSize);
      pgm_file.read(reinterpret_cast<char*>(&buffer[0]), byteSize);
      pgm_file.close();

      // Copy all the image buffer data into the Image<PixelT> object
      PixelReader<ImageT>::readGrayPixels(image,buffer);
   }
   else if (numColors == 65535 ) {
      unsigned elemSize = rows*cols;
      unsigned byteSize = elemSize*2;
      typedef std::vector<uint16_t> BufferT;
      BufferT buffer(elemSize);
      pgm_file.read(reinterpret_cast<char*>(&buffer[0]), byteSize);
      pgm_file.close();

      // Copy all the image buffer data into the Image<PixelTT> object
      typedef types::GrayAlphaPixel<uint16_t> PixelTT;
      typedef types::Image<PixelTT> ImageTT;
      ImageTT image16(rows,cols);
      PixelReader<ImageTT>::readGrayPixels(image16,buffer,true);

      image = image16;
   }
   else {
      std::stringstream ss;
      ss << "Image file \"" << filename << "\" numColors=" << numColors << " unsupported";
      throw std::invalid_argument(ss.str().c_str());
   }

   return image;
}


template<typename ImageT>
void writePPMFile(const std::string& filename,const ImageT& image) {
   if (!utility::endsWith(filename, ".ppm"))
      throw std::invalid_argument("Incorrect file extension, expecting .ppm");

   std::ofstream outfile;
   outfile.open(filename.c_str(), std::ios_base::out | std::ios_base::binary);
   if (!outfile.is_open()) {
      std::stringstream ss;
      ss << "Filename \"" << filename << "\" could not be written. Check path or permissions.";
      throw std::invalid_argument(ss.str().c_str());
   }

   outfile << "P6\n";
   outfile << image.cols() << " " << image.rows() << "\n";
   outfile << "255\n";

   PixelWriter<ImageT>::writeRGBPixels(outfile,image);

   outfile.close();
}



template<unsigned channel,typename ImageT>
void writePGMFile(const std::string& filename,const ImageT& image) {
   if (!utility::endsWith(filename, ".pgm"))
      throw std::invalid_argument("Incorrect file extension, expecting .pgm");

   std::fstream outfile;
   outfile.open(filename.c_str(), std::ios_base::out | std::ios_base::binary);
   if (!outfile.is_open()) {
      std::stringstream ss;
      ss << "Filename \"" << filename << "\" could not be written. Check path or permissions.";
      throw std::invalid_argument(ss.str().c_str());
   }

   outfile << "P5\n";
   outfile << image.cols() << " " << image.rows() << "\n";
   outfile << "255\n";

   PixelWriter<ImageT>::writeGrayPixels(outfile,image,channel);

   outfile.close();
}


} // namespace io
} // namespace batchIP

