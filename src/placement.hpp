#define FMT_HEADER_ONLY
#pragma once
#define FMT_HEADER_ONLY
#include <boost/multiprecision/integer.hpp> // for returning bit length
#include <fmt/core.h>
#include <fmt/format.h>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <vector>

class Placement {
public:
  // COST_SIZE, COST_SIZE_P, COST_SIZE_N, M2_TRACK_#...
  void init_cost_var(FILE *fp);

  // define max subroutine
  void write_max_func(FILE *fp);

  // instance placement info: x#, y# ...
  void write_placement_var(FILE *fp);

  // placable range
  void write_placement_range_constr(FILE *fp);

  // placement variables
  void set_placement_var_pmos(FILE *fp);

  void set_placement_var_nmos(FILE *fp);

  // set G on even columns and S/D on odd columns
  void set_placement_legal_col(FILE *fp);

  // SDB/DDB
  void write_XOL(FILE *fp, bool ifPMOS, int XOL_Mode, int NDE_Parameter,
                 int XOL_Parameter);

  void remove_symmetric_placement(FILE *fp, bool BS_Parameter);

  void set_default_G_metal();

  void unset_rightmost_metal();

  void unset_rightmost_metal_();

  void localize_adjacent_pin(int local_Parameter);

  void init_placement_G_pin();

  void init_placement_overlap_SD_pin();

  // # Generating Instance Group Array
  void init_inst_partition(int Partition_Parameter);

  void write_special_net_constraint(int ML_Parameter);

  void write_partition_constraint(int Partition_Parameter);

  void write_cost_func_mos(FILE *fp);

  void write_cost_func(FILE *fp, int Partition_Parameter);

  void write_top_metal_track_usage(FILE *fp);

  void write_cost_func_special_net(FILE *fp);

  void write_minimize_cost_func(FILE *fp, int Partition_Parameter);

  void write_minimize_cell_width(FILE *fp);

  // align with 3F6T, need to check before deployment
  void write_partition_constraint_3F6T(int Partition_Parameter);
  void write_cost_func_3F6T(FILE *fp, int Partition_Parameter);
  void write_minimize_cost_func_3F6T(FILE *fp, int Objpart_Parameter);

  // Migrating Pin Enhancement developed by Mark
  void write_spacing_two_nets(int PE_Parameter, int PE_Mode,
                              int M1_Separation_Parameter);

  void maximize_pin_access(FILE *fp, int PE_Parameter, int PE_Mode,
                                      int MPL_Parameter);
};