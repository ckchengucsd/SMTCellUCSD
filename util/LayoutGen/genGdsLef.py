# Description: This file is to generate GDS and LEF files from the given
# Original Author: Minsoo Kim et al. (ABK Group)
# Modified by: Yucheng Wang
# Link: https://github.com/ABKGroup/PROBE3.0/blob/main/DesignEnablement/Cadence/scripts/genGdsLef.py

import os
import sys
# import subprocess as sp
# from array import *
# from enum import Enum
# import numpy as np
# import math

# custom class
from cellEntity import *
from techEntity import *
from GdsLefUtil import *

# AGR
# M1_FACTOR = 0
# M3_FACTOR = 0

# Main codes
if len(sys.argv) <= 19:
	print("Usage:   python generate.py     <input> <output> <m1Pitch> <m2Pitch> <cppWidth>")
	print("                                <yMargin> <siteName> <mpoMode> <drMode> <rpaMode>")
	print("                                <pinExtVal> <powerMetalWidth> <finWidth> <finPitch>")
	print("                                <gateWidth> <m0Width> <m1Width> <m2Width> <cpFactor> <m1Factor>")
	sys.exit(1)

# AGR FLAG : extract metal_1 pitch from config file
# M1_FACTOR = check_config.extract_config(configPath, "M1_Factor")
# M3_FACTOR = check_config.extract_config(configPath, "M3_Factor")

# print("M1_FACTOR: ", M1_FACTOR)
# print("M3_FACTOR: ", M3_FACTOR)

# User input
input_dir = sys.argv[1] + "/"
output_dir = sys.argv[2] + "/"
m1_pitch = int(sys.argv[3])
m2_pitch = int(sys.argv[4])
cp_pitch = int(sys.argv[5])
via_enc = int(sys.argv[6])
site_name = sys.argv[7]
mpo_mode = sys.argv[8]
dr_mode = sys.argv[9]
rpa_mode = sys.argv[10]
pin_ext_val = int(sys.argv[11])
power_metal_width = int(sys.argv[12])
fin_width = float(sys.argv[13])
fin_pitch = float(sys.argv[14])
gate_width = float(sys.argv[15])
m0_width = float(sys.argv[16])
m1_width = float(sys.argv[17])
m2_width = float(sys.argv[18])
# config_path = sys.argv[19]
cp_factor = float(sys.argv[19])
m1_factor = float(sys.argv[20])

# print the input parameters
print("######################## GDS/LEF Generation Input Parameters ##########################")
print(f"   input_dir: {input_dir} ... checking directory existence ... {os.path.isdir(input_dir)}")
print(f"   output_dir: {output_dir} ... checking directory existence ... {os.path.isdir(output_dir)}")
print(f"   m1_pitch: {m1_pitch}")
print(f"   m2_pitch: {m2_pitch}")
print(f"   cp_pitch: {cp_pitch}")
print(f"   via_enc: {via_enc}")
print(f"   site_name: {site_name}")
print(f"   mpo_mode: {mpo_mode}")
print(f"   dr_mode: {dr_mode}")
print(f"   rpa_mode: {rpa_mode}")
print(f"   pin_ext_val: {pin_ext_val}")
print(f"   power_metal_width: {power_metal_width}")
print(f"   fin_width: {fin_width}")
print(f"   fin_pitch: {fin_pitch}")
print(f"   gate_width: {gate_width}")
print(f"   m0_width: {m0_width}")
print(f"   m1_width: {m1_width}")
print(f"   m2_width: {m2_width}")
print(f"   cp_factor: {cp_factor}")
print(f"   m1_factor: {m1_factor}")
print("######################## Listing Files From Input Directory ##########################")
fileList = os.listdir(input_dir)

for file in fileList:
	print(f"  {file}")

tech = TechInfo(0,
                0, 
                m1_pitch,
                m2_pitch,
                cp_pitch,
                via_enc,
                site_name,
                BprMode.NONE,
                Utility.GetMpoFlag(mpo_mode),
                Utility.GetDrFlag(dr_mode),
                Utility.GetRpaMode(rpa_mode),
                pin_ext_val,
                power_metal_width,
                fin_width,
                fin_pitch,
                gate_width,
                m0_width,
                m1_width,
                m2_width,
                cp_factor,
                m1_factor,
                1.0,
                1.0
)

# seed set
# np.random.seed(100)

# generate six lef files
#for bprFlag in [BprMode.METAL1, BprMode.METAL2, BprMode.BPR]:
for bprFlag in [BprMode.METAL1, BprMode.BPR]:
	tech.bprFlag = bprFlag
	Utility.GenerateLef(input_dir, fileList, output_dir, tech)

# print("totalMacros:", numTotalMacros, "numMultiMetal1Macros:", numMultiMetal1Macros, "numMultiMetal1MacrosDFF:", numMultiMetal1MacrosDFF) 
