#define FMT_HEADER_ONLY
#include <algorithm>
#include <boost/algorithm/string/classification.hpp> // Include boost::for is_any_of
#include <boost/algorithm/string/split.hpp>          // Include for boost::split
#include <boost/multiprecision/integer.hpp>          // for returning bit length
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

#include "genSMTInputAGR.hpp"
namespace bmp = boost::multiprecision;

int main(int argc, char *argv[]) {
  // ### Pre-processing ########################################################
  auto const workdir = std::filesystem::current_path();
  std::string result_path = "";
  std::string pinlayout_path = "";
  std::string config_path = "";

  dbConfig *config;
  SMTCell *smtcell = new SMTCell();

  if (argc != 4) {
    std::cerr << "\n*** [ERROR] Wrong CMD";
    std::cerr << "\n   [USAGE]: ./PL_FILE [inputfile_pinLayout] "
                 "[inputfile_config] [result_dir]\n\n";
    exit(1);
  } else {
    pinlayout_path = argv[1];
    config_path = argv[2];
    result_path = argv[3];
  }

  std::filesystem::path result_dir(result_path);
  // output directory
  fmt::print("a   Working Directory: {}\n", workdir.c_str());
  fmt::print("a   Result Directory: {}\n", result_dir.c_str());
  // append workdir result_dir and logs
  auto const logdir = result_dir / "logs";
  fmt::print("a   Log Directory: {}\n", logdir.c_str());
  auto const outdir = result_dir / "inputSMT";
  fmt::print("a   Output Directory: {}\n", outdir.c_str());

  // check input files
  std::ifstream config_file(config_path);
  std::ifstream pinlayout_file(pinlayout_path);

  if (!config_file.good()) {
    fmt::print("\n*** [ERROR] FILE DOES NOT EXIST..{}\n", config_path);
    exit(1);
  } else {
    config = new dbConfig(config_path, pinlayout_path);
    smtcell->initTrackInfo(config_path);
  }

  if (!pinlayout_file.good()) {
    fmt::print("\n*** [ERROR] FILE DOES NOT EXIST..{}\n", pinlayout_path);
    exit(1);
  } else {
    fmt::print("\n");
    fmt::print("a   Version Info : 1.0 AGR Version\n");

    fmt::print("a        Minimum Area Rules : [MAR_M1 = {}, MAR_M2 = {}, "
               "MAR_M3 = {}, MAR_M4 = {}]\n",
               config->getMAR_M1_Parameter(), config->getMAR_M2_Parameter(),
               config->getMAR_M3_Parameter(), config->getMAR_M4_Parameter());
    // [EOL Design Rule]
    fmt::print(
        "a        End-Of-Line Rules : [EOL_M1_B_Adj = {}, EOL_M1_B = {},\n",
        config->getEOL_M1_B_ADJ_Parameter(), config->getEOL_M1_B_Parameter());
    fmt::print(
        "a                          : [EOL_M2_R_Adj = {}, EOL_M2_R = {},\n",
        config->getEOL_M2_R_ADJ_Parameter(), config->getEOL_M2_R_Parameter());
    fmt::print(
        "a                          : [EOL_M3_B_Adj = {}, EOL_M3_B = {},\n",
        config->getEOL_M3_B_ADJ_Parameter(), config->getEOL_M3_B_Parameter());
    fmt::print(
        "a                          : [EOL_M4_R_Adj = {}, EOL_M4_R = {},\n",
        config->getEOL_M4_R_ADJ_Parameter(), config->getEOL_M4_R_Parameter());
    // [VR Design Rule]
    fmt::print("a        Via-to-Via Rules  : [VR_M1M2 = {}, VR_M2M3 = {}, "
               "VR_M3M4 = {}]\n",
               config->getVR_M1M2_Parameter(), config->getVR_M2M3_Parameter(),
               config->getVR_M3M4_Parameter());
    // [PRL Design Rule]
    fmt::print(
        "a        Parallel Run Length Rules : [PRL_M1 = {}, PRL_M2 = {}, "
        "PRL_M3 = {}, PRL_M4 = {}]\n",
        config->getPRL_M1_Parameter(), config->getPRL_M2_Parameter(),
        config->getPRL_M3_Parameter(), config->getPRL_M4_Parameter());
    // [SHR Design Rule]
    fmt::print(
        "a        Spacing to Hierarchy Rules : [SHR_M1 = {}, SHR_M2 = {}, "
        "SHR_M3 = {}, SHR_M4 = {}]\n",
        config->getSHR_M1_Parameter(), config->getSHR_M2_Parameter(),
        config->getSHR_M3_Parameter(), config->getSHR_M4_Parameter());
    fmt::print(
        "a        Parameter Options : [MPO = {}], [Localization = {} (T{})]\n",
        config->getMPL_Parameter(), config->getLocal_Parameter(),
        config->getTolerance_Parameter());
    fmt::print("a	                        [Cell Partitioning = {}], [FST "
               "= {}], [Breaking Symmetry = {}]\n",
               config->getPartition_Parameter(),
               (config->getNDE_Parameter() == 0 ? "Disable" : "Enable"),
               config->getBS_Parameter());
    fmt::print("a	                        [DBMode = {}({})], [Objective "
               "Partitioning = {}]\n\n",
               config->getXOL_Mode(), config->getXOL_Parameter(),
               config->getObjpart_Parameter());

    fmt::print("a   Generating SMT-LIB 2.0 Standard inputfile based on the "
               "following files.\n");
    fmt::print("a     Input Layout:  {}/{}\n", workdir.c_str(), pinlayout_path);
  }

  // ### END: Pre-processing
  // ###########################################################################
  // ### ENTITY STRUCTURE
  // Output Directory Creation
  system(("mkdir -p " + outdir.string()).c_str());
  auto cellName = pinlayout_path.substr(pinlayout_path.rfind("/") + 1);
  auto outfile =
      outdir / cellName.substr(0, cellName.find("."))
                   .append(fmt::format("_{}F_{}T.smt2", SMTCell::getNumFin(),
                                       SMTCell::getNumTrack()));
  // DEBUG MODE : Output Log File
  auto designRuleLog = logdir / "debug/designRule.log";
  system(("touch " + designRuleLog.string()).c_str());
  std::FILE *dr_log = std::fopen(designRuleLog.c_str(), "w");

  auto graphLog = logdir / "debug/graph.log";
  system(("touch " + graphLog.string()).c_str());
  std::FILE *graph_log = std::fopen(graphLog.c_str(), "w");

  fmt::print("a     SMT-LIB2.0 File:    {}\n", outfile.c_str());
  // cellName.clear();

  smtcell->setNumMetalLayer(config->getMM_Parameter()); // M1~M4

  LayoutParser *parser = new LayoutParser();
  parser->parsePinLayout(pinlayout_path, config->getPartition_Parameter());
  delete parser;

  // check Partitioning if it is valid
  config->checkPartition_Parameter(SMTCell::getInstPartitionCnt());

  // Placement related will be handled by Placement class
  Placement *plc = new Placement();
  plc->init_inst_partition(config->getPartition_Parameter());

  // ### Remove Power Pin/Net Information from data structure
  // Remove Power Pin
  smtcell->removePowerPin();

  // Remove Power Net
  smtcell->removePowerNet();

  fmt::print("a     # Pins              = {}\n", smtcell->getPinCnt());
  fmt::print("a     # Nets              = {}\n", smtcell->getNetCnt());

  // ### END:  ENTITY STRUCTURE
  // ###########################################################################
  // ### GRAPH STRUCTURE
  Graph *graph = new Graph();

  // ### GRAPH Generation (Required before generating VERTEX)
  graph->init_graph();
  graph->dump_graph(graph_log);

  // ### VERTEX Generation
  graph->init_agr_vertices();
  // SMTCell->dump_vertices();
  fmt::print("a     # Vertices          = {}\n", smtcell->getVertexCnt());
  fmt::print(graph_log, "a     # Vertices          = {}\n",
             smtcell->getVertexCnt());

  // ### UNDIRECTED EDGE Generation
  graph->init_agr_udedges();
  // SMTCell->dump_udEdges();
  fmt::print("a     # udEdges           = {}\n", smtcell->getUdEdgeCnt());
  fmt::print(graph_log, "a     # udEdges           = {}\n",
             smtcell->getUdEdgeCnt());

  // ### BOUNDARY VERTICES Generation.
  graph->init_agr_boundaryVertices(config->getEXT_Parameter());
  fmt::print("a     # Boundary Vertices = {}\n",
             smtcell->getBoundaryVertexCnt());
  fmt::print(graph_log, "a     # Boundary Vertices = {}\n",
             smtcell->getBoundaryVertexCnt());
  // SMTCell::dump_boundaryVertices();

  graph->init_outerPins();
  fmt::print("a     # Outer Pins        = {}\n", smtcell->getOuterPinCnt());
  fmt::print(graph_log, "a     # Outer Pins        = {}\n",
             smtcell->getOuterPinCnt());
  // SMTCell->dump_outerPins();

  graph->init_agr_corner();
  fmt::print("a     # Left Corners      = {}\n", smtcell->getLeftCornerCnt());
  fmt::print("a     # Right Corners     = {}\n", smtcell->getRightCornerCnt());
  fmt::print("a     # Front Corners     = {}\n", smtcell->getFrontCornerCnt());
  fmt::print("a     # Back Corners      = {}\n", smtcell->getBackCornerCnt());
  fmt::print(graph_log, "a     # Left Corners      = {}\n",
             smtcell->getLeftCornerCnt());
  fmt::print(graph_log, "a     # Right Corners     = {}\n",
             smtcell->getRightCornerCnt());
  fmt::print(graph_log, "a     # Front Corners     = {}\n",
             smtcell->getFrontCornerCnt());
  fmt::print(graph_log, "a     # Back Corners      = {}\n",
             smtcell->getBackCornerCnt());

  // ### SOURCE and SINK Generation. All sources and sinks are supernodes.
  // ### DATA STRUCTURE:  SOURCE or SINK [netName] [#subNodes] [Arr. of
  // sub-nodes, i.e., vertices]
  graph->init_source(config->getSON());
  fmt::print("a     # Ext Nets          = {}\n", smtcell->getExtNetCnt());
  fmt::print("a     # Sources           = {}\n", smtcell->getSourceCnt());
  fmt::print("a     # Sinks             = {}\n", smtcell->getSinkCnt());
  fmt::print(graph_log, "a     # Ext Nets          = {}\n",
             smtcell->getExtNetCnt());
  fmt::print(graph_log, "a     # Sources           = {}\n",
             smtcell->getSourceCnt());
  fmt::print(graph_log, "a     # Sinks             = {}\n",
             smtcell->getSinkCnt());

  graph->init_pinSON(config->getSON());
  // SMTCell->dump_pins();

  // ### VIRTUAL EDGE Generation
  // ### We only define directed virtual edges since we know the direction based
  // on source/sink information.
  // ### All supernodes are having names starting with 'pin'.
  // ### DATA STRUCTURE:  VIRTUAL_EDGE [index] [Origin] [Destination] [Cost=0]
  graph->init_agr_virtualEdges();
  // SMTCell->dump_virtualEdges();

  graph->init_edgeInOut();
  graph->init_vedgeInOut();

  fmt::print("a     # Virtual Edges     = {}\n", smtcell->getVirtualEdgeCnt());
  fmt::print(graph_log, "a     # Virtual Edges     = {}\n",
             smtcell->getVirtualEdgeCnt());
  // ### END:  DATA STRUCTURE
  // ###########################################################################

  std::FILE *out = std::fopen(outfile.c_str(), "w");

  fmt::print("a   Generating SMT-LIB 2.0 Standard Input Code.\n");

  // INIT
  fmt::print(out, "; Formulation for SMT\n");
  fmt::print(out, "; Format: SMT-LIB 2.0\n");
  fmt::print(out, "; Version: SMTCell-1.0\n");
  fmt::print(
      out,
      "; Authors: Yucheng Wang, Daeyeal Lee, Dongwon Park, Chung-Kuan Cheng\n");
  fmt::print(out, "; DO NOT DISTRIBUTE IN ANY PURPOSE!\n\n");
  fmt::print(out, "; Input File:  {}/{}\n", workdir.c_str(), pinlayout_path);
  fmt::print(out, "; a        Design Rule Parameters :\n");
  // fmt::print(out,
  //            "; Design Rule Parameters : [MAR = {}, EOL = {}, VR = {}, PRL =
  //            "
  //            "{}, SHR = {}]\n",
  //            config->getMAR_Parameter(), config->getEOL_Parameter(),
  //            config->getVR_Parameter(), config->getPRL_Parameter(),
  //            config->getSHR_Parameter());
  // [MAR Design Rule]
  fmt::print(out,
             "; Minimum Area Rules : [MAR_M1 = {}, MAR_M2 = {}, MAR_M3 = {}, "
             "MAR_M4 = {}]\n",
             config->getMAR_M1_Parameter(), config->getMAR_M2_Parameter(),
             config->getMAR_M3_Parameter(), config->getMAR_M4_Parameter());
  // [EOL Design Rule]
  fmt::print(out, "; End-Of-Line Rules : [EOL_M1_B_Adj = {}, EOL_M1_B = {},\n",
             config->getEOL_M1_B_ADJ_Parameter(),
             config->getEOL_M1_B_Parameter());
  fmt::print(out, ";                     [EOL_M2_R_Adj = {}, EOL_M2_R = {},\n",
             config->getEOL_M2_R_ADJ_Parameter(),
             config->getEOL_M2_R_Parameter());
  fmt::print(out, ";                     [EOL_M3_B_Adj = {}, EOL_M3_B = {},\n",
             config->getEOL_M3_B_ADJ_Parameter(),
             config->getEOL_M3_B_Parameter());
  fmt::print(out, ";                     [EOL_M4_R_Adj = {}, EOL_M4_R = {},\n",
             config->getEOL_M4_R_ADJ_Parameter(),
             config->getEOL_M4_R_Parameter());
  // [VR Design Rule]
  fmt::print(
      out, "; Via-to-Via Rules  : [VR_M1M2 = {}, VR_M2M3 = {}, VR_M3M4 = {}]\n",
      config->getVR_M1M2_Parameter(), config->getVR_M2M3_Parameter(),
      config->getVR_M3M4_Parameter());
  // [PRL Design Rule]
  fmt::print(
      out,
      "; Parallel Run Length Rules : [PRL_M1 = {}, PRL_M2 = {}, PRL_M3 = "
      "{}, PRL_M4 = {}]\n",
      config->getPRL_M1_Parameter(), config->getPRL_M2_Parameter(),
      config->getPRL_M3_Parameter(), config->getPRL_M4_Parameter());
  // [SHR Design Rule]
  fmt::print(
      out,
      "; Spacing to Hierarchy Rules : [SHR_M1 = {}, SHR_M2 = {}, SHR_M3 = "
      "{}, SHR_M4 = {}]\n",
      config->getSHR_M1_Parameter(), config->getSHR_M2_Parameter(),
      config->getSHR_M3_Parameter(), config->getSHR_M4_Parameter());

  fmt::print(out,
             "; Parameter Options : [MPO = {}], [Localization = {} (T{})]\n",
             config->getMPL_Parameter(), config->getLocal_Parameter(),
             config->getTolerance_Parameter());
  fmt::print(
      out, "; [Cell Partitioning = {}], [FST = {}], [Breaking Symmetry = {}]\n",
      config->getPartition_Parameter(),
      (config->getNDE_Parameter() == 0 ? "Disable" : "Enable"),
      config->getBS_Parameter());
  fmt::print(out, "; [DBMode = {}({})], [Objective Partitioning = {}]\n\n",
             config->getXOL_Mode(), config->getXOL_Parameter(),
             config->getObjpart_Parameter());

  fmt::print(out, ";Layout Information\n");
  fmt::print(out, ";	Placement\n");
  fmt::print(out, ";	# Vertical Tracks   = {}\n", smtcell->getNumPTrackV());
  fmt::print(out, ";	# Horizontal Tracks = {}\n", smtcell->getNumPTrackH());
  fmt::print(out, ";	# Instances         = {}\n", smtcell->getNumInstance());
  fmt::print(out, ";	Routing\n");
  fmt::print(out, ";	# Vertical Tracks   = {}\n", smtcell->getNumTrackV());
  fmt::print(out, ";	# Horizontal Tracks = {}\n", smtcell->getNumTrackH());
  fmt::print(out, ";	# Nets              = {}\n", smtcell->getNetCnt());
  fmt::print(out, ";	# Pins              = {}\n", smtcell->getPinCnt());
  fmt::print(out, ";	# Sources           = {}\n", smtcell->getSourceCnt());
  fmt::print(out, ";	List of Sources   = ");

  fmt::print(out, "{}", smtcell->dump_sources());
  fmt::print(out, "\n");
  fmt::print(out, ";	# Sinks             = {}\n", smtcell->getSinkCnt());
  fmt::print(out, ";	List of Sinks     = ");

  fmt::print(out, "{}", smtcell->dump_sinks());
  fmt::print(out, "\n");
  fmt::print(out, ";	# Outer Pins        = {}\n", smtcell->getOuterPinCnt());
  fmt::print(out, ";	List of Outer Pins= ");

  // All SON (Super Outer Node)
  fmt::print(out, "{}", smtcell->dump_SON());
  fmt::print(out, "\n");
  fmt::print(out, ";	Outer Pins Information= ");

  // All SON (Super Outer Node)
  fmt::print(out, "{}", smtcell->dump_SON_detail());
  fmt::print(out, "\n\n\n");

  // ### Z3 Option Set ###
  fmt::print(out, "(set-option :smt.core.minimize true)\n");
  fmt::print(out, "(set-option :smt.random_seed {})\n", config->getZ3Seed());
  fmt::print(out, ";Begin SMT Formulation\n\n");

  // Flow related constraints will be handled by Flow Writer
  FlowWriter *flowWriter = new FlowWriter();

  // Design Rule related constraints will be handled by Design Rule Writer
  DesignRuleWriter *drWriter = new DesignRuleWriter();

  // Init COST_SIZE, M2 Track Usage Varaible
  plc->init_cost_var(out);

  SMTCell::writeCostHint(out);

  // ### Placement ###
  fmt::print("a     ===== A. Variables for Placement =====\n");
  fmt::print(out, ";A. Variables for Placement\n");

  // // SMTCell::debug_variable_assignment();

  // Utility function: max
  fmt::print("a     A-0. Max Function\n");
  fmt::print(out, ";A-0. Max Function\n");
  plc->write_max_func(out);

  // // SMTCell::debug_variable_assignment();

  // Init Placement Variable: X, Y, width, flip, height, number of fingers
  fmt::print("a     A-1. Initialize Placement Variables\n");
  fmt::print(out, ";A-1. Initialize Placement Variables\n");
  plc->write_placement_var(out);

  // // SMTCell::debug_variable_assignment();

  fmt::print("a     ===== B. Constraints for Placement =====\n");
  fmt::print(out, "\n");
  fmt::print(out, ";B. Constraints for Placement\n");

  // Instance Placement Range Constraint given by X
  // AGR FLAG: X should be at least the offset
  fmt::print("a     B-0. Set Placement Range\n");
  fmt::print(out, ";B-0. Set Placement Range\n");
  plc->write_placement_range_constr(out);

  // PMOS Area
  fmt::print("a     B-1. Set PMOS Placement Variables\n");
  fmt::print(out, ";B-1. Set PMOS Placement Variables\n");
  plc->set_placement_var_pmos(out);

  // NMOS Area
  fmt::print("a     B-2. Set NMOS Placement Variable\n");
  fmt::print(out, ";B-2. Set NMOS Placement Variable\n");
  plc->set_placement_var_nmos(out);

  // Force Gate to be placed on even columns, S/D otherwise
  // AGR FLAG: DISABLED. columns now are real numbers.
  fmt::print("a     B-3. Set Legal Placmenet Column\n");
  fmt::print(out, ";B-3. Set Legal Placmenet Column\n");
  plc->set_placement_legal_col(out);

  // Get the minimum width of the MOSFET
  smtcell->setMOSMinWidth();

  fmt::print("a     B-4. Remove Symmetric Placement \n");
  fmt::print(out, ";B-4. Remove Symmetric Placement \n");
  plc->remove_symmetric_placement(out, config->getBS_Parameter());

  // XOL Mode => 0: SDB, 1:DDB, 2:(Default)SDB/DDB mixed
  fmt::print("a     B-5. PMOS: SDB, DDB, Mixed SDB/DDB \n");
  fmt::print(out, ";B-5. PMOS: SDB, DDB, Mixed SDB/DDB \n");

  plc->write_XOL(out, true, config->getXOL_Parameter(),
                 config->getNDE_Parameter(), config->getXOL_Parameter());
  fmt::print("a     B-6. NMOS: SDB, DDB, Mixed SDB/DDB \n");
  fmt::print(out, ";B-6. NMOS: SDB, DDB, Mixed SDB/DDB \n");
  plc->write_XOL(out, false, config->getXOL_Parameter(),
                 config->getNDE_Parameter(), config->getXOL_Parameter());

  fmt::print(out, "\n");

  // ### Routing ###
  // ### SOURCE and SINK DEFINITION per NET per COMMODITY and per VERTEX
  // (including supernodes, i.e., pins)
  // ### Preventing from routing Source/Drain Node using M1 Layer. Only Gate
  // Node can use M1 between PMOS/NMOS Region
  // ### UNDIRECTED_EDGE [index] [Term1] [Term2] [Cost]
  // #";Source/Drain Node between PMOS/NMOS region can not connect using M1
  // Layer.\n\n";
  fmt::print("a     ===== C. Constraints for Placement =====\n");
  fmt::print("a     C-0. Initialize Metal/Net/Commodity Variables\n");
  fmt::print(out, ";C-0. Initialize Metal/Net/Commodity Variables\n");
  graph->init_metal_var();
  graph->init_net_edge_var();
  graph->init_net_commodity_edge_var();

  // ### Extensible Boundary variables
  // # In Extensible Case , Metal binary variables
  fmt::print("a     C-1. Initialize Extensible Variables\n");
  fmt::print(out, ";C-1. Initialize Extensible Variables\n");
  graph->init_extensible_boundary(config->getBoundaryCondition());

  // ### Commodity Flow binary variables
  fmt::print("a     C-2. Initialize Commodity Flow Variables\n");
  fmt::print(out, ";C-2. Initialize Commodity Flow Variables\n");
  flowWriter->init_commodity_flow_var();

  fmt::print("a     C-3. Localization of Intra-FET Routing\n");
  fmt::print(out, ";C-3. Localization of Intra-FET Routing\n");
  plc->localize_adjacent_pin(config->getLocal_Parameter());

  fmt::print(out, "\n");

  int isEnd = 0;
  int numLoop = 0;

  while (isEnd == 0) {
    // SMTCell::debug_variable_assignment();
    if (smtcell->getCandidateAssignCnt() > 0) {
      // ## Merge Assignment Information
      smtcell->mergeAssignment();
    }

    if (numLoop == 0) {
      fmt::print("a     ===== Initial SMT Code Generation =====\n");
      fmt::print(dr_log, "a     ===== Initial SMT Code Generation =====\n");
    } else {
      fmt::print("a     ===== SMT Code Reduction Loop #{} =====\n", numLoop);
      fmt::print(dr_log, "a     ===== SMT Code Reduction Loop #{} =====\n",
                 numLoop);
    }

    smtcell->reset_var();
    smtcell->reset_cnt();

    // ### Set Default Gate Metal according to the capacity variables
    fmt::print("a     0.1. Default G Metal\n");
    smtcell->writeConstraint(
        ";Set Default Gate Metal according to the capacity variables\n");
    plc->set_default_G_metal();

    fmt::print("a     0.2. Default SD Metal\n");
    smtcell->writeConstraint(";Unset Rightmost Metal\n");
    // DEBUG FLAG : test
    plc->unset_rightmost_metal();

    // crosstalk mitigation by special net constraint
    fmt::print("a     0.3. Special Net Constraint\n");
    smtcell->writeConstraint(";Special Net Constraint\n");
    plc->write_special_net_constraint(config->getML_Parameter());

    // device partition constraint
    fmt::print("a     0.4. Partition Constraint\n");
    smtcell->writeConstraint(";Partition Constraint\n");
    plc->write_partition_constraint(config->getPartition_Parameter());

    fmt::print("a     0.5. G Pin Placement\n");
    smtcell->writeConstraint(";G Pin Placement\n");
    plc->init_placement_G_pin();

    fmt::print("a     0.6. SD Pin Placement\n");
    smtcell->writeConstraint(";SD Pin Placement\n");
    plc->init_placement_overlap_SD_pin();

    fmt::print("a     0.7. Flow Capacity Control\n");
    smtcell->writeConstraint(";Flow Capacity Control\n");
    flowWriter->write_flow_capacity_control();

    fmt::print("a     0.8. Localize Commodity\n");
    flowWriter->localize_commodity(config->getLocal_Parameter(),
                                   config->getTolerance_Parameter());

    // SMTCell::debug_variable_assignment();

    // ### COMMODITY FLOW Conservation
    fmt::print("a     1. Commodity flow conservation ");
    flowWriter->write_flow_conservation(out, config->getEXT_Parameter());

    // SMTCell::debug_variable_assignment();

    // ### Exclusiveness use of VERTEX
    // Only considers incoming flows by nature
    fmt::print("a     2. Exclusiveness use of vertex ");
    flowWriter->write_vertex_exclusive(out);

    // ### EDGE assignment
    // Assign edges based on commodity information
    fmt::print("a     3. Edge assignment ");
    flowWriter->write_edge_assignment(out);

    // ### Exclusiveness use of EDGES + Metal segment assignment
    // using edge usage information
    fmt::print("a     4. Exclusiveness use of edge ");
    flowWriter->write_edge_exclusive(out);

    // ### Geometry variables for LEFT, RIGHT, FRONT, BACK directions
    fmt::print("a     6. Geometric variables ");
    drWriter->write_geometric_variables_AGR(dr_log);

    fmt::print("a     7. Minimum area rule ");
    // drWriter->write_MAR_AGR_rule(config->getMAR_Parameter(),
    //                              config->getDoublePowerRail(), dr_log);
    drWriter->write_MAR_AGR_rule(
        config->getMAR_M1_Parameter(), config->getMAR_M2_Parameter(),
        config->getMAR_M3_Parameter(), config->getMAR_M4_Parameter(),
        config->getDoublePowerRail(), dr_log);

    fmt::print("a     8. Tip-to-Tip spacing rule ");
    drWriter->write_EOL_AGR_rule(
        config->getEOL_M1_B_ADJ_Parameter(), config->getEOL_M1_B_Parameter(),
        config->getEOL_M2_R_ADJ_Parameter(), config->getEOL_M2_R_Parameter(),
        config->getEOL_M3_B_ADJ_Parameter(), config->getEOL_M3_B_Parameter(),
        config->getEOL_M4_R_ADJ_Parameter(), config->getEOL_M4_R_Parameter(),
        config->getDoublePowerRail(), dr_log);

    fmt::print("a     9. Via-to-via spacing rule ");
    drWriter->write_VR_AGR_rule(
        config->getVR_M1M2_Parameter(), config->getVR_M2M3_Parameter(),
        config->getVR_M3M4_Parameter(), config->getDoublePowerRail(), dr_log);

    fmt::print("a     10. Parallel Run Length Rule ");
    drWriter->write_PRL_AGR_rule(
        config->getPRL_M1_Parameter(), config->getPRL_M2_Parameter(),
        config->getPRL_M3_Parameter(), config->getPRL_M4_Parameter(),
        config->getDoublePowerRail(), dr_log);

    fmt::print("a     11. Net Consistency");
    smtcell->writeConstraint(";11. Net Consistency\n");
    // Variables should be connected to flow variable though it is nor a
    // direct connection
    flowWriter->write_net_consistency();

    fmt::print("a     12. Pin Accessibility Rule ");
    smtcell->writeConstraint(";12. Pin Accessibility Rule\n");
    // ### Pin Accessibility Rule : External Pin Nets(except VDD/VSS) should
    // have at-least $MPL_Parameter true edges for top-Layer or (top-1)
    // layer(with opening)
    drWriter->write_pin_access_rule(config->getMPL_Parameter(),
                                    config->getMAR_M3_Parameter(),
                                    config->getEOL_M3_B_Parameter(), dr_log);

    fmt::print("a     13. Step Height Rule ");
    drWriter->write_SHR_AGR_rule(
        config->getSHR_M1_Parameter(), config->getSHR_M2_Parameter(),
        config->getSHR_M3_Parameter(), config->getSHR_M4_Parameter(),
        config->getDoublePowerRail(), dr_log);
    // break; // when done, comment out
    numLoop++;
    if (smtcell->getCandidateAssignCnt() == 0 ||
        config->getBCP_Parameter() == 0) {
      isEnd = 1;
    } else {
      smtcell->clear_writeout();
    }
  }

  int total_var = smtcell->getTotalVar();
  int total_clause = smtcell->getTotalClause();
  int total_literal = smtcell->getTotalLiteral();
  fmt::print("a     ===== D. Variables for Routing =====\n");
  fmt::print(out, ";===== D. Variables for Routing =====\n");

  // [2023/10/02] Experimental feature
  flowWriter->disable_cell_boundary();

  // Writing to everything to .SMT2
  smtcell->flushVarAndConstraint(out, config->getBCP_Parameter());

  fmt::print("a     ===== E. Check SAT & Optimization =====\n");
  fmt::print(out, ";===== E. Check SAT & Optimization =====\n");

  // cost related to P/N
  fmt::print("a     E-0. Cost Function for MOSFET\n");
  fmt::print(out, ";E-0. Cost Function for MOSFET\n");
  plc->write_cost_func_mos(out);

  // top metal track usage
  fmt::print("a     E-1. Top Metal Layer Usage\n");
  fmt::print(out, ";E-1. Top Metal Layer Usage\n");
  plc->write_top_metal_track_usage(out);

  fmt::print("a     E-2. Cost Function Calculation\n");
  fmt::print(out, ";E-2. Cost Function Calculation\n");
  plc->write_cost_func(out, config->getPartition_Parameter());

  fmt::print(out, "(assert (= COST_SIZE (max COST_SIZE_P COST_SIZE_N)))\n");
  fmt::print(out, "(minimize COST_SIZE)\n");

  // [2024/01/13] Experimental feature : minimize GV variables
  // if cellName starts with "DFF" or "LHQ", Skip this step
  if (cellName.rfind("DFF", 0) != 0 && cellName.rfind("LHQ", 0) != 0) {
    fmt::print("a     EXP2-1. Minimize GV Variables\n");
    fmt::print(out, ";EXP2-1. Minimize GV Variables\n");
    flowWriter->minimize_wire_segments(out);
  }
  cellName.clear();

  fmt::print(
      "a     E-3. Cost Function for Special Nets [Only used for 3F_6T]\n");
  fmt::print(out,
             ";E-3. Cost Function for Special Nets [Only used for 3F_6T]\n");
  plc->write_cost_func_special_net(out);

  fmt::print("a     E-4. Minimize Cost Function\n");
  fmt::print(out, ";E-4. Minimize Cost Function\n");
  plc->write_minimize_cost_func(out, config->getObjpart_Parameter());

  plc->write_minimize_cell_width(out);

  // fmt::print("a     E-5. Maximize Pin Enhancement\n");
  // fmt::print(out, ";E-5. Maximize Pin Enhancement (TODO)\n");
  // plc->maximize_pin_access(out, config->getPE_Parameter(),
  //                               config->getPE_Mode(),
  //                               config->getMPL_Parameter());

  fmt::print(out, "(check-sat)\n");
  // fmt::print(out, "(check-sat-using (then simplify solve-eqs smt))\n");
  fmt::print(out, "(get-model)\n");
  fmt::print(out, "(get-objectives)\n");
  fmt::print("a      ===== Complexity Summary =====\n");
  fmt::print("a      Total # Variables      = {}\n", total_var);
  fmt::print("a      Total # Literals       = {}\n", total_literal);
  fmt::print("a      Total # Clauses        = {}\n", total_clause);
  fmt::print(out, "; Total # Variables      = {}\n", total_var);
  fmt::print(out, "; Total # Literals       = {}\n", total_literal);
  fmt::print(out, "; Total # Clauses        = {}\n", total_clause);

  // close file
  std::fclose(out);
  std::fclose(dr_log);
  std::fclose(graph_log);

  smtcell->dump_summary();

  // free memory
  delete flowWriter;
  delete drWriter;
  delete plc;
  delete graph;
  delete config;
  delete smtcell;
}
