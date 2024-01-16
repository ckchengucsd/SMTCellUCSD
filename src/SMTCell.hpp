#define FMT_HEADER_ONLY
#pragma once
#define FMT_HEADER_ONLY
#include <boost/multiprecision/integer.hpp> // for returning bit length
#include <fmt/core.h>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "graph.hpp"
#include "obj.hpp"

namespace bmp = boost::multiprecision;

class SMTCell {
  /*
    Wrapper Class for utility functions
  */
public:
  // debug mode
  static bool DEBUG_MODE_;

  static int get_debug_checkpoint() { return ++debug_checkpoint_idx; }

  static void debug_variable_assignment();

  SMTCell(){};

  ~SMTCell() {
    for (auto &i : SMTCell::insts) {
      delete i;
    }
    for (auto &e : SMTCell::udEdges) {
      delete e;
    }
    for (auto &o : SMTCell::outerPins) {
      delete o;
    }
  }

  static bool DEBUG() { return DEBUG_MODE_; }

  static void initTrackInfo(const std::string &config_path);

  // Prepartition: Cost Hint
  static void setCostHint(int cost) { costHint = cost; }

  static void writeCostHint(FILE *fp) {
    fmt::print(fp, "; Prepartition: Cost Size Hint\n");
    if (costHint > 0) {
      int lb = (costHint - 2) * getMetalPitch(1);
      int ub = (costHint + 2) * getMetalPitch(1);
      fmt::print(fp, "(assert (bvsge COST_SIZE (_ bv{} {})))\n", lb,
                 getBitLength_numTrackV());
      fmt::print(fp, "(assert (bvsle COST_SIZE (_ bv{} {})))\n", ub,
                 getBitLength_numTrackV());
    }
  }

  static int getCostHint() { return costHint; }

  // Cell Configurations (related to AGR)
  static void set_cell_config(int width, int height) {
    standardCellWidth = width;
    standardCellHeight = height;
  }

  static int getCellWidth() { return standardCellWidth; }

  static int getCellHeight() { return standardCellHeight; }

  // AGR util functions
  // h_metal_numTrackV
  static void assign_h_metal_numTrackV(int metal, int numTrackV) {
    h_metal_numTrackV[metal] = numTrackV;
  }

  static int get_h_metal_numTrackV(int metal) {
    return h_metal_numTrackV[metal];
  }

  static void dump_h_metal_numTrackH(FILE *graphLog) {
    for (auto &it : h_metal_numTrackH) {
      // std::cout << "      "
      //           << "Metal " << it.first << " numTrackH: " << it.second
      //           << std::endl;
      fmt::print(graphLog,
                 "      "
                 "Metal {} numTrackH: {}\n",
                 it.first, it.second);
    }
  }

  // h_metal_numTrackH
  static void assign_h_metal_numTrackH(int metal, int numTrackH) {
    h_metal_numTrackH[metal] = numTrackH;
  }

  static int get_h_metal_numTrackH(int metal) {
    return h_metal_numTrackH[metal];
  }

  static void dump_h_metal_numTrackV(FILE *graphLog) {
    for (auto &it : h_metal_numTrackV) {
      // std::cout << "      "
      //           << "Metal " << it.first << " numTrackV: " << it.second
      //           << std::endl;
      fmt::print(graphLog,
                 "      "
                 "Metal {} numTrackV: {}\n",
                 it.first, it.second);
    }
  }

  // h_metal_row
  static void assign_h_metal_row(int metal, std::vector<int> rows) {
    h_metal_row[metal] = rows;
  }

  static std::vector<int> get_h_metal_row(int metal) {
    return h_metal_row[metal];
  }

  // use numTrackH instead because it is more accurate
  static int get_h_metal_row_cnt(int metal) {
    return h_metal_row[metal].size();
  }

  static int get_h_metal_row_by_idx(int metal, int idx) {
    return h_metal_row[metal][idx];
  }

  static void dump_h_metal_row(FILE *graphLog) {
    for (auto &it : h_metal_row) {
      // std::cout << "      "
      //           << "Metal " << it.first << " rows: ";
      fmt::print(graphLog,
                 "      "
                 "Metal {} rows: ",
                 it.first);
      for (auto &it2 : it.second) {
        // std::cout << it2 << " ";
        fmt::print(graphLog, "{} ", it2);
      }
      // std::cout << std::endl;
      fmt::print(graphLog, "\n");
    }
  }

  // h_metal_col
  static void assign_h_metal_col(int metal, std::vector<int> cols) {
    h_metal_col[metal] = cols;
  }

  static std::vector<int> get_h_metal_col(int metal) {
    return h_metal_col[metal];
  }

  // use numTrackV instead because it is more accurate
  static int get_h_metal_col_cnt(int metal) {
    return h_metal_col[metal].size();
  }

  static int get_h_metal_col_by_idx(int metal, int idx) {
    return h_metal_col[metal][idx];
  }

  static void dump_h_metal_col(FILE *graphLog) {
    for (auto &it : h_metal_col) {
      // std::cout << "      "
      //           << "Metal " << it.first << " cols: ";
      fmt::print(graphLog,
                 "      "
                 "Metal {} cols: ",
                 it.first);
      for (auto &it2 : it.second) {
        // std::cout << it2 << " ";
        fmt::print(graphLog, "{} ", it2);
      }
      // std::cout << std::endl;
      fmt::print(graphLog, "\n");
    }
  }

  // h_metal_row_idx
  static void assign_h_metal_row_idx(int metal,
                                     std::map<int, int> row_indices) {
    h_metal_row_idx[metal] = row_indices;
  }

  static std::map<int, int> get_h_metal_row_idx(int metal) {
    return h_metal_row_idx[metal];
  }

  static int get_h_metal_row_idx(int metal, int row) {
    return h_metal_row_idx[metal][row];
  }

  static bool ifRowExist(int metal, int row) {
    return h_metal_row_idx[metal].find(row) != h_metal_row_idx[metal].end();
  }

  static void dump_h_metal_row_idx(FILE *graphLog) {
    for (auto &it : h_metal_row_idx) {
      // std::cout << "      "
      //           << "Metal " << it.first << " row_idx: ";
      fmt::print(graphLog,
                 "      "
                 "Metal {} row_idx: ",
                 it.first);
      for (auto &it2 : it.second) {
        // std::cout << it2.first << ":" << it2.second << " ";
        fmt::print(graphLog, "{}:{} ", it2.first, it2.second);
      }
      // std::cout << std::endl;
      fmt::print(graphLog, "\n");
    }
  }

  // h_metal_col_idx
  static void assign_h_metal_col_idx(int metal,
                                     std::map<int, int> col_indices) {
    h_metal_col_idx[metal] = col_indices;
  }

  static std::map<int, int> get_h_metal_col_idx(int metal) {
    return h_metal_col_idx[metal];
  }

  static int get_h_metal_col_idx(int metal, int col) {
    return h_metal_col_idx[metal][col];
  }

  static bool ifColExist(int metal, int col) {
    return h_metal_col_idx[metal].find(col) != h_metal_col_idx[metal].end();
  }

  static void dump_h_metal_col_idx(FILE *graphLog) {
    for (auto &it : h_metal_col_idx) {
      // std::cout << "      "
      //           << "Metal " << it.first << " col_idx: ";
      fmt::print(graphLog,
                 "      "
                 "Metal {} col_idx: ",
                 it.first);
      for (auto &it2 : it.second) {
        // std::cout << it2.first << ":" << it2.second << " ";
        fmt::print(graphLog, "{}:{} ", it2.first, it2.second);
      }
      // std::cout << std::endl;
      fmt::print(graphLog, "\n");
    }
  }

  // AGR Utility : merge all unique columns together to favor different loops
  // metal layer will then only use pick the one exist on the layer
  static void mergeAllUniqueCols() {
    std::vector<int> m1Cols = h_metal_col[1];
    std::vector<int> m3Cols = h_metal_col[3];
    h_unique_col.reserve(m1Cols.size() + m3Cols.size());
    h_unique_col.insert(h_unique_col.end(), m1Cols.begin(), m1Cols.end());
    h_unique_col.insert(h_unique_col.end(), m3Cols.begin(), m3Cols.end());
  }

  static int get_h_unique_col_by_idx(int idx) { return h_unique_col[idx]; }

  static int get_h_unique_col_cnt() { return h_unique_col.size(); }

  // AGR Utility (temporarily)
  // 04/18/2023: Current implementation abstract rows and assume uniform hori MP
  // getMetalPitch(2) is ONE
  // Real Distance    ==>   how many rows?
  static int row_real_to_idx(int realRow) {
    return (floor(realRow / SMTCell::row_distance));
  }

  // 04/18/2023: Current implementation abstract rows and assume uniform hori MP
  // How many rows?   ==>   Real Distance
  // BUG FLAG: FIXING TO 40 for now
  static int row_idx_to_real(int rowIdx) {
    return (rowIdx >= 0 ? rowIdx : 0) * SMTCell::row_distance;
  }

  // AGR Utility
  // Give an arbitrary real column and the metal, find the cloest column
  static int getNearestCol(int metal, int col) {
    int nearestCol = -1;
    int minDist = std::numeric_limits<int>::max();
    for (auto &it : h_metal_col[metal]) {
      if (abs(it - col) < minDist) {
        minDist = abs(it - col);
        nearestCol = it;
      }
    }
    return nearestCol;
  }

  static int getNearestColIdx(int metal, int col) {
    int nearestCol = -1;
    int minDist = std::numeric_limits<int>::max();
    for (auto &it : h_metal_col[metal]) {
      if (abs(it - col) < minDist) {
        minDist = abs(it - col);
        nearestCol = it;
      }
    }
    return h_metal_col_idx[metal][nearestCol];
  }

  // Give an arbitrary real row and the metal, find the cloest row
  static int getNearestRow(int metal, int row) {
    int nearestRow = -1;
    int minDist = std::numeric_limits<int>::max();
    for (auto &it : h_metal_row[metal]) {
      if (abs(it - row) < minDist) {
        minDist = abs(it - row);
        nearestRow = it;
      }
    }
    return nearestRow;
  }

  static int getNearestRowIdx(int metal, int row) {
    int nearestRow = -1;
    int minDist = std::numeric_limits<int>::max();
    for (auto &it : h_metal_row[metal]) {
      if (abs(it - row) < minDist) {
        minDist = abs(it - row);
        nearestRow = it;
      }
    }
    return h_metal_row_idx[metal][nearestRow];
  }

  // Give an arbitrary real column and the metal, find the cloest column to the
  // right
  static int getNearestColToRight(int metal, int col) {
    int nearestCol = -1;
    for (auto &it : h_metal_col[metal]) {
      if (it >= col) {
        nearestCol = it;
        break;
      }
    }
    if (nearestCol == -1) {
      fmt::print("[ERROR] Cannot find the nearest column to the right of {} in "
                 "metal {}\n",
                 col, metal);
      exit(1);
    }
    return nearestCol;
  }

  static int getNearestColIdxToRIght(int metal, int col) {
    int nearestCol = -1;
    for (auto &it : h_metal_col[metal]) {
      if (it >= col) {
        nearestCol = it;
        break;
      }
    }
    if (nearestCol == -1) {
      fmt::print("[ERROR] Cannot find the nearest column to the right of {} in "
                 "metal {}\n",
                 col, metal);
      exit(1);
    }
    return h_metal_col_idx[metal][nearestCol];
  }

  // Give an arbitrary real column and the metal, find the cloest column to the
  // left
  static int getNearestColToLeft(int metal, int col) {
    int nearestCol = -1;
    for (auto &it : h_metal_col[metal]) {
      if (it <= col) {
        nearestCol = it;
      }
    }
    if (nearestCol == -1) {
      fmt::print("[ERROR] Cannot find the nearest column to the left of {} in "
                 "metal {}\n",
                 col, metal);
      exit(1);
    }
    return nearestCol;
  }

  static int getNearestColIdxToLeft(int metal, int col) {
    int nearestCol = -1;
    for (auto &it : h_metal_col[metal]) {
      if (it <= col) {
        nearestCol = it;
      }
    }
    if (nearestCol == -1) {
      fmt::print("[ERROR] Cannot find the nearest column to the left of {} in "
                 "metal {}\n",
                 col, metal);
      exit(1);
    }
    return h_metal_col_idx[metal][nearestCol];
  }

  // Give an arbitrary real row and the metal, find the cloest row to the back
  static int getNearestRowToBack(int metal, int row) {
    int nearestRow = -1;
    for (auto &it : h_metal_row[metal]) {
      if (it >= row) {
        nearestRow = it;
        break;
      }
    }
    if (nearestRow == -1) {
      fmt::print(
          "[ERROR] Cannot find the nearest row to the back of {} in metal {}\n",
          row, metal);
      exit(1);
    }
    return nearestRow;
  }

  static int getNearestRowIdxToBack(int metal, int row) {
    int nearestRow = -1;
    for (auto &it : h_metal_row[metal]) {
      if (it >= row) {
        nearestRow = it;
        break;
      }
    }
    if (nearestRow == -1) {
      fmt::print(
          "[ERROR] Cannot find the nearest row to the back of {} in metal {}\n",
          row, metal);
      exit(1);
    }
    return h_metal_row_idx[metal][nearestRow];
  }

  // Give an arbitrary real row and the metal, find the cloest row to the front
  static int getNearestRowToFront(int metal, int row) {
    int nearestRow = -1;
    for (auto &it : h_metal_row[metal]) {
      if (it <= row) {
        nearestRow = it;
      }
    }
    if (nearestRow == -1) {
      fmt::print("[ERROR] Cannot find the nearest row to the front of {} in "
                 "metal {}\n",
                 row, metal);
      exit(1);
    }
    return nearestRow;
  }

  static int getNearestRowIdxToFront(int metal, int row) {
    int nearestRow = -1;
    for (auto &it : h_metal_row[metal]) {
      if (it <= row) {
        nearestRow = it;
      }
    }
    if (nearestRow == -1) {
      fmt::print("[ERROR] Cannot find the nearest row to the front of {} in "
                 "metal {}\n",
                 row, metal);
      exit(1);
    }
    return h_metal_row_idx[metal][nearestRow];
  }

  // num of track related
  static int getPlacementRow() { return placementRow_; };

  static int getTrackEachPRow() { return trackEachPRow_; };

  static float getTrackEachRow() { return trackEachRow_; };

  static int getNumFin() {
    if (numTrack_ == 4) {
      return 2;
    } else if (numTrack_ == 5) {
      return 3;
    } else if (numTrack_ == 6) {
      return 3;
    }
  };

  static int getNumTrack() { return numTrack_; }

  static int getNumTrackH() { return numTrackH_; };

  static int getNumTrackV() { return numTrackV_; };

  static int getNumMetalLayer() { return numMetalLayer_; };

  static int getNumPTrackH() { return numPTrackH_; };

  static int getNumPTrackV() { return numPTrackV_; };

  static int getFirstRoutingLayer() { return 2; }

  static void setPlacementRow(int placementRow);

  static void setTrackEachPRow(int trackEachPRow);

  static void setTrackEachRow(float trackEachRow);

  static void setNumTrackH(int numTrackH);

  static void setNumTrackV(int numTrackV);

  static void setNumMetalLayer(int numMetalLayer);

  static void setNumPTrackH(int numPTrackH);

  static void setNumPTrackV(int numPTrackV);

  // for retrieving the real track height (w/o power rails)
  static int getTrackHeight() {
    if (numTrack_ == 4) {
      return 3;
    } else if (numTrack_ == 5 || numTrack_ == 6) {
      return 4;
    } else {
      fmt::print("[ERROR] numTrack_ is not 4 or 5");
      exit(-1);
    }
  }

  static int getBitLength_numTrackV();

  static int getBitLength_numTrackH();

  static int getBitLength_numPTrackH();

  static int getBitLength_trackEachPRow();

  static void setLastIdxPMOS(int lastIdxPMOS) { lastIdxPMOS_ = lastIdxPMOS; }

  static int getLastIdxPMOS() { return lastIdxPMOS_; }

  // static int getLastRow() { return numTrackH_ - 3; }

  // static int getLastColIdx() { return numTrackV_ - 1; }

  // for PMOS/NMOS row specific iteration
  // static int getPMOSFirstRow() { return 0; }

  // static int getPMOSLastRow() { return numTrackH_ / 2 - 2; }

  // static int getNMOSFirstRow() { return numTrackH_ / 2 - 1; }

  // static int getNMOSLastRow() { return getLastRow(); }

  static void incrementNumNetsOrg() { numNets_org_++; }

  static int getNumNetsOrg() { return numNets_org_; }

  // PMOS/NMOS/Instance count
  static int getNumPMOS() { return lastIdxPMOS_ + 1; }

  static int getNumNMOS() { return numInstance_ - lastIdxPMOS_ - 1; }

  static void incrementNumInstance() { numInstance_++; }

  static int getNumInstance() { return numInstance_; }

  // return if PMOS or NMOS based on inst Idx (could be simplified further)
  static bool ifInstPMOS(int instIdx) { return instIdx <= lastIdxPMOS_; }

  static bool ifInstNMOS(int instIdx) { return !(instIdx <= lastIdxPMOS_); }

  // check if pin is PMOS/NMOS
  static bool ifPinInstPMOS(int pinIdx) {
    return getPinInstIdx(pinIdx) <= lastIdxPMOS_;
  }

  static bool ifPinInstPMOS(std::string pinName) {
    return getPinInstIdx(pinName) <= lastIdxPMOS_;
  }

  static bool ifPinInstPMOS(std::shared_ptr<Pin> p) {
    return getPinInstIdx(p) <= lastIdxPMOS_;
  }

  static bool ifPinInstNMOS(int pinIdx) {
    return !(getPinInstIdx(pinIdx) <= lastIdxPMOS_);
  }

  static bool ifPinInstNMOS(std::string pinName) {
    return !(getPinInstIdx(pinName) <= lastIdxPMOS_);
  }

  static bool ifPinInstNMOS(std::shared_ptr<Pin> p) {
    return !(getPinInstIdx(p) <= lastIdxPMOS_);
  }

  // Cell Object related
  // ######################### Pin related
  // static void addPin(Pin *pin_obj) { pins.push_back(pin_obj); }
  static void addPin(std::shared_ptr<Pin> pin_obj) { pins.push_back(pin_obj); }

  // overload to directly add pin
  static void addPin(std::string pinName, std::string netID, std::string pinIO,
                     int pinLength, int pinXpos, std::vector<int> pinYpos,
                     std::string instID, std::string pinType) {
    pins.push_back(std::make_shared<Pin>(pinName, netID, pinIO, pinLength,
                                         pinXpos, pinYpos, instID, pinType));
  }

  // retrieve pin obj by pin idx
  // static Pin *getPin(int pinIdx) { return pins[pinIdx]; }
  static std::shared_ptr<Pin> getPin(int pinIdx) { return pins[pinIdx]; }

  // retrieve pin obj by pinName
  // static Pin *getPin(std::string pinName) {
  //   return pins[h_pinName_to_idx[Pin::hash(pinName.data())]];
  // }
  static std::shared_ptr<Pin> getPin(std::string pinName) {
    return pins[h_pinName_to_idx[Pin::hash(pinName.data())]];
  }

  static std::shared_ptr<Pin> popLastPin() {
    // Pin *tmp_pin = pins.back();
    std::shared_ptr<Pin> tmp_pin = pins.back();
    pins.pop_back();
    return tmp_pin;
  }

  static int getPinCnt() { return pins.size(); }

  // set pin NetID
  static void setPinNetID(int pinIdx, std::string pin_netID) {
    pins[pinIdx]->setNetID(pin_netID);
  }

  // set pin type
  static void setPinType(int pinIdx, std::string pin_type) {
    pins[pinIdx]->setPinType(pin_type);
  }

  // set pin type
  static void setPinIO(int pinIdx, std::string pinIO) {
    pins[pinIdx]->setPinIO(pinIO);
  }

  static void addOutPinIdx(std::string pinName, int pinIdx) {
    h_outpinId_idx[Pin::hash(pinName.data())] = pinIdx;
  }

  static int getOutPinIdx(std::string pinName) {
    return h_outpinId_idx[Pin::hash(pinName.data())];
  }

  static bool ifOuterPin(std::string pinName) {
    return h_outpinId_idx.find(Pin::hash(pinName.data())) !=
           h_outpinId_idx.end();
  }

  // if powerpin, assign to 1, no need to retrieve
  static void addPowerPin(std::string pinName) {
    h_pin_power[Pin::hash(pinName.data())] = 1;
  }

  static bool ifPowerPin(std::string pinName) {
    return h_pin_power.find(Pin::hash(pinName.data())) != h_pin_power.end();
  }

  // pinName to pin index in pins
  static int getPinIdx(std::string pinName) {
    return h_pinName_to_idx.at(Pin::hash(pinName.data()));
  }

  static void setPinIdx(std::string pinName, int pinIdx) {
    h_pinName_to_idx[Pin::hash(pinName.data())] = pinIdx;
  }
  //   h_pinName_to_idx[Pin::hash(pinName.data())] = pinIdx;
  // }

  static void removePowerPin() {
    std::vector<std::shared_ptr<Pin>> tmp_pins;
    std::vector<Net *> nets_sorted;

    int pinIDX = 0;

    // for (Pin *p : pins) {
    for (const auto &p : pins) {
      if (!ifPowerPin(p->getPinName())) {
        // tmp_pins.push_back(p);
        tmp_pins.push_back(p);
        setPinIdx(p->getPinName(), pinIDX);

        if (p->getPinType() != Pin::SOURCE && p->getPinType() != Pin::DRAIN &&
            p->getPinType() != Pin::GATE) {
          addOutPinIdx(p->getPinName(), pinIDX);
        }
        pinIDX++;
      }
    }
    pins.clear();
    pins = tmp_pins;
  }

  static void removePowerNet() {
    // std::vector<Net *> tmp_nets;
    std::vector<std::shared_ptr<Net>> tmp_nets;

    int tmp_net_idx = 0;
    // for (Net *n : nets) {
    for (const auto &n : nets) {
      if (!ifPwrNet(n->getNetName())) {
        tmp_nets.push_back(n);
        setNetIdx(n->getNetID(), tmp_net_idx);
        tmp_net_idx++;
      }
    }
    nets.clear();
    nets = tmp_nets;
  }
  static void dump_pins() {
    fmt::print("[DEBUG] pins...\n");
    // for (Pin *p : pins) {
    for (const auto &p : pins) {
      p->dump();
    }

    fmt::print("[DEBUG] h_InstPin_to_netID...\n");
    for (auto const &v : h_InstPin_to_netID) {
      fmt::print("  {} => {}\n", v.first, v.second);
    }

    fmt::print("[DEBUG] h_instID_to_pinStart...\n");
    for (auto const &v : h_instID_to_pinStart) {
      fmt::print("  {} => {}\n", v.first, v.second);
    }

    fmt::print("[DEBUG] h_pinName_to_idx...\n");
    for (auto const &v : h_pinName_to_idx) {
      fmt::print("  {} => {}\n", v.first, v.second);
    }
  }

  // ######################### Instance related
  static void addInst(Instance *inst_obj) { insts.push_back(inst_obj); }

  static void addInst(std::string instName, std::string instType, int instWidth,
                      int instY) {
    insts.push_back(new Instance(instName, instType, instWidth, instY));
  }

  static Instance *getInst(int instIdx) { return insts[instIdx]; }

  static Instance *getInst(std::string instName) {
    return insts[h_inst_idx.at(Instance::hash(instName.data()))];
  }

  // retrieve instance obj directly from pinName
  static Instance *getPinInst(std::string pinName) {
    return insts[h_inst_idx.at(
        Instance::hash(getPin(pinName)->getInstID().data()))];
  }

  static Instance *getPinInst(int pinIdx) {
    return insts[h_inst_idx.at(
        Instance::hash(getPin(pinIdx)->getInstID().data()))];
  }

  static int getPinInstIdx(std::string pinName) {
    return h_inst_idx.at(Instance::hash(getPin(pinName)->getInstID().data()));
  }

  static int getPinInstIdx(int pinIdx) {
    return h_inst_idx.at(Instance::hash(getPin(pinIdx)->getInstID().data()));
  }

  static int getPinInstIdx(std::shared_ptr<Pin> p) {
    return h_inst_idx.at(Instance::hash(p->getInstID().data()));
  }

  // get attribute of instance
  static std::string getPinName(int pinIdx) {
    return getPin(pinIdx)->getPinName();
  }

  static std::string getPinType(int pinIdx) {
    return getPin(pinIdx)->getPinType();
  }

  static int getPinInstWidth(int pinIdx) {
    return getPinInst(pinIdx)->getInstWidth();
  }

  static std::string getPinInstID(int pinIdx) {
    return getPinInst(pinIdx)->getInstID();
  }

  // if instance exists
  static bool ifInst(std::string instName) {
    return h_inst_idx.find(Instance::hash(instName.data())) != h_inst_idx.end();
  }

  static bool ifPinInst(int pinIdx) {
    return h_inst_idx.find(Instance::hash(
               getPin(pinIdx)->getInstID().data())) != h_inst_idx.end();
  }

  // Retrieve instance name to first pin of this inst index inside pins
  // From this start index and #finger, iterate all possible pins
  static int getStartPinIndex(std::string instID) {
    return h_instID_to_pinStart.at(Instance::hash(instID.data()));
  }

  static void setStartPinIndex(std::string instName, int pinIdx) {
    h_instID_to_pinStart[Instance::hash(instName.data())] = pinIdx;
  }

  static void setInstIdx(std::string instName, int instIdx) {
    h_inst_idx[Instance::hash(instName.data())] = instIdx;
  }

  static int getInstIdx(std::string instName) {
    return h_inst_idx[Instance::hash(instName.data())];
  }

  static int getInstCnt() { return insts.size(); }

  // ######################### Net related
  // static void addNet(Net *net_obj) { nets.push_back(net_obj); }
  static void addNet(std::shared_ptr<Net> net_obj) { nets.push_back(net_obj); }

  static void addNet(std::string netName, std::string netID, int N_pinNets,
                     std::string source_ofNet, int numSinks,
                     std::vector<std::string> sinks_inNet,
                     std::vector<std::string> pins_inNet) {
    nets.push_back(std::make_shared<Net>(netName, netID, N_pinNets,
                                         source_ofNet, numSinks, sinks_inNet,
                                         pins_inNet));
  }

  // static Net *getNet(int netIdx) { return nets[netIdx]; }
  static std::shared_ptr<Net> getNet(int netIdx) { return nets[netIdx]; }

  static int getNetCnt() { return nets.size(); }

  // this is not the index in nets, but the "net#" in pinlayout
  static void addExtNet(int net_Idx) { h_extnets[net_Idx] = 1; }

  static int getNetIdx(std::string netId) {
    return h_netId_to_netIdx.at(Net::hash(netId.data()));
  }

  static void setNetIdx(std::string netId, int netIdx) {
    h_netId_to_netIdx[Net::hash(netId.data())] = netIdx;
  }

  // from pin

  // temp solution for flow writer
  static std::map<int, int> getExtNet() { return h_extnets; }

  // static void addPwrNet(int net_Idx) { h_net_power[net_Idx] = 1; }

  static void addPwrNet(std::string netID) {
    h_net_power[Net::hash(netID.data())] = 1;
  }

  static bool ifPwrNet(std::string netName) {
    return h_net_power.find(Net::hash(netName.data())) != h_net_power.end();
  }

  static int getExtNetCnt() { return h_extnets.size(); }

  // subject to delete, not used anywhere
  static void addOutNet(int netIdx) { h_outnets[netIdx] = 1; }

  static int getOutNetCnt() { return h_outnets.size(); }

  static int getOutNet(int netIdx) { return h_outnets[netIdx]; }

  static int ifOutNet(std::string netID) {
    return h_outnets.find(Net::hash(netID.data())) != h_outnets.end();
  }

  // pin related instance name (or ext net) + (_S/G/D/A/B/Y) to net id
  static int getInstWPinNetID(std::string instWPin) {
    return h_InstPin_to_netID.at(Instance::hash(instWPin.data()));
  }

  static void setInstWPinNetIdx(std::string instWPin, int netIdx) {
    h_InstPin_to_netID[Instance::hash(instWPin.data())] = netIdx;
  }

  // count variable
  static void cnt(std::string type, int idx);

  // reset var count
  static void reset_var();

  static void reset_cnt();

  static void clear_writeout();

  // hold constraint (does not directly write to .smt2)
  static void writeConstraint(std::string str);

  // read constraint
  static std::string readConstraint();

  // metal direction
  static bool ifHorzMetal(int metalIdx) { return metalIdx % 2 == 0; }

  static bool ifVertMetal(int metalIdx) { return metalIdx % 2 == 1; }

  // S/G/D columns (AGR Version)
  static bool ifSDCol_AGR(int metal, int col) {
    // fmt::print("SD: metal {} col {}.\n", metal, col);
    if (ifColExist(metal, col) == false && DEBUG_MODE_ == true) {
      // fmt::print(stderr,
      //            "a     [WARNING] column {} does not exist in metal {}. "
      //            "(Inside ifSDCol_AGR)\n",
      //            col, metal);
      // exit(-1);
    }
    return get_h_metal_col_idx(metal, col) % 2 == 1;
  }

  static bool ifGCol_AGR(int metal, int col) {
    // fmt::print("G: metal {} col {}.\n", metal, col);
    if (ifColExist(metal, col) == false && DEBUG_MODE_ == true) {
      // fmt::print(stderr,
      //            "a     [WARNING] column {} does not exist in metal {}. "
      //            "(Inside ifGCol_AGR)\n",
      //            col, metal);
      // exit(-1);
    }
    return get_h_metal_col_idx(metal, col) % 2 == 0;
  }

  // set all track related info
  static void set_numTracks(int placementRow, int trackEachRow,
                            int trackEachPRow, int numTrackH, int numTrackV,
                            int numMetalLayer, int numPTrackH, int numPTrackV);

  // Declare a variable
  static bool setVar(std::string varName, int type);

  // Declare a variable without counting
  static bool setVar_wo_cnt(std::string varName, int type);

  static int assignVar(std::string varName) {
    if (!SMTCell::ifAssigned(varName)) {
      // if(!exists($h_assign{$tmp_str}))
      setVar(varName, 2);
      return 1;
    } else if (SMTCell::ifAssignedTrue(varName)) {
      // if(exists($h_assign{$tmp_str}) && $h_assign{$tmp_str} eq 1)
      setVar_wo_cnt(varName, 0);
      return 0;
    }
    return -1;
  }

  // if var in h_assign
  static bool ifAssigned(std::string varName) {
    return SMTCell::h_assign.find(varName) != SMTCell::h_assign.end();
  }

  static int getAssignedCnt() { return h_assign.size(); }

  static int getAssignedNewCnt() { return h_assign_new.size(); }

  // if var in h_assign and = 1, then assert True
  static bool ifAssignedTrue(std::string varName) {
    return (SMTCell::h_assign.find(varName) != SMTCell::h_assign.end() &&
            h_assign.at(varName) == 1);
  }

  // if var in h_assign and = 0, then assert False
  static bool ifAssignedFalse(std::string varName) {
    return (SMTCell::h_assign.find(varName) != SMTCell::h_assign.end() &&
            h_assign.at(varName) == 0);
  }

  // get the assigned value
  static int getAssigned(std::string varName) {
    if (ifAssigned(varName)) {
      return SMTCell::h_assign.at(varName);
    }
    return -1;
  }

  static void assignFalseVar(std::string varName) {
    SMTCell::h_assign[varName] = 0;
  }

  // check h_assign, then assign either to h_assign_new or h_assign
  static void assignTrueVar(std::string varName, int val, bool toNew) {
    // if not exist
    if (SMTCell::h_assign.find(varName) == SMTCell::h_assign.end()) {
      if (toNew) {
        SMTCell::h_assign_new[varName] = val;
      } else {
        SMTCell::h_assign[varName] = val;
      }
    }
  }
  // check h_assign with "$vName\_$udEdges[$edge_out{$vName}[$i]][2]" and assign
  // "N$nets[$netIndex][1]\_C$commodityIndex\_E_$vName\_$udEdges[$edge_out{$vName}[$i]][2]"
  static void assignTrueVar(std::string netID, int commodityIndex,
                            std::string vName1, std::string vName2, int val,
                            bool toNew) {
    // if not exist
    std::string varName = fmt::format("{}_{}", vName1, vName2);

    if (SMTCell::h_assign.find(varName) == SMTCell::h_assign.end()) {
      std::string variable_name =
          fmt::format("N{}_C{}_E_{}_{}", netID, commodityIndex, vName1, vName2);
      if (toNew) {
        SMTCell::h_assign_new[variable_name] = val;
      } else {
        SMTCell::h_assign[variable_name] = val;
      }
    }
  }

  // Merge assignment information
  static void mergeAssignment() {
    if (h_assign_new.size() > 0) {
      for (const auto &assign_pair : h_assign_new) {
        h_assign[assign_pair.first] = h_assign_new.at(assign_pair.first);
      }
      h_assign_new.clear();
    }
  }

  // If variable declared
  static bool ifVar(std::string varName) {
    return SMTCell::h_var.find(varName) != SMTCell::h_var.end();
  }

  static void flushConstraint(FILE *fp) { fmt::print(fp, readConstraint()); }

  static void flushVarAndConstraint(FILE *fp, int BCP_Parameter) {
    // define boolean
    for (auto const &e : h_var) {
      fmt::print(fp, "(declare-const {} Bool)\n", e.first);
    }

    // write constraints
    flushConstraint(fp);

    // assign T/F
    for (auto const &e : h_assign) {
      if (e.second == 1) {
        fmt::print(fp, "(assert (= {} true))\n", e.first);
      }
    }

    // BCP
    if (BCP_Parameter == 0) {
      fmt::print(fp, ";WO BCP\n");
      for (auto const &e : h_assign_new) {
        if (e.second == 1) {
          fmt::print(fp, "(assert (= {} true))\n", e.first);
        } else {
          fmt::print(fp, "(assert (= {} false))\n", e.first);
        }
      }
    }
  }

  static int getVarCnt() { return h_var.size(); }
  static int getAssignCnt() { return h_assign.size(); }
  static int getCandidateAssignCnt() { return h_assign_new.size(); }

  // Retrieve var, clause, literal
  static int getTotalVar();

  static int getTotalClause();

  static int getTotalLiteral();

  // Dump
  static void dump_summary();

  static void dump_h_assign() {
    for (auto const &assign : SMTCell::h_assign) {
      fmt::print("  {} => {}\n", assign.first, assign.second);
    }
  }

  static void dump_h_netId_to_netIdx() {
    for (auto const &assign : SMTCell::h_netId_to_netIdx) {
      fmt::print("  {} => {}\n", assign.first, assign.second);
    }
  }

  // Retrieve finger count based on transistor width
  static std::vector<int> getAvailableNumFinger(int w,
                                                int track_each_placement_row);

  static std::vector<std::vector<int>> combine(std::vector<int> &l,
                                               std::size_t n);
  static std::vector<std::vector<int>> combine_sub(std::vector<int> &l,
                                                   std::size_t n);

  // Track information
  static bool ifRoutingTrack(int row) {
    return h_mapTrack.find(row) != h_mapTrack.end();
  }

  static int getRoutingTrack(int row) { return h_RTrack.at(row); }

  static int getConn(int k) { return h_numConn.at(k); }

  // AGR / Offset Information
  static void setMetalPitch(int metal, int pitch) { h_MP[metal] = pitch; }
  static int getMetalPitch(int metal) { return h_MP.at(metal); }

  static void setOffset(int metal, int offset) { h_offset[metal] = offset; }
  static int getOffset(int metal) { return h_offset.at(metal); }

  // pinSON
  static std::string getKeySON() { return keySON; }

  static bool ifSON(int pinIdx) { return getPinName(pinIdx) == getKeySON(); }

  // retrieve min(PMOS_width, NMOS_width)
  static void setMOSMinWidth() {
    int tmp_minWidth = 0;
    for (int i = 0; i <= getLastIdxPMOS(); i++) {
      std::vector<int> tmp_finger =
          getAvailableNumFinger(getInst(i)->getInstWidth(), getTrackEachPRow());
      tmp_minWidth += 2 * tmp_finger[0];
    }

    minWidth_ = tmp_minWidth;
    tmp_minWidth = 0;

    for (int i = getNumPMOS(); i < getNumInstance(); i++) {
      std::vector<int> tmp_finger =
          getAvailableNumFinger(getInst(i)->getInstWidth(), getTrackEachPRow());
      tmp_minWidth += 2 * tmp_finger[0];
    }

    if (tmp_minWidth > minWidth_) {
      minWidth_ = tmp_minWidth;
    }
  }

  static int getMOSMinWidth() { return minWidth_; }
  // ### Graph Variable
  // Vertex
  // static void addVertex(Triplet key, Vertex *val) { vertices[key] = val; }
  static void addVertex(Triplet key, std::shared_ptr<Vertex> val) {
    vertices[key] = val;
  }

  // static Vertex *getVertex(int metal, int row, int col) {
  //   return vertices[Triplet(metal, row, col)];
  // }
  static std::shared_ptr<Vertex> getVertex(int metal, int row, int col) {
    return vertices[Triplet(metal, row, col)];
  }

  // static Vertex *getVertex(Triplet key) { return vertices[key]; }
  static std::shared_ptr<Vertex> getVertex(Triplet key) {
    return vertices[key];
  }

  static int getVertexCnt() { return vertices.size(); }

  static std::string getVertexName(int metal, int row, int col) {
    return vertices[Triplet(metal, row, col)]->getVName();
  }

  static bool ifVertex(Triplet key) {
    return vertices.find(key) != vertices.end();
  }

  static void dump_vertices() {
    if (DEBUG_MODE_) {
      for (auto v : vertices) {
        fmt::print("{}\n", v.first.getVName());
        v.second->dump();
      }

      std::cout << "      "
                << "All unique cols: ";
      for (auto &it : h_unique_col) {
        std::cout << it << " ";
      }
      std::cout << std::endl;
    }
  }

  // UdEdge
  static void addUdEdge(UdEdge *udedge) { udEdges.push_back(udedge); }

  static UdEdge *getUdEdge(int udEdgeIdx) { return udEdges[udEdgeIdx]; }

  static int getUdEdgeCnt() { return udEdges.size(); }

  static std::string getUdEdgeFromVarName(int udEdgeIdx) {
    return SMTCell::getUdEdge(udEdgeIdx)->getUdEdgeFromVar()->getVName();
  }

  static std::string getUdEdgeToVarName(int udEdgeIdx) {
    return SMTCell::getUdEdge(udEdgeIdx)->getUdEdgeToVar()->getVName();
  }

  static void dump_udEdges() {
    for (auto udEdge : udEdges) {
      udEdge->dump();
    }
  }

  // boundaryVertices
  // static void addBoundaryVertex(Triplet *boundaryVertex) {
  //   boundaryVertices.push_back(boundaryVertex);
  // }
  static void addBoundaryVertex(std::shared_ptr<Triplet> boundaryVertex) {
    boundaryVertices.push_back(boundaryVertex);
  }

  // static Triplet *getBoundaryVertex(int idx) { return boundaryVertices[idx];
  // }
  static std::shared_ptr<Triplet> getBoundaryVertex(int idx) {
    return boundaryVertices[idx];
  }

  static int getBoundaryVertexCnt() { return boundaryVertices.size(); }

  // static std::vector<Triplet *> copyBoundaryVertex() {
  //   std::vector<Triplet *> tmp_subNodes;
  //   tmp_subNodes.assign(boundaryVertices.begin(), boundaryVertices.end());
  //   return tmp_subNodes;
  // }
  static std::vector<std::shared_ptr<Triplet>> copyBoundaryVertex() {
    std::vector<std::shared_ptr<Triplet>> tmp_subNodes;
    tmp_subNodes.assign(boundaryVertices.begin(), boundaryVertices.end());
    return tmp_subNodes;
  }

  static void dump_boundaryVertices() {
    // for (auto bv : boundaryVertices) {
    //   fmt::print("{}\n", bv->getVName());
    // }
    for (const auto &bv : boundaryVertices) {
      fmt::print("{}\n", bv->getVName());
    }
  }

  // outerPins
  static void addOuterPin(OuterPin *outerPin) { outerPins.push_back(outerPin); }

  static OuterPin *getOuterPin(int idx) { return outerPins[idx]; }

  static int getOuterPinCnt() { return outerPins.size(); }

  static void dump_outerPins() {
    for (auto op : outerPins) {
      op->dump();
    }
  }

  static std::string dump_SON() {
    std::string tmp_str = "";
    for (auto op : outerPins) {
      // 0 : Pin number , 1 : net number
      tmp_str += fmt::format("{} ", op->getPinName());
    }
    return tmp_str;
  }

  static std::string dump_SON_detail() {
    std::string tmp_str = "";
    for (auto op : outerPins) {
      // 0 : Net number , 1 : Commodity number
      tmp_str += fmt::format(" {}={} ", op->getNetID(), op->getCommodityInfo());
    }
    return tmp_str;
  }

  // leftCorners
  // static void addLeftCorner(Triplet *leftCorner) {
  //   leftCorners.push_back(leftCorner);
  // }
  static void addLeftCorner(std::shared_ptr<Triplet> leftCorner) {
    leftCorners.push_back(leftCorner);
  }

  // static Triplet *getLeftCorner(int idx) { return leftCorners[idx]; }
  static std::shared_ptr<Triplet> getLeftCorner(int idx) {
    return leftCorners[idx];
  }

  static int getLeftCornerCnt() { return leftCorners.size(); }

  // rightCorners
  // static void addRightCorner(Triplet *rightCorner) {
  //   rightCorners.push_back(rightCorner);
  // }
  static void addRightCorner(std::shared_ptr<Triplet> rightCorner) {
    rightCorners.push_back(rightCorner);
  }

  // static Triplet *getRightCorner(int idx) { return rightCorners[idx]; }
  static std::shared_ptr<Triplet> getRightCorner(int idx) {
    return rightCorners[idx];
  }

  static int getRightCornerCnt() { return rightCorners.size(); }

  // frontCorners
  // static void addFrontCorner(Triplet *frontCorner) {
  // frontCorners.push_back(frontCorner);
  // }
  static void addFrontCorner(std::shared_ptr<Triplet> frontCorner) {
    frontCorners.push_back(frontCorner);
  }

  // static Triplet *getFrontCorner(int idx) { return frontCorners[idx]; }
  static std::shared_ptr<Triplet> getFrontCorner(int idx) {
    return frontCorners[idx];
  }

  static int getFrontCornerCnt() { return frontCorners.size(); }

  // backCorners
  // static void addBackCorner(Triplet *backCorner) {
  //   backCorners.push_back(backCorner);
  // }
  static void addBackCorner(std::shared_ptr<Triplet> backCorner) {
    backCorners.push_back(backCorner);
  }

  // static Triplet *getBackCorner(int idx) { return backCorners[idx]; }
  static std::shared_ptr<Triplet> getBackCorner(int idx) {
    return backCorners[idx];
  }

  static int getBackCornerCnt() { return backCorners.size(); }

  // source
  static void addSource(std::string key, std::shared_ptr<Source> source) {
    // sources[key] = source;
    sources.insert(std::make_pair(key, std::move(source)));
  }

  static std::shared_ptr<Source> getSource(std::string key) {
    return sources[key];
  }

  static int getSourceCnt() { return sources.size(); }

  static bool ifSource(int pinIdx) {
    return ifSource(getPin(pinIdx)->getPinName());
  }

  static bool ifSource(std::string key) {
    return sources.find(key) != sources.end();
  }

  static std::string dump_sources() {
    std::string tmp_str = "";
    for (const auto &s : sources) {
      tmp_str += fmt::format("{} ", s.first);
    }
    return tmp_str;
  }

  // sink
  static void addSink(std::string key, std::shared_ptr<Sink> sink) {
    // sinks[key] = sink;
    sinks.insert(std::make_pair(key, std::move(sink)));
  }

  static std::shared_ptr<Sink> getSink(std::string key) { return sinks[key]; }

  static int getSinkCnt() { return sinks.size(); }

  static bool ifSink(int pinIdx) {
    return ifSink(getPin(pinIdx)->getPinName());
  }

  static bool ifSink(std::string key) { return sinks.find(key) != sinks.end(); }

  static std::string dump_sinks() {
    std::string tmp_str = "";
    for (const auto &s : sinks) {
      tmp_str += fmt::format("{} ", s.first);
    }
    return tmp_str;
  }

  // virtualEdge
  // static void addVirtualEdge(VirtualEdge *virtualEdge) {
  //   virtualEdges.push_back(virtualEdge);
  // }
  static void addVirtualEdge(std::shared_ptr<VirtualEdge> virtualEdge) {
    virtualEdges.push_back(virtualEdge);
  }

  // static VirtualEdge *getVirtualEdge(int idx) { return virtualEdges[idx]; }
  static std::shared_ptr<VirtualEdge> getVirtualEdge(int idx) {
    return virtualEdges[idx];
  }

  static int getVirtualEdgeCnt() { return virtualEdges.size(); }

  static void dump_virtualEdges() {
    // for (auto ve : virtualEdges) {
    //   ve->dump();
    // }
    for (const auto &ve : virtualEdges) {
      ve->dump();
    }
  }

  // edge_in
  static void addEdgeIn(std::string key, int val) {
    edge_in[key].push_back(val);
  }

  static std::vector<int> getEdgeIn(std::string key) { return edge_in[key]; }

  static int getEdgeInCnt() { return edge_in.size(); }

  static bool ifEdgeIn(std::string pName) {
    return edge_in.find(pName) != edge_in.end();
  }

  // edge_out
  static void addEdgeOut(std::string key, int val) {
    edge_out[key].push_back(val);
  }

  static std::vector<int> getEdgeOut(std::string key) { return edge_out[key]; }

  static int getEdgeOutCnt() { return edge_out.size(); }

  static bool ifEdgeOut(std::string pName) {
    return edge_out.find(pName) != edge_out.end();
  }

  // vedge_in
  static void addVEdgeIn(std::string key, int val) {
    vedge_in[key].push_back(val);
  }

  static std::vector<int> getVEdgeIn(std::string key) { return vedge_in[key]; }

  static int getVEdgeInCnt() { return vedge_in.size(); }

  static bool ifVEdgeIn(std::string pName) {
    return vedge_in.find(pName) != vedge_in.end();
  }

  // vedge_out
  static void addVEdgeOut(std::string key, int val) {
    vedge_out[key].push_back(val);
  }

  static std::vector<int> getVEdgeOut(std::string key) {
    return vedge_out[key];
  }

  static int getVEdgeOutCnt() { return vedge_out.size(); }

  static bool ifVEdgeOut(std::string pName) {
    return vedge_out.find(pName) != vedge_out.end();
  }

  static void addInstPartition(int idx, std::string instName,
                               std::string instType, int instGroup) {
    inst_partition.push_back(std::make_tuple(instName, instType, instGroup));
    h_inst_partition[idx] = instGroup;
  }

  static int getInstPartitionGroupIdx(int idx) { return h_inst_partition[idx]; }

  static bool ifInstPartition(int idx) {
    return h_inst_partition.find(idx) != h_inst_partition.end();
  }

  static void sortInstPartition() {
    std::sort(inst_partition.begin(), inst_partition.end(),
              [](const std::tuple<std::string, std::string, int> &a,
                 const std::tuple<std::string, std::string, int> &b) {
                return std::get<2>(a) < std::get<2>(b);
              });
  }

  static int getInstPartitionCnt() { return inst_partition.size(); }

  static std::string getInstPartitionName(int idx) {
    return std::get<0>(inst_partition[idx]);
  }

  static std::string getInstPartitionType(int idx) {
    return std::get<1>(inst_partition[idx]);
  }

  static int getInstPartitionGroup(int idx) {
    return std::get<2>(inst_partition[idx]);
  }

  static void dump_InstPartition() {
    for (auto i : inst_partition) {
      std::cout << std::get<0>(i) << " " << std::get<1>(i) << " "
                << std::get<2>(i) << std::endl;
    }
  }

  static void dump_InstPartitionPMOS() {
    for (auto i : inst_partition_p) {
      std::cout << "[" << std::endl;
      std::cout << "  " << std::get<0>(i) << std::endl;
      std::cout << "  [" << std::endl;
      // print the inst indices
      for (auto j : std::get<1>(i)) {
        std::cout << "  " << j << std::endl;
      }
      std::cout << "  ]" << std::endl;
      // print the minWidth
      std::cout << "  " << std::get<2>(i) << std::endl;
      std::cout << "]" << std::endl;
    }
  }

  static void dump_InstPartitionNMOS() {
    for (auto i : inst_partition_n) {
      std::cout << "[" << std::endl;
      std::cout << "  " << std::get<0>(i) << std::endl;
      std::cout << "  [" << std::endl;
      // print the inst indices
      for (auto j : std::get<1>(i)) {
        std::cout << "  " << j << std::endl;
      }
      std::cout << "  ]" << std::endl;
      // print the minWidth
      std::cout << "  " << std::get<2>(i) << std::endl;
      std::cout << "]" << std::endl;
    }
  }

  // PMOS partition
  static void addInstPartitionPMOS(int groupIdx, std::vector<int> instIdices,
                                   int minWidth) {
    inst_partition_p.push_back(std::make_tuple(groupIdx, instIdices, minWidth));
  }

  static int getPInstPartitionGroupIdx(int idx) {
    return std::get<0>(inst_partition_p[idx]);
  }

  static int getPInstPartitionMinWidth(int idx) {
    return std::get<2>(inst_partition_p[idx]);
  }

  static int getPInstPartitionCnt() { return inst_partition_p.size(); }

  static int getPInstPartitionInstIndicesCnt(int idx) {
    return std::get<1>(inst_partition_p[idx]).size();
  }

  static std::vector<int> getPInstPartitionInstIndices(int idx) {
    return std::get<1>(inst_partition_p[idx]);
  }

  static int getPInstPartitionInstIdx(int idx, int idx2) {
    return std::get<1>(inst_partition_p[idx])[idx2];
  }

  // NMOS partition
  static void addInstPartitionNMOS(int groupIdx, std::vector<int> instIdices,
                                   int minWidth) {
    inst_partition_n.push_back(std::make_tuple(groupIdx, instIdices, minWidth));
  }

  static int getNInstPartitionGroupIdx(int idx) {
    return std::get<0>(inst_partition_n[idx]);
  }

  static int getNInstPartitionMinWidth(int idx) {
    return std::get<2>(inst_partition_n[idx]);
  }

  static int getNInstPartitionCnt() { return inst_partition_n.size(); }

  static std::vector<int> getNInstPartitionInstIndices(int idx) {
    return std::get<1>(inst_partition_n[idx]);
  }

  static int getNInstPartitionInstIdx(int idx, int idx2) {
    return std::get<1>(inst_partition_n[idx])[idx2];
  }

  static int getNInstPartitionInstIndicesCnt(int idx) {
    return std::get<1>(inst_partition_n[idx]).size();
  }

  // for Crosstalk mitigation Track assign [Used in 3F_6T]
  static void addSpecialNet(int netIdx) { h_specialNet[netIdx] = 1; }

  static int getSpecialNetCnt() { return h_specialNet.size(); }

  static std::map<int, int> getSpecialNet() { return h_specialNet; }

  static bool ifSpecialNet(int netIdx) {
    return h_specialNet.find(netIdx) != h_specialNet.end();
  }

  // for Crosstalk mitigation Net assign [Used in 2F_4T, 3F_5T]
  // netTrack is just row number
  static void addSpNet(std::string netIdx) { h_net_track[netIdx] = 1; }

  static int getSpNetCnt() { return h_net_track.size(); }

  static std::map<std::string, int> getSPNet() { return h_net_track; }

  static bool ifSpNet(int netIdx) {
    return h_net_track.find(getNet(netIdx)->getNetID()) != h_net_track.end();
  }

  static void dump_SPNet() {
    for (auto i : h_net_track) {
      std::cout << i.first << " " << i.second << std::endl;
    }
  }

  // for Crosstalk mitigation Track assign [Used in 2F_4T, 3F_5T]
  // netTrack is just row number
  static void addSpTrack(int netTrack, std::string netIdx) {
    if (h_track_net.find(netTrack) == h_track_net.end()) {
      h_track_net[netTrack] = std::vector<std::string>();
    }
    h_track_net[netTrack].push_back(netIdx);
  }

  static int getSpTrackCnt() { return h_track_net.size(); }

  static std::map<int, std::vector<std::string>> getSPTrack() {
    return h_track_net;
  }

  // if this row is used for special net
  static bool ifSpTrack(int netTrack) {
    return h_track_net.find(netTrack) != h_track_net.end();
  }

  static std::vector<std::string> getSpTrackNet(int netTrack) {
    return h_track_net.at(netTrack);
  }

  // if this net and this row is used for special net
  static bool ifSpTrackNet(int netTrack, int netIdx) {
    if (SMTCell::ifSpTrack(netTrack) == false)
      return false;
    // fmt::print ("searching row: {}\n", netTrack);
    for (std::string netID : h_track_net.at(netTrack)) {
      // matching netID which is a std::string
      if (netID == SMTCell::getNet(netIdx)->getNetID()) {
        // fmt::print ("found netID: {}\n", netID);
        return true;
      }
    }
    return false;
  }

  static void dump_SpTrack() {
    for (auto i : h_track_net) {
      std::cout << "[" << std::endl;
      std::cout << "  " << i.first << std::endl;
      std::cout << "  [" << std::endl;
      for (auto j : i.second) {
        std::cout << "  " << j << std::endl;
      }
      std::cout << "  ]" << std::endl;
      std::cout << "]" << std::endl;
    }
  }

  // SDB Possible Cell Info
  static void addSDBInst(int idx, std::string instName) {
    h_sdbInst[idx] = instName;
  }

  static bool ifSDBInst(int idx) {
    return h_sdbInst.find(idx) != h_sdbInst.end();
  }

  // Net Optimization ()
  static void addNetOpt(int netIdx, std::string opt) {
    h_net_opt[netIdx] = opt;
  }

  static std::string getNetOpt(int netIdx) { return h_net_opt.at(netIdx); }

  static int getNetOptCnt() { return h_net_opt.size(); }

private:
  // debug mode checkpoint
  static int debug_checkpoint_idx;

  // hold all constraints to write out to smt2
  static std::string writeout;

  // Placement vs. Routing Horizontal Track Mapping Array [Placement, Routing]
  static std::map<int, int> mapTrack;

  // Cell Configurations (related to AGR)
  static int standardCellWidth;
  static int standardCellHeight;

  // Number of Routing Contact for Each Width of FET
  static std::map<int, int> numContact;
  static std::map<int, int> h_mapTrack;
  static std::map<int, int> h_RTrack;
  static std::map<int, int> h_numConn;
  static std::map<int, int> h_MP;
  static std::map<int, int> h_offset;
  static int row_distance; // temp solution for not having row in real distance

  // pre-partition
  static int costHint;

  // cnt function specific variables
  static int c_v_placement_;
  static int c_v_placement_aux_;
  static int c_v_routing_;
  static int c_v_routing_aux_;
  static int c_v_connect_;
  static int c_v_connect_aux_;
  static int c_v_dr_;
  static int c_c_placement_;
  static int c_c_routing_;
  static int c_c_connect_;
  static int c_c_dr_;
  static int c_l_placement_;
  static int c_l_routing_;
  static int c_l_connect_;
  static int c_l_dr_;

  // store variable declaration and index
  static int idx_var_;
  static int idx_clause_;
  static std::map<std::string, int> h_var;
  static std::map<std::string, int> h_assign;     // pre writeout
  static std::map<std::string, int> h_assign_new; // post writeout

  // Track related info
  static int numFin_;
  static int numTrack_;
  static int placementRow_;
  static float trackEachRow_;
  static int trackEachPRow_;
  static int numTrackH_;
  static int numTrackV_;
  static int numMetalLayer_;
  static int numPTrackH_;
  static int numPTrackV_;
  static int lastIdxPMOS_;
  static int numInstance_;
  static int numNets_org_;

  // AGR related info
  // store numTrackV and numTrackH for each layer
  static std::map<int, int> h_metal_numTrackV;
  static std::map<int, int> h_metal_numTrackH;

  // store [METAL] ==> [ROW]
  static std::map<int, std::vector<int>> h_metal_row;
  // store [METAL] ==> [COL]
  static std::map<int, std::vector<int>> h_metal_col;

  // store [METAL] ==> [ROW] ==> [ROW_IDX]
  static std::map<int, std::map<int, int>> h_metal_row_idx;
  // store [METAL] ==> [COL] ==> [COL_IDX]
  static std::map<int, std::map<int, int>> h_metal_col_idx;

  // store all unique columns
  static std::vector<int> h_unique_col;

  // Cell related info
  static int minWidth_;

  // ### PIN variables
  // static std::vector<Pin *> pins;
  static std::vector<std::shared_ptr<Pin>> pins;
  // (hashed) pin related instance name (or ext net) + (_S/G/D/A/B/Y) to net id
  // h_pin_id
  static std::map<unsigned int, int> h_InstPin_to_netID;
  // (hashed) instance name to first pin of this inst index inside pins. From
  // this start index and #finger, iterate all possible pins
  // h_pin_idx
  static std::map<unsigned int, int> h_instID_to_pinStart;
  // (hashed) pin name to index inside pins
  // h_pinId_idx
  static std::map<unsigned int, int> h_pinName_to_idx;
  // output pins to idx in pins
  static std::map<unsigned int, int> h_outpinId_idx;
  static std::map<unsigned int, std::string> h_pin_net;

  // ### NET variables
  // static std::vector<Net *> nets;
  static std::vector<std::shared_ptr<Net>> nets;
  static std::map<int, int> h_extnets;
  // h_idx
  static std::map<unsigned int, int> h_netId_to_netIdx; // net ID to idx in nets
  static std::map<unsigned int, int> h_outnets;         // not used anywhere

  // ### INSTANCE variables
  static std::vector<Instance *> insts;
  static std::map<unsigned int, int> h_inst_idx;

  // ### Power Net/Pin Info
  // pinName to 1
  static std::map<unsigned int, int> h_pin_power;
  static std::map<unsigned int, int> h_net_power;

  // ### Partition Info
  static std::vector<std::tuple<std::string, std::string, int>> inst_partition;
  static std::map<int, int> h_inst_partition;
  static std::vector<std::tuple<int, std::vector<int>, int>> inst_partition_p;
  static std::vector<std::tuple<int, std::vector<int>, int>> inst_partition_n;
  static std::map<int, std::string> h_sdbInst; // not used for now

  // ### Crosstalk Mitigation Info [Used in 2F_4T, 3F_5T]
  static std::map<std::string, int> h_net_track;
  static std::map<int, std::vector<std::string>> h_track_net;

  // ### Net Optimization (not used for now)
  static std::map<int, std::string> h_net_opt;

  // ### Crossbar Info [Used in 3F_6T]
  static std::map<int, int> h_specialNet;

  // ### Graph Variable
  // static std::map<Triplet, Vertex *> vertices;
  static std::map<Triplet, std::shared_ptr<Vertex>> vertices;
  static std::vector<UdEdge *> udEdges;
  // static std::vector<Triplet *> boundaryVertices;
  static std::vector<std::shared_ptr<Triplet>> boundaryVertices;
  static std::vector<OuterPin *> outerPins;
  // static std::vector<Triplet *> leftCorners;
  // static std::vector<Triplet *> rightCorners;
  // static std::vector<Triplet *> frontCorners;
  // static std::vector<Triplet *> backCorners;
  static std::vector<std::shared_ptr<Triplet>> leftCorners;
  static std::vector<std::shared_ptr<Triplet>> rightCorners;
  static std::vector<std::shared_ptr<Triplet>> frontCorners;
  static std::vector<std::shared_ptr<Triplet>> backCorners;

  static std::map<std::string, std::shared_ptr<Source>> sources;
  static std::map<std::string, std::shared_ptr<Sink>> sinks;
  // static std::vector<Triplet *> subNodes;
  // static std::vector<VirtualEdge *> virtualEdges;
  static std::vector<std::shared_ptr<VirtualEdge>> virtualEdges;
  static std::map<std::string, std::vector<int>> edge_in;
  static std::map<std::string, std::vector<int>> edge_out;
  static std::map<std::string, std::vector<int>> vedge_in;
  static std::map<std::string, std::vector<int>> vedge_out;

  // Super Outer Node Keyword
  static std::string keySON;

  // unsat core id
  static int constraint_id;
};
