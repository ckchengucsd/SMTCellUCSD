import json
import sys

"""main function takes an argument, which is the path to the config file and a key to retrieve
"""
def main():
    if len(sys.argv) < 3:
        print("[INFO] Usage: python config.py <path_to_config_file> <key>")
        sys.exit(1)
    path = sys.argv[1]
    key = sys.argv[2]
    print(extract_config(path, key))

# check if the content of a config file is valid
def check_config(path):
    with open(path, 'r') as f:
        config = json.load(f)
        # check if the config file contains all the necessary keys
        keys = ["CPP", "M1P", "M1_Offset", "M3_Offset", "MAR_Parameter", "EOL_Parameter", "VR_Parameter", "PRL_Parameter", "SHR_Parameter"]
        for key in keys:
            if key not in config:
                print("[ERROR] Config file does not contain key: {}".format(key))
                sys.exit(1)
        # check if the values are valid
        CPP = config["CPP"]["value"]
        M1P = config["M1P"]["value"]
        M1_Offset = config["M1_Offset"]["value"]
        M3_Offset = config["M3_Offset"]["value"]
        MAR_Parameter = config["MAR_Parameter"]["value"]
        EOL_Parameter = config["EOL_Parameter"]["value"]
        VR_Parameter = config["VR_Parameter"]["value"]
        PRL_Parameter = config["PRL_Parameter"]["value"]
        SHR_Parameter = config["SHR_Parameter"]["value"]
        # TODO check if the values are valid
        pass

# extract a single value from config file
def extract_config(path, key):
    with open(path, 'r') as f:
        config = json.load(f)
        return config[key]["value"]

# extract all values from config file into a dictionary
def extract_all_config(path):
    with open(path, 'r') as f:
        config = json.load(f)
        return config

# dump a dictionary into a config file
def write_config(path, config):
    with open(path, 'w') as f:
        json.dump(config, f, indent=4)

if __name__ == "__main__":
    main()