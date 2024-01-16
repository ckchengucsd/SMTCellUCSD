# import os
# import sys
import subprocess as sp
from array import *
from enum import Enum
# import numpy as np
# import math

# custom class
from cellEntity import *

# To check the total insts
numTotalMacros = 0
numMultiMetal1Macros = 0
numMultiMetal1MacrosDFF = 0

def ExecuteCommand(cmd):
    # print( cmd )
    sp.call(cmd, shell=True)

class BprMode(Enum):
    NONE = 0
    METAL1 = 1
    METAL2 = 2
    BPR = 3

class MpoMode(Enum):
    NONE = 0
    TWO = 1
    THREE = 2
    MAX = 3

class DrMode(Enum):
    NONE = 0
    EL = 1
    ET = 2
    SL = 3
    ST = 4

class RpaMode(Enum):
    NONE = -1
    RANDOM = 0
    BEST = 1
    WORST = 2

class TechInfo:
    def __init__(
        self,
        numCpp,
        numTrack,
        m1Pitch,
        m2Pitch,
        cppWidth,
        yMargin,
        siteName,
        bprFlag,
        mpoFlag,
        drFlag,
        rpaMode,
        pinExtVal,
        powerMetalWidth,
        finWidth,
        finPitch,
        gateWidth,
        m0Width,
        m1Width,
        m2Width,
        cpFactor,
        m1Factor,
        x_scale,
        y_scale
    ):
        self.numCpp = int(numCpp)
        self.numTrack = int(numTrack)
        self.metal1Pitch = int(m1Pitch)
        self.metal2Pitch = int(m2Pitch)
        self.metal0Width = int(m0Width * 1000.0) * y_scale
        self.metal1Width = int(m1Width * 1000.0) * x_scale
        self.metal2Width = int(m2Width * 1000.0) * y_scale
        
		# AGR
        self.cp_factor = cpFactor
        self.m1_factor = m1Factor
        
        # rescale design
        self.x_scale = x_scale
        self.y_scale = y_scale
        
        # temp: m0p == m2p
        self.metal0Pitch = int(m2Pitch)
        print("m1Pitch", self.metal1Pitch, "m2Pitch", self.metal2Pitch)
        self.cppWidth = int(cppWidth)
        self.siteName = siteName
        self.maxCellWidth = 0
        self.realTrack = 0
        self.yMargin = int(yMargin)

        self.bprFlag = bprFlag
        self.mpoFlag = mpoFlag
        self.drFlag = drFlag
        self.rpaMode = rpaMode

        self.finWidth = finWidth
        self.finPitch = finPitch
        self.gateWidth = gateWidth

        self.pinExtVal = pinExtVal
        self.m1GridIdxPitch = int(m1Factor)

        self.update(False)

    def update(self, isMaxCellWidthUpdate):
        # metal width for pgpin width
        # self.metalWidth = int(self.metal2Pitch/2)
        
        self.metalWidth = self.metal2Width
        if self.bprFlag == BprMode.METAL1 or self.bprFlag == BprMode.METAL2:
            self.realTrack = self.numTrack + 2
            # self.realTrack = round(self.numTrack + 1.5, 1)
        elif self.bprFlag == BprMode.BPR:
            self.realTrack = self.numTrack + 1

        self.cellWidth = self.numCpp * self.cppWidth
        # self.cellHeight = int(self.realTrack * self.metal2Pitch - 0.5 * self.metal2Pitch)
        self.cellHeight = int(self.realTrack * self.metal2Pitch)

        if self.numTrack == 6:
            self.numFin = 3
        elif self.numTrack == 5:
            self.numFin = 3
        elif self.numTrack == 4:
            self.numFin = 2
        elif self.numTrack == 3:
            self.numFin = 1

        # only updates maxCellWidth when isMaxCellWidthUpdate is true
        if isMaxCellWidthUpdate:
            self.maxCellWidth = max(self.maxCellWidth, self.cellWidth)

    def getGridX(self, val):
        return val // self.m1GridIdxPitch

    def dump(self):
        print(
            "numTrack: %d, realTrack: %.1f, m1Pitch: %d nm, m2Pitch: %d nm, cppWidth: %d nm, siteName: %s"
            % (
                self.numTrack,
                self.realTrack,
                self.metal1Pitch,
                self.metal2Pitch,
                self.cppWidth,
                self.siteName,
            )
        )

    def getLefSiteStr(self):
        retStr = ""
        retStr += "SITE %s\n" % (self.siteName)
        retStr += "    SIZE %.4f BY %.4f ;\n" % (
            self.cppWidth / 1000.0,
            self.cellHeight / 1000.0,
        )
        retStr += "    CLASS CORE ;\n"
        retStr += "    SYMMETRY Y ;\n"
        retStr += "END %s\n\n" % (self.siteName)
        return retStr

    def getLx(self, val, metalWidthX):
        return ((val / self.cp_factor) * (self.cppWidth / 2.0) - metalWidthX / 2.0) / 1000.0

    # BPRMODE with METAL1 / METAL2 should shift coordinates by +metal2Pitch/2.0
    def getLy(self, val, metalWidthY, isMarginApply):
        calVal = (
            self.metal2Pitch + val * self.metal2Pitch - metalWidthY / 2.0
        ) / 1000.0


        # M1 should have margin
        if isMarginApply:
            calVal -= self.yMargin / 1000.0

        # additional shift for Samsung
        calVal += (self.metal2Pitch / 1000.0) / 4.0

        return calVal

    def getUx(self, val, metalWidthX):
        return ((val / self.cp_factor) * (self.cppWidth / 2.0) + metalWidthX / 2.0) / 1000.0

    # BPRMODE with METAL1 / METAL2 should shift coordinates by +metal2Pitch/2.0
    def getUy(self, val, metalWidthY, isMarginApply):
        calVal = (
            self.metal2Pitch + val * self.metal2Pitch + metalWidthY / 2.0
        ) / 1000.0

        # M1 should have margin
        if isMarginApply:
            calVal += self.yMargin / 1000.0

        # additional shift for S
        calVal += (self.metal2Pitch / 1000.0) / 4.0

        return calVal

    def getMpoStr(self):
        mpoStr = ""
        if self.mpoFlag == MpoMode.TWO:
            mpoStr = "2MPO"
        elif self.mpoFlag == MpoMode.THREE:
            mpoStr = "3MPO"
        elif self.mpoFlag == MpoMode.MAX:
            mpoStr = "MAXMPO"
        return mpoStr

    def getBprStr(self):
        bprStr = ""
        if self.bprFlag == BprMode.METAL1:
            bprStr = "M0"
        elif self.bprFlag == BprMode.METAL2:
            bprStr = "M2"
        elif self.bprFlag == BprMode.BPR:
            bprStr = "BPR"
        return bprStr

    def getDesignRuleStr(self):
        drStr = ""
        if self.drFlag == DrMode.EL:
            drStr = "EL"
        elif self.drFlag == DrMode.ET:
            drStr = "ET"
        elif self.drFlag == DrMode.SL:
            drStr = "SL"
        elif self.drFlag == DrMode.ST:
            drStr = "ST"
        return drStr

    def getCellName(self, origName):
        return "_".join(origName.split("_")[:2]) + "_" + self.getLibraryName()

    def getLibraryName(self):
        realTrackStr = str(self.realTrack)
        if self.realTrack != int(self.realTrack):
            realTrackStr = realTrackStr.replace(".", "p")
        return "%sT_%dF_%dCPP_%dM0P_%dM1P_%dM2P_%s_%s_%s" % (
            realTrackStr,
            self.numFin,
            self.cppWidth,
            self.metal0Pitch,
            self.metal1Pitch,
            self.metal2Pitch,
            self.getMpoStr(),
            self.getDesignRuleStr(),
            self.getBprStr(),
        )

    # for RPA calculation
    def getDInt(self):
        if self.drFlag == DrMode.EL or self.drFlag == DrMode.SL:
            return 3
        elif self.drFlag == DrMode.ET or self.drFlag == DrMode.ST:
            return 5
