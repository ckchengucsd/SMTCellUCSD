import sys

"""main function takes an argument, which is the path to the .z3 file
    returns the result of the satisfiability check
"""
def ifSAT(path):
    with open(path, 'r') as f:
        lines = f.readlines()
        # check if the first 1000 lines contain "unsat"
        if any("unsat" in line for line in lines[:1000]):
            return False
        # check if any line contains "sat"
        elif any("sat" in line for line in lines[:1000]):
            return True
        else:
            print("[ERROR] UNKNOWN Z3 RESULT")
            sys.exit(1)

def main():
    if len(sys.argv) < 2:
        print("[INFO] Usage: python check_sat.py <path_to_z3_file>")
        sys.exit(1)
    path = sys.argv[1]

    if ifSAT(path):
        print("a     ##########     Z3 Result: SAT      ##########")
    else:
        print("a     ##########     Z3 Result: UNSAT    ##########")

if __name__ == "__main__":
    main()