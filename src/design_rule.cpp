#include "design_rule.hpp"
#include "SMTCell.hpp"

namespace bmp = boost::multiprecision;

// GV
void DesignRuleWriter::write_geometric_variables_AGR(FILE *drlog) {
  SMTCell::writeConstraint(";6. Geometry variables for Left (GL), Right "
                           "(GR), Front (GF), and Back (GB) directions\n");
  fmt::print(drlog, ";6. Geometry variables for Left (GL), Right (GR), Front "
                    "(GF), and Back (GB) directions\n");
  // ### DATA STRUCTURE:  VERTEX [index] [name] [Z-pos] [Y-pos] [X-pos]
  // [Arr. of adjacent vertices]
  // ### DATA STRUCTURE:  ADJACENT_VERTICES [0:Left] [1:Right] [2:Front]
  // [3:Back] [4:Up] [5:Down] [6:FL] [7:FR] [8:BL] [9:BR]
  SMTCell::writeConstraint(
      ";6-A. Geometry variables for Left-tip on each vertex\n");
  fmt::print(drlog, ";6-A. Geometry variables for Left-tip on each vertex\n");
  DesignRuleWriter::write_geometric_variables_AGR_L_tip_helper(drlog);
  SMTCell::writeConstraint("\n");

  // ### DATA STRUCTURE:  VERTEX [index] [name] [Z-pos] [Y-pos] [X-pos]
  // [Arr. of adjacent vertices]
  // ### DATA STRUCTURE:  ADJACENT_VERTICES [0:Left] [1:Right] [2:Front]
  // [3:Back] [4:Up] [5:Down] [6:FL] [7:FR] [8:BL] [9:BR]
  SMTCell::writeConstraint(
      ";6-B. Geometry variables for Right-tip on each vertex\n");
  fmt::print(drlog, ";6-B. Geometry variables for Right-tip on each vertex\n");
  DesignRuleWriter::write_geometric_variables_AGR_R_tip_helper(drlog);
  SMTCell::writeConstraint("\n");

  // ### DATA STRUCTURE:  VERTEX [index] [name] [Z-pos] [Y-pos] [X-pos]
  // [Arr. of adjacent vertices]
  // ### DATA STRUCTURE:  ADJACENT_VERTICES [0:Left] [1:Right] [2:Front]
  // [3:Back] [4:Up] [5:Down] [6:FL] [7:FR] [8:BL] [9:BR]
  SMTCell::writeConstraint(
      ";6-C. Geometry variables for Front-tip on each vertex\n");
  fmt::print(drlog, ";6-C. Geometry variables for Front-tip on each vertex\n");
  // # At the top-most metal layer, only vias exist.
  DesignRuleWriter::write_geometric_variables_AGR_F_tip_helper(drlog);
  SMTCell::writeConstraint("\n");

  // ### DATA STRUCTURE:  VERTEX [index] [name] [Z-pos] [Y-pos] [X-pos]
  // [Arr. of adjacent vertices]
  // ### DATA STRUCTURE:  ADJACENT_VERTICES [0:Left] [1:Right] [2:Front]
  // [3:Back] [4:Up] [5:Down] [6:FL] [7:FR] [8:BL] [9:BR]
  SMTCell::writeConstraint(
      ";6-D. Geometry variables for Back-tip on each vertex\n");
  fmt::print(drlog, ";6-D. Geometry variables for Back-tip on each vertex\n");
  DesignRuleWriter::write_geometric_variables_AGR_B_tip_helper(drlog);
  SMTCell::writeConstraint("\n");
  std::cout << "have been written.\n";
}

void DesignRuleWriter::write_geometric_variables_AGR_L_tip_helper(FILE *drlog) {
  // for (int metal = SMTCell::getFirstRoutingLayer();
  //      metal <= SMTCell::getNumMetalLayer(); metal++) {
  //   if (SMTCell::ifHorzMetal(metal)) { // M2, M4
  //     for (int row = 0; row <= SMTCell::getLastRow(); row++) {
  //       for (int col = 0; col <= SMTCell::getLastColIdx(); col++) {
  for (int metal = SMTCell::getFirstRoutingLayer();
       metal <= SMTCell::getNumMetalLayer(); metal++) {
    if (SMTCell::ifHorzMetal(metal)) { // M2, M4
      for (int row_idx = 0;
           row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3; row_idx++) {
        for (int col_idx = 0;
             col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
          // retrieve row/col
          int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
          int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);
          // if (metal > 1 && SMTCell::ifVertMetal(metal) &&
          //     SMTCell::ifSDCol(col)) {
          //   continue;
          // }

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d     GV_L: METAL {} ROW {} COL {} \n", metal,
                       row, col);
          }

          std::string tmp_g =
              fmt::format("GL_V_{}", SMTCell::getVertexName(metal, row, col));
          std::string tmp_m1 = "";

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        tmp_g: {} \n", tmp_g);
          }

          if (SMTCell::getVertex(metal, row, col)->getLeftADJ()->ifValid()) {
            tmp_m1 = fmt::format(
                "M_{}_{}",
                SMTCell::getVertex(metal, row, col)->getLeftADJ()->getVName(),
                SMTCell::getVertexName(metal, row, col));
          } else {
            tmp_m1 = fmt::format("M_LeftEnd_{}",
                                 SMTCell::getVertexName(metal, row, col));
          }

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        tmp_m1: {} \n", tmp_m1);
          }

          if (!SMTCell::ifAssigned(tmp_g) && !SMTCell::ifAssigned(tmp_m1)) {
            SMTCell::setVar(tmp_g, 6);
            SMTCell::setVar(tmp_m1, 2);
            SMTCell::writeConstraint(
                fmt::format("(assert ((_ at-most 1) {} {}))\n", tmp_g, tmp_m1));
            SMTCell::cnt("l", 3);
            SMTCell::cnt("l", 3);
            SMTCell::cnt("c", 3);
          } else if (SMTCell::ifAssigned(tmp_m1) &&
                     SMTCell::getAssigned(tmp_m1) == 1) {
            if (!SMTCell::ifAssigned(tmp_g)) {
              SMTCell::assignTrueVar(tmp_g, 0, false);
            }
          } else if (SMTCell::ifAssigned(tmp_g) &&
                     SMTCell::getAssigned(tmp_g) == 1) {
            if (!SMTCell::ifAssigned(tmp_m1)) {
              SMTCell::assignTrueVar(tmp_m1, 0, false);
            }
          }
          std::string tmp_m2 = "";
          if (SMTCell::getVertex(metal, row, col)->getRightADJ()->ifValid()) {
            tmp_m2 = fmt::format(
                "M_{}_{}", SMTCell::getVertexName(metal, row, col),
                SMTCell::getVertex(metal, row, col)->getRightADJ()->getVName());
          } else {
            tmp_m2 = fmt::format("M_{}_RightEnd",
                                 SMTCell::getVertexName(metal, row, col));
          }

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        tmp_m2: {} \n", tmp_m2);
          }

          if (!SMTCell::ifAssigned(tmp_g) && !SMTCell::ifAssigned(tmp_m2)) {
            SMTCell::setVar(tmp_g, 6);
            SMTCell::setVar(tmp_m2, 2);
            SMTCell::writeConstraint(fmt::format(
                "(assert (ite (= {} true) (= {} true) (= true true)))\n", tmp_g,
                tmp_m2));
            SMTCell::cnt("l", 3);
            SMTCell::cnt("l", 3);
            SMTCell::cnt("c", 3);
          } else if (SMTCell::ifAssigned(tmp_g) &&
                     SMTCell::getAssigned(tmp_g) == 1) {
            if (!SMTCell::ifAssigned(tmp_m2)) {
              SMTCell::assignTrueVar(tmp_m2, 1, false);
              SMTCell::setVar_wo_cnt(tmp_m2, 1);
            }
          }
          if (!SMTCell::ifAssigned(tmp_g) && !SMTCell::ifAssigned(tmp_m1)) {
            if (!SMTCell::ifAssigned(tmp_m2)) {
              SMTCell::setVar(tmp_g, 6);
              SMTCell::setVar(tmp_m1, 2);
              SMTCell::setVar(tmp_m2, 2);
              SMTCell::writeConstraint(
                  fmt::format("(assert (ite (= (or {} {}) false) (= {} "
                              "false) (= true true)))\n",
                              tmp_g, tmp_m1, tmp_m2));
              SMTCell::cnt("l", 3);
              SMTCell::cnt("l", 3);
              SMTCell::cnt("l", 3);
              SMTCell::cnt("c", 3);
            }
          } else if (SMTCell::ifAssigned(tmp_g) &&
                     SMTCell::getAssigned(tmp_g) == 0) {
            if (SMTCell::ifAssigned(tmp_m1) &&
                SMTCell::getAssigned(tmp_m1) == 0) {
              if (!SMTCell::ifAssigned(tmp_m2)) {
                SMTCell::assignTrueVar(tmp_m2, 1, false);
                SMTCell::setVar_wo_cnt(tmp_m2, 1);
              }
            } else if (!SMTCell::ifAssigned(tmp_m1)) {
              if (!SMTCell::ifAssigned(tmp_m2)) {
                SMTCell::setVar(tmp_m1, 2);
                SMTCell::setVar(tmp_m2, 2);
                SMTCell::writeConstraint(
                    fmt::format("(assert (ite (= {} false) (= {} false) (= "
                                "true true)))\n",
                                tmp_m1, tmp_m2));
                SMTCell::cnt("l", 3);
                SMTCell::cnt("l", 3);
                SMTCell::cnt("c", 3);
              }
            }
          } else if (SMTCell::ifAssigned(tmp_m1) &&
                     SMTCell::getAssigned(tmp_m1) == 0) {
            if (SMTCell::ifAssigned(tmp_g) &&
                SMTCell::getAssigned(tmp_g) == 0) {
              if (!SMTCell::ifAssigned(tmp_g)) {
                SMTCell::assignTrueVar(tmp_g, 1, false);
                SMTCell::setVar_wo_cnt(tmp_g, 1);
              }
            } else if (!SMTCell::ifAssigned(tmp_g)) {
              if (!SMTCell::ifAssigned(tmp_m2)) {
                SMTCell::setVar(tmp_g, 2);
                SMTCell::setVar(tmp_m2, 2);
                SMTCell::writeConstraint(
                    fmt::format("(assert (ite (= {} false) (= {} false) (= "
                                "true true)))\n",
                                tmp_g, tmp_m2));
                SMTCell::cnt("l", 3);
                SMTCell::cnt("l", 3);
                SMTCell::cnt("c", 3);
              }
            }
          }
        }
      }
    }
  }
}

void DesignRuleWriter::write_geometric_variables_AGR_R_tip_helper(FILE *drlog) {
  // for (int metal = SMTCell::getFirstRoutingLayer();
  //      metal <= SMTCell::getNumMetalLayer(); metal++) {
  //   if (SMTCell::ifHorzMetal(metal)) {
  //     for (int row = 0; row <= SMTCell::getLastRow(); row++) {
  //       for (int col = 0; col <= SMTCell::getLastColIdx(); col++) {
  for (int metal = SMTCell::getFirstRoutingLayer();
       metal <= SMTCell::getNumMetalLayer(); metal++) {
    if (SMTCell::ifHorzMetal(metal)) {
      for (int row_idx = 0;
           row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3; row_idx++) {
        for (int col_idx = 0;
             col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
          // retrieve row/col
          int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
          int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);
          // if (metal > 1 && SMTCell::ifVertMetal(metal) &&
          //     SMTCell::ifSDCol(col)) {
          //   continue;
          // }

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d     GV_R: METAL {} ROW {} COL {} \n", metal,
                       row, col);
          }

          std::string tmp_g =
              fmt::format("GR_V_{}", SMTCell::getVertexName(metal, row, col));

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        tmp_g: {} \n", tmp_g);
          }

          std::string tmp_m1 = "";
          if (SMTCell::getVertex(metal, row, col)->getLeftADJ()->ifValid()) {
            tmp_m1 = fmt::format(
                "M_{}_{}",
                SMTCell::getVertex(metal, row, col)->getLeftADJ()->getVName(),
                SMTCell::getVertexName(metal, row, col));
          } else {
            tmp_m1 = fmt::format("M_LeftEnd_{}",
                                 SMTCell::getVertexName(metal, row, col));
          }

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        tmp_m1: {} \n", tmp_m1);
          }

          if (!SMTCell::ifAssigned(tmp_g) && !SMTCell::ifAssigned(tmp_m1)) {
            SMTCell::setVar(tmp_g, 6);
            SMTCell::setVar(tmp_m1, 2);
            SMTCell::writeConstraint(fmt::format(
                "(assert (ite (= {} true) (= {} true) (= true true)))\n", tmp_g,
                tmp_m1));
            SMTCell::cnt("l", 3);
            SMTCell::cnt("l", 3);
            SMTCell::cnt("c", 3);
          } else if (SMTCell::ifAssigned(tmp_g) &&
                     SMTCell::getAssigned(tmp_g) == 1) {
            if (!SMTCell::ifAssigned(tmp_m1)) {
              SMTCell::assignTrueVar(tmp_m1, 1, false);
              SMTCell::setVar_wo_cnt(tmp_m1, 1);
            }
          }
          std::string tmp_m2 = "";
          if (SMTCell::getVertex(metal, row, col)->getRightADJ()->ifValid()) {
            tmp_m2 = fmt::format(
                "M_{}_{}", SMTCell::getVertexName(metal, row, col),
                SMTCell::getVertex(metal, row, col)->getRightADJ()->getVName());
          } else {
            tmp_m2 = fmt::format("M_{}_RightEnd",
                                 SMTCell::getVertexName(metal, row, col));
          }

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        tmp_m2: {} \n", tmp_m2);
          }

          if (!SMTCell::ifAssigned(tmp_g) && !SMTCell::ifAssigned(tmp_m2)) {
            SMTCell::setVar(tmp_g, 6);
            SMTCell::setVar(tmp_m2, 2);
            SMTCell::writeConstraint(
                fmt::format("(assert ((_ at-most 1) {} {}))\n", tmp_g, tmp_m2));
            SMTCell::cnt("l", 3);
            SMTCell::cnt("l", 3);
            SMTCell::cnt("c", 3);
          } else if (SMTCell::ifAssigned(tmp_m2) &&
                     SMTCell::getAssigned(tmp_m2) == 1) {
            if (!SMTCell::ifAssigned(tmp_g)) {
              SMTCell::assignTrueVar(tmp_g, 0, false);
            }
          } else if (SMTCell::ifAssigned(tmp_g) &&
                     SMTCell::getAssigned(tmp_g) == 1) {
            if (!SMTCell::ifAssigned(tmp_m2)) {
              SMTCell::assignTrueVar(tmp_m2, 0, false);
            }
          }
          if (!SMTCell::ifAssigned(tmp_g) && !SMTCell::ifAssigned(tmp_m2)) {
            if (!SMTCell::ifAssigned(tmp_m1)) {
              SMTCell::setVar(tmp_g, 6);
              SMTCell::setVar(tmp_m1, 2);
              SMTCell::setVar(tmp_m2, 2);
              SMTCell::writeConstraint(
                  fmt::format("(assert (ite (= (or {} {}) false) (= {} "
                              "false) (= true true)))\n",
                              tmp_g, tmp_m2, tmp_m1));
              SMTCell::cnt("l", 3);
              SMTCell::cnt("l", 3);
              SMTCell::cnt("l", 3);
              SMTCell::cnt("c", 3);
            }
          } else if (SMTCell::ifAssigned(tmp_g) &&
                     SMTCell::getAssigned(tmp_g) == 0) {
            if (SMTCell::ifAssigned(tmp_m2) &&
                SMTCell::getAssigned(tmp_m2) == 0) {
              if (!SMTCell::ifAssigned(tmp_m1)) {
                SMTCell::assignTrueVar(tmp_m1, 1, false);
                SMTCell::setVar_wo_cnt(tmp_m1, 1);
              }
            } else if (!SMTCell::ifAssigned(tmp_m2)) {
              if (!SMTCell::ifAssigned(tmp_m1)) {
                SMTCell::setVar(tmp_m2, 2);
                SMTCell::setVar(tmp_m1, 2);
                SMTCell::writeConstraint(
                    fmt::format("(assert (ite (= {} false) (= {} false) (= "
                                "true true)))\n",
                                tmp_m2, tmp_m1));
                SMTCell::cnt("l", 3);
                SMTCell::cnt("l", 3);
                SMTCell::cnt("c", 3);
              }
            }
          } else if (SMTCell::ifAssigned(tmp_m2) &&
                     SMTCell::getAssigned(tmp_m2) == 0) {
            if (SMTCell::ifAssigned(tmp_g) &&
                SMTCell::getAssigned(tmp_g) == 0) {
              if (!SMTCell::ifAssigned(tmp_g)) {
                SMTCell::assignTrueVar(tmp_g, 1, false);
                SMTCell::setVar_wo_cnt(tmp_g, 1);
              }
            } else if (!SMTCell::ifAssigned(tmp_g)) {
              if (!SMTCell::ifAssigned(tmp_m1)) {
                SMTCell::setVar(tmp_g, 2);
                SMTCell::setVar(tmp_m1, 2);
                SMTCell::writeConstraint(
                    fmt::format("(assert (ite (= {} false) (= {} false) (= "
                                "true true)))\n",
                                tmp_g, tmp_m1));
                SMTCell::cnt("l", 3);
                SMTCell::cnt("l", 3);
                SMTCell::cnt("c", 3);
              }
            }
          }
        }
      }
    }
  }
}

void DesignRuleWriter::write_geometric_variables_AGR_F_tip_helper(FILE *drlog) {
  // for (int metal = SMTCell::getFirstRoutingLayer();
  //      metal <= SMTCell::getNumMetalLayer(); metal++) {
  //   if (SMTCell::ifVertMetal(metal)) {
  //     for (int row = 0; row <= SMTCell::getLastRow(); row++) {
  //       for (int col = 0; col <= SMTCell::getLastColIdx(); col++) {
  for (int metal = 1; metal <= SMTCell::getNumMetalLayer(); metal++) {
    if (SMTCell::ifVertMetal(metal)) {
      for (int row_idx = 0;
           row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3; row_idx++) {
        for (int col_idx = 0;
             col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
          // retrieve row/col
          int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
          int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);
          if (metal == 1 && SMTCell::ifSDCol_AGR(metal, col)) {
            continue;
          }
          // if (metal > 1 && SMTCell::ifVertMetal(metal) &&
          //     SMTCell::ifSDCol(col)) {
          //   continue;
          // }

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d     GV_F: METAL {} ROW {} COL {} \n", metal,
                       row, col);
          }

          std::string tmp_g =
              fmt::format("GF_V_{}", SMTCell::getVertexName(metal, row, col));

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        tmp_g: {} \n", tmp_g);
          }

          std::string tmp_m1 = "";
          if (SMTCell::getVertex(metal, row, col)->getFrontADJ()->ifValid()) {
            tmp_m1 = fmt::format(
                "M_{}_{}",
                SMTCell::getVertex(metal, row, col)->getFrontADJ()->getVName(),
                SMTCell::getVertexName(metal, row, col));
          } else {
            tmp_m1 = fmt::format("M_FrontEnd_{}",
                                 SMTCell::getVertexName(metal, row, col));
          }

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        tmp_m1: {} \n", tmp_m1);
          }

          if (!SMTCell::ifAssigned(tmp_g) && !SMTCell::ifAssigned(tmp_m1)) {
            SMTCell::setVar(tmp_g, 6);
            SMTCell::setVar(tmp_m1, 2);
            SMTCell::writeConstraint(
                fmt::format("(assert ((_ at-most 1) {} {}))\n", tmp_g, tmp_m1));
            SMTCell::cnt("l", 3);
            SMTCell::cnt("l", 3);
            SMTCell::cnt("c", 3);
          } else if (SMTCell::ifAssigned(tmp_m1) &&
                     SMTCell::getAssigned(tmp_m1) == 1) {
            if (!SMTCell::ifAssigned(tmp_g)) {
              SMTCell::assignTrueVar(tmp_g, 0, false);
            }
          } else if (SMTCell::ifAssigned(tmp_g) &&
                     SMTCell::getAssigned(tmp_g) == 1) {
            if (!SMTCell::ifAssigned(tmp_m1)) {
              SMTCell::assignTrueVar(tmp_m1, 0, false);
            }
          }
          std::string tmp_m2 = "";
          if (SMTCell::getVertex(metal, row, col)->getBackADJ()->ifValid()) {
            tmp_m2 = fmt::format(
                "M_{}_{}", SMTCell::getVertexName(metal, row, col),
                SMTCell::getVertex(metal, row, col)->getBackADJ()->getVName());
          } else {
            tmp_m2 = fmt::format("M_{}_BackEnd",
                                 SMTCell::getVertexName(metal, row, col));
          }

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        tmp_m2: {} \n", tmp_m2);
          }

          if (!SMTCell::ifAssigned(tmp_g) && !SMTCell::ifAssigned(tmp_m2)) {
            SMTCell::setVar(tmp_g, 6);
            SMTCell::setVar(tmp_m2, 2);
            SMTCell::writeConstraint(fmt::format(
                "(assert (ite (= {} true) (= {} true) (= true true)))\n", tmp_g,
                tmp_m2));
            SMTCell::cnt("l", 3);
            SMTCell::cnt("l", 3);
            SMTCell::cnt("c", 3);
          } else if (SMTCell::ifAssigned(tmp_g) &&
                     SMTCell::getAssigned(tmp_g) == 1) {
            if (!SMTCell::ifAssigned(tmp_m2)) {
              SMTCell::assignTrueVar(tmp_m2, 1, false);
              SMTCell::setVar_wo_cnt(tmp_m2, 1);
            }
          }
          if (!SMTCell::ifAssigned(tmp_g) && !SMTCell::ifAssigned(tmp_m1)) {
            if (!SMTCell::ifAssigned(tmp_m2)) {
              SMTCell::setVar(tmp_g, 6);
              SMTCell::setVar(tmp_m1, 2);
              SMTCell::setVar(tmp_m2, 2);
              SMTCell::writeConstraint(
                  fmt::format("(assert (ite (= (or {} {}) false) (= {} "
                              "false) (= true true)))\n",
                              tmp_g, tmp_m1, tmp_m2));
              SMTCell::cnt("l", 3);
              SMTCell::cnt("l", 3);
              SMTCell::cnt("l", 3);
              SMTCell::cnt("c", 3);
            }
          } else if (SMTCell::ifAssigned(tmp_g) &&
                     SMTCell::getAssigned(tmp_g) == 0) {
            if (SMTCell::ifAssigned(tmp_m1) &&
                SMTCell::getAssigned(tmp_m1) == 0) {
              if (!SMTCell::ifAssigned(tmp_m2)) {
                SMTCell::assignTrueVar(tmp_m2, 1, false);
                SMTCell::setVar_wo_cnt(tmp_m2, 1);
              }
            } else if (!SMTCell::ifAssigned(tmp_m1)) {
              if (!SMTCell::ifAssigned(tmp_m2)) {
                SMTCell::setVar(tmp_m1, 2);
                SMTCell::setVar(tmp_m2, 2);
                SMTCell::writeConstraint(
                    fmt::format("(assert (ite (= {} false) (= {} false) (= "
                                "true true)))\n",
                                tmp_m1, tmp_m2));
                SMTCell::cnt("l", 3);
                SMTCell::cnt("l", 3);
                SMTCell::cnt("c", 3);
              }
            }
          } else if (SMTCell::ifAssigned(tmp_m1) &&
                     SMTCell::getAssigned(tmp_m1) == 0) {
            if (SMTCell::ifAssigned(tmp_g) &&
                SMTCell::getAssigned(tmp_g) == 0) {
              if (!SMTCell::ifAssigned(tmp_g)) {
                SMTCell::assignTrueVar(tmp_g, 1, false);
                SMTCell::setVar_wo_cnt(tmp_g, 1);
              }
            } else if (!SMTCell::ifAssigned(tmp_g)) {
              if (!SMTCell::ifAssigned(tmp_m2)) {
                SMTCell::setVar(tmp_g, 2);
                SMTCell::setVar(tmp_m2, 2);
                SMTCell::writeConstraint(
                    fmt::format("(assert (ite (= {} false) (= {} false) (= "
                                "true true)))\n",
                                tmp_g, tmp_m2));
                SMTCell::cnt("l", 3);
                SMTCell::cnt("l", 3);
                SMTCell::cnt("c", 3);
              }
            }
          }
        }
      }
    }
  }
}

void DesignRuleWriter::write_geometric_variables_AGR_B_tip_helper(FILE *drlog) {
  // # At the top-most metal layer, only vias exist.
  // for (int metal = SMTCell::getFirstRoutingLayer();
  //      metal <= SMTCell::getNumMetalLayer(); metal++) {
  //   if (SMTCell::ifVertMetal(metal)) {
  //     for (int row = 0; row <= SMTCell::getLastRow(); row++) {
  //       for (int col = 0; col <= SMTCell::getLastColIdx(); col++) {
  for (int metal = 1; metal <= SMTCell::getNumMetalLayer(); metal++) {
    if (SMTCell::ifVertMetal(metal)) {
      for (int row_idx = 0;
           row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3; row_idx++) {
        for (int col_idx = 0;
             col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
          // retrieve row/col
          int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
          int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);
          if (metal == 1 && SMTCell::ifSDCol_AGR(metal, col)) {
            continue;
          }
          // if (metal > 1 && SMTCell::ifVertMetal(metal) &&
          //     SMTCell::ifSDCol(col)) {
          //   continue;
          // }

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d     GV_B: METAL {} ROW {} COL {} \n", metal,
                       row, col);
          }

          std::string tmp_g =
              fmt::format("GB_V_{}", SMTCell::getVertexName(metal, row, col));

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        tmp_g: {} \n", tmp_g);
          }

          std::string tmp_m1 = "";
          if (SMTCell::getVertex(metal, row, col)->getFrontADJ()->ifValid()) {
            tmp_m1 = fmt::format(
                "M_{}_{}",
                SMTCell::getVertex(metal, row, col)->getFrontADJ()->getVName(),
                SMTCell::getVertexName(metal, row, col));
          } else {
            tmp_m1 = fmt::format("M_FrontEnd_{}",
                                 SMTCell::getVertexName(metal, row, col));
          }

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        tmp_m1: {} \n", tmp_m1);
          }

          if (!SMTCell::ifAssigned(tmp_g) && !SMTCell::ifAssigned(tmp_m1)) {
            SMTCell::setVar(tmp_g, 6);
            SMTCell::setVar(tmp_m1, 2);
            SMTCell::writeConstraint(fmt::format(
                "(assert (ite (= {} true) (= {} true) (= true true)))\n", tmp_g,
                tmp_m1));
            SMTCell::cnt("l", 3);
            SMTCell::cnt("l", 3);
            SMTCell::cnt("c", 3);
          } else if (SMTCell::ifAssigned(tmp_g) &&
                     SMTCell::getAssigned(tmp_g) == 1) {
            if (!SMTCell::ifAssigned(tmp_m1)) {
              SMTCell::assignTrueVar(tmp_m1, 1, false);
              SMTCell::setVar_wo_cnt(tmp_m1, 1);
            }
          }
          std::string tmp_m2 = "";
          if (SMTCell::getVertex(metal, row, col)->getBackADJ()->ifValid()) {
            tmp_m2 = fmt::format(
                "M_{}_{}", SMTCell::getVertexName(metal, row, col),
                SMTCell::getVertex(metal, row, col)->getBackADJ()->getVName());
          } else {
            tmp_m2 = fmt::format("M_{}_BackEnd",
                                 SMTCell::getVertexName(metal, row, col));
          }

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        tmp_m2: {} \n", tmp_m2);
          }

          if (!SMTCell::ifAssigned(tmp_g) && !SMTCell::ifAssigned(tmp_m2)) {
            SMTCell::setVar(tmp_g, 6);
            SMTCell::setVar(tmp_m2, 2);
            SMTCell::writeConstraint(
                fmt::format("(assert ((_ at-most 1) {} {}))\n", tmp_g, tmp_m2));
            SMTCell::cnt("l", 3);
            SMTCell::cnt("l", 3);
            SMTCell::cnt("c", 3);
          } else if (SMTCell::ifAssigned(tmp_m2) &&
                     SMTCell::getAssigned(tmp_m2) == 1) {
            if (!SMTCell::ifAssigned(tmp_g)) {
              SMTCell::assignTrueVar(tmp_g, 0, false);
            }
          } else if (SMTCell::ifAssigned(tmp_g) &&
                     SMTCell::getAssigned(tmp_g) == 1) {
            if (!SMTCell::ifAssigned(tmp_m2)) {
              SMTCell::assignTrueVar(tmp_m2, 0, false);
            }
          }
          if (!SMTCell::ifAssigned(tmp_g) && !SMTCell::ifAssigned(tmp_m2)) {
            if (!SMTCell::ifAssigned(tmp_m1)) {
              SMTCell::setVar(tmp_g, 6);
              SMTCell::setVar(tmp_m2, 2);
              SMTCell::setVar(tmp_m1, 2);
              SMTCell::writeConstraint(
                  fmt::format("(assert (ite (= (or {} {}) false) (= {} "
                              "false) (= true true)))\n",
                              tmp_g, tmp_m2, tmp_m1));
              SMTCell::cnt("l", 3);
              SMTCell::cnt("l", 3);
              SMTCell::cnt("l", 3);
              SMTCell::cnt("c", 3);
            }
          } else if (SMTCell::ifAssigned(tmp_g) &&
                     SMTCell::getAssigned(tmp_g) == 0) {
            if (SMTCell::ifAssigned(tmp_m2) &&
                SMTCell::getAssigned(tmp_m2) == 0) {
              if (!SMTCell::ifAssigned(tmp_m1)) {
                SMTCell::assignTrueVar(tmp_m1, 1, false);
                SMTCell::setVar_wo_cnt(tmp_m1, 1);
              }
            } else if (!SMTCell::ifAssigned(tmp_m2)) {
              if (!SMTCell::ifAssigned(tmp_m1)) {
                SMTCell::setVar(tmp_m2, 2);
                SMTCell::setVar(tmp_m1, 2);
                SMTCell::writeConstraint(
                    fmt::format("(assert (ite (= {} false) (= {} false) (= "
                                "true true)))\n",
                                tmp_m2, tmp_m1));
                SMTCell::cnt("l", 3);
                SMTCell::cnt("l", 3);
                SMTCell::cnt("c", 3);
              }
            }
          } else if (SMTCell::ifAssigned(tmp_m2) &&
                     SMTCell::getAssigned(tmp_m2) == 0) {
            if (SMTCell::ifAssigned(tmp_g) &&
                SMTCell::getAssigned(tmp_g) == 0) {
              if (!SMTCell::ifAssigned(tmp_g)) {
                SMTCell::assignTrueVar(tmp_g, 1, false);
                SMTCell::setVar_wo_cnt(tmp_g, 1);
              }
            } else if (!SMTCell::ifAssigned(tmp_g)) {
              if (!SMTCell::ifAssigned(tmp_m1)) {
                SMTCell::setVar(tmp_g, 2);
                SMTCell::setVar(tmp_m1, 2);
                SMTCell::writeConstraint(
                    fmt::format("(assert (ite (= {} false) (= {} false) (= "
                                "true true)))\n",
                                tmp_g, tmp_m1));
                SMTCell::cnt("l", 3);
                SMTCell::cnt("l", 3);
                SMTCell::cnt("c", 3);
              }
            }
          }
        }
      }
    }
  }
}

/**
 * Minimum Area Rule (MAR):
 * Enforces each disjoint metal segment should be larger than the
 * minimum manufacturable size.
 *
 * @note
 * Example: (MAR = 2) No Violation
 *     |    |    |
 * ====X====X====X
 *     |    |    |
 *
 * @param   PRL_Parameter   Distance in L1
 * @param   doublePowerRail If using Double Power Rail
 *
 * @return  void
 */
void DesignRuleWriter::write_MAR_AGR_rule(int MAR_M1_Parameter,
                                          int MAR_M2_Parameter,
                                          int MAR_M3_Parameter,
                                          int MAR_M4_Parameter,
                                          int doublePowerRail, FILE *drlog) {
  SMTCell::writeConstraint(";7. Minimum Area Rule\n");
  fmt::print(drlog, "7. Minimum Area Rule\n");
  if (MAR_M1_Parameter == 0 && MAR_M2_Parameter == 0 && MAR_M3_Parameter == 0 &&
      MAR_M4_Parameter == 0) {
    fmt::print("is disabled\n");
    SMTCell::writeConstraint(";MAR is disabled\n");
  } else {
    // # MAR Rule Enable /Disable
    // ### Minimum Area Rule to prevent from having small metal segment
    for (int metal = SMTCell::getFirstRoutingLayer();
         metal <= SMTCell::getNumMetalLayer(); metal++) {
      // use Layer-specific MAR Parameter
      int MAR_Parameter = 0;
      if (metal == 1) {
        MAR_Parameter = MAR_M1_Parameter;
      } else if (metal == 2) {
        MAR_Parameter = MAR_M2_Parameter;
      } else if (metal == 3) {
        MAR_Parameter = MAR_M3_Parameter;
      } else if (metal == 4) {
        MAR_Parameter = MAR_M4_Parameter;
      }
      // Horizontal Direction: consider columnwise constraint
      if (SMTCell::ifHorzMetal(metal)) {
        for (int row_idx = 0;
             row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3; row_idx++) {
          for (int col_idx = 0;
               col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1;
               col_idx++) {
            // retrieve row/col
            int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
            int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);

            if (SMTCell::DEBUG_MODE_) {
              fmt::print(drlog, "d     MAR : METAL {} ROW {} COL {} \n", metal,
                         row, col);
            }

            // AGR FLAG: iterate until col - MAR_Parameter
            if (col > (SMTCell::getCellWidth() - MAR_Parameter)) {
              break;
            }

            std::vector<std::string> tmp_var;
            int cnt_var = 0;
            int cnt_true = 0;

            // AGR FLAG:  Map maximum MAR_Parameter to a grid column idx and
            //            iterate till this idx
            // Grid View:       +---POI-------------+-------+
            // MAR Distance:    +---POI\\\\\\\\\\\\\\\\-----+
            // AGR View:        +---POI-------------X-------+

            // "X" index: Retrieve the nearest column idx to the left of the max
            // MAR_Parameter
            int max_marIndex =
                SMTCell::getNearestColIdxToLeft(metal, col + MAR_Parameter);

            fmt::print(drlog, "d       => MAR max_marIndex: {} \n",
                       max_marIndex);

            // Start from current col_idx and iterate till max_marIndex - 1
            for (int marIndex = col_idx; marIndex <= max_marIndex; marIndex++) {
              int colNumber = SMTCell::get_h_metal_col_by_idx(metal, marIndex);
              // Triplet *vCoord = new Triplet(metal, row, colNumber);
              std::shared_ptr<Triplet> vCoord =
                  std::make_shared<Triplet>(metal, row, colNumber);

              // Prohibit GV mapping to both side of the metal
              // Left side
              std::string tmp_str = fmt::format("GL_V_{}", vCoord->getVName());
              if (!SMTCell::ifAssigned(tmp_str)) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog, "d       => Adding Forbidden MAR L : {} \n",
                             tmp_str);
                }
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssigned(tmp_str) &&
                         SMTCell::getAssigned(tmp_str) == 1) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog, "d       => Skipping MAR L : {} \n",
                             tmp_str);
                }
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }
              // Right side
              tmp_str = fmt::format("GR_V_{}", vCoord->getVName());
              if (!SMTCell::ifAssigned(tmp_str)) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog, "d       => Adding Forbidden MAR R : {} \n",
                             tmp_str);
                }
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssigned(tmp_str) &&
                         SMTCell::getAssigned(tmp_str) == 1) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog, "d       => Skipping MAR R : {} \n",
                             tmp_str);
                }
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }
              // delete vCoord;
            }

            if (cnt_true > 0) {
              if (cnt_true > 1) {
                std::cerr << "\n[ERROR] MAR : more than one G Variables are "
                             "true!!!\n";
                exit(1);
              } else {
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::assignTrueVar(tmp_var[i], 0, true);
                }
              }
            } else {
              if (cnt_var > 1) {
                SMTCell::writeConstraint(fmt::format("(assert ((_ at-most 1)"));
                for (auto s : tmp_var) {
                  SMTCell::writeConstraint(fmt::format(" {}", s));
                  SMTCell::cnt("l", 3);
                }
                SMTCell::writeConstraint(fmt::format("))\n"));
                SMTCell::cnt("c", 3);
              }
            }
          }
        }
      }
      // Vertical Direction: consider rowwise constraint
      else {
        // *NOTICE*: CURRENT IMPLEMENTATION DOES NOT SUPPORT ROWWISE REAL NUMBER
        // AGR FLAG:  Map maximum MAR_Parameter to a grid row idx and
        //            iterate till this idx
        // Grid View: +   MAR Distance: +     AGR View: +
        //            |                 |               |
        //            |                 /               |
        //            +                 /               X
        //            |                 /               |
        //            |                 /               |
        //           POI               POI             POI
        //            |                 |               |
        //            |                 |               |
        //            +                 +               +

        // BUG FLAG: This has to be at least 0
        int max_marIndex = (SMTCell::row_real_to_idx(MAR_Parameter) - 1) > 0
                               ? SMTCell::row_real_to_idx(MAR_Parameter) - 1
                               : 0;

        // Double Power Rail => MaxIndex - 1
        if (doublePowerRail == 1) {
          max_marIndex -= 1;
        }

        // AGR FLAG: Directly iterate till max_marIndex is possible here because
        // we abstract row as index for now

        for (int row_idx = 0; row_idx <= SMTCell::get_h_metal_numTrackH(metal) -
                                             3 - max_marIndex;
             row_idx++) {
          for (int col_idx = 0;
               col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1;
               col_idx++) {
            // retrieve row/col
            int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
            int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);

            if (SMTCell::DEBUG_MODE_) {
              fmt::print(drlog, "d     MAR : METAL {} ROW {} COL {} \n", metal,
                         row, col);
            }

            std::shared_ptr<Triplet> vCoord =
                std::make_shared<Triplet>(metal, row, col);

            int newRow = row;

            if (SMTCell::getVertex((*vCoord))->getBackADJ()->ifValid() ==
                false) {
              continue;
            }

            std::vector<std::string> tmp_var;
            int cnt_var = 0;
            int cnt_true = 0;

            // AGR FLAG: Increment by Real number for MAR distance
            for (int marIndex = 0; marIndex <= MAR_Parameter - 1;
                 marIndex += SMTCell::row_idx_to_real(1)) {

              std::string tmp_str = fmt::format("GF_V_{}", vCoord->getVName());
              if (!SMTCell::ifAssigned(tmp_str)) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog, "d       => Adding Forbidden MAR F : {} \n",
                             tmp_str);
                }
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssigned(tmp_str) &&
                         SMTCell::getAssigned(tmp_str) == 1) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }
              tmp_str = fmt::format("GB_V_{}", vCoord->getVName());
              if (!SMTCell::ifAssigned(tmp_str)) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog, "d       => Adding Forbidden MAR B : {} \n",
                             tmp_str);
                }
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssigned(tmp_str) &&
                         SMTCell::getAssigned(tmp_str) == 1) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }
              newRow++;
              vCoord = SMTCell::getVertex((*vCoord))->getBackADJ();
              if (vCoord->ifValid() == false) {
                break;
              }
            }
            if (cnt_true > 0) {
              if (cnt_true > 1) {
                std::cerr << "\n[ERROR] MAR : more than one G Variables are "
                             "true!!!\n";
                exit(1);
              } else {
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::assignTrueVar(tmp_var[i], 0, true);
                }
              }
            } else {
              if (cnt_var > 1) {
                SMTCell::writeConstraint(fmt::format("(assert ((_ at-most 1)"));
                for (auto s : tmp_var) {
                  SMTCell::writeConstraint(fmt::format(" {}", s));
                  SMTCell::cnt("l", 3);
                }
                SMTCell::writeConstraint(fmt::format("))\n"));
                SMTCell::cnt("c", 3);
              }
            }
          }
        }
      }
    }
    SMTCell::writeConstraint("\n");
    std::cout << "have been written.\n";
  }
}

/**
 * End-Of-Line Spacing Rule (EOL) rule:
 * Enforces the distance between each EOL of two metal segments that are coming
 * from opposite directions should be greater than the minimum spacing distance.
 *
 * @note
 * Example: (EOL = 2) No Violation
 * ----+----X====X===
 *     |    |    |
 * ====X----+----X===
 *     |    |    |
 * ----+----X====X===
 *
 * @param   EOL_Parameter   Distance in L1
 * @param   doublePowerRail If using Double Power Rail
 *
 * @return  void
 */
void DesignRuleWriter::write_EOL_AGR_rule(
    int EOL_M1_B_ADJ_Parameter, int EOL_M1_B_Parameter,
    int EOL_M2_R_ADJ_Parameter, int EOL_M2_R_Parameter,
    int EOL_M3_B_ADJ_Parameter, int EOL_M3_B_Parameter,
    int EOL_M4_R_ADJ_Parameter, int EOL_M4_R_Parameter, int doublePowerRail,
    FILE *drlog) {
  SMTCell::writeConstraint(";8. Tip-to-Tip Spacing Rule\n");
  fmt::print(drlog, "8. Tip-to-Tip Spacing Rule\n");
  // if (EOL_Parameter == 0) {
  if (EOL_M1_B_ADJ_Parameter && EOL_M1_B_Parameter && EOL_M2_R_ADJ_Parameter &&
      EOL_M2_R_Parameter && EOL_M3_B_ADJ_Parameter && EOL_M3_B_Parameter &&
      EOL_M4_R_ADJ_Parameter && EOL_M4_R_Parameter) {
    fmt::print("is disabled\n");
    SMTCell::writeConstraint(";EOL is disabled\n");
  } else {
    // # EOL Rule Enable /Disable
    // ### Tip-to-Tip Spacing Rule to prevent from having too close metal
    // tips.
    // ### DATA STRUCTURE:  VERTEX [index] [name] [Z-pos] [Y-pos] [X-pos]
    // [Arr. of adjacent vertices]
    // ### DATA STRUCTURE:  ADJACENT_VERTICES [0:Left] [1:Right] [2:Front]
    // [3:Back] [4:Up] [5:Down] [6:FL] [7:FR] [8:BL] [9:BR]
    SMTCell::writeConstraint(
        ";8-A. from Right Tip to Left Tips for each vertex\n");
    fmt::print(drlog, ";8-A. from Right Tip to Left Tips for each vertex\n");
    DesignRuleWriter::write_EOL_AGR_RL_tip_helper(
        EOL_M2_R_ADJ_Parameter, EOL_M2_R_Parameter, EOL_M4_R_ADJ_Parameter,
        EOL_M4_R_Parameter, doublePowerRail, drlog);
    SMTCell::writeConstraint("\n");

    // ### DATA STRUCTURE:  VERTEX [index] [name] [Z-pos] [Y-pos] [X-pos]
    // [Arr. of adjacent vertices]
    // ### DATA STRUCTURE:  ADJACENT_VERTICES [0:Left] [1:Right] [2:Front]
    // [3:Back] [4:Up] [5:Down] [6:FL] [7:FR] [8:BL] [9:BR]
    SMTCell::writeConstraint(
        ";8-B. from Left Tip to Right Tips for each vertex\n");
    fmt::print(drlog, ";8-B. from Left Tip to Right Tips for each vertex\n");
    DesignRuleWriter::write_EOL_AGR_LR_tip_helper(
        EOL_M2_R_ADJ_Parameter, EOL_M2_R_Parameter, EOL_M4_R_ADJ_Parameter,
        EOL_M4_R_Parameter, doublePowerRail, drlog);
    SMTCell::writeConstraint("\n");

    // ### DATA STRUCTURE:  VERTEX [index] [name] [Z-pos] [Y-pos] [X-pos]
    // [Arr. of adjacent vertices]
    // ### DATA STRUCTURE:  ADJACENT_VERTICES [0:Left] [1:Right] [2:Front]
    // [3:Back] [4:Up] [5:Down] [6:FL] [7:FR] [8:BL] [9:BR]
    // #
    // ##### one Power Rail vertice has 2 times cost of other vertices.
    // #
    SMTCell::writeConstraint(
        ";8-C. from Back Tip to Front Tips for each vertex\n");
    fmt::print(drlog, ";8-C. from Back Tip to Front Tips for each vertex\n");
    DesignRuleWriter::write_EOL_AGR_BF_tip_helper(
        EOL_M1_B_ADJ_Parameter, EOL_M1_B_Parameter, EOL_M3_B_ADJ_Parameter,
        doublePowerRail, EOL_M3_B_Parameter, drlog);
    SMTCell::writeConstraint("\n");

    // ### DATA STRUCTURE:  VERTEX [index] [name] [Z-pos] [Y-pos] [X-pos]
    // [Arr. of adjacent vertices]
    // ### DATA STRUCTURE:  ADJACENT_VERTICES [0:Left] [1:Right] [2:Front]
    // [3:Back] [4:Up] [5:Down] [6:FL] [7:FR] [8:BL] [9:BR]
    SMTCell::writeConstraint(
        ";8-D. from Front Tip to Back Tips for each vertex\n");
    fmt::print(drlog, ";8-D. from Front Tip to Back Tips for each vertex\n");
    DesignRuleWriter::write_EOL_AGR_FB_tip_helper(
        EOL_M1_B_ADJ_Parameter, EOL_M1_B_Parameter, EOL_M3_B_ADJ_Parameter,
        EOL_M3_B_Parameter, doublePowerRail, drlog);
    SMTCell::writeConstraint("\n");
    std::cout << "have been written.\n";
  }
}

/**
 * @note
 *                            ----+----X====X==== <--- BR Direction Checking
 *                                |    |    |
 *                            ▒▒▒▒X----+----X==== <--- R Direction Checking
 *                                |    |    |
 *                            ----+----X====X==== <--- FR Direction Checking
 */
void DesignRuleWriter::write_EOL_AGR_RL_tip_helper(int EOL_M2_R_ADJ_Parameter,
                                                   int EOL_M2_R_Parameter,
                                                   int EOL_M4_R_ADJ_Parameter,
                                                   int EOL_M4_R_Parameter,
                                                   int doublePowerRail,
                                                   FILE *drlog) {
  for (int metal = SMTCell::getFirstRoutingLayer();
       metal <= SMTCell::getNumMetalLayer(); metal++) {
    if (SMTCell::ifHorzMetal(metal)) {
      for (int row_idx = 0;
           row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3; row_idx++) {
        for (int col_idx = 0;
             col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 2; col_idx++) {
          // retrieve row/col
          int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
          int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d     EOL_RL: METAL {} ROW {} COL {} \n", metal,
                       row, col);
          }
          // FR Direction Checking
          std::shared_ptr<Triplet> vCoord_FR =
              SMTCell::getVertex(metal, row, col)->getFrontRightADJ();

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        vCoord_FR: {} \n",
                       vCoord_FR->getVName());
          }

          int EOL_R_ADJ_Parameter = 0;
          int EOL_R_Parameter = 0;

          // Layer-specific EOL Parameter
          if (metal == 2) {
            EOL_R_ADJ_Parameter = EOL_M2_R_ADJ_Parameter;
            EOL_R_Parameter = EOL_M2_R_Parameter;
          } else if (metal == 4) {
            EOL_R_ADJ_Parameter = EOL_M4_R_ADJ_Parameter;
            EOL_R_Parameter = EOL_M4_R_Parameter;
          } else {
            fmt::print("[ERROR] Something Wrong in EOL for M{} \n", metal);
          }

          // FR Direction Checking
          // AGR FLAG: EOL at least should be greater than row height to work
          if (vCoord_FR->ifValid() && row_idx != 0 &&
              EOL_R_ADJ_Parameter > SMTCell::row_idx_to_real(1)) {
            std::vector<std::string> tmp_var;
            int cnt_var = 0;
            int cnt_true = 0;
            std::string tmp_str =
                fmt::format("GR_V_{}", SMTCell::getVertexName(metal, row, col));
            if (!SMTCell::ifAssigned(tmp_str)) {
              tmp_var.push_back(tmp_str);
              SMTCell::setVar(tmp_str, 2);
              cnt_var++;
            } else if (SMTCell::ifAssigned(tmp_str) &&
                       SMTCell::getAssigned(tmp_str) == 1) {
              SMTCell::setVar_wo_cnt(tmp_str, 0);
              cnt_true++;
            }

            // AGR FLAG
            int currCol = vCoord_FR->col_;
            int distToPOICol = abs(currCol - col);
            int realEOL = EOL_R_ADJ_Parameter - SMTCell::row_idx_to_real(1);

            // for (int eolIndex = 0; eolIndex <= EOL_Parameter - 2; eolIndex++)
            // {
            while (distToPOICol <= realEOL) {
              std::string tmp_str =
                  fmt::format("GL_V_{}", vCoord_FR->getVName());
              if (!SMTCell::ifAssigned(tmp_str)) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog,
                             "d       => Adding Forbidden EOL RL : {} \n",
                             tmp_str);
                }
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssigned(tmp_str) &&
                         SMTCell::getAssigned(tmp_str) == 1) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }
              vCoord_FR = SMTCell::getVertex((*vCoord_FR))->getRightADJ();
              if (vCoord_FR->ifValid() == false) {
                break;
              } else {
                currCol = vCoord_FR->col_;
                distToPOICol = abs(currCol - col);
              }
            }
            if (cnt_true > 0) {
              if (cnt_true > 1) {
                std::cerr << "\n[ERROR] TIP2TIP : more than one G "
                             "Variables are true!!!\n";
                exit(1);
              } else {
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::assignTrueVar(tmp_var[i], 0, true);
                }
              }
            } else {
              if (cnt_var > 1) {
                SMTCell::writeConstraint(fmt::format("(assert ((_ at-most 1)"));
                for (auto s : tmp_var) {
                  SMTCell::writeConstraint(fmt::format(" {}", s));
                  SMTCell::cnt("l", 3);
                }
                SMTCell::writeConstraint(fmt::format("))\n"));
                SMTCell::cnt("c", 3);
              }
            }
          }

          // R Direction Checking
          std::shared_ptr<Triplet> vCoord_R =
               SMTCell::getVertex(metal, row, col)->getRightADJ();

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        vCoord_R: {} \n", vCoord_R->getVName());
          }

          if (vCoord_R->ifValid()) {
            std::vector<std::string> tmp_var;
            int cnt_var = 0;
            int cnt_true = 0;
            std::string tmp_str =
                fmt::format("GR_V_{}", SMTCell::getVertexName(metal, row, col));
            if (!SMTCell::ifAssigned(tmp_str)) {
              tmp_var.push_back(tmp_str);
              SMTCell::setVar(tmp_str, 2);
              cnt_var++;
            } else if (SMTCell::ifAssigned(tmp_str) &&
                       SMTCell::getAssigned(tmp_str) == 1) {
              SMTCell::setVar_wo_cnt(tmp_str, 0);
              cnt_true++;
            }

            // AGR FLAG
            int currCol = vCoord_R->col_;
            int distToPOICol = abs(currCol - col);
            int realEOL = EOL_R_Parameter;

            while (distToPOICol <= realEOL) {
              std::string tmp_str =
                  fmt::format("GL_V_{}", vCoord_R->getVName());
              if (!SMTCell::ifAssigned(tmp_str)) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog,
                             "d       => Adding Forbidden EOL RL : {} \n",
                             tmp_str);
                }
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssigned(tmp_str) &&
                         SMTCell::getAssigned(tmp_str) == 1) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }

              vCoord_R = SMTCell::getVertex((*vCoord_R))->getRightADJ();
              if (vCoord_R->ifValid() == false) {
                break;
              } else {
                currCol = vCoord_R->col_;
                distToPOICol = abs(currCol - col);
              }
            }
            if (cnt_true > 0) {
              if (cnt_true > 1) {
                std::cerr << "\n[ERROR] TIP2TIP : more than one G "
                             "Variables are true!!!\n";
                exit(1);
              } else {
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::assignTrueVar(tmp_var[i], 0, true);
                }
              }
            } else {
              if (cnt_var > 1) {
                SMTCell::writeConstraint(fmt::format("(assert ((_ at-most 1)"));
                for (auto s : tmp_var) {
                  SMTCell::writeConstraint(fmt::format(" {}", s));
                  SMTCell::cnt("l", 3);
                }
                SMTCell::writeConstraint(fmt::format("))\n"));
                SMTCell::cnt("c", 3);
              }
            }
          }

          // BR Direction Checking
          std::shared_ptr<Triplet> vCoord_BR =
              SMTCell::getVertex(metal, row, col)->getBackRightADJ();

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        vCoord_BR: {} \n",
                       vCoord_BR->getVName());
          }

          if (vCoord_BR->ifValid() && row != (SMTCell::getNumTrackH() - 3) &&
              EOL_R_ADJ_Parameter > SMTCell::row_idx_to_real(1)) {
            std::vector<std::string> tmp_var;
            int cnt_var = 0;
            int cnt_true = 0;
            std::string tmp_str =
                fmt::format("GR_V_{}", SMTCell::getVertexName(metal, row, col));
            if (!SMTCell::ifAssigned(tmp_str)) {
              tmp_var.push_back(tmp_str);
              SMTCell::setVar(tmp_str, 2);
              cnt_var++;
            } else if (SMTCell::ifAssigned(tmp_str) &&
                       SMTCell::getAssigned(tmp_str) == 1) {
              SMTCell::setVar_wo_cnt(tmp_str, 0);
              cnt_true++;
            }

            // AGR FLAG
            int currCol = vCoord_BR->col_;
            int distToPOICol = abs(currCol - col);
            int realEOL = EOL_R_ADJ_Parameter - SMTCell::row_idx_to_real(1);

            while (distToPOICol <= realEOL) {
              std::string tmp_str =
                  fmt::format("GL_V_{}", vCoord_BR->getVName());
              if (!SMTCell::ifAssigned(tmp_str)) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog,
                             "d       => Adding Forbidden EOL RL : {} \n",
                             tmp_str);
                }
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssigned(tmp_str) &&
                         SMTCell::getAssigned(tmp_str) == 1) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }
              vCoord_BR = SMTCell::getVertex((*vCoord_BR))->getRightADJ();
              if (vCoord_BR->ifValid() == false) {
                break;
              } else {
                currCol = vCoord_BR->col_;
                distToPOICol = abs(currCol - col);
              }
            }
            if (cnt_true > 0) {
              if (cnt_true > 1) {
                std::cerr << "\n[ERROR] TIP2TIP : more than one G "
                             "Variables are true!!!\n";
                exit(1);
              } else {
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::assignTrueVar(tmp_var[i], 0, true);
                }
              }
            } else {
              if (cnt_var > 1) {
                SMTCell::writeConstraint(fmt::format("(assert ((_ at-most 1)"));
                for (auto s : tmp_var) {
                  SMTCell::writeConstraint(fmt::format(" {}", s));
                  SMTCell::cnt("l", 3);
                }
                SMTCell::writeConstraint(fmt::format("))\n"));
                SMTCell::cnt("c", 3);
              }
            }
          }
        }
      }
    }
  }
}
/**
 * @note
 *                            ====X====X----+---- <--- BL Direction Checking
 *                                |    |    |
 *                            ====X----+----X▒▒▒▒ <--- L Direction Checking
 *                                |    |    |
 *                            ====X====X----+---- <--- FL Direction Checking
 */
void DesignRuleWriter::write_EOL_AGR_LR_tip_helper(int EOL_M2_R_ADJ_Parameter,
                                                   int EOL_M2_R_Parameter,
                                                   int EOL_M4_R_ADJ_Parameter,
                                                   int EOL_M4_R_Parameter,
                                                   int doublePowerRail,
                                                   FILE *drlog) {
  for (int metal = SMTCell::getFirstRoutingLayer();
       metal <= SMTCell::getNumMetalLayer(); metal++) {
    if (SMTCell::ifHorzMetal(metal)) {
      for (int row_idx = 0;
           row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3; row_idx++) {
        for (int col_idx = 0;
             col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
          // retrieve row/col
          int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
          int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d     EOL_LR: METAL {} ROW {} COL {} \n", metal,
                       row, col);
          }

          std::shared_ptr<Triplet> vCoord_FL =
              SMTCell::getVertex(metal, row, col)->getFrontLeftADJ();

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        vCoord_FL: {} \n",
                       vCoord_FL->getVName());
          }

          int EOL_L_ADJ_Parameter = 0;
          int EOL_L_Parameter = 0;

          // Layer-specific EOL Parameter
          if (metal == 2) {
            EOL_L_ADJ_Parameter = EOL_M2_R_ADJ_Parameter;
            EOL_L_Parameter = EOL_M2_R_Parameter;
          } else if (metal == 4) {
            EOL_L_ADJ_Parameter = EOL_M4_R_ADJ_Parameter;
            EOL_L_Parameter = EOL_M4_R_Parameter;
          } else {
            fmt::print("[ERROR] Something Wrong in EOL for M{} \n", metal);
          }

          // FL Direction Checking
          // AGR FLAG
          if (vCoord_FL->ifValid() && row_idx != 0 &&
              EOL_L_ADJ_Parameter > SMTCell::row_idx_to_real(1)) {
            std::vector<std::string> tmp_var;
            int cnt_var = 0;
            int cnt_true = 0;
            std::string tmp_str =
                fmt::format("GL_V_{}", SMTCell::getVertexName(metal, row, col));
            if (!SMTCell::ifAssigned(tmp_str)) {
              tmp_var.push_back(tmp_str);
              SMTCell::setVar(tmp_str, 2);
              cnt_var++;
            } else if (SMTCell::ifAssigned(tmp_str) &&
                       SMTCell::getAssigned(tmp_str) == 1) {
              SMTCell::setVar_wo_cnt(tmp_str, 0);
              cnt_true++;
            }

            // AGR FLAG
            int currCol = vCoord_FL->col_;
            int distToPOICol = abs(currCol - col);
            int realEOL = EOL_L_ADJ_Parameter - SMTCell::row_idx_to_real(1);

            while (distToPOICol <= realEOL) {
              std::string tmp_str =
                  fmt::format("GR_V_{}", vCoord_FL->getVName());
              if (!SMTCell::ifAssigned(tmp_str)) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog,
                             "d       => Adding Forbidden EOL LR : {} \n",
                             tmp_str);
                }
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssigned(tmp_str) &&
                         SMTCell::getAssigned(tmp_str) == 1) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }

              vCoord_FL = SMTCell::getVertex((*vCoord_FL))->getLeftADJ();
              if (vCoord_FL->ifValid() == false) {
                break;
              } else {
                currCol = vCoord_FL->col_;
                distToPOICol = abs(currCol - col);
              }
              // }
            }
            if (cnt_true > 0) {
              if (cnt_true > 1) {
                std::cerr << "\n[ERROR] TIP2TIP : more than one G "
                             "Variables are true!!!\n";
                exit(1);
              } else {
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::assignTrueVar(tmp_var[i], 0, true);
                }
              }
            } else {
              if (cnt_var > 1) {
                SMTCell::writeConstraint(fmt::format("(assert ((_ at-most 1)"));
                for (auto s : tmp_var) {
                  SMTCell::writeConstraint(fmt::format(" {}", s));
                  SMTCell::cnt("l", 3);
                }
                SMTCell::writeConstraint(fmt::format("))\n"));
                SMTCell::cnt("c", 3);
              }
            }
          }

          // L Direction Checking
          std::shared_ptr<Triplet> vCoord_L =
              SMTCell::getVertex(metal, row, col)->getLeftADJ();

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        vCoord_L: {} \n",
                       vCoord_FL->getVName());
          }

          if (vCoord_L->ifValid()) {
            std::vector<std::string> tmp_var;
            int cnt_var = 0;
            int cnt_true = 0;
            std::string tmp_str =
                fmt::format("GL_V_{}", SMTCell::getVertexName(metal, row, col));

            int currCol = vCoord_L->col_;
            int distToPOICol = abs(currCol - col);
            int realEOL = EOL_L_Parameter;

            while (distToPOICol <= realEOL) {
              std::string tmp_str =
                  fmt::format("GR_V_{}", vCoord_L->getVName());
              if (!SMTCell::ifAssigned(tmp_str)) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog,
                             "d       => Adding Forbidden EOL LR : {} \n",
                             tmp_str);
                }
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssigned(tmp_str) &&
                         SMTCell::getAssigned(tmp_str) == 1) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }

              vCoord_L = SMTCell::getVertex((*vCoord_L))->getLeftADJ();
              if (vCoord_L->ifValid() == false) {
                break;
              } else {
                currCol = vCoord_L->col_;
                distToPOICol = abs(currCol - col);
              }
            }
            if (cnt_true > 0) {
              if (cnt_true > 1) {
                std::cerr << "\n[ERROR] TIP2TIP : more than one G "
                             "Variables are true!!!\n";
                exit(1);
              } else {
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::assignTrueVar(tmp_var[i], 0, true);
                }
              }
            } else {
              if (cnt_var > 1) {
                SMTCell::writeConstraint(fmt::format("(assert ((_ at-most 1)"));
                for (auto s : tmp_var) {
                  SMTCell::writeConstraint(fmt::format(" {}", s));
                  SMTCell::cnt("l", 3);
                }
                SMTCell::writeConstraint(fmt::format("))\n"));
                SMTCell::cnt("c", 3);
              }
            }
          }

          // BL Direction Checking
          std::shared_ptr<Triplet> vCoord_BL =
              SMTCell::getVertex(metal, row, col)->getBackLeftADJ();

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        vCoord_BL: {} \n",
                       vCoord_BL->getVName());
          }
          if (vCoord_BL->ifValid() && row != (SMTCell::getNumTrackH() - 3) &&
              EOL_L_ADJ_Parameter > SMTCell::row_idx_to_real(1)) {
            std::vector<std::string> tmp_var;
            int cnt_var = 0;
            int cnt_true = 0;
            std::string tmp_str =
                fmt::format("GL_V_{}", SMTCell::getVertexName(metal, row, col));
            if (!SMTCell::ifAssigned(tmp_str)) {
              tmp_var.push_back(tmp_str);
              SMTCell::setVar(tmp_str, 2);
              cnt_var++;
            } else if (SMTCell::ifAssigned(tmp_str) &&
                       SMTCell::getAssigned(tmp_str) == 1) {
              SMTCell::setVar_wo_cnt(tmp_str, 0);
              cnt_true++;
            }

            // AGR FLAG
            int currCol = vCoord_BL->col_;
            int distToPOICol = abs(currCol - col);
            int realEOL = EOL_L_ADJ_Parameter - SMTCell::row_idx_to_real(1);

            while (distToPOICol <= realEOL) {
              std::string tmp_str =
                  fmt::format("GR_V_{}", vCoord_BL->getVName());
              if (!SMTCell::ifAssigned(tmp_str)) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog,
                             "d       => Adding Forbidden EOL LR : {} \n",
                             tmp_str);
                }
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssigned(tmp_str) &&
                         SMTCell::getAssigned(tmp_str) == 1) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }
              vCoord_BL = SMTCell::getVertex((*vCoord_BL))->getLeftADJ();
              if (vCoord_BL->ifValid() == false) {
                break;
              } else {
                currCol = vCoord_BL->col_;
                distToPOICol = abs(currCol - col);
              }
            }

            if (cnt_true > 0) {
              if (cnt_true > 1) {
                std::cerr << "\n[ERROR] TIP2TIP : more than one G "
                             "Variables are true!!!\n";
                exit(1);
              } else {
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::assignTrueVar(tmp_var[i], 0, true);
                }
              }
            } else {
              if (cnt_var > 1) {
                SMTCell::writeConstraint(fmt::format("(assert ((_ at-most 1)"));
                for (auto s : tmp_var) {
                  SMTCell::writeConstraint(fmt::format(" {}", s));
                  SMTCell::cnt("l", 3);
                }
                SMTCell::writeConstraint(fmt::format("))\n"));
                SMTCell::cnt("c", 3);
              }
            }
          }
        }
      }
    }
  }
}

/**
 * @note
 *                                ║    ║    ║
 *                            ----X----X----X---- <--- B Direction Checking
 *                                ║    |    ║
 * BL Direction Checking ---> ----X----+----X---- <--- BR Direction Checking
 *                                |    |    |
 *                            ----+----X----+----
 *                                |    ▒    |
 */
// 04/20/2023 LEGACY BUG FOUND : Contradicting Metal Direction checking
void DesignRuleWriter::write_EOL_AGR_BF_tip_helper(int EOL_M1_B_ADJ_Parameter,
                                                   int EOL_M1_B_Parameter,
                                                   int EOL_M3_B_ADJ_Parameter,
                                                   int EOL_M3_B_Parameter,
                                                   int doublePowerRail,
                                                   FILE *drlog) {

  // for (int metal = SMTCell::getFirstRoutingLayer();
  //      metal <= SMTCell::getNumMetalLayer(); metal++) { // no DR on M1
  //   if (SMTCell::ifVertMetal(metal)) {
  //     for (int row = 0; row <= SMTCell::getNumTrackH() - 4; row++) {
  //       for (int col = 0; col <= SMTCell::getLastColIdx(); col++) {
  for (int metal = 1; metal <= SMTCell::getNumMetalLayer(); metal++) {
    if (SMTCell::ifVertMetal(metal)) {
      for (int row_idx = 0;
           row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 4; row_idx++) {
        for (int col_idx = 0;
             col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
          // retrieve row/col
          int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
          int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);

          if (metal == 1 && SMTCell::ifSDCol_AGR(metal, col)) {
            continue;
          }

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d     EOL_BF: METAL {} ROW {} COL {} \n", metal,
                       row, col);
          }
          // if (metal > 1 && SMTCell::ifVertMetal(metal) &&
          //     SMTCell::ifSDCol(col)) {
          //   continue;
          // }
          // Triplet *vCoord_BL =
          //     SMTCell::getVertex(metal, row, col)->getBackLeftADJ();
          std::shared_ptr<Triplet> vCoord_BL =
              SMTCell::getVertex(metal, row, col)->getBackLeftADJ();

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        vCoord_BL: {} \n",
                       vCoord_BL->getVName());
          }

          int EOL_B_ADJ_Parameter = 0;
          int EOL_B_Parameter = 0;

          // Layer-specific EOL Parameter
          if (metal == 1) {
            EOL_B_ADJ_Parameter = EOL_M1_B_ADJ_Parameter;
            EOL_B_Parameter = EOL_M1_B_Parameter;
          } else if (metal == 3) {
            EOL_B_ADJ_Parameter = EOL_M3_B_ADJ_Parameter;
            EOL_B_Parameter = EOL_M3_B_Parameter;
          } else {
            fmt::print("[ERROR] Something Wrong in EOL for M{} \n", metal);
          }

          // BL Direction Checking
          // if (vCoord_BL->ifValid() && metal >= 2 &&
          //     SMTCell::ifHorzMetal(metal) && EOL_Parameter >= 2) {
          // AGR FLAG: Fix the bug of checking the metal direction. EOL should
          // be at least the metal pitch of the current vertical layer
          if (vCoord_BL->ifValid() &&
              EOL_B_ADJ_Parameter > SMTCell::getMetalPitch(metal)) {
            int newRow_idx = row_idx + 1;
            // if (doublePowerRail == 1 &&
            //     (newRow % static_cast<int>((SMTCell::getTrackEachRow() + 1))
            //     ==
            //      0) &&
            //     EOL_Parameter == 2) {
            if (doublePowerRail == 1 &&
                ((newRow_idx) %
                     static_cast<int>((SMTCell::getTrackEachRow() + 1)) ==
                 0) &&
                EOL_B_ADJ_Parameter == SMTCell::getMetalPitch(metal) +
                                           SMTCell::row_idx_to_real(1)) {
              // Skip the BR Direction
            } else {
              std::vector<std::string> tmp_var;
              int cnt_var = 0;
              int cnt_true = 0;
              std::string tmp_str = fmt::format(
                  "GB_V_{}", SMTCell::getVertexName(metal, row, col));
              if (!SMTCell::ifAssigned(tmp_str)) {
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssigned(tmp_str) &&
                         SMTCell::getAssigned(tmp_str) == 1) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }

              // int powerRailFlag = 0;
              // int powerRailIndex = 0;

              // AGR FLAG
              int currCol = vCoord_BL->col_;
              int distToPOIRow = SMTCell::row_idx_to_real(1);
              int realEOL = EOL_B_ADJ_Parameter - abs(currCol - col);

              // for (int eolIndex = 1; eolIndex <= EOL_Parameter - 1;
              //      eolIndex++) {
              while (distToPOIRow <= realEOL) {
                // DPR FLAG
                // powerRailIndex =
                //     std::ceil(eolIndex / (SMTCell::getTrackEachRow() + 2));
                // if (doublePowerRail == 1 &&
                //     (newRow_idx %
                //          static_cast<int>((SMTCell::getTrackEachRow() + 1))
                //          ==
                //      0) &&
                //     powerRailFlag < powerRailIndex) {
                //   powerRailFlag++;
                //   continue;
                // }
                std::string tmp_str =
                    fmt::format("GF_V_{}", vCoord_BL->getVName());
                if (!SMTCell::ifAssigned(tmp_str)) {
                  // DEBUG MODE
                  if (SMTCell::DEBUG_MODE_) {
                    fmt::print(drlog,
                               "d       => Adding Forbidden EOL BF : {} \n",
                               tmp_str);
                  }
                  tmp_var.push_back(tmp_str);
                  SMTCell::setVar(tmp_str, 2);
                  cnt_var++;
                } else if (SMTCell::ifAssigned(tmp_str) &&
                           SMTCell::getAssigned(tmp_str) == 1) {
                  SMTCell::setVar_wo_cnt(tmp_str, 0);
                  cnt_true++;
                }
                // if (eolIndex < (EOL_Parameter - 1)) {
                vCoord_BL = SMTCell::getVertex((*vCoord_BL))->getBackADJ();
                newRow_idx++;
                if (vCoord_BL->ifValid() == false) {
                  break;
                } else {
                  distToPOIRow += SMTCell::row_idx_to_real(1);
                }
                // }
              }
              if (cnt_true > 0) {
                if (cnt_true > 1) {
                  std::cerr << "\n[ERROR] TIP2TIP : more than one G "
                               "Variables are true!!!\n";
                  exit(1);
                } else {
                  for (std::size_t i = 0; i < tmp_var.size(); i++) {
                    SMTCell::assignTrueVar(tmp_var[i], 0, true);
                  }
                }
              } else {
                if (cnt_var > 1) {
                  SMTCell::writeConstraint(
                      fmt::format("(assert ((_ at-most 1)"));
                  for (auto s : tmp_var) {
                    SMTCell::writeConstraint(fmt::format(" {}", s));
                    SMTCell::cnt("l", 3);
                  }
                  SMTCell::writeConstraint(fmt::format("))\n"));
                  SMTCell::cnt("c", 3);
                }
              }
            }
          }

          // B Direction Checking
          // Triplet *vCoord_B = SMTCell::getVertex(metal, row, col)->getBackADJ();
          std::shared_ptr<Triplet> vCoord_B =
              SMTCell::getVertex(metal, row, col)->getBackADJ();

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        vCoord_B: {} \n", vCoord_B->getVName());
          }

          if (vCoord_B->ifValid()) {
            int newRow_idx = row_idx + 1;
            if (doublePowerRail == 1 &&
                (newRow_idx %
                     static_cast<int>((SMTCell::getTrackEachRow() + 1)) ==
                 0) &&
                EOL_B_Parameter == SMTCell::row_idx_to_real(1)) {
              // Skip the B Direction
            } else {
              std::vector<std::string> tmp_var;
              int cnt_var = 0;
              int cnt_true = 0;
              std::string tmp_str = fmt::format(
                  "GB_V_{}", SMTCell::getVertexName(metal, row, col));
              if (!SMTCell::ifAssigned(tmp_str)) {
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssigned(tmp_str) &&
                         SMTCell::getAssigned(tmp_str) == 1) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }

              // int powerRailFlag = 0;
              // int powerRailIndex = 0;

              // AGR FLAG
              // int currCol = vCoord_B->col_;
              int distToPOIRow = SMTCell::row_idx_to_real(1);
              int realEOL = EOL_B_Parameter;

              while (distToPOIRow <= realEOL) {
                // for (int eolIndex = 1; eolIndex <= EOL_Parameter; eolIndex++)
                // { DPR FLAG powerRailIndex =
                //     std::ceil(eolIndex / (SMTCell::getTrackEachRow() + 2));
                // if (doublePowerRail == 1 &&
                //     (newRow_idx %
                //          static_cast<int>((SMTCell::getTrackEachRow() + 1))
                //          ==
                //      0) &&
                //     powerRailFlag < powerRailIndex) {
                //   powerRailFlag++;
                //   continue;
                // }
                std::string tmp_str =
                    fmt::format("GF_V_{}", vCoord_B->getVName());
                if (!SMTCell::ifAssigned(tmp_str)) {
                  tmp_var.push_back(tmp_str);
                  SMTCell::setVar(tmp_str, 2);
                  cnt_var++;
                } else if (SMTCell::ifAssigned(tmp_str) &&
                           SMTCell::getAssigned(tmp_str) == 1) {
                  SMTCell::setVar_wo_cnt(tmp_str, 0);
                  cnt_true++;
                }
                // if (eolIndex < EOL_Parameter) {
                vCoord_B = SMTCell::getVertex((*vCoord_B))->getBackADJ();
                newRow_idx++;
                if (vCoord_B->ifValid() == false) {
                  break;
                } else {
                  distToPOIRow += SMTCell::row_idx_to_real(1);
                }
                // }
              }
              if (cnt_true > 0) {
                if (cnt_true > 1) {
                  std::cerr << "\n[ERROR] TIP2TIP : more than one G "
                               "Variables are true!!!\n";
                  exit(1);
                } else {
                  for (std::size_t i = 0; i < tmp_var.size(); i++) {
                    SMTCell::assignTrueVar(tmp_var[i], 0, true);
                  }
                }
              } else {
                if (cnt_var > 1) {
                  SMTCell::writeConstraint(
                      fmt::format("(assert ((_ at-most 1)"));
                  for (auto s : tmp_var) {
                    SMTCell::writeConstraint(fmt::format(" {}", s));
                    SMTCell::cnt("l", 3);
                  }
                  SMTCell::writeConstraint(fmt::format("))\n"));
                  SMTCell::cnt("c", 3);
                }
              }
            }
          }

          // BR Direction Checking
          // Triplet *vCoord_BR =
          //     SMTCell::getVertex(metal, row, col)->getBackRightADJ();
          std::shared_ptr<Triplet> vCoord_BR =
              SMTCell::getVertex(metal, row, col)->getBackRightADJ();

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        vCoord_BR: {} \n",
                       vCoord_BR->getVName());
          }

          // if (vCoord_BR->ifValid() && metal >= 2 &&
          //     SMTCell::ifHorzMetal(metal) && EOL_Parameter >= 2) {
          if (vCoord_BR->ifValid() &&
              EOL_B_ADJ_Parameter > SMTCell::getMetalPitch(metal)) {
            int newRow_idx = row_idx + 1;
            if (doublePowerRail == 1 &&
                (newRow_idx %
                     static_cast<int>((SMTCell::getTrackEachRow() + 1)) ==
                 0) &&
                EOL_B_ADJ_Parameter == 2) {
              // Skip the BR Direction
            } else {
              std::vector<std::string> tmp_var;
              int cnt_var = 0;
              int cnt_true = 0;
              std::string tmp_str = fmt::format(
                  "GB_V_{}", SMTCell::getVertexName(metal, row, col));
              if (!SMTCell::ifAssigned(tmp_str)) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog,
                             "d       => Adding Forbidden EOL BF : {} \n",
                             tmp_str);
                }
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssigned(tmp_str) &&
                         SMTCell::getAssigned(tmp_str) == 1) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }

              // int powerRailFlag = 0;
              // int powerRailIndex = 0;

              // int currCol = vCoord_BR->col_;

              for (int eolIndex = 1; eolIndex <= EOL_B_ADJ_Parameter - 1;
                   eolIndex++) {
                // DPR FLAG
                // powerRailIndex =
                //     std::ceil(eolIndex / (SMTCell::getTrackEachRow() + 2));
                // if (doublePowerRail == 1 &&
                //     (newRow_idx %
                //          static_cast<int>((SMTCell::getTrackEachRow() + 1))
                //          ==
                //      0) &&
                //     powerRailFlag < powerRailIndex) {
                //   powerRailFlag++;
                //   continue;
                // }
                std::string tmp_str =
                    fmt::format("GF_V_{}", vCoord_BR->getVName());
                if (!SMTCell::ifAssigned(tmp_str)) {
                  // DEBUG MODE
                  if (SMTCell::DEBUG_MODE_) {
                    fmt::print(drlog,
                               "d       => Adding Forbidden EOL BF : {} \n",
                               tmp_str);
                  }
                  tmp_var.push_back(tmp_str);
                  SMTCell::setVar(tmp_str, 2);
                  cnt_var++;
                } else if (SMTCell::ifAssigned(tmp_str) &&
                           SMTCell::getAssigned(tmp_str) == 1) {
                  SMTCell::setVar_wo_cnt(tmp_str, 0);
                  cnt_true++;
                }
                if (eolIndex < (EOL_B_ADJ_Parameter - 1)) {
                  vCoord_BR = SMTCell::getVertex((*vCoord_BR))->getBackADJ();
                  newRow_idx++;
                  if (vCoord_BR->ifValid() == false) {
                    break;
                  }
                }
              }
              if (cnt_true > 0) {
                if (cnt_true > 1) {
                  std::cerr << "\n[ERROR] TIP2TIP : more than one G "
                               "Variables are true!!!\n";
                  exit(1);
                } else {
                  for (std::size_t i = 0; i < tmp_var.size(); i++) {
                    SMTCell::assignTrueVar(tmp_var[i], 0, true);
                  }
                }
              } else {
                if (cnt_var > 1) {
                  SMTCell::writeConstraint(
                      fmt::format("(assert ((_ at-most 1)"));
                  for (auto s : tmp_var) {
                    SMTCell::writeConstraint(fmt::format(" {}", s));
                    SMTCell::cnt("l", 3);
                  }
                  SMTCell::writeConstraint(fmt::format("))\n"));
                  SMTCell::cnt("c", 3);
                }
              }
            }
          }
        }
      }
    }
  }
}

/**
 * @note
 *                                |    ▒    |
 *                            ----+----X----+---- <--- F Direction Checking
 *                                |    |    |
 * FL Direction Checking ---> ----X----+----X---- <--- FR Direction Checking
 *                                ║    |    ║
 *                            ----X----X----X----
 *                                ║    ║    ║
 */
void DesignRuleWriter::write_EOL_AGR_FB_tip_helper(int EOL_M1_B_ADJ_Parameter,
                                                   int EOL_M1_B_Parameter,
                                                   int EOL_M3_B_ADJ_Parameter,
                                                   int EOL_M3_B_Parameter,
                                                   int doublePowerRail,
                                                   FILE *drlog) {
  for (int metal = 1; metal <= SMTCell::getNumMetalLayer();
       metal++) { // no DR on M1
    if (SMTCell::ifVertMetal(metal)) {
      for (int row_idx = 0;
           row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3; row_idx++) {
        for (int col_idx = 0;
             col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
          // retrieve row/col
          int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
          int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);

          if (metal == 1 && SMTCell::ifSDCol_AGR(metal, col)) {
            continue;
          }

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d     EOL_FB: METAL {} ROW {} COL {} \n", metal,
                       row, col);
          }

          // if (metal > 1 && SMTCell::ifVertMetal(metal) &&
          //     SMTCell::ifSDCol(col)) {
          //   continue;
          // }
          // std::shared_ptr<Triplet> vCoord = std::make_shared<Triplet>(metal,
          // row, col);
          // Triplet *vCoord_FL =
          //     SMTCell::getVertex(metal, row, col)->getFrontLeftADJ();
          std::shared_ptr<Triplet> vCoord_FL =
              SMTCell::getVertex(metal, row, col)->getFrontLeftADJ();

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        vCoord_FL: {} \n",
                       vCoord_FL->getVName());
          }

          // Layer-specific EOL Parameter
          int EOL_B_ADJ_Parameter = 0;
          int EOL_B_Parameter = 0;

          // FL Direction
          if (vCoord_FL->ifValid() && metal >= 2 &&
              SMTCell::ifHorzMetal(metal) && EOL_B_ADJ_Parameter >= 2) {
            int newRow = row - 1;
            if (doublePowerRail == 1 &&
                (newRow % static_cast<int>((SMTCell::getTrackEachRow() + 1)) ==
                 0) &&
                EOL_B_ADJ_Parameter == 2) {
              // Skip the BR Direction
            } else {
              std::vector<std::string> tmp_var;
              int cnt_var = 0;
              int cnt_true = 0;
              std::string tmp_str = fmt::format(
                  "GF_V_{}", SMTCell::getVertexName(metal, row, col));
              if (!SMTCell::ifAssigned(tmp_str)) {
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssigned(tmp_str) &&
                         SMTCell::getAssigned(tmp_str) == 1) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }

              // int powerRailFlag = 0;
              // int powerRailIndex = 0;

              // AGR FLAG
              int currCol = vCoord_FL->col_;
              int distToPOIRow = SMTCell::row_idx_to_real(1);
              int realEOL = EOL_B_ADJ_Parameter - abs(currCol - col);

              // for (int eolIndex = 1; eolIndex <= EOL_Parameter - 1;
              //  eolIndex++) {
              while (distToPOIRow <= realEOL) {
                // DPR FLAG
                // powerRailIndex =
                //     std::ceil(eolIndex / (SMTCell::getTrackEachRow() + 2));
                // if (doublePowerRail == 1 &&
                //     (newRow %
                //          static_cast<int>((SMTCell::getTrackEachRow() + 1))
                //          ==
                //      0) &&
                //     powerRailFlag < powerRailIndex) {
                //   powerRailFlag++;
                //   continue;
                // }
                std::string tmp_str =
                    fmt::format("GB_V_{}", vCoord_FL->getVName());
                if (!SMTCell::ifAssigned(tmp_str)) {
                  // DEBUG MODE
                  if (SMTCell::DEBUG_MODE_) {
                    fmt::print(drlog,
                               "d       => Adding Forbidden EOL FB : {} \n",
                               tmp_str);
                  }
                  tmp_var.push_back(tmp_str);
                  SMTCell::setVar(tmp_str, 2);
                  cnt_var++;
                } else if (SMTCell::ifAssigned(tmp_str) &&
                           SMTCell::getAssigned(tmp_str) == 1) {
                  SMTCell::setVar_wo_cnt(tmp_str, 0);
                  cnt_true++;
                }
                // if (eolIndex < (EOL_Parameter - 1)) {
                vCoord_FL = SMTCell::getVertex((*vCoord_FL))->getFrontADJ();
                newRow--;
                if (vCoord_FL->ifValid() == false) {
                  break;
                } else {
                  distToPOIRow += SMTCell::row_idx_to_real(1);
                }
                // }
              }
              if (cnt_true > 0) {
                if (cnt_true > 1) {
                  std::cerr << "\n[ERROR] TIP2TIP : more than one G "
                               "Variables are true!!!\n";
                  exit(1);
                } else {
                  for (std::size_t i = 0; i < tmp_var.size(); i++) {
                    SMTCell::assignTrueVar(tmp_var[i], 0, true);
                  }
                }
              } else {
                if (cnt_var > 1) {
                  SMTCell::writeConstraint(
                      fmt::format("(assert ((_ at-most 1)"));
                  for (auto s : tmp_var) {
                    SMTCell::writeConstraint(fmt::format(" {}", s));
                    SMTCell::cnt("l", 3);
                  }
                  SMTCell::writeConstraint(fmt::format("))\n"));
                  SMTCell::cnt("c", 3);
                }
              }
            }
          }

          // F Direction Checking
          // Triplet *vCoord_F =
          //     SMTCell::getVertex(metal, row, col)->getFrontADJ();
          std::shared_ptr<Triplet> vCoord_F =
              SMTCell::getVertex(metal, row, col)->getFrontADJ();
          if (vCoord_F->ifValid()) {
            int newRow = row - 1;
            if (doublePowerRail == 1 &&
                (newRow % static_cast<int>((SMTCell::getTrackEachRow() + 1)) ==
                 0) &&
                EOL_B_Parameter == 1) {
              // Skip the F Direction
            } else {
              std::vector<std::string> tmp_var;
              int cnt_var = 0;
              int cnt_true = 0;
              std::string tmp_str = fmt::format(
                  "GF_V_{}", SMTCell::getVertexName(metal, row, col));
              if (!SMTCell::ifAssigned(tmp_str)) {
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssigned(tmp_str) &&
                         SMTCell::getAssigned(tmp_str) == 1) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }

              // int powerRailFlag = 0;
              // int powerRailIndex = 0;

              // AGR FLAG : enforcing EOL rule only on routing layers
              // int eolMax = 0;
              // if (metal == 1) {
              //   eolMax = EOL_Parameter - 1;
              // } else {
              //   eolMax = EOL_Parameter;
              // }

              // AGR FLAG
              // int currCol = vCoord_F->col_;
              int distToPOIRow = SMTCell::row_idx_to_real(1);
              int realEOL = EOL_B_Parameter;

              // for (int eolIndex = 1; eolIndex <= eolMax; eolIndex++) {
              // DPR FLAG
              // powerRailIndex =
              //     std::ceil(eolIndex / (SMTCell::getTrackEachRow() + 2));
              // if (doublePowerRail == 1 &&
              //     (newRow %
              //          static_cast<int>((SMTCell::getTrackEachRow() + 1))
              //          ==
              //      0) &&
              //     powerRailFlag < powerRailIndex) {
              //   powerRailFlag++;
              //   continue;
              // }
              while (distToPOIRow <= realEOL) {
                std::string tmp_str =
                    fmt::format("GB_V_{}", vCoord_F->getVName());
                if (!SMTCell::ifAssigned(tmp_str)) {
                  // DEBUG MODE
                  if (SMTCell::DEBUG_MODE_) {
                    fmt::print(drlog,
                               "d       => Adding Forbidden EOL FB : {} \n",
                               tmp_str);
                  }
                  tmp_var.push_back(tmp_str);
                  SMTCell::setVar(tmp_str, 2);
                  cnt_var++;
                } else if (SMTCell::ifAssigned(tmp_str) &&
                           SMTCell::getAssigned(tmp_str) == 1) {
                  SMTCell::setVar_wo_cnt(tmp_str, 0);
                  cnt_true++;
                }
                // if (eolIndex < eolMax) {
                vCoord_F = SMTCell::getVertex((*vCoord_F))->getFrontADJ();
                newRow--;
                if (vCoord_F->ifValid() == false) {
                  break;
                } else {
                  distToPOIRow += SMTCell::row_idx_to_real(1);
                }
                // }
              }
              if (cnt_true > 0) {
                if (cnt_true > 1) {
                  std::cerr << "\n[ERROR] TIP2TIP : more than one G "
                               "Variables are true!!!\n";
                  exit(1);
                } else {
                  for (std::size_t i = 0; i < tmp_var.size(); i++) {
                    SMTCell::assignTrueVar(tmp_var[i], 0, true);
                  }
                }
              } else {
                if (cnt_var > 1) {
                  SMTCell::writeConstraint(
                      fmt::format("(assert ((_ at-most 1)"));
                  for (auto s : tmp_var) {
                    SMTCell::writeConstraint(fmt::format(" {}", s));
                    SMTCell::cnt("l", 3);
                  }
                  SMTCell::writeConstraint(fmt::format("))\n"));
                  SMTCell::cnt("c", 3);
                }
              }
            }
          }

          // FR Direction Checking
          // Triplet *vCoord_FR =
          //     SMTCell::getVertex(metal, row, col)->getFrontRightADJ();
          std::shared_ptr<Triplet> vCoord_FR =
              SMTCell::getVertex(metal, row, col)->getFrontRightADJ();
          if (vCoord_FR->ifValid() && metal >= 2 &&
              SMTCell::ifHorzMetal(metal) && EOL_B_ADJ_Parameter >= 2) {
            int newRow = row - 1;
            if (doublePowerRail == 1 &&
                (newRow % static_cast<int>((SMTCell::getTrackEachRow() + 1)) ==
                 0) &&
                EOL_B_ADJ_Parameter == 2) {
              // Skip the BR Direction
            } else {
              std::vector<std::string> tmp_var;
              int cnt_var = 0;
              int cnt_true = 0;
              std::string tmp_str = fmt::format(
                  "GF_V_{}", SMTCell::getVertexName(metal, row, col));
              if (!SMTCell::ifAssigned(tmp_str)) {
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssigned(tmp_str) &&
                         SMTCell::getAssigned(tmp_str) == 1) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }

              // int powerRailFlag = 0;
              // int powerRailIndex = 0;

              // AGR FLAG
              int currCol = vCoord_FR->col_;
              int distToPOIRow = SMTCell::row_idx_to_real(1);
              int realEOL = EOL_B_ADJ_Parameter - abs(currCol - col);

              // for (int eolIndex = 1; eolIndex <= EOL_Parameter - 1;
              //      eolIndex++) {
              while (distToPOIRow <= realEOL) {
                // DPR FLAG
                // powerRailIndex =
                //     std::ceil(eolIndex / (SMTCell::getTrackEachRow() + 2));
                // if (doublePowerRail == 1 &&
                //     (newRow %
                //          static_cast<int>((SMTCell::getTrackEachRow() + 1))
                //          ==
                //      0) &&
                //     powerRailFlag < powerRailIndex) {
                //   powerRailFlag++;
                //   continue;
                // }
                std::string tmp_str =
                    fmt::format("GB_V_{}", vCoord_FR->getVName());
                if (!SMTCell::ifAssigned(tmp_str)) {
                  // DEBUG MODE
                  if (SMTCell::DEBUG_MODE_) {
                    fmt::print(drlog,
                               "d       => Adding Forbidden EOL FB : {} \n",
                               tmp_str);
                  }
                  tmp_var.push_back(tmp_str);
                  SMTCell::setVar(tmp_str, 2);
                  cnt_var++;
                } else if (SMTCell::ifAssigned(tmp_str) &&
                           SMTCell::getAssigned(tmp_str) == 1) {
                  SMTCell::setVar_wo_cnt(tmp_str, 0);
                  cnt_true++;
                }
                // if (eolIndex < (EOL_Parameter - 1)) {
                vCoord_FR = SMTCell::getVertex((*vCoord_FR))->getFrontADJ();
                newRow--;
                if (vCoord_FR->ifValid() == false) {
                  break;
                } else {
                  distToPOIRow += SMTCell::row_idx_to_real(1);
                }
                // }
              }
              if (cnt_true > 0) {
                if (cnt_true > 1) {
                  std::cerr << "\n[ERROR] TIP2TIP : more than one G "
                               "Variables are true!!!\n";
                  exit(1);
                } else {
                  for (std::size_t i = 0; i < tmp_var.size(); i++) {
                    SMTCell::assignTrueVar(tmp_var[i], 0, true);
                  }
                }
              } else {
                if (cnt_var > 1) {
                  SMTCell::writeConstraint(
                      fmt::format("(assert ((_ at-most 1)"));
                  for (auto s : tmp_var) {
                    SMTCell::writeConstraint(fmt::format(" {}", s));
                    SMTCell::cnt("l", 3);
                  }
                  SMTCell::writeConstraint(fmt::format("))\n"));
                  SMTCell::cnt("c", 3);
                }
              }
            }
          }
        }
      }
    }
  }
}

// VR
void DesignRuleWriter::write_VR_AGR_rule(float VR_M1M2_Parameter,
                                         float VR_M2M3_Parameter,
                                         float VR_M3M4_Parameter,
                                         int doublePowerRail, FILE *drlog) {
  SMTCell::writeConstraint(";9. Via-to-Via Spacing Rule\n");
  fmt::print(drlog, "9. Via-to-Via Spacing Rule\n");

  // if (VR_Parameter == 0) {
  if (VR_M1M2_Parameter == 0 && VR_M2M3_Parameter == 0 &&
      VR_M3M4_Parameter == 0) {
    fmt::print("is disabled\n");
    SMTCell::writeConstraint(";VR is disabled\n");
  } else {
    // # VR Rule Enable /Disable
    // ### Via-to-Via Spacing Rule to prevent from having too close vias and
    // stacked vias.
    SMTCell::writeConstraint(fmt::format(
        ";VIA Rule for M1, VIA Rule is applied only for all vias\n"));
    fmt::print(drlog,
               ";VIA Rule for M1, VIA Rule is applied only for all vias\n");
    DesignRuleWriter::write_VR_AGR_M1_helper(VR_M1M2_Parameter, doublePowerRail,
                                             drlog);
    SMTCell::writeConstraint(
        fmt::format(";VIA Rule for M2~M4, VIA Rule is applied only for vias "
                    "between different nets\n"));
    fmt::print(drlog, ";VIA Rule for M2~M4, VIA Rule is applied only for vias "
                      "between different nets\n");
    DesignRuleWriter::write_VR_AGR_M2_M4_helper(
        VR_M2M3_Parameter, VR_M3M4_Parameter, doublePowerRail, drlog);
    std::cout << "have been written.\n";
  }
}

void DesignRuleWriter::write_VR_AGR_M1_helper(float VR_Parameter,
                                              int doublePowerRail,
                                              FILE *drlog) {
  for (int metal = 1; metal <= 1; metal++) {
    for (int row_idx = 0; row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3;
         row_idx++) {
      for (int col_idx = 0;
           col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
        // retrieve row
        int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
        // retrieve col
        int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);
        // if (metal > 1 && SMTCell::ifVertMetal(metal) &&
        // SMTCell::ifSDCol(col)) {
        //   continue;
        // }

        // ignore upper right corner
        if (row_idx == SMTCell::get_h_metal_numTrackH(metal) - 3 &&
            col_idx == SMTCell::get_h_metal_numTrackV(metal) - 1) {
          continue;
        }

        std::shared_ptr<Triplet> vCoord =
            std::make_shared<Triplet>(metal, row, col);

        if (SMTCell::DEBUG_MODE_) {
          fmt::print(drlog, "d1     VR: M_{}_{} \n", vCoord->getVName(),
                     SMTCell::getVertex((*vCoord))->getUpADJ()->getVName());
        }

        if (SMTCell::getVertex((*vCoord))
                ->getUpADJ()
                ->ifValid()) { // Up Neighbor, i.e., VIA from the vName vertex
          std::vector<std::string> tmp_var;
          int cnt_var = 0;
          int cnt_true = 0;
          // metal variable for the via
          std::string tmp_str = fmt::format(
              "M_{}_{}", vCoord->getVName(),
              SMTCell::getVertex((*vCoord))->getUpADJ()->getVName());
          if (!SMTCell::ifAssigned(tmp_str)) {
            tmp_var.push_back(tmp_str);
            SMTCell::setVar(tmp_str, 2);
            cnt_var++;
          } else if (SMTCell::ifAssignedTrue(tmp_str)) {
            SMTCell::setVar_wo_cnt(tmp_str, 0);
            cnt_true++;
          }

          // iterate from current via
          for (int newRow_idx = row_idx;
               newRow_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3;
               newRow_idx++) {
            for (int newCol_idx = col_idx;
                 newCol_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1;
                 newCol_idx++) {
              if (SMTCell::DEBUG_MODE_) {
                fmt::print(drlog, "d1     im in \n");
              }
              // for (int newRow = row; newRow <= SMTCell::getNumTrackH() - 3;
              //      newRow++) {
              //   for (int newCol = col; newCol <= SMTCell::getNumTrackV() - 1;
              //        newCol++) {
              // retrieve row
              int newRow = SMTCell::get_h_metal_row_by_idx(metal, newRow_idx);
              // retrieve col
              int newCol = SMTCell::get_h_metal_col_by_idx(metal, newCol_idx);

              int distCol = newCol - col;
              int distRow = (newRow - row) * SMTCell::row_idx_to_real(1);
              // Check power rail between newRow and row. (Double Power Rail
              // Rule Applying)
              if (doublePowerRail == 1 &&
                  floor(newRow / (SMTCell::getTrackEachRow() + 1)) !=
                      floor(row / (SMTCell::getTrackEachRow() + 1))) {
                distRow++;
              }

              if (SMTCell::DEBUG_MODE_) {
                fmt::print(drlog, "d1.1     im in \n");
              }

              if (newRow == row &&
                  newCol_idx == col_idx) { // Skip Initial Value
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog, "d       => Skip Initial Value\n");
                }
                continue;
              }

              if (SMTCell::DEBUG_MODE_) {
                fmt::print(drlog, "d1.2     im in \n");
              }

              if ((distCol * distCol + distRow * distRow) >
                  (VR_Parameter * VR_Parameter)) { // Check the Via Distance
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog, "d       => Fullfilled VR Distance\n");
                }
                break;
              }

              if (SMTCell::DEBUG_MODE_) {
                fmt::print(drlog, "d1.3     im in \n");
              }
              // Need to consider the Power rail distance by 2 like EOL rule
              // Triplet *neighborCoord = new Triplet();
              // std::memcpy(neighborCoord, vCoord, sizeof(Triplet));
              // use copy constructor
              // Triplet *neighborCoord = new Triplet(*vCoord);
              std::shared_ptr<Triplet> neighborCoord =
                  std::make_shared<Triplet>(*vCoord);
              // if distCol > 0, then move to the right
              while (distCol > 0) {
                // AGR FLAG : only consider M1 Pitch
                distCol -= SMTCell::getMetalPitch(metal);
                // right most neighbor
                neighborCoord =
                    SMTCell::getVertex((*neighborCoord))->getRightADJ();
                // if neighborCoord is not valid (at boundary), then break
                if (neighborCoord->ifValid() == false) {
                  if (SMTCell::DEBUG_MODE_) {
                    fmt::print(
                        drlog,
                        "d       => neighborCoord not valid! Skip VR Rule\n");
                  }
                  break;
                }
              }

              // BUG FLAG : neighborCoord could be null
              if (neighborCoord->ifValid() == false) {
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog,
                             "d       => neighborCoord null! Skip VR Rule\n");
                }
                break;
              }

              if (SMTCell::DEBUG_MODE_) {
                fmt::print(drlog, "d2     im in \n");
              }

              // only used for double power rail
              int currentRow = row;
              int FlagforSecond = 0;
              while (distRow > 0) {
                distRow -= SMTCell::row_idx_to_real(1);
                currentRow++; // ignore the current row
                // Double Power Rail Effective Flag Code
                if (doublePowerRail == 1 &&
                    currentRow % static_cast<int>(
                                     (SMTCell::getTrackEachRow() + 1)) ==
                        0 &&
                    FlagforSecond == 0) {
                  FlagforSecond = 1;
                  currentRow--;
                  if (SMTCell::DEBUG_MODE_) {
                    fmt::print(drlog,
                               "d       => Double Power Rail! Skip VR Rule\n");
                  }
                  continue;
                }
                FlagforSecond = 0;
                // Back (up) most neighbor
                neighborCoord =
                    SMTCell::getVertex((*neighborCoord))->getBackADJ();
                if (neighborCoord->ifValid() == false) {
                  if (SMTCell::DEBUG_MODE_) {
                    fmt::print(drlog,
                               "d       => neighborCoord null! Skip VR Rule\n");
                  }
                  break;
                }
              }

              // Triplet *neighborUpCoord;
              std::shared_ptr<Triplet> neighborUpCoord;

              if (neighborCoord->ifValid()) {
                neighborUpCoord =
                    SMTCell::getVertex((*neighborCoord))->getUpADJ();
                if (!neighborUpCoord->ifValid() &&
                    SMTCell::DEBUG_MODE_ == true) {
                  // AGR FLAG : This is possible in AGR Condition
                  fmt::print(
                      drlog,
                      "\nd     [WARNING] : There is some bug in switch box "
                      "definition !\n");
                  // std::cerr << vCoord->getVName() << "\n";
                  fmt::print(drlog, "d     vCoord : {}\n", vCoord->getVName());
                  fmt::print(drlog, "d     neighborCoord : {}\n",
                             neighborCoord->getVName());
                  fmt::print(drlog, "d     neighborUpCoord : {}\n",
                             neighborUpCoord->getVName());
                  continue;
                  // exit(1);
                }
              }

              if (SMTCell::DEBUG_MODE_) {
                fmt::print(drlog, "d3     im in \n");
              }

              if (metal > 1 && SMTCell::ifVertMetal(metal) &&
                  SMTCell::ifSDCol_AGR(
                      1, SMTCell::getVertex((*neighborCoord))->getCol())) {
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(
                      drlog,
                      "d       => Metal > 1 and on SD col! Skip VR Rule\n");
                }
                continue;
              } else {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog, "d       => IM IN HERE\n");
                }
                std::string tmp_str =
                    fmt::format("M_{}_{}", neighborCoord->getVName(),
                                SMTCell::getVertex((*neighborCoord))
                                    ->getUpADJ()
                                    ->getVName());
                if (!SMTCell::ifAssigned(tmp_str)) {
                  // DEBUG MODE
                  if (SMTCell::DEBUG_MODE_) {
                    fmt::print(drlog,
                               "d       => Adding Forbidden VR M1: {} \n",
                               tmp_str);
                  }
                  tmp_var.push_back(tmp_str);
                  SMTCell::setVar(tmp_str, 2);
                  cnt_var++;
                } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                  SMTCell::setVar_wo_cnt(tmp_str, 0);
                  cnt_true++;

                  // DEBUG MODE
                  if (SMTCell::DEBUG_MODE_) {
                    fmt::print(drlog, "d       => Skip VR M1: {} \n", tmp_str);
                  }
                }
              }
            }
          }

          if (cnt_true > 0) {
            if (cnt_true > 1) {
              std::cerr << "\n[ERROR] VIA2VIA: more than one G Variables "
                           "are true!!!\n";
              exit(1);
            } else {
              for (std::size_t i = 0; i < tmp_var.size(); i++) {
                SMTCell::assignTrueVar(tmp_var[i], 0, true);
              }
            }
          } else {
            if (cnt_var > 1) {
              SMTCell::writeConstraint(fmt::format("(assert ((_ at-most 1)"));
              for (auto s : tmp_var) {
                SMTCell::writeConstraint(fmt::format(" {}", s));
                SMTCell::cnt("l", 3);
              }
              SMTCell::writeConstraint(fmt::format("))\n"));
              SMTCell::cnt("c", 3);
            }
          }
        }
        // delete vCoord;
      }
    }
  }
}
void DesignRuleWriter::write_VR_AGR_M2_M4_helper(float VR_M2M3_Parameter,
                                                 float VR_M3M4_Parameter,
                                                 int doublePowerRail,
                                                 FILE *drlog) {
  for (int netIndex = 0; netIndex < SMTCell::getNetCnt(); netIndex++) {
    for (int metal = 2; metal <= SMTCell::getNumMetalLayer() - 1; metal++) {
      for (int row_idx = 0;
           row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3; row_idx++) {
        for (int col_idx = 0;
             col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
          // if (metal > 1 && SMTCell::ifSDCol(col)) {
          //   continue;
          // }
          // retrieve row
          int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
          // retrieve col
          int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);

          // ignore upper right corner
          if (row_idx == SMTCell::get_h_metal_numTrackH(metal) - 3 &&
              col_idx == SMTCell::get_h_metal_numTrackV(metal) - 1) {
            continue;
          }

          // Layer-Specific Parameter
          float VR_Parameter = 0;

          if (metal == 2) {
            VR_Parameter = VR_M2M3_Parameter;
          } else if (metal == 3) {
            VR_Parameter = VR_M3M4_Parameter;
          }

          std::shared_ptr<Triplet> vCoord =
              std::make_shared<Triplet>(metal, row, col);

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d     VR: N{}_E_{}_{} \n",
                       SMTCell::getNet(netIndex)->getNetID(),
                       vCoord->getVName(),
                       SMTCell::getVertex((*vCoord))->getUpADJ()->getVName());
          }

          if (SMTCell::getVertex((*vCoord))->getUpADJ()->ifValid()) {
            std::vector<std::string> tmp_var;
            int cnt_var = 0;
            int cnt_true = 0;
            std::string tmp_str = fmt::format(
                "N{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                vCoord->getVName(),
                SMTCell::getVertex((*vCoord))->getUpADJ()->getVName());
            if (!SMTCell::ifAssigned(tmp_str)) {
              tmp_var.push_back(tmp_str);
              SMTCell::setVar(tmp_str, 2);
              cnt_var++;
            } else if (SMTCell::ifAssigned(tmp_str) &&
                       SMTCell::getAssigned(tmp_str) == 1) {
              SMTCell::setVar_wo_cnt(tmp_str, 0);
              cnt_true++;
            }
            // for (int newRow = row; newRow <= SMTCell::getNumTrackH() - 3;
            //      newRow++) {
            //   for (int newCol = col; newCol <= SMTCell::getNumTrackV() - 1;
            //        newCol++) {
            for (int newRow_idx = row_idx;
                 newRow_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3;
                 newRow_idx++) {
              for (int newCol_idx = col_idx;
                   newCol_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1;
                   newCol_idx++) {
                int newRow = SMTCell::get_h_metal_row_by_idx(metal, newRow_idx);
                int newCol = SMTCell::get_h_metal_col_by_idx(metal, newCol_idx);
                int distCol = newCol - col;
                int distRow = (newRow - row) * SMTCell::row_idx_to_real(1);
                // Check power rail between newRow and row. (Double Power Rail
                // Rule Applying)
                if (doublePowerRail == 1 &&
                    floor(newRow / (SMTCell::getTrackEachRow() + 1)) !=
                        floor(row / (SMTCell::getTrackEachRow() + 1))) {
                  distRow++;
                }

                if (newRow == row && newCol == col) { // Initial Value
                  if (SMTCell::DEBUG_MODE_) {
                    fmt::print(drlog, "d       => Skip Initial Value\n");
                  }
                  continue;
                }

                if ((distCol * distCol + distRow * distRow) >
                    (VR_Parameter * VR_Parameter)) { // Check the Via Distance
                  if (SMTCell::DEBUG_MODE_) {
                    fmt::print(drlog, "d       => Fullfilled VR Rule\n");
                  }
                  break;
                }
                // Need to consider the Power rail distance by 2 like EOL rule
                // Triplet *neighborCoord = new Triplet();
                // std::memcpy(neighborCoord, vCoord, sizeof(Triplet));
                // use copy constructor
                // Triplet *neighborCoord;
                std::shared_ptr<Triplet> neighborCoord;

                while (distCol > 0) {
                  distCol--;
                  // neighborCoord =
                  //     SMTCell::getVertex((*neighborCoord))->getRightADJ();
                  // free neighborCoord and get the right neighbor
                  neighborCoord = SMTCell::getVertex((*vCoord))->getRightADJ();
                  if (neighborCoord->ifValid() == false) {
                    if (SMTCell::DEBUG_MODE_) {
                      fmt::print(
                          drlog,
                          "d       => neighborCoord not valid! Skip VR Rule\n");
                    }
                    break;
                  }
                }

                // BUG FLAG : neighborCoord could be null
                if (neighborCoord->ifValid() == false) {
                  if (SMTCell::DEBUG_MODE_) {
                    fmt::print(drlog,
                               "d       => neighborCoord null! Skip VR Rule\n");
                  }
                  break;
                }

                int currentRow = row;
                int FlagforSecond = 0;
                while (distRow > 0) {
                  distRow -= SMTCell::row_idx_to_real(1);
                  currentRow++;
                  // Double Power Rail Effective Flag Code
                  if (doublePowerRail == 1 &&
                      currentRow % static_cast<int>(
                                       (SMTCell::getTrackEachRow() + 1)) ==
                          0 &&
                      FlagforSecond == 0) {
                    FlagforSecond = 1;
                    currentRow--;
                    if (SMTCell::DEBUG_MODE_) {
                      fmt::print(
                          drlog,
                          "d       => Double Power Rail! Skip VR Rule\n");
                    }
                    continue;
                  }
                  FlagforSecond = 0;
                  neighborCoord =
                      SMTCell::getVertex((*neighborCoord))->getBackADJ();
                  if (neighborCoord->ifValid() == false) {
                    if (SMTCell::DEBUG_MODE_) {
                      fmt::print(
                          drlog,
                          "d       => neighborCoord null! Skip VR Rule\n");
                    }
                    break;
                  }
                }
                // Triplet *neighborUpCoord;
                std::shared_ptr<Triplet> neighborUpCoord;
                if (neighborCoord->ifValid()) {
                  neighborUpCoord =
                      SMTCell::getVertex((*neighborCoord))->getUpADJ();
                  if (!neighborUpCoord->ifValid() &&
                      SMTCell::DEBUG_MODE_ == true) {
                    // AGR FLAG : This is possible in AGR Condition
                    fmt::print(
                        drlog,
                        "\nd     [WARNING] : There is some bug in switch box "
                        "definition !\n");
                    // std::cerr << vCoord->getVName() << "\n";
                    fmt::print(drlog, "d     vCoord : {}\n",
                               vCoord->getVName());
                    fmt::print(drlog, "d     neighborCoord : {}\n",
                               neighborCoord->getVName());
                    fmt::print(drlog, "d     neighborUpCoord : {}\n",
                               neighborUpCoord->getVName());
                    continue;
                    // exit(1);
                  }
                }
                if (metal > 1 &&
                    SMTCell::ifSDCol_AGR(
                        metal,
                        SMTCell::getVertex((*neighborCoord))->getCol())) {
                  if (SMTCell::DEBUG_MODE_) {
                    fmt::print(
                        drlog,
                        "d       => Metal > 1 and on SD col! Skip VR Rule\n");
                  }
                  continue;
                } else {
                  std::string tmp_str =
                      fmt::format("C_VIA_WO_N{}_E_{}_{}",
                                  SMTCell::getNet(netIndex)->getNetID(),
                                  neighborCoord->getVName(),
                                  SMTCell::getVertex((*neighborCoord))
                                      ->getUpADJ()
                                      ->getVName());
                  if (!SMTCell::ifAssigned(tmp_str)) {
                    // DEBUG MODE
                    if (SMTCell::DEBUG_MODE_) {
                      fmt::print(drlog,
                                 "d       => Adding Forbidden VR Net : {} \n",
                                 tmp_str);
                    }
                    tmp_var.push_back(tmp_str);
                    SMTCell::setVar(tmp_str, 2);
                    cnt_var++;
                  } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                    SMTCell::setVar_wo_cnt(tmp_str, 0);
                    cnt_true++;
                  }
                }
                // delete neighborCoord;
                // delete neighborUpCoord;
              }
            }

            if (cnt_true > 0) {
              if (cnt_true > 1) {
                std::cerr << "\n[ERROR] VIA2VIA: more than one G Variables "
                             "are true!!!\n";
                exit(1);
              } else {
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::assignTrueVar(tmp_var[i], 0, true);
                }
              }
            } else {
              if (cnt_var > 1) {
                SMTCell::writeConstraint(fmt::format("(assert ((_ at-most 1)"));
                for (auto s : tmp_var) {
                  SMTCell::writeConstraint(fmt::format(" {}", s));
                  SMTCell::cnt("l", 3);
                }
                SMTCell::writeConstraint(fmt::format("))\n"));
                SMTCell::cnt("c", 3);
              }
            }
          }
          // delete vCoord;
        }
      }
    }
  }

  for (int metal = 2; metal <= SMTCell::getNumMetalLayer() - 1; metal++) {
    for (int row_idx = 0; row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3;
         row_idx++) {
      for (int col_idx = 0;
           col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
        // retrieve row
        int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
        // retrieve col
        int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);
        // if (metal > 1 && SMTCell::ifSDCol(col)) {
        //   continue;
        // }
        // ignore upper right corner
        if (row_idx == SMTCell::get_h_metal_numTrackH(metal) - 3 &&
            col_idx == SMTCell::get_h_metal_numTrackV(metal) - 1) {
          continue;
        }

        // Layer-Specific Parameter
        float VR_Parameter = 0;

        if (metal == 2) {
          VR_Parameter = VR_M2M3_Parameter;
        } else if (metal == 3) {
          VR_Parameter = VR_M3M4_Parameter;
        }

        std::shared_ptr<Triplet> vCoord =
            std::make_shared<Triplet>(metal, row, col);

        if (SMTCell::DEBUG_MODE_) {
          fmt::print(drlog, "d     VR Coord: {}\n", vCoord->getVName());
        }

        if (SMTCell::getVertex((*vCoord))->getUpADJ()->ifValid()) {
          // for (int newRow = row; newRow <= SMTCell::getNumTrackH() - 3;
          //      newRow++) {
          //   for (int newCol = col; newCol <= SMTCell::getNumTrackV() - 1;
          //        newCol++) {
          for (int newRow_idx = row_idx;
               newRow_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3;
               newRow_idx++) {
            for (int newCol_idx = col_idx;
                 newCol_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1;
                 newCol_idx++) {
              // retrieve newRow
              int newRow = SMTCell::get_h_metal_row_by_idx(metal, newRow_idx);
              // retrieve newCol
              int newCol = SMTCell::get_h_metal_col_by_idx(metal, newCol_idx);

              int distCol = newCol - col;
              int distRow = (newRow - row) * SMTCell::row_idx_to_real(1);
              // Check power rail between newRow and row. (Double Power Rail
              // Rule Applying)
              if (doublePowerRail == 1 &&
                  floor(newRow / (SMTCell::getTrackEachRow() + 1)) !=
                      floor(row / (SMTCell::getTrackEachRow() + 1))) {
                distRow++;
              }

              if (newRow == row && newCol == col) { // Initial Value
                continue;
              }

              if ((distCol * distCol + distRow * distRow) >
                  (VR_Parameter * VR_Parameter)) { // Check the Via Distance
                break;
              }

              // Need to consider the Power rail distance by 2 like EOL rule
              // Triplet *neighborCoord = new Triplet();
              // std::memcpy(neighborCoord, vCoord, sizeof(Triplet));
              // use copy constructor
              // Triplet *neighborCoord;
              std::shared_ptr<Triplet> neighborCoord;

              while (distCol > 0) {
                distCol--;
                neighborCoord = SMTCell::getVertex((*vCoord))->getRightADJ();
                if (neighborCoord->ifValid() == false) {
                  break;
                }
              }

              // BUG FLAG : neighborCoord could be null
              if (neighborCoord->ifValid() == false) {
                break;
              }

              int currentRow = row;
              int FlagforSecond = 0;
              while (distRow > 0) {
                distRow -= SMTCell::row_idx_to_real(1);
                currentRow++;
                // Double Power Rail Effective Flag Code
                if (doublePowerRail == 1 &&
                    currentRow % static_cast<int>(
                                     (SMTCell::getTrackEachRow() + 1)) ==
                        0 &&
                    FlagforSecond == 0) {
                  FlagforSecond = 1;
                  currentRow--;
                  continue;
                }
                FlagforSecond = 0;
                neighborCoord =
                    SMTCell::getVertex((*neighborCoord))->getBackADJ();
                if (neighborCoord->ifValid() == false) {
                  break;
                }
              }

              // Triplet *neighborUpCoord;
              std::shared_ptr<Triplet> neighborUpCoord;
              if (neighborCoord->ifValid()) {
                neighborUpCoord =
                    SMTCell::getVertex((*neighborCoord))->getUpADJ();
                if (!neighborUpCoord->ifValid() &&
                    SMTCell::DEBUG_MODE_ == true) {
                  // AGR FLAG : This is possible in AGR Condition
                  fmt::print(
                      drlog,
                      "\nd     [WARNING] : There is some bug in switch box "
                      "definition !\n");
                  // std::cerr << vCoord->getVName() << "\n";
                  fmt::print(drlog, "d     vCoord : {}\n", vCoord->getVName());
                  fmt::print(drlog, "d     neighborCoord : {}\n",
                             neighborCoord->getVName());
                  fmt::print(drlog, "d     neighborUpCoord : {}\n",
                             neighborUpCoord->getVName());
                  continue;
                  // exit(1);
                }
              }
              // delete neighborUpCoord;
              if (metal > 1 &&
                  SMTCell::ifSDCol_AGR(
                      metal, SMTCell::getVertex((*neighborCoord))->getCol())) {
                // delete neighborCoord;
                continue;
              } else {
                for (int netIndex = 0; netIndex < SMTCell::getNetCnt();
                     netIndex++) {
                  std::vector<std::string> tmp_var;
                  int cnt_var = 0;
                  int cnt_true = 0;
                  std::string tmp_str_c =
                      fmt::format("C_VIA_WO_N{}_E_{}_{}",
                                  SMTCell::getNet(netIndex)->getNetID(),
                                  neighborCoord->getVName(),
                                  SMTCell::getVertex((*neighborCoord))
                                      ->getUpADJ()
                                      ->getVName());
                  if (!SMTCell::ifAssigned(tmp_str_c)) { // DSY check:
                    // "h_var" ifAssigned
                    for (int netIndex_sub = 0;
                         netIndex_sub < SMTCell::getNetCnt(); netIndex_sub++) {
                      if (netIndex_sub == netIndex) {
                        continue;
                      }
                      std::string tmp_str =
                          fmt::format("N{}_E_{}_{}",
                                      SMTCell::getNet(netIndex_sub)->getNetID(),
                                      neighborCoord->getVName(),
                                      SMTCell::getVertex((*neighborCoord))
                                          ->getUpADJ()
                                          ->getVName());
                      if (!SMTCell::ifAssigned(tmp_str)) {
                        // DEBUG MODE
                        if (SMTCell::DEBUG_MODE_) {
                          fmt::print(
                              drlog,
                              "d       => Adding Forbidden VR Net: {} \n",
                              tmp_str);
                        }
                        tmp_var.push_back(tmp_str);
                        SMTCell::setVar(tmp_str, 2);
                        cnt_var++;
                      } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                        SMTCell::setVar_wo_cnt(tmp_str, 0);
                        cnt_true++;
                      }
                    }
                  }
                  // delete neighborCoord;
                  if (cnt_true > 0) {
                    if (cnt_true > 1) {
                      std::cerr
                          << "\n[ERROR] VIA2VIA: more than one G Variables "
                             "are true!!!\n";
                      exit(1);
                    } else {
                      for (std::size_t i = 0; i < tmp_var.size(); i++) {
                        SMTCell::assignTrueVar(tmp_var[i], 0, true);
                      }
                    }
                  } else {
                    if (cnt_var > 1) {
                      SMTCell::writeConstraint(
                          fmt::format("(assert (= {} (or", tmp_str_c));
                      for (auto s : tmp_var) {
                        SMTCell::writeConstraint(fmt::format(" {}", s));
                        SMTCell::cnt("l", 3);
                      }
                      SMTCell::writeConstraint(fmt::format(")))\n"));
                      SMTCell::cnt("c", 3);
                    }
                  }
                }
              }
            }
          }
        }
        // delete vCoord;
      }
    }
  }
}

// Pin Access Rule
void DesignRuleWriter::write_pin_access_rule(int MPL_Parameter,
                                             int MAR_Parameter,
                                             int EOL_Parameter, FILE *drlog) {
  DesignRuleWriter::write_pin_access_rule_helper(MPL_Parameter, MAR_Parameter,
                                                 EOL_Parameter, drlog);
  // fmt::print("have been written.\n");

  // ### Pin Accessibility Rule : any vias to the metal3 layer should be
  // connected to metal3 in-layer
  // ### DATA STRUCTURE:  VERTEX [index] [name] [Z-pos] [Y-pos] [X-pos]
  // [Arr. of adjacent vertices]
  // ### DATA STRUCTURE:  ADJACENT_VERTICES [0:Left] [1:Right] [2:Front]
  // [3:Back] [4:Up] [5:Down] [6:FL] [7:FR] [8:BL] [9:BR]
  SMTCell::writeConstraint(";12-A. VIA enclosure for each normal vertex\n");
  DesignRuleWriter::write_pin_access_rule_via_enclosure_helper(drlog);
  SMTCell::writeConstraint("\n");

  SMTCell::writeConstraint(
      ";[DISABLED] 12-B. VIA enclosure for each normal vertex\n");

  SMTCell::writeConstraint(";12-C. VIA23 for each vertex\n");
  DesignRuleWriter::write_pin_access_rule_via23_helper(drlog);
  SMTCell::writeConstraint("\n");

  // ignore this as we are fixed to 4 metals
  // if (SMTCell::getNumMetalLayer() >= 4) {
  SMTCell::writeConstraint(";12-D. VIA34 for each vertex\n");
  // Mergable with above
  DesignRuleWriter::write_pin_access_rule_via34_helper(drlog);
  SMTCell::writeConstraint("\n");
  // }

  fmt::print("have been written.\n");
}

void DesignRuleWriter::write_pin_access_rule_helper(int MPL_Parameter,
                                                    int MAR_Parameter,
                                                    int EOL_Parameter,
                                                    FILE *drlog) {
  // AGR FLAG : use M3 pitch only
  int metal = SMTCell::getNumMetalLayer() - 1;
  std::map<std::string, int> h_tmp;

  // #Vertical
  for (auto en : SMTCell::getExtNet()) {
    int netIndex_i = SMTCell::getNetIdx(std::to_string(en.first));
    for (int col_idx = 0; col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1;
         col_idx++) {
      // retrieve col
      int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);
      std::string tmp_net_col_name = fmt::format("{}_{}", en.first, col);
      if (h_tmp.find(tmp_net_col_name) == h_tmp.end()) {
        for (int row_i_idx = 0;
             row_i_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3;
             row_i_idx++) {
          // for (int row_i = 0; row_i <= SMTCell::getNumTrackH() - 3; row_i++)
          // {

          // if (metal > 1 && SMTCell::ifVertMetal(metal) &&
          //     SMTCell::ifSDCol(col)) {
          //   continue;
          // }
          // retrieve row
          int row_i = SMTCell::get_h_metal_row_by_idx(metal, row_i_idx);
          // Triplet *vCoord_i = new Triplet(metal, row_i, col);
          std::shared_ptr<Triplet> vCoord_i =
              std::make_shared<Triplet>(metal, row_i, col);
          if (SMTCell::ifVEdgeOut(vCoord_i->getVName())) {
            for (int i : SMTCell::getVEdgeOut(vCoord_i->getVName())) {
              for (int commodityIndex = 0;
                   commodityIndex < SMTCell::getNet(netIndex_i)->getNumSinks();
                   commodityIndex++) {
                if (SMTCell::getVirtualEdge(i)->getPinName() ==
                        SMTCell::getNet(netIndex_i)
                            ->getSinks_inNet(commodityIndex) &&
                    SMTCell::getVirtualEdge(i)->getPinName() == Pin::keySON) {
                  if (h_tmp.find(tmp_net_col_name) == h_tmp.end()) {
                    h_tmp[tmp_net_col_name] = 1;
                  }

                  std::string tmp_str_i = fmt::format(
                      "N{}_E_{}_{}", SMTCell::getNet(netIndex_i)->getNetID(),
                      vCoord_i->getVName(),
                      SMTCell::getVirtualEdge(i)->getPinName());

                  // check declaration
                  if (!SMTCell::ifAssigned(tmp_str_i)) {
                    SMTCell::setVar(tmp_str_i, 2);
                    SMTCell::writeConstraint(fmt::format(
                        "(assert (ite (and (= N{}_M2_TRACK false) (= "
                        "{} true)) ((_ at-least {})",
                        en.first, tmp_str_i, MPL_Parameter));
                  } else if (SMTCell::ifAssignedTrue(tmp_str_i)) {
                    SMTCell::setVar_wo_cnt(tmp_str_i, 0);
                    SMTCell::writeConstraint(
                        fmt::format("(assert (ite (= N{}_M2_TRACK false) ((_ "
                                    "at-least {})",
                                    en.first, MPL_Parameter));
                  }

                  for (int row_j_idx = 0;
                       row_j_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3;
                       row_j_idx++) {
                    // retrieve row
                    int row_j =
                        SMTCell::get_h_metal_row_by_idx(metal, row_j_idx);
                    std::string tmp_str_j = "";

                    std::vector<std::string> tmp_var_i;
                    int cnt_var_i = 0;
                    int cnt_true_i = 0;

                    std::vector<std::string> tmp_var_j;
                    int cnt_var_j = 0;
                    int cnt_true_j = 0;

                    // Triplet *vCoord_j = new Triplet(metal, row_j, col);
                    std::shared_ptr<Triplet> vCoord_j =
                        std::make_shared<Triplet>(metal, row_j, col);
                    if (SMTCell::ifVertex((*vCoord_j)) &&
                        SMTCell::getVertex((*vCoord_j))
                            ->getFrontADJ()
                            ->ifValid()) {
                      std::string curr_front_variable =
                          fmt::format("N{}_E_{}_{}", en.first,
                                      SMTCell::getVertex((*vCoord_j))
                                          ->getFrontADJ()
                                          ->getVName(),
                                      vCoord_j->getVName());
                      // check declaration
                      if (!SMTCell::ifAssigned(curr_front_variable)) {
                        tmp_var_j.push_back(curr_front_variable);
                        SMTCell::setVar(curr_front_variable, 2);
                        cnt_var_j++;
                      } else if (SMTCell::ifAssignedTrue(curr_front_variable)) {
                        SMTCell::setVar_wo_cnt(curr_front_variable, 2);
                        cnt_true_j++;
                      }
                    }

                    if (SMTCell::ifVertex((*vCoord_j)) &&
                        SMTCell::getVertex((*vCoord_j))
                            ->getBackADJ()
                            ->ifValid()) {
                      std::string curr_back_variable = fmt::format(
                          "N{}_E_{}_{}", en.first, vCoord_j->getVName(),
                          SMTCell::getVertex((*vCoord_j))
                              ->getBackADJ()
                              ->getVName());
                      // check declaration
                      if (!SMTCell::ifAssigned(curr_back_variable)) {
                        tmp_var_j.push_back(curr_back_variable);
                        SMTCell::setVar(curr_back_variable, 2);
                        cnt_var_j++;
                      } else if (SMTCell::ifAssignedTrue(curr_back_variable)) {
                        SMTCell::setVar_wo_cnt(curr_back_variable, 2);
                        cnt_true_j++;
                      }
                    }

                    SMTCell::writeConstraint(" (or");

                    // BUG FLAG & AGR FLAG
                    // loop index has no actual usage within the loop
                    // we can use a counter instead
                    for (int marIdx = 0;
                         marIdx <=
                         floor(MAR_Parameter / SMTCell::getMetalPitch(3));
                         marIdx++) {
                      tmp_var_i.clear();
                      cnt_var_i = 0;
                      cnt_true_i = 0;

                      // # Upper Layer => Left = EOL, Right = EOL+MAR should
                      // be keepout region from other nets
                      // Triplet *vCoord_j = new Triplet(metal + 1, row_j, col);
                      std::shared_ptr<Triplet> vCoord_j =
                          std::make_shared<Triplet>(metal + 1, row_j, col);
                      // fmt::print("vCoord_j:{}\n", vCoord_j->getVName());
                      for (int netIndex_j = 0;
                           netIndex_j < SMTCell::getNetCnt(); netIndex_j++) {
                        if (SMTCell::getNet(netIndex_j)->getNetID() !=
                            std::to_string(en.first)) {
                          // Deep copy mem
                          std::shared_ptr<Triplet> prev_left_vCoord =
                              std::make_shared<Triplet>(*vCoord_j);
                          for (int eol_mar_idx = 0;
                               eol_mar_idx <= floor(EOL_Parameter /
                                                    SMTCell::getMetalPitch(3)) +
                                                  marIdx;
                               eol_mar_idx++) {
                            if (SMTCell::ifVertex((*prev_left_vCoord)) &&
                                SMTCell::getVertex((*prev_left_vCoord))
                                    ->getLeftADJ()
                                    ->ifValid()) {
                              tmp_str_j = fmt::format(
                                  "N{}_E_{}_{}",
                                  SMTCell::getNet(netIndex_j)->getNetID(),
                                  SMTCell::getVertex((*prev_left_vCoord))
                                      ->getLeftADJ()
                                      ->getVName(),
                                  prev_left_vCoord->getVName());
                              // check declaration
                              if (!SMTCell::ifAssigned(tmp_str_j)) {
                                tmp_var_i.push_back(tmp_str_j);
                                SMTCell::setVar(tmp_str_j, 2);
                                cnt_var_i++;
                              } else if (SMTCell::ifAssignedTrue(tmp_str_j)) {
                                SMTCell::setVar_wo_cnt(tmp_str_j, 2);
                                cnt_true_i++;
                              }
                              // iterate to next vertex
                              prev_left_vCoord =
                                  SMTCell::getVertex((*prev_left_vCoord))
                                      ->getLeftADJ();
                            } else {
                              continue;
                            }
                          }

                          std::shared_ptr<Triplet> prev_right_vCoord =
                              std::make_shared<Triplet>(*vCoord_j);
                          // bug found on ifValid()
                          for (int eol_mar_idx = 0;
                               eol_mar_idx <=
                               (floor(MAR_Parameter /
                                      SMTCell::getMetalPitch(3)) -
                                marIdx +
                                floor(EOL_Parameter /
                                      SMTCell::getMetalPitch(3)));
                               eol_mar_idx++) {
                            if (SMTCell::ifVertex((*prev_right_vCoord)) &&
                                SMTCell::getVertex((*prev_right_vCoord))
                                    ->getRightADJ()
                                    ->ifValid()) {
                              tmp_str_j = fmt::format(
                                  "N{}_E_{}_{}",
                                  SMTCell::getNet(netIndex_j)->getNetID(),
                                  prev_right_vCoord->getVName(),
                                  SMTCell::getVertex((*prev_right_vCoord))
                                      ->getRightADJ()
                                      ->getVName());
                              // fmt::print("tmp_str_j2 {}\n", tmp_str_j);
                              // check declaration
                              if (!SMTCell::ifAssigned(tmp_str_j)) {
                                tmp_var_i.push_back(tmp_str_j);
                                SMTCell::setVar(tmp_str_j, 2);
                                cnt_var_i++;
                              } else if (SMTCell::ifAssignedTrue(tmp_str_j)) {
                                SMTCell::setVar_wo_cnt(tmp_str_j, 2);
                                cnt_true_i++;
                              }
                              prev_right_vCoord =
                                  SMTCell::getVertex((*prev_right_vCoord))
                                      ->getRightADJ();
                            } else {
                              continue;
                            }
                          }
                        }
                      }
                      if (cnt_true_j > 0) {
                        if (cnt_true_i == 0) {
                          if (cnt_var_i == 1) {
                            SMTCell::writeConstraint(
                                fmt::format(" (= {} false)", tmp_var_i[0]));
                            SMTCell::cnt("l", 3);
                          } else if (cnt_var_i > 1) {
                            SMTCell::writeConstraint(" (and");
                            for (std::size_t m = 0; m < tmp_var_i.size(); m++) {
                              SMTCell::writeConstraint(
                                  fmt::format(" (= {} false)", tmp_var_i[m]));
                              SMTCell::cnt("l", 3);
                            }
                            SMTCell::writeConstraint(")");
                          }
                        }
                      } else if (cnt_var_j > 0) {
                        if (cnt_true_i == 0) {
                          if (cnt_var_j == 1) {
                            SMTCell::writeConstraint(
                                fmt::format(" (and (= {} true)", tmp_var_j[0]));
                            SMTCell::cnt("l", 3);
                            for (std::size_t m = 0; m < tmp_var_i.size(); m++) {
                              SMTCell::writeConstraint(
                                  fmt::format(" (= {} false)", tmp_var_i[m]));
                              SMTCell::cnt("l", 3);
                            }
                            SMTCell::writeConstraint(")");
                          } else {
                            SMTCell::writeConstraint(
                                fmt::format(" (and (or", tmp_var_j[0]));
                            for (std::size_t m = 0; m < tmp_var_j.size(); m++) {
                              SMTCell::writeConstraint(
                                  fmt::format(" (= {} true)", tmp_var_j[m]));
                              SMTCell::cnt("l", 3);
                            }
                            SMTCell::writeConstraint(")");
                            for (std::size_t m = 0; m < tmp_var_i.size(); m++) {
                              SMTCell::writeConstraint(
                                  fmt::format(" (= {} false)", tmp_var_i[m]));
                              SMTCell::cnt("l", 3);
                            }
                            SMTCell::writeConstraint(")");
                          }
                        }
                      }
                    }
                    SMTCell::writeConstraint(")");
                  }
                  if (!SMTCell::ifAssigned(tmp_str_i) ||
                      SMTCell::ifAssignedTrue(tmp_str_i)) {
                    SMTCell::writeConstraint(") (= true true)))\n");
                    SMTCell::cnt("c", 3);
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

void DesignRuleWriter::write_pin_access_rule_via_enclosure_helper(FILE *drlog) {
  for (int netIndex = 0; netIndex < SMTCell::getNetCnt(); netIndex++) {
    for (int metal = 1; metal <= SMTCell::getNumMetalLayer() - 1; metal++) {
      for (int row_idx = 0;
           row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3; row_idx++) {
        for (int col_idx = 0;
             col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
          // retrieve row
          int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
          // retrieve col
          int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);
          std::shared_ptr<Triplet> vCoord =
              std::make_shared<Triplet>(metal, row, col);
          std::shared_ptr<Triplet> vCoord_Up =
              SMTCell::getVertex((*vCoord))->getUpADJ();

          // AGR FLAG: on M2, vCoord_Up could be null
          if (vCoord_Up->ifValid() == false) {
            continue;
          }

          std::string tmp_i = fmt::format(
              "N{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
              vCoord->getVName(),
              SMTCell::getVertex((*vCoord))->getUpADJ()->getVName());
          std::string tmp_j = "";
          std::string tmp_k = "";

          // horizontal direction
          if (SMTCell::ifHorzMetal(metal)) {
            // Front Vertex
            // potential bug, this should be vCoord_Up?
            if (SMTCell::getVertex((*vCoord_Up))->getFrontADJ()->ifValid()) {
              tmp_j = fmt::format(
                  "N{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                  SMTCell::getVertex((*vCoord_Up))->getFrontADJ()->getVName(),
                  vCoord_Up->getVName());
            } else {
              tmp_j = "null";
            }

            // Back Vertex
            if (SMTCell::getVertex((*vCoord_Up))->getBackADJ()->ifValid()) {
              tmp_k = fmt::format(
                  "N{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                  vCoord_Up->getVName(),
                  SMTCell::getVertex((*vCoord_Up))->getBackADJ()->getVName());
            } else {
              tmp_k = "null";
            }
          } else {
            // vertical direction
            // Left Vertex
            if (SMTCell::getVertex((*vCoord_Up))->getLeftADJ()->ifValid()) {
              tmp_j = fmt::format(
                  "N{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                  SMTCell::getVertex((*vCoord_Up))->getLeftADJ()->getVName(),
                  vCoord_Up->getVName());
            } else {
              tmp_j = "null";
            }

            // Right Vertex
            if (SMTCell::getVertex((*vCoord_Up))->getRightADJ()->ifValid()) {
              tmp_k = fmt::format(
                  "N{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                  vCoord_Up->getVName(),
                  SMTCell::getVertex((*vCoord_Up))->getRightADJ()->getVName());
            } else {
              tmp_k = "null";
            }
          }

          if (tmp_j == "null" || SMTCell::ifAssignedFalse(tmp_j)) {
            if (tmp_k == "null" || SMTCell::ifAssignedFalse(tmp_k)) {
              SMTCell::setVar(tmp_i, 6);
              SMTCell::writeConstraint(fmt::format(
                  "(assert (ite (= {} true) (= true false) (= true true)))\n",
                  tmp_i));
              SMTCell::cnt("l", 3);
              SMTCell::cnt("c", 3);
            } else if (tmp_k != "null" && !SMTCell::ifAssigned(tmp_k)) {
              SMTCell::setVar(tmp_i, 6);
              SMTCell::setVar(tmp_k, 6);
              SMTCell::writeConstraint(fmt::format(
                  "(assert (ite (= {} true) (= {} true) (= true true)))\n",
                  tmp_i, tmp_k));
              SMTCell::cnt("l", 3);
              SMTCell::cnt("l", 3);
              SMTCell::cnt("c", 3);
            }
          } else if (tmp_j != "null" && !SMTCell::ifAssigned(tmp_j)) {
            if (tmp_k == "null" || SMTCell::ifAssignedFalse(tmp_k)) {
              SMTCell::setVar(tmp_i, 6);
              SMTCell::setVar(tmp_j, 6);
              SMTCell::writeConstraint(fmt::format(
                  "(assert (ite (= {} true) (= {} true) (= true true)))\n",
                  tmp_i, tmp_j));
              SMTCell::cnt("l", 3);
              SMTCell::cnt("l", 3);
              SMTCell::cnt("c", 3);
            } else if (tmp_k != "null" && !SMTCell::ifAssigned(tmp_k)) {
              SMTCell::setVar(tmp_i, 6);
              SMTCell::setVar(tmp_j, 6);
              SMTCell::setVar(tmp_k, 6);
              SMTCell::writeConstraint(
                  fmt::format("(assert (ite (= {} true) ((_ at-least 1) {} "
                              "{}) (= true true)))\n",
                              tmp_i, tmp_j, tmp_k));
              SMTCell::cnt("l", 3);
              SMTCell::cnt("l", 3);
              SMTCell::cnt("l", 3);
              SMTCell::cnt("c", 3);
            }
          }
        }
      }
    }
  }
}

void DesignRuleWriter::write_pin_access_rule_via23_helper(FILE *drlog) {
  for (auto en : SMTCell::getExtNet()) {
    // int netIndex = SMTCell::getNetIdx(std::to_string(en.first));
    // only subject to M3 (M1)
    int metal = 3;
    for (int row_idx = 0; row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3;
         row_idx++) {
      for (int col_idx = 0;
           col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
        // retrieve row
        int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
        // retrieve col
        int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);
        std::shared_ptr<Triplet> vCoord =
            std::make_shared<Triplet>(metal, row, col);
        if (SMTCell::ifVEdgeOut(vCoord->getVName())) {
          for (int i : SMTCell::getVEdgeOut(vCoord->getVName())) {
            std::string vName_i = SMTCell::getVirtualEdge(i)->getVName();
            std::string vName_j = SMTCell::getVirtualEdge(i)->getPinName();

            std::string tmp_i =
                fmt::format("N{}_E_{}_{}", en.first, vName_i, vName_j);
            std::string tmp_j = fmt::format(
                "N{}_E_{}_{}", en.first,
                SMTCell::getVertex((*vCoord))->getDownADJ()->getVName(),
                vCoord->getVName());

            // Front Vertex
            std::string tmp_k;
            if (SMTCell::getVertex((*vCoord))->getFrontADJ()->ifValid()) {
              tmp_k = fmt::format(
                  "N{}_E_{}_{}", en.first,
                  SMTCell::getVertex((*vCoord))->getFrontADJ()->getVName(),
                  vCoord->getVName());
            } else {
              tmp_k = fmt::format("N{}_E_{}_{}", en.first, vCoord->getVName(),
                                  "FrontEnd");
            }

            // Back Vertex
            std::string tmp_h;
            if (SMTCell::getVertex((*vCoord))->getBackADJ()->ifValid()) {
              tmp_h = fmt::format(
                  "N{}_E_{}_{}", en.first, vCoord->getVName(),
                  SMTCell::getVertex((*vCoord))->getBackADJ()->getVName());
            } else {
              tmp_h = fmt::format("N{}_E_{}_{}", en.first, vCoord->getVName(),
                                  "BackEnd");
            }

            if (!SMTCell::ifAssigned(tmp_i) && !SMTCell::ifAssigned(tmp_j)) {
              if (SMTCell::ifAssignedFalse(tmp_k)) {
                if (SMTCell::ifAssignedFalse(tmp_h)) {
                  SMTCell::setVar(tmp_i, 6);
                  SMTCell::setVar(tmp_j, 6);
                  SMTCell::writeConstraint(
                      fmt::format("(assert (ite (and (= {} true) (= {} "
                                  "true)) (= true false) (= true true)))\n",
                                  tmp_i, tmp_j));
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("c", 3);
                } else if (!SMTCell::ifAssigned(tmp_k)) {
                  SMTCell::setVar(tmp_i, 6);
                  SMTCell::setVar(tmp_j, 6);
                  SMTCell::setVar(tmp_k, 6);
                  SMTCell::writeConstraint(
                      fmt::format("(assert (ite (and (= {} true) (= {} "
                                  "true)) (= {} true) (= true true)))\n",
                                  tmp_i, tmp_j, tmp_h));
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("c", 3);
                }
              } else if (!SMTCell::ifAssigned(tmp_h)) {
                if (SMTCell::ifAssignedFalse(tmp_k)) {
                  SMTCell::setVar(tmp_i, 6);
                  SMTCell::setVar(tmp_j, 6);
                  SMTCell::setVar(tmp_k, 6);
                  SMTCell::writeConstraint(
                      fmt::format("(assert (ite (and (= {} true) (= {} "
                                  "true)) (= {} true) (= true true)))\n",
                                  tmp_i, tmp_j, tmp_k));
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("c", 3);
                } else if (!SMTCell::ifAssigned(tmp_k)) {
                  SMTCell::setVar(tmp_i, 6);
                  SMTCell::setVar(tmp_j, 6);
                  SMTCell::setVar(tmp_k, 6);
                  SMTCell::setVar(tmp_h, 6);
                  SMTCell::writeConstraint(fmt::format(
                      "(assert (ite (and (= {} true) (= {} true)) ((_ "
                      "at-least 1) {} {}) (= true true)))\n",
                      tmp_i, tmp_j, tmp_k, tmp_h));
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("c", 3);
                }
              }
            } else if (SMTCell::ifAssignedTrue(tmp_i) &&
                       SMTCell::ifAssignedTrue(tmp_j)) {
              if (SMTCell::ifAssignedFalse(tmp_k)) {
                if (SMTCell::ifAssignedFalse(tmp_h)) {
                  fmt::print("[ERROR] MPL : UNSAT Condition!!!\n");
                  exit(1);
                }
              }
            } else if (SMTCell::ifAssignedTrue(tmp_i)) {
              if (SMTCell::ifAssignedFalse(tmp_k)) {
                if (SMTCell::ifAssignedFalse(tmp_h)) {
                  SMTCell::setVar(tmp_j, 6);
                  SMTCell::writeConstraint(
                      fmt::format("(assert (ite (= {} true) (= true false) "
                                  "(= true true)))\n",
                                  tmp_j));
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("c", 3);
                } else if (!SMTCell::ifAssigned(tmp_h)) {
                  SMTCell::setVar(tmp_j, 6);
                  SMTCell::setVar(tmp_h, 6);
                  SMTCell::writeConstraint(
                      fmt::format("(assert (ite (= {} true) (= {} true) "
                                  "(= true true)))\n",
                                  tmp_j, tmp_h));
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("c", 3);
                }
              } else if (!SMTCell::ifAssigned(tmp_k)) {
                if (SMTCell::ifAssignedFalse(tmp_h)) {
                  SMTCell::setVar(tmp_j, 6);
                  SMTCell::setVar(tmp_k, 6);
                  SMTCell::writeConstraint(
                      fmt::format("(assert (ite (= {} true) (= {} true) "
                                  "(= true true)))\n",
                                  tmp_j, tmp_k));
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("c", 3);
                } else if (!SMTCell::ifAssigned(tmp_h)) {
                  SMTCell::setVar(tmp_j, 6);
                  SMTCell::setVar(tmp_k, 6);
                  SMTCell::setVar(tmp_h, 6);
                  SMTCell::writeConstraint(
                      fmt::format("(assert (ite (= {} true) ((_ at-least 1) "
                                  "{} {}) (= true true)))\n",
                                  tmp_j, tmp_k, tmp_h));
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("c", 3);
                }
              }
            } else if (SMTCell::ifAssignedTrue(tmp_j)) {
              if (SMTCell::ifAssignedFalse(tmp_k)) {
                if (SMTCell::ifAssignedFalse(tmp_h)) {
                  SMTCell::setVar(tmp_i, 6);
                  SMTCell::writeConstraint(
                      fmt::format("(assert (ite (= {} true) (= true false) "
                                  "(= true true)))\n",
                                  tmp_i));
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("c", 3);
                } else if (!SMTCell::ifAssigned(tmp_h)) {
                  SMTCell::setVar(tmp_i, 6);
                  SMTCell::setVar(tmp_h, 6);
                  SMTCell::writeConstraint(
                      fmt::format("(assert (ite (= {} true) (= {} true) "
                                  "(= true true)))\n",
                                  tmp_i, tmp_h));
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("c", 3);
                }
              } else if (!SMTCell::ifAssigned(tmp_k)) {
                if (SMTCell::ifAssignedFalse(tmp_h)) {
                  SMTCell::setVar(tmp_i, 6);
                  SMTCell::setVar(tmp_k, 6);
                  SMTCell::writeConstraint(
                      fmt::format("(assert (ite (= {} true) (= {} true) "
                                  "(= true true)))\n",
                                  tmp_i, tmp_k));
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("c", 3);
                } else if (!SMTCell::ifAssigned(tmp_h)) {
                  SMTCell::setVar(tmp_i, 6);
                  SMTCell::setVar(tmp_k, 6);
                  SMTCell::setVar(tmp_h, 6);
                  SMTCell::writeConstraint(
                      fmt::format("(assert (ite (= {} true) ((_ at-least 1) "
                                  "{} {}) (= true true)))\n",
                                  tmp_i, tmp_k, tmp_h));
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("c", 3);
                }
              }
            }
          }
        }
        // delete vCoord;
      }
    }
  }
}

void DesignRuleWriter::write_pin_access_rule_via34_helper(FILE *drlog) {
  for (auto en : SMTCell::getExtNet()) {
    // int netIndex = SMTCell::getNetIdx(std::to_string(en.first));
    // only subject to M3 (M1)
    int metal = 3;
    for (int row_idx = 0; row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3;
         row_idx++) {
      for (int col_idx = 0;
           col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
        // retrieve row
        int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
        // retrieve col
        int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);
        // if (metal > 1 && SMTCell::ifVertMetal(metal) &&
        // SMTCell::ifSDCol(col))
        // {
        //   continue;
        // }
        std::shared_ptr<Triplet> vCoord =
            std::make_shared<Triplet>(metal, row, col);
        if (SMTCell::ifVEdgeOut(vCoord->getVName())) {
          for (int i : SMTCell::getVEdgeOut(vCoord->getVName())) {
            std::string vName_i = SMTCell::getVirtualEdge(i)->getVName();
            std::string vName_j = SMTCell::getVirtualEdge(i)->getPinName();

            std::string tmp_i =
                fmt::format("N{}_E_{}_{}", en.first, vName_i, vName_j);
            // mergable flag: change up to down
            std::string tmp_j = fmt::format(
                "N{}_E_{}_{}", en.first, vCoord->getVName(),
                SMTCell::getVertex((*vCoord))->getUpADJ()->getVName());

            // Front Vertex
            std::string tmp_k;
            if (SMTCell::getVertex((*vCoord))->getFrontADJ()->ifValid()) {
              tmp_k = fmt::format(
                  "N{}_E_{}_{}", en.first,

                  SMTCell::getVertex((*vCoord))->getFrontADJ()->getVName(),
                  vCoord->getVName());
            } else {
              tmp_k = fmt::format("N{}_E_{}_{}", en.first, vCoord->getVName(),
                                  "FrontEnd");
            }

            // Back Vertex
            std::string tmp_h;
            if (SMTCell::getVertex((*vCoord))->getBackADJ()->ifValid()) {
              tmp_h = fmt::format(
                  "N{}_E_{}_{}", en.first, vCoord->getVName(),
                  SMTCell::getVertex((*vCoord))->getBackADJ()->getVName());
            } else {
              tmp_h = fmt::format("N{}_E_{}_{}", en.first, vCoord->getVName(),
                                  "BackEnd");
            }

            if (!SMTCell::ifAssigned(tmp_i) && !SMTCell::ifAssigned(tmp_j)) {
              if (SMTCell::ifAssignedFalse(tmp_k)) {
                if (SMTCell::ifAssignedFalse(tmp_h)) {
                  SMTCell::setVar(tmp_i, 6);
                  SMTCell::setVar(tmp_j, 6);
                  SMTCell::writeConstraint(
                      fmt::format("(assert (ite (and (= {} true) (= {} "
                                  "true)) (= true false) (= true true)))\n",
                                  tmp_i, tmp_j));
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("c", 3);
                } else if (!SMTCell::ifAssigned(tmp_k)) {
                  SMTCell::setVar(tmp_i, 6);
                  SMTCell::setVar(tmp_j, 6);
                  SMTCell::setVar(tmp_k, 6);
                  SMTCell::writeConstraint(
                      fmt::format("(assert (ite (and (= {} true) (= {} "
                                  "true)) (= {} true) (= true true)))\n",
                                  tmp_i, tmp_j, tmp_h));
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("c", 3);
                }
              } else if (!SMTCell::ifAssigned(tmp_h)) {
                if (SMTCell::ifAssignedFalse(tmp_k)) {
                  SMTCell::setVar(tmp_i, 6);
                  SMTCell::setVar(tmp_j, 6);
                  SMTCell::setVar(tmp_k, 6);
                  SMTCell::writeConstraint(
                      fmt::format("(assert (ite (and (= {} true) (= {} "
                                  "true)) (= {} true) (= true true)))\n",
                                  tmp_i, tmp_j, tmp_k));
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("c", 3);
                } else if (!SMTCell::ifAssigned(tmp_k)) {
                  SMTCell::setVar(tmp_i, 6);
                  SMTCell::setVar(tmp_j, 6);
                  SMTCell::setVar(tmp_k, 6);
                  SMTCell::setVar(tmp_h, 6);
                  SMTCell::writeConstraint(fmt::format(
                      "(assert (ite (and (= {} true) (= {} true)) ((_ "
                      "at-least 1) {} {}) (= true true)))\n",
                      tmp_i, tmp_j, tmp_k, tmp_h));
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("c", 3);
                }
              }
            } else if (SMTCell::ifAssignedTrue(tmp_i) &&
                       SMTCell::ifAssignedTrue(tmp_j)) {
              if (SMTCell::ifAssignedFalse(tmp_k)) {
                if (SMTCell::ifAssignedFalse(tmp_h)) {
                  fmt::print("[ERROR] MPL : UNSAT Condition!!!\n");
                  exit(1);
                }
              }
            } else if (SMTCell::ifAssignedTrue(tmp_i)) {
              if (SMTCell::ifAssignedFalse(tmp_k)) {
                if (SMTCell::ifAssignedFalse(tmp_h)) {
                  SMTCell::setVar(tmp_j, 6);
                  SMTCell::writeConstraint(
                      fmt::format("(assert (ite (= {} true) (= true false) "
                                  "(= true true)))\n",
                                  tmp_j));
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("c", 3);
                } else if (!SMTCell::ifAssigned(tmp_h)) {
                  SMTCell::setVar(tmp_j, 6);
                  SMTCell::setVar(tmp_h, 6);
                  SMTCell::writeConstraint(
                      fmt::format("(assert (ite (= {} true) (= {} true) "
                                  "(= true true)))\n",
                                  tmp_j, tmp_h));
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("c", 3);
                }
              } else if (!SMTCell::ifAssigned(tmp_k)) {
                if (SMTCell::ifAssignedFalse(tmp_h)) {
                  SMTCell::setVar(tmp_j, 6);
                  SMTCell::setVar(tmp_k, 6);
                  SMTCell::writeConstraint(
                      fmt::format("(assert (ite (= {} true) (= {} true) "
                                  "(= true true)))\n",
                                  tmp_j, tmp_k));
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("c", 3);
                } else if (!SMTCell::ifAssigned(tmp_h)) {
                  SMTCell::setVar(tmp_j, 6);
                  SMTCell::setVar(tmp_k, 6);
                  SMTCell::setVar(tmp_h, 6);
                  SMTCell::writeConstraint(
                      fmt::format("(assert (ite (= {} true) ((_ at-least 1) "
                                  "{} {}) (= true true)))\n",
                                  tmp_j, tmp_k, tmp_h));
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("c", 3);
                }
              }
            } else if (SMTCell::ifAssignedTrue(tmp_j)) {
              if (SMTCell::ifAssignedFalse(tmp_k)) {
                if (SMTCell::ifAssignedFalse(tmp_h)) {
                  SMTCell::setVar(tmp_i, 6);
                  SMTCell::writeConstraint(
                      fmt::format("(assert (ite (= {} true) (= true false) "
                                  "(= true true)))\n",
                                  tmp_i));
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("c", 3);
                } else if (!SMTCell::ifAssigned(tmp_h)) {
                  SMTCell::setVar(tmp_i, 6);
                  SMTCell::setVar(tmp_h, 6);
                  SMTCell::writeConstraint(
                      fmt::format("(assert (ite (= {} true) (= {} true) "
                                  "(= true true)))\n",
                                  tmp_i, tmp_h));
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("c", 3);
                }
              } else if (!SMTCell::ifAssigned(tmp_k)) {
                if (SMTCell::ifAssignedFalse(tmp_h)) {
                  SMTCell::setVar(tmp_i, 6);
                  SMTCell::setVar(tmp_k, 6);
                  SMTCell::writeConstraint(
                      fmt::format("(assert (ite (= {} true) (= {} true) "
                                  "(= true true)))\n",
                                  tmp_i, tmp_k));
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("c", 3);
                } else if (!SMTCell::ifAssigned(tmp_h)) {
                  SMTCell::setVar(tmp_i, 6);
                  SMTCell::setVar(tmp_k, 6);
                  SMTCell::setVar(tmp_h, 6);
                  SMTCell::writeConstraint(
                      fmt::format("(assert (ite (= {} true) ((_ at-least 1) "
                                  "{} {}) (= true true)))\n",
                                  tmp_i, tmp_k, tmp_h));
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("l", 3);
                  SMTCell::cnt("c", 3);
                }
              }
            }
          }
        }
        // delete vCoord;
      }
    }
  }
}

/**
 * Parallel Run Length (PRL) rule:
 * Enforces the avoidance of “single-pointcontact” in SADP mask manufacturing
 *
 * @note
 * Example: (PRL = 2) No Violation
 * ----X====X====X===
 *     |    |    |
 * =========X====X
 *     |    |    |
 * ----X====X====X===
 *
 * @param   PRL_Parameter   Distance in L1
 * @param   doublePowerRail If using Double Power Rail
 *
 * @return  void
 */
void DesignRuleWriter::write_PRL_AGR_rule(int PRL_M1_Parameter,
                                          int PRL_M2_Parameter,
                                          int PRL_M3_Parameter,
                                          int PRL_M4_Parameter,
                                          int doublePowerRail, FILE *drlog) {
  SMTCell::writeConstraint(";10. Parallel Run Length Rule\n");
  fmt::print(drlog, ";10. Parallel Run Length Rule\n");
  if (PRL_M1_Parameter == 0 && PRL_M2_Parameter == 0 && PRL_M3_Parameter == 0 &&
      PRL_M4_Parameter == 0) {
    std::cout << "is disable....\n";
    SMTCell::writeConstraint(";PRL is disabled\n");
  } else {
    // # PRL Rule Enable /Disable
    // ### Paralle Run-Length Rule to prevent from having too close metal
    // tips.
    // ### DATA STRUCTURE:  VERTEX [index] [name] [Z-pos] [Y-pos] [X-pos]
    // [Arr. of adjacent vertices]
    // ### DATA STRUCTURE:  ADJACENT_VERTICES [0:Left] [1:Right] [2:Front]
    // [3:Back] [4:Up] [5:Down] [6:FL] [7:FR] [8:BL] [9:BR]
    SMTCell::writeConstraint(
        fmt::format(";10-A. from Right Tip to Left Tips for each vertex\n"));
    fmt::print(drlog, ";10-A. from Right Tip to Left Tips for each vertex\n");
    DesignRuleWriter::write_PRL_AGR_RL_tip_helper(
        PRL_M2_Parameter, PRL_M4_Parameter, doublePowerRail, drlog);
    SMTCell::writeConstraint("\n");

    // Skip to check exact adjacent GV variable, (From right to left is
    // enough) 10-B is executed when PRL_Parameter > 1
    SMTCell::writeConstraint(
        fmt::format(";10-B. from Left Tip to Right Tips for each vertex\n"));
    fmt::print(drlog, ";10-B. from Left Tip to Right Tips for each vertex\n");
    DesignRuleWriter::write_PRL_AGR_LR_tip_helper(
        PRL_M2_Parameter, PRL_M4_Parameter, doublePowerRail, drlog);
    SMTCell::writeConstraint("\n");

    // DATA STRUCTURE:  VERTEX [index] [name] [Z-pos] [Y-pos] [X-pos] [Arr. of
    // adjacent vertices] DATA STRUCTURE:  ADJACENT_VERTICES [0:Left]
    // [1:Right] [2:Front] [3:Back] [4:Up] [5:Down] [6:FL] [7:FR] [8:BL]
    // [9:BR]
    // one Power Rail vertice has 2 times cost of other vertices. ($Double
    // Power Rail)
    SMTCell::writeConstraint(
        fmt::format(";10-C. from Back Tip to Front Tips for each vertex\n"));
    fmt::print(drlog, ";10-C. from Back Tip to Front Tips for each vertex\n");
    DesignRuleWriter::write_PRL_AGR_BF_tip_helper(
        PRL_M1_Parameter, PRL_M3_Parameter, doublePowerRail, drlog);
    SMTCell::writeConstraint("\n");

    SMTCell::writeConstraint(
        fmt::format(";10-D. from Front Tip to Back Tips for each vertex\n"));
    fmt::print(drlog, ";10-D. from Front Tip to Back Tips for each vertex\n");
    DesignRuleWriter::write_PRL_AGR_FB_tip_helper(
        PRL_M1_Parameter, PRL_M3_Parameter, doublePowerRail, drlog);
    SMTCell::writeConstraint("\n");
    std::cout << "have been written.\n";
  }
}

/**
 * @note
 *                            ----+----X====X==== <--- B Direction Checking
 *                                |    |    |
 *                            ▒▒▒▒▒▒▒▒▒X----+----
 *                                ║    |    |
 *                            ----+----X====X==== <--- F Direction Checking
 */
void DesignRuleWriter::write_PRL_AGR_RL_tip_helper(int PRL_M2_Parameter,
                                                   int PRL_M4_Parameter,
                                                   int doublePowerRail,
                                                   FILE *drlog) {
  // for (int metal = SMTCell::getFirstRoutingLayer(); metal <=
  // SMTCell::getNumMetalLayer();
  //        metal++) {                      // no DR on M1
  //     if (SMTCell::ifHorzMetal(metal)) { // M2
  //       for (int row = 0; row <= SMTCell::getLastRow(); row++) {
  //         for (int col = 0; col <= SMTCell::getLastColIdx(); col++) {
  for (int metal = SMTCell::getFirstRoutingLayer();
       metal <= SMTCell::getNumMetalLayer(); metal++) {
    if (SMTCell::ifHorzMetal(metal)) { // M2
      for (int row_idx = 0;
           row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3; row_idx++) {
        for (int col_idx = 0;
             col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
          // retrieve row/col
          int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
          int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d     PRL_RL: METAL {} ROW {} COL {} \n", metal,
                       row, col);
          }

          // F Direction Checking
          // Triplet *vCoord_F =
          //     SMTCell::getVertex(metal, row, col)->getFrontADJ();
          std::shared_ptr<Triplet> vCoord_F =
              SMTCell::getVertex(metal, row, col)->getFrontADJ();

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        vCoord_F: {} \n", vCoord_F->getVName());
          }

          // Layer-specific PRL Parameter
          int PRL_Parameter = 0;

          if (metal == 2) {
            PRL_Parameter = PRL_M2_Parameter;
          } else if (metal == 4) {
            PRL_Parameter = PRL_M4_Parameter;
          }

          if (vCoord_F->ifValid() && row != 0) {
            std::vector<std::string> tmp_var;
            int cnt_var = 0;
            int cnt_true = 0;

            std::string tmp_str =
                fmt::format("GR_V_{}", SMTCell::getVertexName(metal, row, col));

            if (!SMTCell::ifAssigned(tmp_str)) {
              tmp_var.push_back(tmp_str);
              SMTCell::setVar(tmp_str, 2);
              cnt_var++;
            } else if (SMTCell::ifAssigned(tmp_str) &&
                       SMTCell::getAssigned(tmp_str) == 1) {
              SMTCell::setVar_wo_cnt(tmp_str, 0);
              cnt_true++;
            }

            // AGR FLAG
            int currCol = vCoord_F->col_;
            int distToPOICol = abs(currCol - col);
            int realPRL = PRL_Parameter - SMTCell::row_idx_to_real(1);

            while (distToPOICol <= realPRL) {
              // for (int prlIndex = 0; prlIndex <= PRL_Parameter - 1;
              // prlIndex++) {
              std::string tmp_str =
                  fmt::format("GL_V_{}", vCoord_F->getVName());
              if (!SMTCell::ifAssigned(tmp_str)) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog,
                             "d       => Adding Forbidden PRL RL : {} \n",
                             tmp_str);
                }
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssigned(tmp_str) &&
                         SMTCell::getAssigned(tmp_str) == 1) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }
              // if (prlIndex != (PRL_Parameter - 1)) {
              vCoord_F = SMTCell::getVertex((*vCoord_F))->getLeftADJ();
              if (vCoord_F->ifValid() == false) {
                break;
              } else {
                // update current distance
                currCol = vCoord_F->col_;
                distToPOICol = abs(currCol - col);
              }
              // }
            }

            if (cnt_true > 0) {
              if (cnt_true > 1) {
                std::cerr << "\n[ERROR] PRL : more than one G Variables "
                             "are true!!!\n";
                exit(1);
              } else {
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::assignTrueVar(tmp_var[i], 0, true);
                }
              }
            } else {
              if (cnt_var > 1) {
                SMTCell::writeConstraint(fmt::format("(assert ((_ at-most 1)"));
                for (auto s : tmp_var) {
                  SMTCell::writeConstraint(fmt::format(" {}", s));
                  SMTCell::cnt("l", 3);
                }
                SMTCell::writeConstraint(fmt::format("))\n"));
                SMTCell::cnt("c", 3);
              }
            }
          }

          // B Track Direction Checking
          // Triplet *vCoord_B = SMTCell::getVertex(metal, row, col)->getBackADJ();
          std::shared_ptr<Triplet> vCoord_B =
              SMTCell::getVertex(metal, row, col)->getBackADJ();

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        vCoord_B: {} \n", vCoord_B->getVName());
          }

          if (vCoord_B->ifValid() && row != (SMTCell::getNumTrackH() - 3)) {
            std::vector<std::string> tmp_var;
            int cnt_var = 0;
            int cnt_true = 0;
            std::string tmp_str =
                fmt::format("GR_V_{}", SMTCell::getVertexName(metal, row, col));
            if (!SMTCell::ifAssigned(tmp_str)) {
              tmp_var.push_back(tmp_str);
              SMTCell::setVar(tmp_str, 2);
              cnt_var++;
            } else if (SMTCell::ifAssigned(tmp_str) &&
                       SMTCell::getAssigned(tmp_str) == 1) {
              SMTCell::setVar_wo_cnt(tmp_str, 0);
              cnt_true++;
            }

            // AGR FLAG
            int currCol = vCoord_F->col_;
            int distToPOICol = abs(currCol - col);
            int realPRL = PRL_Parameter - SMTCell::row_idx_to_real(1);

            while (distToPOICol <= realPRL) {
              // for (int prlIndex = 0; prlIndex <= PRL_Parameter - 1;
              // prlIndex++) {
              std::string tmp_str =
                  fmt::format("GL_V_{}", vCoord_B->getVName());
              if (!SMTCell::ifAssigned(tmp_str)) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog,
                             "d       => Adding Forbidden PRL RL : {} \n",
                             tmp_str);
                }
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssigned(tmp_str) &&
                         SMTCell::getAssigned(tmp_str) == 1) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }
              // if (prlIndex != (PRL_Parameter - 1)) {
              vCoord_B = SMTCell::getVertex((*vCoord_B))->getLeftADJ();
              if (vCoord_B->ifValid() == false) {
                break;
              } else {
                // update current distance
                currCol = vCoord_B->col_;
                distToPOICol = abs(currCol - col);
              }
              // }
            }

            if (cnt_true > 0) {
              if (cnt_true > 1) {
                std::cerr << "\n[ERROR] PRL : more than one G Variables "
                             "are true!!!\n";
                exit(1);
              } else {
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::assignTrueVar(tmp_var[i], 0, true);
                }
              }
            } else {
              if (cnt_var > 1) {
                SMTCell::writeConstraint(fmt::format("(assert ((_ at-most 1)"));
                for (auto s : tmp_var) {
                  SMTCell::writeConstraint(fmt::format(" {}", s));
                  SMTCell::cnt("l", 3);
                }
                SMTCell::writeConstraint(fmt::format("))\n"));
                SMTCell::cnt("c", 3);
              }
            }
          }
        }
      }
    }
  }
}

/**
 * @note
 *                            ====X====X----+---- <--- B Direction Checking
 *                                |    |    |
 *                            ----+----X▒▒▒▒▒▒▒▒▒
 *                                ║    |    |
 *                            ====X====X----+---- <--- F Direction Checking
 */
// 04/19/2023 LEGACY BUG FLAG : Should retrieve vCoord_F not vCoord_FR
void DesignRuleWriter::write_PRL_AGR_LR_tip_helper(int PRL_M2_Parameter,
                                                   int PRL_M4_Parameter,
                                                   int doublePowerRail,
                                                   FILE *drlog) {
  // for (int metal = SMTCell::getFirstRoutingLayer(); metal <=
  // SMTCell::getNumMetalLayer();
  //      metal++) {                      // no DR on M1
  //   if (SMTCell::ifHorzMetal(metal)) { // M2
  //     for (int row = 0; row <= SMTCell::getLastRow(); row++) {
  //       if (row == 0 || row == (SMTCell::getNumTrackH() - 3)) {
  //         // skip the PRL rule related with power Rail.
  //       }
  //       for (int col = 0; col <= SMTCell::getLastColIdx(); col++) {
  for (int metal = 2; metal <= SMTCell::getNumMetalLayer(); metal++) {
    if (SMTCell::ifHorzMetal(metal)) { // M2
      for (int row_idx = 0;
           row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3; row_idx++) {
        for (int col_idx = 0;
             col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
          // retrieve row/col
          int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
          int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d     PRL_LR: METAL {} ROW {} COL {} \n", metal,
                       row, col);
          }

          // Layer-specific PRL Parameter
          int PRL_Parameter = 0;

          if (metal == 2) {
            PRL_Parameter = PRL_M2_Parameter;
          } else if (metal == 4) {
            PRL_Parameter = PRL_M4_Parameter;
          }

          // F Direction Checking
          // Triplet *vCoord_F =
          //     SMTCell::getVertex(metal, row, col)->getFrontADJ();
          std::shared_ptr<Triplet> vCoord_F =
              SMTCell::getVertex(metal, row, col)->getFrontADJ();

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        vCoord_F: {} \n", vCoord_F->getVName());
          }

          if (vCoord_F->ifValid() && row_idx != 0 && PRL_Parameter > 1) {
            std::vector<std::string> tmp_var;
            int cnt_var = 0;
            int cnt_true = 0;
            std::string tmp_str =
                fmt::format("GL_V_{}", SMTCell::getVertexName(metal, row, col));
            if (!SMTCell::ifAssigned(tmp_str)) {
              tmp_var.push_back(tmp_str);
              SMTCell::setVar(tmp_str, 2);
              cnt_var++;
            } else if (SMTCell::ifAssigned(tmp_str) &&
                       SMTCell::getAssigned(tmp_str) == 1) {
              SMTCell::setVar_wo_cnt(tmp_str, 0);
              cnt_true++;
            }

            // AGR FLAG
            int currCol = vCoord_F->col_;
            int distToPOICol = abs(currCol - col);
            int realPRL = PRL_Parameter - SMTCell::row_idx_to_real(1);

            // for (int prlIndex = 0; prlIndex <= PRL_Parameter - 2; prlIndex++)
            // {
            while (distToPOICol <= realPRL) {
              std::string tmp_str =
                  fmt::format("GR_V_{}", vCoord_F->getVName());
              if (!SMTCell::ifAssigned(tmp_str)) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog,
                             "d       => Adding Forbidden PRL LR : {} \n",
                             tmp_str);
                }
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssigned(tmp_str) &&
                         SMTCell::getAssigned(tmp_str) == 1) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }
              vCoord_F = SMTCell::getVertex((*vCoord_F))->getRightADJ();
              if (vCoord_F->ifValid() == false) {
                break;
              } else {
                // update current distance
                currCol = vCoord_F->col_;
                distToPOICol = abs(currCol - col);
              }
              // }
            }

            if (cnt_true > 0) {
              if (cnt_true > 1) {
                std::cerr << "\n[ERROR] PRL : more than one G Variables "
                             "are true!!!\n";
                exit(1);
              } else {
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::assignTrueVar(tmp_var[i], 0, true);
                }
              }
            } else {
              if (cnt_var > 1) {
                SMTCell::writeConstraint(fmt::format("(assert ((_ at-most 1)"));
                for (auto s : tmp_var) {
                  SMTCell::writeConstraint(fmt::format(" {}", s));
                  SMTCell::cnt("l", 3);
                }
                SMTCell::writeConstraint(fmt::format("))\n"));
                SMTCell::cnt("c", 3);
              }
            }
          }

          // B Track Direction Checking
          // Triplet *vCoord_B = SMTCell::getVertex(metal, row, col)->getBackADJ();
          std::shared_ptr<Triplet> vCoord_B =
              SMTCell::getVertex(metal, row, col)->getBackADJ();

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        vCoord_B: {} \n", vCoord_B->getVName());
          }

          if (vCoord_B->ifValid() && row != (SMTCell::getNumTrackH() - 3) &&
              PRL_Parameter > 1) {
            std::vector<std::string> tmp_var;
            int cnt_var = 0;
            int cnt_true = 0;
            std::string tmp_str =
                fmt::format("GL_V_{}", SMTCell::getVertexName(metal, row, col));
            if (!SMTCell::ifAssigned(tmp_str)) {
              tmp_var.push_back(tmp_str);
              SMTCell::setVar(tmp_str, 2);
              cnt_var++;
            } else if (SMTCell::ifAssigned(tmp_str) &&
                       SMTCell::getAssigned(tmp_str) == 1) {
              SMTCell::setVar_wo_cnt(tmp_str, 0);
              cnt_true++;
            }

            // AGR FLAG
            int currCol = vCoord_F->col_;
            int distToPOICol = abs(currCol - col);
            int realPRL = PRL_Parameter - SMTCell::row_idx_to_real(1);

            // for (int prlIndex = 0; prlIndex <= PRL_Parameter - 2; prlIndex++)
            // {
            while (distToPOICol <= realPRL) {
              std::string tmp_str =
                  fmt::format("GR_V_{}", vCoord_B->getVName());
              if (!SMTCell::ifAssigned(tmp_str)) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog,
                             "d       => Adding Forbidden PRL LR : {} \n",
                             tmp_str);
                }
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssigned(tmp_str) &&
                         SMTCell::getAssigned(tmp_str) == 1) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }

              vCoord_B = SMTCell::getVertex((*vCoord_B))->getRightADJ();
              if (vCoord_B->ifValid() == false) {
                break;
              } else {
                // update current distance
                currCol = vCoord_B->col_;
                distToPOICol = abs(currCol - col);
              }
            }
            if (cnt_true > 0) {
              if (cnt_true > 1) {
                std::cerr << "\n[ERROR] PRL : more than one G Variables "
                             "are true!!!\n";
                exit(1);
              } else {
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::assignTrueVar(tmp_var[i], 0, true);
                }
              }
            } else {
              if (cnt_var > 1) {
                SMTCell::writeConstraint(fmt::format("(assert ((_ at-most 1)"));
                for (auto s : tmp_var) {
                  SMTCell::writeConstraint(fmt::format(" {}", s));
                  SMTCell::cnt("l", 3);
                }
                SMTCell::writeConstraint(fmt::format("))\n"));
                SMTCell::cnt("c", 3);
              }
            }
          }
        }
      }
    }
  }
}
/**
 * @note
 *                                |    ▒    |
 *                            ----+----▒----+----
 *                                |    ▒    |
 *  L Direction Checking ---> ----X----X----X---- <--- R Direction Checking
 *                                ║    |    ║
 *                            ----X----+----X----
 *                                ║    |    ║
 */
void DesignRuleWriter::write_PRL_AGR_BF_tip_helper(int PRL_M1_Parameter,
                                                   int PRL_M3_Parameter,
                                                   int doublePowerRail,
                                                   FILE *drlog) {
  // currently disabled
  // fmt::print("\na     [WARNING] PRL_AGR_back_tip is disabled.\n");
  fmt::print(drlog, "\na     [WARNING] PRL_AGR_back_tip is disabled.\n");
}

/**
 * @note
 *                                ║    |    ║
 *                            ----X----+----X----
 *                                ║    |    ║
 *  L Direction Checking ---> ----X----X----X---- <--- R Direction Checking
 *                                |    ▒    |
 *                            ----+----▒----+----
 *                                |    ▒    |
 */
void DesignRuleWriter::write_PRL_AGR_FB_tip_helper(int PRL_M1_Parameter,
                                                   int PRL_M3_Parameter,
                                                   int doublePowerRail,
                                                   FILE *drlog) {
  // currently disabled
  // fmt::print("a     [WARNING] PRL_AGR_front_tip is disabled.\n");
  fmt::print(drlog, "a     [WARNING] PRL_AGR_front_tip is disabled.\n");
}

/**
 * Step Height Rule (SHR) rule:
 * Enforces avoiding “the small step” in SADP mask manufacturing.
 *
 * @note
 * Example: (SHR = 2) *Violation*
 * ====X====X----+---
 *     |    |    |
 * ====X----+----+---
 *     |    |    |
 * ====X====X----+---
 *
 * @param   SHR_Parameter   Distance in L1
 * @param   doublePowerRail If using Double Power Rail
 *
 * @return  void
 */
void DesignRuleWriter::write_SHR_AGR_rule(int SHR_M1_Parameter,
                                          int SHR_M2_Parameter,
                                          int SHR_M3_Parameter,
                                          int SHR_M4_Parameter,
                                          int doublePowerRail, FILE *drlog) {
  SMTCell::writeConstraint(";13. Step Height Rule\n");
  fmt::print(drlog, ";13. Step Height Rule\n");
  // if (SHR_Parameter == 0 || SHR_Parameter < 2) {
  // Disable SHR if it is less than an "L-shape" distance
  if (SHR_M1_Parameter == 0 || SHR_M2_Parameter == 0 || SHR_M3_Parameter == 0 ||
      SHR_M4_Parameter == 0) {
    fmt::print("is disabled\n");
    SMTCell::writeConstraint(";SHR is disabled\n");
  } else {
    SMTCell::writeConstraint(
        ";13-A. from Right Tip to Right Tips for each vertex\n");
    fmt::print(drlog, ";13-A. from Right Tip to Right Tips for each vertex\n");
    DesignRuleWriter::write_SHR_AGR_R_tip_helper(
        SHR_M2_Parameter, SHR_M4_Parameter, doublePowerRail, drlog);
    SMTCell::writeConstraint("\n");

    SMTCell::writeConstraint(
        ";13-B. from Left Tip to Left Tips for each vertex\n");
    fmt::print(drlog, ";13-B. from Left Tip to Left Tips for each vertex\n");
    DesignRuleWriter::write_SHR_AGR_L_tip_helper(
        SHR_M2_Parameter, SHR_M4_Parameter, doublePowerRail, drlog);
    SMTCell::writeConstraint("\n");

    SMTCell::writeConstraint(
        ";13-C. from Back Tip to Back Tips for each vertex\n");
    fmt::print(drlog, ";13-C. from Back Tip to Back Tips for each vertex\n");
    DesignRuleWriter::write_SHR_AGR_B_tip_helper(
        SHR_M1_Parameter, SHR_M3_Parameter, doublePowerRail, drlog);
    SMTCell::writeConstraint("\n");

    SMTCell::writeConstraint(
        ";13-D. from Front Tip to Front Tips for each vertex\n");
    fmt::print(drlog, ";13-D. from Front Tip to Front Tips for each vertex\n");
    DesignRuleWriter::write_SHR_AGR_F_tip_helper(
        SHR_M1_Parameter, SHR_M3_Parameter, doublePowerRail, drlog);
    SMTCell::writeConstraint("\n");
    fmt::print("have been written.\n");
  }
}

/**
 * @note
 *                            ====+====X----X---- <--- BR Direction Checking
 *                                |    |    |
 *                            ▒▒▒▒X----+----+----
 *                                |    |    |
 *                            ====+====X----X---- <--- FR Direction Checking
 */
void DesignRuleWriter::write_SHR_AGR_R_tip_helper(int SHR_M2_Parameter,
                                                  int SHR_M4_Parameter,
                                                  int doublePowerRail,
                                                  FILE *drlog) {
  // for (int metal = SMTCell::getFirstRoutingLayer(); metal <=
  // SMTCell::getNumMetalLayer();
  //      metal++) {                      // no DR on M1
  //   if (SMTCell::ifHorzMetal(metal)) { // M2
  //     for (int row = 0; row <= SMTCell::getLastRow(); row++) {
  //       for (int col = 0; col <= SMTCell::getNumTrackV() - 2; col++) {
  for (int metal = SMTCell::getFirstRoutingLayer();
       metal <= SMTCell::getNumMetalLayer(); metal++) {
    if (SMTCell::ifHorzMetal(metal)) { // M2
      for (int row_idx = 0;
           row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3; row_idx++) {
        for (int col_idx = 0;
             col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 2; col_idx++) {
          // retrieve row/col
          int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
          int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d     SHR_R: METAL {} ROW {} COL {} \n", metal,
                       row, col);
          }
          // # FR Direction Checking
          // Triplet *vCoord_FR =
          //     SMTCell::getVertex(metal, row, col)->getFrontRightADJ();
          std::shared_ptr<Triplet> vCoord_FR =
              SMTCell::getVertex(metal, row, col)->getFrontRightADJ();

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        vCoord_FR: {} \n",
                       vCoord_FR->getVName());
          }

          // Layer-specific SHR Parameter
          int SHR_Parameter = 0;

          if (metal == 2) {
            SHR_Parameter = SHR_M2_Parameter;
          } else if (metal == 4) {
            SHR_Parameter = SHR_M4_Parameter;
          }

          if (vCoord_FR->ifValid()) {
            std::string tmp_str = "";
            std::vector<std::string> tmp_var;
            int cnt_var = 0;
            int cnt_true = 0;

            tmp_str =
                fmt::format("GR_V_{}", SMTCell::getVertexName(metal, row, col));

            // check declaration
            if (!SMTCell::ifAssigned(tmp_str)) {
              tmp_var.push_back(tmp_str);
              SMTCell::setVar(tmp_str, 2);
              cnt_var++;
            } else if (SMTCell::ifAssignedTrue(tmp_str)) {
              SMTCell::setVar_wo_cnt(tmp_str, 0);
              cnt_true++;
            }

            // AGR Flag
            int currCol = vCoord_FR->col_;
            int distToPOICol = abs(currCol - col);
            int realSHR = SHR_Parameter - SMTCell::row_idx_to_real(1);

            // for (int shrIndex = 0; shrIndex <= SHR_Parameter - 2; shrIndex++)
            // {
            while (distToPOICol <= realSHR) {
              tmp_str = fmt::format("GR_V_{}", vCoord_FR->getVName());
              // check declaration
              if (!SMTCell::ifAssigned(tmp_str)) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog, "d       => Adding Forbidden SHR R : {} \n",
                             tmp_str);
                }
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }

              // if (shrIndex != (SHR_Parameter - 2)) {
              // iterate to the right
              vCoord_FR = SMTCell::getVertex((*vCoord_FR))->getRightADJ();
              // fmt::print("iterate {}", vName_FR)
              if (!vCoord_FR->ifValid()) {
                break;
              } else {
                currCol = vCoord_FR->col_;
                distToPOICol = abs(currCol - col);
              }
              // }
            }

            if (cnt_true > 0) {
              if (cnt_true > 1) {
                fmt::print("[ERROR] SHR : more than one G Variables are "
                           "true!!!\n");
                exit(1);
              } else {
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::assignTrueVar(tmp_var[i], 0, true);
                }
              }
            } else {
              if (cnt_var > 1) {
                SMTCell::writeConstraint("(assert ((_ at-most 1)");
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::writeConstraint(fmt::format(" {}", tmp_var[i]));
                  SMTCell::cnt("l", 3);
                }
                SMTCell::writeConstraint("))\n");
                SMTCell::cnt("c", 3);
              }
            }
          }

          // # BR Direction Checking
          // Triplet *vName_BR =
          //     SMTCell::getVertex(metal, row, col)->getBackRightADJ();
          std::shared_ptr<Triplet> vName_BR =
              SMTCell::getVertex(metal, row, col)->getBackRightADJ();

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        vName_BR: {} \n", vName_BR->getVName());
          }

          if (vName_BR->ifValid()) {
            std::string tmp_str = "";
            std::vector<std::string> tmp_var;
            int cnt_var = 0;
            int cnt_true = 0;

            tmp_str =
                fmt::format("GR_V_{}", SMTCell::getVertexName(metal, row, col));

            // check declaration
            if (!SMTCell::ifAssigned(tmp_str)) {
              tmp_var.push_back(tmp_str);
              SMTCell::setVar(tmp_str, 2);
              cnt_var++;
            } else if (SMTCell::ifAssignedTrue(tmp_str)) {
              SMTCell::setVar_wo_cnt(tmp_str, 0);
              cnt_true++;
            }

            // AGR Flag
            int currCol = vName_BR->col_;
            int distToPOICol = abs(currCol - col);
            int realSHR = SHR_Parameter - SMTCell::row_idx_to_real(1);

            // for (int shrIndex = 0; shrIndex <= SHR_Parameter - 2; shrIndex++)
            // {
            while (distToPOICol <= realSHR) {
              tmp_str = fmt::format("GR_V_{}", vName_BR->getVName());

              // check declaration
              if (!SMTCell::ifAssigned(tmp_str)) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog, "d       => Adding Forbidden SHR R : {} \n",
                             tmp_str);
                }
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }

              // if (shrIndex != (SHR_Parameter - 2)) {
              // iterate to the right
              vName_BR = SMTCell::getVertex((*vName_BR))->getRightADJ();
              if (!vName_BR->ifValid()) {
                break;
              } else {
                currCol = vName_BR->col_;
                distToPOICol = abs(currCol - col);
              }
              // }
            }

            if (cnt_true > 0) {
              if (cnt_true > 1) {
                fmt::print("[ERROR] SHR : more than one G Variables are "
                           "true!!!\n");
                exit(1);
              } else {
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::assignTrueVar(tmp_var[i], 0, true);
                }
              }
            } else {
              if (cnt_var > 1) {
                SMTCell::writeConstraint("(assert ((_ at-most 1)");
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::writeConstraint(fmt::format(" {}", tmp_var[i]));
                  SMTCell::cnt("l", 3);
                }
                SMTCell::writeConstraint("))\n");
                SMTCell::cnt("c", 3);
              }
            }
          }
        }
      }
    }
  }
}
/**
 * @note
 * BL Direction Checking ---> ----+----X====X====
 *                                |    |    |
 *                            ----+----+----X▒▒▒▒
 *                                |    |    |
 * FL Direction Checking ---> ----+----X====X====
 */
void DesignRuleWriter::write_SHR_AGR_L_tip_helper(int SHR_M2_Parameter,
                                                  int SHR_M4_Parameter,
                                                  int doublePowerRail,
                                                  FILE *drlog) {
  // for (int metal = SMTCell::getFirstRoutingLayer(); metal <=
  // SMTCell::getNumMetalLayer();
  //      metal++) {                      // no DR on M1
  //   if (SMTCell::ifHorzMetal(metal)) { // M2
  //     for (int row = 0; row <= SMTCell::getLastRow(); row++) {
  //       for (int col = 1; col <= SMTCell::getNumTrackV() - 1; col++) {
  for (int metal = 2; metal <= SMTCell::getNumMetalLayer(); metal++) {
    if (SMTCell::ifHorzMetal(metal)) { // M2
      for (int row_idx = 0;
           row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3; row_idx++) {
        for (int col_idx = 0;
             col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
          // retrieve row/col
          int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
          int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d     SHR_L: METAL {} ROW {} COL {} \n", metal,
                       row, col);
          }

          // # FL Direction Checking
          // Triplet *vCoord_FL =
          //     SMTCell::getVertex(metal, row, col)->getFrontLeftADJ();
          std::shared_ptr<Triplet> vCoord_FL =
              SMTCell::getVertex(metal, row, col)->getFrontLeftADJ();

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        vCoord_FL: {} \n",
                       vCoord_FL->getVName());
          }

          // Layer-specific SHR Parameter
          int SHR_Parameter = 0;

          if (metal == 2) {
            SHR_Parameter = SHR_M2_Parameter;
          } else if (metal == 4) {
            SHR_Parameter = SHR_M4_Parameter;
          }

          if (vCoord_FL->ifValid()) {
            std::string tmp_str = "";
            std::vector<std::string> tmp_var;
            int cnt_var = 0;
            int cnt_true = 0;

            tmp_str =
                fmt::format("GL_V_{}", SMTCell::getVertexName(metal, row, col));

            // check declaration
            if (!SMTCell::ifAssigned(tmp_str)) {
              tmp_var.push_back(tmp_str);
              SMTCell::setVar(tmp_str, 2);
              cnt_var++;
            } else if (SMTCell::ifAssignedTrue(tmp_str)) {
              SMTCell::setVar_wo_cnt(tmp_str, 0);
              cnt_true++;
            }

            int currCol = vCoord_FL->col_;
            int distToPOICol = abs(currCol - col);
            int realSHR = SHR_Parameter - SMTCell::row_idx_to_real(1);

            // for (int shrIndex = 0; shrIndex <= SHR_Parameter - 2; shrIndex++)
            // {
            while (distToPOICol <= realSHR) {
              tmp_str = fmt::format("GL_V_{}", vCoord_FL->getVName());
              // check declaration
              if (!SMTCell::ifAssigned(tmp_str)) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog, "d       => Adding Forbidden SHR L : {} \n",
                             tmp_str);
                }
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }

              // if (shrIndex != (SHR_Parameter - 2)) {
              // iterate to the right
              vCoord_FL = SMTCell::getVertex((*vCoord_FL))->getLeftADJ();
              // fmt::print("iterate {}", vName_FR)
              if (!vCoord_FL->ifValid()) {
                break;
              } else {
                currCol = vCoord_FL->col_;
                distToPOICol = abs(currCol - col);
              }
              // }
            }

            if (cnt_true > 0) {
              if (cnt_true > 1) {
                fmt::print("[ERROR] SHR : more than one G Variables are "
                           "true!!!\n");
                exit(1);
              } else {
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::assignTrueVar(tmp_var[i], 0, true);
                }
              }
            } else {
              if (cnt_var > 1) {
                SMTCell::writeConstraint("(assert ((_ at-most 1)");
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::writeConstraint(fmt::format(" {}", tmp_var[i]));
                  SMTCell::cnt("l", 3);
                }
                SMTCell::writeConstraint("))\n");
                SMTCell::cnt("c", 3);
              }
            }
          }

          // # BL Direction Checking
          // Triplet *vName_BL =
          //     SMTCell::getVertex(metal, row, col)->getBackLeftADJ();
          std::shared_ptr<Triplet> vName_BL =
              SMTCell::getVertex(metal, row, col)->getBackLeftADJ();

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        vName_BL: {} \n", vName_BL->getVName());
          }

          if (vName_BL->ifValid()) {
            std::string tmp_str = "";
            std::vector<std::string> tmp_var;
            int cnt_var = 0;
            int cnt_true = 0;

            tmp_str =
                fmt::format("GL_V_{}", SMTCell::getVertexName(metal, row, col));

            // check declaration
            if (!SMTCell::ifAssigned(tmp_str)) {
              tmp_var.push_back(tmp_str);
              SMTCell::setVar(tmp_str, 2);
              cnt_var++;
            } else if (SMTCell::ifAssignedTrue(tmp_str)) {
              SMTCell::setVar_wo_cnt(tmp_str, 0);
              cnt_true++;
            }

            // AGR FLAG
            int currCol = vName_BL->col_;
            int distToPOICol = abs(currCol - col);
            int realSHR = SHR_Parameter - SMTCell::row_idx_to_real(1);

            // for (int shrIndex = 0; shrIndex <= SHR_Parameter - 2; shrIndex++)
            // {
            while (distToPOICol <= realSHR) {
              tmp_str = fmt::format("GL_V_{}", vName_BL->getVName());
              // check declaration
              if (!SMTCell::ifAssigned(tmp_str)) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog, "d       => Adding Forbidden SHR L : {} \n",
                             tmp_str);
                }
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }

              // if (shrIndex != (SHR_Parameter - 2)) {
              // iterate to the right
              vName_BL = SMTCell::getVertex((*vName_BL))->getLeftADJ();
              if (!vName_BL->ifValid()) {
                break;
              } else {
                currCol = vName_BL->col_;
                distToPOICol = abs(currCol - col);
              }
              // }
            }

            if (cnt_true > 0) {
              if (cnt_true > 1) {
                fmt::print("[ERROR] SHR : more than one G Variables are "
                           "true!!!\n");
                exit(1);
              } else {
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::assignTrueVar(tmp_var[i], 0, true);
                }
              }
            } else {
              if (cnt_var > 1) {
                SMTCell::writeConstraint("(assert ((_ at-most 1)");
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::writeConstraint(fmt::format(" {}", tmp_var[i]));
                  SMTCell::cnt("l", 3);
                }
                SMTCell::writeConstraint("))\n");
                SMTCell::cnt("c", 3);
              }
            }
          }
        }
      }
    }
  }
}

/**
 * @note
 *                                |    |    |
 *                            ----+----+----+----
 *                                |    |    |
 * BL Direction Checking ---> ----X----+----X---- <--- BR Direction Checking
 *                                ║    |    ║
 *                            ----X----X----X----
 *                                ║    ▒    ║
 */
void DesignRuleWriter::write_SHR_AGR_B_tip_helper(int SHR_M1_Parameter,
                                                  int SHR_M3_Parameter,
                                                  int doublePowerRail,
                                                  FILE *drlog) {
  // for (int metal = SMTCell::getFirstRoutingLayer(); metal <=
  // SMTCell::getNumMetalLayer(); metal++) {
  //   if (SMTCell::ifVertMetal(metal)) {
  //     for (int row = 0; row <= SMTCell::getNumTrackH() - 4; row++) {
  //       for (int col = 0; col <= SMTCell::getLastColIdx(); col++) {
  //         if (metal > 1 && SMTCell::ifVertMetal(metal)) {
  //           continue;
  //         }
  for (int metal = 2; metal <= SMTCell::getNumMetalLayer(); metal++) {
    if (SMTCell::ifVertMetal(metal)) {
      for (int row_idx = 0;
           row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 4; row_idx++) {
        for (int col_idx = 0;
             col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
          // retrieve row/col
          int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
          int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d     SHR_B: METAL {} ROW {} COL {} \n", metal,
                       row, col);
          }

          // Triplet *vCoord_BL =
          //     SMTCell::getVertex(metal, row, col)->getBackLeftADJ();
          std::shared_ptr<Triplet> vCoord_BL =
              SMTCell::getVertex(metal, row, col)->getBackLeftADJ();

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        vCoord_BL: {} \n",
                       vCoord_BL->getVName());
          }

          // Layer-specific SHR Parameter
          int SHR_Parameter = 0;

          if (metal == 1) {
            SHR_Parameter = SHR_M1_Parameter;
          } else if (metal == 3) {
            SHR_Parameter = SHR_M3_Parameter;
          }

          if (vCoord_BL->ifValid()) {
            std::string tmp_str = "";
            std::vector<std::string> tmp_var;
            int cnt_var = 0;
            int cnt_true = 0;

            tmp_str =
                fmt::format("GB_V_{}", SMTCell::getVertexName(metal, row, col));

            // check declaration
            if (!SMTCell::ifAssigned(tmp_str)) {
              tmp_var.push_back(tmp_str);
              SMTCell::setVar(tmp_str, 2);
              cnt_var++;
            } else if (SMTCell::ifAssignedTrue(tmp_str)) {
              SMTCell::setVar_wo_cnt(tmp_str, 0);
              cnt_true++;
            }

            // AGR FLAG
            int currCol = vCoord_BL->col_;
            int distToPOIRow = SMTCell::row_idx_to_real(1);
            int realSHR = SHR_Parameter - abs(currCol - distToPOIRow);

            // for (int shrIndex = 0; shrIndex <= SHR_Parameter - 2; shrIndex++)
            // {
            while (distToPOIRow <= realSHR) {
              tmp_str = fmt::format("GB_V_{}", vCoord_BL->getVName());
              // check declaration
              if (!SMTCell::ifAssigned(tmp_str)) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog, "d       => Adding Forbidden SHR B : {} \n",
                             tmp_str);
                }
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }

              // if (shrIndex != (SHR_Parameter - 2)) {
              // iterate to the right
              vCoord_BL = SMTCell::getVertex((*vCoord_BL))->getBackADJ();
              // fmt::print("iterate {}", vName_FR)
              if (!vCoord_BL->ifValid()) {
                break;
              } else {
                distToPOIRow += SMTCell::row_idx_to_real(1);
              }
              // }
            }

            if (cnt_true > 0) {
              if (cnt_true > 1) {
                fmt::print("[ERROR] SHR : more than one G Variables are "
                           "true!!!\n");
                exit(1);
              } else {
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::assignTrueVar(tmp_var[i], 0, true);
                }
              }
            } else {
              if (cnt_var > 1) {
                SMTCell::writeConstraint("(assert ((_ at-most 1)");
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::writeConstraint(fmt::format(" {}", tmp_var[i]));
                  SMTCell::cnt("l", 3);
                }
                SMTCell::writeConstraint("))\n");
                SMTCell::cnt("c", 3);
              }
            }
          }

          // # BR Direction Checking
          // Triplet *vName_BR =
          //     SMTCell::getVertex(metal, row, col)->getBackLeftADJ();
          std::shared_ptr<Triplet> vName_BR =
              SMTCell::getVertex(metal, row, col)->getBackRightADJ();

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        vName_BR: {} \n", vName_BR->getVName());
          }

          if (vName_BR->ifValid()) {
            std::string tmp_str = "";
            std::vector<std::string> tmp_var;
            int cnt_var = 0;
            int cnt_true = 0;

            tmp_str =
                fmt::format("GB_V_{}", SMTCell::getVertexName(metal, row, col));

            // check declaration
            if (!SMTCell::ifAssigned(tmp_str)) {
              tmp_var.push_back(tmp_str);
              SMTCell::setVar(tmp_str, 2);
              cnt_var++;
            } else if (SMTCell::ifAssignedTrue(tmp_str)) {
              SMTCell::setVar_wo_cnt(tmp_str, 0);
              cnt_true++;
            }

            // AGR FLAG
            int currCol = vName_BR->col_;
            int distToPOIRow = SMTCell::row_idx_to_real(1);
            int realSHR = SHR_Parameter - abs(currCol - col);

            // for (int shrIndex = 0; shrIndex <= SHR_Parameter - 2; shrIndex++)
            // {
            while (distToPOIRow <= realSHR) {
              tmp_str = fmt::format("GB_V_{}", vName_BR->getVName());
              // check declaration
              if (!SMTCell::ifAssigned(tmp_str)) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog, "d       => Adding Forbidden SHR B : {} \n",
                             tmp_str);
                }
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }

              // if (shrIndex != (SHR_Parameter - 2)) {
              // iterate to the right
              vName_BR = SMTCell::getVertex((*vName_BR))->getBackADJ();
              if (!vName_BR->ifValid()) {
                break;
              } else {
                distToPOIRow += SMTCell::row_idx_to_real(1);
              }
              // }
            }

            if (cnt_true > 0) {
              if (cnt_true > 1) {
                fmt::print("[ERROR] SHR : more than one G Variables are "
                           "true!!!\n");
                exit(1);
              } else {
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::assignTrueVar(tmp_var[i], 0, true);
                }
              }
            } else {
              if (cnt_var > 1) {
                SMTCell::writeConstraint("(assert ((_ at-most 1)");
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::writeConstraint(fmt::format(" {}", tmp_var[i]));
                  SMTCell::cnt("l", 3);
                }
                SMTCell::writeConstraint("))\n");
                SMTCell::cnt("c", 3);
              }
            }
          }
        }
      }
    }
  }
}

/**
 * @note
 *                                ║    ▒    ║
 *                            ----X----X----X----
 *                                ║    |    ║
 * FL Direction Checking ---> ----X----+----X---- <--- FR Direction Checking
 *                                |    |    |
 *                            ----+----+----+----
 *                                |    |    |
 */
void DesignRuleWriter::write_SHR_AGR_F_tip_helper(int SHR_M1_Parameter,
                                                  int SHR_M3_Parameter,
                                                  int doublePowerRail,
                                                  FILE *drlog) {
  // for (int metal = SMTCell::getFirstRoutingLayer(); metal <=
  // SMTCell::getNumMetalLayer();
  //      metal++) {                      // no DR on M1
  //   if (SMTCell::ifVertMetal(metal)) { // M2
  //     for (int row = 1; row <= SMTCell::getNumTrackH() - 3; row++) {
  //       for (int col = 0; col <= SMTCell::getLastColIdx(); col++) {
  //         if (metal > 1 && SMTCell::ifVertMetal(metal)) {
  //           continue;
  //         }
  for (int metal = 2; metal <= SMTCell::getNumMetalLayer(); metal++) {
    if (SMTCell::ifVertMetal(metal)) { // M2
      for (int row_idx = 0;
           row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3; row_idx++) {
        for (int col_idx = 0;
             col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
          // retrieve row/col
          int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
          int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d     SHR_F: METAL {} ROW {} COL {} \n", metal,
                       row, col);
          }

          // # FL Direction Checking
          // Triplet *vCoord_FL =
          //     SMTCell::getVertex(metal, row, col)->getFrontLeftADJ();
          std::shared_ptr<Triplet> vCoord_FL =
              SMTCell::getVertex(metal, row, col)->getFrontLeftADJ();

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        vCoord_FL: {} \n",
                       vCoord_FL->getVName());
          }

          // Layer-specific SHR Parameter
          int SHR_Parameter = 0;

          if (metal == 1) {
            SHR_Parameter = SHR_M1_Parameter;
          } else if (metal == 3) {
            SHR_Parameter = SHR_M3_Parameter;
          }

          if (vCoord_FL->ifValid()) {
            std::string tmp_str = "";
            std::vector<std::string> tmp_var;
            int cnt_var = 0;
            int cnt_true = 0;

            tmp_str =
                fmt::format("GL_V_{}", SMTCell::getVertexName(metal, row, col));

            // check declaration
            if (!SMTCell::ifAssigned(tmp_str)) {
              tmp_var.push_back(tmp_str);
              SMTCell::setVar(tmp_str, 2);
              cnt_var++;
            } else if (SMTCell::ifAssignedTrue(tmp_str)) {
              SMTCell::setVar_wo_cnt(tmp_str, 0);
              cnt_true++;
            }

            // AGR FLAG
            int currCol = vCoord_FL->col_;
            int distToPOIRow = SMTCell::row_idx_to_real(1);
            int realSHR = SHR_Parameter - abs(currCol - col);

            // for (int shrIndex = 0; shrIndex <= SHR_Parameter - 2; shrIndex++)
            // {
            while (distToPOIRow <= realSHR) {
              tmp_str = fmt::format("GL_V_{}", vCoord_FL->getVName());
              // check declaration
              if (!SMTCell::ifAssigned(tmp_str)) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog, "d       => Adding Forbidden SHR F : {} \n",
                             tmp_str);
                }
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }

              // if (shrIndex != (SHR_Parameter - 2)) {
              // iterate to the right
              vCoord_FL = SMTCell::getVertex((*vCoord_FL))->getFrontADJ();
              // fmt::print("iterate {}", vName_FR)
              if (!vCoord_FL->ifValid()) {
                break;
              } else {
                distToPOIRow += SMTCell::row_idx_to_real(1);
              }
              // }
            }

            if (cnt_true > 0) {
              if (cnt_true > 1) {
                fmt::print("[ERROR] SHR : more than one G Variables are "
                           "true!!!\n");
                exit(1);
              } else {
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::assignTrueVar(tmp_var[i], 0, true);
                }
              }
            } else {
              if (cnt_var > 1) {
                SMTCell::writeConstraint("(assert ((_ at-most 1)");
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::writeConstraint(fmt::format(" {}", tmp_var[i]));
                  SMTCell::cnt("l", 3);
                }
                SMTCell::writeConstraint("))\n");
                SMTCell::cnt("c", 3);
              }
            }
          }

          // # FR Direction Checking
          // Triplet *vCoord_FR =
          //     SMTCell::getVertex(metal, row, col)->getFrontRightADJ();
          std::shared_ptr<Triplet> vCoord_FR =
              SMTCell::getVertex(metal, row, col)->getFrontRightADJ();

          if (SMTCell::DEBUG_MODE_) {
            fmt::print(drlog, "d        vCoord_FR: {} \n",
                       vCoord_FR->getVName());
          }

          if (vCoord_FR->ifValid()) {
            std::string tmp_str = "";
            std::vector<std::string> tmp_var;
            int cnt_var = 0;
            int cnt_true = 0;

            tmp_str =
                fmt::format("GL_V_{}", SMTCell::getVertexName(metal, row, col));

            // check declaration
            if (!SMTCell::ifAssigned(tmp_str)) {
              tmp_var.push_back(tmp_str);
              SMTCell::setVar(tmp_str, 2);
              cnt_var++;
            } else if (SMTCell::ifAssignedTrue(tmp_str)) {
              SMTCell::setVar_wo_cnt(tmp_str, 0);
              cnt_true++;
            }

            // AGR FLAG
            int currCol = vCoord_FR->col_;
            int distToPOIRow = SMTCell::row_idx_to_real(1);
            int realSHR = SHR_Parameter - abs(currCol - col);

            // for (int shrIndex = 0; shrIndex <= SHR_Parameter - 2; shrIndex++)
            // {
            while (distToPOIRow <= realSHR) {
              tmp_str = fmt::format("GL_V_{}", vCoord_FR->getVName());
              // check declaration
              if (!SMTCell::ifAssigned(tmp_str)) {
                // DEBUG MODE
                if (SMTCell::DEBUG_MODE_) {
                  fmt::print(drlog, "d       => Adding Forbidden SHR F : {} \n",
                             tmp_str);
                }
                tmp_var.push_back(tmp_str);
                SMTCell::setVar(tmp_str, 2);
                cnt_var++;
              } else if (SMTCell::ifAssignedTrue(tmp_str)) {
                SMTCell::setVar_wo_cnt(tmp_str, 0);
                cnt_true++;
              }

              // if (shrIndex != (SHR_Parameter - 2)) {
              // iterate to the right
              vCoord_FR = SMTCell::getVertex((*vCoord_FR))->getFrontADJ();
              if (vCoord_FR->ifValid() == false) {
                break;
              } else {
                distToPOIRow += SMTCell::row_idx_to_real(1);
              }
              // }
            }

            if (cnt_true > 0) {
              if (cnt_true > 1) {
                fmt::print("[ERROR] SHR : more than one G Variables are "
                           "true!!!\n");
                exit(1);
              } else {
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::assignTrueVar(tmp_var[i], 0, true);
                }
              }
            } else {
              if (cnt_var > 1) {
                SMTCell::writeConstraint("(assert ((_ at-most 1)");
                for (std::size_t i = 0; i < tmp_var.size(); i++) {
                  SMTCell::writeConstraint(fmt::format(" {}", tmp_var[i]));
                  SMTCell::cnt("l", 3);
                }
                SMTCell::writeConstraint("))\n");
                SMTCell::cnt("c", 3);
              }
            }
          }
        }
      }
    }
  }
}