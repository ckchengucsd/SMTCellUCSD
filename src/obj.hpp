#pragma once
#include <iostream>
#include <string>
#include <vector>

/*
    Contains Pin objects, Net objects, Instance objects
*/
class Pin {
  /*
    Netlist Objects: Pin
  */
public:
  // net related PinIO constants
  static std::string SOURCE_NET;
  static std::string SINK_NET;

  // pin related PinType constants
  static std::string SOURCE;
  static std::string DRAIN;
  static std::string GATE;

  Pin(std::string pinName, std::string netID, std::string pinIO, int pinLength,
      int pinXpos, std::vector<int> pinYpos, std::string instID,
      std::string pinType);

  ~Pin(){};

  // genTestCase
  Pin(int pinID, int netID, std::string instID, std::string pinName,
      std::string pinDirection, int pinlength);

  static std::string keySON;

  std::string getPinName() const { return pinName_; }
  void setPinName(std::string pinName) { pinName_ = pinName; }
  std::string getNetID() const { return netID_; }
  void setNetID(std::string pin_netID) { netID_ = pin_netID; }
  std::string getPinIO() const { return pinIO_; }
  void setPinIO(std::string pin_IO) { pinIO_ = pin_IO; }
  int getPinLength() const { return pinLength_; }
  int getPinXpos() const { return pinXpos_; }
  std::vector<int> getPinYpos() const { return pinYpos_; }
  std::string getInstID() const { return instID_; }
  std::string getPinType() const { return pinType_; }
  void setPinType(std::string pin_type) { pinType_ = pin_type; }
  // genTestCase
  int getPinID() const { return pinID_; }
  int getNet_id() const { return net_id_; }
  std::string getPinDirection() { return pinDirection_; };
  // hash function
  static unsigned int hash(char *str);
  void dump();

private:
  std::string pinName_;
  std::string netID_;
  std::string pinIO_;
  int pinLength_;
  int pinXpos_;
  std::vector<int> pinYpos_;
  std::string instID_;
  std::string pinType_;
  // genTestCase
  int pinID_;
  int net_id_; // TODO: clean this up with netID_
  std::string pinDirection_;
};

class Net {
  /*
    Netlist Objects: Net
  */
public:
  static std::string NetSON;
  Net(std::string netName, std::string netID, int N_pinNets,
      std::string source_ofNet, int numSinks,
      std::vector<std::string> sinks_inNet,
      std::vector<std::string> pins_inNet);
  // genTestCase
  Net(int netID, int NpinNet, std::string PinList);

  ~Net(){};

  std::string getNetName() const { return netName_; }
  std::string getNetID() const { return netID_; }
  int getN_pinNets() const { return N_pinNets_; }
  std::string getSource_ofNet() const { return source_ofNet_; }
  int getNumSinks() const { return numSinks_; }
  std::string getSinks_inNet(int idx) const { return sinks_inNet_[idx]; }
  std::string getPins_inNet(int idx) const { return pins_inNet_[idx]; }

  void setSource_ofNet(std::string source) { source_ofNet_ = source; }
  void setSinks_inNet(int idx, std::string sink) { sinks_inNet_[idx] = sink; }
  void setPins_inNet(int idx, std::string pin) { pins_inNet_[idx] = pin; }

  // genTestcase
  int getNet_id() const { return net_id_; }
  int getNpinNet() const { return NpinNet_; }
  std::string getPinList() const { return PinList_; }
  void addNpinNet(int a) { NpinNet_ += a; }
  void setPinList(std::string s) { PinList_ = s; }

  // hash function
  static unsigned int hash(char *str);
  // for debug purpose
  void dump();

private:
  std::string netName_;
  std::string netID_;
  int N_pinNets_;
  std::string source_ofNet_;
  int numSinks_;
  std::vector<std::string> sinks_inNet_;
  std::vector<std::string> pins_inNet_;
  // genTestCase
  int NpinNet_;
  int net_id_;
  std::string PinList_;
};

class Instance {
  /*
    Netlist Objects: P/N Instance
  */
public:
  Instance(std::string instName, std::string instType, int instWidth,
           int instY);
  // for inst_partition
  Instance(std::string instName, std::string instType, int instWidth);

  ~Instance(){};

  // inst type enum
  enum class InstType { NMOS, PMOS };

  std::string getInstName() const { return instName_; }
  std::string getInstType() const { return instType_; }
  int getPartitionGroup() const { return partitionGroup_; }
  int getInstWidth() const { return instWidth_; }
  int getInstY() const { return instY_; }
  // genTestCase
  std::string getInstID() const { return instID_; }
  int getWidth() const { return instWidth_; } // same as above?
  void addWidth(int w) { instWidth_ += w; }

  // hash function
  static unsigned int hash(char *str);
  // debug
  void dump();

private:
  std::string instName_;
  std::string instType_;
  int instWidth_;
  int instY_;
  int partitionGroup_;
  // genTestCase
  std::string instID_;
  int width_;
};