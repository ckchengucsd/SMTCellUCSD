#include "layoutParser.hpp"
#include "SMTCell.hpp"

void LayoutParser::parsePinLayout(std::string pinlayout_path,
                                  int Partition_Parameter) {
  // ### Read Inputfile and Build Data Structure
  std::string file_status = "init";

  std::ifstream in;
  in.open(pinlayout_path);
  std::string line;

  std::map<unsigned int, std::string> h_tmp_pin_net;
  int pinIDX = 0;
  int pinTotalIDX = 0;
  std::vector<int> pinYpos; // pinYpos will propagate to next Pin

  while (getline(in, line)) {
    boost::smatch m;
    if (boost::regex_search(line, m, boost::regex("\\===InstanceInfo==="))) {
      file_status = "inst";
    } else if (boost::regex_search(line, m, boost::regex("\\===NetInfo==="))) {
      file_status = "net";
      for (int i = 0; i < SMTCell::getPinCnt(); i++) {
        if (h_tmp_pin_net.find(
                Net::hash(SMTCell::getPin(i)->getNetID().data())) !=
            h_tmp_pin_net.end()) {
          if (SMTCell::getPin(i)->getPinIO() == Pin::SOURCE_NET) {
            h_tmp_pin_net[Net::hash(SMTCell::getPin(i)->getNetID().data())] =
                std::string(h_tmp_pin_net.at(
                    Net::hash(SMTCell::getPin(i)->getNetID().data()))) +
                std::string(" ") +
                std::string(SMTCell::getPin(i)->getPinName());
          } else {
            h_tmp_pin_net[Net::hash(SMTCell::getPin(i)->getNetID().data())] =
                std::string(SMTCell::getPin(i)->getPinName()) +
                std::string(" ") +
                std::string(h_tmp_pin_net.at(
                    Net::hash(SMTCell::getPin(i)->getNetID().data())));
          }
        } else {
          h_tmp_pin_net[Net::hash(SMTCell::getPin(i)->getNetID().data())] =
              SMTCell::getPin(i)->getPinName();
        }
      }
    } else if (boost::regex_search(line, m, boost::regex("\\===PinInfo==="))) {
      file_status = "pin";
    } else if (boost::regex_search(line, m,
                                   boost::regex("\\===PartitionInfo==="))) {
      file_status = "partition";
    } else if (boost::regex_search(line, m,
                                   boost::regex("\\===SpecialNetInfo==="))) {
      // This is only used in Daeyeal's code
      file_status = "specialnet";
    } else if (boost::regex_search(line, m,
                                   boost::regex("\\===SDBCellInfo==="))) {
      file_status = "sdbcell";
    } else if (boost::regex_search(line, m,
                                   boost::regex("\\===M2TrackAssignInfo==="))) {
      file_status = "track";
    } else if (boost::regex_search(line, m,
                                   boost::regex("\\===NetOptInfo==="))) {
      file_status = "netopt";
    } else if (boost::regex_search(line, m, boost::regex("\\===CostSize==="))) {
      file_status = "cost";
    }

    // ### pinlayout_path Status: init
    if (file_status == "init") {
      if (boost::regex_search(
              line, m, boost::regex("Width of Routing Clip([\\s]+)= (\\d+)"))) {
        SMTCell::setNumTrackV(std::stoi(m[2]));
        fmt::print("a     # Vertical Tracks   = {}\n", SMTCell::getNumTrackV());
      } else if (boost::regex_search(
                     line, m,
                     boost::regex("Height of Routing Clip([\\s]+)= (\\d+)"))) {
        int placementRow = std::stoi(m[2]);
        SMTCell::setPlacementRow(placementRow);
      } else if (boost::regex_search(
                     line, m,
                     boost::regex(
                         "Tracks per Placement Row([\\s]+)= (\\d+(.?)\\d*)"))) {
        SMTCell::setTrackEachRow(std::stof(m[2]));
        fmt::print("a     # Placement Rows = {}\n",
                   static_cast<int>(std::round(std::stof(m[2]))));
        SMTCell::setNumTrackH(SMTCell::getPlacementRow() *
                              SMTCell::getTrackEachRow());
        fmt::print("a     # Horizontal Tracks = {}\n", SMTCell::getNumTrackH());
      } else if (boost::regex_search(
                     line, m,
                     boost::regex("Width of Placement Clip([\\s]+)= (\\d+)"))) {
        SMTCell::setNumPTrackV(std::stoi(m[2]));
        fmt::print("a     # Vertical Placement Tracks   = {}\n",
                   SMTCell::getNumPTrackV());
      } else if (boost::regex_search(
                     line, m,
                     boost::regex(
                         "Tracks per Placement Clip([\\s]+)= (\\d+)"))) {
        SMTCell::setNumPTrackH(std::stoi(m[2]) * 2);
        SMTCell::setTrackEachPRow(std::stoi(m[2]));
        fmt::print("a     # Horizontal Placement Tracks = {}\n",
                   SMTCell::getNumPTrackH());
      }
    }

    // ### pinlayout_path Status: Instance Info
    if (file_status == "inst") {
      if (boost::regex_search(
              line, m, boost::regex("i   ins([\\S]+) ([\\S]+) ([\\d]+)"))) {
        std::string instName = std::string("ins") + std::string(m[1]);
        std::string instType = m[2];
        int instWidth = std::stoi(m[3]);
        int instY = 0;

        std::vector<int> tmp_finger = SMTCell::getAvailableNumFinger(
            instWidth, SMTCell::getTrackEachPRow());

        if (instType == "NMOS") {
          if (SMTCell::getLastIdxPMOS() == -1) {
            SMTCell::setLastIdxPMOS(SMTCell::getNumInstance() - 1);
          }
          instY = 0;
        } else {
          instY = SMTCell::getNumPTrackH() - instWidth / tmp_finger.at(0);
        }
        // [instName] [instType] [instWidth] [instY]

        SMTCell::addInst(instName, instType, instWidth, instY);

        // ### Generate Maximum possible pin arrays for each instances
        // ### # of Maximum Possible pin = instWidth * 2 + 1
        for (int i = 0; i < tmp_finger.back() * 2 + 1; i++) {
          if (i == 0) {

            SMTCell::setStartPinIndex(instName, pinIDX);
          }

          pinYpos.clear();
          for (int pinRow = 1; pinRow < SMTCell::getTrackEachPRow() + 1;
               pinRow++) {
            pinYpos.push_back(pinRow);
          }

          // [PIN_NAME][NET_ID][pinIO][PIN_LENGTH][pinXpos][@pinYpos][INST_ID][PIN_TYPE]
          std::string tmp_pinName = std::string("pin") + std::string(m[1]) +
                                    std::string("_") + std::to_string(i);

          SMTCell::addPin(tmp_pinName, "", Pin::SINK_NET,
                          SMTCell::getTrackEachPRow(), i, pinYpos, instName,
                          "");

          SMTCell::setPinIdx(tmp_pinName, pinIDX);

          pinIDX++;
          pinTotalIDX++;
        }

        SMTCell::setInstIdx(instName, SMTCell::getNumInstance());
        // fmt::print(" index {} => name {}\n", SMTCell::getNumInstance(),
        //            instName.c_str());
        // numInstance++;
        SMTCell::incrementNumInstance();
      }
    }

    // ### pinlayout_path Status: pin
    std::string pin_type_IO;
    Instance *tmp_inst;
    if (file_status == "pin") {
      if (boost::regex_search(
              line, m,
              boost::regex("i   pin([\\d]+) net([\\d]+) ([\\S]+) "
                           "([\\S]+) ([\\S]+) ([\\S]+) ([\\S]+)"))) {
        pin_type_IO = m[7];
      }

      if (boost::regex_search(
              line, m,
              boost::regex("i   pin([\\d]+) net([\\d]+) ([\\S]+) "
                           "([\\S]+) ([\\S]+) ([\\S]+)"))) {
        std::string pinName =
            std::string("pin") + std::to_string(std::stoi(m[1]));
        std::string pin_netID =
            std::string("net") + std::to_string(std::stoi(m[2]));
        std::string pin_instID = m[3];
        std::string pin_type = m[4];
        std::string pinIO = m[5];
        int pinLength = std::stoi(m[6]);
        std::vector<int> tmp_finger;

        // use try-catch b/c tmp_inst can be none
        // if none, you want to use the prev fetched tmp_inst
        try {
          tmp_inst = SMTCell::getInst(pin_instID);
          tmp_finger = SMTCell::getAvailableNumFinger(
              tmp_inst->getInstWidth(), SMTCell::getTrackEachPRow());
        } catch (const std::out_of_range &oor) {
          tmp_finger = SMTCell::getAvailableNumFinger(
              tmp_inst->getInstWidth(), SMTCell::getTrackEachPRow());
        }

        if (pin_instID != "ext" && pin_type == Pin::SOURCE) {
          for (int i = 0; i < tmp_finger.back() * 2 + 1; i++) {
            if (i % 4 == 0) {

              SMTCell::setPinNetID(SMTCell::getStartPinIndex(pin_instID) + i,
                                   pin_netID);
              SMTCell::setPinType(SMTCell::getStartPinIndex(pin_instID) + i,
                                  pin_type);

              if (i == 0) {

                SMTCell::setPinIO(SMTCell::getStartPinIndex(pin_instID) + i,
                                  pinIO);
              }
            }
          }
        } else if (pin_instID != "ext" && pin_type == Pin::DRAIN) {
          for (int i = 0; i < tmp_finger.back() * 2 + 1; i++) {
            if (i >= 2 && (i - 2) % 4 == 0) {

              SMTCell::setPinNetID(SMTCell::getStartPinIndex(pin_instID) + i,
                                   pin_netID);
              SMTCell::setPinType(SMTCell::getStartPinIndex(pin_instID) + i,
                                  pin_type);

              if (i == 2) {

                SMTCell::setPinIO(SMTCell::getStartPinIndex(pin_instID) + i,
                                  pinIO);
              }
            }
          }
        } else if (pin_instID != "ext" && pin_type == Pin::GATE) {
          for (int i = 0; i < tmp_finger.back() * 2 + 1; i++) {
            if (i >= 1 && i % 2 == 1) {

              SMTCell::setPinNetID(SMTCell::getStartPinIndex(pin_instID) + i,
                                   pin_netID);
              SMTCell::setPinType(SMTCell::getStartPinIndex(pin_instID) + i,
                                  pin_type);

              if (i == 1) {

                SMTCell::setPinIO(SMTCell::getStartPinIndex(pin_instID) + i,
                                  pinIO);
              }
            }
          }
        } else if (pin_instID == "ext") {

          int pinXpos = -1; // never set to any meaningful value
          SMTCell::addPin(pinName, pin_netID, pinIO, pinLength, pinXpos,
                          pinYpos, pin_instID, pin_type);

          SMTCell::addOutPinIdx(pinName, pinTotalIDX);

          pinTotalIDX++;

          if (pin_type != "VDD" && pin_type != "VSS") {
            // h_extnets[std::stoi(m[2])] = 1;

            SMTCell::addExtNet(std::stoi(m[2]));
          } else {
            // not used, so ignore
            // h_pwrnets[std::stoi(m[2])] = 1;

            // SMTCell::addPwrNet(std::stoi(m[2]));
          }

          if (pin_type_IO == "O") {
            // h_outnets[pin_netID] = 1;
            // h_outnets[Net::hash(pin_netID.data())] = 1;
            // partition info
            SMTCell::addOutNet(Net::hash(pin_netID.data()));
          }
        }
        std::string tmp_instance_w_pin =
            std::string(pin_instID) + std::string("_") + std::string(pin_type);

        SMTCell::setInstWPinNetIdx(tmp_instance_w_pin, std::stoi(m[2]));
      }
    }

    std::vector<std::string> tmp_netlist;
    // ### pinlayout_path Status: net
    if (file_status == "net") {
      if (boost::regex_search(line, m,
                              boost::regex("i   net([\\S]+) ([\\d]+)PinNet"))) {
        // numNets_org++;
        SMTCell::incrementNumNetsOrg();
        std::string netID = m[1];
        std::string netName = std::string("net") + std::string(netID);
        int powerinNet = 0;
        std::string powerNet = "";

        if (h_tmp_pin_net.find(Net::hash(netName.data())) !=
            h_tmp_pin_net.end()) {
          boost::split(tmp_netlist, h_tmp_pin_net.at(Net::hash(netName.data())),
                       boost::is_any_of(" "), boost::token_compress_on);
          // fmt::print("{}", fmt::join(tmp_netlist, ""));
        } else {
          std::cerr
              << "[ERROR] Parsing Net Info : Net Information is not correct!! ["
              << netName.c_str() << "]\n";
          exit(1);
        }

        for (std::size_t pinIndex_inNet = 0; pinIndex_inNet < tmp_netlist.size();
             pinIndex_inNet++) {
          if (SMTCell::ifOuterPin(tmp_netlist[pinIndex_inNet])) {
            if ((SMTCell::getPin(
                     SMTCell::getOutPinIdx(tmp_netlist[pinIndex_inNet]))
                         ->getPinType() == "VDD" ||
                 SMTCell::getPin(
                     SMTCell::getOutPinIdx(tmp_netlist[pinIndex_inNet]))
                         ->getPinType() == "VSS")) {
              powerinNet = 1;
              powerNet = tmp_netlist[pinIndex_inNet];
            }
          }
        }

        std::vector<std::string> sinks_inNet;
        std::vector<std::string> pins_inNet;
        if (powerinNet == 0) {
          int N_pinNets = tmp_netlist.size();
          pins_inNet.clear();
          int num_outpin = 0;
          for (int pinIndex_inNet = 0; pinIndex_inNet < N_pinNets;
               pinIndex_inNet++) {
            pins_inNet.push_back(tmp_netlist[pinIndex_inNet]);
          }
          std::string source_ofNet = pins_inNet[N_pinNets - 1];

          int numSinks = N_pinNets - 1;

          for (int sinkIndex_inNet = 0; sinkIndex_inNet < numSinks;
               sinkIndex_inNet++) {
            sinks_inNet.push_back(tmp_netlist[sinkIndex_inNet]);
          }
          numSinks = numSinks - num_outpin;

          // nets.push_back(new Net(netName, netID, N_pinNets, source_ofNet,
          //                        numSinks, sinks_inNet, pins_inNet));
          SMTCell::addNet(netName, netID, N_pinNets, source_ofNet, numSinks,
                          sinks_inNet, pins_inNet);
          // idx_nets++;
        } else {
          int subidx_net = 0;

          for (std::size_t pinIndex_inNet = 0; pinIndex_inNet < tmp_netlist.size();
               pinIndex_inNet++) {

            SMTCell::addPowerPin(tmp_netlist[pinIndex_inNet]);

            int N_pinNets = 2;
            pins_inNet.clear();
            // sinks_inNet.clear();

            if (tmp_netlist[pinIndex_inNet] != powerNet) {
              pins_inNet.push_back(powerNet);
              pins_inNet.push_back(tmp_netlist[pinIndex_inNet]);
              std::string source_ofNet = tmp_netlist[pinIndex_inNet];
              int numSinks = 1;

              sinks_inNet.push_back(powerNet);
              auto tmp_netID = std::string(netName) + std::string("_") +
                               std::to_string(subidx_net);
              // nets.push_back(new Net(tmp_netID,
              //                        std::string(netID) + std::string("_") +
              //                            std::to_string(subidx_net),
              //                        N_pinNets, source_ofNet, numSinks,
              //                        sinks_inNet, pins_inNet));

              SMTCell::addNet(tmp_netID,
                              std::string(netID) + std::string("_") +
                                  std::to_string(subidx_net),
                              N_pinNets, source_ofNet, numSinks, sinks_inNet,
                              pins_inNet);

              SMTCell::setPinNetID(SMTCell::getOutPinIdx(powerNet), tmp_netID);
              SMTCell::setPinNetID(SMTCell::getPinIdx(source_ofNet), tmp_netID);
              SMTCell::setPinIO(SMTCell::getPinIdx(source_ofNet),
                                Pin::SOURCE_NET);
              SMTCell::addPwrNet(tmp_netID);

              // idx_nets++;
              subidx_net++;
              // ## Generate Instance Information for applying DDA
              // if (SMTCell::getPin(source_ofNet)->getPinType() == Pin::SOURCE) {
              //   std::vector<int> tmp_finger = SMTCell::getAvailableNumFinger(
              //       SMTCell::getPinInst(source_ofNet)->getInstWidth(),
              //       SMTCell::getTrackEachPRow());
              //   int FlipFlag;
              //   if (tmp_finger[0] % 2 == 0) {
              //     FlipFlag = 2;
              //   }
              // } else if (SMTCell::getPin(source_ofNet)->getPinType() ==
              //            Pin::DRAIN) {
              //   std::vector<int> tmp_finger = SMTCell::getAvailableNumFinger(
              //       SMTCell::getPinInst(source_ofNet)->getInstWidth(),
              //       SMTCell::getTrackEachPRow());
              //   int FlipFlag;
              //   if (tmp_finger[0] % 2 == 1) {
              //     FlipFlag = 1;
              //   } else {
              //     continue;
              //   }
              // }
            }
          }
        }
      }
    }

    // ### pinlayout_path Status: Partition Info
    // if (Partition_Parameter == 1 && file_status == "partition") {
    //   if (boost::regex_search(
    //           line, m, boost::regex("i   ins([\\S]+) ([\\S]+) ([\\d+])"))) {
    //     std::string instName = std::string("ins") + std::string(m[1]);
    //     std::string instType = m[2];
    //     int instGroup = std::stoi(m[3]);

    //     if (!SMTCell::ifInst(instName)) {
    //       std::cerr << "[ERROR] Instance [" << instName.c_str()
    //                 << "] in PartitionInfo not found!!\n";
    //       exit(1);
    //     }

    //     int idx = SMTCell::getInstIdx(instName);
    //     fmt::print("a     [Cell Partitioning] {} => Group {}\n",
    //                instName.c_str(), std::to_string(instGroup).c_str());

    //     SMTCell::addInstPartition(idx, instName, instType, instGroup);
    //   }
    // }

    // ### pinlayout_path Status: Partition Info
    if (Partition_Parameter == 2 && file_status == "partition") {
      if (boost::regex_search(
              line, m, boost::regex("i   ins([\\S]+) ([\\S]+) ([\\S]+)"))) {
        std::string instName = std::string("ins") + std::string(m[1]);
        std::string instType = m[2];
        int instGroup = std::stoi(m[3]);

        if (!SMTCell::ifInst(instName)) {
          std::cerr << "[ERROR] Instance [" << instName.c_str()
                    << "] in PartitionInfo not found!!\n";
          exit(-1);
        }

        int idx = SMTCell::getInstIdx(instName);
        fmt::print("a     [Partitioning] {} [{}] => Group {}\n",
                   instName.c_str(), instType.c_str(), std::to_string(instGroup).c_str());

        SMTCell::addInstPartition(idx, instName, instType, instGroup);
      }
    }

    // ### pinlayout_path Status: Special Net Info
    if (file_status == "specialnet") {
      if (boost::regex_search(line, m, boost::regex("i   net([\\d]+)"))) {
        int netIdx = std::stoi(m[1]);
        fmt::print("a     [Crosstalk Mitigation Net Assign] net {}\n", netIdx);
        SMTCell::addSpecialNet(netIdx);
      }
    }

    // ### pinlayout_path Status: SDB Possible Sell Info
    if (file_status == "sdbcell") {
      if (boost::regex_search(line, m,
                              boost::regex("i   ins([\\S]+) ([\\S]+)"))) {
        std::string instName = std::string("ins") + std::string(m[1]);
        std::string instType = m[2];
        int idx = SMTCell::getInstIdx(instName);

        fmt::print("a     [SDB in Crossover] {}({})\n", instName, instType);
        SMTCell::addSDBInst(idx, instName);
      }
    }

    // ### Infile Status: TrackUsageInfo
    if (file_status == "track") {
      if (boost::regex_search(line, m,
                              boost::regex("i   net([\\d]+)\\s*([\\d+])"))) {
        std::string netIdx = m[1];
        int net_track = std::stoi(m[2]);

        fmt::print(
            "a     [Crosstalk Mitigation Net Assign] net {} => track {}\n",
            netIdx, net_track);

        // prev implementation is redundant, keeping only h_net_track here
        // double linked map
        SMTCell::addSpNet(netIdx);
        SMTCell::addSpTrack(net_track, netIdx);
      }
    }

    // ### Infile Status: NetOptimizationInfo
    if (file_status == "netopt") {
      if (boost::regex_search(line, m,
                              boost::regex("i   net([\\d]+)\\s*([\\S+])"))) {
        int netIdx = std::stoi(m[1]);
        std::string netOpt = m[2];
        SMTCell::addNetOpt(netIdx, netOpt);
      }
    }

    // ### Infile Status: CostSizeHint
    if (file_status == "cost") {
      if (boost::regex_search(line, m, boost::regex("i   (\\d+)"))) {
        int costHint = std::stoi(m[1]);
        SMTCell::setCostHint(costHint);
        fmt::print("a     [Cost Hint] {}\n", costHint);
      }
    }
  }

  // standard cell width for boundary checking
  SMTCell::set_cell_config(
      SMTCell::getOffset(1) +
          SMTCell::getMetalPitch(1) * (SMTCell::getNumTrackV() - 1),
      SMTCell::getOffset(4) +
          SMTCell::getMetalPitch(4) * (SMTCell::getNumTrackH() - 1));

  in.close();
}