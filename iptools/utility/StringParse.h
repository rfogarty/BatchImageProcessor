#pragma once

#include <exception>
#include <istream>
#include <sstream>
#include <string>

// endsWith - checks if a string ends with the suffix
bool endsWith(const std::string& str, const std::string& suffix);


// used to grab metadata row/cols out of PNM format - not really
// a robust function (so hardly deserving to be its own function)
bool readNext2Integers(std::istream& ins,unsigned& val1,unsigned& val2);



///////////////////////////////////////////////////////////////////////////////
// ParseError - exception type that may be thrown if some sort of parsing
//              error occurs.
//
class ParseError : public std::exception {
private:
   std::string mMessage;
public:
   /** Takes a character string describing the error.  */
   explicit ParseError(const std::string& msg);

   virtual ~ParseError() throw();

   virtual const char* what() const throw();
};


// parseWord - pull a specific type out of an inputstream
//             or throw ParseError if unsuccessful.
template<typename ReturnT>
ReturnT parseWord(std::istream& ins) {
   ReturnT retval;
   ins >> retval;
   if(ins.fail()) throw ParseError("Reading string from stream failed");
   return retval;
}


