#define FMT_HEADER_ONLY
#include "graph.hpp"
#include "SMTCell.hpp"
#include "dbConfig.hpp"
#include <fmt/core.h>
#include <fmt/format.h>

void Graph::init_graph() {
  // store numTrackV and numTrackH for each layer
  std::map<int, int> h_metal_numTrackV;
  std::map<int, int> h_metal_numTrackH;

  // store [METAL] ==> [ROW]
  std::map<int, std::vector<int>> h_metal_row;
  // store [METAL] ==> [COL]
  std::map<int, std::vector<int>> h_metal_col;

  // store [METAL] ==> [ROW] ==> [ROW_IDX]
  std::map<int, std::map<int, int>> h_metal_row_idx;
  // store [METAL] ==> [COL] ==> [COL_IDX]
  std::map<int, std::map<int, int>> h_metal_col_idx;

  // $newNumTrackV = ($numTrackV-1)*$verticalSubGrid;

  // store rows/cols skip factor
  std::vector<int> temp_cols;
  std::vector<int> temp_rows;

  for (int metal = 1; metal <= SMTCell::getNumMetalLayer(); metal++) {
    // clear temp
    temp_cols.clear();
    temp_rows.clear();

    // get current metal layer
    int curMP = SMTCell::getMetalPitch(metal);

    // ***** Placement Layer *****
    if (metal == 1) {
      // M2 Metal Offset
      int horzOffset = SMTCell::getOffset(metal + 1);
      // M1 Metal Offset
      int vertOffset = SMTCell::getOffset(metal);

      // M2 Metal Pitch
      int uppMP = SMTCell::getMetalPitch(metal + 1);
      // No lower layer
      // int lowMP = 0;

      // init rows/cols
      int row = horzOffset;
      int col = vertOffset;

      while (col <= SMTCell::getCellWidth()) {
        // reset row
        row = horzOffset;
        temp_cols.push_back(col);
        while (row <= SMTCell::getCellHeight()) {
          // if row is not already added
          if (std::find(temp_rows.begin(), temp_rows.end(), row) ==
              temp_rows.end()) {
            temp_rows.push_back(row);
          }
          row += uppMP; // jump to next row
        }
        col += curMP; // jump to next col
      }
    }
    // ***** Top-Most Layer *****
    else if (metal == SMTCell::getNumMetalLayer()) {
      // Top layer is odd (vertical, col-based)
      if (SMTCell::ifVertMetal(metal)) {
        // Top - 1 Metal Offset
        int horzOffset = SMTCell::getOffset(metal - 1);
        // Top Metal Offset
        int vertOffset = SMTCell::getOffset(metal);

        // Top - 1 Metal Pitch
        int lowMP = SMTCell::getMetalPitch(metal - 1);
        // No upper layer
        // int uppMP = 0;

        // init rows/cols
        int row = horzOffset;
        int col = vertOffset;

        // Top layer is odd (vertical, col-based)
        while (col <= SMTCell::getCellWidth()) {
          // reset row
          row = horzOffset;
          temp_cols.push_back(col);
          while (row <= SMTCell::getCellHeight()) {
            // if row is not already added
            if (std::find(temp_rows.begin(), temp_rows.end(), row) ==
                temp_rows.end()) {
              temp_rows.push_back(row);
            }
            row += lowMP; // jump to next row
          }
          col += curMP; // jump to next col
        }
      }
      // Top layer is even (horizontal, row-based)
      else if (SMTCell::ifHorzMetal(metal)) {
        int horzOffset = SMTCell::getOffset(metal);
        int vertOffset = SMTCell::getOffset(metal - 1);

        // M3 Metal Pitch
        int lowMP = SMTCell::getMetalPitch(metal - 1);
        // No upper layer
        // int uppMP = 0;

        // init rows/cols
        int row = horzOffset;
        int col = vertOffset;

        // Top layer is even (horizontal, row-based)
        while (row <= SMTCell::getCellHeight()) {
          // reset col
          col = vertOffset;
          temp_rows.push_back(row);
          while (col <= SMTCell::getCellWidth()) {
            // if col is not already added
            if (std::find(temp_cols.begin(), temp_cols.end(), col) ==
                temp_cols.end()) {
              temp_cols.push_back(col);
            }
            col += lowMP; // jump to next col
          }
          row += curMP; // jump to next row
        }
      }
    }
    // ***** Vertical Middle Layer *****
    else if (SMTCell::ifVertMetal(metal)) {
      // Vertical Middle Layer Offset
      int vertOffset = SMTCell::getOffset(metal);
      // Horizontal Middle Layer Offset
      int uppHorzOffset = SMTCell::getOffset(metal + 1);
      // Horizontal Middle Layer Offset
      int lowHorzOffset = SMTCell::getOffset(metal - 1);

      // M + 1, horziontal metal pitch
      int uppMP = SMTCell::getMetalPitch(metal + 1);
      // M - 1, horziontal metal pitch
      int lowMP = SMTCell::getMetalPitch(metal - 1);

      // init rows/cols
      int col = vertOffset;
      int upRow = uppHorzOffset;
      int lowRow = lowHorzOffset;

      while (col <= SMTCell::getCellWidth()) {
        // reset row
        upRow = uppHorzOffset;
        lowRow = lowHorzOffset;
        temp_cols.push_back(col);
        while (upRow <= SMTCell::getCellHeight()) {
          // if row is not already added
          if (std::find(temp_rows.begin(), temp_rows.end(), upRow) ==
              temp_rows.end()) {
            temp_rows.push_back(upRow);
          }
          upRow += uppMP; // jump to next row
        }
        while (lowRow <= SMTCell::getCellHeight()) {
          // if row is not already added
          if (std::find(temp_rows.begin(), temp_rows.end(), lowRow) ==
              temp_rows.end()) {
            temp_rows.push_back(lowRow);
          }
          lowRow += lowMP; // jump to next row
        }
        col += curMP; // jump to next col
      }
    }
    // ***** Horizontal Middle Layer *****
    else if (SMTCell::ifHorzMetal(metal)) {
      int horzOffset = SMTCell::getOffset(metal);
      int uppVertOffset = SMTCell::getOffset(metal + 1);
      int lowVertOffset = SMTCell::getOffset(metal - 1);

      // M + 1, vertical metal pitch
      int uppMP = SMTCell::getMetalPitch(metal + 1);
      // M - 1, vertical metal pitch
      int lowMP = SMTCell::getMetalPitch(metal - 1);

      // init rows/cols
      int row = horzOffset;
      int upCol = uppVertOffset;
      int lowCol = lowVertOffset;

      while (row <= SMTCell::getCellHeight()) {
        // reset col
        upCol = uppVertOffset;
        lowCol = lowVertOffset;
        temp_rows.push_back(row);
        while (upCol <= SMTCell::getCellWidth()) {
          // if col is not already added
          if (std::find(temp_cols.begin(), temp_cols.end(), upCol) ==
              temp_cols.end()) {
            temp_cols.push_back(upCol);
          }
          upCol += uppMP; // jump to next col
        }
        while (lowCol <= SMTCell::getCellWidth()) {
          // if col is not already added
          if (std::find(temp_cols.begin(), temp_cols.end(), lowCol) ==
              temp_cols.end()) {
            temp_cols.push_back(lowCol);
          }
          lowCol += lowMP; // jump to next col
        }
        row += curMP; // jump to next row
      }
    }

    // sort temp_rows (ascending)
    std::sort(temp_rows.begin(), temp_rows.end());
    // sort temp_cols (ascending)
    std::sort(temp_cols.begin(), temp_cols.end());

    // map number of metal tracks
    SMTCell::assign_h_metal_numTrackV(metal, temp_cols.size());
    SMTCell::assign_h_metal_numTrackH(metal, temp_rows.size());

    // map metal rows/cols (excluding pwr rail)
    SMTCell::assign_h_metal_col(metal, temp_cols);
    SMTCell::assign_h_metal_row(metal, temp_rows);

    // map metal rows/cols index
    std::map<int, int> temp_map_row_to_idx;
    std::map<int, int> temp_map_col_to_idx;
    // assign row --> row_idx
    for (std::size_t i = 0; i < temp_rows.size() - 2; i++) {
      temp_map_row_to_idx[temp_rows[i]] = i;
    }
    // assign col --> col_idx
    for (std::size_t i = 0; i < temp_cols.size(); i++) {
      temp_map_col_to_idx[temp_cols[i]] = i;
    }

    SMTCell::assign_h_metal_col_idx(metal, temp_map_col_to_idx);
    SMTCell::assign_h_metal_row_idx(metal, temp_map_row_to_idx);
  }
}

void Graph::dump_graph(FILE *graphLog) {
  // DEBUG MODE
  if (SMTCell::DEBUG()) {
    fmt::print(graphLog,
               "d     ############################## DEBUG MODE: VERTEX "
               "##############################\n");
    fmt::print(graphLog, "      [h_metal_row]\n");
    SMTCell::dump_h_metal_row(graphLog);

    fmt::print(graphLog, "      [h_metal_col]\n");
    SMTCell::dump_h_metal_col(graphLog);

    fmt::print(graphLog, "      [h_metal_row_idx]\n");
    SMTCell::dump_h_metal_row_idx(graphLog);

    fmt::print(graphLog, "      [h_metal_col_idx]\n");
    SMTCell::dump_h_metal_col_idx(graphLog);

    fmt::print(graphLog, "      [h_metal_numTrackV]\n");
    SMTCell::dump_h_metal_numTrackV(graphLog);

    fmt::print(graphLog, "      [h_metal_numTrackH]\n");
    SMTCell::dump_h_metal_numTrackH(graphLog);

    fmt::print(graphLog,
               "d     ############################## DEBUG MODE: VERTEX "
               "##############################\n");
  }
}

void Graph::init_agr_vertices() {
  int vIndex = 0;
  // ### VERTEX Generation
  // ### VERTEX Variables
  for (int metal = 1; metal <= SMTCell::getNumMetalLayer(); metal++) {
    for (int row_idx = 0; row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3;
         row_idx++) {
      for (int col_idx = 0;
           col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {

        std::vector<std::shared_ptr<Triplet>> tmp_vADJ;
        std::shared_ptr<Triplet> vL_p, vR_p, vF_p, vB_p, vU_p, vD_p, vFL_p,
            vFR_p, vBL_p, vBR_p;
        // std::vector<Triplet *> tmp_vADJ;
        // Triplet *vL_p, *vR_p, *vF_p, *vB_p, *vU_p, *vD_p, *vFL_p, *vFR_p,
        //     *vBL_p, *vBR_p;

        // retrieve row/col
        int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
        int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);

        Triplet *tmp_coord_p =
            new Triplet(metal, row, col); // a ptr to tmp coordinate

        // ### Left Vertex
        if (col == SMTCell::get_h_metal_col_by_idx(metal, 0)) {
          // vL_p = new Triplet();
          vL_p = std::make_shared<Triplet>();
        } else {
          // vL_p = new Triplet(
          //     metal, row,
          //     SMTCell::SMTCell::get_h_metal_col_by_idx(metal, col_idx - 1));
          vL_p = std::make_shared<Triplet>(
              metal, row,
              SMTCell::SMTCell::get_h_metal_col_by_idx(metal, col_idx - 1));
        }

        // ### Right Vertex
        if (col == SMTCell::get_h_metal_col_by_idx(
                       metal, SMTCell::get_h_metal_numTrackV(metal) - 1)) {
          // vR_p = new Triplet();
          vR_p = std::make_shared<Triplet>();
        } else {
          // vR_p = new Triplet(
          //     metal, row,
          //     SMTCell::SMTCell::get_h_metal_col_by_idx(metal, col_idx + 1));
          vR_p = std::make_shared<Triplet>(
              metal, row,
              SMTCell::SMTCell::get_h_metal_col_by_idx(metal, col_idx + 1));
        }

        // ### Front Vertex
        if (row == SMTCell::get_h_metal_row_by_idx(metal, 0)) {
          // vF_p = new Triplet();
          vF_p = std::make_shared<Triplet>();
        } else {
          // vF_p = new Triplet(
          //     metal, SMTCell::get_h_metal_row_by_idx(metal, row_idx - 1),
          //     col);
          vF_p = std::make_shared<Triplet>(
              metal, SMTCell::get_h_metal_row_by_idx(metal, row_idx - 1), col);
        }

        // ### Back Vertex
        if (row == SMTCell::get_h_metal_row_by_idx(
                       metal, SMTCell::get_h_metal_numTrackH(metal) - 3)) {
          // vB_p = new Triplet();
          vB_p = std::make_shared<Triplet>();
        } else {
          // vB_p = new Triplet(
          //     metal, SMTCell::get_h_metal_row_by_idx(metal, row_idx + 1),
          //     col);
          vB_p = std::make_shared<Triplet>(
              metal, SMTCell::get_h_metal_row_by_idx(metal, row_idx + 1), col);
        }

        // ### Up Vertex
        if (metal == SMTCell::getNumMetalLayer()) {
          // vU_p = new Triplet();
          vU_p = std::make_shared<Triplet>();
        } else {
          // need to make sure upper metal layer has the same row/col
          if (SMTCell::ifRowExist(metal + 1, row) &&
              SMTCell::ifColExist(metal + 1, col)) {
            // vU_p = new Triplet(metal + 1, row, col);
            vU_p = std::make_shared<Triplet>(metal + 1, row, col);
          } else {
            // vU_p = new Triplet();
            vU_p = std::make_shared<Triplet>();
          }
        }

        // ### Down Vertex
        if (metal == 1) {
          // vD_p = new Triplet();
          vD_p = std::make_shared<Triplet>();
        } else {
          // need to make sure upper metal layer has the same row/col
          if (SMTCell::ifRowExist(metal - 1, row) &&
              SMTCell::ifColExist(metal - 1, col)) {
            // vD_p = new Triplet(metal - 1, row, col);
            vD_p = std::make_shared<Triplet>(metal - 1, row, col);
          } else {
            // vD_p = new Triplet();
            vD_p = std::make_shared<Triplet>();
          }
        }

        // ### Front-Left Vertex
        if (row == SMTCell::get_h_metal_row_by_idx(metal, 0) ||
            col == SMTCell::get_h_metal_col_by_idx(metal, 0)) {
          // vFL_p = new Triplet();
          vFL_p = std::make_shared<Triplet>();
        } else {
          // vFL_p = new Triplet(
          //     metal, SMTCell::get_h_metal_row_by_idx(metal, row_idx - 1),
          //     SMTCell::get_h_metal_col_by_idx(metal, col_idx - 1));
          vFL_p = std::make_shared<Triplet>(
              metal, SMTCell::get_h_metal_row_by_idx(metal, row_idx - 1),
              SMTCell::get_h_metal_col_by_idx(metal, col_idx - 1));
        }

        // ### Front-Right Vertex
        if (row == SMTCell::get_h_metal_row_by_idx(metal, 0) ||
            col == SMTCell::get_h_metal_col_by_idx(
                       metal, SMTCell::get_h_metal_numTrackV(metal) - 1)) {
          // vFR_p = new Triplet();
          vFR_p = std::make_shared<Triplet>();
        } else {
          // vFR_p = new Triplet(
          //     metal, SMTCell::get_h_metal_row_by_idx(metal, row_idx - 1),
          //     SMTCell::get_h_metal_col_by_idx(metal, col_idx + 1));
          vFR_p = std::make_shared<Triplet>(
              metal, SMTCell::get_h_metal_row_by_idx(metal, row_idx - 1),
              SMTCell::get_h_metal_col_by_idx(metal, col_idx + 1));
        }

        // ### Back-Left Vertex
        if (row == SMTCell::get_h_metal_row_by_idx(
                       metal, SMTCell::get_h_metal_numTrackH(metal) - 3) ||
            col == SMTCell::get_h_metal_col_by_idx(metal, 0)) {
          // vBL_p = new Triplet();
          vBL_p = std::make_shared<Triplet>();
        } else {
          // vBL_p = new Triplet(
          //     metal, SMTCell::get_h_metal_row_by_idx(metal, row_idx + 1),
          //     SMTCell::get_h_metal_col_by_idx(metal, col_idx - 1));
          vBL_p = std::make_shared<Triplet>(
              metal, SMTCell::get_h_metal_row_by_idx(metal, row_idx + 1),
              SMTCell::get_h_metal_col_by_idx(metal, col_idx - 1));
        }

        // ### Back-Right Vertex
        if (row == SMTCell::get_h_metal_row_by_idx(
                       metal, SMTCell::get_h_metal_numTrackH(metal) - 3) ||
            col == SMTCell::get_h_metal_col_by_idx(
                       metal, SMTCell::get_h_metal_numTrackV(metal) - 1)) {
          // vBR_p = new Triplet();
          vBR_p = std::make_shared<Triplet>();
        } else {
          // vBR_p = new Triplet(
          //     metal, SMTCell::get_h_metal_row_by_idx(metal, row_idx + 1),
          //     SMTCell::get_h_metal_col_by_idx(metal, col_idx + 1));
          vBR_p = std::make_shared<Triplet>(
              metal, SMTCell::get_h_metal_row_by_idx(metal, row_idx + 1),
              SMTCell::get_h_metal_col_by_idx(metal, col_idx + 1));
        }

        tmp_vADJ = {vL_p, vR_p,  vF_p,  vB_p,  vU_p,
                    vD_p, vFL_p, vFR_p, vBL_p, vBR_p};
        // SMTCell::addVertex((*tmp_coord_p),
        //                    new Vertex(vIndex, tmp_coord_p, tmp_vADJ));
        SMTCell::addVertex((*tmp_coord_p), std::make_shared<Vertex>(
                                               vIndex, tmp_coord_p, tmp_vADJ));
        vIndex++;
      }
    }
  }

  // AGR FLAG : merge all unique tracks
  SMTCell::mergeAllUniqueCols();
}

void Graph::init_agr_udedges() {
  // ### UNDIRECTED EDGE Generation
  // ### UNDIRECTED EDGE Variables
  int udEdgeIndex = 0;
  // int udEdgeNumber = -1;
  int viaCost = 50;
  // int metalCost = 1;
  int viaCost_1 = 50;
  // int metalCost_1 = 1;
  int viaCost_34 = 50;
  // int metalCost_4 = 1;
  // int wireCost = 1;

  std::shared_ptr<Triplet> udEdgeTerm1_p;
  std::shared_ptr<Triplet> udEdgeTerm2_p;

  for (int metal = 1; metal <= SMTCell::getNumMetalLayer(); metal++) {
    for (int row_idx = 0; row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3;
         row_idx++) {
      for (int col_idx = 0;
           col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
        // retrieve row/col
        int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
        int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);

        udEdgeTerm1_p = std::make_shared<Triplet>(metal, row, col);
        // # Even Layers ==> Horizontal
        if (SMTCell::ifHorzMetal(metal)) {
          // # Right Edge
          udEdgeTerm2_p = SMTCell::getVertex((*udEdgeTerm1_p))->getRightADJ();
          if (udEdgeTerm2_p->ifValid()) {
            if (metal == 4) {
              int mcost = udEdgeTerm2_p->col_ - udEdgeTerm1_p->col_;
              // SMTCell::addUdEdge(new UdEdge(udEdgeIndex, udEdgeTerm1_p,
              //                               udEdgeTerm2_p, metalCost_4,
              //                               wireCost));
              SMTCell::addUdEdge(new UdEdge(udEdgeIndex, udEdgeTerm1_p,
                                            udEdgeTerm2_p, mcost, mcost));
            } else {
              int mcost = udEdgeTerm2_p->col_ - udEdgeTerm1_p->col_;
              // SMTCell::addUdEdge(new UdEdge(udEdgeIndex, udEdgeTerm1_p,
              //                               udEdgeTerm2_p, metalCost,
              //                               wireCost));
              SMTCell::addUdEdge(new UdEdge(udEdgeIndex, udEdgeTerm1_p,
                                            udEdgeTerm2_p, mcost, mcost));
            }
            udEdgeIndex++;
          }
          // # Up Edge
          udEdgeTerm2_p = SMTCell::getVertex((*udEdgeTerm1_p))->getUpADJ();
          if (udEdgeTerm2_p->ifValid()) {
            if (SMTCell::ifGCol_AGR(metal, col)) {
              SMTCell::addUdEdge(new UdEdge(udEdgeIndex, udEdgeTerm1_p,
                                            udEdgeTerm2_p, viaCost, viaCost));
              udEdgeIndex++;
            }
          }
        }
        // # Odd Layers ==> Vertical
        else {
          // # Back Edge
          udEdgeTerm2_p = SMTCell::getVertex((*udEdgeTerm1_p))->getBackADJ();
          if (udEdgeTerm2_p->ifValid()) {
            if (metal == 3) {
              // SMTCell::addUdEdge(new UdEdge(udEdgeIndex, udEdgeTerm1_p,
              //                               udEdgeTerm2_p, metalCost,
              //                               wireCost));
              // AGR FLAG : hard setting to 24 as horizontal pitch
              int mcost = (udEdgeTerm2_p->row_ - udEdgeTerm1_p->row_) * 24;
              SMTCell::addUdEdge(new UdEdge(udEdgeIndex, udEdgeTerm1_p,
                                            udEdgeTerm2_p, mcost, mcost));

            } else {
              // SMTCell::addUdEdge(new UdEdge(udEdgeIndex, udEdgeTerm1_p,
              //                               udEdgeTerm2_p, metalCost_1,
              //                               wireCost));
              int mcost = (udEdgeTerm2_p->row_ - udEdgeTerm1_p->row_) * 24;
              SMTCell::addUdEdge(new UdEdge(udEdgeIndex, udEdgeTerm1_p,
                                            udEdgeTerm2_p, mcost, mcost));
            }
            udEdgeIndex++;
          }

          // # Up Edge
          udEdgeTerm2_p = SMTCell::getVertex((*udEdgeTerm1_p))->getUpADJ();
          if (udEdgeTerm2_p->ifValid()) {
            if (metal == 1) {
              SMTCell::addUdEdge(new UdEdge(udEdgeIndex, udEdgeTerm1_p,
                                            udEdgeTerm2_p, viaCost_1, viaCost));
              udEdgeIndex++;
            }
            // AGR FLAG : No need to be on G col
            else {
              SMTCell::addUdEdge(new UdEdge(udEdgeIndex, udEdgeTerm1_p,
                                            udEdgeTerm2_p, viaCost_34,
                                            viaCost));
              udEdgeIndex++;
            }
          }
        }
      }
    }
  }
}

void Graph::init_source(bool SON) {
  // std::vector<Triplet *> tmp_subNodes;
  int numSubNodes;
  // int numSources;
  int outerPinFlagSource = 0;
  int outerPinFlagSink = 0;
  std::string keyValue;

  for (int i = 0; i < SMTCell::getPinCnt(); i++) {
    // tmp_subNodes.clear();
    // std::vector<Triplet *> tmp_subNodes;
    std::vector<std::shared_ptr<Triplet>> tmp_subNodes;
    if (SMTCell::getPin(i)->getPinIO() == Pin::SOURCE_NET) { // source
      if (SMTCell::getPin(i)->getPinLength() == -1) {
        if (SON == 1) {
          if (outerPinFlagSource == 0) {
            // copy boundary vertices from begin to end
            // subNodes.assign(boundaryVertices.begin(),
            // boundaryVertices.end());
            tmp_subNodes = SMTCell::copyBoundaryVertex();
            outerPinFlagSource = 1;
            keyValue = SMTCell::getKeySON();
          } else {
            continue;
          }
        } else { // SON Disable
                 // copy boundary vertices from begin to end
          tmp_subNodes = SMTCell::copyBoundaryVertex();
          keyValue = SMTCell::getPin(i)->getPinName();
        }
      } else {
        for (int j = 0; j < SMTCell::getPin(i)->getPinLength(); j++) {
          // Bug Fix
          // tmp_subNodes.push_back(new Triplet(
          //     1, SMTCell::getPin(i)->getPinYpos()[j],
          //     SMTCell::getPin(i)->getPinXpos() * SMTCell::getMetalPitch(1)));
          tmp_subNodes.push_back(std::make_shared<Triplet>(
              1, SMTCell::getPin(i)->getPinYpos()[j],
              SMTCell::getPin(i)->getPinXpos() * SMTCell::getMetalPitch(1)));
        }
        keyValue = SMTCell::getPin(i)->getPinName();
      }
      numSubNodes = tmp_subNodes.size();

      // Outer Pin should be at last in the input File Format
      SMTCell::addSource(
          keyValue, std::make_shared<Source>(SMTCell::getPin(i)->getNetID(),
                                             numSubNodes, tmp_subNodes));
    } else if (SMTCell::getPin(i)->getPinIO() == Pin::SINK_NET) { // sink
      if (SMTCell::getPin(i)->getPinLength() == -1) {
        if (SON == 1) {
          if (outerPinFlagSink == 0) {
            // copy boundary vertices from begin to end
            tmp_subNodes = SMTCell::copyBoundaryVertex();
            outerPinFlagSink = 1;
            keyValue = SMTCell::getKeySON();
          } else {
            continue;
          }
        } else {
          // copy boundary vertices from begin to end
          tmp_subNodes = SMTCell::copyBoundaryVertex();
          keyValue = SMTCell::getPin(i)->getPinName();
        }
      } else {
        for (int j = 0; j < SMTCell::getPin(i)->getPinLength(); j++) {
          // Bug Fix
          // tmp_subNodes.push_back(new Triplet(
          //     1, SMTCell::getPin(i)->getPinYpos()[j],
          //     SMTCell::getPin(i)->getPinXpos() * SMTCell::getMetalPitch(1)));
          tmp_subNodes.push_back(std::make_shared<Triplet>(
              1, SMTCell::getPin(i)->getPinYpos()[j],
              SMTCell::getPin(i)->getPinXpos() * SMTCell::getMetalPitch(1)));
        }
        keyValue = SMTCell::getPin(i)->getPinName();
      }
      numSubNodes = tmp_subNodes.size();

      SMTCell::addSink(keyValue,
                       std::make_shared<Sink>(SMTCell::getPin(i)->getNetID(),
                                              numSubNodes, tmp_subNodes));

      // free memory
      // for (auto &v : tmp_subNodes) {
      //   delete v;
      // }
    }
  }
}

void Graph::init_agr_boundaryVertices(bool EXT_Parameter) {
  // int numBoundaries = 0;

  // ### Normal External Pins - Top&top-1 layer only
  for (int metal = SMTCell::getNumMetalLayer() - 1;
       metal <= SMTCell::getNumMetalLayer(); metal++) {
    for (int row_idx = 0; row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3;
         row_idx++) {
      for (int col_idx = 0;
           col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
        // retrieve row/col
        int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
        int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);

        if (SMTCell::ifHorzMetal(metal)) {
          continue;
        }

        if (EXT_Parameter == 0) {
          // AGR FLAG
          if (row_idx == 1 ||
              row_idx == SMTCell::get_h_metal_numTrackH(metal) - 4) {
            // SMTCell::addBoundaryVertex(new Triplet(metal, row, col));
            SMTCell::addBoundaryVertex(
                std::make_shared<Triplet>(metal, row, col));
          }
        } else {
          // SMTCell::addBoundaryVertex(new Triplet(metal, row, col));
          SMTCell::addBoundaryVertex(
              std::make_shared<Triplet>(metal, row, col));
        }
      }
    }
  }
}

void Graph::init_outerPins() {
  int commodityInfo = -1;
  // this needs to migrate to SMTCell
  for (int i = 0; i < SMTCell::getPinCnt(); i++) {
    // fmt::print("Pin {} is being initialized\n", i);
    // Pin *p = SMTCell::getPin(i);
    std::shared_ptr<Pin> p = SMTCell::getPin(i);
    if (p->getPinLength() == -1) {
      // # Initializing
      commodityInfo = -1;
      // # Find Commodity Infomation
      for (int j = 0; j < SMTCell::getNetCnt(); j++) {
        // Net *n = SMTCell::getNet(j);
        std::shared_ptr<Net> n = SMTCell::getNet(j);
        if (n->getNetName() == p->getNetID()) {
          for (int sinkIndexofNet = 0; sinkIndexofNet < n->getNumSinks();
               sinkIndexofNet++) {
            // std::cout << sinkIndexofNet << std::endl;
            if (n->getSinks_inNet(sinkIndexofNet) == p->getPinName()) {
              commodityInfo = sinkIndexofNet;
            }
          }
        }
      }

      if (commodityInfo == -1) {
        std::cerr << "[ERROR] Cannot Find the commodity "
                     "Information!!\n\n";
      }

      SMTCell::addOuterPin(
          new OuterPin(p->getPinName(), p->getNetID(), commodityInfo));
    }
  }
}

void Graph::init_agr_corner() {
  // ### (LEFT | RIGHT | FRONT | BACK) CORNER VERTICES Generation
  // # At the top-most metal layer, only vias exist.
  for (int metal = 1; metal <= SMTCell::getNumMetalLayer(); metal++) {
    for (int row_idx = 0; row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3;
         row_idx++) {
      for (int col_idx = 0;
           col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1; col_idx++) {
        // retrieve row/col
        int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
        int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);

        if (metal == 1 && SMTCell::ifSDCol_AGR(metal, col)) {
          continue;
        }
        // else if (SMTCell::ifVertMetal(metal) &&
        //            SMTCell::ifSDCol_AGR(metal, col)) {
        //   continue;
        // }

        // Triplet *cornerVertex = new Triplet(metal, row, col);
        std::shared_ptr<Triplet> cornerVertex =
            std::make_shared<Triplet>(metal, row, col);

        // AGR FLAG
        if (col_idx == 0) {
          SMTCell::addLeftCorner(cornerVertex);
        }

        if (col_idx == SMTCell::get_h_metal_numTrackV(metal) - 1) {
          SMTCell::addRightCorner(cornerVertex);
        }

        if (row_idx == 0) {
          SMTCell::addFrontCorner(cornerVertex);
        }

        if (row_idx == SMTCell::get_h_metal_numTrackH(metal) - 3) {
          SMTCell::addBackCorner(cornerVertex);
        }
      }
    }
  }
}

/**
 * Abstract all external pins as pinSON.
 *
 * @param SON if using Super Outer Node
 * @return void
 */
void Graph::init_pinSON(bool SON) {
  if (SON == 1) {
    // Pin Information Modification
    for (int pinIndex = 0; pinIndex < SMTCell::getPinCnt(); pinIndex++) {
      for (int outerPinIndex = 0; outerPinIndex < SMTCell::getOuterPinCnt();
           outerPinIndex++) {
        if (SMTCell::getPin(pinIndex)->getPinName() ==
            SMTCell::getOuterPin(outerPinIndex)->getPinName()) {

          SMTCell::getPin(pinIndex)->setPinName(Pin::keySON);
          SMTCell::getPin(pinIndex)->setNetID(Net::NetSON);
          continue;
        }
      }
    }
    // SON Node should be last elements to pop
    int SONFlag = 0;
    int tmp_cnt = SMTCell::getPinCnt() - 1;
    std::shared_ptr<Pin> pin;
    for (int i = 0; i <= tmp_cnt; i++) {
      if (SMTCell::getPin(tmp_cnt - i)->getPinName() == Pin::keySON) {
        SONFlag = 1;
        // pin = SMTCell::popLastPin();
        pin = SMTCell::popLastPin();
      }
    }
    if (SONFlag == 1) {
      // SMTCell::addPin(pin);
      SMTCell::addPin(pin);
    }
  }

  // Net Information Modification
  for (int netIndex = 0; netIndex < SMTCell::getNetCnt(); netIndex++) {
    for (int sinkIndex = 0;
         sinkIndex < SMTCell::getNet(netIndex)->getNumSinks(); sinkIndex++) {
      for (int outerPinIndex = 0; outerPinIndex < SMTCell::getOuterPinCnt();
           outerPinIndex++) {
        if (SMTCell::getNet(netIndex)->getSinks_inNet(sinkIndex) ==
            SMTCell::getOuterPin(outerPinIndex)->getPinName()) {
          SMTCell::getNet(netIndex)->setSinks_inNet(sinkIndex, Pin::keySON);
          continue;
        }
      }
    }
    for (int pinIndex = 0; pinIndex < SMTCell::getNet(netIndex)->getN_pinNets();
         pinIndex++) {
      for (int outerPinIndex = 0; outerPinIndex < SMTCell::getOuterPinCnt();
           outerPinIndex++) {
        if (SMTCell::getNet(netIndex)->getPins_inNet(pinIndex) ==
            SMTCell::getOuterPin(outerPinIndex)->getPinName()) {
          SMTCell::getNet(netIndex)->setPins_inNet(pinIndex, Pin::keySON);
          continue;
        }
      }
    }
  }
}

/**
 * Associate grid vertices with pins.
 *
 * @param void
 * @return void
 */
void Graph::init_agr_virtualEdges() {
  int vEdgeIndex = 0;
  // int vEdgeNumber = 0;
  int virtualCost = 0;
  for (int pinIdx = 0; pinIdx < SMTCell::getPinCnt(); pinIdx++) {
    // internal pin: either net source or sink
    if (SMTCell::ifPinInst(pinIdx) &&
        ((SMTCell::getPin(pinIdx)->getPinIO() == Pin::SOURCE_NET &&
          SMTCell::ifSource(pinIdx)) ||
         (SMTCell::getPin(pinIdx)->getPinIO() == Pin::SINK_NET &&
          SMTCell::ifSink(pinIdx)))) {
      std::vector<int> tmp_finger = SMTCell::getAvailableNumFinger(
          SMTCell::getPinInstWidth(pinIdx), SMTCell::getTrackEachPRow());
      // PMOS
      if (SMTCell::ifPinInstPMOS(pinIdx)) {
        int upRow_idx = SMTCell::getRoutingTrack(
            SMTCell::getNumPTrackH() - 1 -
            SMTCell::getConn(SMTCell::getPinInstWidth(pinIdx) / tmp_finger[0]));
        int lowRow_idx = SMTCell::getRoutingTrack(SMTCell::getNumPTrackH() - 1);

        // AGR FLAG: fix M1 since we are mapping placement pins
        int metal = 1;
        // AGR FLAG : adapt to different track height
        int beginRow_idx = 0;
        int endRow_idx = -1;
        if (SMTCell::getNumTrack() == 4 || SMTCell::getNumTrack() == 5) {
          endRow_idx =
              floor(SMTCell::get_h_metal_numTrackH(metal) / 2 + 0.5) - 2;
        } else if (SMTCell::getNumTrack() == 6) {
          endRow_idx = SMTCell::get_h_metal_numTrackH(metal) / 2 - 2;
        } else {
          fmt::print(stderr, "[ERROR] unsupported track height\n");
          exit(1);
        }

        for (int row_idx = beginRow_idx; row_idx <= endRow_idx; row_idx++) {
          for (int col_idx = 0;
               col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1;
               col_idx++) {
            // retrieve row/col
            int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
            int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);

            // AGR FLAG
            if (SMTCell::ifRoutingTrack(row_idx) && row_idx <= upRow_idx &&
                row_idx >= lowRow_idx) {
              // ROW  | 1 | 2 | 3 | 4 | 5 |
              // PIN  | S | G | D | G | S |
              // Gate Pin should be connected to even column
              if (SMTCell::getPinType(pinIdx) == Pin::GATE &&
                  SMTCell::ifSDCol_AGR(metal, col)) {
                continue;
              }
              // Source or Drain Pin should be connected to odd column
              else if (SMTCell::getPinType(pinIdx) != Pin::GATE &&
                       SMTCell::ifGCol_AGR(metal, col)) {
                continue;
              }
              // SMTCell::addVirtualEdge(
              //     new VirtualEdge(vEdgeIndex, new Triplet(1, row, col),
              //                     SMTCell::getPinName(pinIdx), virtualCost));
              SMTCell::addVirtualEdge(std::make_shared<VirtualEdge>(
                  vEdgeIndex, std::make_shared<Triplet>(1, row, col),
                  SMTCell::getPinName(pinIdx), virtualCost));
              vEdgeIndex++;
            }
          }
        }
      }
      // NMOS
      else {
        int upRow_idx = SMTCell::getRoutingTrack(0);
        int lowRow_idx = SMTCell::getRoutingTrack(
            SMTCell::getConn(SMTCell::getPinInstWidth(pinIdx) / tmp_finger[0]));
        // // AGR FLAG: fix M1 since we are mapping placement pins
        int metal = 1;
        // // AGR FLAG : adapt to different track height
        // int beginRow_idx = -1;
        // int endRow_idx = SMTCell::get_h_metal_numTrackH(metal) - 3;
        // if (SMTCell::getNumTrack() == 4 || SMTCell::getNumTrack() == 5) {
        //   beginRow_idx =
        //       floor(SMTCell::get_h_metal_numTrackH(metal) / 2 + 0.5) - 1;
        // } else if (SMTCell::getNumTrack() == 6) {
        //   beginRow_idx = SMTCell::get_h_metal_numTrackH(metal) / 2 - 1;
        // } else {
        //   fmt::print(stderr, "[ERROR] unsupported track height\n");
        //   exit(1);
        // }

        for (int row_idx =
                 floor(SMTCell::get_h_metal_numTrackH(metal) / 2 + 0.5) - 1;
             row_idx <= SMTCell::get_h_metal_numTrackH(metal) - 3; row_idx++) {
          for (int col_idx = 0;
               col_idx <= SMTCell::get_h_metal_numTrackV(metal) - 1;
               col_idx++) {
            // retrieve row/col
            int row = SMTCell::get_h_metal_row_by_idx(metal, row_idx);
            int col = SMTCell::get_h_metal_col_by_idx(metal, col_idx);
            if (SMTCell::ifRoutingTrack(row_idx) && row_idx <= upRow_idx &&
                row_idx >= lowRow_idx) {
              // ROW  | 1 | 2 | 3 | 4 | 5 |
              // PIN  | S | G | D | G | S |
              // Gate Pin should be connected to even column
              if (SMTCell::getPinType(pinIdx) == Pin::GATE &&
                  SMTCell::ifSDCol_AGR(metal, col)) {
                continue;
              }
              // Source or Drain Pin should be connected to odd column
              else if (SMTCell::getPinType(pinIdx) != Pin::GATE &&
                       SMTCell::ifGCol_AGR(metal, col)) {
                continue;
              }
              // SMTCell::addVirtualEdge(
              //     new VirtualEdge(vEdgeIndex, new Triplet(1, row, col),
              //                     SMTCell::getPinName(pinIdx), virtualCost));
              SMTCell::addVirtualEdge(std::make_shared<VirtualEdge>(
                  vEdgeIndex, std::make_shared<Triplet>(1, row, col),
                  SMTCell::getPinName(pinIdx), virtualCost));
              vEdgeIndex++;
            }
          }
        }
      }
    }
    // external pin: only net sink
    else if (SMTCell::getPin(pinIdx)->getPinIO() == Pin::SINK_NET &&
             SMTCell::ifSink(pinIdx) && SMTCell::ifSON(pinIdx)) { // sink
      for (int term = 0;
           term <
           SMTCell::getSink(SMTCell::getPinName(pinIdx))->getNumSubNodes();
           term++) {
        // SMTCell::addVirtualEdge(
        //     new VirtualEdge(vEdgeIndex,
        //                     SMTCell::getSink(SMTCell::getPinName(pinIdx))
        //                         ->getBoundaryVertices(term),
        //                     SMTCell::getPinName(pinIdx), virtualCost));
        SMTCell::addVirtualEdge(std::make_shared<VirtualEdge>(
            vEdgeIndex,
            SMTCell::getSink(SMTCell::getPinName(pinIdx))
                ->getBoundaryVertices(term),
            SMTCell::getPinName(pinIdx), virtualCost));
        vEdgeIndex++;
      }
    } else {
      std::cerr << "[ERROR] Virtual Edge Generation : Instance Information"
                   "not found!!"
                << std::endl;
      exit(1);
    }
  }
}

/**
 * Initialize the edge in and out
 *
 * @param void
 * @return void
 */
void Graph::init_edgeInOut() {
  for (int i = 0; i < SMTCell::getUdEdgeCnt(); i++) {
    // EdgeOut: [FROM VAR NAME] -> [EDGE INDEX]
    SMTCell::addEdgeOut(SMTCell::getUdEdgeFromVarName(i), i);
    // EdgeIn:  [TO VAR NAME] -> [EDGE INDEX]
    SMTCell::addEdgeIn(SMTCell::getUdEdgeToVarName(i), i);
  }
}

/**
 * Initialize the virtual edge in and out
 *
 * @param void
 * @return void
 */
void Graph::init_vedgeInOut() {
  for (int i = 0; i < SMTCell::getVirtualEdgeCnt(); i++) {
    // VEdgeOut: [VAR NAME] -> [VEDGE INDEX]
    SMTCell::addVEdgeOut(SMTCell::getVirtualEdge(i)->getVName(), i);
    // VEdgeIn:  [PIN NAME] -> [VEDGE INDEX]
    SMTCell::addVEdgeIn(SMTCell::getVirtualEdge(i)->getPinName(), i);
  }
}

void Graph::init_metal_var() {
  for (int udEdgeIndex = 0; udEdgeIndex < SMTCell::getUdEdgeCnt();
       udEdgeIndex++) {
    std::shared_ptr<Triplet> fromVar =
        SMTCell::getUdEdge(udEdgeIndex)->getUdEdgeFromVar();
    std::shared_ptr<Triplet> toVar =
        SMTCell::getUdEdge(udEdgeIndex)->getUdEdgeToVar();
    int fromCol = fromVar->col_;
    int toCol = toVar->col_;
    int fromMetal = fromVar->metal_;
    int toMetal = toVar->metal_;
    if ((SMTCell::ifSDCol_AGR(fromMetal, fromCol) ||
         SMTCell::ifSDCol_AGR(toMetal, toCol)) &&
        (fromMetal == 1 && toMetal == 1)) {
      std::string variable_name =
          fmt::format("M_{}_{}", SMTCell::getUdEdgeFromVarName(udEdgeIndex),
                      SMTCell::getUdEdgeToVarName(udEdgeIndex));
      SMTCell::assignTrueVar(variable_name, 0, false);
    }
  }
}

void Graph::init_net_edge_var() {
  for (int netIndex = 0; netIndex < SMTCell::getNetCnt(); netIndex++) {
    for (int udEdgeIndex = 0; udEdgeIndex < SMTCell::getUdEdgeCnt();
         udEdgeIndex++) {
      std::shared_ptr<Triplet> fromVar =
          SMTCell::getUdEdge(udEdgeIndex)->getUdEdgeFromVar();
      std::shared_ptr<Triplet> toVar =
          SMTCell::getUdEdge(udEdgeIndex)->getUdEdgeToVar();
      int fromCol = fromVar->col_;
      int toCol = toVar->col_;
      int fromMetal = fromVar->metal_;
      int toMetal = toVar->metal_;
      if ((SMTCell::ifSDCol_AGR(fromMetal, fromCol) ||
           SMTCell::ifSDCol_AGR(toMetal, toCol)) &&
          (fromMetal == 1 && toMetal == 1)) {
        std::string variable_name =
            fmt::format("N{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
                        SMTCell::getUdEdgeFromVarName(udEdgeIndex),
                        SMTCell::getUdEdgeToVarName(udEdgeIndex));
        SMTCell::assignTrueVar(variable_name, 0, false);
      }
    }
  }
}

void Graph::init_net_commodity_edge_var() {
  for (int netIndex = 0; netIndex < SMTCell::getNetCnt(); netIndex++) {
    for (int commodityIndex = 0;
         commodityIndex < SMTCell::getNet(netIndex)->getNumSinks();
         commodityIndex++) {
      for (int udEdgeIndex = 0; udEdgeIndex < SMTCell::getUdEdgeCnt();
           udEdgeIndex++) {
        std::shared_ptr<Triplet> fromVar =
            SMTCell::getUdEdge(udEdgeIndex)->getUdEdgeFromVar();
        std::shared_ptr<Triplet> toVar =
            SMTCell::getUdEdge(udEdgeIndex)->getUdEdgeToVar();
        int fromCol = fromVar->col_;
        int toCol = toVar->col_;
        int fromMetal = fromVar->metal_;
        int toMetal = toVar->metal_;
        if ((SMTCell::ifSDCol_AGR(fromMetal, fromCol) ||
             SMTCell::ifSDCol_AGR(toMetal, toCol)) &&
            (fromMetal == 1 && toMetal == 1)) {
          std::string variable_name = fmt::format(
              "N{}_C{}_E_{}_{}", SMTCell::getNet(netIndex)->getNetID(),
              commodityIndex, SMTCell::getUdEdgeFromVarName(udEdgeIndex),
              SMTCell::getUdEdgeToVarName(udEdgeIndex));
          SMTCell::assignFalseVar(variable_name);
        }
      }
    }
  }
}

void Graph::init_extensible_boundary(int boundaryCondition) {
  if (boundaryCondition == 1) {
    // do nothing
  } else {
    for (int leftVertex = 0; leftVertex < SMTCell::getLeftCornerCnt();
         leftVertex++) {
      int metal = SMTCell::getLeftCorner(leftVertex)->metal_;
      if (SMTCell::ifHorzMetal(metal)) {
        std::string variable_name = fmt::format(
            "M_LeftEnd_{}", SMTCell::getLeftCorner(leftVertex)->getVName());
        SMTCell::assignTrueVar(variable_name, 0, false);
      }
    }

    for (int rightVertex = 0; rightVertex < SMTCell::getRightCornerCnt();
         rightVertex++) {
      int metal = SMTCell::getRightCorner(rightVertex)->metal_;
      if (SMTCell::ifHorzMetal(metal)) {
        std::string variable_name = fmt::format(
            "M_{}_RightEnd", SMTCell::getRightCorner(rightVertex)->getVName());
        SMTCell::assignTrueVar(variable_name, 0, false);
      }
    }

    for (int frontVertex = 0; frontVertex < SMTCell::getFrontCornerCnt();
         frontVertex++) {
      int metal = SMTCell::getFrontCorner(frontVertex)->metal_;
      if (SMTCell::ifVertMetal(metal)) {
        std::string variable_name = fmt::format(
            "M_FrontEnd_{}", SMTCell::getFrontCorner(frontVertex)->getVName());
        SMTCell::assignTrueVar(variable_name, 0, false);
      }
    }

    for (int backVertex = 0; backVertex < SMTCell::getBackCornerCnt();
         backVertex++) {
      int metal = SMTCell::getBackCorner(backVertex)->metal_;
      if (SMTCell::ifVertMetal(metal)) {
        std::string variable_name = fmt::format(
            "M_{}_BackEnd", SMTCell::getBackCorner(backVertex)->getVName());
        SMTCell::assignTrueVar(variable_name, 0, false);
      }
    }
  }
}

Vertex::Vertex(int vIndex, Triplet *vCoord,
               std::vector<std::shared_ptr<Triplet>> vADJ) {
  vIndex_ = vIndex;
  vCoord_ = vCoord;
  vADJ_ = vADJ;
}

uint64_t Vertex::hash(uint16_t metal, uint16_t row, uint16_t col) {
  uint64_t key = (uint64_t)metal << 32;
  key = key | (uint64_t)row << 16;
  key = key | (uint64_t)col;
  return key;
}

// debug
void Vertex::dump() {
  fmt::print("[\n");
  fmt::print("  {},\n", vIndex_);
  fmt::print("  {},\n", vCoord_->getVName());
  fmt::print("  [\n");
  for (auto i : vADJ_)
    fmt::print("    {},\n", i->getVName());
  fmt::print("  ]\n");
  fmt::print("]\n");
}

UdEdge::UdEdge(int udEdgeIndex, std::shared_ptr<Triplet> udEdgeTerm1,
               std::shared_ptr<Triplet> udEdgeTerm2, int metalCost,
               int wireCost) {
  udEdgeIndex_ = udEdgeIndex;
  udEdgeTerm1_ = udEdgeTerm1;
  udEdgeTerm2_ = udEdgeTerm2;
  mCost_ = metalCost;
  wCost_ = wireCost;
}

// debug
void UdEdge::dump() {
  fmt::print("[\n");
  fmt::print("  {},\n", udEdgeIndex_);
  fmt::print("  {},\n", udEdgeTerm1_->getVName());
  fmt::print("  {},\n", udEdgeTerm2_->getVName());
  fmt::print("  {},\n", mCost_);
  fmt::print("  {},\n", wCost_);
  fmt::print("]\n");
}

OuterPin::OuterPin(std::string pinName, std::string netID, int commodityInfo) {
  pinName_ = pinName;
  netID_ = netID;
  commodityInfo_ = commodityInfo;
}

// debug
void OuterPin::dump() {
  fmt::print("[\n");
  fmt::print("  {},\n", pinName_);
  fmt::print("  {},\n", netID_);
  fmt::print("  {},\n", commodityInfo_);
  fmt::print("]\n");
}

// Source::Source(std::string netID, int numSubNodes,
//                std::vector<Triplet *> boundaryVertices) {
Source::Source(std::string netID, int numSubNodes,
               std::vector<std::shared_ptr<Triplet>> boundaryVertices) {
  netID_ = netID;
  numSubNodes_ = numSubNodes;
  boundaryVertices_ = boundaryVertices;
}

// debug
void Source::dump() {
  fmt::print("[\n");
  fmt::print("  {},\n", netID_);
  fmt::print("  {},\n", numSubNodes_);
  fmt::print("  [\n");
  for (auto bv : boundaryVertices_)
    fmt::print("  {},\n", bv->getVName());
  fmt::print("  ]\n");
  fmt::print("]\n");
}

// Sink::Sink(std::string netID, int numSubNodes,
//            std::vector<Triplet *> boundaryVertices) {
Sink::Sink(std::string netID, int numSubNodes,
           std::vector<std::shared_ptr<Triplet>> boundaryVertices) {
  netID_ = netID;
  numSubNodes_ = numSubNodes;
  boundaryVertices_ = boundaryVertices;
}

// debug
void Sink::dump() {
  fmt::print("[\n");
  fmt::print("  {},\n", netID_);
  fmt::print("  {},\n", numSubNodes_);
  fmt::print("  [\n");
  for (auto bv : boundaryVertices_)
    fmt::print("  {},\n", bv->getVName());
  fmt::print("  ]\n");
  fmt::print("]\n");
}

VirtualEdge::VirtualEdge(int vEdgeIndex, std::shared_ptr<Triplet> vCoord,
                         std::string pinName, int virtualCost) {
  vEdgeIndex_ = vEdgeIndex;
  vCoord_ = vCoord;
  pinName_ = pinName;
  virtualCost_ = virtualCost;
}

// debug
void VirtualEdge::dump() {
  fmt::print("[\n");
  fmt::print("  {},\n", vEdgeIndex_);
  fmt::print("  {},\n", vCoord_->getVName());
  fmt::print("  {},\n", pinName_);
  fmt::print("  {},\n", virtualCost_);
  fmt::print("]\n");
}