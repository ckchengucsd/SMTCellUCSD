#pragma once
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

class dbConfig {
  /*
    Configuration for Design Rule, Flow Constraints, Constraint Simplification
    and Graph Settings.
  */
public:
  dbConfig(const std::string &config_path);

  dbConfig(int BoundaryCondition, int SON, int DoublePowerRail,
           int MM_Parameter, int EXT_Parameter, int BCP_Parameter,
           int MAR_M1_Parameter, int MAR_M2_Parameter, int MAR_M3_Parameter,
           int MAR_M4_Parameter, int EOL_M1_B_ADJ_Parameter,
           int EOL_M1_B_Parameter, int EOL_M2_R_ADJ_Parameter,
           int EOL_M2_R_Parameter, int EOL_M3_B_ADJ_Parameter,
           int EOL_M3_B_Parameter, int EOL_M4_R_ADJ_Parameter,
           int EOL_M4_R_Parameter, float VR_M1M2_Parameter,
           float VR_M2M3_Parameter, float VR_M3M4_Parameter,
           int PRL_M1_Parameter, int PRL_M2_Parameter, int PRL_M3_Parameter,
           int PRL_M4_Parameter, int SHR_M1_Parameter, int SHR_M2_Parameter,
           int SHR_M3_Parameter, int SHR_M4_Parameter, int MPL_Parameter,
           int XOL_Mode, int NDE_Parameter, int Partition_Parameter,
           int ML_Parameter, int Local_Parameter, int tolerance_Parameter,
           int BS_Parameter, int objpart_Parameter, int XOL_Parameter);

  dbConfig(const std::string &config_path, std::string infile);

  void checkParameter();

  // disable partition when info is not provided to avoid error
  void checkPartition_Parameter(int sizeOfPartition);

  void setZ3Seed(int Z3Seed) { Z3Seed_ = Z3Seed; }

  void setBoundaryCondition(int BoundaryCondition) {
    BoundaryCondition_ = BoundaryCondition;
  }

  void setSON(int SON) { SON_ = SON; }

  void setDoublePowerRail(int DoublePowerRail) {
    DoublePowerRail_ = DoublePowerRail;
  }

  void setMM_Parameter(int MM_Parameter) { MM_Parameter_ = MM_Parameter; }

  void setEXT_Parameter(int EXT_Parameter) { EXT_Parameter_ = EXT_Parameter; }

  void setBCP_Parameter(int BCP_Parameter) { BCP_Parameter_ = BCP_Parameter; }

  // void setMAR_Parameter(int MAR_Parameter) { MAR_Parameter_ = MAR_Parameter; }
  void setMAR_M1_Parameter(int MAR_M1_Parameter) {
    MAR_M1_Parameter_ = MAR_M1_Parameter;
  }

  void setMAR_M2_Parameter(int MAR_M2_Parameter) {
    MAR_M2_Parameter_ = MAR_M2_Parameter;
  }

  void setMAR_M3_Parameter(int MAR_M3_Parameter) {
    MAR_M3_Parameter_ = MAR_M3_Parameter;
  }

  void setMAR_M4_Parameter(int MAR_M4_Parameter) {
    MAR_M4_Parameter_ = MAR_M4_Parameter;
  }

  // void setEOL_Parameter(int EOL_Parameter) { EOL_Parameter_ = EOL_Parameter; }
  void setEOL_M1_B_ADJ_Parameter(int EOL_M1_B_ADJ_Parameter) {
    EOL_M1_B_ADJ_Parameter_ = EOL_M1_B_ADJ_Parameter;
  }

  void setEOL_M1_B_Parameter(int EOL_M1_B_Parameter) {
    EOL_M1_B_Parameter_ = EOL_M1_B_Parameter;
  }

  void setEOL_M2_R_ADJ_Parameter(int EOL_M2_R_ADJ_Parameter) {
    EOL_M2_R_ADJ_Parameter_ = EOL_M2_R_ADJ_Parameter;
  }

  void setEOL_M2_R_Parameter(int EOL_M2_R_Parameter) {
    EOL_M2_R_Parameter_ = EOL_M2_R_Parameter;
  }

  void setEOL_M3_B_ADJ_Parameter(int EOL_M3_B_ADJ_Parameter) {
    EOL_M3_B_ADJ_Parameter_ = EOL_M3_B_ADJ_Parameter;
  }

  void setEOL_M3_B_Parameter(int EOL_M3_B_Parameter) {
    EOL_M3_B_Parameter_ = EOL_M3_B_Parameter;
  }

  void setEOL_M4_R_ADJ_Parameter(int EOL_M4_R_ADJ_Parameter) {
    EOL_M4_R_ADJ_Parameter_ = EOL_M4_R_ADJ_Parameter;
  }

  void setEOL_M4_R_Parameter(int EOL_M4_R_Parameter) {
    EOL_M4_R_Parameter_ = EOL_M4_R_Parameter;
  }

  // void setVR_Parameter(float VR_Parameter) { VR_Parameter_ = VR_Parameter; }
  void setVR_M1M2_Parameter(float VR_M1M2_Parameter) {
    VR_M1M2_Parameter_ = VR_M1M2_Parameter;
  }

  void setVR_M2M3_Parameter(float VR_M2M3_Parameter) {
    VR_M2M3_Parameter_ = VR_M2M3_Parameter;
  }

  void setVR_M3M4_Parameter(float VR_M3M4_Parameter) {
    VR_M3M4_Parameter_ = VR_M3M4_Parameter;
  }

  // void setPRL_Parameter(int PRL_Parameter) { PRL_Parameter_ = PRL_Parameter; }
  void setPRL_M1_Parameter(int PRL_M1_Parameter) {
    PRL_M1_Parameter_ = PRL_M1_Parameter;
  }

  void setPRL_M2_Parameter(int PRL_M2_Parameter) {
    PRL_M2_Parameter_ = PRL_M2_Parameter;
  }

  void setPRL_M3_Parameter(int PRL_M3_Parameter) {
    PRL_M3_Parameter_ = PRL_M3_Parameter;
  }

  void setPRL_M4_Parameter(int PRL_M4_Parameter) {
    PRL_M4_Parameter_ = PRL_M4_Parameter;
  }

  // void setSHR_Parameter(int SHR_Parameter) { SHR_Parameter_ = SHR_Parameter; }
  void setSHR_M1_Parameter(int SHR_M1_Parameter) {
    SHR_M1_Parameter_ = SHR_M1_Parameter;
  }

  void setSHR_M2_Parameter(int SHR_M2_Parameter) {
    SHR_M2_Parameter_ = SHR_M2_Parameter;
  }

  void setSHR_M3_Parameter(int SHR_M3_Parameter) {
    SHR_M3_Parameter_ = SHR_M3_Parameter;
  }

  void setSHR_M4_Parameter(int SHR_M4_Parameter) {
    SHR_M4_Parameter_ = SHR_M4_Parameter;
  }

  void setMPL_Parameter(int MPL_Parameter) { MPL_Parameter_ = MPL_Parameter; }

  void setXOL_Mode(int XOL_Mode) {
    XOL_Mode_ = XOL_Mode;
    if (XOL_Mode == 0) {
      XOL_Parameter_ = 2;
    } else {
      XOL_Parameter_ = 4;
    }
  }

  void setNDE_Parameter(int NDE_Parameter) { NDE_Parameter_ = NDE_Parameter; }

  void setPartition_Parameter(int Partition_Parameter) {
    Partition_Parameter_ = Partition_Parameter;
  }

  void setML_Parameter(int ML_Parameter) { ML_Parameter_ = ML_Parameter; }

  void setLocal_Parameter(int Local_Parameter) {
    Local_Parameter_ = Local_Parameter;
  }

  void setTolerance_Parameter(int tolerance_Parameter) {
    tolerance_Parameter_ = tolerance_Parameter;
  }

  void setBS_Parameter(int BS_Parameter) { BS_Parameter_ = BS_Parameter; }

  void setObjpart_Parameter(int objpart_Parameter) {
    objpart_Parameter_ = objpart_Parameter;
  }

  void setXOL_Parameter(int XOL_Parameter) { XOL_Parameter_ = XOL_Parameter; }

  int getZ3Seed() { return Z3Seed_; }

  int getBoundaryCondition() { return BoundaryCondition_; }

  int getSON() { return SON_; }

  int getDoublePowerRail() { return DoublePowerRail_; }

  int getMM_Parameter() { return MM_Parameter_; }

  int getEXT_Parameter() { return EXT_Parameter_; }

  int getBCP_Parameter() { return BCP_Parameter_; }

  // int getMAR_Parameter() { return MAR_Parameter_; }
  int getMAR_M1_Parameter() { return MAR_M1_Parameter_; }

  int getMAR_M2_Parameter() { return MAR_M2_Parameter_; }

  int getMAR_M3_Parameter() { return MAR_M3_Parameter_; }

  int getMAR_M4_Parameter() { return MAR_M4_Parameter_; }

  // int getEOL_Parameter() { return EOL_Parameter_; }
  int getEOL_M1_B_ADJ_Parameter() { return EOL_M1_B_ADJ_Parameter_; }

  int getEOL_M1_B_Parameter() { return EOL_M1_B_Parameter_; }

  int getEOL_M2_R_ADJ_Parameter() { return EOL_M2_R_ADJ_Parameter_; }

  int getEOL_M2_R_Parameter() { return EOL_M2_R_Parameter_; }

  int getEOL_M3_B_ADJ_Parameter() { return EOL_M3_B_ADJ_Parameter_; }

  int getEOL_M3_B_Parameter() { return EOL_M3_B_Parameter_; }

  int getEOL_M4_R_ADJ_Parameter() { return EOL_M4_R_ADJ_Parameter_; }

  int getEOL_M4_R_Parameter() { return EOL_M4_R_Parameter_; }

  // float getVR_Parameter() { return VR_Parameter_; }
  float getVR_M1M2_Parameter() { return VR_M1M2_Parameter_; }

  float getVR_M2M3_Parameter() { return VR_M2M3_Parameter_; }

  float getVR_M3M4_Parameter() { return VR_M3M4_Parameter_; }

  // int getPRL_Parameter() { return PRL_Parameter_; }
  int getPRL_M1_Parameter() { return PRL_M1_Parameter_; }

  int getPRL_M2_Parameter() { return PRL_M2_Parameter_; }

  int getPRL_M3_Parameter() { return PRL_M3_Parameter_; }

  int getPRL_M4_Parameter() { return PRL_M4_Parameter_; }

  // int getSHR_Parameter() { return SHR_Parameter_; }
  int getSHR_M1_Parameter() { return SHR_M1_Parameter_; }

  int getSHR_M2_Parameter() { return SHR_M2_Parameter_; }

  int getSHR_M3_Parameter() { return SHR_M3_Parameter_; }

  int getSHR_M4_Parameter() { return SHR_M4_Parameter_; }

  int getMPL_Parameter() { return MPL_Parameter_; }

  int getXOL_Mode() { return XOL_Mode_; }

  int getNDE_Parameter() { return NDE_Parameter_; }

  int getPartition_Parameter() { return Partition_Parameter_; }

  int getML_Parameter() { return ML_Parameter_; }

  int getLocal_Parameter() { return Local_Parameter_; }

  int getTolerance_Parameter() { return tolerance_Parameter_; }

  int getBS_Parameter() { return BS_Parameter_; }

  int getObjpart_Parameter() { return objpart_Parameter_; }

  int getXOL_Parameter() { return XOL_Parameter_; }

  int getPE_Parameter() { return PE_Parameter_; }

  int getPE_Mode() { return PE_Mode_; }

  int getM1_Separation_Parameter() { return M1_Separation_Parameter_; }

private:
  // User defined Design Parameteres
  int Z3Seed_;
  // 0: Fixed, 1: Extensible
  int BoundaryCondition_;
  // 0: Disable, 1: Enable
  int SON_;
  // 0: Disable, 1: Enable
  int DoublePowerRail_;
  // 3: Maximum Number of MetalLayer
  int MM_Parameter_;
  // 0 only
  int EXT_Parameter_;
  // 0: Disable 1: Enable BCP(Default)
  int BCP_Parameter_;
  // ARGV[1], Minimum Area 1: (Default), Integer
  int MAR_M1_Parameter_;
  int MAR_M2_Parameter_;
  int MAR_M3_Parameter_;
  int MAR_M4_Parameter_;
  // ARGV[2], End-of-Line  1: (Default), Integer
  int EOL_M1_B_ADJ_Parameter_;
  int EOL_M1_B_Parameter_;
  int EOL_M2_R_ADJ_Parameter_;
  int EOL_M2_R_Parameter_;
  int EOL_M3_B_ADJ_Parameter_;
  int EOL_M3_B_Parameter_;
  int EOL_M4_R_ADJ_Parameter_;
  int EOL_M4_R_Parameter_;
  // ARGV[3], VIA sqrt(2)=1.5 : (Default), Floating
  float VR_M1M2_Parameter_;
  float VR_M2M3_Parameter_;
  float VR_M3M4_Parameter_;
  // ARGV[4], Parallel Run-length 1: (Default). Integer
  int PRL_M1_Parameter_;
  int PRL_M2_Parameter_;
  int PRL_M3_Parameter_;
  int PRL_M4_Parameter_;
  // ARGV[5], Step Heights 1: (Default), Integer
  int SHR_M1_Parameter_;
  int SHR_M2_Parameter_;
  int SHR_M3_Parameter_;
  int SHR_M4_Parameter_;
  // ARGV[6], 3: (Default) Minimum Pin Opening
  int MPL_Parameter_;
  // ARGV[7], 0: SDB, 1:DDB, 2:(Default)SDB/DDB mixed
  int XOL_Mode_;
  // ARGV[8], 0: Disable(Default) 1: Enable NDE
  int NDE_Parameter_;
  // ARGV[9], 0: Disable(Default) 1. Cell Partitioning
  int Partition_Parameter_;
  // ARGV[10], 5: (Default) Min Metal Length for avoiding adjacent signal
  // routing (only works with special net setting)
  int ML_Parameter_;
  // ARGV[11], 0: Disable(Default) 1: Localization for
  // Internal node within same diffusion region
  int Local_Parameter_;
  // ARGV[12], Localization Offset Margin, Integer
  int tolerance_Parameter_;
  // ARGV[13], 0: Disable(Default) 1: Enable BS(Breaking Symmetry)
  int BS_Parameter_;
  // ARGV[14], 0: Disable(Default) 1. Objective Partitioning
  int objpart_Parameter_;
  // Depends on XOL_Mode
  int XOL_Parameter_;

  int PE_Parameter_;
  int PE_Mode_;
  int M1_Separation_Parameter_;
};