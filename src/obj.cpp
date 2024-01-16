#define FMT_HEADER_ONLY
#include "obj.hpp"
#include <fmt/core.h>
#include <fmt/format.h>

// keySON keyword
std::string Pin::keySON = "pinSON";
// net related PinIO constants
std::string Pin::SOURCE_NET = "s";
std::string Pin::SINK_NET = "t";
// pin related PinType constants
std::string Pin::SOURCE = "S";
std::string Pin::DRAIN = "D";
std::string Pin::GATE = "G";

Pin::Pin(std::string pinName, std::string netID, std::string pinIO,
         int pinLength, int pinXpos, std::vector<int> pinYpos,
         std::string instID, std::string pinType) {
  pinName_ = pinName;
  netID_ = netID;
  pinIO_ = pinIO;
  pinLength_ = pinLength;
  pinXpos_ = pinXpos;
  pinYpos_ = pinYpos;
  instID_ = instID;
  pinType_ = pinType;
}

Pin::Pin(int pinID, int net_id, std::string instID, std::string pinName,
         std::string pinDirection, int pinlength) {
  pinID_ = pinID;
  net_id_ = net_id;
  instID_ = instID;
  pinName_ = pinName;
  pinDirection_ = pinDirection;
  pinLength_ = pinlength;
}

// hash instance name
unsigned int Pin::hash(char *str) {
  unsigned int key = 0;
  unsigned int M = 37;
  unsigned char *p;

  for (p = (unsigned char *)str; *p != '\0'; p++) {
    key = M * key + *p;
  }
  return key;
}

// debug
void Pin::dump() {
  fmt::print("[\n");
  fmt::print("  {},\n", pinName_.c_str());
  fmt::print("  {},\n", netID_);
  fmt::print("  {},\n", pinIO_);
  fmt::print("  {},\n", pinLength_);
  fmt::print("  {},\n", pinXpos_);
  fmt::print("  [\n");
  for (int i : pinYpos_)
    fmt::print("    {},\n", i);
  fmt::print("  ],\n");
  fmt::print("  {},\n", instID_);
  fmt::print("  {},\n", pinType_);
  fmt::print("]\n");
}

std::string Net::NetSON = "Multi";

Net::Net(std::string netName, std::string netID, int N_pinNets,
         std::string source_ofNet, int numSinks,
         std::vector<std::string> sinks_inNet,
         std::vector<std::string> pins_inNet) {
  netName_ = netName;
  netID_ = netID;
  N_pinNets_ = N_pinNets;
  source_ofNet_ = source_ofNet;
  numSinks_ = numSinks;
  sinks_inNet_ = sinks_inNet;
  pins_inNet_ = pins_inNet;
};

Net::Net(int netID, int NpinNet, std::string PinList) {
  net_id_ = netID;
  NpinNet_ = NpinNet;
  PinList_ = PinList;
}

// hash instance name
unsigned int Net::hash(char *str) {
  unsigned int key = 0;
  unsigned int M = 37;
  unsigned char *p;

  for (p = (unsigned char *)str; *p != '\0'; p++) {
    key = M * key + *p;
  }
  return key;
}

// debug
void Net::dump() {
  fmt::print("[\n");
  fmt::print("  {},\n", netName_);
  fmt::print("  {},\n", netID_);
  fmt::print("  {},\n", N_pinNets_);
  fmt::print("  {},\n", source_ofNet_);
  fmt::print("  {},\n", numSinks_);

  fmt::print("  [\n");
  for (std::string i : sinks_inNet_)
    fmt::print("    {},\n", i);
  fmt::print("  ],\n");

  fmt::print("  [\n");
  for (std::string i : pins_inNet_)
    fmt::print("    {},\n", i);
  fmt::print("  ],\n");

  fmt::print("]\n");
}

Instance::Instance(std::string instName, std::string instType, int instWidth,
                   int instY) {
  instName_ = instName;
  instType_ = instType;
  instWidth_ = instWidth;
  instY_ = instY;
  partitionGroup_ = -1;
}

Instance::Instance(std::string instName, std::string instType, int instWidth) {
  instName_ = instName;
  instType_ = instType;
  instWidth_ = instWidth;
  partitionGroup_ = -1;
}

// hash instance name
unsigned int Instance::hash(char *str) {
  unsigned int key = 0;
  unsigned int M = 37;
  unsigned char *p;

  for (p = (unsigned char *)str; *p != '\0'; p++) {
    key = M * key + *p;
  }
  return key;
}

// debug
void Instance::dump() {
  fmt::print("[\n");
  fmt::print("  {},\n", instName_);
  fmt::print("  {},\n", instType_);
  fmt::print("  {},\n", instWidth_);
  fmt::print("  {},\n", instY_);
  fmt::print("]\n");
}