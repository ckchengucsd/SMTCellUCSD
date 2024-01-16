# import os
# import sys
# import subprocess as sp
# from array import *
# from enum import Enum
# import numpy as np
# import math

# custom class
from techEntity import *

class Instance:
    def __init__(self, idx, lx, ly, numFinger, isFlip, totalWidth, unitWidth):
        self.idx = int(idx)
        self.lx = int(lx)
        self.ly = int(ly)
        self.numFinger = int(numFinger)
        self.isFlip = int(isFlip)
        self.totalWidth = int(totalWidth)
        self.unitWidth = int(unitWidth)

    def dump(self):
        print(
            "Instance idx: %d, (%d, %d), finger: %d, isfilp: %d, totalWidth: %d, unitWidth: %d"
            % (
                self.idx,
                self.lx,
                self.ly,
                self.numFinger,
                self.isFlip,
                self.totalWidth,
                self.unitWidth,
            )
        )

class Via:
    def __init__(self, fromMetal, toMetal, x, y, netID):
        self.fromMetal = int(fromMetal)
        self.toMetal = int(toMetal)
        self.x = int(x)
        self.y = int(y)
        self.netID = int(netID)

    def dump(self):
        print(
            "Via layer (%d -> %d), (%d, %d), netID: %d"
            % (self.fromMetal, self.toMetal, self.x, self.y, self.netID)
        )

    # Via should not have YMargin
    # Note that VIA must have "SQUARE" shape, so
    # metal pitch value <= min(m1Pitch, m2Pitch)
    def getLefStr(self, techInfo):
        # minPitchVal = min(techInfo.metal1Pitch, techInfo.metal2Pitch)
        # retStr = ""
        # retStr += "        RECT %.4f %.4f %.4f %.4f ;\n" \
        #    % (techInfo.getLx(self.y, minPitchVal/2.0),
        #        techInfo.getLy(abs(techInfo.numFin*2-1-self.x), minPitchVal/2.0, False), \
        #        techInfo.getUx(self.y, minPitchVal/2.0),
        #        techInfo.getUy(abs(techInfo.numFin*2-1-self.x), minPitchVal/2.0, False))
        # return retStr

        if techInfo.bprFlag == BprMode.METAL1 or techInfo.bprFlag == BprMode.METAL2:
            # 1.5T
            # yOffset = 1.25 * techInfo.metal2Pitch/1000.0
            # 2T
            yOffset = 1.5 * techInfo.metal2Pitch / 1000.0
        else:
            yOffset = techInfo.metal2Pitch / 1000.0

        # if self.fromMetal == 1 and self.toMetal == 2:
        #  viaWidth = min(gateWidth, techInfo.metal0Pitch/2/1000.0)
        #  viaHeight = min(gateWidth, techInfo.metal0Pitch/2/1000.0)
        if self.fromMetal == 2 and self.toMetal == 3:
            viaWidth = min(techInfo.metal0Width / 1000.0, techInfo.metal1Width / 1000.0)
            viaHeight = min(
                techInfo.metal0Width / 1000.0, techInfo.metal1Width / 1000.0
            )
        elif self.fromMetal == 3 and self.toMetal == 4:
            viaWidth = min(techInfo.metal1Width / 1000.0, techInfo.metal2Width / 1000.0)
            viaHeight = min(
                techInfo.metal1Width / 1000.0, techInfo.metal2Width / 1000.0
            )
        else:
            print("ERROR: Wrong via info!\n")

        if techInfo.numTrack == 4 or techInfo.numTrack == 6:
            yOffset_via = yOffset
        else:
            # RT=5
            yOffset_via = yOffset - techInfo.metal2Pitch / 1000.0

        gatePitch = round(int(techInfo.cppWidth * 10) / 10000.0, 4)
        xOffset = 0.000
        xCoordVia = (self.y * gatePitch / 2 / techInfo.cp_factor) + xOffset
        yCoordVia = (
            abs(techInfo.numFin * 2 - 1 - self.x) * techInfo.metal2Pitch / 1000.0
        ) + yOffset_via
        lxVia = xCoordVia - viaWidth / 2
        uxVia = xCoordVia + viaWidth / 2
        lyVia = yCoordVia - viaHeight / 2
        uyVia = yCoordVia + viaHeight / 2

        retStr = ""
        retStr += "        RECT %.4f %.4f %.4f %.4f ;\n" % (lxVia, lyVia, uxVia, uyVia)
        return retStr

    def getGdtStr(self, techInfo):
        minPitchVal = min(techInfo.metal1Pitch, techInfo.metal2Pitch)
        lx = techInfo.getLx(self.y, minPitchVal / 2.0)
        ly = techInfo.getLy(
            abs(techInfo.numFin * 2 - 1 - self.x), minPitchVal / 2.0, False
        )
        ux = techInfo.getUx(self.y, minPitchVal / 2.0)
        uy = techInfo.getUy(
            abs(techInfo.numFin * 2 - 1 - self.x), minPitchVal / 2.0, False
        )

        retStr = ""
        retStr += "xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)" % (
            lx,
            ly,
            ux,
            ly,
            ux,
            uy,
            lx,
            uy,
        )
        return retStr

class Metal:
    def __init__(self, layer, fromRow, fromCol, toRow, toCol, netID):
        self.layer = int(layer)
        self.fromRow = int(fromRow)
        self.fromCol = int(fromCol)
        self.toRow = int(toRow)
        self.toCol = int(toCol)
        self.netID = int(netID)

    def __lt__(self, other):
        if self.layer > other.layer:
            return True
        elif self.layer < other.layer:
            return False

        if self.netID > other.netID:
            return True
        elif self.netID < other.netID:
            return False

        if self.fromRow > other.fromRow:
            return True
        elif self.fromRow < other.fromRow:
            return False

        return self.fromCol > other.fromCol

    def dump(self):
        print(
            "Metal layer: %s, (%d, %d) - (%d, %d), netID: %d"
            % (
                self.getLayerName(),
                self.fromRow,
                self.fromCol,
                self.toRow,
                self.toCol,
                self.netID,
            )
        )

    def getLayerName(self):
        if self.layer == 1:
            return "Poly Layer"
        elif self.layer == 2:
            return "0"
        elif self.layer == 3:
            return "1"
        elif self.layer == 4:
            return "2"

    def getLefStr(self, techInfo, isMarginApply):
        gatePitch = round(int(techInfo.cppWidth * 10) / 10000.0, 4)
        xOffset = 0.000

        if techInfo.bprFlag == BprMode.BPR:
            yOffset = techInfo.metal2Pitch / 1000.0
        else:
            yOffset = 1.5 * techInfo.metal2Pitch / 1000.0
        if techInfo.numTrack == 4 or techInfo.numTrack == 6:
            yOffset_metal = yOffset
        else:
            # RT=5
            yOffset_metal = yOffset - techInfo.metal2Pitch / 1000.0

        xCoordMetalFrom = (self.fromCol * gatePitch / 2 / techInfo.cp_factor) + xOffset
        xCoordMetalTo = (self.toCol * gatePitch / 2 / techInfo.cp_factor) + xOffset
        yCoordMetalFrom = (
            abs(techInfo.numFin * 2 - 1 - self.fromRow) * techInfo.metal2Pitch / 1000.0
        ) + yOffset_metal
        yCoordMetalTo = (
            abs(techInfo.numFin * 2 - 1 - self.toRow) * techInfo.metal2Pitch / 1000.0
        ) + yOffset_metal

        if self.layer == 3:  # M1
            viaSize = min(techInfo.metal1Width / 1000.0, techInfo.metal2Width / 1000.0)
            viaEnc = techInfo.yMargin / 1000.0
        elif self.layer == 4:  # M2
            viaSize = techInfo.metal2Width / 1000.0
            viaEnc = techInfo.yMargin / 1000.0

        if self.layer == 4:  # M0/M2
            lxMetal = xCoordMetalFrom - viaSize / 2 - viaEnc
            uxMetal = xCoordMetalTo + viaSize / 2 + viaEnc
            lyMetal = yCoordMetalFrom - techInfo.metal2Width / 1000.0 / 2
            uyMetal = lyMetal + techInfo.metal2Width / 1000.0
        else:  # M1
            lxMetal = xCoordMetalFrom - techInfo.metal1Width / 1000.0 / 2
            uxMetal = lxMetal + techInfo.metal1Width / 1000.0
            lyMetal = yCoordMetalFrom + viaSize / 2 + viaEnc
            uyMetal = yCoordMetalTo - viaSize / 2 - viaEnc

        minPitchVal = min(techInfo.metal1Pitch, techInfo.metal2Pitch)
        retStr = ""
        retStr += "        RECT %.4f %.4f %.4f %.4f ;\n" % (
            lxMetal,
            lyMetal,
            uxMetal,
            uyMetal,
        )  # (techInfo.getLx(self.fromCol, techInfo.metal1Pitch/2.0),
        # techInfo.getLy(abs(techInfo.numFin*2-1-self.fromRow), minPitchVal/2.0, isMarginApply), \
        # techInfo.getUx(self.toCol, techInfo.metal1Pitch/2.0),
        # techInfo.getUy(abs(techInfo.numFin*2-1-self.toRow), minPitchVal/2.0, isMarginApply))
        return retStr

    def getGdtStr(self, techInfo, isMarginApply):
        # minPitchVal = min(techInfo.metal1Pitch, techInfo.metal2Pitch)
        # lx = techInfo.getLx(self.fromCol, techInfo.metal1Pitch/2.0)
        # ly = techInfo.getLy(abs(techInfo.numFin*2-1-self.fromRow), minPitchVal/2.0, isMarginApply)
        # ux = techInfo.getUx(self.toCol, techInfo.metal1Pitch/2.0)
        # uy = techInfo.getUy(abs(techInfo.numFin*2-1-self.toRow), minPitchVal/2.0, isMarginApply)

        #    if techInfo.bprFlag == BprMode.BPR:
        #      yOffset = 0.012
        #    else:
        #      yOffset = 0.000
        if techInfo.bprFlag == BprMode.BPR:
            yOffset = techInfo.metal2Pitch / 1000.0
        else:
            yOffset = 1.5 * techInfo.metal2Pitch / 1000.0

        if techInfo.numTrack == 4 or techInfo.numTrack == 6:
            yOffset_metal = yOffset
        else:
            # RT=5
            yOffset_metal = yOffset - techInfo.metal2Pitch / 1000.0

        # gateWidth = 0.016
        gateWidth = techInfo.gateWidth * techInfo.x_scale
        gatePitch = round(int(techInfo.cppWidth * 10) / 10000.0, 4)
        xOffset = 0.000
        xCoordMetalFrom = (self.fromCol * gatePitch / 2 / techInfo.cp_factor) + xOffset
        xCoordMetalTo = (self.toCol * gatePitch / 2 / techInfo.cp_factor) + xOffset
        yCoordMetalFrom = (
            abs(techInfo.numFin * 2 - 1 - self.fromRow) * techInfo.metal2Pitch / 1000.0
        ) + yOffset_metal
        yCoordMetalTo = (
            abs(techInfo.numFin * 2 - 1 - self.toRow) * techInfo.metal2Pitch / 1000.0
        ) + yOffset_metal

        if self.layer == 2:  # M0
            viaSize = min(gateWidth, techInfo.metal2Width / 1000.0)
            viaEnc = 0.003
        elif self.layer == 3:  # M1
            viaSize = min(techInfo.metal1Width / 1000.0, techInfo.metal2Width / 1000.0)
            viaEnc = techInfo.yMargin / 1000.0
        elif self.layer == 4:  # M2
            viaSize = techInfo.metal2Width / 1000.0
            viaEnc = techInfo.yMargin / 1000.0

        if self.layer == 2 or self.layer == 4:  # M0/M2
            lxMetal = xCoordMetalFrom - viaSize / 2 - viaEnc
            uxMetal = xCoordMetalTo + viaSize / 2 + viaEnc
            lyMetal = yCoordMetalFrom - techInfo.metal2Width / 1000.0 / 2
            uyMetal = lyMetal + techInfo.metal2Width / 1000.0
        else:  # M1
            lxMetal = xCoordMetalFrom - techInfo.metal1Width / 1000.0 / 2
            uxMetal = lxMetal + techInfo.metal1Width / 1000.0
            lyMetal = yCoordMetalFrom + viaSize / 2 + viaEnc
            uyMetal = yCoordMetalTo - viaSize / 2 - viaEnc

        # xText = (lx + ux) / 2
        # yText = (ly + uy) / 2
        xText = (lxMetal + uxMetal) / 2
        yText = (lyMetal + uyMetal) / 2

        retStr = ""
        # retStr += "xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)" \
        #    % (lx, ly, ux, ly, ux, uy, lx, uy)
        retStr += "xy(%.4f %.4f)" % (xText, yText)

        return retStr

class ExtPin:
    def __init__(self, layer, x, y, netID, pinName, isInput):
        self.layer = int(layer)
        self.x = int(x)
        self.y = int(y)
        self.netID = int(netID)
        self.pinName = pinName
        self.isInput = True if isInput.startswith("I") == True else False

    # for stable sort for all system
    # note that __lt__ is the same with PinInfo
    # to have the same idx after sorting
    def __lt__(self, other):
        if self.netID > other.netID:
            return True
        elif self.netID < other.netID:
            return False

        if self.pinName > other.pinName:
            return True
        elif self.pinName < other.pinName:
            return False

        return self.isInput

    def dump(self):
        print(
            "ExtPin layer: %d, (%d, %d) - ID: %d: %s, isInput: %d"
            % (self.layer, self.x, self.y, self.netID, self.pinName, self.isInput)
        )

class Pin:
    def __init__(self, pinName, row, col):
        self.pinName = pinName
        self.row = int(row)
        self.col = int(col)

    # for stable sort for all system
    # note that __lt__ is the same with PinInfo
    # to have the same idx after sorting
    def __lt__(self, other):
        if self.pinName > other.pinName:
            return True
        elif self.pinName < other.pinName:
            return False

    def dump(self):
        print("Pin: %s, (%d, %d)" % (self.pinName, self.row, self.col))

class PinInfo:
    def __init__(self, name, netID, via0s, metal1s, via1s, metal2s, isInput):
        self.name = name
        self.netID = int(netID)
        self.via0s = via0s
        self.metal1s = metal1s
        self.via1s = via1s
        self.metal2s = metal2s
        self.isInput = isInput

    # for stable sort for all system
    # note that __lt__ is the same with ExtPin
    # to have the same idx after sorting
    def __lt__(self, other):
        if self.netID > other.netID:
            return True
        elif self.netID < other.netID:
            return False

        if self.name > other.name:
            return True
        elif self.name < other.name:
            return False

        return self.isInput

    def dump(self):
        print("PinInfo: [%d]: %s" % (self.netID, self.name))
        for via0 in self.via0s:
            via0.dump()
        for metal1 in self.metal1s:
            metal1.dump()
        for via1 in self.via1s:
            via1.dump()
        for metal2 in self.metal2s:
            metal2.dump()
        print("")

    def getLefStr(self, techInfo):
        retStr = ""
        retStr += "  PIN %s\n" % (self.name)
        retStr += "    DIRECTION %s ;\n" % ("INPUT" if self.isInput else "OUTPUT")
        retStr += "    USE SIGNAL ;\n"
        retStr += "    PORT\n"

        # retStr += "      LAYER V0\n" if len(self.via0s) >= 1 else ""
        # for via0 in self.via0s:
        #  retStr += via0.getLefStr(techInfo)
        retStr += "      LAYER V1 ;\n" if len(self.via1s) >= 1 else ""
        for via1 in self.via1s:
            retStr += via1.getLefStr(techInfo)
        retStr += "      LAYER M1 ;\n" if len(self.metal1s) >= 1 else ""
        for m1 in self.metal1s:
            retStr += m1.getLefStr(techInfo, True)
        retStr += "      LAYER M2 ;\n" if len(self.metal2s) >= 1 else ""
        for m2 in self.metal2s:
            retStr += m2.getLefStr(techInfo, False)
        retStr += "    END\n"
        retStr += "  END %s\n" % (self.name)
        return retStr

    def getGdtStr(self, techInfo):
        retStr = ""
        # for via1 in self.via1s:
        #  retStr += "b{21 "
        #  retStr += via1.getGdtStr(techInfo)
        #  retStr += "}\n"

        for m1 in self.metal1s:
            # retStr += "b{19 "
            # M1 text layer
            retStr += "t{19 tt251 mc m0.05 "
            retStr += m1.getGdtStr(techInfo, True)
            retStr += " '%s'}\n" % (self.name)

        for m2 in self.metal2s:
            # retStr += "b{20 "
            # M2 text layer
            retStr += "t{20 tt251 mc m0.05 "
            retStr += m2.getGdtStr(techInfo, False)
            retStr += " '%s'}\n" % (self.name)
        return retStr

class ObsInfo:
    def __init__(self, via0s, metal1s, via1s, metal2s):
        self.via0s = via0s
        self.metal1s = metal1s
        self.via1s = via1s
        self.metal2s = metal2s

    def dump(self):
        print("ObsInfo:")
        for via0 in self.via0s:
            via0.dump()
        for metal1 in self.metal1s:
            metal1.dump()
        for via1 in self.via1s:
            via1.dump()
        for metal2 in self.metal2s:
            metal2.dump()
        print("")

    def getLefStr(self, techInfo):
        if len(self.via1s) + len(self.metal1s) + len(self.metal2s) == 0:
            return ""

        retStr = ""
        retStr += "  OBS\n"
        retStr += "      LAYER V1 ;\n" if len(self.via1s) >= 1 else ""
        for via1 in self.via1s:
            retStr += via1.getLefStr(techInfo)
        retStr += "      LAYER M1 ;\n" if len(self.metal1s) >= 1 else ""
        for m1 in self.metal1s:
            retStr += m1.getLefStr(techInfo, True)
        retStr += "      LAYER M2 ;\n" if len(self.metal2s) >= 1 else ""
        for m2 in self.metal2s:
            retStr += m2.getLefStr(techInfo, False)
        retStr += "  END\n"
        return retStr

    def getGdtStr(self, techInfo):
        retStr = ""
        for via1 in self.via1s:
            retStr += "b{21 "
            retStr += via1.getGdtStr(techInfo)
            retStr += "}\n"

        for m1 in self.metal1s:
            retStr += "b{19 "
            retStr += m1.getGdtStr(techInfo, True)
            retStr += "}\n"

        for m2 in self.metal2s:
            retStr += "b{20 "
            retStr += m2.getGdtStr(techInfo, False)
            retStr += "}\n"
        return retStr

