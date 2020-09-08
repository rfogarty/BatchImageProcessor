#pragma once

#include "image/image.h"
#include "utility/stringParse.h"
#include <fstream>
#include <string>

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
   pgm_file.close();

   // Copy all the image buffer data into the Image<PixelT> object
   typedef Image<PixelT> ImageT;
   ImageT image(rows,cols);
   
   typename ImageT::iterator tpos = image.begin();
   typename ImageT::iterator tend = image.end();
   BufferT::const_iterator spos = buffer.begin();

   for(;tpos != tend;++tpos) {
      PixelT& pixel = *tpos;
      pixel.namedColor.red = *spos; ++spos;
      pixel.namedColor.green = *spos; ++spos;
      pixel.namedColor.blue = *spos; ++spos;
   }

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
   typedef Image<PixelT> ImageT;
   ImageT image(rows,cols);
   
   typename ImageT::iterator tpos = image.begin();
   typename ImageT::iterator tend = image.end();
   BufferT::const_iterator spos = buffer.begin();

   for(;tpos != tend;++tpos) {
      PixelT& pixel = *tpos;
      pixel.namedColor.gray = *spos; ++spos;
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


