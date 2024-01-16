#define FMT_HEADER_ONLYget_n
#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp>          // Include for boost::split
#include <boost/multiprecision/integer.hpp>          // for returning bit length
#include <boost/regex.hpp>
#include <cmath>
#include <filesystem>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

class LayoutParser {
public:
  void parsePinLayout(std::string pinlayout_path, int Partition_Parameter);
};