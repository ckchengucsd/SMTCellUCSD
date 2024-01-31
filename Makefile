################################################
# Tool config                                  #
################################################
# python
PYTHON=$(shell which python3)

# Z3 (4.8.5)
Z3=$(shell which z3)
Z3FLAGS=parallel.enable=true

# Klayout
KLAYOUT=$(shell which klayout)

# GDT to GDS
GDT2GDS=$(shell which gdt2gds.Linux)

# Misc
RM=rm -rf
TIME=time

################################################
# Input config                                 #
################################################
CELL_NAME = INV_X1 INV_X2 INV_X4 INV_X8 AND2_X1 AND2_X2 AND3_X1 AND3_X2 \
OR2_X1 OR2_X2 OR3_X1 OR3_X2 NAND2_X1 NAND2_X2 NAND3_X1 NAND3_X2 NAND4_X1 NAND4_X2 \
NOR2_X1 NOR3_X1 NOR4_X1 NOR2_X2 NOR3_X2 NOR4_X2 \
AOI21_X1 AOI21_X2 OAI21_X1 OAI21_X2 AOI22_X1 AOI22_X2 OAI22_X1 OAI22_X2 \
XOR2_X1 MUX2_X1 BUF_X1 BUF_X2 BUF_X4 BUF_X8 DFFHQN_X1 LHQ_X1

CONFIG=./config/config_3F_5T.json
UTIL_CONFIG=./util/check_config.py
FIN 	:= $(shell $(PYTHON) $(UTIL_CONFIG) $(CONFIG) NumFin)
TRACK 	:= $(shell $(PYTHON) $(UTIL_CONFIG) $(CONFIG) NumTrack)

# GDS/LEF config
CHSTR 		:= $(shell $(PYTHON) $(UTIL_CONFIG) $(CONFIG) CHSTR)
SITE_NAME 	:= $(shell $(PYTHON) $(UTIL_CONFIG) $(CONFIG) SiteName)
MPO 		:= $(shell $(PYTHON) $(UTIL_CONFIG) $(CONFIG) MPO)
PGPIN 		:= $(shell $(PYTHON) $(UTIL_CONFIG) $(CONFIG) PGPIN)
DR 			:= $(shell $(PYTHON) $(UTIL_CONFIG) $(CONFIG) DR)
CPP 		:= $(shell $(PYTHON) $(UTIL_CONFIG) $(CONFIG) CPP)
M1_Factor 	:= $(shell $(PYTHON) $(UTIL_CONFIG) $(CONFIG) M1_Factor)
M0P 		:= $(shell $(PYTHON) $(UTIL_CONFIG) $(CONFIG) M0P)
M2_Factor 	:= $(shell $(PYTHON) $(UTIL_CONFIG) $(CONFIG) M2_Factor)
M1P 		:= $(shell $(PYTHON) $(UTIL_CONFIG) $(CONFIG) M1P)
M3_Factor 	:= $(shell $(PYTHON) $(UTIL_CONFIG) $(CONFIG) M3_Factor)
M2P 		:= $(shell $(PYTHON) $(UTIL_CONFIG) $(CONFIG) M2P)
M4_Factor 	:= $(shell $(PYTHON) $(UTIL_CONFIG) $(CONFIG) M4_Factor)
ENC 		:= $(shell $(PYTHON) $(UTIL_CONFIG) $(CONFIG) ENC)
FINWIDTH 	:= $(shell $(PYTHON) $(UTIL_CONFIG) $(CONFIG) FinWidth)
FINPITCH 	:= $(shell $(PYTHON) $(UTIL_CONFIG) $(CONFIG) FinPitch)
GATEWIDTH 	:= $(shell $(PYTHON) $(UTIL_CONFIG) $(CONFIG) GateWidth)
M0WIDTH 	:= $(shell $(PYTHON) $(UTIL_CONFIG) $(CONFIG) M0Width)
M1WIDTH 	:= $(shell $(PYTHON) $(UTIL_CONFIG) $(CONFIG) M1Width)
M2WIDTH 	:= $(shell $(PYTHON) $(UTIL_CONFIG) $(CONFIG) M2Width)
LIB_NAME 	:= $(CHSTR)T_$(FIN)F_$(CPP)CPP_$(M0P)M0P_$(M1P)M1P_$(M2P)M2P_$(MPO)MPO_$(DR)_$(PGPIN)

################################################
# Output config                                #
################################################
RESULTS_DIR=./results/results_$(FIN)F$(TRACK)T_$(CPP)$(M1P)
LOG_DIR=$(RESULTS_DIR)/logs

################################################
# Commands generating executable files         #
# ################################################

genSMTInputAGR=./build/genSMTInputAGR
convSMTResult=./build/convSMTResult
genTestCase=./build/genTestCase

clean:
	$(RM) genSMTInputAGR genTestCase genTestCase_cfet convSMTResult

################################################
# Commands to run SMTCell flow                 #
################################################

# preliminary check
AGRcheck:
	$(PYTHON) ./util/check_AGR.py $(CONFIG)

# generate PinLayouts
genPinLayouts:
	make genTestCase
	./genTestCase ./Library/ASAP7_PDKandLIB_v1p5/asap7libs_24/cdl/lvs/asap7_75t_R.cdl 5 3

# basic flow
SMTCell:
	mkdir -m 777 -p $(RESULTS_DIR)/inputSMT
	mkdir -m 777 -p $(RESULTS_DIR)/Z3
	mkdir -m 777 -p $(RESULTS_DIR)/solutionSMT
	mkdir -m 777 -p $(RESULTS_DIR)/logs
	mkdir -m 777 -p $(RESULTS_DIR)/logs/debug
	$(foreach CELL,$(CELL_NAME),\
		$(genSMTInputAGR) ./inputPinLayouts/pinLayouts_$(FIN)F_$(TRACK)T/$(CELL).pinLayout $(CONFIG) $(RESULTS_DIR);\
		$(TIME) $(Z3) $(Z3FLAGS) $(RESULTS_DIR)/inputSMT/$(CELL)_$(FIN)F_$(TRACK)T.smt2 > $(RESULTS_DIR)/Z3/$(CELL)_$(FIN)F_$(TRACK)T.z3;\
		$(PYTHON) ./util/check_sat.py $(RESULTS_DIR)/Z3/$(CELL)_$(FIN)F_$(TRACK)T.z3;\
		$(convSMTResult) $(RESULTS_DIR)/Z3/$(CELL)_$(FIN)F_$(TRACK)T.z3 $(CELL) $(RESULTS_DIR)/solutionSMT ./inputPinLayouts/pinLayouts_$(FIN)F_$(TRACK)T $(M1_Factor) $(M3_Factor);)
	mkdir -m 777 -p $(RESULTS_DIR)/gdslef
	mkdir -m 777 -p $(RESULTS_DIR)/gdslef/$(LIB_NAME)
	$(PYTHON) ./util/LayoutGen/genGdsLef.py $(RESULTS_DIR)/solutionSMT $(RESULTS_DIR)/gdslef $(M1P) $(M2P) $(CPP) $(ENC) $(SITE_NAME) $(MPO) $(DR) best 0 0 $(FINWIDTH) $(FINPITCH) $(GATEWIDTH) $(M0WIDTH) $(M1WIDTH) $(M2WIDTH) $(M1_Factor) $(M3_Factor)
	$(GDT2GDS) < $(RESULTS_DIR)/gdslef/$(LIB_NAME)/$(LIB_NAME).gdt > $(RESULTS_DIR)/gdslef/$(LIB_NAME)/$(LIB_NAME).gds

# pre-partition flow
SMTCell_prepartition:
	mkdir -m 777 -p $(RESULTS_DIR)/inputSMT
	mkdir -m 777 -p $(RESULTS_DIR)/Z3
	mkdir -m 777 -p $(RESULTS_DIR)/solutionSMT
	mkdir -m 777 -p $(RESULTS_DIR)/logs
	mkdir -m 777 -p $(RESULTS_DIR)/logs/debug
	$(foreach CELL,$(CELL_NAME),\
		$(genSMTInputAGR) ./inputPinLayouts/pinLayouts_$(FIN)F_$(TRACK)T_prepartition/$(CELL).pinLayout $(CONFIG) $(RESULTS_DIR);\
		$(TIME) $(Z3) $(Z3FLAGS) $(RESULTS_DIR)/inputSMT/$(CELL)_$(FIN)F_$(TRACK)T.smt2 > $(RESULTS_DIR)/Z3/$(CELL)_$(FIN)F_$(TRACK)T.z3;\
		$(PYTHON) ./util/check_sat.py $(RESULTS_DIR)/Z3/$(CELL)_$(FIN)F_$(TRACK)T.z3;\
		./convSMTResult $(RESULTS_DIR)/Z3/$(CELL)_$(FIN)F_$(TRACK)T.z3 $(CELL) $(RESULTS_DIR)/solutionSMT ./inputPinLayouts/pinLayouts_$(FIN)F_$(TRACK)T $(M1_Factor) $(M3_Factor);)
	mkdir -m 777 -p $(RESULTS_DIR)/gdslef
	mkdir -m 777 -p $(RESULTS_DIR)/gdslef/$(LIB_NAME)
	$(PYTHON) ./util/LayoutGen/genGdsLef.py $(RESULTS_DIR)/solutionSMT $(RESULTS_DIR)/gdslef $(M1P) $(M2P) $(CPP) $(ENC) $(SITE_NAME) $(MPO) $(DR) best 0 0 $(FINWIDTH) $(FINPITCH) $(GATEWIDTH) $(M0WIDTH) $(M1WIDTH) $(M2WIDTH) $(M1_Factor) $(M3_Factor)
	$(GDT2GDS) < $(RESULTS_DIR)/gdslef/$(LIB_NAME)/$(LIB_NAME).gdt > $(RESULTS_DIR)/gdslef/$(LIB_NAME)/$(LIB_NAME).gds

convertSMTResult:
	$(foreach CELL,$(CELL_NAME),\
		$(convSMTResult) $(RESULTS_DIR)/Z3/$(CELL)_$(FIN)F_$(TRACK)T.z3 $(CELL) $(RESULTS_DIR)/solutionSMT ./inputPinLayouts/pinLayouts_$(FIN)F_$(TRACK)T $(M1_Factor) $(M3_Factor);)

viewSMTCell:
	$(PYTHON) ./util/LayoutGen/genGdsLef.py $(RESULTS_DIR)/solutionSMT $(RESULTS_DIR)/gdslef $(M1P) $(M2P) $(CPP) $(ENC) $(SITE_NAME) $(MPO) $(DR) best 0 0 $(FINWIDTH) $(FINPITCH) $(GATEWIDTH) $(M0WIDTH) $(M1WIDTH) $(M2WIDTH) $(M1_Factor) $(M3_Factor)
	$(GDT2GDS) < $(RESULTS_DIR)/gdslef/$(LIB_NAME)/$(LIB_NAME).gdt > $(RESULTS_DIR)/gdslef/$(LIB_NAME)/$(LIB_NAME).gds
	$(KLAYOUT) $(RESULTS_DIR)/gdslef/$(LIB_NAME)/$(LIB_NAME).gds

# use the following commands to run components of the flow
SMTCell_z3:
	mkdir -m 777 -p $(RESULTS_DIR)/Z3
	mkdir -m 777 -p $(RESULTS_DIR)/solutionSMT
	$(foreach CELL,$(CELL_NAME),\
		$(TIME) $(Z3) $(RESULTS_DIR)/inputSMT/$(CELL)_$(FIN)F_$(TRACK)T.smt2 > $(RESULTS_DIR)/Z3/$(CELL)_$(FIN)F_$(TRACK)T.z3;\
		$(PYTHON) ./util/check_sat.py $(RESULTS_DIR)/Z3/$(CELL)_$(FIN)F_$(TRACK)T.z3;\
		$(convSMTResult) $(RESULTS_DIR)/Z3/$(CELL)_$(FIN)F_$(TRACK)T.z3 $(CELL) $(RESULTS_DIR)/solutionSMT ./inputPinLayouts/pinLayouts_$(FIN)F_$(TRACK)T $(M1_Factor) $(M3_Factor);)

# clean commands
clean_cache:
	$(RM) ./util/__pycache__
	$(RM) ./util/prepartition/__pycache__

cleanAll:
	make clean_cache
	@echo "[WARNING] Are you sure you want to clean everything? All SMTCell results will be removed! (y/n)"
	@read -r response; \
    if [ "$$response" = "y" ]; then \
        rm -rf -r ./results/*; \
        echo "Cleaning completed."; \
    else \
        echo "Cleaning aborted."; \
    fi

cleanSMTCell:
	make clean_cache
	@echo "[WARNING] Are you sure you want to clean $(RESULTS_DIR)? (y/n)"
	@read -r response; \
    if [ "$$response" = "y" ]; then \
        $(RM) $(RESULTS_DIR)/*; \
        echo "Cleaning completed."; \
    else \
        echo "Cleaning aborted."; \
    fi

clean_genSMTCInputAGR:
	$(RM) ./genSMTInputAGR

clean_viewSMTCell:
	make clean_cache
	$(RM) ./results/gdslef/*
