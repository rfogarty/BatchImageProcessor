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
                   unsigned& rows, unsigned& cols,
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

   // This bit seems a little troublesome (especially if there are extra comment lines)
   std::string line;
   std::getline(ifs,line);
}

template<typename PixelT> 
types::Image<PixelT> readPPMFile(const std::string& filename) {
   /* PPM Color Image */
   if (!utility::endsWith(filename, ".ppm"))
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
   typedef std::vector<uint8_t> BufferT;
   BufferT buffer(byteSize);
   pgm_file.read(reinterpret_cast<char*>(&buffer[0]), byteSize);
   pgm_file.close();

   // Copy all the image buffer data into the Image<PixelT> object
   typedef types::Image<PixelT> ImageT;
   ImageT image(rows,cols);
  
   PixelReader<ImageT>::readRGBPixels(image,buffer);

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
      ss << "Filename \"" << filename << "\" could not be written. Check path or permissions.";
      throw std::invalid_argument(ss.str().c_str());
   }

   unsigned cols = 0, rows = 0;
   readPNMHeader(pgm_file,filename,rows,cols,"P5","PPM");

   // Read grayscale bytes
   unsigned byteSize = rows*cols;
   typedef std::vector<uint8_t> BufferT;
   BufferT buffer(byteSize);
   pgm_file.read(reinterpret_cast<char*>(&buffer[0]), byteSize);
   pgm_file.close();

   // Copy all the image buffer data into the Image<PixelT> object
   typedef types::Image<PixelT> ImageT;
   ImageT image(rows,cols);
   
   PixelReader<ImageT>::readGrayPixels(image,buffer);
   
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

