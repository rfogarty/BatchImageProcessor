#pragma once

#include <exception>
#include <istream>
#include <sstream>
#include <string>

bool endsWith(const std::string& str, const std::string& suffix);


// This is a little ugly to try and be resilient to all the PNM formats; 
// this would be much easier in ANTLR!
bool readNext2Integers(std::istream& ins,unsigned& val1,unsigned& val2);


class ParseError : public std::exception {
private:
   std::string mMessage;
public:
   /** Takes a character string describing the error.  */
   explicit ParseError(const std::string& msg);

   virtual ~ParseError() throw();

   virtual const char* what() const throw();
};


template<typename ReturnT>
ReturnT parseWord(std::stringstream& ss) {
   ReturnT retval;
   ss >> retval;
   if(ss.fail()) throw ParseError("Reading string from stream failed");
   return retval;
}


