#include "flow.hpp"
#include "SMTCell.hpp"
#include "obj.hpp"

namespace bmp = boost::multiprecision;

void FlowWriter::localize_commodity(int local_Parameter,
                                    int tolerance_Parameter) {
  if (local_Parameter == 1) {
    SMTCell::writeConstraint(";Localization.\n\n");
    SMTCell::writeConstraint(
        ";Conditional Localization for All Commodities\n\n");
    for (int netIndex = 0; netIndex < SMTCell::getNetCnt(); netIndex++) {
      for (int commodityIndex = 0;
           commodityIndex < SMTCell::getNet(netIndex)->getNumSinks();
           commodityIndex++) {

        // inst pin idx
        std::string pidx_s = SMTCell::getNet(netIndex)->getSource_ofNet();
        std::string pidx_t =
            SMTCell::getNet(netIndex)->getSinks_inNet(commodityIndex);
        // external net should NOT be considered
        if (pidx_s == Pin::keySON || pidx_t == Pin::keySON) {
          continue;
        }
        // pin ptr
        std::shared_ptr<Pin> pin_s = SMTCell::getPin(pidx_s);
        std::shared_ptr<Pin> pin_t = SMTCell::getPin(pidx_t);
        // external net should NOT be considered
        if (pin_s->getInstID() == "ext" || pin_t->getInstID() == "ext") {
          continue;
        }
        // inst idx
        int inst_pin_s = SMTCell::getPinInstIdx(pin_s);
        int inst_pin_t = SMTCell::getPinInstIdx(pin_t);
        // type
        std::string type_s = pin_s->getPinType();
        std::string type_t = pin_t->getPinType();
        // finger
        std::vector<int> finger_s = SMTCell::getAvailableNumFinger(
            SMTCell::getInst(inst_pin_s)->getInstWidth(),
            SMTCell::getTrackEachPRow());
        std::vector<int> finger_t = SMTCell::getAvailableNumFinger(
            SMTCell::getInst(inst_pin_t)->getInstWidth(),
            SMTCell::getTrackEachPRow());

        // width
        int w_s = finger_s[0] * 2;
        int w_t = finger_t[0] * 2;
        // int len = SMTCell::getBitLength_numTrackV();

        int tmp_pidx_s = std::stoi(
            std::regex_replace(pidx_s, std::regex("pin\\S+_(\\d+)"), "$1"));
        int tmp_pidx_t = std::stoi(
            std::regex_replace(pidx_t, std::regex("pin\\S+_(\\d+)"), "$1"));

        if (type_s == Pin::GATE) {
          w_s = 2 * tmp_pidx_s;
        }

        if (type_t == Pin::GATE) {
          w_t = 2 * tmp_pidx_t;
        }

        if (pidx_t != Pin::keySON) {
          // AGR FLAG : traverse through all columns
          int metal = 1;
          for (int col_idx = 0;
               col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1;
               col_idx++) {
            SMTCell::writeConstraint(fmt::format(
                "(assert (ite (and (= ff{} false) (= ff{} false)) (ite "
                "(bvsge (bvadd x{} (_ bv{} {})) (bvadd x{} (_ bv{} {})))\n",
                inst_pin_s, inst_pin_t, inst_pin_s,
                tmp_pidx_s * SMTCell::getMetalPitch(1),
                SMTCell::getBitLength_numTrackV(), inst_pin_t,
                tmp_pidx_t * SMTCell::getMetalPitch(1),
                SMTCell::getBitLength_numTrackV()));
            SMTCell::cnt("l", 0);
            SMTCell::cnt("l", 0);
            SMTCell::cnt("l", 0);
            SMTCell::cnt("l", 0);

            int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);

            // AGR FLAG : M1 only
            int tmp_col = col - (tolerance_Parameter + tmp_pidx_s) *
                                          SMTCell::getMetalPitch(1) >=
                                  0
                              ? (col - (tolerance_Parameter + tmp_pidx_s) *
                                           SMTCell::getMetalPitch(1))
                              : (0);

            SMTCell::writeConstraint(fmt::format(
                "             (and (ite (bvslt x{} (_ bv{} {})) (and",
                inst_pin_s, tmp_col, SMTCell::getBitLength_numTrackV()));
            SMTCell::cnt("l", 0);

            // # 1 mergeable
            // SMTCell::writeConstraint(";1\n");
            // fmt::print("col: {}, commodityIndex: {}, netIndex: {}\n", col,
            //            commodityIndex, netIndex);
            FlowWriter::localize_commodity_helper(col, commodityIndex,
                                                  netIndex);

            SMTCell::writeConstraint(") (= true true))\n");
            int tmp_bv =
                ((col + (tolerance_Parameter - tmp_pidx_t) *
                            SMTCell::getMetalPitch(1)) >=
                         (SMTCell::getCellWidth())
                     ? (SMTCell::getCellWidth())
                     : ((col + (tolerance_Parameter - tmp_pidx_t) *
                                   SMTCell::getMetalPitch(1)) >= 0
                            ? (col + (tolerance_Parameter - tmp_pidx_t) *
                                         SMTCell::getMetalPitch(1))
                            : (0)));
            SMTCell::writeConstraint(fmt::format(
                "                  (ite (bvsgt x{} (_ bv{} {})) (and",
                inst_pin_t, tmp_bv, SMTCell::getBitLength_numTrackV()));
            SMTCell::cnt("l", 0);
            // fmt::print(
            //     "1. bv:{}, col:{}, tolerance_Parameter:{}, tmp_pidx_t:{}\n",
            //     tmp_bv, col, tolerance_Parameter, tmp_pidx_t);

            // 2
            // SMTCell::writeConstraint(";2\n");
            FlowWriter::localize_commodity_helper(col, commodityIndex,
                                                  netIndex);

            SMTCell::writeConstraint(") (= true true)))\n");
            tmp_bv = (col - (tolerance_Parameter + tmp_pidx_t) *
                                      SMTCell::getMetalPitch(1) >=
                              0
                          ? (col - (tolerance_Parameter + tmp_pidx_t) *
                                       SMTCell::getMetalPitch(1))
                          : (0));
            SMTCell::writeConstraint(fmt::format(
                "             (and (ite (bvslt x{} (_ bv{} {})) (and",
                inst_pin_t, tmp_bv, SMTCell::getBitLength_numTrackV()));
            SMTCell::cnt("l", 0);
            // fmt::print("2. bv:{}\n", tmp_bv);

            // 3
            // SMTCell::writeConstraint(";3\n");
            FlowWriter::localize_commodity_helper(col, commodityIndex,
                                                  netIndex);

            SMTCell::writeConstraint(") (= true true))\n");
            tmp_bv = (col + (tolerance_Parameter - tmp_pidx_s) *
                                      SMTCell::getMetalPitch(1) >=
                              SMTCell::getCellWidth()
                          ? (SMTCell::getCellWidth())
                          : (col + (tolerance_Parameter - tmp_pidx_s) *
                                             SMTCell::getMetalPitch(1) >=
                                     0
                                 ? (col + (tolerance_Parameter - tmp_pidx_s) *
                                              SMTCell::getMetalPitch(1))
                                 : (0)));

            SMTCell::writeConstraint(fmt::format(
                "                  (ite (bvsgt x{} (_ bv{} {})) (and",
                inst_pin_s, tmp_bv, SMTCell::getBitLength_numTrackV()));
            SMTCell::cnt("l", 0);
            // fmt::print("3. bv:{}\n", tmp_bv);
            // 4
            // SMTCell::writeConstraint(";4\n");
            FlowWriter::localize_commodity_helper(col, commodityIndex,
                                                  netIndex);

            SMTCell::writeConstraint(") (= true true))))\n");
            SMTCell::writeConstraint(fmt::format(
                "	(ite (and (= ff{} false) (= ff{} true)) (ite (bvsge "
                "(bvadd x{} (_ bv{} {})) (bvadd x{} (_ bv{} {})))\n",
                inst_pin_s, inst_pin_t, inst_pin_s,
                tmp_pidx_s * SMTCell::getMetalPitch(1),
                SMTCell::getBitLength_numTrackV(), inst_pin_t,
                (w_t - tmp_pidx_t) * SMTCell::getMetalPitch(1),
                SMTCell::getBitLength_numTrackV()));
            SMTCell::cnt("l", 0);
            SMTCell::cnt("l", 0);
            SMTCell::cnt("l", 0);
            SMTCell::cnt("l", 0);
            tmp_bv = (col - (tolerance_Parameter + tmp_pidx_s) *
                                      SMTCell::getMetalPitch(1) >=
                              0
                          ? (col - (tolerance_Parameter + tmp_pidx_s) *
                                       SMTCell::getMetalPitch(1))
                          : (0));
            SMTCell::writeConstraint(fmt::format(
                "             (and (ite (bvslt x{} (_ bv{} {})) (and",
                inst_pin_s, tmp_bv, SMTCell::getBitLength_numTrackV()));
            SMTCell::cnt("l", 0);

            // 5
            // SMTCell::writeConstraint(";5\n");
            FlowWriter::localize_commodity_helper(col, commodityIndex,
                                                  netIndex);

            SMTCell::writeConstraint(") (= true true))\n");
            tmp_bv =
                (col + (tolerance_Parameter - w_t + tmp_pidx_t) *
                                 SMTCell::getMetalPitch(1) >=
                         SMTCell::getCellWidth()
                     ? (SMTCell::getCellWidth())
                     : (col + (tolerance_Parameter - w_t + tmp_pidx_t) *
                                        SMTCell::getMetalPitch(1) >=
                                0
                            ? (col + (tolerance_Parameter - w_t + tmp_pidx_t) *
                                         SMTCell::getMetalPitch(1))
                            : (0)));
            SMTCell::writeConstraint(fmt::format(
                "                  (ite (bvsgt x{} (_ bv{} {})) (and",
                inst_pin_t, tmp_bv, SMTCell::getBitLength_numTrackV()));
            SMTCell::cnt("l", 0);

            // 6
            // SMTCell::writeConstraint(";6\n");
            FlowWriter::localize_commodity_helper(col, commodityIndex,
                                                  netIndex);

            SMTCell::writeConstraint(") (= true true)))\n");
            tmp_bv = (col - (tolerance_Parameter + w_t - tmp_pidx_t) *
                                      SMTCell::getMetalPitch(1) >=
                              0
                          ? (col - (tolerance_Parameter + w_t - tmp_pidx_t) *
                                       SMTCell::getMetalPitch(1))
                          : 0);
            SMTCell::writeConstraint(fmt::format(
                "             (and (ite (bvslt x{} (_ bv{} {})) (and",
                inst_pin_t, tmp_bv, SMTCell::getBitLength_numTrackV()));
            SMTCell::cnt("l", 0);

            // 7
            // SMTCell::writeConstraint(";7\n");
            FlowWriter::localize_commodity_helper(col, commodityIndex,
                                                  netIndex);

            SMTCell::writeConstraint(") (= true true))\n");
            tmp_bv = (col + (tolerance_Parameter - tmp_pidx_s) *
                                      SMTCell::getMetalPitch(1) >=
                              SMTCell::getCellWidth()
                          ? (SMTCell::getCellWidth())
                          : (col + (tolerance_Parameter - tmp_pidx_s) *
                                             SMTCell::getMetalPitch(1) >=
                                     0
                                 ? (col + (tolerance_Parameter - tmp_pidx_s) *
                                              SMTCell::getMetalPitch(1))
                                 : (0)));

            SMTCell::writeConstraint(fmt::format(
                "                  (ite (bvsgt x{} (_ bv{} {})) (and",
                inst_pin_s, tmp_bv, SMTCell::getBitLength_numTrackV()));
            SMTCell::cnt("l", 0);

            // 8
            // SMTCell::writeConstraint(";8\n");
            FlowWriter::localize_commodity_helper(col, commodityIndex,
                                                  netIndex);

            SMTCell::writeConstraint(") (= true true))))\n");
            SMTCell::writeConstraint(fmt::format(
                "	(ite (and (= ff{} true) (= ff{} false)) (ite (bvsge "
                "(bvadd x{} (_ bv{} {})) (bvadd x{} (_ bv{} {})))\n",
                inst_pin_s, inst_pin_t, inst_pin_s,
                (w_s - tmp_pidx_s) * SMTCell::getMetalPitch(1),
                SMTCell::getBitLength_numTrackV(), inst_pin_t,
                tmp_pidx_t * SMTCell::getMetalPitch(1),
                SMTCell::getBitLength_numTrackV()));
            SMTCell::cnt("l", 0);
            SMTCell::cnt("l", 0);
            SMTCell::cnt("l", 0);
            SMTCell::cnt("l", 0);
            tmp_bv = (col - (tolerance_Parameter + w_s - tmp_pidx_s) *
                                      SMTCell::getMetalPitch(1) >=
                              0
                          ? (col - (tolerance_Parameter + w_s - tmp_pidx_s) *
                                       SMTCell::getMetalPitch(1))
                          : (0));
            SMTCell::writeConstraint(fmt::format(
                "             (and (ite (bvslt x{} (_ bv{} {})) (and",
                inst_pin_s, tmp_bv, SMTCell::getBitLength_numTrackV()));
            SMTCell::cnt("l", 0);

            // 9
            // SMTCell::writeConstraint(";9\n");
            FlowWriter::localize_commodity_helper(col, commodityIndex,
                                                  netIndex);

            SMTCell::writeConstraint(") (= true true))\n");

            tmp_bv = (col + (tolerance_Parameter - tmp_pidx_t) *
                                      SMTCell::getMetalPitch(1) >=
                              (SMTCell::getCellWidth())
                          ? (SMTCell::getCellWidth())
                          : (col + (tolerance_Parameter - tmp_pidx_t) *
                                             SMTCell::getMetalPitch(1) >=
                                     0
                                 ? (col + (tolerance_Parameter - tmp_pidx_t) *
                                              SMTCell::getMetalPitch(1))
                                 : (0)));
            SMTCell::writeConstraint(fmt::format(
                "                  (ite (bvsgt x{} (_ bv{} {})) (and",
                inst_pin_t, tmp_bv, SMTCell::getBitLength_numTrackV()));
            SMTCell::cnt("l", 0);

            // 10
            // SMTCell::writeConstraint(";10\n");
            FlowWriter::localize_commodity_helper(col, commodityIndex,
                                                  netIndex);

            SMTCell::writeConstraint(") (= true true)))\n");
            tmp_bv = (col - (tolerance_Parameter + tmp_pidx_t) *
                                      SMTCell::getMetalPitch(1) >=
                              0
                          ? (col - (tolerance_Parameter + tmp_pidx_t) *
                                       SMTCell::getMetalPitch(1))
                          : (0));
            SMTCell::writeConstraint(fmt::format(
                "             (and (ite (bvslt x{} (_ bv{} {})) (and",
                inst_pin_t, tmp_bv, SMTCell::getBitLength_numTrackV()));
            SMTCell::cnt("l", 0);

            // 11
            // SMTCell::writeConstraint(";11\n");
            FlowWriter::localize_commodity_helper(col, commodityIndex,
                                                  netIndex);

            SMTCell::writeConstraint(") (= true true))\n");
            tmp_bv =
                (col + (tolerance_Parameter - w_s + tmp_pidx_s) *
                                 SMTCell::getMetalPitch(1) >=
                         SMTCell::getCellWidth()
                     ? (SMTCell::getCellWidth())
                     : (col + (tolerance_Parameter - w_s + tmp_pidx_s) *
                                        SMTCell::getMetalPitch(1) >=
                                0
                            ? (col + (tolerance_Parameter - w_s + tmp_pidx_s) *
                                         SMTCell::getMetalPitch(1))
                            : (0)));
            SMTCell::writeConstraint(fmt::format(
                "                  (ite (bvsgt "
                "x{} (_ bv{} {})) (and",
                inst_pin_s, tmp_bv, SMTCell::getBitLength_numTrackV()));
            SMTCell::cnt("l", 0);

            // 12
            // SMTCell::writeConstraint(";12\n");
            FlowWriter::localize_commodity_helper(col, commodityIndex,
                                                  netIndex);

            SMTCell::writeConstraint(") (= true true))))\n");
            SMTCell::writeConstraint(fmt::format(
                "	(ite (bvsge (bvadd x{} (_ bv{} {})) (bvadd x{} (_ bv{} "
                "{})))\n",
                inst_pin_s, (w_s - tmp_pidx_s) * SMTCell::getMetalPitch(1),
                SMTCell::getBitLength_numTrackV(), inst_pin_t,
                (w_t - tmp_pidx_t) * SMTCell::getMetalPitch(1),
                SMTCell::getBitLength_numTrackV()));
            SMTCell::cnt("l", 0);
            SMTCell::cnt("l", 0);
            SMTCell::cnt("l", 0);
            SMTCell::cnt("l", 0);
            tmp_bv = (col - (tolerance_Parameter + w_s - tmp_pidx_s) *
                                      SMTCell::getMetalPitch(1) >=
                              0
                          ? (col - (tolerance_Parameter + w_s - tmp_pidx_s) *
                                       SMTCell::getMetalPitch(1))
                          : (0));
            SMTCell::writeConstraint(fmt::format(
                "             (and (ite (bvslt x{} (_ bv{} {})) (and",
                inst_pin_s, tmp_bv, SMTCell::getBitLength_numTrackV()));
            SMTCell::cnt("l", 0);

            // 13
            // SMTCell::writeConstraint(";13\n");
            FlowWriter::localize_commodity_helper(col, commodityIndex,
                                                  netIndex);

            SMTCell::writeConstraint(") (= true true))\n");
            tmp_bv =
                ((col + (tolerance_Parameter - w_t + tmp_pidx_t) *
                            SMTCell::getMetalPitch(1)) >=
                         (SMTCell::getCellWidth())
                     ? (SMTCell::getCellWidth())
                     : ((col + (tolerance_Parameter - w_t + tmp_pidx_t) *
                                   SMTCell::getMetalPitch(1)) >= 0
                            ? (col + (tolerance_Parameter - w_t + tmp_pidx_t) *
                                         SMTCell::getMetalPitch(1))
                            : (0)));
            SMTCell::writeConstraint(fmt::format(
                "                  (ite (bvsgt x{} (_ bv{} {})) (and",
                inst_pin_t, tmp_bv, SMTCell::getBitLength_numTrackV()));
            SMTCell::cnt("l", 0);
            // fmt::print("13. bv:{}, w_t:{}\n", tmp_bv, w_t);
            // 14
            // SMTCell::writeConstraint(";14\n");
            FlowWriter::localize_commodity_helper(col, commodityIndex,
                                                  netIndex);

            SMTCell::writeConstraint(") (= true true)))\n");
            tmp_bv = (col - (tolerance_Parameter + w_t - tmp_pidx_t) *
                                      SMTCell::getMetalPitch(1) >=
                              0
                          ? (col - (tolerance_Parameter + w_t - tmp_pidx_t) *
                                       SMTCell::getMetalPitch(1))
                          : (0));
            SMTCell::writeConstraint(fmt::format(
                "             (and (ite (bvslt x{} (_ bv{} {})) (and",
                inst_pin_t, tmp_bv, SMTCell::getBitLength_numTrackV()));
            SMTCell::cnt("l", 0);

            // 15
            // SMTCell::writeConstraint(";15\n");
            FlowWriter::localize_commodity_helper(col, commodityIndex,
                                                  netIndex);

            SMTCell::writeConstraint(") (= true true))\n");
            tmp_bv =
                (col + (tolerance_Parameter - w_s + tmp_pidx_s) *
                                 SMTCell::getMetalPitch(1) >=
                         SMTCell::getCellWidth()
                     ? (SMTCell::getCellWidth())
                     : (col + (tolerance_Parameter - w_s + tmp_pidx_s) *
                                        SMTCell::getMetalPitch(1) >=
                                0
                            ? (col + (tolerance_Parameter - w_s + tmp_pidx_s) *
                                         SMTCell::getMetalPitch(1))
                            : (0)));
            SMTCell::writeConstraint(fmt::format(
                "                  (ite (bvsgt "
                "x{} (_ bv{} {})) (and",
                inst_pin_s, tmp_bv, SMTCell::getBitLength_numTrackV()));
            SMTCell::cnt("l", 0);

            // 16
            // SMTCell::writeConstraint(";16\n");
            FlowWriter::localize_commodity_helper(col, commodityIndex,
                                                  netIndex);

            SMTCell::writeConstraint(") (= true true))))))))\n");
            SMTCell::cnt("c", 1);
          }
        }
      }
    }
  }

  SMTCell::writeConstraint(";End of Localization\n\n");
}

void FlowWriter::localize_commodity_helper(int col, int commodityIndex,
                                           int netIndex) {
  std::string tmp_str = "";
  std::map<std::string, int> h_edge;
  for (int metal = 1; metal <= SMTCell::getNumMetalLayer(); metal++) {
    for (int row_idx = 0; row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3;
         row_idx++) {
      // if (metal > 1 && SMTCell::ifVertMetal(metal) &&
      //     SMTCell::ifSDCol_AGR(metal, col)) {
      //   continue;
      // }

      // AGR FLAG : check if this column exists in this metal
      int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
      if (SMTCell::ifColExist(metal, col) == false) {
        continue;
      }

      Triplet vCoord = Triplet(metal, row, col);
      // incoming
      if (SMTCell::ifEdgeIn(vCoord.getVName())) {
        for (int i : SMTCell::getEdgeIn(vCoord.getVName())) {
          std::string metal_variable = fmt::format(
              "{}_{}", SMTCell::getUdEdgeFromVarName(i), vCoord.getVName());
          if (h_edge.find(metal_variable) == h_edge.end()) {
            std::string variable_name = fmt::format(
                "N{}_C{}_E_{}", SMTCell::getNet(netIndex)->getNetID(),
                commodityIndex, metal_variable);
            if (!SMTCell::ifAssigned(variable_name)) {
              SMTCell::setVar(variable_name, 2);
              SMTCell::writeConstraint(
                  fmt::format(" (= {} false)", variable_name));
              h_edge[metal_variable] = 1;
              SMTCell::cnt("l", 1);
            }
          }
        }
      }

      // outgoing
      if (SMTCell::ifEdgeOut(vCoord.getVName())) {
        for (int i : SMTCell::getEdgeOut(vCoord.getVName())) {
          std::string metal_variable = fmt::format(
              "{}_{}", vCoord.getVName(), SMTCell::getUdEdgeToVarName(i));
          if (h_edge.find(metal_variable) == h_edge.end()) {
            std::string variable_name = fmt::format(
                "N{}_C{}_E_{}", SMTCell::getNet(netIndex)->getNetID(),
                commodityIndex, metal_variable);
            if (!SMTCell::ifAssigned(variable_name)) {
              SMTCell::setVar(variable_name, 2);
              SMTCell::writeConstraint(
                  fmt::format(" (= {} false)", variable_name));
              h_edge[metal_variable] = 1;
              SMTCell::cnt("l", 1);
            }
          }
        }
      }

      // BUG FLAG : has no effect on final output
      // sink
      // if (SMTCell::ifVEdgeOut(vCoord.getVName())) {
      //   for (int i : SMTCell::getVEdgeOut(vCoord.getVName())) {
      //     std::string metal_variable =
      //         fmt::format("{}_{}", vCoord.getVName(),
      //                     SMTCell::getVirtualEdge(i)->getPinName());
      //     if (h_edge.find(metal_variable) == h_edge.end()) {
      //       if (SMTCell::getVirtualEdge(i)->getPinName() ==
      //               SMTCell::getNet(netIndex)->getSource_ofNet() ||
      //           SMTCell::getVirtualEdge(i)->getPinName() ==
      //               SMTCell::getNet(netIndex)->getSinks_inNet(commodityIndex))
      //               {
      //         tmp_str = fmt::format("N{}_C{}_E_{}",
      //                               SMTCell::getNet(netIndex)->getNetID(),
      //                               commodityIndex, metal_variable);
      //         if (!SMTCell::ifAssigned(tmp_str)) {
      //           SMTCell::setVar(tmp_str, 2);
      //           SMTCell::writeConstraint(fmt::format(" (= {} false)",
      //           tmp_str)); h_edge[metal_variable] = 1; SMTCell::cnt("l", 1);
      //         }
      //       }
      //     }
      //   }
      // }
    }
  }
}

void FlowWriter::init_commodity_flow_var() {
  for (int netIndex = 0; netIndex < SMTCell::getNetCnt(); netIndex++) {
    for (int commodityIndex = 0;
         commodityIndex < SMTCell::getNet(netIndex)->getNumSinks();
         commodityIndex++) {
      for (int vEdgeIndex = 0; vEdgeIndex < SMTCell::getVirtualEdgeCnt();
           vEdgeIndex++) {
        std::string tmp_vname = "";
        // ignoring $virtualEdges[$vEdgeIndex][2] =~ /^pin/ since this is always
        // a pin name
        if (SMTCell::getVirtualEdge(vEdgeIndex)->getPinName() ==
            SMTCell::getNet(netIndex)->getSource_ofNet()) {
          // ### GATE Pin
          if (SMTCell::getPin(SMTCell::getNet(netIndex)->getSource_ofNet())
                  ->getPinType() == Pin::GATE) {
            int col = SMTCell::getVirtualEdge(vEdgeIndex)->getVCoord()->col_;
            // Gate on odd col
            int metal = 1;
            if (SMTCell::ifSDCol_AGR(metal, col)) {
              std::string variable_name = fmt::format(
                  "N{}_C{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                  commodityIndex,
                  SMTCell::getVirtualEdge(vEdgeIndex)->getVName(),
                  SMTCell::getVirtualEdge(vEdgeIndex)->getPinName());

              SMTCell::assignFalseVar(variable_name);
            }
          } else {
            int col = SMTCell::getVirtualEdge(vEdgeIndex)->getVCoord()->col_;
            // Gate on even col
            int metal = 1;
            if (SMTCell::ifGCol_AGR(metal, col)) {
              std::string variable_name = fmt::format(
                  "N{}_C{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                  commodityIndex,
                  SMTCell::getVirtualEdge(vEdgeIndex)->getVName(),
                  SMTCell::getVirtualEdge(vEdgeIndex)->getPinName());

              SMTCell::assignFalseVar(variable_name);
            }
          }
        } else if (SMTCell::getVirtualEdge(vEdgeIndex)->getPinName() ==
                   SMTCell::getNet(netIndex)->getSinks_inNet(commodityIndex)) {
          if (SMTCell::getVirtualEdge(vEdgeIndex)->getPinName() !=
              Pin::keySON) {
            if (SMTCell ::getPin(
                    SMTCell::getNet(netIndex)->getSinks_inNet(commodityIndex))
                    ->getPinType() == Pin::GATE) {
              int col = SMTCell::getVirtualEdge(vEdgeIndex)->getVCoord()->col_;
              int metal = 1;
              if (SMTCell::ifSDCol_AGR(metal, col)) {
                std::string variable_name = fmt::format(
                    "N{}_C{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                    commodityIndex,
                    SMTCell::getVirtualEdge(vEdgeIndex)->getVName(),
                    SMTCell::getVirtualEdge(vEdgeIndex)->getPinName());

                SMTCell::assignFalseVar(variable_name);
              }
            } else {
              int col = SMTCell::getVirtualEdge(vEdgeIndex)->getVCoord()->col_;
              int metal = 1;
              if (SMTCell::ifGCol_AGR(metal, col)) {
                std::string variable_name = fmt::format(
                    "N{}_C{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                    commodityIndex,
                    SMTCell::getVirtualEdge(vEdgeIndex)->getVName(),
                    SMTCell::getVirtualEdge(vEdgeIndex)->getPinName());

                SMTCell::assignFalseVar(variable_name);
              }
            }
          }
        }
      }
    }
  }
}

void FlowWriter::write_flow_capacity_control() {
  for (int netIndex = 0; netIndex < SMTCell::getNetCnt(); netIndex++) {
    for (int commodityIndex = 0;
         commodityIndex < SMTCell::getNet(netIndex)->getNumSinks();
         commodityIndex++) {
      std::string tmp_vname = "";
      // int startIdx = 0;
      // int endIdx = 0;
      // int instIdx = 0;
      // int upRow_idx = -1;
      // int lowRow_idx = -1;
      // int beginRow_idx = -1;
      // int endRow_idx = -1;
      // int beginCol_idx = -1;
      // int endCol_idx = -1;

      // ## Source MaxFlow Indicator
      std::string pidx_s = SMTCell::getNet(netIndex)->getSource_ofNet();
      FlowWriter::flow_indicator_helper(pidx_s, netIndex, commodityIndex);

      // ## Sink MaxFlow Indicator
      // mergeable with Source MaxFlow Indicator above
      std::string pidx_t =
          SMTCell::getNet(netIndex)->getSinks_inNet(commodityIndex);
      FlowWriter::flow_indicator_helper(pidx_t, netIndex, commodityIndex);
    }
  }
}

void FlowWriter::flow_indicator_helper(std::string pidx, int netIndex,
                                       int commodityIndex) {
  std::string tmp_vname = "";
  int startIdx = 0;
  int endIdx = 0;
  // int instIdx = 0;
  int upRow_idx = -1;
  int lowRow_idx = -1;
  int beginRow_idx = -1;
  int endRow_idx = -1;
  int beginCol_idx = -1;
  int endCol_idx = -1;

  // external net should NOT be considered
  if (pidx == Pin::keySON) {
    return;
  }

  std::shared_ptr<Pin> pin = SMTCell::getPin(pidx);
  int pin_inst_idx = SMTCell::getPinInstIdx(pin);
  std::vector<int> tmp_finger = SMTCell::getAvailableNumFinger(
      SMTCell::getInst(pin_inst_idx)->getInstWidth(),
      SMTCell::getTrackEachPRow());

  // PMOS
  if (pin_inst_idx <= SMTCell::getLastIdxPMOS()) {
    upRow_idx = SMTCell::getRoutingTrack(
        SMTCell::getNumPTrackH() - 1 -
        SMTCell::getConn(SMTCell::getInst(pin_inst_idx)->getInstWidth() /
                         tmp_finger[0]));
    lowRow_idx = SMTCell::getRoutingTrack(SMTCell::getNumPTrackH() - 1);
    beginRow_idx = 0;

    // AGR FLAG : adapt to different track height
    if (SMTCell::getNumTrack() == 4 || SMTCell::getNumTrack() == 5) {
      endRow_idx = floor(SMTCell::get_h_metal_numTrackH(1) / 2 + 0.5) - 2;
    } else if (SMTCell::getNumTrack() == 6) {
      endRow_idx = SMTCell::get_h_metal_numTrackH(1) / 2 - 2;
    } else {
      fmt::print(stderr, "[ERROR] unsupported track height\n");
      exit(1);
    }
    beginCol_idx = 0;
    endCol_idx = SMTCell::get_h_metal_numTrackV(1) - 1;
  }
  // NMOS
  else {
    upRow_idx = SMTCell::getRoutingTrack(0);
    lowRow_idx = SMTCell::getRoutingTrack(SMTCell::getConn(
        SMTCell::getInst(pin_inst_idx)->getInstWidth() / tmp_finger[0]));
    // AGR FLAG : adapt to different track height
    if (SMTCell::getNumTrack() == 4 || SMTCell::getNumTrack() == 5) {
      beginRow_idx = floor(SMTCell::get_h_metal_numTrackH(1) / 2 + 0.5) - 1;
    } else if (SMTCell::getNumTrack() == 6) {
      beginRow_idx = SMTCell::get_h_metal_numTrackH(1) / 2 - 1;
    } else {
      fmt::print(stderr, "[ERROR] unsupported track height\n");
      exit(1);
    }
    endRow_idx = SMTCell::get_h_metal_numTrackH(1) - 3;
    beginCol_idx = 0;
    endCol_idx = SMTCell::get_h_metal_numTrackV(1) - 1;
  }

  for (int col_idx = beginCol_idx; col_idx <= endCol_idx; col_idx++) {
    int metal = 1;
    int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);
    // ### GATE Pin
    if (pin->getPinType() == Pin::GATE) {
      int tmp_pidx = std::stoi(
          std::regex_replace(pidx, std::regex("pin\\S+_(\\d+)"), "$1"));
      startIdx = 0;
      endIdx = 0;
      // SMTCell::writeConstraint(";Source Flow Indicator: GATE Pin\n");
      if (SMTCell::ifGCol_AGR(metal, col)) {
        for (std::size_t j = 0; j < tmp_finger.size(); j++) {
          if (j > 0) {
            startIdx = tmp_finger[j - 1] * 2 + 1;
          }

          endIdx = tmp_finger[j] * 2 + 1;
          if (tmp_pidx >= startIdx && tmp_pidx <= endIdx - 1) {
            if (tmp_pidx % 2 == 1) {
              if (j == 0) {
                if (SMTCell::get_h_metal_col_by_idx(1, tmp_pidx) > col) {
                  for (int row_idx = beginRow_idx; row_idx <= endRow_idx;
                       row_idx++) {
                    // retrieve row
                    int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
                    if (SMTCell::ifRoutingTrack(row) && lowRow_idx <= row &&
                        upRow_idx >= row) {
                      Triplet vCoord = Triplet(1, row, col);
                      tmp_vname =
                          fmt::format("N{}_C{}_E_{}_{}",
                                      SMTCell::getNet(netIndex)->getNetID(),
                                      commodityIndex, vCoord.getVName(), pidx);
                      SMTCell::assignTrueVar(tmp_vname, 0, true);
                    }
                  }
                } else {
                  // int len = SMTCell::getBitLength_numTrackV();
                  std::string tmp_str = "";
                  std::vector<std::string> tmp_var;
                  int cnt_var = 0;
                  int cnt_true = 0;

                  for (int row_idx = beginRow_idx; row_idx <= endRow_idx;
                       row_idx++) {
                    // retrieve row
                    int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
                    Triplet vCoord = Triplet(1, row, col);
                    std::string variable_name =
                        fmt::format("N{}_C{}_E_{}_{}",
                                    SMTCell::getNet(netIndex)->getNetID(),
                                    commodityIndex, vCoord.getVName(), pidx);
                    if (SMTCell::ifRoutingTrack(row) && lowRow_idx <= row &&
                        upRow_idx >= row) {
                      if (!SMTCell::ifAssigned(variable_name)) {
                        tmp_var.push_back(variable_name);
                        SMTCell::setVar(variable_name, 2);
                        cnt_var++;
                      } else if (SMTCell::ifAssignedTrue(tmp_vname)) {
                        SMTCell::setVar_wo_cnt(variable_name, 2);
                        cnt_true++;
                      }
                    }
                  }

                  if (cnt_true > 1) {
                    fmt::print("[ERROR] at-leat 2 variables are true in "
                               "the exactly 1 clause!!\n");
                    exit(1);
                  } else if (cnt_var > 0) {
                    SMTCell::writeConstraint(fmt::format(
                        "(assert (ite (= x{} (_ bv{} {}))", pin_inst_idx,
                        (col - SMTCell::get_h_metal_col_by_idx(1, tmp_pidx)),
                        SMTCell::getBitLength_numTrackV()));
                    SMTCell::cnt("l", 0);
                    if (cnt_true == 1) {
                      SMTCell::writeConstraint(" (and");
                      // assign false
                      for (std::size_t m = 0; m < tmp_var.size(); m++) {
                        SMTCell::writeConstraint(
                            fmt::format(" (= {} false)", tmp_var[m]));
                        SMTCell::cnt("l", 1);
                      }
                      SMTCell::writeConstraint(") (and");
                    } else {
                      SMTCell::writeConstraint(" (and ((_ at-least 1)");
                      // AL1 literal
                      for (std::size_t m = 0; m < tmp_var.size(); m++) {
                        SMTCell::writeConstraint(
                            fmt::format(" {}", tmp_var[m]));
                        SMTCell::cnt("l", 1);
                      }
                      SMTCell::writeConstraint(") ((_ at-most 1)");
                      // AM1 literal
                      for (std::size_t m = 0; m < tmp_var.size(); m++) {
                        SMTCell::writeConstraint(
                            fmt::format(" {}", tmp_var[m]));
                        SMTCell::cnt("l", 1);
                      }
                      SMTCell::writeConstraint(")) (and");
                    }
                    // assign false
                    for (std::size_t m = 0; m < tmp_var.size(); m++) {
                      SMTCell::writeConstraint(
                          fmt::format(" (= {} false)", tmp_var[m]));
                      SMTCell::cnt("l", 1);
                    }

                    SMTCell::writeConstraint(")))\n");
                    SMTCell::cnt("c", 1);
                  }
                }
              }
              break;
            }
          }
        }
      }
    } else if (pin->getPinType() == Pin::SOURCE) { // ### Source Pin
      int tmp_pidx = std::stoi(
          std::regex_replace(pidx, std::regex("pin\\S+_(\\d+)"), "$1"));
      startIdx = 0;
      endIdx = 0;
      int isValid = 0;
      std::string tmp_str_1;
      // std::string tmp_str_1 = ";Source Flow Indicator: Source Pin\n";

      if (SMTCell::ifSDCol_AGR(1, col)) {
        for (std::size_t j = 0; j < tmp_finger.size(); j++) {
          if (j > 0) {
            startIdx = tmp_finger[j - 1] * 2 + 1;
          }

          endIdx = tmp_finger[j] * 2 + 1;
          if (tmp_pidx >= startIdx && tmp_pidx <= endIdx - 1) {
            if (tmp_pidx % 4 == 0) {
              if (j == 0) {
                if (SMTCell::get_h_metal_col_by_idx(1, tmp_pidx) <= col) {
                  isValid = 1;
                  tmp_str_1 += fmt::format(
                      "(assert (ite (or (and (= ff{} false)", pin_inst_idx);
                  SMTCell::cnt("l", 0);
                  tmp_str_1 += fmt::format(
                      " (= x{} (_ bv{} {})))", pin_inst_idx,
                      (col - SMTCell::get_h_metal_col_by_idx(1, tmp_pidx)),
                      SMTCell::getBitLength_numTrackV());
                  SMTCell::cnt("l", 0);
                }
              }
              break;
            }
          }
        }
        startIdx = 0;
        endIdx = 0;

        for (std::size_t j = 0; j < tmp_finger.size(); j++) {
          if (j > 0) {
            startIdx = tmp_finger[j - 1] * 2 + 1;
          }

          endIdx = tmp_finger[j] * 2 + 1;
          if (tmp_pidx >= startIdx && tmp_pidx <= endIdx - 1) {
            if (tmp_pidx % 4 == 0) {
              if (j == 0) {
                int tmp_col = ((
                    SMTCell::ifGCol_AGR(metal, startIdx + endIdx - 1 - tmp_pidx)
                        ? SMTCell::get_h_metal_col_by_idx(
                              1, (startIdx + endIdx - 1 - tmp_pidx))
                        : SMTCell::get_h_metal_col_by_idx(
                              1, (startIdx + endIdx - 1 - tmp_pidx + 1))));
                if (tmp_col <= col) {
                  if (isValid == 0) {
                    tmp_str_1 += fmt::format(
                        "(assert (ite (or (and (= ff{} true)", pin_inst_idx);
                    SMTCell::cnt("l", 0);
                    isValid = 1;
                  } else {
                    tmp_str_1 +=
                        fmt::format(" (and (= ff{} true)", pin_inst_idx);
                    SMTCell::cnt("l", 0);
                  }
                  tmp_str_1 += fmt::format(" (= x{} (_ bv{} {})))",
                                           pin_inst_idx, (col - tmp_col),
                                           SMTCell::getBitLength_numTrackV());
                  SMTCell::cnt("l", 0);
                }
              }
              break;
            }
          }
        }

        if (isValid == 1) {
          std::string tmp_str = "";
          std::vector<std::string> tmp_var;
          int cnt_var = 0;
          int cnt_true = 0;
          for (int row_idx = beginRow_idx; row_idx <= endRow_idx; row_idx++) {
            // retrieve row
            int row = SMTCell::get_h_metal_row_by_idx(1, row_idx);
            Triplet vCoord = Triplet(1, row, col);
            std::string variable_name = fmt::format(
                "N{}_C{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                commodityIndex, vCoord.getVName(), pidx);
            if (SMTCell::ifRoutingTrack(row) && lowRow_idx <= row &&
                upRow_idx >= row) {
              if (!SMTCell::ifAssigned(variable_name)) {
                tmp_var.push_back(variable_name);
                SMTCell::setVar(variable_name, 2);
                cnt_var++;
              } else if (SMTCell::ifAssignedTrue(tmp_vname)) {
                SMTCell::setVar_wo_cnt(variable_name, 2);
                cnt_true++;
              }
            }
          }

          if (cnt_true > 1) {
            fmt::print("[ERROR] at-leat 2 variables are true in "
                       "the exactly 1 clause!!\n");
            exit(1);
          } else if (cnt_var > 0) {
            SMTCell::writeConstraint(tmp_str_1);

            if (cnt_true == 1) {
              SMTCell::writeConstraint(") (and");
              // assign false
              for (std::size_t m = 0; m < tmp_var.size(); m++) {
                SMTCell::writeConstraint(
                    fmt::format(" (= {} false)", tmp_var[m]));
                SMTCell::cnt("l", 1);
              }
              SMTCell::writeConstraint(") (and");
            } else {
              SMTCell::writeConstraint(") (and ((_ at-least 1)");
              // AL1 literal
              for (std::size_t m = 0; m < tmp_var.size(); m++) {
                SMTCell::writeConstraint(fmt::format(" {}", tmp_var[m]));
                SMTCell::cnt("l", 1);
              }
              SMTCell::writeConstraint(") ((_ at-most 1)");
              // AM1 literal
              for (std::size_t m = 0; m < tmp_var.size(); m++) {
                SMTCell::writeConstraint(fmt::format(" {}", tmp_var[m]));
                SMTCell::cnt("l", 1);
              }
              SMTCell::writeConstraint(")) (and");
            }
            // assign false
            for (std::size_t m = 0; m < tmp_var.size(); m++) {
              SMTCell::writeConstraint(
                  fmt::format(" (= {} false)", tmp_var[m]));
              SMTCell::cnt("l", 1);
            }

            SMTCell::writeConstraint(")))\n");
            SMTCell::cnt("c", 1);
          }
        } else {
          for (int row_idx = beginRow_idx; row_idx <= endRow_idx; row_idx++) {
            // retrieve row
            int row = SMTCell::get_h_metal_row_by_idx(1, row_idx);
            if (SMTCell::ifRoutingTrack(row) && lowRow_idx <= row &&
                upRow_idx >= row) {
              Triplet vCoord = Triplet(1, row, col);
              std::string variable_name = fmt::format(
                  "N{}_C{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                  commodityIndex, vCoord.getVName(), pidx);
              SMTCell::assignTrueVar(variable_name, 0, true);
            }
          }
        }
      }
    } else if (pin->getPinType() == Pin::DRAIN) { // ### Drain Pin
      // mergeable with source pin above
      int tmp_pidx_s = std::stoi(
          std::regex_replace(pidx, std::regex("pin\\S+_(\\d+)"), "$1"));
      startIdx = 0;
      endIdx = 0;
      int isValid = 0;
      std::string tmp_str_1;
      // std::string tmp_str_1 = ";Source Flow Indicator: Drain Pin\n";

      if (SMTCell::ifSDCol_AGR(1, col)) {
        for (std::size_t j = 0; j < tmp_finger.size(); j++) {
          if (j > 0) {
            startIdx = tmp_finger[j - 1] * 2 + 1;
          }

          endIdx = tmp_finger[j] * 2 + 1;
          if (tmp_pidx_s >= startIdx && tmp_pidx_s <= endIdx - 1) {
            if (tmp_pidx_s % 4 == 2) {
              if (j == 0) {
                if (SMTCell::get_h_metal_col_by_idx(1, tmp_pidx_s) <= col) {
                  isValid = 1;
                  tmp_str_1 += fmt::format(
                      "(assert (ite (or (and (= ff{} false)", pin_inst_idx);
                  SMTCell::cnt("l", 0);
                  tmp_str_1 += fmt::format(
                      " (= x{} (_ bv{} {})))", pin_inst_idx,
                      (col - SMTCell::get_h_metal_col_by_idx(1, tmp_pidx_s)),
                      SMTCell::getBitLength_numTrackV());
                  SMTCell::cnt("l", 0);
                }
              }
              break;
            }
          }
        }
        startIdx = 0;
        endIdx = 0;

        for (std::size_t j = 0; j < tmp_finger.size(); j++) {
          if (j > 0) {
            startIdx = tmp_finger[j - 1] * 2 + 1;
          }

          endIdx = tmp_finger[j] * 2 + 1;
          if (tmp_pidx_s >= startIdx && tmp_pidx_s <= endIdx - 1) {
            if (tmp_pidx_s % 4 == 2) {
              if (j == 0) {
                int tmp_col =
                    ((SMTCell::ifGCol_AGR(metal,
                                          startIdx + endIdx - 1 - tmp_pidx_s)
                          ? SMTCell::get_h_metal_col_by_idx(
                                1, (startIdx + endIdx - 1 - tmp_pidx_s))
                          : SMTCell::get_h_metal_col_by_idx(
                                1, (startIdx + endIdx - 1 - tmp_pidx_s + 1))));
                if (tmp_col <= col) {
                  if (isValid == 0) {
                    tmp_str_1 += fmt::format(
                        "(assert (ite (or (and (= ff{} true)", pin_inst_idx);
                    SMTCell::cnt("l", 0);
                    isValid = 1;
                  } else {
                    tmp_str_1 +=
                        fmt::format(" (and (= ff{} true)", pin_inst_idx);
                    SMTCell::cnt("l", 0);
                  }
                  tmp_str_1 += fmt::format(" (= x{} (_ bv{} {})))",
                                           pin_inst_idx, (col - tmp_col),
                                           SMTCell::getBitLength_numTrackV());
                  SMTCell::cnt("l", 0);
                }
              }
              break;
            }
          }
        }

        if (isValid == 1) {
          std::string tmp_str = "";
          std::vector<std::string> tmp_var;
          int cnt_var = 0;
          int cnt_true = 0;
          for (int row_idx = beginRow_idx; row_idx <= endRow_idx; row_idx++) {
            // retrieve row
            int row = SMTCell::get_h_metal_row_by_idx(1, row_idx);
            Triplet vCoord = Triplet(1, row, col);
            std::string variable_name = fmt::format(
                "N{}_C{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                commodityIndex, vCoord.getVName(), pidx);
            if (SMTCell::ifRoutingTrack(row) && lowRow_idx <= row &&
                upRow_idx >= row) {
              if (!SMTCell::ifAssigned(variable_name)) {
                tmp_var.push_back(variable_name);
                SMTCell::setVar(variable_name, 2);
                cnt_var++;
              } else if (SMTCell::ifAssignedTrue(tmp_vname)) {
                SMTCell::setVar_wo_cnt(variable_name, 2);
                cnt_true++;
              }
            }
          }

          if (cnt_true > 1) {
            fmt::print("[ERROR] at-leat 2 variables are true in "
                       "the exactly 1 clause!!\n");
            exit(1);
          } else if (cnt_var > 0) {
            SMTCell::writeConstraint(tmp_str_1);

            if (cnt_true == 1) {
              SMTCell::writeConstraint(") (and");
              // assign false
              for (std::size_t m = 0; m < tmp_var.size(); m++) {
                SMTCell::writeConstraint(
                    fmt::format(" (= {} false)", tmp_var[m]));
                SMTCell::cnt("l", 1);
              }
              SMTCell::writeConstraint(") (and");
            } else {
              SMTCell::writeConstraint(") (and ((_ at-least 1)");
              // AL1 literal
              for (std::size_t m = 0; m < tmp_var.size(); m++) {
                SMTCell::writeConstraint(fmt::format(" {}", tmp_var[m]));
                SMTCell::cnt("l", 1);
              }
              SMTCell::writeConstraint(") ((_ at-most 1)");
              // AM1 literal
              for (std::size_t m = 0; m < tmp_var.size(); m++) {
                SMTCell::writeConstraint(fmt::format(" {}", tmp_var[m]));
                SMTCell::cnt("l", 1);
              }
              SMTCell::writeConstraint(")) (and");
            }
            // assign false
            for (std::size_t m = 0; m < tmp_var.size(); m++) {
              SMTCell::writeConstraint(
                  fmt::format(" (= {} false)", tmp_var[m]));
              SMTCell::cnt("l", 1);
            }

            SMTCell::writeConstraint(")))\n");
            SMTCell::cnt("c", 1);
          }
        } else {
          for (int row_idx = beginRow_idx; row_idx <= endRow_idx; row_idx++) {
            // retrieve row
            int row = SMTCell::get_h_metal_row_by_idx(1, row_idx);
            if (SMTCell::ifRoutingTrack(row) && lowRow_idx <= row &&
                upRow_idx >= row) {
              Triplet vCoord = Triplet(1, row, col);
              std::string variable_name = fmt::format(
                  "N{}_C{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                  commodityIndex, vCoord.getVName(), pidx);
              SMTCell::assignTrueVar(variable_name, 0, true);
            }
          }
        }
      }
    }
  }
}

void FlowWriter::write_flow_conservation(FILE *fp, int EXT_Parameter) {
  SMTCell::writeConstraint(";1. Commodity flow conservation for each vertex "
                           "and every connected edge to the vertex\n");
  for (int netIndex = 0; netIndex < SMTCell::getNetCnt(); netIndex++) {
    for (int commodityIndex = 0;
         commodityIndex < SMTCell::getNet(netIndex)->getNumSinks();
         commodityIndex++) {
      for (int metal = 1; metal <= SMTCell::getNumMetalLayer(); metal++) {
        for (int row_idx = 0;
             row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3; row_idx++) {
          for (int col_idx = 0;
               col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1;
               col_idx++) {
            // retrieve row/col
            int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
            int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);

            // if (metal > 1 && SMTCell::ifVertMetal(metal) &&
            //     SMTCell::ifSDCol_AGR(1, col)) {
            //   continue;
            // }
            Triplet vCoord = Triplet(metal, row, col);
            std::string tmp_str = "";         // building literal
            std::vector<std::string> tmp_var; // store all clauses
            int cnt_var = 0;
            int cnt_true = 0;

            if (SMTCell::ifEdgeIn(vCoord.getVName())) {
              // incoming
              for (int i : SMTCell::getEdgeIn(vCoord.getVName())) {
                tmp_str = fmt::format(
                    "N{}_C{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                    commodityIndex, SMTCell::getUdEdgeFromVarName(i),
                    vCoord.getVName());
                if (!SMTCell::ifAssigned(tmp_str)) {
                  tmp_var.push_back(tmp_str);
                  SMTCell::setVar(tmp_str, 2);
                  cnt_var++;
                } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                  SMTCell::setVar_wo_cnt(tmp_str, 0);
                  cnt_true++;
                }
              }
            }

            if (SMTCell::ifEdgeOut(vCoord.getVName())) {
              // outcoming
              for (int i : SMTCell::getEdgeOut(vCoord.getVName())) {
                tmp_str = fmt::format("N{}_C{}_E_{}_{}",
                                      SMTCell::getNet(netIndex)->getNetID(),
                                      commodityIndex, vCoord.getVName(),
                                      SMTCell::getUdEdgeToVarName(i));
                if (!SMTCell::ifAssigned(tmp_str)) {
                  tmp_var.push_back(tmp_str);
                  SMTCell::setVar(tmp_str, 2);
                  cnt_var++;
                } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                  SMTCell::setVar_wo_cnt(tmp_str, 0);
                  cnt_true++;
                }
              }
            }

            if (SMTCell::ifVEdgeOut(vCoord.getVName())) {
              // sink
              for (int i : SMTCell::getVEdgeOut(vCoord.getVName())) {
                if (SMTCell::getVirtualEdge(i)->getPinName() ==
                        SMTCell::getNet(netIndex)->getSource_ofNet() ||
                    SMTCell::getVirtualEdge(i)->getPinName() ==
                        SMTCell::getNet(netIndex)->getSinks_inNet(
                            commodityIndex)) {
                  tmp_str = fmt::format(
                      "N{}_C{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                      commodityIndex, vCoord.getVName(),
                      SMTCell::getVirtualEdge(i)->getPinName());
                  if (!SMTCell::ifAssigned(tmp_str)) {
                    tmp_var.push_back(tmp_str);
                    SMTCell::setVar(tmp_str, 2);
                    cnt_var++;
                  } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                    SMTCell::setVar_wo_cnt(tmp_str, 0);
                    cnt_true++;
                  }
                }
              }
            }

            if (cnt_true > 0) {
              if (cnt_true > 2) {
                fmt::print("[ERROR] number of true commodity variable exceed "
                           "2!! in Vertex[$vName]\n");
                exit(1);
              } else if (cnt_true > 1) {
                // # if # of true variables is two, then remained variables
                // should be false
                SMTCell::assignTrueVar(tmp_var[0], 0, true);
              } else {
                // # if # of true variables is one, then remained variable
                // should be exactly one
                if (cnt_var > 1) {
                  SMTCell::assignTrueVar(tmp_var[0], 1, true);
                  SMTCell::setVar_wo_cnt(tmp_var[0], 0);
                } else if (cnt_var > 1) {
                  // # at-most 1
                  SMTCell::writeConstraint("(assert ((_ at-most 1)");
                  for (auto s : tmp_var) {
                    SMTCell::writeConstraint(fmt::format(" {}", s));
                    ;
                    SMTCell::cnt("l", 1);
                  }
                  SMTCell::writeConstraint("))\n");
                  SMTCell::cnt("c", 1);
                  // # at-least 1
                  SMTCell::writeConstraint("(assert ((_ at-least 1)");
                  for (auto s : tmp_var) {
                    SMTCell::writeConstraint(fmt::format(" {}", s));
                    ;
                    SMTCell::cnt("l", 1);
                  }
                  SMTCell::writeConstraint("))\n");
                  SMTCell::cnt("c", 1);
                }
              }
            } else {
              // # true-assigned variable is not included in terms
              // # if # of rest variables is one, then that variable should be
              // false
              if (cnt_var == 1) {
                SMTCell::assignTrueVar(tmp_var[0], 0, true);
              } else if (cnt_var == 2) {
                SMTCell::writeConstraint(
                    fmt::format("(assert (= (or (not {}) {}) true))\n",
                                tmp_var[0], tmp_var[1]));
                SMTCell::cnt("l", 1);
                SMTCell::cnt("l", 1);
                SMTCell::cnt("c", 1);
                SMTCell::writeConstraint(
                    fmt::format("(assert (= (or {} (not {})) true))\n",
                                tmp_var[0], tmp_var[1]));
                SMTCell::cnt("l", 1);
                SMTCell::cnt("l", 1);
                SMTCell::cnt("c", 1);
              } else if (cnt_var > 2) {
                // #at-most 2
                SMTCell::writeConstraint("(assert ((_ at-most 2)");
                for (auto s : tmp_var) {
                  SMTCell::writeConstraint(fmt::format(" {}", s));
                  ;
                  SMTCell::cnt("l", 1);
                }
                SMTCell::writeConstraint("))\n");
                SMTCell::cnt("c", 1);
                // # not exactly-1
                for (auto s_i : tmp_var) {
                  SMTCell::writeConstraint("(assert (= (or");
                  for (auto s_j : tmp_var) {
                    if (s_i == s_j) {
                      SMTCell::writeConstraint(fmt::format(" (not {})", s_j));
                    } else {
                      SMTCell::writeConstraint(fmt::format(" {}", s_j));
                    }
                    SMTCell::cnt("l", 1);
                  }
                  SMTCell::writeConstraint(") true))\n");
                  SMTCell::cnt("c", 1);
                }
              }
            }
          }
        }
      }
    }
  }
  // for (auto n: nets) {
  //   n->dump();
  // }

  SMTCell::writeConstraint(";M4 Layer Constraints\n");
  // ### M4 Layer's flow should be true for a pair of edge flows because only
  // via3,4 is only feasible at the even or odd columns
  for (int netIndex = 0; netIndex < SMTCell::getNetCnt(); netIndex++) {
    for (int commodityIndex = 0;
         commodityIndex < SMTCell::getNet(netIndex)->getNumSinks();
         commodityIndex++) {
      for (int metal = 4; metal <= SMTCell::getNumMetalLayer(); metal++) {
        for (int row_idx = 0;
             row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3; row_idx++) {
          for (int col_idx = 0;
               col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1;
               col_idx++) {
            // retrieve row/col
            int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
            int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);
            if (SMTCell::ifVertMetal(metal)) {
              continue;
            }
            if (SMTCell::ifGCol_AGR(1, col)) {
              continue;
            }
            Triplet vCoord = Triplet(metal, row, col);
            std::string tmp_str = "";         // building literal
            std::vector<std::string> tmp_var; // store all clauses
            int cnt_var = 0;
            int cnt_true = 0;
            // incoming
            for (int i : SMTCell::getEdgeIn(vCoord.getVName())) {
              int fromMetal = SMTCell::getUdEdge(i)->getUdEdgeFromVar()->metal_;
              if (fromMetal != metal) {
                continue;
              }
              tmp_str = fmt::format(
                  "N{}_C{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                  commodityIndex, SMTCell::getUdEdgeFromVarName(i),
                  vCoord.getVName());
              // make this into a class method
              switch (SMTCell::assignVar(tmp_str)) {
              case 1:
                tmp_var.push_back(tmp_str);
                cnt_var++;
                break;
              case 0:
                cnt_true++;
                break;
              }
            }
            // outcoming
            for (int i : SMTCell::getEdgeOut(vCoord.getVName())) {
              int fromMetal = SMTCell::getUdEdge(i)->getUdEdgeFromVar()->metal_;
              if (fromMetal != metal) {
                continue;
              }
              tmp_str = fmt::format("N{}_C{}_E_{}_{}",
                                    SMTCell::getNet(netIndex)->getNetID(),
                                    commodityIndex, vCoord.getVName(),
                                    SMTCell::getUdEdgeToVarName(i));
              // make this into a class method
              switch (SMTCell::assignVar(tmp_str)) {
              case 1:
                tmp_var.push_back(tmp_str);
                cnt_var++;
                break;
              case 0:
                cnt_true++;
                break;
              }
            }

            if (cnt_true > 0) {
              if (cnt_true > 2) {
                fmt::print("[ERROR] number of true commodity variable exceed "
                           "2!! in Vertex[$vName]\n");
                exit(1);
              } else {
                // # if # of true variables is one, then remained variable
                // should be true
                if (cnt_var > 1) {
                  fmt::print("[ERROR] number of commodity pair exceeds 2!! "
                             "in Vertex[$vName]\n");
                  exit(1);
                } else if (cnt_var == 1) {
                  // 04/07 bug found
                  SMTCell::assignTrueVar(tmp_var[0], 1, true);
                  SMTCell::setVar_wo_cnt(tmp_var[0], 0);
                }
              }
            } else if (cnt_var > 0) {
              // # true-assigned variable is not included in terms
              // # if # of rest variables is one, then that variable should be
              // false
              if (cnt_var != 2) {
                fmt::print("[ERROR] number of commodity pair is not equal to "
                           "2!! in Vertex[$vName -> $cnt_var]\n");
                exit(1);
              } else {
                SMTCell::writeConstraint(fmt::format("(assert (= {} {}))\n",
                                                     tmp_var[0], tmp_var[1]));
                SMTCell::cnt("l", 1);
                SMTCell::cnt("l", 1);
                SMTCell::cnt("c", 1);
              }
            }
          }
        }
      }
    }
  }

  // ### Net Variables for CFC
  for (int netIndex = 0; netIndex < SMTCell::getNetCnt(); netIndex++) {
    for (int commodityIndex = 0;
         commodityIndex < SMTCell::getNet(netIndex)->getNumSinks();
         commodityIndex++) {
      // ########### It might be redundancy, some net don't need some pins
      // ..... ###########
      for (int pinIndex = 0; pinIndex < SMTCell::getPinCnt(); pinIndex++) {
        std::string pName = SMTCell::getPin(pinIndex)->getPinName();
        if (pName ==
            SMTCell::getNet(netIndex)->getSinks_inNet(commodityIndex)) {
          if (pName == Pin::keySON) {
            // Super Outer Node
            if (EXT_Parameter == 0) {
              std::string tmp_str = "";         // building literal
              std::vector<std::string> tmp_var; // store all clauses
              int cnt_var = 0;
              int cnt_true = 0;
              // sink
              if (SMTCell::ifVEdgeIn(pName)) {
                for (int i : SMTCell::getVEdgeIn(pName)) {
                  int metal = SMTCell::getVirtualEdge(i)->getVCoord()->metal_;
                  int row = SMTCell::getVirtualEdge(i)->getVCoord()->row_;
                  if (SMTCell::ifVertMetal(metal) &&
                      (row == 0 || row == SMTCell::getNumTrackH() - 3)) {
                    tmp_str = fmt::format(
                        "N{}_C{}_E_{}_{}",
                        SMTCell::getNet(netIndex)->getNetID(), commodityIndex,
                        SMTCell::getVirtualEdge(i)->getVName(), pName);
                    switch (SMTCell::assignVar(tmp_str)) {
                    case 1:
                      tmp_var.push_back(tmp_str);
                      cnt_var++;
                      break;
                    case 0:
                      cnt_true++;
                      break;
                    }
                  }
                }
              }
              if (cnt_true > 0) {
                if (cnt_true > 1) {
                  fmt::print(
                      "[ERROR] # of true pinSON in Net[$nets[$netIndex][1]] "
                      "Commodity[$commodityIndex] exceeds one!!\n");
                  exit(1);
                } else {
                  // # if # of true variables is one, then remained variables
                  // should be false
                  for (auto s : tmp_var) {
                    SMTCell::assignTrueVar(s, 0, true);
                  }
                }
              } else {
                // # true-assigned variable is not included in terms
                if (cnt_var == 1) {
                  SMTCell::assignTrueVar(tmp_var[0], 1, true);
                  SMTCell::setVar_wo_cnt(tmp_var[0], 0);
                } else if (cnt_var > 0) {
                  // #at-most 1
                  SMTCell::writeConstraint("(assert ((_ at-most 1)");
                  for (auto s : tmp_var) {
                    SMTCell::writeConstraint(fmt::format(" {}", s));
                    SMTCell::cnt("l", 1);
                  }
                  SMTCell::writeConstraint("))\n");
                  SMTCell::cnt("c", 1);
                  // #at-least 1
                  SMTCell::writeConstraint("(assert ((_ at-least 1)");
                  for (auto s : tmp_var) {
                    SMTCell::writeConstraint(fmt::format(" {}", s));
                    ;
                    SMTCell::cnt("l", 1);
                  }
                  SMTCell::writeConstraint("))\n");
                  SMTCell::cnt("c", 1);
                }
              }
            } else {
              std::string tmp_str = "";         // building literal
              std::vector<std::string> tmp_var; // store all clauses
              int cnt_var = 0;
              int cnt_true = 0;
              // sink
              if (SMTCell::ifVEdgeIn(pName)) {
                for (int i : SMTCell::getVEdgeIn(pName)) {
                  int metal = SMTCell::getVirtualEdge(i)->getVCoord()->metal_;
                  // int row = SMTCell::getVirtualEdge(i)->getVCoord()->row_;
                  if (SMTCell::ifVertMetal(metal)) {
                    tmp_str = fmt::format(
                        "N{}_C{}_E_{}_{}",
                        SMTCell::getNet(netIndex)->getNetID(), commodityIndex,
                        SMTCell::getVirtualEdge(i)->getVName(), pName);
                    switch (SMTCell::assignVar(tmp_str)) {
                    case 1:
                      tmp_var.push_back(tmp_str);
                      cnt_var++;
                      break;
                    case 0:
                      cnt_true++;
                      break;
                    }
                  }
                }
              }
              if (cnt_true > 0) {
                if (cnt_true > 1) {
                  fmt::print(
                      "[ERROR] # of true pinSON in Net[$nets[$netIndex][1]] "
                      "Commodity[$commodityIndex] exceeds one!!\n");
                  exit(1);
                } else {
                  // # if # of true variables is one, then remained variables
                  // should be false
                  for (auto s : tmp_var) {
                    SMTCell::assignTrueVar(s, 0, true);
                  }
                }
              } else {
                // # true-assigned variable is not included in terms
                if (cnt_var == 1) {
                  SMTCell::assignTrueVar(tmp_var[0], 1, true);
                  SMTCell::setVar_wo_cnt(tmp_var[0], 0);
                } else if (cnt_var > 0) {
                  // #at-most 1
                  SMTCell::writeConstraint("(assert ((_ at-most 1)");
                  for (auto s : tmp_var) {
                    SMTCell::writeConstraint(fmt::format(" {}", s));
                    ;
                    SMTCell::cnt("l", 1);
                  }
                  SMTCell::writeConstraint("))\n");
                  SMTCell::cnt("c", 1);
                  // #at-least 1
                  SMTCell::writeConstraint("(assert ((_ at-least 1)");
                  for (auto s : tmp_var) {
                    SMTCell::writeConstraint(fmt::format(" {}", s));
                    ;
                    SMTCell::cnt("l", 1);
                  }
                  SMTCell::writeConstraint("))\n");
                  SMTCell::cnt("c", 1);
                }
              }
            }
          } else {
            int beginRow_idx = 0;
            int endRow_idx = 0;
            int instIdx = 0;
            // ## Sink MaxFlow Indicator
            if (instIdx <= SMTCell::getLastIdxPMOS()) {
              beginRow_idx = 0;
              // AGR FLAG : adapt to different track height
              if (SMTCell::getNumTrack() == 4 || SMTCell::getNumTrack() == 5) {
                endRow_idx =
                    floor(SMTCell::get_h_metal_numTrackH(1) / 2 + 0.5) - 2;
              } else if (SMTCell::getNumTrack() == 6) {
                endRow_idx = SMTCell::get_h_metal_numTrackH(1) / 2 - 2;
              } else {
                fmt::print(stderr, "[ERROR] unsupported track height\n");
                exit(1);
              }
            } else {
              // AGR FLAG : adapt to different track height
              if (SMTCell::getNumTrack() == 4 || SMTCell::getNumTrack() == 5) {
                beginRow_idx =
                    floor(SMTCell::get_h_metal_numTrackH(1) / 2 + 0.5) - 1;
              } else if (SMTCell::getNumTrack() == 6) {
                beginRow_idx = SMTCell::get_h_metal_numTrackH(1) / 2 - 1;
              } else {
                fmt::print(stderr, "[ERROR] unsupported track height\n");
                exit(1);
              }
              endRow_idx = SMTCell::get_h_metal_numTrackH(1) - 3;
            }

            for (int row_idx = beginRow_idx; row_idx <= endRow_idx; row_idx++) {
              // AGR FLAG : fix to metal 1
              int row = SMTCell::get_h_metal_row_by_idx(1, row_idx);
              if (SMTCell::ifRoutingTrack(row)) {
                std::string tmp_str = "";         // building literal
                std::vector<std::string> tmp_var; // store all clauses
                int cnt_var = 0;
                int cnt_true = 0;
                // sink
                if (SMTCell::ifVEdgeIn(pName)) {
                  for (int i : SMTCell::getVEdgeIn(pName)) {
                    tmp_str = fmt::format(
                        "N{}_C{}_E_{}_{}",
                        SMTCell::getNet(netIndex)->getNetID(), commodityIndex,
                        SMTCell::getVirtualEdge(i)->getVName(), pName);
                    switch (SMTCell::assignVar(tmp_str)) {
                    case 1:
                      tmp_var.push_back(tmp_str);
                      cnt_var++;
                      break;
                    case 0:
                      cnt_true++;
                      break;
                    }
                  }
                }
                if (cnt_true > 0) {
                  if (cnt_true > 1) {
                    fmt::print("[ERROR] # of true pinSON in "
                               "Net[$nets[$netIndex][1]] "
                               "Commodity[$commodityIndex] exceeds one!!\n");
                    exit(1);
                  } else {
                    // # if # of true variables is one, then remained
                    // variables should be false
                    for (auto s : tmp_var) {
                      SMTCell::assignTrueVar(s, 0, true);
                    }
                  }
                } else {
                  // # true-assigned variable is not included in terms
                  if (cnt_var == 1) {
                    SMTCell::assignTrueVar(tmp_var[0], 1, true);
                    SMTCell::setVar_wo_cnt(tmp_var[0], 0);
                  } else if (cnt_var > 0) {
                    // #at-most 1
                    SMTCell::writeConstraint("(assert ((_ at-most 1)");
                    for (auto s : tmp_var) {
                      SMTCell::writeConstraint(fmt::format(" {}", s));
                      ;
                      SMTCell::cnt("l", 1);
                    }
                    SMTCell::writeConstraint("))\n");
                    SMTCell::cnt("c", 1);
                    // #at-least 1
                    SMTCell::writeConstraint("(assert ((_ at-least 1)");
                    for (auto s : tmp_var) {
                      SMTCell::writeConstraint(fmt::format(" {}", s));
                      ;
                      SMTCell::cnt("l", 1);
                    }
                    SMTCell::writeConstraint("))\n");
                    SMTCell::cnt("c", 1);
                  }
                }
              }
            }
          }
        }

        if (pName == SMTCell::getNet(netIndex)->getSource_ofNet()) {
          // int beginRow_idx = 0;
          // int endRow_idx = 0;
          // int instIdx = 0;
          // // ## Source MaxFlow Indicator
          // if (instIdx <= SMTCell::getLastIdxPMOS()) {
          //   beginRow_idx = 0;
          //   // AGR FLAG : adapt to different track height
          //   if (SMTCell::getNumTrack() == 4 || SMTCell::getNumTrack() == 5) {
          //     endRow_idx =
          //         floor(SMTCell::get_h_metal_numTrackH(1) / 2 + 0.5) - 2;
          //   } else if (SMTCell::getNumTrack() == 6) {
          //     endRow_idx = SMTCell::get_h_metal_numTrackH(1) / 2 - 2;
          //   } else {
          //     fmt::print(stderr, "[ERROR] unsupported track height\n");
          //     exit(1);
          //   }
          // } else {
          //   // AGR FLAG : adapt to different track height
          //   if (SMTCell::getNumTrack() == 4 || SMTCell::getNumTrack() == 5) {
          //     beginRow_idx =
          //         floor(SMTCell::get_h_metal_numTrackH(1) / 2 + 0.5) - 1;
          //   } else if (SMTCell::getNumTrack() == 6) {
          //     beginRow_idx = SMTCell::get_h_metal_numTrackH(1) / 2 - 1;
          //   } else {
          //     fmt::print(stderr, "[ERROR] unsupported track height\n");
          //     exit(1);
          //   }
          //   endRow_idx = SMTCell::get_h_metal_numTrackH(1) - 3;
          // }

          std::string tmp_str = "";         // building literal
          std::vector<std::string> tmp_var; // store all clauses
          int cnt_var = 0;
          int cnt_true = 0;
          // sink
          if (SMTCell::ifVEdgeIn(pName)) {
            for (int i : SMTCell::getVEdgeIn(pName)) {
              tmp_str = fmt::format(
                  "N{}_C{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                  commodityIndex, SMTCell::getVirtualEdge(i)->getVName(),
                  pName);
              switch (SMTCell::assignVar(tmp_str)) {
              case 1:
                tmp_var.push_back(tmp_str);
                cnt_var++;
                break;
              case 0:
                cnt_true++;
                break;
              }
            }
          }
          if (cnt_true > 0) {
            if (cnt_true > 1) {
              fmt::print("[ERROR] # of true pinSON in Net[$nets[$netIndex][1]] "
                         "Commodity[$commodityIndex] exceeds one!!\n");
              exit(1);
            } else {
              // # if # of true variables is one, then remained variables
              // should be false
              for (auto s : tmp_var) {
                SMTCell::assignTrueVar(s, 0, true);
              }
            }
          } else {
            // # true-assigned variable is not included in terms
            if (cnt_var == 1) {
              SMTCell::assignTrueVar(tmp_var[0], 1, true);
              SMTCell::setVar_wo_cnt(tmp_var[0], 0);
            } else if (cnt_var > 0) {
              // #at-most 1
              SMTCell::writeConstraint("(assert ((_ at-most 1)");
              for (auto s : tmp_var) {
                SMTCell::writeConstraint(fmt::format(" {}", s));
                ;
                SMTCell::cnt("l", 1);
              }
              SMTCell::writeConstraint("))\n");
              SMTCell::cnt("c", 1);
              // #at-least 1
              SMTCell::writeConstraint("(assert ((_ at-least 1)");
              for (auto s : tmp_var) {
                SMTCell::writeConstraint(fmt::format(" {}", s));
                ;
                SMTCell::cnt("l", 1);
              }
              SMTCell::writeConstraint("))\n");
              SMTCell::cnt("c", 1);
            }
          }
        }
      }
    }
  }

  std::string tmp_str = "";
  std::vector<std::string> tmp_var;
  int cnt_var = 0;
  int cnt_true = 0;

  for (int netIndex = 0; netIndex < SMTCell::getNetCnt(); netIndex++) {
    for (int commodityIndex = 0;
         commodityIndex < SMTCell::getNet(netIndex)->getNumSinks();
         commodityIndex++) {
      for (int metal = 1; metal <= SMTCell::getNumMetalLayer(); metal++) {
        for (int row_idx = 0;
             row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3; row_idx++) {
          for (int col_idx = 0;
               col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1;
               col_idx++) {
            // retrieve row/col
            int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
            int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);
            // if (metal > 1 && SMTCell::ifVertMetal(metal) &&
            //     SMTCell::ifSDCol_AGR(1, col)) {
            //   continue;
            // }
            Triplet vCoord = Triplet(metal, row, col);
            // sink
            if (SMTCell::ifVEdgeOut(vCoord.getVName())) {
              for (int i : SMTCell::getVEdgeOut(vCoord.getVName())) {
                if (SMTCell::getVirtualEdge(i)->getPinName() ==
                        SMTCell::getNet(netIndex)->getSinks_inNet(
                            commodityIndex) &&
                    SMTCell::getVirtualEdge(i)->getPinName() == Pin::keySON) {
                  tmp_str = fmt::format(
                      "N{}_C{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                      commodityIndex, vCoord.getVName(),
                      SMTCell::getVirtualEdge(i)->getPinName());
                  switch (SMTCell::assignVar(tmp_str)) {
                  case 1:
                    tmp_var.push_back(tmp_str);
                    cnt_var++;
                    break;
                  case 0:
                    cnt_true++;
                    break;
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  if (cnt_true > 0) {
    if (cnt_true > SMTCell::getOuterPinCnt()) {
      fmt::print("[ERROR] # of true pinSON exceeds $numOuterPins!!\n");
      exit(1);
    } else if (cnt_true == SMTCell::getOuterPinCnt()) {
      // # if # of true variables is the same as # of outerpins, then remained
      // variables should be false
      for (auto s : tmp_var) {
        SMTCell::assignTrueVar(s, 0, true);
      }
    } else {
      // # at-most $numOuterPins-$cnt_true
      SMTCell::writeConstraint(fmt::format(
          "(assert ((_ at-most {})", (SMTCell::getOuterPinCnt() - cnt_true)));
      for (auto s : tmp_var) {
        SMTCell::writeConstraint(fmt::format(" {}", s));
        SMTCell::cnt("l", 1);
      }
      SMTCell::writeConstraint("))\n");
      SMTCell::cnt("c", 1);
      // # at-least $numOuterPins-$cnt_true
      SMTCell::writeConstraint(fmt::format(
          "(assert ((_ at-least {})", (SMTCell::getOuterPinCnt() - cnt_true)));
      for (auto s : tmp_var) {
        SMTCell::writeConstraint(fmt::format(" {}", s));
        SMTCell::cnt("l", 1);
      }
      SMTCell::writeConstraint("))\n");
      SMTCell::cnt("c", 1);
    }
  } else {
    if (cnt_var > 0) {
      // #at-most numOuterPins
      SMTCell::writeConstraint(
          fmt::format("(assert ((_ at-most {})", SMTCell::getOuterPinCnt()));
      for (auto s : tmp_var) {
        SMTCell::writeConstraint(fmt::format(" {}", s));
        SMTCell::cnt("l", 1);
      }
      SMTCell::writeConstraint("))\n");
      SMTCell::cnt("c", 1);
      // #at-least numOuterPins
      SMTCell::writeConstraint(
          fmt::format("(assert ((_ at-least {})", SMTCell::getOuterPinCnt()));
      for (auto s : tmp_var) {
        SMTCell::writeConstraint(fmt::format(" {}", s));
        SMTCell::cnt("l", 1);
      }
      SMTCell::writeConstraint("))\n");
      SMTCell::cnt("c", 1);
    }
  }

  std::cout << "has been written.\n";
}

void FlowWriter::write_vertex_exclusive(FILE *fp) {
  SMTCell::writeConstraint(";2. Exclusiveness use of vertex for each vertex "
                           "and every connected edge to the vertex\n");
  for (int metal = 1; metal <= SMTCell::getNumMetalLayer(); metal++) {
    for (int row_idx = 0; row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3;
         row_idx++) {
      for (int col_idx = 0;
           col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
        // retrieve row/col
        int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
        int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);
        // if (metal > 1 && SMTCell::ifVertMetal(metal) &&
        //     SMTCell::ifSDCol_AGR(1, col)) {
        //   continue;
        // }
        Triplet vCoord = {metal, row, col};
        int cnt_true_net = 0;
        std::vector<std::string> tmp_var_net;
        int cnt_var_net = 0;
        for (int netIndex = 0; netIndex < SMTCell::getNetCnt(); netIndex++) {
          std::string tmp_str = "";         // building literal
          std::vector<std::string> tmp_var; // store all clauses
          int cnt_var = 0;
          int cnt_true = 0;
          // incoming
          if (SMTCell::ifEdgeIn(vCoord.getVName())) {
            for (int i : SMTCell::getEdgeIn(vCoord.getVName())) {
              tmp_str = fmt::format(
                  "N{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                  SMTCell::getUdEdgeFromVarName(i), vCoord.getVName());
              // make this into a class method
              switch (SMTCell::assignVar(tmp_str)) {
              case 1:
                tmp_var.push_back(tmp_str);
                cnt_var++;
                break;
              case 0:
                cnt_true++;
                break;
              }
            }
          }

          // outcoming
          if (SMTCell::ifEdgeOut(vCoord.getVName())) {
            for (int i : SMTCell::getEdgeOut(vCoord.getVName())) {
              tmp_str = fmt::format(
                  "N{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                  vCoord.getVName(), SMTCell::getUdEdgeToVarName(i));
              // make this into a class method
              switch (SMTCell::assignVar(tmp_str)) {
              case 1:
                tmp_var.push_back(tmp_str);
                cnt_var++;
                break;
              case 0:
                cnt_true++;
                break;
              }
            }
          }

          // v-outcoming
          if (SMTCell::ifVEdgeOut(vCoord.getVName())) {
            for (int i : SMTCell::getVEdgeOut(vCoord.getVName())) {
              if (SMTCell::getVirtualEdge(i)->getPinName() ==
                  SMTCell::getNet(netIndex)->getSource_ofNet()) {
                tmp_str = fmt::format("N{}_E_{}_{}",
                                      SMTCell::getNet(netIndex)->getNetID(),
                                      vCoord.getVName(),
                                      SMTCell::getVirtualEdge(i)->getPinName());
                switch (SMTCell::assignVar(tmp_str)) {
                case 1:
                  tmp_var.push_back(tmp_str);
                  cnt_var++;
                  break;
                case 0:
                  cnt_true++;
                  break;
                }
              }
              for (int commodityIndex = 0;
                   commodityIndex < SMTCell::getNet(netIndex)->getNumSinks();
                   commodityIndex++) {
                if (SMTCell::getVirtualEdge(i)->getPinName() ==
                    SMTCell::getNet(netIndex)->getSinks_inNet(commodityIndex)) {
                  tmp_str = fmt::format(
                      "N{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                      vCoord.getVName(),
                      SMTCell::getVirtualEdge(i)->getPinName());
                  switch (SMTCell::assignVar(tmp_str)) {
                  case 1:
                    tmp_var.push_back(tmp_str);
                    cnt_var++;
                    break;
                  case 0:
                    cnt_true++;
                    break;
                  }
                }
              }
            }
          }
          std::string tmp_enc = "";
          tmp_enc =
              fmt::format("C_N{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                          vCoord.getVName());
          if (cnt_true > 0) {
            SMTCell::setVar_wo_cnt(tmp_enc, 0);
            cnt_true_net++;
          } else if (cnt_var > 0) {
            SMTCell::setVar(tmp_enc, 0);
            tmp_var_net.push_back(tmp_enc);
            cnt_var_net++;
            SMTCell::writeConstraint(fmt::format("(assert (= {} (or", tmp_enc));
            SMTCell::cnt("l", 1);
            for (auto s : tmp_var) {
              SMTCell::writeConstraint(fmt::format(" {}", s));
              SMTCell::cnt("l", 1);
            }
            SMTCell::writeConstraint(")))\n");
            SMTCell::cnt("c", 1);
          }
        }
        if (cnt_true_net > 0) {
          fmt::print(
              "[ERROR] There exsits more than 2 nets sharing same vertex[{}]",
              vCoord.getVName());
          exit(1);
        } else if (cnt_true_net == 1) {
          // remained net encode variables shoule be false
          for (auto s : tmp_var_net) {
            SMTCell::assignTrueVar(s, 0, true);
          }
        } else if (cnt_var_net > 0) {
          // at-most 1
          SMTCell::writeConstraint("(assert ((_ at-most 1)");
          for (auto s : tmp_var_net) {
            SMTCell::writeConstraint(fmt::format(" {}", s));
            SMTCell::cnt("l", 1);
          }
          SMTCell::writeConstraint("))\n");
          SMTCell::cnt("c", 1);
        }
      }
    }
  }

  for (int netIndex = 0; netIndex < SMTCell::getNetCnt(); netIndex++) {
    std::string tmp_str = "";
    std::vector<std::string> tmp_var;
    int cnt_var = 0;
    int cnt_true = 0;
    for (int metal = 1; metal <= SMTCell::getNumMetalLayer(); metal++) {
      for (int row_idx = 0;
           row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3; row_idx++) {
        for (int col_idx = 0;
             col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
          // retrieve row/col
          int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
          int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);
          // if (metal > 1 && SMTCell::ifVertMetal(metal) &&
          //     SMTCell::ifSDCol_AGR(1, col)) {
          //   continue;
          // }
          Triplet vCoord = {metal, row, col};
          if (SMTCell::ifVEdgeOut(vCoord.getVName())) {
            for (int i : SMTCell::getVEdgeOut(vCoord.getVName())) {
              if (SMTCell::getVirtualEdge(i)->getPinName() ==
                  SMTCell::getNet(netIndex)->getSource_ofNet()) {
                tmp_str = fmt::format("N{}_E_{}_{}",
                                      SMTCell::getNet(netIndex)->getNetID(),
                                      vCoord.getVName(),
                                      SMTCell::getVirtualEdge(i)->getPinName());
                switch (SMTCell::assignVar(tmp_str)) {
                case 1:
                  tmp_var.push_back(tmp_str);
                  cnt_var++;
                  break;
                case 0:
                  cnt_true++;
                  break;
                }
              }
            }
          }
        }
      }
    }
    if (cnt_true > 0) {
      if (cnt_true > 1) {
        fmt::print("[ERROR] # of true pin in Net{}] exceeds one!!\n",
                   SMTCell::getNet(netIndex)->getNetID());
        exit(1);
      } else {
        // if # of true variables is one, then remained variables should be
        // false
        for (auto s : tmp_var) {
          SMTCell::assignTrueVar(s, 0, true);
        }
      }
    }
    // true-assigned variable is not included in terms
    else {
      if (cnt_var == 1) {
        SMTCell::assignTrueVar(tmp_var[0], 1, true);
        SMTCell::setVar_wo_cnt(tmp_var[0], 0);
      } else if (cnt_var > 0) {
        // at-most 1
        SMTCell::writeConstraint("(assert ((_ at-most 1)");
        for (auto s : tmp_var) {
          SMTCell::writeConstraint(fmt::format(" {}", s));
          SMTCell::cnt("l", 1);
        }
        SMTCell::writeConstraint("))\n");
        SMTCell::cnt("c", 1);
        // at-least 1
        SMTCell::writeConstraint("(assert ((_ at-least 1)");
        for (auto s : tmp_var) {
          SMTCell::writeConstraint(fmt::format(" {}", s));
          SMTCell::cnt("l", 1);
        }
        SMTCell::writeConstraint("))\n");
        SMTCell::cnt("c", 1);
      }
    }
  }

  // tmp_str.clear();
  // tmp_var.clear();
  // cnt_var = 0;
  // cnt_true = 0;
  std::string tmp_str;
  std::vector<std::string> tmp_var;
  int cnt_var = 0;
  int cnt_true = 0;
  for (int netIndex = 0; netIndex < SMTCell::getNetCnt(); netIndex++) {
    for (int metal = 1; metal <= SMTCell::getNumMetalLayer(); metal++) {
      for (int row_idx = 0;
           row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3; row_idx++) {
        for (int col_idx = 0;
             col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
          // retrieve row/col
          int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
          int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);
          // if (metal > 1 && SMTCell::ifVertMetal(metal) &&
          //     SMTCell::ifSDCol_AGR(1, col)) {
          //   continue;
          // }
          Triplet vCoord = {metal, row, col};
          if (SMTCell::ifVEdgeOut(vCoord.getVName())) {
            for (int i : SMTCell::getVEdgeOut(vCoord.getVName())) {
              for (int commodityIndex = 0;
                   commodityIndex < SMTCell::getNet(netIndex)->getNumSinks();
                   commodityIndex++) {
                if (SMTCell::getVirtualEdge(i)->getPinName() ==
                        SMTCell::getNet(netIndex)->getSinks_inNet(
                            commodityIndex) &&
                    SMTCell::getVirtualEdge(i)->getPinName() == Pin::keySON) {
                  tmp_str = fmt::format(
                      "N{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                      vCoord.getVName(),
                      SMTCell::getVirtualEdge(i)->getPinName());
                  switch (SMTCell::assignVar(tmp_str)) {
                  case 1:
                    tmp_var.push_back(tmp_str);
                    cnt_var++;
                    break;
                  case 0:
                    cnt_true++;
                    break;
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  if (cnt_true > 0) {
    if (cnt_true > SMTCell::getOuterPinCnt()) {
      fmt::print("[ERROR] # of true pinson exceeds {}!!\n",
                 SMTCell::getOuterPinCnt());
      exit(1);
    } else if (cnt_true == SMTCell::getOuterPinCnt()) {
      // # if # of true variables is the same as # of outerpins, then
      // remained variables should be false
      for (auto s : tmp_var) {
        switch (SMTCell::assignVar(s)) {
        case 1:
          SMTCell::assignTrueVar(s, 0, true);
          break;
        case 0:
          break;
        }
      }
    } else {
      // # at-most $numOuterPins-cnt_true
      SMTCell::writeConstraint(fmt::format(
          "(assert ((_ at-most {})", (SMTCell::getOuterPinCnt() - cnt_true)));
      for (auto s : tmp_var) {
        SMTCell::writeConstraint(fmt::format(" {}", s));
        SMTCell::cnt("l", 1);
      }
      SMTCell::writeConstraint("))\n");
      SMTCell::cnt("c", 1);
      // at-least numOuterPins-cnt_true
      SMTCell::writeConstraint(fmt::format(
          "(assert ((_ at-least {})", (SMTCell::getOuterPinCnt() - cnt_true)));
      for (auto s : tmp_var) {
        SMTCell::writeConstraint(fmt::format(" {}", s));
        SMTCell::cnt("l", 1);
      }
      SMTCell::writeConstraint("))\n");
      SMTCell::cnt("c", 1);
    }
  }
  // true-assigned variable is not included in terms
  else {
    if (cnt_var > 0) {
      // at-most numOuterPins
      SMTCell::writeConstraint(
          fmt::format("(assert ((_ at-most {})", SMTCell::getOuterPinCnt()));
      for (auto s : tmp_var) {
        SMTCell::writeConstraint(fmt::format(" {}", s));
        SMTCell::cnt("l", 1);
      }
      SMTCell::writeConstraint("))\n");
      SMTCell::cnt("c", 1);
      // at-least numOuterPins
      SMTCell::writeConstraint(
          fmt::format("(assert ((_ at-least {})", SMTCell::getOuterPinCnt()));
      for (auto s : tmp_var) {
        SMTCell::writeConstraint(fmt::format(" {}", s));
        SMTCell::cnt("l", 1);
      }
      SMTCell::writeConstraint("))\n");
      SMTCell::cnt("c", 1);
    }
  }
  std::cout << "has been written.\n";
}

void FlowWriter::write_edge_assignment(FILE *fp) {
  SMTCell::writeConstraint(";3. Edge assignment for each edge for every net\n");
  for (int netIndex = 0; netIndex < SMTCell::getNetCnt(); netIndex++) {
    for (int udeIndex = 0; udeIndex < SMTCell::getUdEdgeCnt(); udeIndex++) {
      for (int commodityIndex = 0;
           commodityIndex < SMTCell::getNet(netIndex)->getNumSinks();
           commodityIndex++) {
        std::string tmp_com = "";
        tmp_com = fmt::format(
            "N{}_C{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
            commodityIndex,
            SMTCell::getUdEdge(udeIndex)->getUdEdgeFromVar()->getVName(),
            SMTCell::getUdEdge(udeIndex)->getUdEdgeToVar()->getVName());
        std::string tmp_net = "";
        tmp_net = fmt::format(
            "N{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
            SMTCell::getUdEdge(udeIndex)->getUdEdgeFromVar()->getVName(),
            SMTCell::getUdEdge(udeIndex)->getUdEdgeToVar()->getVName());
        if (!SMTCell::ifAssigned(tmp_com) && !SMTCell::ifAssigned(tmp_net)) {
          SMTCell::setVar(tmp_net, 2);
          SMTCell::setVar(tmp_net, 2);
          SMTCell::writeConstraint(
              fmt::format("(assert (ite (= {} true) (= {} true) (= 1 1)))\n",
                          tmp_com, tmp_net));
          SMTCell::cnt("l", 1);
          SMTCell::cnt("l", 1);
          SMTCell::cnt("c", 1);
        } else if (SMTCell::ifAssigned(tmp_com) &&
                   SMTCell::getAssigned(tmp_com) == 1) {
          SMTCell::assignTrueVar(tmp_net, 1, true);
          SMTCell::setVar_wo_cnt(tmp_net, 2);
        }
      }
    }
    for (int vEdgeIndex = 0; vEdgeIndex < SMTCell::getVirtualEdgeCnt();
         vEdgeIndex++) {
      int isInNet = 0;
      // ignoring $virtualEdges[$vEdgeIndex][2] =~ /^pin/ since this is always
      // a pin name
      if (SMTCell::getVirtualEdge(vEdgeIndex)->getPinName() ==
          SMTCell::getNet(netIndex)->getSource_ofNet()) {
        isInNet = 1;
      }
      if (isInNet == 1) {
        for (int commodityIndex = 0;
             commodityIndex < SMTCell::getNet(netIndex)->getNumSinks();
             commodityIndex++) {
          std::string tmp_com = "";
          tmp_com = fmt::format(
              "N{}_C{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
              commodityIndex, SMTCell::getVirtualEdge(vEdgeIndex)->getVName(),
              SMTCell::getVirtualEdge(vEdgeIndex)->getPinName());
          std::string tmp_net = "";
          tmp_net =
              fmt::format("N{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                          SMTCell::getVirtualEdge(vEdgeIndex)->getVName(),
                          SMTCell::getVirtualEdge(vEdgeIndex)->getPinName());
          if (!SMTCell::ifAssigned(tmp_com) && !SMTCell::ifAssigned(tmp_net)) {
            SMTCell::setVar(tmp_net, 2);
            SMTCell::setVar(tmp_net, 2);
            SMTCell::writeConstraint(
                fmt::format("(assert (ite (= {} true) (= {} true) (= 1 1)))\n",
                            tmp_com, tmp_net));
            SMTCell::cnt("l", 1);
            SMTCell::cnt("l", 1);
            SMTCell::cnt("c", 1);
          } else if (SMTCell::ifAssigned(tmp_com) &&
                     SMTCell::getAssigned(tmp_com) == 1) {
            SMTCell::assignTrueVar(tmp_net, 1, true);
            SMTCell::setVar_wo_cnt(tmp_net, 2);
          }
        }
      }
      isInNet = 0;
      for (int i = 0; i < SMTCell::getNet(netIndex)->getNumSinks(); i++) {
        if (SMTCell::getVirtualEdge(vEdgeIndex)->getPinName() ==
            SMTCell::getNet(netIndex)->getSinks_inNet(i)) {
          isInNet = 1;
        }
      }
      if (isInNet == 1) {
        for (int commodityIndex = 0;
             commodityIndex < SMTCell::getNet(netIndex)->getNumSinks();
             commodityIndex++) {
          if (SMTCell::getVirtualEdge(vEdgeIndex)->getPinName() ==
              SMTCell::getNet(netIndex)->getSinks_inNet(commodityIndex)) {
            std::string tmp_com = "";
            tmp_com = fmt::format(
                "N{}_C{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                commodityIndex, SMTCell::getVirtualEdge(vEdgeIndex)->getVName(),
                SMTCell::getVirtualEdge(vEdgeIndex)->getPinName());
            std::string tmp_net = "";
            tmp_net = fmt::format(
                "N{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                SMTCell::getVirtualEdge(vEdgeIndex)->getVName(),
                SMTCell::getVirtualEdge(vEdgeIndex)->getPinName());
            if (!SMTCell::ifAssigned(tmp_com) &&
                !SMTCell::ifAssigned(tmp_net)) {
              SMTCell::setVar(tmp_net, 2);
              SMTCell::setVar(tmp_net, 2);
              SMTCell::writeConstraint(fmt::format(
                  "(assert (ite (= {} true) (= {} true) (= 1 1)))\n", tmp_com,
                  tmp_net));
              SMTCell::cnt("l", 1);
              SMTCell::cnt("l", 1);
              SMTCell::cnt("c", 1);
            } else if (SMTCell::ifAssigned(tmp_com) &&
                       SMTCell::getAssigned(tmp_com) == 1) {
              SMTCell::assignTrueVar(tmp_net, 1, true);
              SMTCell::setVar_wo_cnt(tmp_net, 2);
            }
          }
        }
      }
    }
  }
  std::cout << "has been written.\n";
  SMTCell::writeConstraint("\n");
}

void FlowWriter::write_edge_exclusive(FILE *fp) {
  SMTCell::writeConstraint(
      ";4. Exclusiveness use of each edge + Metal segment assignment by "
      "using edge usage information\n");
  for (int udeIndex = 0; udeIndex < SMTCell::getUdEdgeCnt(); udeIndex++) {
    // tmp_str.clear();
    // tmp_var.clear();
    // cnt_var = 0;
    // cnt_true = 0;
    std::string tmp_str;
    std::vector<std::string> tmp_var;
    int cnt_var = 0;
    int cnt_true = 0;
    for (int netIndex = 0; netIndex < SMTCell::getNetCnt(); netIndex++) {
      tmp_str = fmt::format(
          "N{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
          SMTCell::getUdEdge(udeIndex)->getUdEdgeFromVar()->getVName(),
          SMTCell::getUdEdge(udeIndex)->getUdEdgeToVar()->getVName());
      switch (SMTCell::assignVar(tmp_str)) {
      case 1:
        tmp_var.push_back(tmp_str);
        cnt_var++;
        break;
      case 0:
        cnt_true++;
        break;
      }
    }

    std::string tmp_str_metal = fmt::format(
        "M_{}_{}", SMTCell::getUdEdge(udeIndex)->getUdEdgeFromVar()->getVName(),
        SMTCell::getUdEdge(udeIndex)->getUdEdgeToVar()->getVName());

    if (cnt_true > 0) {
      SMTCell::assignTrueVar(tmp_str_metal, 1, true);
      SMTCell::setVar_wo_cnt(tmp_str_metal, 0);
    } else if (cnt_var > 0) {
      if (SMTCell::getAssigned(tmp_str_metal) == 0) {
        for (auto s : tmp_var) {
          SMTCell::assignTrueVar(s, 0, true);
        }
      } else {
        SMTCell::setVar(tmp_str_metal, 2);
        // # OR
        SMTCell::writeConstraint(
            fmt::format("(assert (= {} (or", tmp_str_metal));
        SMTCell::cnt("l", 1);
        for (auto s : tmp_var) {
          SMTCell::writeConstraint(fmt::format(" {}", s));
          SMTCell::cnt("l", 1);
        }
        SMTCell::writeConstraint(")))\n");
        SMTCell::cnt("c", 1);
        // at-most 1
        SMTCell::writeConstraint("(assert ((_ at-most 1)");
        for (auto s : tmp_var) {
          SMTCell::writeConstraint(fmt::format(" {}", s));
          SMTCell::cnt("l", 1);
        }
        SMTCell::writeConstraint("))\n");
        SMTCell::cnt("c", 1);
      }
    }
  }

  for (int vEdgeIndex = 0; vEdgeIndex < SMTCell::getVirtualEdgeCnt();
       vEdgeIndex++) {
    // tmp_str.clear();
    // tmp_var.clear();
    // cnt_var = 0;
    // cnt_true = 0;
    std::string tmp_str;
    std::vector<std::string> tmp_var;
    int cnt_var = 0;
    int cnt_true = 0;

    for (int netIndex = 0; netIndex < SMTCell::getNetCnt(); netIndex++) {
      int isInNet = 0;
      // ignore regex for pin
      if (SMTCell::getVirtualEdge(vEdgeIndex)->getPinName() ==
          SMTCell::getNet(netIndex)->getSource_ofNet()) {
        isInNet = 1;
      }

      if (isInNet == 1) {
        tmp_str =
            fmt::format("N{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                        SMTCell::getVirtualEdge(vEdgeIndex)->getVName(),
                        SMTCell::getVirtualEdge(vEdgeIndex)->getPinName());
        switch (SMTCell::assignVar(tmp_str)) {
        case 1:
          tmp_var.push_back(tmp_str);
          cnt_var++;
          break;
        case 0:
          cnt_true++;
          break;
        }
      }

      isInNet = 0;
      for (int i = 0; i < SMTCell::getNet(netIndex)->getNumSinks(); i++) {
        if (SMTCell::getVirtualEdge(vEdgeIndex)->getPinName() ==
            SMTCell::getNet(netIndex)->getSinks_inNet(i)) {
          isInNet = 1;
        }
      }

      if (isInNet == 1) {
        tmp_str =
            fmt::format("N{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                        SMTCell::getVirtualEdge(vEdgeIndex)->getVName(),
                        SMTCell::getVirtualEdge(vEdgeIndex)->getPinName());
        switch (SMTCell::assignVar(tmp_str)) {
        case 1:
          tmp_var.push_back(tmp_str);
          cnt_var++;
          break;
        case 0:
          cnt_true++;
          break;
        }
      }
    }

    std::string tmp_str_metal =
        fmt::format("M_{}_{}", SMTCell::getVirtualEdge(vEdgeIndex)->getVName(),
                    SMTCell::getVirtualEdge(vEdgeIndex)->getPinName());

    if (cnt_true > 0) {
      SMTCell::assignTrueVar(tmp_str_metal, 1, true);
      SMTCell::setVar_wo_cnt(tmp_str_metal, 0);
    } else if (cnt_var > 0) {
      if (SMTCell::getAssigned(tmp_str_metal) == 0) {
        for (auto s : tmp_var) {
          SMTCell::assignTrueVar(s, 0, true);
        }
      } else if (cnt_var == 1) {
        SMTCell::setVar(tmp_str_metal, 2);
        SMTCell::writeConstraint(
            fmt::format("(assert (= {} {}))\n", tmp_var[0], tmp_str_metal));
      } else {
        SMTCell::setVar(tmp_str_metal, 2);
        // # OR
        SMTCell::writeConstraint(
            fmt::format("(assert (= {} (or", tmp_str_metal));
        SMTCell::cnt("l", 1);
        for (auto s : tmp_var) {
          SMTCell::writeConstraint(fmt::format(" {}", s));
          SMTCell::cnt("l", 1);
        }
        SMTCell::writeConstraint(")))\n");
        SMTCell::cnt("c", 1);
        // at-most 1
        SMTCell::writeConstraint("(assert ((_ at-most 1)");
        for (auto s : tmp_var) {
          SMTCell::writeConstraint(fmt::format(" {}", s));
          SMTCell::cnt("l", 1);
        }
        SMTCell::writeConstraint("))\n");
        SMTCell::cnt("c", 1);
      }
    }
  }
  std::cout << "has been written.\n";
}

void FlowWriter::write_net_consistency() {
  if (SMTCell::getNumMetalLayer() > 2) {
    std::map<std::string, int> h_tmp;
    for (auto en : SMTCell::getExtNet()) {
      int netIndex = SMTCell::getNetIdx(std::to_string(en.first));
      int metal = 3;
      for (int col_idx = 0;
           col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
        // retrieve col
        int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);
        std::string tmp_net_col_name = fmt::format("{}_{}", en.first, col);
        if (h_tmp.find(tmp_net_col_name) == h_tmp.end()) {
          // for (int row_i = 0; row_i <= SMTCell::getNumTrackH() - 3; row_i++)
          // {
          for (int row_i_idx = 0;
               row_i_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3;
               row_i_idx++) {
            // retrieve row
            int row_i = SMTCell::get_h_metal_row_by_idx(metal, row_i_idx);
            if (metal > 1 && SMTCell::ifVertMetal(metal) &&
                SMTCell::ifSDCol_AGR(metal, col)) {
              continue;
            }
            Triplet *vCoord_i = new Triplet(metal, row_i, col);
            if (SMTCell::ifVEdgeOut(vCoord_i->getVName())) {
              for (int i : SMTCell::getVEdgeOut(vCoord_i->getVName())) {
                for (int commodityIndex = 0;
                     commodityIndex < SMTCell::getNet(netIndex)->getNumSinks();
                     commodityIndex++) {
                  if (SMTCell::getVirtualEdge(i)->getPinName() ==
                          SMTCell::getNet(netIndex)->getSinks_inNet(
                              commodityIndex) &&
                      SMTCell::getVirtualEdge(i)->getPinName() == Pin::keySON) {
                    if (h_tmp.find(tmp_net_col_name) == h_tmp.end()) {
                      h_tmp[tmp_net_col_name] = 1;
                    }

                    std::string tmp_str_e = fmt::format(
                        "N{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                        vCoord_i->getVName(),
                        SMTCell::getVirtualEdge(i)->getPinName());

                    if (!SMTCell::ifAssigned(tmp_str_e) ||
                        SMTCell::ifAssignedTrue(tmp_str_e)) {
                      for (int row_j = 0; row_j <= SMTCell::getNumTrackH() - 3;
                           row_j++) {
                        Triplet *vCoord_j = new Triplet(metal, row_j, col);
                        std::string tmp_str = "";
                        std::vector<std::string> tmp_var_self;
                        std::vector<std::string> tmp_var_self_c;
                        int cnt_var_self = 0;
                        int cnt_var_self_c = 0;
                        int cnt_true_self = 0;
                        int cnt_true_self_c = 0;

                        if (SMTCell::ifVertex((*vCoord_j)) &&
                            SMTCell::getVertex((*vCoord_j))
                                ->getBackADJ()
                                ->ifValid()) {
                          tmp_str =
                              fmt::format("N{}_E_{}_{}",
                                          SMTCell::getNet(netIndex)->getNetID(),
                                          vCoord_j->getVName(),
                                          SMTCell::getVertex((*vCoord_j))
                                              ->getBackADJ()
                                              ->getVName());
                          if (!SMTCell::ifAssigned(tmp_str)) {
                            tmp_var_self.push_back(tmp_str);
                            SMTCell::setVar(tmp_str, 2);
                            cnt_var_self++;
                          } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                            SMTCell::setVar_wo_cnt(tmp_str, 2);
                            cnt_true_self++;
                          }
                          // potential bug: nested for loop with same index
                          // name?
                          for (int commodityIndex = 0;
                               commodityIndex <
                               SMTCell::getNet(netIndex)->getNumSinks();
                               commodityIndex++) {
                            tmp_str = fmt::format(
                                "N{}_C{}_E_{}_{}",
                                SMTCell::getNet(netIndex)->getNetID(),
                                commodityIndex, vCoord_j->getVName(),
                                SMTCell::getVertex((*vCoord_j))
                                    ->getBackADJ()
                                    ->getVName());
                            if (!SMTCell::ifAssigned(tmp_str)) {
                              tmp_var_self_c.push_back(tmp_str);
                              SMTCell::setVar(tmp_str, 2);
                              cnt_var_self_c++;
                            } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                              SMTCell::setVar_wo_cnt(tmp_str, 2);
                              cnt_true_self_c++;
                            }
                          }
                          if (!SMTCell::ifAssigned(tmp_str_e)) {
                            SMTCell::setVar(tmp_str_e, 2);
                            SMTCell::writeConstraint(
                                fmt::format("(assert (ite (and (= "
                                            "N{}_M2_TRACK false) (= {} true)",
                                            en.first, tmp_str_e));
                            SMTCell::cnt("l", 3);
                          } else if (SMTCell::ifAssignedTrue(tmp_str_e)) {
                            SMTCell::setVar_wo_cnt(tmp_str_e, 0);
                            SMTCell::writeConstraint(fmt::format(
                                "(assert (ite (and (= N{}_M2_TRACK false)",
                                en.first));
                          }

                          SMTCell::writeConstraint(
                              fmt::format(" (= {} true)", tmp_var_self[0]));

                          for (std::size_t i = 0; i < tmp_var_self_c.size();
                               i++) {
                            SMTCell::writeConstraint(fmt::format(
                                " (= {} false)", tmp_var_self_c[i]));
                            SMTCell::cnt("l", 3);
                          }

                          SMTCell::writeConstraint(") ((_ at-least 1)");

                          std::vector<std::string> tmp_var_com;
                          int cnt_var_com = 0;
                          int cnt_true_com = 0;

                          Triplet *vCoord_k =
                              new Triplet(metal, row_j + 1, col);

                          if (SMTCell::ifVertex((*vCoord_k)) &&
                              SMTCell::getVertex((*vCoord_k))
                                  ->getBackADJ()
                                  ->ifValid()) {
                            for (int commodityIndex = 0;
                                 commodityIndex <
                                 SMTCell::getNet(netIndex)->getNumSinks();
                                 commodityIndex++) {
                              tmp_str = fmt::format(
                                  "N{}_C{}_E_{}_{}",
                                  SMTCell::getNet(netIndex)->getNetID(),
                                  commodityIndex, vCoord_k->getVName(),
                                  SMTCell::getVertex((*vCoord_k))
                                      ->getBackADJ()
                                      ->getVName());
                              if (!SMTCell::ifAssigned(tmp_str)) {
                                tmp_var_com.push_back(tmp_str);
                                SMTCell::setVar(tmp_str, 2);
                                cnt_var_com++;
                              } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                                SMTCell::setVar_wo_cnt(tmp_str, 2);
                                cnt_true_com++;
                              }
                            }
                          }

                          if (SMTCell::ifVertex((*vCoord_k)) &&
                              SMTCell::getVertex((*vCoord_k))
                                  ->getDownADJ()
                                  ->ifValid()) {
                            for (int commodityIndex = 0;
                                 commodityIndex <
                                 SMTCell::getNet(netIndex)->getNumSinks();
                                 commodityIndex++) {
                              tmp_str = fmt::format(
                                  "N{}_C{}_E_{}_{}",
                                  SMTCell::getNet(netIndex)->getNetID(),
                                  commodityIndex,
                                  SMTCell::getVertex((*vCoord_k))
                                      ->getDownADJ()
                                      ->getVName(),
                                  vCoord_k->getVName());
                              if (!SMTCell::ifAssigned(tmp_str)) {
                                tmp_var_com.push_back(tmp_str);
                                SMTCell::setVar(tmp_str, 2);
                                cnt_var_com++;
                              } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                                SMTCell::setVar_wo_cnt(tmp_str, 2);
                                cnt_true_com++;
                              }
                            }
                          }

                          if (SMTCell::ifVertex((*vCoord_k)) &&
                              SMTCell::getVertex((*vCoord_k))
                                  ->getUpADJ()
                                  ->ifValid()) {
                            for (int commodityIndex = 0;
                                 commodityIndex <
                                 SMTCell::getNet(netIndex)->getNumSinks();
                                 commodityIndex++) {
                              tmp_str = fmt::format(
                                  "N{}_C{}_E_{}_{}",
                                  SMTCell::getNet(netIndex)->getNetID(),
                                  commodityIndex, vCoord_k->getVName(),
                                  SMTCell::getVertex((*vCoord_k))
                                      ->getUpADJ()
                                      ->getVName());
                              if (!SMTCell::ifAssigned(tmp_str)) {
                                tmp_var_com.push_back(tmp_str);
                                SMTCell::setVar(tmp_str, 2);
                                cnt_var_com++;
                              } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                                SMTCell::setVar_wo_cnt(tmp_str, 2);
                                cnt_true_com++;
                              }
                            }
                          }

                          if (cnt_true_com == 0) {
                            if (cnt_var_com == 1) {
                              for (std::size_t m = 0; m < tmp_var_com.size();
                                   m++) {
                                SMTCell::writeConstraint(
                                    fmt::format(" {}", tmp_var_com[m]));
                                SMTCell::cnt("l", 3);
                              }
                            } else if (cnt_var_com >= 1) {
                              SMTCell::writeConstraint(" (or");
                              for (std::size_t m = 0; m < tmp_var_com.size();
                                   m++) {
                                SMTCell::writeConstraint(
                                    fmt::format(" {}", tmp_var_com[m]));
                                SMTCell::cnt("l", 3);
                              }
                              SMTCell::writeConstraint(")");
                            }
                          }

                          for (int row_k = row_j + 1;
                               row_k <= SMTCell::getNumTrackH() - 3; row_k++) {
                            std::vector<std::string> tmp_var_net;
                            tmp_var_com.clear();
                            int cnt_var_net = 0;
                            cnt_var_com = 0;
                            int cnt_true_net = 0;
                            cnt_true_com = 0;
                            for (int j = 0; j <= row_k - row_j - 1; j++) {
                              Triplet *vCoord_j =
                                  new Triplet(metal, (row_j + 1 + j), col);
                              if (SMTCell::ifVertex((*vCoord_j)) &&
                                  SMTCell::getVertex((*vCoord_j))
                                      ->getBackADJ()
                                      ->ifValid()) {
                                tmp_str = fmt::format(
                                    "N{}_E_{}_{}",
                                    SMTCell::getNet(netIndex)->getNetID(),
                                    vCoord_j->getVName(),
                                    SMTCell::getVertex((*vCoord_j))
                                        ->getBackADJ()
                                        ->getVName());
                                if (!SMTCell::ifAssigned(tmp_str)) {
                                  tmp_var_net.push_back(tmp_str);
                                  SMTCell::setVar(tmp_str, 2);
                                  cnt_var_net++;
                                } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                                  SMTCell::setVar_wo_cnt(tmp_str, 2);
                                  cnt_true_net++;
                                }
                              }
                              delete vCoord_j;
                            }
                            delete vCoord_k;

                            vCoord_k = new Triplet(metal, row_k + 1, col);

                            if (SMTCell::ifVertex((*vCoord_k)) &&
                                SMTCell::getVertex((*vCoord_k))
                                    ->getBackADJ()
                                    ->ifValid()) {
                              for (int commodityIndex = 0;
                                   commodityIndex <
                                   SMTCell::getNet(netIndex)->getNumSinks();
                                   commodityIndex++) {
                                tmp_str = fmt::format(
                                    "N{}_C{}_E_{}_{}",
                                    SMTCell::getNet(netIndex)->getNetID(),
                                    commodityIndex, vCoord_k->getVName(),
                                    SMTCell::getVertex((*vCoord_k))
                                        ->getBackADJ()
                                        ->getVName());
                                if (!SMTCell::ifAssigned(tmp_str)) {
                                  tmp_var_com.push_back(tmp_str);
                                  SMTCell::setVar(tmp_str, 2);
                                  cnt_var_com++;
                                } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                                  SMTCell::setVar_wo_cnt(tmp_str, 2);
                                  cnt_true_com++;
                                }
                              }
                            }

                            if (SMTCell::ifVertex((*vCoord_k)) &&
                                SMTCell::getVertex((*vCoord_k))
                                    ->getDownADJ()
                                    ->ifValid()) {
                              for (int commodityIndex = 0;
                                   commodityIndex <
                                   SMTCell::getNet(netIndex)->getNumSinks();
                                   commodityIndex++) {
                                tmp_str = fmt::format(
                                    "N{}_C{}_E_{}_{}",
                                    SMTCell::getNet(netIndex)->getNetID(),
                                    commodityIndex,
                                    SMTCell::getVertex((*vCoord_k))
                                        ->getDownADJ()
                                        ->getVName(),
                                    vCoord_k->getVName());
                                if (!SMTCell::ifAssigned(tmp_str)) {
                                  tmp_var_com.push_back(tmp_str);
                                  SMTCell::setVar(tmp_str, 2);
                                  cnt_var_com++;
                                } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                                  SMTCell::setVar_wo_cnt(tmp_str, 2);
                                  cnt_true_com++;
                                }
                              }
                            }

                            if (SMTCell::ifVertex((*vCoord_k)) &&
                                SMTCell::getVertex((*vCoord_k))
                                    ->getUpADJ()
                                    ->ifValid()) {
                              for (int commodityIndex = 0;
                                   commodityIndex <
                                   SMTCell::getNet(netIndex)->getNumSinks();
                                   commodityIndex++) {
                                tmp_str = fmt::format(
                                    "N{}_C{}_E_{}_{}",
                                    SMTCell::getNet(netIndex)->getNetID(),
                                    commodityIndex, vCoord_k->getVName(),
                                    SMTCell::getVertex((*vCoord_k))
                                        ->getUpADJ()
                                        ->getVName());
                                if (!SMTCell::ifAssigned(tmp_str)) {
                                  tmp_var_com.push_back(tmp_str);
                                  SMTCell::setVar(tmp_str, 2);
                                  cnt_var_com++;
                                } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                                  SMTCell::setVar_wo_cnt(tmp_str, 2);
                                  cnt_true_com++;
                                }
                              }
                            }

                            if (cnt_true_com == 0) {
                              if (cnt_var_com == 1) {
                                for (std::size_t m = 0; m < tmp_var_com.size();
                                     m++) {
                                  SMTCell::writeConstraint(
                                      fmt::format(" (and {}", tmp_var_com[m]));
                                  SMTCell::cnt("l", 3);
                                }
                                for (std::size_t m = 0; m < tmp_var_net.size();
                                     m++) {
                                  SMTCell::writeConstraint(
                                      fmt::format(" {}", tmp_var_net[m]));
                                  SMTCell::cnt("l", 3);
                                }
                                SMTCell::writeConstraint(")");
                              } else if (cnt_var_com >= 1) {
                                SMTCell::writeConstraint(" (and (or");
                                for (std::size_t m = 0; m < tmp_var_com.size();
                                     m++) {
                                  SMTCell::writeConstraint(
                                      fmt::format(" {}", tmp_var_com[m]));
                                  SMTCell::cnt("l", 3);
                                }
                                SMTCell::writeConstraint(")");
                                for (std::size_t m = 0; m < tmp_var_net.size();
                                     m++) {
                                  SMTCell::writeConstraint(
                                      fmt::format(" {}", tmp_var_net[m]));
                                  SMTCell::cnt("l", 3);
                                }
                                SMTCell::writeConstraint(")");
                              }
                            }
                          }
                          delete vCoord_k;

                          tmp_var_com.clear();
                          cnt_var_com = 0;
                          cnt_true_com = 0;

                          vCoord_k = new Triplet(metal, row_j, col);

                          if (SMTCell::ifVertex((*vCoord_k)) &&
                              SMTCell::getVertex((*vCoord_k))
                                  ->getFrontADJ()
                                  ->ifValid()) {
                            for (int commodityIndex = 0;
                                 commodityIndex <
                                 SMTCell::getNet(netIndex)->getNumSinks();
                                 commodityIndex++) {
                              tmp_str = fmt::format(
                                  "N{}_C{}_E_{}_{}",
                                  SMTCell::getNet(netIndex)->getNetID(),
                                  commodityIndex,
                                  SMTCell::getVertex((*vCoord_k))
                                      ->getFrontADJ()
                                      ->getVName(),
                                  vCoord_k->getVName());
                              if (!SMTCell::ifAssigned(tmp_str)) {
                                tmp_var_com.push_back(tmp_str);
                                SMTCell::setVar(tmp_str, 2);
                                cnt_var_com++;
                              } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                                SMTCell::setVar_wo_cnt(tmp_str, 2);
                                cnt_true_com++;
                              }
                            }
                          }

                          if (SMTCell::ifVertex((*vCoord_k)) &&
                              SMTCell::getVertex((*vCoord_k))
                                  ->getDownADJ()
                                  ->ifValid()) {
                            for (int commodityIndex = 0;
                                 commodityIndex <
                                 SMTCell::getNet(netIndex)->getNumSinks();
                                 commodityIndex++) {
                              tmp_str = fmt::format(
                                  "N{}_C{}_E_{}_{}",
                                  SMTCell::getNet(netIndex)->getNetID(),
                                  commodityIndex,
                                  SMTCell::getVertex((*vCoord_k))
                                      ->getDownADJ()
                                      ->getVName(),
                                  vCoord_k->getVName());
                              if (!SMTCell::ifAssigned(tmp_str)) {
                                tmp_var_com.push_back(tmp_str);
                                SMTCell::setVar(tmp_str, 2);
                                cnt_var_com++;
                              } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                                SMTCell::setVar_wo_cnt(tmp_str, 2);
                                cnt_true_com++;
                              }
                            }
                          }

                          if (SMTCell::ifVertex((*vCoord_k)) &&
                              SMTCell::getVertex((*vCoord_k))
                                  ->getUpADJ()
                                  ->ifValid()) {
                            for (int commodityIndex = 0;
                                 commodityIndex <
                                 SMTCell::getNet(netIndex)->getNumSinks();
                                 commodityIndex++) {
                              tmp_str = fmt::format(
                                  "N{}_C{}_E_{}_{}",
                                  SMTCell::getNet(netIndex)->getNetID(),
                                  commodityIndex, vCoord_k->getVName(),
                                  SMTCell::getVertex((*vCoord_k))
                                      ->getUpADJ()
                                      ->getVName());
                              if (!SMTCell::ifAssigned(tmp_str)) {
                                tmp_var_com.push_back(tmp_str);
                                SMTCell::setVar(tmp_str, 2);
                                cnt_var_com++;
                              } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                                SMTCell::setVar_wo_cnt(tmp_str, 2);
                                cnt_true_com++;
                              }
                            }
                          }

                          if (cnt_true_com == 0) {
                            if (cnt_var_com == 1) {
                              for (std::size_t m = 0; m < tmp_var_com.size();
                                   m++) {
                                SMTCell::writeConstraint(
                                    fmt::format(" {}", tmp_var_com[m]));
                                SMTCell::cnt("l", 3);
                              }
                            } else if (cnt_var_com >= 1) {
                              SMTCell::writeConstraint(" (or");
                              for (std::size_t m = 0; m < tmp_var_com.size();
                                   m++) {
                                SMTCell::writeConstraint(
                                    fmt::format(" {}", tmp_var_com[m]));
                                SMTCell::cnt("l", 3);
                              }
                              SMTCell::writeConstraint(")");
                            }
                          }

                          delete vCoord_k;

                          for (int row_k = 0; row_k <= row_j - 1; row_k++) {
                            std::vector<std::string> tmp_var_net;
                            tmp_var_com.clear();
                            int cnt_var_net = 0;
                            cnt_var_com = 0;
                            int cnt_true_net = 0;
                            cnt_true_com = 0;
                            for (int j = 0; j <= row_k; j++) {
                              Triplet *vCoord_j =
                                  new Triplet(metal, (row_j - j), col);
                              if (SMTCell::ifVertex((*vCoord_j)) &&
                                  SMTCell::getVertex((*vCoord_j))
                                      ->getFrontADJ()
                                      ->ifValid()) {
                                tmp_str = fmt::format(
                                    "N{}_E_{}_{}",
                                    SMTCell::getNet(netIndex)->getNetID(),
                                    SMTCell::getVertex((*vCoord_j))
                                        ->getFrontADJ()
                                        ->getVName(),
                                    vCoord_j->getVName());
                                if (!SMTCell::ifAssigned(tmp_str)) {
                                  tmp_var_net.push_back(tmp_str);
                                  SMTCell::setVar(tmp_str, 2);
                                  cnt_var_net++;
                                } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                                  SMTCell::setVar_wo_cnt(tmp_str, 2);
                                  cnt_true_net++;
                                }
                              }
                              delete vCoord_j;
                            }

                            vCoord_k =
                                new Triplet(metal, row_j - row_k - 1, col);

                            if (SMTCell::ifVertex((*vCoord_k)) &&
                                SMTCell::getVertex((*vCoord_k))
                                    ->getFrontADJ()
                                    ->ifValid()) {
                              for (int commodityIndex = 0;
                                   commodityIndex <
                                   SMTCell::getNet(netIndex)->getNumSinks();
                                   commodityIndex++) {
                                tmp_str = fmt::format(
                                    "N{}_C{}_E_{}_{}",
                                    SMTCell::getNet(netIndex)->getNetID(),
                                    commodityIndex,
                                    SMTCell::getVertex((*vCoord_k))
                                        ->getFrontADJ()
                                        ->getVName(),
                                    vCoord_k->getVName());
                                if (!SMTCell::ifAssigned(tmp_str)) {
                                  tmp_var_com.push_back(tmp_str);
                                  SMTCell::setVar(tmp_str, 2);
                                  cnt_var_com++;
                                } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                                  SMTCell::setVar_wo_cnt(tmp_str, 2);
                                  cnt_true_com++;
                                }
                              }
                            }

                            if (SMTCell::ifVertex((*vCoord_k)) &&
                                SMTCell::getVertex((*vCoord_k))
                                    ->getDownADJ()
                                    ->ifValid()) {
                              for (int commodityIndex = 0;
                                   commodityIndex <
                                   SMTCell::getNet(netIndex)->getNumSinks();
                                   commodityIndex++) {
                                tmp_str = fmt::format(
                                    "N{}_C{}_E_{}_{}",
                                    SMTCell::getNet(netIndex)->getNetID(),
                                    commodityIndex,
                                    SMTCell::getVertex((*vCoord_k))
                                        ->getDownADJ()
                                        ->getVName(),
                                    vCoord_k->getVName());
                                if (!SMTCell::ifAssigned(tmp_str)) {
                                  tmp_var_com.push_back(tmp_str);
                                  SMTCell::setVar(tmp_str, 2);
                                  cnt_var_com++;
                                } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                                  SMTCell::setVar_wo_cnt(tmp_str, 2);
                                  cnt_true_com++;
                                }
                              }
                            }

                            if (SMTCell::ifVertex((*vCoord_k)) &&
                                SMTCell::getVertex((*vCoord_k))
                                    ->getUpADJ()
                                    ->ifValid()) {
                              for (int commodityIndex = 0;
                                   commodityIndex <
                                   SMTCell::getNet(netIndex)->getNumSinks();
                                   commodityIndex++) {
                                tmp_str = fmt::format(
                                    "N{}_C{}_E_{}_{}",
                                    SMTCell::getNet(netIndex)->getNetID(),
                                    commodityIndex, vCoord_k->getVName(),
                                    SMTCell::getVertex((*vCoord_k))
                                        ->getUpADJ()
                                        ->getVName());
                                if (!SMTCell::ifAssigned(tmp_str)) {
                                  tmp_var_com.push_back(tmp_str);
                                  SMTCell::setVar(tmp_str, 2);
                                  cnt_var_com++;
                                } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                                  SMTCell::setVar_wo_cnt(tmp_str, 2);
                                  cnt_true_com++;
                                }
                              }
                            }

                            if (cnt_true_com == 0) {
                              if (cnt_var_com == 1) {
                                for (std::size_t m = 0; m < tmp_var_com.size();
                                     m++) {
                                  SMTCell::writeConstraint(
                                      fmt::format(" (and {}", tmp_var_com[m]));
                                  SMTCell::cnt("l", 3);
                                }
                                for (std::size_t m = 0; m < tmp_var_net.size();
                                     m++) {
                                  SMTCell::writeConstraint(
                                      fmt::format(" {}", tmp_var_net[m]));
                                  SMTCell::cnt("l", 3);
                                }
                                SMTCell::writeConstraint(")");
                              } else if (cnt_var_com >= 1) {
                                SMTCell::writeConstraint(" (and (or");
                                for (std::size_t m = 0; m < tmp_var_com.size();
                                     m++) {
                                  SMTCell::writeConstraint(
                                      fmt::format(" {}", tmp_var_com[m]));
                                  SMTCell::cnt("l", 3);
                                }
                                SMTCell::writeConstraint(")");
                                for (std::size_t m = 0; m < tmp_var_net.size();
                                     m++) {
                                  SMTCell::writeConstraint(
                                      fmt::format(" {}", tmp_var_net[m]));
                                  SMTCell::cnt("l", 3);
                                }
                                SMTCell::writeConstraint(")");
                              }
                            }
                            delete vCoord_k;
                          }
                          SMTCell::writeConstraint(") (= true true)))\n");
                          SMTCell::cnt("c", 3);
                        }
                        delete vCoord_j;
                      }
                    }
                  }
                }
              }
            }
            delete vCoord_i;
          }
        }
      }
    }
  }
  fmt::print(" has been written\n");
}

void FlowWriter::disable_cell_boundary() {
  fmt::print("a     EXP-1. Writing cell boundary constraint\n");
  SMTCell::writeConstraint("; EXP-1. Disable any use of vertices at cell "
                           "boundaries for better cell abutment\n");
  // for (int metal = 1; metal <= SMTCell::getNumMetalLayer(); metal++) {
  // disable M1 via acess to M2
  int metal = 1;
  for (int row_idx = 0; row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3;
       row_idx++) {
    // retrieve row
    int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
    // retrieve col
    int leftmost_col = SMTCell::get_h_metal_col_by_idx(metal, 0);
    int rightmost_col = SMTCell::get_h_metal_col_by_idx(
        metal, SMTCell::get_h_metal_numTrackV(metal) - 1);

    // set the rightmost and leftmost via access to be false
    std::string left_via_variable =
        fmt::format("M_m{}r{}c{}_m{}r{}c{}", metal, row, leftmost_col,
                    metal + 1, row, leftmost_col);
    SMTCell::writeConstraint(
        fmt::format("(assert (= {} false))\n", left_via_variable));

    std::string right_via_variable =
        fmt::format("M_m{}r{}c{}_m{}r{}c{}", metal, row, rightmost_col,
                    metal + 1, row, rightmost_col);
    SMTCell::writeConstraint(
        fmt::format("(assert (= {} false))\n", right_via_variable));
  }
  // }

  // disable Geometric Variable at M2 boundary
  metal = 2;
  for (int row_idx = 0; row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3;
       row_idx++) {
    // retrieve row
    int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
    // retrieve col
    int leftmost_col = SMTCell::get_h_metal_col_by_idx(metal, 0);
    int rightmost_col = SMTCell::get_h_metal_col_by_idx(
        metal, SMTCell::get_h_metal_numTrackV(metal) - 1);

    // set the rightmost and leftmost Geometric Variable to be false
    std::string left_gv_variable =
        fmt::format("GL_V_m{}r{}c{}", metal, row, leftmost_col);
    SMTCell::writeConstraint(
        fmt::format("(assert (= {} false))\n", left_gv_variable));

    std::string right_gv_variable =
        fmt::format("GR_V_m{}r{}c{}", metal, row, rightmost_col);
    SMTCell::writeConstraint(
        fmt::format("(assert (= {} false))\n", right_gv_variable));
  }
}

void FlowWriter::minimize_wire_segments(FILE *fp) {
  fmt::print("a     EXP-2. Minimizing wire segments\n");
  // SMTCell::writeConstraint("; EXP-2. Minimizing wire segments\n");
  // SMTCell::writeConstraint("(minimize (bvadd \n");
  fmt::print(fp, "; EXP-2. Minimizing wire segments\n");
  fmt::print(fp, "(minimize (bvadd \n");
  int gv_cost = 10; // fixed cost for each geometric variable
  int metal = SMTCell::getFirstRoutingLayer();

  // for (int metal = SMTCell::getFirstRoutingLayer();
  //      metal <= SMTCell::getNumMetalLayer(); metal++) {

  for (int row_idx = 0; row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3;
       row_idx++) {
    for (int col_idx = 0; col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1;
         col_idx++) {
      // retrieve row/col
      int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
      int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);

      std::string tmp_gr_v =
          fmt::format("GR_V_{}", SMTCell::getVertexName(metal, row, col));
      std::string tmp_gl_v =
          fmt::format("GL_V_{}", SMTCell::getVertexName(metal, row, col));

      // if (SMTCell::ifAssigned(tmp_gr_v)) {
      // SMTCell::writeConstraint(fmt::format("(ite (= {} true) (_ bv{} "
      //                                      "32) (_ bv0 32))\n",
      //                                      tmp_gr_v, gv_cost));
      fmt::print(fp, "  (ite (= {} true) (_ bv{} 32) (_ bv0 32))\n", tmp_gr_v,
                 gv_cost);
      // }

      // if (SMTCell::ifAssigned(tmp_gl_v)) {
      // SMTCell::writeConstraint(fmt::format("(ite (= {} true) (_ bv{} "
      //                                      "32) (_ bv0 32))\n",
      //                                      tmp_gl_v, gv_cost));
      fmt::print(fp, "  (ite (= {} true) (_ bv{} 32) (_ bv0 32))\n", tmp_gl_v,
                 gv_cost);
      // }
    }
  }
  fmt::print(fp, "))\n");
}