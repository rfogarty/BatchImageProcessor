
#include "StringParse.h"


bool endsWith(const std::string& str, const std::string& suffix) {
   return str.size() >= suffix.size() && 
          0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

// This is a little ugly to try and be resilient to all the PNM formats; 
// this would be much easier in ANTLR!
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
      if(readVal2 = !(ss >> val2).fail()) break;
   } while(ins);

   return readVal2;
}


ParseError::ParseError(const std::string& msg) : mMessage(msg) {}

ParseError::~ParseError() throw() {}

const char* ParseError::what() const throw() { return mMessage.c_str(); }




// TODO: pretty ure I do not need this, but leaving in for now just in case.
///*-----------------------------------------------------------------------**/
//int image::getint(FILE *fp) 
//{
//	int item, i, flag;
//
///* skip forward to start of next number */
//	item  = getc(fp); flag = 1;
//	do {
//
//		if (item =='#') {   /* comment*/
//			while (item != '\n' && item != EOF) item=getc(fp);
//		}
//
//		if (item ==EOF) return 0;
//		if (item >='0' && item <='9') 
//			{flag = 0;  break;}
//
//	/* illegal values */
//		if ( item !='\t' && item !='\r' 
//			&& item !='\n' && item !=',') return(-1);
//
//		item = getc(fp);
//	} while (flag == 1);
//
//
///* get the number*/
//	i = 0; flag = 1;
//	do {
//		i = (i*10) + (item - '0');
//		item = getc(fp);
//		if (item <'0' || item >'9') {
//			flag = 0; break;}
//		if (item==EOF) break; 
//	} while (flag == 1);
//
//	return i;
//}
