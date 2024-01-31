import os
import sys

"""main function takes an argument, which is the path to the all the .conv files
    returns the result of the cell metrics"""


def evaluate(conv_path, horizontal_pitch, cpp):
    """Evaluate the current cell layout"""
    # dict to store the cell name and its corresponding cell metrics
    # via_cnt_dict = {}
    # metal_length_dict = {}
    # cell_width_dict = {}
    # top_track_cnt_dict = {}
    # cpp_dict = {}

    # Metrics to consider
    via_cnt = 0
    metal_length = 0
    cell_width = 0
    top_track_cnt = 0

    # check if CELL_PATH is a valid path
    if not os.path.exists(conv_path):
        print("[ERROR] NOT A VALID PATH: " + conv_path)
        sys.exit()

    line_cnt = 0
    with open(conv_path, "r") as f:
        line = f.read()
        # print all lines that start with 'METAL'
        for line in line.splitlines():
            line_cnt += 1

            if line.startswith("COST"):
                cell_width = int(line.split()[1])

            if line.startswith("METAL"):
                fromRow = int(line.split()[2])
                fromCol = int(line.split()[3])
                toRow = int(line.split()[4])
                toCol = int(line.split()[5])

                # vertical metal
                if fromCol == toCol:
                    metal_length += abs(fromRow - toRow) * horizontal_pitch

                # horizontal metal
                elif fromRow == toRow:
                    metal_length += abs(fromCol - toCol)

                # check top track
                metal_layer = int(line.split()[1])
                if metal_layer == 4:
                    top_track_cnt += 1

            if line.startswith("VIA "):
                via_cnt += 1

        if line_cnt <= 4:
            print("[ERROR] SOLUTION NOT COMPLETE")
            return -1, -1, -1, -1

        print("         [INFO] METAL LENGTH: " + str(metal_length))
        print("         [INFO] VIA COUNT: " + str(via_cnt))
        print("         [INFO] TOP TRACK COUNT: " + str(top_track_cnt))
        print("         [INFO] CELL WIDTH: " + str(cell_width))
        print("         [INFO] CPP: " + str(cell_width//(2*cpp) + 1))
    
    return via_cnt, metal_length, cell_width, top_track_cnt

        # # store the metrics into the dict
        # via_cnt_dict[file] = via_cnt
        # metal_length_dict[file] = metal_length
        # cell_width_dict[file] = cell_width
        # top_track_cnt_dict[file] = top_track_cnt
        # cpp_dict[file] = cell_width//(2*cpp) + 1
        
    # return via_cnt_dict, metal_length_dict, cell_width_dict, top_track_cnt_dict, cpp_dict


def main():
    if len(sys.argv) < 2:
        print("[INFO] Usage: python metrics.py <path_to_conv_files> <CPP>")
        sys.exit(1)

    curr_cell_path = sys.argv[1]
    cpp = int(sys.argv[2])

    print("a     [INFO] Evaluating cell: {}".format(curr_cell_path))

    # extract values from config file
    evaluate(curr_cell_path, 24, cpp)

    # use pandas to create a dataframe
    # df = pd.DataFrame.from_dict(via_cnt_dict, orient="index")
    # df.columns = ["via_cnt"]
    # df["metal_length"] = metal_length_dict.values()
    # df["cell_width"] = cell_width_dict.values()
    # df["top_track_cnt"] = top_track_cnt_dict.values()
    # df["cpp"] = cpp_dict.values()

    # print the dataframe to a csv file
    # df.to_csv(os.path.join(conv_path, "cell_metrics.csv"))


if __name__ == "__main__":
    main()
