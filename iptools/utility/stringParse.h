#pragma once

#include <istream>
#include <sstream>
#include <string>

bool endsWith(const std::string& str, const std::string& suffix);


// This is a little ugly to try and be resilient to all the PNM formats; 
// this would be much easier in ANTLR!
bool readNext2Integers(std::istream& ins,unsigned& val1,unsigned& val2);


