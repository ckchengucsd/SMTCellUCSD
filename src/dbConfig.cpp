#include "dbConfig.hpp"

dbConfig::dbConfig(const std::string &config_path) {
  std::ifstream config_file(config_path);
  nlohmann::json config = nlohmann::json::parse(config_file);
  config_file.close();
  
  // parse into variables
  Z3Seed_ = config["z3_seed"]["value"];
  // BoundaryCondition_ = config["BoundaryCondition"]["value"];
  BoundaryCondition_ = 0;
  // SON_ = config["SON"]["value"];
  SON_ = 1;
  // DoublePowerRail_ = config["DoublePowerRail"]["value"];
  DoublePowerRail_ = 0;
  // MM_Parameter_ = config["MM_Parameter"]["value"];
  MM_Parameter_ = 4;
  // EXT_Parameter_ = config["EXT_Parameter"]["value"];
  EXT_Parameter_ = 0;
  // BCP_Parameter_ = config["BCP_Parameter"]["value"];
  BCP_Parameter_ = 1;
  // MAR_Parameter_ = config["MAR_Parameter"]["value"];
  MAR_M1_Parameter_= config["MAR_M1_Parameter"]["value"];
  MAR_M2_Parameter_ = config["MAR_M2_Parameter"]["value"];
  MAR_M3_Parameter_ = config["MAR_M3_Parameter"]["value"];
  MAR_M4_Parameter_ = config["MAR_M4_Parameter"]["value"];
  // EOL_Parameter_ = config["EOL_Parameter"]["value"];
  EOL_M1_B_ADJ_Parameter_ = config["EOL_M1_B_ADJ_Parameter"]["value"];
  EOL_M1_B_Parameter_ = config["EOL_M1_B_Parameter"]["value"];
  EOL_M2_R_ADJ_Parameter_ = config["EOL_M2_R_ADJ_Parameter"]["value"];
  EOL_M2_R_Parameter_ = config["EOL_M2_R_Parameter"]["value"];
  EOL_M3_B_ADJ_Parameter_ = config["EOL_M3_B_ADJ_Parameter"]["value"];
  EOL_M3_B_Parameter_ = config["EOL_M3_B_Parameter"]["value"];
  EOL_M4_R_ADJ_Parameter_ = config["EOL_M4_R_ADJ_Parameter"]["value"];
  EOL_M4_R_Parameter_ = config["EOL_M4_R_Parameter"]["value"];
  // VR_Parameter_ = config["VR_Parameter"]["value"];
  VR_M1M2_Parameter_ = config["VR_M1M2_Parameter"]["value"];
  VR_M2M3_Parameter_ = config["VR_M2M3_Parameter"]["value"];
  VR_M3M4_Parameter_ = config["VR_M3M4_Parameter"]["value"];
  // PRL_Parameter_ = config["PRL_Parameter"]["value"];
  PRL_M1_Parameter_ = config["PRL_M1_Parameter"]["value"];
  PRL_M2_Parameter_ = config["PRL_M2_Parameter"]["value"];
  PRL_M3_Parameter_ = config["PRL_M3_Parameter"]["value"];
  PRL_M4_Parameter_ = config["PRL_M4_Parameter"]["value"];
  // SHR_Parameter_ = config["SHR_Parameter"]["value"];
  SHR_M1_Parameter_ = config["SHR_M1_Parameter"]["value"];
  SHR_M2_Parameter_ = config["SHR_M2_Parameter"]["value"];
  SHR_M3_Parameter_ = config["SHR_M3_Parameter"]["value"];
  SHR_M4_Parameter_ = config["SHR_M4_Parameter"]["value"];
  // MPL_Parameter_ = config["MPL_Parameter"]["value"];
  MPL_Parameter_ = 2;
  // XOL_Mode_ = config["XOL_Mode"]["value"];
  XOL_Mode_ = 0;
  // NDE_Parameter_ = config["NDE_Parameter"]["value"];
  NDE_Parameter_ = 1;
  Partition_Parameter_ = config["Partition_Parameter"]["value"];
  // ML_Parameter_ = config["ML_Parameter"]["value"];
  ML_Parameter_ = 5;
  // Local_Parameter_ = config["Local_Parameter"]["value"];
  Local_Parameter_ = 1;
  // tolerance_Parameter_ = config["tolerance_Parameter"]["value"];
  tolerance_Parameter_ = 3;
  // BS_Parameter_ = config["BS_Parameter"]["value"];
  BS_Parameter_ = 1;
  // objpart_Parameter_ = config["objpart_Parameter"]["value"];
  objpart_Parameter_ = 0;
  // XOL_Parameter_ = config["XOL_Parameter"]["value"];
  XOL_Parameter_ = 2;
  dbConfig::checkParameter();
}

dbConfig::dbConfig(int BoundaryCondition, int SON, int DoublePowerRail,
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
           int BS_Parameter, int objpart_Parameter, int XOL_Parameter){
  BoundaryCondition_ = BoundaryCondition;
  SON_ = SON;
  DoublePowerRail_ = DoublePowerRail;
  MM_Parameter_ = MM_Parameter;
  EXT_Parameter_ = EXT_Parameter;
  BCP_Parameter_ = BCP_Parameter;
  MAR_M1_Parameter_ = MAR_M1_Parameter;
  MAR_M2_Parameter_ = MAR_M2_Parameter;
  MAR_M3_Parameter_ = MAR_M3_Parameter;
  MAR_M4_Parameter_ = MAR_M4_Parameter;
  EOL_M1_B_ADJ_Parameter_ = EOL_M1_B_ADJ_Parameter;
  EOL_M1_B_Parameter_ = EOL_M1_B_Parameter;
  EOL_M2_R_ADJ_Parameter_ = EOL_M2_R_ADJ_Parameter;
  EOL_M2_R_Parameter_ = EOL_M2_R_Parameter;
  EOL_M3_B_ADJ_Parameter_ = EOL_M3_B_ADJ_Parameter;
  EOL_M3_B_Parameter_ = EOL_M3_B_Parameter;
  EOL_M4_R_ADJ_Parameter_ = EOL_M4_R_ADJ_Parameter;
  EOL_M4_R_Parameter_ = EOL_M4_R_Parameter;
  VR_M1M2_Parameter_ = VR_M1M2_Parameter;
  VR_M2M3_Parameter_ = VR_M2M3_Parameter;
  VR_M3M4_Parameter_ = VR_M3M4_Parameter;
  PRL_M1_Parameter_ = PRL_M1_Parameter;
  PRL_M2_Parameter_ = PRL_M2_Parameter;
  PRL_M3_Parameter_ = PRL_M3_Parameter;
  PRL_M4_Parameter_ = PRL_M4_Parameter;
}

dbConfig::dbConfig(const std::string &config_path, std::string infile) {
  std::ifstream config_file(config_path);
  nlohmann::json config = nlohmann::json::parse(config_file);
  config_file.close();
  
  // parse into variables
  Z3Seed_ = config["z3_seed"]["value"];
  // BoundaryCondition_ = config["BoundaryCondition"]["value"];
  BoundaryCondition_ = 0;
  // SON_ = config["SON"]["value"];
  SON_ = 1;
  // DoublePowerRail_ = config["DoublePowerRail"]["value"];
  DoublePowerRail_ = 0;
  // MM_Parameter_ = config["MM_Parameter"]["value"];
  MM_Parameter_ = 4;
  // EXT_Parameter_ = config["EXT_Parameter"]["value"];
  EXT_Parameter_ = 0;
  // BCP_Parameter_ = config["BCP_Parameter"]["value"];
  BCP_Parameter_ = 1;
  // MAR_Parameter_ = config["MAR_Parameter"]["value"];
  MAR_M1_Parameter_ = config["MAR_M1_Parameter"]["value"];
  MAR_M2_Parameter_ = config["MAR_M2_Parameter"]["value"];
  MAR_M3_Parameter_ = config["MAR_M3_Parameter"]["value"];
  MAR_M4_Parameter_ = config["MAR_M4_Parameter"]["value"];
  // EOL_Parameter_ = config["EOL_Parameter"]["value"];
  EOL_M1_B_ADJ_Parameter_ = config["EOL_M1_B_ADJ_Parameter"]["value"];
  EOL_M1_B_Parameter_ = config["EOL_M1_B_Parameter"]["value"];
  EOL_M2_R_ADJ_Parameter_ = config["EOL_M2_R_ADJ_Parameter"]["value"];
  EOL_M2_R_Parameter_ = config["EOL_M2_R_Parameter"]["value"];
  EOL_M3_B_ADJ_Parameter_ = config["EOL_M3_B_ADJ_Parameter"]["value"];
  EOL_M3_B_Parameter_ = config["EOL_M3_B_Parameter"]["value"];
  EOL_M4_R_ADJ_Parameter_ = config["EOL_M4_R_ADJ_Parameter"]["value"];
  EOL_M4_R_Parameter_ = config["EOL_M4_R_Parameter"]["value"];
  // VR_Parameter_ = config["VR_Parameter"]["value"];
  VR_M1M2_Parameter_ = config["VR_M1M2_Parameter"]["value"];
  VR_M2M3_Parameter_ = config["VR_M2M3_Parameter"]["value"];
  VR_M3M4_Parameter_ = config["VR_M3M4_Parameter"]["value"];
  // PRL_Parameter_ = config["PRL_Parameter"]["value"];
  PRL_M1_Parameter_ = config["PRL_M1_Parameter"]["value"];
  PRL_M2_Parameter_ = config["PRL_M2_Parameter"]["value"];
  PRL_M3_Parameter_ = config["PRL_M3_Parameter"]["value"];
  PRL_M4_Parameter_ = config["PRL_M4_Parameter"]["value"];
  // SHR_Parameter_ = config["SHR_Parameter"]["value"];
  SHR_M1_Parameter_ = config["SHR_M1_Parameter"]["value"];
  SHR_M2_Parameter_ = config["SHR_M2_Parameter"]["value"];
  SHR_M3_Parameter_ = config["SHR_M3_Parameter"]["value"];
  SHR_M4_Parameter_ = config["SHR_M4_Parameter"]["value"];
  // MPL_Parameter_ = config["MPL_Parameter"]["value"];
  MPL_Parameter_ = 2;
  // XOL_Mode_ = config["XOL_Mode"]["value"];
  XOL_Mode_ = 0;
  // NDE_Parameter_ = config["NDE_Parameter"]["value"];
  NDE_Parameter_ = 1;
  Partition_Parameter_ = config["Partition_Parameter"]["value"];
  // ML_Parameter_ = config["ML_Parameter"]["value"];
  ML_Parameter_ = 5;
  // Local_Parameter_ = config["Local_Parameter"]["value"];
  Local_Parameter_ = 1;
  // tolerance_Parameter_ = config["tolerance_Parameter"]["value"];
  tolerance_Parameter_ = 3;
  // BS_Parameter_ = config["BS_Parameter"]["value"];
  BS_Parameter_ = 1;
  // objpart_Parameter_ = config["objpart_Parameter"]["value"];
  objpart_Parameter_ = 0;
  // XOL_Parameter_ = config["XOL_Parameter"]["value"];
  XOL_Parameter_ = 2;
  // PE_Parameter_ = config["PE_Parameter"]["value"];
  PE_Parameter_ = 0;
  // PE_Mode_ = config["PE_Mode"]["value"];
  PE_Mode_ = 1;
  // M1_Separation_Parameter_ = config["M1_Separation_Parameter"]["value"];
  M1_Separation_Parameter_ = 1;

  // Special Condition for DFF cells
  if (infile.find("DFF") != std::string::npos) {
    Partition_Parameter_ = 2;
    BCP_Parameter_ = 1;
    NDE_Parameter_ = 0;
    BS_Parameter_ = 0;
    std::cout << "\na     *** DFF Cell Detected. Forcing the following constraint ... ***";
    std::cout << "\na     *** [INFO] Disable NDE, BS; Partitioning = 2; BCP = 1; ***";
  } else if (Partition_Parameter_ == 2) 
  {
    std::cout << "\n    *** Partitioning = 2; BCP = 1; ***";
    BCP_Parameter_ = 1;
    NDE_Parameter_ = 0;
    BS_Parameter_ = 0;
  }
  

  dbConfig::checkParameter();
}

void dbConfig::checkParameter() {
  // if (MAR_Parameter_ == 0) {
  //   std::cout << "\na     *** Disable MAR (When Parameter == 0) ***";
  // }
  if (MAR_M1_Parameter_ == 0) {
    std::cout << "\na     *** Disable MAR_M1 (When Parameter == 0) ***";
  }
  if (MAR_M2_Parameter_ == 0) {
    std::cout << "\na     *** Disable MAR_M2 (When Parameter == 0) ***";
  }
  if (MAR_M3_Parameter_ == 0) {
    std::cout << "\na     *** Disable MAR_M3 (When Parameter == 0) ***";
  }
  if (MAR_M4_Parameter_ == 0) {
    std::cout << "\na     *** Disable MAR_M4 (When Parameter == 0) ***";
  }

  // if (EOL_Parameter_ == 0) {
  //   std::cout << "\na     *** Disable EOL (When Parameter == 0) ***";
  // }
  if (EOL_M1_B_ADJ_Parameter_ == 0) {
    std::cout << "\na     *** Disable EOL_M1_B_ADJ (When Parameter == 0) ***";
  }
  if (EOL_M1_B_Parameter_ == 0) {
    std::cout << "\na     *** Disable EOL_M1_B (When Parameter == 0) ***";
  }
  if (EOL_M2_R_ADJ_Parameter_ == 0) {
    std::cout << "\na     *** Disable EOL_M2_R_ADJ (When Parameter == 0) ***";
  }
  if (EOL_M2_R_Parameter_ == 0) {
    std::cout << "\na     *** Disable EOL_M2_R (When Parameter == 0) ***";
  }
  if (EOL_M3_B_ADJ_Parameter_ == 0) {
    std::cout << "\na     *** Disable EOL_M3_B_ADJ (When Parameter == 0) ***";
  }
  if (EOL_M3_B_Parameter_ == 0) {
    std::cout << "\na     *** Disable EOL_M3_B (When Parameter == 0) ***";
  }
  if (EOL_M4_R_ADJ_Parameter_ == 0) {
    std::cout << "\na     *** Disable EOL_M4_R_ADJ (When Parameter == 0) ***";
  }
  if (EOL_M4_R_Parameter_ == 0) {
    std::cout << "\na     *** Disable EOL_M4_R (When Parameter == 0) ***";
  }

  // if (VR_Parameter_ == 0) {
  //   std::cout << "\na     *** Disable VR (When Parameter == 0) ***";
  // }
  if (VR_M1M2_Parameter_ == 0) {
    std::cout << "\na     *** Disable VR_M1M2 (When Parameter == 0) ***";
  }
  if (VR_M2M3_Parameter_ == 0) {
    std::cout << "\na     *** Disable VR_M2M3 (When Parameter == 0) ***";
  }
  if (VR_M3M4_Parameter_ == 0) {
    std::cout << "\na     *** Disable VR_M3M4 (When Parameter == 0) ***";
  }

  // if (PRL_Parameter_ == 0) {
  //   std::cout << "\na     *** Disable PRL (When Parameter == 0) ***";
  // }
  if (PRL_M1_Parameter_ == 0) {
    std::cout << "\na     *** Disable PRL_M1 (When Parameter == 0) ***";
  }
  if (PRL_M2_Parameter_ == 0) {
    std::cout << "\na     *** Disable PRL_M2 (When Parameter == 0) ***";
  }
  if (PRL_M3_Parameter_ == 0) {
    std::cout << "\na     *** Disable PRL_M3 (When Parameter == 0) ***";
  }
  if (PRL_M4_Parameter_ == 0) {
    std::cout << "\na     *** Disable PRL_M4 (When Parameter == 0) ***";
  }

  // if (SHR_Parameter_ < 2) {
  //   std::cout << "\na     *** Disable SHR (When Parameter <= 1) ***";
  // }
  if (SHR_M1_Parameter_ < 2) {
    std::cout << "\na     *** Disable SHR_M1 (When Parameter <= 1) ***";
  }
  if (SHR_M2_Parameter_ < 2) {
    std::cout << "\na     *** Disable SHR_M2 (When Parameter <= 1) ***";
  }
  if (SHR_M3_Parameter_ < 2) {
    std::cout << "\na     *** Disable SHR_M3 (When Parameter <= 1) ***";
  }
  if (SHR_M4_Parameter_ < 2) {
    std::cout << "\na     *** Disable SHR_M4 (When Parameter <= 1) ***";
  }
  
  if (Local_Parameter_ == 0) {
    std::cout << "\na     *** Disable Localization (When Parameter == 0) ***";
  }
  if (Partition_Parameter_ == 0) {
    std::cout << "\na     *** Disable Cell Partitioning (When Parameter == 0) ***";
  }
  if (objpart_Parameter_ == 0) {
    std::cout
        << "\na     *** Disable Objective Partitioning (When Parameter == 0) ***";
  }
  if (BS_Parameter_ == 0) {
    std::cout << "\na     *** Disable Breaking Symmetry (When Parameter == 0) ***";
  }
  if (NDE_Parameter_ == 0) {
    std::cout << "\na     *** Disable FST (When Parameter == 0) ***";
  }

  if (MPL_Parameter_ < 2 || MPL_Parameter_ > 4) {
    std::cout << "[ERROR] MPL_Parameter is not valid!\n";
    exit(-1);
  }

  std::cout << "\n";

  if (XOL_Mode_ == 0) {
    XOL_Parameter_ = 2;
  } else {
    XOL_Parameter_ = 4;
  }
}

void dbConfig::checkPartition_Parameter(int sizeOfPartition) {
  if (Partition_Parameter_ != 0) {
    if (sizeOfPartition == 0) {
      std::cout << "a     [WARNING] Partition_Parameter is not valid! Partition size is 0!\n";
      Partition_Parameter_ = 0;
      NDE_Parameter_ = 1;
      BS_Parameter_ = 1;
    }
  }
}