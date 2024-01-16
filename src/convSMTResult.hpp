#define FMT_HEADER_ONLY
#pragma once
#include <iostream>
#include <string>
#include <fmt/core.h>
#include <fmt/format.h>

struct InstVar {
  int InstID;
  int xPos;
  int yPos;
  int numFinger;
  int flipFlag;
  int width;
  int unitWidth;

  InstVar(int InstID_, int xPos_, int yPos_, int numFinger_, int flipFlag_,
          int width_, int unitWidth_) {
    InstID = InstID_;
    xPos = xPos_;
    yPos = yPos_;
    numFinger = numFinger_;
    flipFlag = flipFlag_;
    width = width_;
    unitWidth = unitWidth_;
  }

  void setxPos(int xPos_) { xPos = xPos_; }
  void setyPos(int yPos_) { yPos = yPos_; }
  void setNumFinger(int numFinger_) { numFinger = numFinger_; }
  void setFlipFlag(int flipFlag_) { flipFlag = flipFlag_; }
  void setWidth(int width_) { width = width_; }
  void setUnitWidth(int unitWidth_) { unitWidth = unitWidth_; }

  std::string dump() {
    return "INST " + std::to_string(InstID) + " " + std::to_string(xPos) + " " +
           std::to_string(yPos) + " " + std::to_string(numFinger) + " " +
           std::to_string(flipFlag) + " " + std::to_string(width) + " " +
           std::to_string(unitWidth) + "\n";
  }
};

struct MetalVar {
  int metalID;
  int fromRow;
  int fromCol;
  int toRow;
  int toCol;
  int netID;

  MetalVar() {}

  MetalVar(int metalID_, int fromRow_, int fromCol_, int toRow_, int toCol_) {
    metalID = metalID_;
    fromRow = fromRow_;
    fromCol = fromCol_;
    toRow = toRow_;
    toCol = toCol_;
    netID = -1;
  }

  // for final metal
  MetalVar(int metalID_, int fromRow_, int fromCol_, int toRow_, int toCol_,
           int netID_) {
    metalID = metalID_;
    fromRow = fromRow_;
    fromCol = fromCol_;
    toRow = toRow_;
    toCol = toCol_;
    netID = netID_;
  }

  void setNetID(int netID_) { netID = netID_; }

  void setFromRow(int fromRow_) { fromRow = fromRow_; }

  void setFromCol(int fromCol_) { fromCol = fromCol_; }

  void setToRow(int toRow_) { toRow = toRow_; }

  void setToCol(int toCol_) { toCol = toCol_; }

  // always indicate fromMetal and toMetal
  unsigned int toInt() const {
    return metalID * 1000000000 + metalID * 100000000 + fromRow * 1000000 +
           fromCol * 10000 + toRow * 100 + toCol;
  }

  std::string dump() const {
    return "METAL " + std::to_string(metalID) + " " + std::to_string(fromRow) +
           " " + std::to_string(fromCol) + " " + std::to_string(toRow) + " " +
           std::to_string(toCol) + " " + std::to_string(netID) + "\n";
  }
};

struct ViaVar {
  int fromMetal;
  int toMetal;
  int row;
  int col;

  ViaVar(int fromMetal_, int toMetal_, int row_, int col_) {
    fromMetal = fromMetal_;
    toMetal = toMetal_;
    row = row_;
    col = col_;
  }

  // h_net lookup key to retrieve netID
  // unsigned int getNetIndex() const {
  //   return fromMetal * 1000000000 + toMetal * 100000000 + row * 1000000 +
  //          col * 10000 + row * 100 + col;
  // }

  std::string getNetIndex() const {
    return fmt::format("{}_{}_{}_{}_{}_{}", fromMetal, toMetal, row, col, row,
                      col);
  }

  std::string dump(int netID) const {
    return "VIA " + std::to_string(fromMetal) + " " + std::to_string(toMetal) +
           " " + std::to_string(row) + " " + std::to_string(col) + " " +
           std::to_string(netID) + "\n";
  }
};

struct WireVar {
  int metalID;
  int toMetal; // this is only used for h_all_wires. Might change later
  int fromRow;
  int fromCol;
  int toRow;
  int toCol;

  WireVar(int metalID_, int toMetal_, int fromRow_, int fromCol_, int toRow_,
          int toCol_) {
    metalID = metalID_;
    toMetal = toMetal_;
    fromRow = fromRow_;
    fromCol = fromCol_;
    toRow = toRow_;
    toCol = toCol_;
  }

  // unsigned int toInt() const {
  //   return metalID * 1000000000 + toMetal * 100000000 + fromRow * 1000000 +
  //          fromCol * 10000 + toRow * 100 + toCol;
  // }

  std::string toStr() const {
    return fmt::format("{}_{}_{}_{}_{}_{}", metalID, toMetal, fromRow, fromCol,
                      toRow, toCol);
  }

  std::string dump() {
    if (fromRow > toRow) {
      return "WIRE " + std::to_string(metalID) + " " + std::to_string(toRow) +
             " " + std::to_string(fromCol) + " " + std::to_string(fromRow) +
             " " + std::to_string(toCol) + "\n";
    } else if (fromCol > toCol) {
      return "WIRE " + std::to_string(metalID) + " " + std::to_string(fromRow) +
             " " + std::to_string(toCol) + " " + std::to_string(toRow) + " " +
             std::to_string(fromCol) + "\n";
    } else {
      return "WIRE " + std::to_string(metalID) + " " + std::to_string(fromRow) +
             " " + std::to_string(fromCol) + " " + std::to_string(toRow) + " " +
             std::to_string(toCol) + "\n";
    }
  }
};

struct ViaWireVar {
  int fromMetal;
  int toMetal;
  int row;
  int col;

  ViaWireVar(int fromMetal_, int toMetal_, int row_, int col_) {
    fromMetal = fromMetal_;
    toMetal = toMetal_;
    row = row_;
    col = col_;
  }

  unsigned int toInt() const {
    return fromMetal * 10000000 + toMetal * 1000000 + row * 10000 + col * 100;
  }

  std::string dump() {
    if (fromMetal < toMetal) {
      return "VIA_WIRE " + std::to_string(fromMetal) + " " +
             std::to_string(toMetal) + " " + std::to_string(row) + " " +
             std::to_string(col) + "\n";
    } else if (fromMetal > toMetal) {
      return "VIA_WIRE " + std::to_string(toMetal) + " " +
             std::to_string(fromMetal) + " " + std::to_string(row) + " " +
             std::to_string(col) + "\n";
    } else {
      std::cout << "ERROR: fromMetal == toMetal" << std::endl;
      exit(-1);
    }
  }
};

struct NetVar {
  int fromMetal;
  int toMetal;
  int fromRow;
  int fromCol;
  int toRow;
  int toCol;

  NetVar(int fromMetal_, int toMetal_, int fromRow_, int fromCol_, int toRow_,
         int toCol_) {
    fromMetal = fromMetal_;
    toMetal = toMetal_;
    fromRow = fromRow_;
    fromCol = fromCol_;
    toRow = toRow_;
    toCol = toCol_;
  }

  // unsigned int toInt() const {
  //   return fromMetal * 1000000000 + toMetal * 100000000 + fromRow * 1000000 +
  //          fromCol * 10000 + toRow * 100 + toCol;
  // }

  std::string toStr() const {
    return fmt::format("{}_{}_{}_{}_{}_{}", fromMetal, toMetal, fromRow,
                      fromCol, toRow, toCol);
  }
};

struct PinVar {
  std::string pinName;
  int row;
  int col;

  PinVar(std::string pinName_, int row_, int col_) {
    pinName = pinName_;
    row = row_;
    col = col_;
  }

  // hash instance name
  unsigned int toInt() {
    char *str = (char *)pinName.c_str();
    unsigned int key = 0;
    unsigned int M = 37;
    unsigned char *p;

    for (p = (unsigned char *)str; *p != '\0'; p++) {
      key = M * key + *p;
    }
    return key;
  }

  std::string dump() {
    return "PIN " + pinName + " " + std::to_string(row) + " " +
           std::to_string(col) + "\n";
  }
};

struct ExtPinVar {
  int netID;
  int metalID;
  int row;
  int col;

  ExtPinVar(int netID_, int metalID_, int row_, int col_) {
    netID = netID_;
    metalID = metalID_;
    row = row_;
    col = col_;
  }

  unsigned int toInt() const {
    return netID * 10000000 + metalID * 1000000 + row * 10000 + col * 100;
  }

  std::string dump(std::string extPinName, std::string extPinType) {
    return "EXTPIN " + std::to_string(netID) + " " + std::to_string(metalID) +
           " " + std::to_string(row) + " " + std::to_string(col) + " " +
           extPinName + " " + extPinType + "\n";
  }
};