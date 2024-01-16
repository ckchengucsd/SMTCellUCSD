import sys
import check_config
from fractions import Fraction

"""main function to check if the AGR is feasible for graph generation
    - CPP cannot be a odd number since
"""

def main():
    if len(sys.argv) == 2:
        print("[INFO] Usage: python check_AGR.py <path_to_config_file>")
        sys.exit(1)

    print(
        "[INFO] Performing AGR check. Conversion to your config file may be needed..."
    )

    # extract values from config file
    path = sys.argv[1]
    config_dict = check_config.extract_all_config(path)

    # Metal Pitch => Metal Pitch Factor
    CPP = config_dict["CPP"]["value"]
    M1P = config_dict["M1P"]["value"]

    if CPP % 2 == 1:
        print("a     ##########     CPP is odd => AGR Conversion    ##########")
        converted_config_dict = convert_AGR(config_dict)
        check_config.write_config(path, converted_config_dict)
    else:
        print("a     ##########     CPP is even => No Conversion Needed     ##########")
        M1_Factor, M3_Factor = CPP / 2, M1P
        config_dict["M1_Factor"]["value"] = M1_Factor
        config_dict["M3_Factor"]["value"] = M3_Factor
        check_config.write_config(path, config_dict)


def convert_AGR(config_dict):
    # update Factors and keep same ratio by multiplying M1P by 2
    M1_Factor = config_dict["CPP"]["value"]
    M3_Factor = config_dict["M1P"]["value"] * 2

    # check if M1_Factor and M3_Factor are already converted
    if (
        M1_Factor == config_dict["M1_Factor"]["value"]
        and M3_Factor == config_dict["M3_Factor"]["value"]
    ):
        return config_dict

    # update offsets
    # NOTICE: M2/M4 Offset is not used in AGR
    M1_Offset = config_dict["M1_Offset"]["value"] * 2
    M3_Offset = config_dict["M3_Offset"]["value"] * 2

    # update Design Rule related in config_dict
    MAR_Parameter = config_dict["MAR_Parameter"]["value"] * 2
    EOL_Parameter = config_dict["EOL_Parameter"]["value"] * 2
    VR_Parameter = config_dict["VR_Parameter"]["value"] * 2
    PRL_Parameter = config_dict["PRL_Parameter"]["value"] * 2
    SHR_Parameter = config_dict["SHR_Parameter"]["value"] * 2

    # write back to config_dict
    config_dict["M1_Factor"]["value"] = M1_Factor
    config_dict["M3_Factor"]["value"] = M3_Factor
    config_dict["M1_Offset"]["value"] = M1_Offset
    config_dict["M3_Offset"]["value"] = M3_Offset
    config_dict["MAR_Parameter"]["value"] = MAR_Parameter
    config_dict["EOL_Parameter"]["value"] = EOL_Parameter
    config_dict["VR_Parameter"]["value"] = VR_Parameter
    config_dict["PRL_Parameter"]["value"] = PRL_Parameter
    config_dict["SHR_Parameter"]["value"] = SHR_Parameter

    return config_dict


if __name__ == "__main__":
    main()
