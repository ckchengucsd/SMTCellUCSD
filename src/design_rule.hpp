#define FMT_HEADER_ONLYget_n
#include <fmt/core.h>
#include <fmt/format.h>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

class DesignRuleWriter {
public:
  // step 6 (AGR version)
  void write_geometric_variables_AGR(FILE *drlog);
  void write_geometric_variables_AGR_L_tip_helper(FILE *drlog);
  void write_geometric_variables_AGR_R_tip_helper(FILE *drlog);
  void write_geometric_variables_AGR_F_tip_helper(FILE *drlog);
  void write_geometric_variables_AGR_B_tip_helper(FILE *drlog);

  // step 7 (AGR version)
  void write_MAR_AGR_rule(int MAR_M1_Parameter, int MAR_M2_Parameter,
                          int MAR_M3_Parameter, int MAR_M4_Parameter,
                          int doublePowerRail, FILE *drlog);

  // step 8 (AGR version)
  void write_EOL_AGR_rule(int EOL_M1_B_ADJ_Parameter, int EOL_M1_B_Parameter,
                          int EOL_M2_R_ADJ_Parameter, int EOL_M2_R_Parameter,
                          int EOL_M3_B_ADJ_Parameter, int EOL_M3_B_Parameter,
                          int EOL_M4_R_ADJ_Parameter, int EOL_M4_R_Parameter,
                          int doublePowerRail, FILE *drlog);
  void write_EOL_AGR_RL_tip_helper(int EOL_M2_R_ADJ_Parameter,
                                   int EOL_M2_R_Parameter,
                                   int EOL_M4_R_ADJ_Parameter,
                                   int EOL_M4_R_Parameter, int doublePowerRail,
                                   FILE *drlog);
  void write_EOL_AGR_LR_tip_helper(int EOL_M2_R_ADJ_Parameter,
                                   int EOL_M2_R_Parameter,
                                   int EOL_M4_R_ADJ_Parameter,
                                   int EOL_M4_R_Parameter, int doublePowerRail,
                                   FILE *drlog);
  void write_EOL_AGR_BF_tip_helper(int EOL_M1_B_ADJ_Parameter,
                                   int EOL_M1_B_Parameter,
                                   int EOL_M3_B_ADJ_Parameter,
                                   int EOL_M3_B_Parameter, int doublePowerRail,
                                   FILE *drlog);
  void write_EOL_AGR_FB_tip_helper(int EOL_M1_B_ADJ_Parameter,
                                   int EOL_M1_B_Parameter,
                                   int EOL_M3_B_ADJ_Parameter,
                                   int EOL_M3_B_Parameter, int doublePowerRail,
                                   FILE *drlog);

  // step 9 (AGR version)
  void write_VR_AGR_rule(float VR_M1M2_Parameter, float VR_M2M3_Parameter,
                         float VR_M3M4_Parameter, int doublePowerRail,
                         FILE *drlog);
  void write_VR_AGR_M1_helper(float VR_M1M2_Parameter, int doublePowerRail,
                              FILE *drlog);
  void write_VR_AGR_M2_M4_helper(float VR_M2M3_Parameter,
                                 float VR_M3M4_Parameter, int doublePowerRail,
                                 FILE *drlog);

  // step 10 (AGR version)
  void write_PRL_AGR_rule(int PRL_M1_Parameter, int PRL_M2_Parameter,
                          int PRL_M3_Parameter, int PRL_M4_Parameter,
                          int doublePowerRail, FILE *drlog);
  void write_PRL_AGR_LR_tip_helper(int PRL_M2_Parameter, int PRL_M4_Parameter,
                                   int doublePowerRail, FILE *drlog);
  void write_PRL_AGR_RL_tip_helper(int PRL_M2_Parameter, int PRL_M4_Parameter,
                                   int doublePowerRail, FILE *drlog);
  // disabled the following two functions
  void write_PRL_AGR_FB_tip_helper(int PRL_M1_Parameter, int PRL_M3_Parameter,
                                   int doublePowerRail, FILE *drlog);
  void write_PRL_AGR_BF_tip_helper(int PRL_M1_Parameter, int PRL_M3_Parameter,
                                   int doublePowerRail, FILE *drlog);

  // step 12
  void write_pin_access_rule(int MPL_Parameter, int MAR_Parameter,
                             int EOL_Parameter, FILE *drlog);
  void write_pin_access_rule_helper(int MPL_Parameter, int MAR_Parameter,
                                    int EOL_Parameter, FILE *drlog);
  void write_pin_access_rule_via_enclosure_helper(FILE *drlog);
  void write_pin_access_rule_via23_helper(FILE *drlog);
  void write_pin_access_rule_via34_helper(FILE *drlog);
  void write_pin_access_rule_via12_helper(FILE *drlog);

  // step 13 (AGR version)
  void write_SHR_AGR_rule(int SHR_M1_Parameter, int SHR_M2_Parameter,
                          int SHR_M3_Parameter, int SHR_M4_Parameter,
                          int doublePowerRail, FILE *drlog);
  void write_SHR_AGR_L_tip_helper(int SHR_M2_Parameter, int SHR_M4_Parameter,
                                  int doublePowerRail, FILE *drlog);
  void write_SHR_AGR_R_tip_helper(int SHR_M2_Parameter, int SHR_M4_Parameter,
                                  int doublePowerRail, FILE *drlog);
  void write_SHR_AGR_F_tip_helper(int SHR_M1_Parameter, int SHR_M3_Parameter,
                                  int doublePowerRail, FILE *drlog);
  void write_SHR_AGR_B_tip_helper(int SHR_M1_Parameter, int SHR_M3_Parameter,
                                  int doublePowerRail, FILE *drlog);
};