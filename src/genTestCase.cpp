#define FMT_HEADER_ONLY
#include "genTestCase.hpp"
#include <algorithm>
#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp>          // Include for boost::split
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>

// ### Pre-processing ##########################################################
// Horizontal Track std::mapping
std::map<int, int> h_mapTrack;
int subcktMode = 0;
int numPTrackH = 2;
float numTrackH = 3;
int numClip = 2;
// Offset for M1 layer (sync w/ genSMTInput)
int sizeOffset;
// input/output files
std::string infile;
std::string workdir;
std::filesystem::path outdir;
std::filesystem::path outfile;
// Std cell subcircuit to extract
std::vector<std::string> Stdcell{"INVx2_ASAP7_75t_R"};
// std::vector<std::string> Stdcell{"AND2_X2"};
// ### DATA Structure ##########################################################
// matched design name
std::string designName;
// if matched, set to extract subckt mode
std::map<int, int> mapTrack;
std::map<unsigned int, int>
    h_inst; // Instance ID --> Instance index in inst_vec
std::map<unsigned int, int> h_nets;        // Net name --> Net index in inst_vec
std::map<int, std::string> h_name_nets;    // Net index in inst_vec --> Net name
std::map<unsigned int, int> h_netcnt;      // P/N_[net index] --> net count
std::map<unsigned int, int> h_nets_source; // Net Source Name --> pin Index
std::map<int, int> h_pinmatch; // match S and D pairs (double linked)
std::map<unsigned int, std::string> h_pintype; // pin Name --> pin Type

std::vector<std::shared_ptr<Instance>> inst_vec;
std::vector<std::shared_ptr<Net>> nets_vec;
std::vector<std::shared_ptr<Pin>> pins_vec;

std::vector<std::pair<std::string, std::string>>
    ext_pins_vec; // <pinName, pinType>
std::vector<std::pair<std::string, std::string>>
    ext_powerpins_vec; // <pinName, pinType>
int idx_inst;
int idx_net;
int idx_pin;
int finMode;

int getTrack(int track) {
  int nTrack = -1;
  for (auto const &x : mapTrack) {
    if (x.first == track) {
      nTrack = x.second;
    }
  }

  if (nTrack == -1) {
    std::cout << "[ERROR] Track Matching Failed. Input Track => " << std::endl;
  }
  return nTrack;
}

// #### Meta Information
int main(int argc, char **argv) {
  // std::cout << argv[0] << std::endl;
  // Take argument from command line
  if (argc != 4) {
    std::cout << "[ERROR] Usage: genTestcase [inputfile_spfile] \
            [numTrackV Offset] [2F/3F Mode], Got "
              << argc << " instead" << std::endl;
    exit(-1);
  } else {
    std::cout << "a   [INFO] Parsing Input: [inputfile_spfile] = " << argv[1]
              << " [numTrackV Offset] = " << argv[2] << std::endl;
    infile = argv[1];
    sizeOffset = std::__cxx11::stoi(argv[2]);
    finMode = std::stoi(argv[3]); // 2F or 3F

    if (finMode != 2 && finMode != 3) {
      std::cerr << "\n*** Error:: FinMode should be 2 or 3";
      std::cerr << "\n   [USAGE]: ./PL_FILE [inputfile_pinLayout] "
                   "[inputfile_config] [2F/3F Mode]\n\n";
      exit(-1);
    }

    // TODO check if infile exists
    std::ifstream f(infile);
    if (!f.good()) {
      std::cout << "[ERROR] Could not find SPICE file " << infile << std::endl;
      exit(-1);
    } else {
      std::cout << "a   Version Info : 1.0 Initial Version\n\
                    a   Generating TestCase pinLayout based on the following files.\n\
                    a     Input Circuit :  "
                << infile << std::endl;
    }
  }

  if (finMode == 2) {
    numPTrackH = 2;
    numTrackH = 3;
    mapTrack = {{1, 1}, {2, 2}, {3, 2}, {4, 1}, {5, 2}, {6, 2}, {7, 2}};
  } else if (finMode == 3) {
    numPTrackH = 3;
    numTrackH = 3.5;
    mapTrack = {{1, 1}, {2, 1}, {3, 1}, {4, 2}, {5, 2}, {6, 3}, {7, 3}};
  }

  // Output Directory Creation
  char temp[200];
  getcwd(temp, 200);
  workdir = temp;
  outdir /= workdir;
  outdir /= "pinLayouts";
  std::string command = "mkdir -p " + outdir.generic_string();
  system(command.c_str());
  std::cout << "a   [INFO] Output Directory Created: " << outdir << std::endl;

  // h_mapTrack: [routing resource] ==> 1
  for (auto const &x : mapTrack) {
    h_mapTrack.insert({x.second, 1});
  }

  // Read Inputfile and Build Data Structure
  std::string line;
  std::ifstream infileStream(infile);
  while (getline(infileStream, line)) {
    // std::cout << line << std::endl;
    std::smatch m;
    // regex matching subcircuit name
    if (std::regex_search(line, m, std::__cxx11::regex("\\.SUBCKT ([\\S]+)"))) {
      // reset data stucture to store new info
      h_inst = {};
      h_nets = {};
      h_netcnt = {};
      h_nets_source = {};
      h_pinmatch = {};
      h_pintype = {};
      idx_inst = 0;
      idx_net = 0;
      idx_pin = 0;
      inst_vec = {};
      nets_vec = {};
      pins_vec = {};
      ext_pins_vec = {};
      ext_powerpins_vec = {};
      designName = m[1];
      // std::cout << "a     [INFO] Found" << designName << std::endl;

      // match input std cells
      if (find(Stdcell.begin(), Stdcell.end(), designName) != Stdcell.end()) {
        std::cout << "a     Cell Design Name :   " << designName << std::endl;
        subcktMode = 1;
        // std::coutput file Creation
        outfile /= outdir;
        outfile /= designName + ".pinLayout";
        std::cout << "a     Writing output to : " << outfile.generic_string()
                  << std::endl;
      }

    } else if (subcktMode == 1 &&
               std::regex_search(line, std::__cxx11::regex("\\*\\.PININFO"))) {
      std::cout << "a     [INFO] " << line << std::endl;
      // NOTE: set to token_compress_on, adjacent separators are merged
      // together. Otherwise, every two separators delimit a token
      std::vector<std::string> tmp_vec;
      boost::split(tmp_vec, line, boost::is_any_of(" "),
                   boost::token_compress_on);

      // iterate through matched non-whitespace std::string
      for (auto tmp_str : tmp_vec) {
        std::string pinName = "";
        std::string pinType = "";
        std::smatch m;
        if (subcktMode == 1 &&
            std::regex_search(tmp_str, m,
                              std::__cxx11::regex("([\\S]+):([\\S]+)"))) {
          pinName = m[1];
          pinType = m[2];

          // pinType has to be either external I/O or power
          if (pinType == "I" || pinType == "O" ||
              (pinType == "B" && pinName == "VDD") ||
              (pinType == "B" && pinName == "VSS")) {

            // temp use for printing
            std::string tmp_print = "";
            if (pinType == "I") {
              tmp_print = "Input";
            } else if (pinType == "O") {
              tmp_print = "Output";
            } else if (pinType == "P") {
              tmp_print = "Power";
            } else {
              tmp_print = "Ground";
            }
            std::cout << "a     [INFO] External Pin Info : " << pinName << " ["
                      << tmp_print << "]" << std::endl;

            // reassign "B" --> "P"
            if (pinType == "B") {
              pinType = "P";
            }

            h_pintype[Pin::hash(pinName.data())] = pinType;

            // if pin Type is I/O, push to I/O pins std::vector or powerpin
            // std::vector respectively
            if (pinType == "I" || pinType == "O") {
              ext_pins_vec.push_back(make_pair(pinName, pinType));
            } else {
              ext_powerpins_vec.push_back(make_pair(pinName, pinType));
            }
          }
        }
      }

    } else if (subcktMode == 1 &&
               std::regex_search(
                   line, m,
                   std::__cxx11::regex(
                       "([MM\\d]+) ([\\S]+) ([\\S]+) ([\\S]+) ([\\S]+) "
                       "([\\S]+) w=([\\S]+) l=([\\S]+) nfin=([\\d]+)"))) {
      // store information for each net
      std::string inst = m[1];
      std::string instID = m[1];
      std::string net_s = m[4];
      std::string net_g = m[3];
      std::string net_d = m[2];
      std::cout << net_s << " * " << net_g << " * " << net_d << std::endl;
      std::string instType = (m[6] == "nmos_rvt" ? "NMOS" : "PMOS");
      int nFin = std::__cxx11::stoi(m[9]);

      if (finMode != 3) {
        // if nFin is less than 3, get the track width
        if (nFin <= 3) {
          nFin = getTrack(nFin);
        } else if (nFin > 3) {
          float mult = ceil(static_cast<float>(nFin) / 3.0);
          nFin = getTrack(3) * mult;
        }
      }

      // if instance not in h_inst
      if (h_inst.find(Instance::hash(inst.data())) == h_inst.end()) {
        // inst_vec.push_back(new Instance(instID, instType, nFin));
        inst_vec.push_back(std::make_shared<Instance>(instID, instType, nFin));
        std::cout << "Inst => ID:" << inst << ", Type:" << instType
                  << ", Width:" << nFin << " -> " << std::to_string(nFin)
                  << std::endl;
        h_inst.insert({Instance::hash(instID.data()), idx_inst});
        idx_inst++;
      } else {
        // otherwise add instance width
        inst_vec.at(h_inst.at(Instance::hash(instID.data())))->addWidth(nFin);
      }

      // add source if not already
      if (h_nets.find(Net::hash(net_s.data())) == h_nets.end()) {
        // nets_vec.push_back(new Net(idx_net, 0, ""));
        nets_vec.push_back(std::make_shared<Net>(idx_net, 0, ""));
        h_nets.insert({Net::hash(net_s.data()), idx_net});
        h_name_nets.insert({idx_net, net_s});
        idx_net++;
      }

      // add gate if not already
      if (h_nets.find(Net::hash(net_g.data())) == h_nets.end()) {
        // nets_vec.push_back(new Net(idx_net, 0, ""));
        nets_vec.push_back(std::make_shared<Net>(idx_net, 0, ""));
        h_nets.insert({Net::hash(net_g.data()), idx_net});
        h_name_nets.insert({idx_net, net_g});
        idx_net++;
      }

      // add drain if not already
      if (h_nets.find(Net::hash(net_d.data())) == h_nets.end()) {
        // nets_vec.push_back(new Net(idx_net, 0, ""));
        nets_vec.push_back(std::make_shared<Net>(idx_net, 0, ""));
        h_nets.insert({Net::hash(net_d.data()), idx_net});
        h_name_nets.insert({idx_net, net_d});
        idx_net++;
      }

      // temp use for net name according to P/NMOS
      std::string tmp_nets_str;
      std::string tmp_netd_str;
      if (instType == "PMOS") {
        tmp_nets_str =
            "P_" + std::to_string(h_nets.at(Net::hash(net_s.data())));
        tmp_netd_str =
            "P_" + std::to_string(h_nets.at(Net::hash(net_d.data())));

      } else {
        tmp_nets_str =
            "N_" + std::to_string(h_nets.at(Net::hash(net_s.data())));
        tmp_netd_str =
            "N_" + std::to_string(h_nets.at(Net::hash(net_d.data())));
      }

      // update source induced net count
      if (h_netcnt.find(Net::hash(tmp_nets_str.data())) == h_netcnt.end()) {
        h_netcnt.insert({Net::hash(tmp_nets_str.data()), 1});
        std::cout << net_s << " " << tmp_nets_str << " "
                  << std::to_string(h_netcnt.at(Net::hash(tmp_nets_str.data())))
                  << std::endl;
      } else {
        h_netcnt[Net::hash(tmp_nets_str.data())] += 1;
      }

      // add drain induced net count
      if (h_netcnt.find(Net::hash(tmp_netd_str.data())) == h_netcnt.end()) {
        h_netcnt.insert({Net::hash(tmp_netd_str.data()), 1});
        std::cout << net_d << " " << tmp_netd_str << " "
                  << std::to_string(h_netcnt.at(Net::hash(tmp_netd_str.data())))
                  << std::endl;
      } else {
        h_netcnt[Net::hash(tmp_netd_str.data())] += 1;
      }

      // ################################ Adding Source
      int isSource = 0;
      // if net source not in h_nets_source
      if (h_nets_source.find(Net::hash(net_s.data())) == h_nets_source.end()) {
        isSource = 1;
        h_nets_source.insert({Net::hash(net_s.data()), idx_pin});
      }

      // store new pin instance
      // pins_vec.push_back(new Pin(
      //     idx_pin,
      //     (h_nets.find(Net::hash(net_s.data())) != h_nets.end())
      //         ? h_nets.at(Net::hash(net_s.data()))
      //         : idx_net,
      //     instID, "S", isSource == 1 ? "s" : "t",
      //     inst_vec.at(h_inst.at(Instance::hash(instID.data())))->getWidth()));
      pins_vec.push_back(std::make_shared<Pin>(
          idx_pin,
          (h_nets.find(Net::hash(net_s.data())) != h_nets.end())
              ? h_nets.at(Net::hash(net_s.data()))
              : idx_net,
          instID, "S", isSource == 1 ? "s" : "t",
          inst_vec.at(h_inst.at(Instance::hash(instID.data())))->getWidth()));

      // update net info
      nets_vec.at(h_nets.at(Net::hash(net_s.data())))->addNpinNet(1);

      // if source pin, append to the end of PinList. Otherwise, append to head
      // of PinList
      if (isSource == 1) {
        std::string temp_str = "";
        if (nets_vec.at(h_nets.at(Net::hash(net_s.data())))->getPinList() !=
            "") {
          temp_str = " ";
        }
        nets_vec.at(h_nets.at(Net::hash(net_s.data())))
            ->setPinList(
                temp_str +
                nets_vec.at(h_nets.at(Net::hash(net_s.data())))->getPinList() +
                "pin" + std::to_string(idx_pin));
      } else {
        nets_vec.at(h_nets.at(Net::hash(net_s.data())))
            ->setPinList(
                "pin" + std::to_string(idx_pin) + " " +
                nets_vec.at(h_nets.at(Net::hash(net_s.data())))->getPinList());
      }

      // temp use for storing S/D pairs later
      int idx_pin_s = idx_pin;
      idx_pin++;

      // ################################ Adding Gate
      isSource = 0;
      // if net source not in h_nets_source
      if (h_nets_source.find(Net::hash(net_g.data())) == h_nets_source.end()) {
        isSource = 1;
        h_nets_source.insert({Net::hash(net_g.data()), idx_pin});
      }

      // store new pin instance
      // pins_vec.push_back(new Pin(
      //     idx_pin,
      //     (h_nets.find(Net::hash(net_g.data())) != h_nets.end())
      //         ? h_nets.at(Net::hash(net_g.data()))
      //         : idx_net,
      //     instID, "G", isSource == 1 ? "s" : "t",
      //     inst_vec.at(h_inst.at(Instance::hash(instID.data())))->getWidth()));
      pins_vec.push_back(std::make_shared<Pin>(
          idx_pin,
          (h_nets.find(Net::hash(net_g.data())) != h_nets.end())
              ? h_nets.at(Net::hash(net_g.data()))
              : idx_net,
          instID, "G", isSource == 1 ? "s" : "t",
          inst_vec.at(h_inst.at(Instance::hash(instID.data())))->getWidth()));

      // update net info
      nets_vec.at(h_nets.at(Net::hash(net_g.data())))->addNpinNet(1);

      // if source pin, append to the end of PinList. Otherwise, append to head
      // of PinList
      if (isSource == 1) {
        //(($nets[$h_nets{$net_s}][2] eq ""?"":"
        //").$nets[$h_nets{$net_s}][2]."pin$idx_pin")
        std::string temp_str = "";
        if (nets_vec.at(h_nets.at(Net::hash(net_g.data())))->getPinList() !=
            "") {
          temp_str = " ";
        }
        nets_vec.at(h_nets.at(Net::hash(net_g.data())))
            ->setPinList(
                temp_str +
                nets_vec.at(h_nets.at(Net::hash(net_g.data())))->getPinList() +
                "pin" + std::to_string(idx_pin));
      } else {
        //("pin$idx_pin ".$nets[$h_nets{$net_s}][2])
        nets_vec.at(h_nets.at(Net::hash(net_g.data())))
            ->setPinList(
                "pin" + std::to_string(idx_pin) + " " +
                nets_vec.at(h_nets.at(Net::hash(net_g.data())))->getPinList());
      }

      idx_pin++;

      // ################################ Adding Drain
      isSource = 0;
      // if net source not in h_nets_source
      if (h_nets_source.find(Net::hash(net_d.data())) == h_nets_source.end()) {
        isSource = 1;
        h_nets_source.insert({Net::hash(net_d.data()), idx_pin});
      }

      // store new pin instance
      // pins_vec.push_back(new Pin(
      //     idx_pin,
      //     (h_nets.find(Net::hash(net_d.data())) != h_nets.end())
      //         ? h_nets.at(Net::hash(net_d.data()))
      //         : idx_net,
      //     instID, "D", isSource == 1 ? "s" : "t",
      //     inst_vec.at(h_inst.at(Instance::hash(instID.data())))->getWidth()));
      pins_vec.push_back(std::make_shared<Pin>(
          idx_pin,
          (h_nets.find(Net::hash(net_d.data())) != h_nets.end())
              ? h_nets.at(Net::hash(net_d.data()))
              : idx_net,
          instID, "D", isSource == 1 ? "s" : "t",
          inst_vec.at(h_inst.at(Instance::hash(instID.data())))->getWidth()));

      // update net info
      nets_vec.at(h_nets.at(Net::hash(net_d.data())))->addNpinNet(1);

      // if source pin, append to the end of PinList. Otherwise, append to head
      // of PinList
      if (isSource == 1) {
        //(($nets[$h_nets{$net_s}][2] eq ""?"":"
        //").$nets[$h_nets{$net_s}][2]."pin$idx_pin")
        std::string temp_str = "";
        if (nets_vec.at(h_nets.at(Net::hash(net_d.data())))->getPinList() !=
            "") {
          temp_str = " ";
        }
        nets_vec.at(h_nets.at(Net::hash(net_d.data())))
            ->setPinList(
                temp_str +
                nets_vec.at(h_nets.at(Net::hash(net_d.data())))->getPinList() +
                "pin" + std::to_string(idx_pin));
      } else {
        //("pin$idx_pin ".$nets[$h_nets{$net_s}][2])
        nets_vec.at(h_nets.at(Net::hash(net_d.data())))
            ->setPinList(
                "pin" + std::to_string(idx_pin) + " " +
                nets_vec.at(h_nets.at(Net::hash(net_d.data())))->getPinList());
      }

      // store S/D pairs
      h_pinmatch.insert({idx_pin_s, idx_pin});
      h_pinmatch.insert({idx_pin, idx_pin_s});
      idx_pin++;

    } else if (subcktMode == 1 &&
               std::regex_search(line, std::__cxx11::regex("\\.ENDS"))) {
      // for (auto const &p : h_netcnt) {
      //   std::cout << p.first.c_str() << " !=> "
      //             << std::to_string(p.second).c_str() << std::endl;
      // }

      subcktMode = 0;

      // Process data
      // Add External Pins
      for (auto tmp_ext_powerpin : ext_powerpins_vec) {
        // store new pin instance
        // pins_vec.push_back(new Pin(
        //     idx_pin, h_nets.at(Net::hash(tmp_ext_powerpin.first.data())),
        //     "ext", tmp_ext_powerpin.first, "t", -1));
        pins_vec.push_back(std::make_shared<Pin>(
            idx_pin, h_nets.at(Net::hash(tmp_ext_powerpin.first.data())), "ext",
            tmp_ext_powerpin.first, "t", -1));
        // update net info
        nets_vec.at(h_nets.at(Net::hash(tmp_ext_powerpin.first.data())))
            ->addNpinNet(1);
        nets_vec.at(h_nets.at(Net::hash(tmp_ext_powerpin.first.data())))
            ->setPinList(
                "pin" + std::to_string(idx_pin) + " " +
                nets_vec
                    .at(h_nets.at(Net::hash(tmp_ext_powerpin.first.data())))
                    ->getPinList());
        idx_pin++;
      }

      for (auto tmp_ext_pin : ext_pins_vec) {
        // store new pin instance
        // pins_vec.push_back(
        //     new Pin(idx_pin, h_nets.at(Net::hash(tmp_ext_pin.first.data())),
        //             "ext", tmp_ext_pin.first, "t", -1));
        pins_vec.push_back(std::make_shared<Pin>(
            idx_pin, h_nets.at(Net::hash(tmp_ext_pin.first.data())), "ext",
            tmp_ext_pin.first, "t", -1));
        // update net info
        nets_vec.at(h_nets.at(Net::hash(tmp_ext_pin.first.data())))
            ->addNpinNet(1);
        nets_vec.at(h_nets.at(Net::hash(tmp_ext_pin.first.data())))
            ->setPinList(
                "pin" + std::to_string(idx_pin) + " " +
                nets_vec.at(h_nets.at(Net::hash(tmp_ext_pin.first.data())))
                    ->getPinList());
        idx_pin++;
      }

      // Compute PMOS/NMOS total size
      int sizeNMOS = 0;
      int sizePMOS = 0;

      for (auto tmp_inst : inst_vec) {
        std::cout << "a     Instance Info : ID => " << tmp_inst->getInstName()
                  << ", Type => " << tmp_inst->getInstType() << ", Width => "
                  << tmp_inst->getWidth() << std::endl;

        if (tmp_inst->getInstType() == "NMOS") {
          sizeNMOS += 2 * ceil(static_cast<float>(tmp_inst->getWidth()) /
                               static_cast<float>(numPTrackH)) +
                      1;
          std::cout << tmp_inst->getWidth() << std::endl;
          std::cout << "sizeNMOS: " << sizeNMOS << std::endl;
        } else {
          sizePMOS += 2 * ceil(static_cast<float>(tmp_inst->getWidth()) /
                               static_cast<float>(numPTrackH)) +
                      1;
          std::cout << tmp_inst->getWidth() << std::endl;
          std::cout << "sizePMOS: " << sizePMOS << std::endl;
        }
      }

      sizeNMOS += sizeOffset;
      sizePMOS += sizeOffset;

      // temp use for std::mapping S/D pin to 1
      std::map<int, int> h_touchedpin = {};

      // print h_pintype
      // fmt::print("h_pintype size: {}\n", h_pintype.size());
      // for (auto const &p : h_pintype) {
      //   std::cout << p.first << " => " << p.second.c_str() << std::endl;
      // }

      for (auto tmp_pin : pins_vec) {
        if (tmp_pin->getInstID() != "ext" && tmp_pin->getPinName() != "G" &&
            h_touchedpin.find(tmp_pin->getPinID()) == h_touchedpin.end()) {

          // retrieve instance type ()
          std::string instType =
              inst_vec
                  .at(h_inst.at(Instance::hash(tmp_pin->getInstID().data())))
                  ->getInstType();
          std::string prefix_type = "";

          if (instType == "PMOS") {
            prefix_type = "P";
          } else {
            prefix_type = "N";
          }

          // current pin net count
          int numCon_cur = h_netcnt.at(Net::hash(
              (prefix_type + "_" + std::to_string(tmp_pin->getNet_id()))
                  .data()));
          // matched pin net count (S->D, D->S)
          int numCon_match = h_netcnt.at(Net::hash(
              (prefix_type + "_" +
               std::to_string(pins_vec.at(h_pinmatch.at(tmp_pin->getPinID()))
                                  ->getNet_id()))
                  .data()));

          // if (h_pintype.at(Pin::hash(
          //         h_name_nets.at(tmp_pin->getNet_id()).data())) == "P") {
          //   numCon_cur = 1;
          // } else if (h_pintype.at(Pin::hash(
          //                h_name_nets
          //                    .at(pins_vec
          //                            .at(h_pinmatch.at(tmp_pin->getPinID()))
          //                            ->getNet_id())
          //                    .data())) == "P") {
          //   numCon_match = 1;
          // }

          // NOTE: Found a bug in original code where h_pinType is not callable
          // with "S/G/D" as keys
          std::cout
              << tmp_pin->getPinID() << " " << tmp_pin->getPinName() << " "
              << std::to_string(numCon_cur) << " vs "
              << pins_vec.at(h_pinmatch.at(tmp_pin->getPinID()))->getPinID()
              << " " << std::to_string(numCon_match) << std::endl;

          if (tmp_pin->getPinName() == "S" && numCon_cur < numCon_match) {
            tmp_pin->setPinName("D");
            pins_vec.at(h_pinmatch.at(tmp_pin->getPinID()))->setPinName("S");
          } else if (tmp_pin->getPinName() == "D" &&
                     numCon_cur > numCon_match) {
            tmp_pin->setPinName("S");
            pins_vec.at(h_pinmatch.at(tmp_pin->getPinID()))->setPinName("D");
          } else if (numCon_cur == numCon_match) {

            // if (h_pintype.at(Pin::hash(
            //         h_name_nets.at(tmp_pin->getNet_id()).data())) == "P") {
            //   tmp_pin->setPinName("S");
            //   pins_vec.at(h_pinmatch.at(tmp_pin->getPinID()))->setPinName("D");
            // } else if (pins_vec.at(h_pinmatch.at(tmp_pin->getPinID()))
            //                ->getPinName() == "P") {
            //   tmp_pin->setPinName("D");
            //   pins_vec.at(h_pinmatch.at(tmp_pin->getPinID()))->setPinName("S");
            // }
          }

          // add both current pin and matched pin
          h_touchedpin.insert({tmp_pin->getPinID(), 1});
          h_touchedpin.insert(
              {pins_vec.at(h_pinmatch.at(tmp_pin->getPinID()))->getPinID(), 1});
        }
      }

      // for (auto tmp_net : nets_vec) {
      for (auto const &tmp_net : nets_vec) {
        std::cout << "a     Net Info : ID => " << tmp_net->getNet_id()
                  << ", #Pin => " << tmp_net->getNpinNet() << ", PinList => "
                  << tmp_net->getPinList() << std::endl;
      }

      // Handling reminder to make sure size is enough to cover
      if (sizeNMOS % 2 == 0) {
        sizeNMOS++;
      }

      if (sizePMOS % 2 == 0) {
        sizePMOS++;
      }

      // # Output file
      // ### Write PinLayout
      std::ofstream out(outfile);
      out << "a   PNR Testcase Generation::  DesignName = " << designName
          << std::endl;
      out << "a   Output File:" << std::endl;
      out << "a   " << outfile.generic_string() << std::endl;
      // since CFET has P-N stacked structure, whichever one is wider defines
      // the std cell width
      out << "a   Width of Routing Clip    = "
          << (sizeNMOS > sizePMOS ? sizeNMOS : sizePMOS) << std::endl;
      out << "a   Height of Routing Clip   = " << numClip << std::endl;
      out << "a   Tracks per Placement Row = " << numTrackH << std::endl;
      out << "a   Width of Placement Clip  = "
          << (sizeNMOS > sizePMOS ? sizeNMOS : sizePMOS) << std::endl;
      out << "a   Tracks per Placement Clip = " << numPTrackH << std::endl;

      // ################################ Adding Instance Info
      out << "i   ===InstanceInfo===" << std::endl;
      out << "i   InstID Type Width" << std::endl;

      for (auto tmp_inst : inst_vec) {
        if (tmp_inst->getInstType() == "PMOS") {
          out << "i   ins" << tmp_inst->getInstName() << " "
              << tmp_inst->getInstType() << " " << tmp_inst->getWidth()
              << std::endl;
        }
      }

      for (auto tmp_inst : inst_vec) {
        if (tmp_inst->getInstType() == "NMOS") {
          out << "i   ins" << tmp_inst->getInstName() << " "
              << tmp_inst->getInstType() << " " << tmp_inst->getWidth()
              << std::endl;
        }
      }
      // ################################ Adding Pin Info
      out << "i   ===PinInfo===" << std::endl;
      out << "i   PinID NetID InstID PinName PinDirection PinLength"
          << std::endl;

      for (auto tmp_pin : pins_vec) {
        if (tmp_pin->getInstID() != "ext") {
          out << "i   pin" << tmp_pin->getPinID() << " net"
              << tmp_pin->getNet_id() << " "
              << ((tmp_pin->getInstID() == "ext")
                      ? "ext"
                      : ("ins" + tmp_pin->getInstID()))
                     .c_str()
              << " " << tmp_pin->getPinName() << " "
              << tmp_pin->getPinDirection() << " " << tmp_pin->getPinLength()
              << std::endl;
        } else {
          // out << "i   pin" << tmp_pin->getPinID() << " net"
          //     << tmp_pin->getNet_id() << " "
          //     << ((tmp_pin->getInstID() == "ext")
          //             ? "ext"
          //             : ("ins" + tmp_pin->getInstID()))
          //            .c_str()
          //     << " " << tmp_pin->getPinName() << " "
          //     << tmp_pin->getPinDirection() << " " << tmp_pin->getPinLength()
          //     << " " << h_pintype.at(Pin::hash(tmp_pin->getPinName().data()))
          //     << std::endl;
          out << "i   pin" << tmp_pin->getPinID() << " net"
              << tmp_pin->getNet_id() << " "
              << ((tmp_pin->getInstID() == "ext")
                      ? "ext"
                      : ("ins" + tmp_pin->getInstID()))
                     .c_str()
              << " " << tmp_pin->getPinName() << " "
              << tmp_pin->getPinDirection() << " " << tmp_pin->getPinLength()
              << " " << std::endl;
        }
      }

      // ################################ Adding Net Info
      out << "i   ===NetInfo===" << std::endl;
      out << "i   NetID N-PinNet PinList" << std::endl;
      // for (auto tmp_net : nets_vec) {
      for (auto const &tmp_net : nets_vec) {
        out << "i   net" << tmp_net->getNet_id() << " " << tmp_net->getNpinNet()
            << "PinNet " << tmp_net->getPinList() << std::endl;
      }
      out.close();
    }
  }

  return 0;
}