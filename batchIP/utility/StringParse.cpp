
#include "StringParse.h"


namespace batchIP {
namespace utility {

bool endsWith(const std::string& str, const std::string& suffix) {
   // Extra parans aren't needed but added for reader clarification
   return (str.size() >= suffix.size()) && 
          (0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix));
}

bool startsWith(const std::string& str, const std::string& prefix) {
   // Extra parans aren't needed but added for reader clarification
   return (str.size() >= prefix.size()) && 
          (0 == str.compare(0, prefix.size(), prefix));
}


// This operation is specifically targeted at reading metadata from the
//    PNM format, and is a little ugly, but tries to be resilient to comments. 
//    This would be much easier in a proper tokenizing/parsing tool such as
//    flex/bison or ANTLR
bool readNext2Integers(std::istream& ins,unsigned& val1,unsigned& val2) {
   // Note: a successful read of val2 implies a successful read of val1
   bool readVal2 = false;
   do {
      std::string line;
      std::getline(ins,line);
      std::stringstream ss(line);
      if(!(ss >> val1).fail()) {
         if(!ss.eof()) readVal2 = !(ss >> val2).fail();
         break;
      }
   } while(ins);

   if(ins && !readVal2)
   do {
      std::string line;
      std::getline(ins,line);
      std::stringstream ss(line);
      if((readVal2 = !(ss >> val2).fail())) break;
   } while(ins);

   return readVal2;
}

// This operation is specifically targeted at reading metadata from the
//    PNM format, and is a little ugly, but tries to be resilient to comments.
//    This would be much easier in a proper tokenizing/parsing tool such as
//    flex/bison or ANTLR
bool readNextInteger(std::istream& ins,unsigned& val1) {
   // Note: a successful read of val2 implies a successful read of val1
   do {
      std::string line;
      std::getline(ins,line);
      std::stringstream ss(line);
      if(!(ss >> val1).fail()) return true;
   } while(ins);

   return false;
}

//////////////////////////////////////////////////////////////////////////
// ParseError Implementation
//////////////////////////////////////////////////////////////////////////
ParseError::ParseError(const std::string& msg) : mMessage(msg) {}

ParseError::~ParseError() throw() {}

const char* ParseError::what() const throw() { return mMessage.c_str(); }

} // namespace utility
} // namespace batchIP

