import os
import sys
import subprocess as sp
from array import *
from enum import Enum
# import numpy as np
import random
import math

# custom class
from cellEntity import *
from techEntity import *

class Utility(object):
	@staticmethod
	def GetVddVssPinLefStr(techInfo):
		if techInfo.bprFlag == BprMode.NONE:
			return ""

		lefStr = ""

		vddPrefix = ""
		vddPrefix += "  PIN VDD\n"
		vddPrefix += "    DIRECTION INOUT ;\n"
		vddPrefix += "    USE POWER ;\n"
		vddPrefix += "    SHAPE ABUTMENT ;\n"
		vddPrefix += "    PORT\n"

		vssPrefix = ""
		vssPrefix += "  PIN VSS\n"
		vssPrefix += "    DIRECTION INOUT ;\n"
		vssPrefix += "    USE GROUND ;\n"
		vssPrefix += "    SHAPE ABUTMENT ;\n"
		vssPrefix += "    PORT\n"

		# Different Metal Width with given BPR mode.
		rectWidth = 0
		if techInfo.bprFlag == BprMode.METAL1 or techInfo.bprFlag == BprMode.METAL2:
			# rectWidth = float(techInfo.metalWidth)*1.5
			rectWidth = float(techInfo.metal0Pitch / 2.0) * 1.5
		elif techInfo.bprFlag == BprMode.BPR:
			rectWidth = techInfo.metalWidth / 2.0

		vddRectStr = "        RECT %.4f %.4f %.4f %.4f ;\n" % (
			0.0,
			(techInfo.cellHeight - rectWidth) / 1000.0,
			techInfo.cellWidth / 1000.0,
			(techInfo.cellHeight + rectWidth) / 1000.0,
		)

		vssRectStr = "        RECT %.4f %.4f %.4f %.4f ;\n" % (
			0.0,
			(-rectWidth) / 1000.0,
			techInfo.cellWidth / 1000.0,
			(rectWidth) / 1000.0,
		)

		# METAL1 and BPR have M1
		if techInfo.bprFlag == BprMode.METAL1 or techInfo.bprFlag == BprMode.BPR:
			lefStr += vddPrefix
			lefStr += "      LAYER M0 ;\n"
			lefStr += vddRectStr
			lefStr += "    END\n"
			lefStr += "  END VDD\n"
			lefStr += vssPrefix
			lefStr += "      LAYER M0 ;\n"
			lefStr += vssRectStr
			lefStr += "    END\n"
			lefStr += "  END VSS\n"
		# METAL2 have M2
		elif techInfo.bprFlag == BprMode.METAL2:
			lefStr += vddPrefix
			lefStr += "      LAYER M1 ;\n"
			lefStr += vddRectStr
			lefStr += "      LAYER M2 ;\n"
			lefStr += vddRectStr
			lefStr += "    END\n"
			lefStr += "  END VDD\n"
			lefStr += vssPrefix
			lefStr += "      LAYER M1 ;\n"
			lefStr += vssRectStr
			lefStr += "      LAYER M2 ;\n"
			lefStr += vssRectStr
			lefStr += "    END\n"
			lefStr += "  END VSS\n"
		return lefStr

	@staticmethod
	def GetVddVssPinGdtStr(techInfo):
		if techInfo.bprFlag == BprMode.NONE:
			return ""

		gdtStr = ""
		# Different Metal Width with given BPR mode.
		rectWidth = 0
		if techInfo.bprFlag == BprMode.METAL1 or techInfo.bprFlag == BprMode.METAL2:
			# rectWidth = techInfo.metalWidth
			# rectWidth = float(techInfo.metalWidth)*1.5
			rectWidth = float(techInfo.metal0Pitch / 2.0) * 1.5
		elif techInfo.bprFlag == BprMode.BPR:
			rectWidth = techInfo.metalWidth / 2.0

		lx = 0.0
		ly = (techInfo.cellHeight - rectWidth) / 1000.0
		ux = techInfo.cellWidth / 1000.0 * techInfo.x_scale
		uy = (techInfo.cellHeight + rectWidth) / 1000.0 * techInfo.y_scale

		vddRectStr = "xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)" % (
			lx,
			ly,
			ux,
			ly,
			ux,
			uy,
			lx,
			uy,
		)

		# Vdd text (M0)
		x = (lx + ux) / 2
		y = (ly + uy) / 2
		vddTextStr = "xy(%.4f %.4f)" % (x, y)

		lx = 0.0
		ly = (-rectWidth) / 1000.0
		ux = techInfo.cellWidth / 1000.0 * techInfo.x_scale
		uy = (rectWidth) / 1000.0 * techInfo.y_scale

		vssRectStr = "xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)" % (
			lx,
			ly,
			ux,
			ly,
			ux,
			uy,
			lx,
			uy,
		)

		# Vss text (M0)
		x = (lx + ux) / 2
		y = (ly + uy) / 2
		vssTextStr = "xy(%.4f %.4f)" % (x, y)

		## METAL1 and BPR have M1
		# if techInfo.bprFlag == BprMode.METAL1 or techInfo.bprFlag == BprMode.BPR:
		#  gdtStr += "b{19 "
		#  gdtStr += vddRectStr
		#  gdtStr += "}\n"
		#  gdtStr += "b{19 "
		#  gdtStr += vssRectStr
		#  gdtStr += "}\n"

		# PGpin == M0 (layernum = 15)
		if techInfo.bprFlag == BprMode.METAL1 or techInfo.bprFlag == BprMode.BPR:
			gdtStr += "b{15 "
			gdtStr += vddRectStr
			gdtStr += "}\n"
			gdtStr += "b{15 "
			gdtStr += vssRectStr
			gdtStr += "}\n"

			# text layer
			gdtStr += "t{15 tt251 mc m0.05 "
			gdtStr += vddTextStr
			gdtStr += " 'VDD'}\n"
			gdtStr += "t{15 tt251 mc m0.05 "
			gdtStr += vssTextStr
			gdtStr += " 'VSS'}\n"

			# layer 1.251 and 3.251
			gdtStr += "t{1 tt251 mc m0.05 "
			gdtStr += vddTextStr
			gdtStr += " 'VDD'}\n"
			gdtStr += "t{3 tt251 mc m0.05 "
			gdtStr += vssTextStr
			gdtStr += " 'VSS'}\n"

		# METAL2 have M2
		elif techInfo.bprFlag == BprMode.METAL2:
			gdtStr += "b{19 "
			gdtStr += vddRectStr
			gdtStr += "}\n"
			gdtStr += "b{20 "
			gdtStr += vddRectStr
			gdtStr += "}\n"
			gdtStr += "b{19 "
			gdtStr += vssRectStr
			gdtStr += "}\n"
			gdtStr += "b{20 "
			gdtStr += vssRectStr
			gdtStr += "}\n"

		return gdtStr

	@staticmethod
	def GenerateLef(inputDir, inputFileList, outputDir, techInfo):
		######### RPA gen and choose best pin
		rpaStr = ""

		# bestMetalPinInfo extraction is required at this moment
		bestMetalPinInfo = {}

		for curFile in inputFileList:
			if curFile.endswith(".conv") is False:
				continue
			f = open(inputDir + curFile, "r")
			cont = f.read()
			f.close()

			print(inputDir + curFile)
			# print(cont)
			if cont.startswith("COST 0 0"):
				continue
			
			curRpaStr, curMacroPinInfo = Utility.GetMacroRPAStr(cont, curFile[:-5], techInfo)
			rpaStr += curRpaStr
			bestMetalPinInfo.update(curMacroPinInfo)

		ExecuteCommand("mkdir -p %s/%s" % (outputDir, techInfo.getLibraryName()))
		f = open(
			outputDir
			+ "/"
			+ techInfo.getLibraryName()
			+ "/"
			+ techInfo.getLibraryName()
			+ ".rpa",
			"w",
		)
		f.write(rpaStr)
		f.close()

		########## Original LEF gen
		lefStr = "VERSION 5.8 ;\n"
		lefStr += 'BUSBITCHARS "[]" ;\n'
		lefStr += 'DIVIDERCHAR "/" ;\n'
		lefStr += "CLEARANCEMEASURE EUCLIDEAN ;\n\n"

		lefStr += techInfo.getLefSiteStr()

		########## Original GDF gen
		gdtStr = "gds2{5\n"
		gdtStr += "m=2021-04-02 20:32:00 a=2020-04-02 22:32:00\n"
		gdtStr += "lib 'probe_fake_gds' 0.00025 2.5e-10\n"

		for curFile in inputFileList:
			if curFile.endswith(".conv") is False:
				continue
			f = open(inputDir + curFile, "r")
			cont = f.read()
			f.close()

			lefStr += Utility.GetMacroLefStr(cont, curFile[:-5], techInfo, False, bestMetalPinInfo)

			gdtStr += Utility.GetMacroGdtStr(cont, curFile[:-5], techInfo, False, bestMetalPinInfo)

		lefStr += "END LIBRARY\n"

		gdtStr += "}\n"

		f = open(
			outputDir
			+ "/"
			+ techInfo.getLibraryName()
			+ "/"
			+ techInfo.getLibraryName()
			+ ".lef",
			"w",
		)
		f.write(lefStr)
		f.close()

		f = open(
			outputDir
			+ "/"
			+ techInfo.getLibraryName()
			+ "/"
			+ techInfo.getLibraryName()
			+ ".gdt",
			"w",
		)
		f.write(gdtStr)
		f.close()

		########## info file gen
		for curFile in inputFileList:
			if curFile.endswith(".conv") is False:
				continue
			f = open(inputDir + curFile, "r")
			cont = f.read()
			f.close()

			infoStr = Utility.GetMacroInfoStr(cont, curFile[:-5], outputDir, techInfo)

			cellName = techInfo.getCellName(curFile[:-5])

			f = open(
				outputDir + "/" + techInfo.getLibraryName() + "/" + cellName + ".info", "w"
			)
			f.write(infoStr)
			f.close()

		########## Padded LEF gen
		lefStr = "VERSION 5.8 ;\n"
		lefStr += 'BUSBITCHARS "[]" ;\n'
		lefStr += 'DIVIDERCHAR "/" ;\n'
		lefStr += "CLEARANCEMEASURE EUCLIDEAN ;\n\n"

		lefStr += techInfo.getLefSiteStr()

		gdtStr = "gds2{5\n"
		gdtStr += "m=2021-04-02 20:32:00 a=2020-04-02 22:32:00\n"
		gdtStr += "lib 'probe_fake_gds_padded' 0.00025 2.5e-10\n"

		for curFile in inputFileList:
			if curFile.endswith(".conv") is False:
				continue
			f = open(inputDir + curFile, "r")
			cont = f.read()
			f.close()

			lefStr += Utility.GetMacroLefStr(cont, curFile[:-5], techInfo, True, bestMetalPinInfo)

			gdtStr += Utility.GetMacroGdtStr(cont, curFile[:-5], techInfo, True, bestMetalPinInfo)

		lefStr += "END LIBRARY\n"

		gdtStr += "}\n"

		f = open(
			outputDir
			+ "/"
			+ techInfo.getLibraryName()
			+ "/"
			+ techInfo.getLibraryName()
			+ ".bloat.lef",
			"w",
		)
		f.write(lefStr)
		f.close()

		f = open(
			outputDir
			+ "/"
			+ techInfo.getLibraryName()
			+ "/"
			+ techInfo.getLibraryName()
			+ ".bloat.gdt",
			"w",
		)
		f.write(gdtStr)
		f.close()

	@staticmethod
	def GetMacroLefStr(conv, cellName, techInfo, isUseMaxCellWidth, macroPinInfo):
		insts = []
		metals = []
		vias = []
		extpins = []

		for curLine in conv.split("\n"):
			words = curLine.split(" ")
			if words[0] == "COST":
				# techInfo.numCpp = int(words[1])//120 + 1
				# techInfo.numCpp = int(words[1])//120 original
				techInfo.numCpp = int(int(words[1]) // (2 * techInfo.cp_factor))
			elif words[0] == "TRACK":
				techInfo.numTrack = int(words[2])
			elif words[0] == "INST":
				insts.append(
					Instance(
						words[1], words[2], words[3], words[4], words[5], words[6], words[7]
					)
				)
			elif words[0] == "METAL":
				metals.append(
					Metal(words[1], words[2], words[3], words[4], words[5], words[6])
				)
			elif words[0] == "VIA":
				vias.append(Via(words[1], words[2], words[3], words[4], words[5]))
			elif words[0] == "EXTPIN":
				extpins.append(
					ExtPin(words[2], words[3], words[4], words[1], words[5], words[6])
				)

		# Prevent maxCellWidth if
		# cellName has "DFFHQ"
		isMaxCellWidthUpdate = not ("DFFHQ" in cellName)
		techInfo.update(isMaxCellWidthUpdate)

		cellName = techInfo.getCellName(cellName)
		cellWidth = (
			techInfo.maxCellWidth
			if isUseMaxCellWidth and isMaxCellWidthUpdate
			else techInfo.cellWidth
		) * techInfo.x_scale

		# print("CellName: ", cellName, "cellWidth: ", cellWidth)

		pinInfos = []
		pinNetId = set()

		via1PinObs = []
		metal1PinObs = []

		for pIdx, extpin in enumerate(sorted(extpins)):
			pinNetId.add(extpin.netID)

			via0Arr = [
				via for via in vias if via.netID == extpin.netID and via.fromMetal == 2
			]
			metal1Arr = [
				metal
				for metal in metals
				if metal.netID == extpin.netID and metal.layer == 3
			]
			via1Arr = [
				via for via in vias if via.netID == extpin.netID and via.fromMetal == 3
			]
			metal2Arr = [
				metal
				for metal in metals
				if metal.netID == extpin.netID and metal.layer == 4
			]

			# pin extension handling
			if techInfo.pinExtVal > 0:
				for m1 in metal1Arr:
					# note that grid will exist as 0 to numTrack - 1 indices.
					m1.fromRow = max(0, m1.fromRow - techInfo.pinExtVal)
					m1.toRow = min(techInfo.numTrack - 1, m1.toRow + techInfo.pinExtVal)

			# if there exists metal2 pin
			# M1/V1 should be in OBS
			if len(metal2Arr) >= 1:
				via1PinObs += via1Arr
				metal1PinObs += metal1Arr

				via1Arr = []
				metal1Arr = []
				# print("pIdx:", pIdx, extpin.pinName, cellName, "has metal2")
			# pin choice using RPA.
			# only happen when top metal is 1.
			elif cellName in macroPinInfo:
				for metalList in macroPinInfo.get(cellName):

					# print( "pIdx:", pIdx, extpin.pinName, cellName, "has metalList:", metalList)

					# when pinIdx is matched
					if metalList[0] == pIdx:
						# print( 'pIdx matched' )
						mIdx = metalList[1]

						newMetal1Arr = []
						for idx, metal1 in enumerate(sorted(metal1Arr)):
							# print(cellName, extpin.pinName, "MIdx:", idx, end=' ')
							# metal1.dump()
							if idx == mIdx:
								newMetal1Arr.append(metal1)
							else:
								metal1PinObs.append(metal1)
						# print("updated obs:")
						# for obs in metal1PinObs:
						#  obs.dump()

						# reset metal1Arr
						metal1Arr = newMetal1Arr

						break

			pinInfos.append(
				PinInfo(
					extpin.pinName,
					extpin.netID,
					via0Arr,
					metal1Arr,
					via1Arr,
					metal2Arr,
					extpin.isInput,
				)
			)

		# OBS handling
		via0Obs = [via for via in vias if via.netID not in pinNetId and via.fromMetal == 2]
		metal1Obs = [
			metal for metal in metals if metal.netID not in pinNetId and metal.layer == 3
		]
		via1Obs = [via for via in vias if via.netID not in pinNetId and via.fromMetal == 3]
		metal2Obs = [
			metal for metal in metals if metal.netID not in pinNetId and metal.layer == 4
		]

		# recover Pin OBS
		metal1Obs += metal1PinObs
		via1Obs += via1PinObs

		obsInfo = ObsInfo(via0Obs, metal1Obs, via1Obs, metal2Obs)

		lefStr = ""
		lefStr += "MACRO %s\n" % (cellName)
		lefStr += "  CLASS CORE ;\n"
		lefStr += "  ORIGIN 0 0 ;\n"
		lefStr += "  FOREIGN %s 0 0 ;\n" % (cellName)
		lefStr += "  SIZE %.4f BY %.4f ;\n" % (
			cellWidth / 1000.0,
			techInfo.cellHeight / 1000.0,
		)
		lefStr += "  SYMMETRY X Y ;\n"
		lefStr += "  SITE %s ;\n" % (techInfo.siteName)

		for pinInfo in sorted(pinInfos):
			lefStr += pinInfo.getLefStr(techInfo)
		lefStr += Utility.GetVddVssPinLefStr(techInfo)
		lefStr += obsInfo.getLefStr(techInfo)
		lefStr += "END %s\n\n" % (cellName)

		return lefStr

	@staticmethod
	def GetMacroGdtStr(conv, cellName, techInfo, isUseMaxCellWidth, macroPinInfo):
		insts = []
		metals = []
		vias = []
		extpins = []
		pins = []

		for curLine in conv.split("\n"):
			words = curLine.split(" ")
			if words[0] == "COST":
				# techInfo.numCpp = int(words[1])//120 + 1
				# techInfo.numCpp = int(words[1])//120 original
				techInfo.numCpp = int(int(words[1]) // (2 * techInfo.cp_factor))
			elif words[0] == "TRACK":
				techInfo.numTrack = int(words[2])
			elif words[0] == "INST":
				insts.append(
					Instance(
						words[1], words[2], words[3], words[4], words[5], words[6], words[7]
					)
				)
			elif words[0] == "METAL":
				metals.append(
					Metal(words[1], words[2], words[3], words[4], words[5], words[6])
				)
			elif words[0] == "VIA":
				vias.append(Via(words[1], words[2], words[3], words[4], words[5]))
			elif words[0] == "EXTPIN":
				extpins.append(
					ExtPin(words[2], words[3], words[4], words[1], words[5], words[6])
				)
			elif words[0] == "PIN":
				pins.append(Pin(words[1], words[2], words[3]))

		# Prevent maxCellWidth if
		# cellName has "DFFHQ"
		isMaxCellWidthUpdate = not ("DFFHQ" in cellName)
		techInfo.update(isMaxCellWidthUpdate)

		cellName = techInfo.getCellName(cellName)
		cellWidth = (
			techInfo.maxCellWidth
			if isUseMaxCellWidth and isMaxCellWidthUpdate
			else techInfo.cellWidth
		)

		# print("CellName: ", cellName, "cellWidth: ", cellWidth)

		pinInfos = []
		pinNetId = set()

		via1PinObs = []
		metal1PinObs = []

		for pIdx, extpin in enumerate(sorted(extpins)):
			pinNetId.add(extpin.netID)

			via0Arr = [
				via for via in vias if via.netID == extpin.netID and via.fromMetal == 2
			]
			metal1Arr = [
				metal
				for metal in metals
				if metal.netID == extpin.netID and metal.layer == 3
			]
			via1Arr = [
				via for via in vias if via.netID == extpin.netID and via.fromMetal == 3
			]
			metal2Arr = [
				metal
				for metal in metals
				if metal.netID == extpin.netID and metal.layer == 4
			]

			# pin extension handling
			if techInfo.pinExtVal > 0:
				for m1 in metal1Arr:
					# note that grid will exist as 0 to numTrack - 1 indices.
					m1.fromRow = max(0, m1.fromRow - techInfo.pinExtVal)
					m1.toRow = min(techInfo.numTrack - 1, m1.toRow + techInfo.pinExtVal)

			# if there exists metal2 pin
			# M1/V1 should be in OBS
			if len(metal2Arr) >= 1:
				via1PinObs += via1Arr
				metal1PinObs += metal1Arr

				via1Arr = []
				metal1Arr = []
				# print("pIdx:", pIdx, extpin.pinName, cellName, "has metal2")
			# pin choice using RPA.
			# only happen when top metal is 1.
			elif cellName in macroPinInfo:
				for metalList in macroPinInfo.get(cellName):

					# print( "pIdx:", pIdx, extpin.pinName, cellName, "has metalList:", metalList)

					# when pinIdx is matched
					if metalList[0] == pIdx:
						# print( 'pIdx matched' )
						mIdx = metalList[1]

						newMetal1Arr = []
						for idx, metal1 in enumerate(sorted(metal1Arr)):
							# print(cellName, extpin.pinName, "MIdx:", idx, end=' ')
							# metal1.dump()
							if idx == mIdx:
								newMetal1Arr.append(metal1)
							else:
								metal1PinObs.append(metal1)
						# print("updated obs:")
						# for obs in metal1PinObs:
						#  obs.dump()

						# reset metal1Arr
						metal1Arr = newMetal1Arr

						break

			pinInfos.append(
				PinInfo(
					extpin.pinName,
					extpin.netID,
					via0Arr,
					metal1Arr,
					via1Arr,
					metal2Arr,
					extpin.isInput,
				)
			)

		# OBS handling
		via0Obs = [via for via in vias if via.netID not in pinNetId and via.fromMetal == 2]
		metal1Obs = [
			metal for metal in metals if metal.netID not in pinNetId and metal.layer == 3
		]
		via1Obs = [via for via in vias if via.netID not in pinNetId and via.fromMetal == 3]
		metal2Obs = [
			metal for metal in metals if metal.netID not in pinNetId and metal.layer == 4
		]

		# recover Pin OBS
		metal1Obs += metal1PinObs
		via1Obs += via1PinObs

		obsInfo = ObsInfo(via0Obs, metal1Obs, via1Obs, metal2Obs)

		lx = ly = 0
		ux = cellWidth / 1000.0 * techInfo.x_scale
		uy = techInfo.cellHeight / 1000.0 * techInfo.y_scale

		gdtStr = "cell{c=2021-04-02 20:00:00 m=2021-04-02 20:00:00 '%s'\n" % (cellName)

		# cellWidth/Height
		gdtStr += "b{100 xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
			lx,
			ly,
			ux,
			ly,
			ux,
			uy,
			lx,
			uy,
		)

		# MSK
		# NWell (layernum = 1)
		lyNw = uy / 2
		gdtStr += "b{1 xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
			lx,
			lyNw,
			ux,
			lyNw,
			ux,
			uy,
			lx,
			uy,
		)

		# NSELECT (layernum = 12)
		uyNs = uy / 2
		gdtStr += "b{12 xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
			lx,
			ly,
			ux,
			ly,
			ux,
			uyNs,
			lx,
			uyNs,
		)
		# PSELECT (layernum = 13)
		gdtStr += "b{13 xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
			lx,
			lyNw,
			ux,
			lyNw,
			ux,
			uy,
			lx,
			uy,
		)

		
		# Fin (layernum = 2)
		# finWidth = 0.006
		finWidth = techInfo.finWidth
		totalFinNum = (techInfo.numFin * 2) + 3
		# finPitch = 0.024
		finPitch = techInfo.finPitch

		if techInfo.bprFlag == BprMode.BPR:
			yOffset = finPitch / 2
			yRange = range(
				0,
				int(techInfo.cellHeight * 10.0) - int(yOffset * 10000.0),
				int(finPitch * 10000.0),
			)
		else:
			yOffset = 0.000
			yRange = range(
				0,
				int(techInfo.cellHeight * 10.0) - int(yOffset * 10000.0) + 1,
				int(finPitch * 10000.0),
			)

		for i in yRange:
			if i == yRange[0] or i == yRange[-1]:
				continue
			lxFin = 0
			uxFin = cellWidth / 1000.0
			lyFin = (i / 10000.0) - (finWidth / 2) + yOffset
			uyFin = (i / 10000.0) + (finWidth / 2) + yOffset
			gdtStr += "b{2 xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
				lxFin,
				lyFin,
				uxFin * techInfo.x_scale,
				lyFin,
				uxFin * techInfo.x_scale,
				uyFin * techInfo.y_scale,
				lxFin,
				uyFin * techInfo.y_scale,
			)
		

		# # NanoSheet for P-channel
		# middle_idx = int(len(yRange) / 2) - 1

		# lxFin = 0
		# uxFin = cellWidth / 1000.0
		# lyFin = (yRange[1]/ 10000.0) - (finWidth / 2) + yOffset
		# uyFin = (yRange[middle_idx] / 10000.0) + (finWidth / 2) + yOffset
		# gdtStr += "b{23 xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
		# 	lxFin,
		# 	lyFin,
		# 	uxFin * techInfo.x_scale,
		# 	lyFin,
		# 	uxFin * techInfo.x_scale,
		# 	uyFin * techInfo.y_scale,
		# 	lxFin,
		# 	uyFin * techInfo.y_scale,
		# )

		# # NanoSheet for N-channel
		# lxFin = 0
		# uxFin = cellWidth / 1000.0
		# lyFin = (yRange[middle_idx + 1]) / 10000.0 - (finWidth / 2) + yOffset
		# uyFin = (yRange[-2] / 10000.0) + (finWidth / 2) + yOffset
		# gdtStr += "b{24 xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
		# 	lxFin,
		# 	lyFin,
		# 	uxFin * techInfo.x_scale,
		# 	lyFin,
		# 	uxFin * techInfo.x_scale,
		# 	uyFin * techInfo.y_scale,
		# 	lxFin,
		# 	uyFin * techInfo.y_scale,
		# )
		

		# Gate (layernum = 7)
		# gateWidth = 0.016 #HP
		gateWidth = techInfo.gateWidth * techInfo.x_scale
		gatePitch = round(int(techInfo.cppWidth * 10) / 10000.0, 4) * techInfo.x_scale
		xOffset = 0.000

		for i in range(
			0, int(techInfo.cellWidth * techInfo.x_scale) - int(xOffset * 1000.0) + 1, int(gatePitch * 1000.0)
		):
			lxGate = (i / 1000.0) - (gateWidth / 2) + xOffset
			uxGate = (i / 1000.0) + (gateWidth / 2) + xOffset
			lyGate = -0.005
			uyGate = techInfo.cellHeight / 1000.0 + 0.005
			gdtStr += "b{7 xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
				lxGate,
				lyGate,
				uxGate,
				lyGate,
				uxGate,
				uyGate,
				lxGate,
				uyGate,
			)

		# CA (layernum = 14) for VDD/VSS
		caWidth = min(gateWidth, techInfo.metal0Width / 1000.0)
		for i in range(
			int(xOffset * 1000.0),
			int(techInfo.cellWidth) - int(xOffset * 1000.0) + 1,
			int(gatePitch * 1000.0),
		):
			lxCa = (i / 1000.0) - (caWidth / 2) + xOffset
			uxCa = (i / 1000.0) + (caWidth / 2) + xOffset
			lyCa = 0 - caWidth / 2
			uyCa = 0 + caWidth / 2
			gdtStr += "b{14 xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
				lxCa,
				lyCa,
				uxCa,
				lyCa,
				uxCa,
				uyCa,
				lxCa,
				uyCa,
			)
			lyCa = (techInfo.cellHeight / 1000.0) - (caWidth / 2)
			uyCa = (techInfo.cellHeight / 1000.0) + (caWidth / 2)
			gdtStr += "b{14 xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
				lxCa,
				lyCa,
				uxCa,
				lyCa,
				uxCa,
				uyCa,
				lxCa,
				uyCa,
			)

		# prep for GateCut
		gateCutX = [-1]
		for x in range(1, techInfo.numCpp * 2, 2):
			gateCutX.append(x)
		# print(gateCutX)

		# Active (layernum = 11)
		for inst in insts:
			# xCoordActive = round(((1/2+(inst.lx/120.0))*techInfo.cppWidth)/1000.0,4)
			xCoordActive = round(
				((1 / 2 + (inst.lx / (techInfo.cp_factor * 2))) * techInfo.cppWidth) / 1000.0, 4
			)

			activeHeight = 0.000
			if inst.unitWidth == 1:
				activeHeight = 0.023
			elif inst.unitWidth == 2:
				activeHeight = 0.046
			elif inst.unitWidth == 3:
				# activeHeight = 0.069
				activeHeight = 0.060

			activeWidth = round(gatePitch * (inst.totalWidth - 1) / 2, 4) * techInfo.x_scale
			activeHeight *= techInfo.y_scale

			# For GateCut position
			for i in range(
				int(inst.lx / techInfo.cp_factor),
				int((inst.lx / techInfo.cp_factor) + (inst.totalWidth - 1)),
				2,
			):
				if i in gateCutX:
					gateCutX.remove(i)

			if techInfo.bprFlag == BprMode.BPR:
				yOffset_Active = techInfo.metal0Pitch / 4 / 1000.0 + finPitch / 2 - 0.001
			else:
				yOffset_Active = techInfo.metal0Pitch / 4 / 1000.0 - 0.001

			lxActive = xCoordActive - gatePitch / 2 - 0.007
			uxActive = xCoordActive + (activeWidth - gatePitch / 2) + 0.007
			if (inst.ly / techInfo.numFin) < 1:  # PMOS
				# lyActive = finPitch - (activeHeight-finPitch)/2 + 0.007
				# lyActive = finPitch - (activeHeight-finPitch)/2 + yOffset_Active
				lyActive = finPitch - yOffset_Active
				uyActive = lyActive + activeHeight
			else:  # NMOS
				# uyActive = techInfo.cellHeight/1000.0 - (finPitch - (activeHeight-finPitch)/2 + 0.007)
				# uyActive = techInfo.cellHeight/1000.0 - (finPitch - (activeHeight-finPitch)/2 + yOffset_Active)
				uyActive = techInfo.cellHeight / 1000.0 - (finPitch - yOffset_Active)
				lyActive = uyActive - activeHeight

			gdtStr += "b{11 xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
				lxActive,
				lyActive,
				uxActive,
				lyActive,
				uxActive,
				uyActive,
				lxActive,
				uyActive,
			)

		# LISD (CA, Source-Drain interconnect layer, layernum = 17)
		# lisdWidth = gateWidth + 0.002
		lisdWidth = gateWidth

		# Collect columns for non-contact to VDD/VSS
		pinCols_PMOS = []
		pinCols_NMOS = []
		for pin in sorted(pins):
			if pin.row >= techInfo.numFin:  # PMOS
				pinCols_PMOS.append(int(pin.col / techInfo.cp_factor))
			else:  # NMOS
				pinCols_NMOS.append(int(pin.col / techInfo.cp_factor))

		# print("pinCols: ", pinCols)

		# Collect columns for lisd, number of fin (int), isPgContact, (isExtCol)
		colListPMOS = []
		colListNMOS = []
		for inst in insts:
			xInst = int(inst.lx / techInfo.cp_factor)

			# print(inst.numFinger)

			for i in range(xInst, xInst + inst.totalWidth, 2):
				if int(inst.ly) < techInfo.numFin and i in pinCols_PMOS:
					isPgContact = 0
				elif int(inst.ly) >= techInfo.numFin and i in pinCols_NMOS:
					isPgContact = 0
				else:
					isPgContact = 1

				# isExtCol
				isExtCol = 0
				if (inst.ly / techInfo.numFin) < 1:  # NMOS
					for via in vias:
						if via.fromMetal == 1 and int(via.y / techInfo.cp_factor) == int(i):
							# print(via.x)
							if (
								via.x >= techInfo.numFin
								and via.x <= (2 * techInfo.numFin) - 1 - inst.unitWidth
							):
								# print("!!!")
								isExtCol = 1
				else:  # PMOS
					for via in vias:
						if via.fromMetal == 1 and int(via.y / techInfo.cp_factor) == int(i):
							# print(via.x)
							if via.x < techInfo.numFin and via.x >= inst.unitWidth:
								# print("!!!")
								isExtCol = 1

				if (inst.ly / techInfo.numFin) >= 1:  # PMOS
					colListPMOS.append([i, inst.unitWidth, isPgContact, isExtCol])
				else:  # NMOS
					colListNMOS.append([i, inst.unitWidth, isPgContact, isExtCol])

		colListNMOS = sorted(colListNMOS)
		colListPMOS = sorted(colListPMOS)

		prev = [0, 0, 0, 0]
		sortedColListNMOS = []
		cnt = 0
		for i in range(0, len(colListNMOS)):
			col = colListNMOS[i]
			if col[0] == prev[0]:
				if col[1] > prev[1]:
					sortedColListNMOS[cnt - 1] = col
					prev = col
				else:
					prev = col
					continue
			else:
				sortedColListNMOS.append(col)
				cnt += 1
			prev = col

		prev = [0, 0, 0, 0]
		sortedColListPMOS = []
		cnt = 0
		for i in range(0, len(colListPMOS)):
			col = colListPMOS[i]
			if col[0] == prev[0]:
				if col[1] > prev[1]:
					sortedColListPMOS[cnt - 1] = col
					prev = col
				else:
					prev = col
					continue
			else:
				sortedColListPMOS.append(col)
				cnt += 1
			prev = col

		sortedColListNMOS = sorted(sortedColListNMOS)
		sortedColListPMOS = sorted(sortedColListPMOS)
		# print("PMOS: ", sortedColListPMOS)
		# print("NMOS: ", sortedColListNMOS)

		# Draw LISD
		for col in sortedColListNMOS:
			xCoordLisd = xOffset + col[0] * gatePitch / 2
			if col[2]:
				lyLisd = 0
				if techInfo.numTrack == 4 or techInfo.numTrack == 6:
					if techInfo.bprFlag == BprMode.BPR:
						uyLisd = lyLisd + finPitch * (col[1] + col[2] + col[3] - 0.75)
					else:
						uyLisd = lyLisd + finPitch * (col[1] + col[2] + col[3] - 0.25)
				else:
					# RT == 5
					if techInfo.bprFlag == BprMode.BPR:
						uyLisd = lyLisd + finPitch * (col[1] + col[2] + col[3] - 1.25)
					else:
						uyLisd = lyLisd + finPitch * (col[1] + col[2] + col[3] - 0.75)

			else:
				# lyLisd = finPitch
				if techInfo.bprFlag == BprMode.BPR:
					lyLisd = finPitch / 2 - 0.004
				else:
					lyLisd = finPitch - 0.004

				if techInfo.numTrack == 4 or techInfo.numTrack == 6:
					uyLisd = lyLisd + finPitch * (col[1] + col[3])
				else:
					uyLisd = lyLisd + finPitch * (col[1] + col[3] - 0.5)
			# uyLisd = lyLisd + finPitch*(col[1]+col[2]+col[3]+0.25)
			lxLisd = xCoordLisd - lisdWidth / 2
			uxLisd = xCoordLisd + lisdWidth / 2

			gdtStr += "b{17 xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
				lxLisd,
				lyLisd,
				uxLisd,
				lyLisd,
				uxLisd,
				uyLisd,
				lxLisd,
				uyLisd,
			)

		for col in sortedColListPMOS:
			xCoordLisd = xOffset + col[0] * gatePitch / 2
			if col[2]:
				uyLisd = uy
				if techInfo.numTrack == 4 or techInfo.numTrack == 6:
					if techInfo.bprFlag == BprMode.BPR:
						lyLisd = uyLisd - finPitch * (col[1] + col[2] + col[3] - 0.75)
					else:
						lyLisd = uyLisd - finPitch * (col[1] + col[2] + col[3] - 0.25)
				else:
					if techInfo.bprFlag == BprMode.BPR:
						lyLisd = uyLisd - finPitch * (col[1] + col[2] + col[3] - 1.25)
					else:
						lyLisd = uyLisd - finPitch * (col[1] + col[2] + col[3] - 0.75)

			else:
				# uyLisd = uy - finPitch
				if techInfo.bprFlag == BprMode.BPR:
					uyLisd = uy - finPitch / 2 + 0.004
				else:
					uyLisd = uy - finPitch + 0.004

				if techInfo.numTrack == 4 or techInfo.numTrack == 6:
					lyLisd = uyLisd - finPitch * (col[1] + col[3])
				else:
					lyLisd = uyLisd - finPitch * (col[1] + col[3] - 0.5)
			# lyLisd = uyLisd - finPitch*(col[1]+col[2]+col[3]+0.25)
			lxLisd = xCoordLisd - lisdWidth / 2
			uxLisd = xCoordLisd + lisdWidth / 2

			gdtStr += "b{17 xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
				lxLisd,
				lyLisd,
				uxLisd,
				lyLisd,
				uxLisd,
				uyLisd,
				lxLisd,
				uyLisd,
			)

		# SDT (layernum == 88)
		for col in sortedColListNMOS:
			xCoordLisd = xOffset + col[0] * gatePitch / 2

			if techInfo.numTrack == 4 or techInfo.numTrack == 6:
				if techInfo.bprFlag == BprMode.BPR:
					lyLisd = finPitch / 2
				else:
					lyLisd = finPitch
				uyLisd = lyLisd + finPitch * (col[1] - 0.25)
			else:
				# RT=5
				if techInfo.bprFlag == BprMode.BPR:
					lyLisd = finPitch / 2
				else:
					lyLisd = finPitch
				uyLisd = lyLisd + finPitch * (col[1] - 0.75)

			lxLisd = xCoordLisd - lisdWidth / 2
			uxLisd = xCoordLisd + lisdWidth / 2
			gdtStr += "b{88 xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
				lxLisd,
				lyLisd,
				uxLisd,
				lyLisd,
				uxLisd,
				uyLisd,
				lxLisd,
				uyLisd,
			)

		for col in sortedColListPMOS:
			xCoordLisd = xOffset + col[0] * gatePitch / 2

			if techInfo.numTrack == 4 or techInfo.numTrack == 6:
				if techInfo.bprFlag == BprMode.BPR:
					uyLisd = uy - finPitch / 2
				else:
					uyLisd = uy - finPitch
				lyLisd = uyLisd - finPitch * (col[1] - 0.25)
			else:
				if techInfo.bprFlag == BprMode.BPR:
					uyLisd = uy - finPitch / 2
				else:
					uyLisd = uy - finPitch
				lyLisd = uyLisd - finPitch * (col[1] - 0.75)

			lxLisd = xCoordLisd - lisdWidth / 2
			uxLisd = xCoordLisd + lisdWidth / 2
			gdtStr += "b{88 xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
				lxLisd,
				lyLisd,
				uxLisd,
				lyLisd,
				uxLisd,
				uyLisd,
				lxLisd,
				uyLisd,
			)

		# LIG (Gate interconnect layer, layernum = 16)
		# V0 (layernum = 18), V1 (layernum = 21)
		if techInfo.bprFlag == BprMode.METAL1 or techInfo.bprFlag == BprMode.METAL2:
			# 1.5T
			# yOffset = 1.25 * techInfo.metal2Pitch/1000.0
			# 2T
			yOffset = 1.5 * techInfo.metal2Pitch / 1000.0
		else:
			yOffset = techInfo.metal2Pitch / 1000.0

		for via in vias:
			if via.fromMetal == 1 and via.toMetal == 2:
				layerNum = 16
				viaWidth = min(gateWidth, techInfo.metal0Width / 1000.0)
				viaHeight = min(gateWidth, techInfo.metal0Width / 1000.0)
			elif via.fromMetal == 2 and via.toMetal == 3:
				layerNum = 18
				viaWidth = min(techInfo.metal0Width / 1000.0, techInfo.metal1Width / 1000.0)
				viaHeight = min(
					techInfo.metal0Width / 1000.0, techInfo.metal1Width / 1000.0
				)
			elif via.fromMetal == 3 and via.toMetal == 4:
				layerNum = 21
				viaWidth = min(techInfo.metal1Width / 1000.0, techInfo.metal2Width / 1000.0)
				viaHeight = min(
					techInfo.metal1Width / 1000.0, techInfo.metal2Width / 1000.0
				)
			else:
				print("ERROR: Wrong via info!\n")

			if techInfo.numTrack == 4 or techInfo.numTrack == 6:
				yOffset_via = yOffset
			elif techInfo.numTrack == 5:
				# RT=5
				yOffset_via = yOffset - techInfo.metal2Pitch / 1000.0

			# MSK TODO: fix x and y for via class
			# class definition is wrong
			xCoordVia = (via.y * gatePitch / 2 / techInfo.cp_factor) + xOffset
			yCoordVia = (
				abs(techInfo.numFin * 2 - 1 - via.x) * techInfo.metal2Pitch / 1000.0
			) + yOffset_via

			# For GateCut position
			if via.fromMetal == 1 and via.toMetal == 2:
				# print ("VIAY:", int(via.y/60.0))
				if int(via.y / techInfo.cp_factor) - 1 in gateCutX:
					gateCutX.remove(int(via.y / techInfo.cp_factor) - 1)
					# print ("VIAY:", int(via.y/60.0))

			lxVia = xCoordVia - viaWidth / 2
			uxVia = xCoordVia + viaWidth / 2
			lyVia = yCoordVia - viaHeight / 2
			uyVia = yCoordVia + viaHeight / 2

			if via.fromMetal == 1 and via.toMetal == 2:
				if (via.y / techInfo.cp_factor) % 2 == 0:
					gdtStr += "b{14 xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
						lxVia,
						lyVia,
						uxVia,
						lyVia,
						uxVia,
						uyVia,
						lxVia,
						uyVia,
					)
					# print(f"VIA was drawn: {lxVia} {lyVia} {uxVia} {uyVia}")
					gdtStr += "b{16 xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
						lxVia,
						lyVia,
						uxVia,
						lyVia,
						uxVia,
						uyVia,
						lxVia,
						uyVia,
					)
				else:
					gdtStr += "b{14 xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
						lxVia,
						lyVia,
						uxVia,
						lyVia,
						uxVia,
						uyVia,
						lxVia,
						uyVia,
					)
			else:
				gdtStr += "b{%d xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
					layerNum,
					lxVia,
					lyVia,
					uxVia,
					lyVia,
					uxVia,
					uyVia,
					lxVia,
					uyVia,
				)

		# LIG for VSS/VDD
		lxVia = lx
		uxVia = ux
		lyVia = 0 - techInfo.metal0Width / 1000.0 / 2
		uyVia = 0 + techInfo.metal0Width / 1000.0 / 2

		gdtStr += "b{16 xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
			lxVia,
			lyVia,
			uxVia,
			lyVia,
			uxVia,
			uyVia,
			lxVia,
			uyVia,
		)

		lyVia = techInfo.cellHeight / 1000.0 - techInfo.metal0Width / 1000.0 / 2
		uyVia = techInfo.cellHeight / 1000.0 + techInfo.metal0Width / 1000.0 / 2

		gdtStr += "b{16 xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
			lxVia,
			lyVia,
			uxVia,
			lyVia,
			uxVia,
			uyVia,
			lxVia,
			uyVia,
		)

		# Diffusion (layernum = 10)
		for x_ind in range(len(gateCutX)):
			# use diffusion in place of Gate Cut
			x = int(gateCutX[x_ind] / 2 + 1) * int(gatePitch * 1000.0)
			lxGate = (x / 1000.0) - (gateWidth / 2) + xOffset
			uxGate = (x / 1000.0) + (gateWidth / 2) + xOffset
			lyGate = -0.005
			# CFET FLAG
			uyGate = techInfo.cellHeight / 1000.0 + 0.005
			gdtStr += "b{10 xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
				lxGate,
				lyGate,
				uxGate,
				lyGate,
				uxGate,
				uyGate,
				lxGate,
				uyGate,
			)


		# VSS GateCut
		if techInfo.bprFlag == BprMode.BPR:
			gcWidth = techInfo.metal0Width / 1000.0 + 0.001
		else:
			gcWidth = 1.5 * techInfo.metal0Pitch / 1000.0 + 0.001

		lyGC = ly - gcWidth / 2
		uyGC = ly + gcWidth / 2
		lxGC = lx
		uxGC = ux
		gdtStr += "b{10 xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
			lxGC,
			lyGC,
			uxGC,
			lyGC,
			uxGC,
			uyGC,
			lxGC,
			uyGC,
		)

		# VDD GateCut
		# lyGC = uy - finPitch + 0.002
		# lyGC = uy - finPitch + 0.005
		lyGC = uy - gcWidth / 2
		# uyGC = uy + finPitch - 0.002
		# uyGC = uy + finPitch - 0.005
		uyGC = uy + gcWidth / 2
		gdtStr += "b{10 xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
			lxGC,
			lyGC,
			uxGC,
			lyGC,
			uxGC,
			uyGC,
			lxGC,
			uyGC,
		)

		# M0 (layernum = 15), M1 (layernum = 19), M2 (layernum = 20)
		for metal in metals:
			if metal.layer == 1:
				continue
			elif metal.layer == 2:  # M0
				viaSize = min(gateWidth, techInfo.metal2Width / 1000.0)
				# viaEnc = techInfo.yMargin/1000.0
				viaEnc = 0.003
				layerNum = 15
			elif metal.layer == 3:  # M1
				viaSize = min(techInfo.metal1Width / 1000.0, techInfo.metal2Width / 1000.0)
				viaEnc = techInfo.yMargin / 1000.0
				layerNum = 19
			elif metal.layer == 4:  # M2
				viaSize = techInfo.metal2Width / 1000.0
				viaEnc = techInfo.yMargin / 1000.0
				layerNum = 20

			if techInfo.numTrack == 4 or techInfo.numTrack == 6:
				yOffset_metal = yOffset
			elif techInfo.numTrack == 5:
				# RT=5
				yOffset_metal = yOffset - techInfo.metal2Pitch / 1000.0

			xCoordMetalFrom = (metal.fromCol * gatePitch / 2 / techInfo.cp_factor) + xOffset
			xCoordMetalTo = (metal.toCol * gatePitch / 2 / techInfo.cp_factor) + xOffset
			yCoordMetalFrom = (
				abs(techInfo.numFin * 2 - 1 - metal.fromRow) * techInfo.metal2Pitch / 1000.0
			) + yOffset_metal
			yCoordMetalTo = (
				abs(techInfo.numFin * 2 - 1 - metal.toRow) * techInfo.metal2Pitch / 1000.0
			) + yOffset_metal

			if metal.layer == 2 or metal.layer == 4:  # M0/M2
				lxMetal = xCoordMetalFrom - viaSize / 2 - viaEnc
				uxMetal = xCoordMetalTo + viaSize / 2 + viaEnc
				lyMetal = yCoordMetalFrom - techInfo.metal2Width / 1000.0 / 2
				uyMetal = lyMetal + techInfo.metal2Width / 1000.0
			else:  # M1
				lxMetal = xCoordMetalFrom - techInfo.metal1Width / 1000.0 / 2
				uxMetal = lxMetal + techInfo.metal1Width / 1000.0
				lyMetal = yCoordMetalFrom + viaSize / 2 + viaEnc
				uyMetal = yCoordMetalTo - viaSize / 2 - viaEnc

			gdtStr += "b{%d xy(%.4f %.4f %.4f %.4f %.4f %.4f %.4f %.4f)}\n" % (
				layerNum,
				lxMetal,
				lyMetal,
				uxMetal,
				lyMetal,
				uxMetal,
				uyMetal,
				lxMetal,
				uyMetal,
			)

		for pinInfo in sorted(pinInfos):
			gdtStr += pinInfo.getGdtStr(techInfo)

		gdtStr += Utility.GetVddVssPinGdtStr(techInfo)
		# gdtStr += obsInfo.getGdtStr(techInfo)
		gdtStr += "}\n"

		return gdtStr

	@staticmethod
	def GetMpoFlag(inpStr):
		if inpStr == "2":
			return MpoMode.TWO
		elif inpStr == "3":
			return MpoMode.THREE
		elif inpStr == "MAX":
			return MpoMode.MAX
		return MpoMode.NONE
		
	@staticmethod
	def GetDrFlag(inpStr):
		if inpStr == "EL":
			return DrMode.EL
		elif inpStr == "ET":
			return DrMode.ET
		elif inpStr == "SL":
			return DrMode.SL
		elif inpStr == "ST":
			return DrMode.ST
		else:
			return DrMode.NONE
	
	@staticmethod
	def GetRpaMode(inpStr):
		if inpStr == "random":
			return RpaMode.RANDOM
		elif inpStr == "best":
			return RpaMode.BEST
		elif inpStr == "worst":
			return RpaMode.WORST
		else:
			return RpaMode.NONE

	###########################################################
	# RPA functions
	@staticmethod
	def PrintGrid(grid, gridX, gridY):
		for j in range(gridY):
			for i in range(gridX):
				print('%5d' % (grid[j][i]), end=' ')
				print()
	
	# given dint, get neightbor's pin netIdx
	@staticmethod
	def GetNeightbors(grid, j, i, dint, gridX, obsId):
		idxSet = set()
		for x in range( -dint + i, i+ dint+1):
			if x < 0 or x >= gridX:
				continue

			# escape myself
			if x == i:
				continue

			if grid[j][x] != 0 and grid[j][x] != obsId:
				idxSet.add( grid[j][x] )

		return idxSet

	# netId cannot exceed 1000
	@staticmethod
	def GetHashId(netId, metalId):
		return 1000 * (netId+1) + metalId

	@staticmethod
	def GetPairFromHash(val):
		return (val // 1000)-1, val % 1000

	@staticmethod
	def GetMacroRPAStr(conv, cellName, techInfo):
		global numTotalMacros
		global numMultiMetal1Macros
		global numMultiMetal1MacrosDFF

		insts = []
		metals = []
		vias = []
		extpins = []
		pins = []

		gridX = 0
		gridY = 0

		for curLine in conv.split("\n"):
			words = curLine.split(" ")
			if words[0] == "COST":
				# techInfo.numCpp = int(words[1])//120 + 1
				# techInfo.numCpp = int(words[1])//120 origin
				techInfo.numCpp = int(int(words[1]) // (techInfo.cp_factor * 2))
				gridX = int(words[1])
			elif words[0] == "TRACK":
				techInfo.numTrack = int(words[2])
				gridY = int(words[2])
			elif words[0] == "INST":
				insts.append(
					Instance(
						words[1], words[2], words[3], words[4], words[5], words[6], words[7]
					)
				)
			elif words[0] == "METAL":
				metals.append(
					Metal(words[1], words[2], words[3], words[4], words[5], words[6])
				)
			elif words[0] == "VIA":
				vias.append(Via(words[1], words[2], words[3], words[4], words[5]))
			elif words[0] == "EXTPIN":
				extpins.append(
					ExtPin(words[2], words[3], words[4], words[1], words[5], words[6])
				)
			elif words[0] == "PIN":
				pins.append(Pin(words[1], words[2], words[3]))

		techInfo.update(False)

		# divide X grid coordi
		gridX = int(math.ceil(gridX / techInfo.m1GridIdxPitch))

		cellName = techInfo.getCellName(cellName)
		# print( gridX, gridY )

		pinInfos = []
		pinNetId = set()

		for extpin in sorted(extpins):
			pinNetId.add(extpin.netID)

			via0Arr = [
				via for via in vias if via.netID == extpin.netID and via.fromMetal == 2
			]
			metal1Arr = [
				metal
				for metal in metals
				if metal.netID == extpin.netID and metal.layer == 3
			]
			via1Arr = [
				via for via in vias if via.netID == extpin.netID and via.fromMetal == 3
			]
			metal2Arr = [
				metal
				for metal in metals
				if metal.netID == extpin.netID and metal.layer == 4
			]

			# if there exists metal2 pin
			# reset all pin info
			if len(metal2Arr) >= 1:
				via0Arr = []
				metal1Arr = []
				via1Arr = []
				metal2Arr = []

			# note that metal2 pins must be in the pinInfos!!
			# because of comforming index with MacroLef's pinInfos
			pinInfos.append(
				PinInfo(
					extpin.pinName,
					extpin.netID,
					via0Arr,
					metal1Arr,
					via1Arr,
					metal2Arr,
					extpin.isInput,
				)
			)

		# OBS handling
		metal1Obs = [
			metal for metal in metals if metal.netID not in pinNetId and metal.layer == 3
		]

		# obsID
		obsId = 999

		# Generate Grid structure
		grid = [[0] * gridX for i in range(gridY)]

		# obs update
		for metal in metal1Obs:
			for j in range(metal.fromCol, metal.toCol + 1, techInfo.m1GridIdxPitch):
				newJ = techInfo.getGridX(j)
				for i in range(metal.fromRow, metal.toRow + 1):
					# here i/j shifted
					grid[i][newJ] = obsId

		# netIdtoApCnt
		netIdToNumAp = dict()

		# m1 metal update
		for pinInfo in sorted(pinInfos):

			# for each metal pin access points --> (i, j)
			for mIdx, metal in enumerate(sorted(pinInfo.metal1s)):

				# netIdToNumAp update
				if metal.fromRow == metal.toRow:
					addVal = metal.toCol - metal.fromCol + 1

					# add in hashmap
					if Utility.GetHashId(pinInfo.netID, mIdx) in netIdToNumAp:
						netIdToNumAp[Utility.GetHashId(pinInfo.netID, mIdx)] += addVal
					else:
						netIdToNumAp[Utility.GetHashId(pinInfo.netID, mIdx)] = addVal

				elif metal.fromCol == metal.toCol:
					addVal = metal.toRow - metal.fromRow + 1

					# add in hashmap
					if Utility.GetHashId(pinInfo.netID, mIdx) in netIdToNumAp:
						netIdToNumAp[Utility.GetHashId(pinInfo.netID, mIdx)] += addVal
					else:
						netIdToNumAp[Utility.GetHashId(pinInfo.netID, mIdx)] = addVal

				# update grid
				for j in range(metal.fromCol, metal.toCol + 1, techInfo.m1GridIdxPitch):
					newJ = techInfo.getGridX(j)
					for i in range(metal.fromRow, metal.toRow + 1):
						# here i/j shifted
						grid[i][newJ] = Utility.GetHashId(pinInfo.netID, mIdx)

		retStr = "MACRO %s\n" % (cellName)

		# iocVal value init
		iocVal = 0

		bestMetalPinInfo = []

		rpaStor = {}

		# foreach pins in cell
		isMultiMetal1Cell = False
		for pIdx, pinInfo in enumerate(sorted(pinInfos)):

			# metal pin candidates
			bestMetalRpa = -9999 if techInfo.rpaMode == RpaMode.BEST else 9999
			bestMetalIdx = -1

			if len(pinInfo.metal1s) == 0:
				continue

			# for each metal segment in each pin
			for mIdx, metal in enumerate(sorted(pinInfo.metal1s)):

				# upa/rpa should be calculated in each metal segment!!!
				upaVal = 0

				# for each metal access points in each metal segment
				for j in range(metal.fromCol, metal.toCol + 1, techInfo.m1GridIdxPitch):
					newJ = techInfo.getGridX(j)
					for i in range(metal.fromRow, metal.toRow + 1):

						# get neightbors in each metal access points
						# here i/j shifted.
						neightbors = Utility.GetNeightbors(
							grid, i, newJ, techInfo.getDInt(), gridX, obsId
						)
						for neightbor in neightbors:
							# extract neightbor's netID and metalIdx
							nNetId, nMIdx = Utility.GetPairFromHash(neightbor)
							upaVal += 1.0 / netIdToNumAp[Utility.GetHashId(nNetId, nMIdx)]

				rpaVal = netIdToNumAp[Utility.GetHashId(pinInfo.netID, mIdx)] - upaVal
				rpaStor[Utility.GetHashId(pinInfo.netID, mIdx)] = rpaVal

				# get bestMetalRpa
				if techInfo.rpaMode == RpaMode.BEST:
					if bestMetalRpa < rpaVal:
						bestMetalRpa = rpaVal
						bestMetalIdx = mIdx
				elif techInfo.rpaMode == RpaMode.WORST:
					if bestMetalRpa > rpaVal:
						bestMetalRpa = rpaVal
						bestMetalIdx = mIdx

				# print(pinInfo.name, "RPA",  mIdx, rpaVal)

			# update best metal pin Info
			if len(pinInfo.metal1s) > 1:
				isMultiMetal1Cell = True
				if techInfo.rpaMode == RpaMode.WORST or techInfo.rpaMode == RpaMode.BEST:
					bestMetalPinInfo.append([pIdx, bestMetalIdx, bestMetalRpa])
				elif techInfo.rpaMode == RpaMode.RANDOM:
					# randMetalIdx = np.random.choice(len(pinInfo.metal1s))
					# use python random instead of numpy
					randMetalIdx = random.randint(0, len(pinInfo.metal1s) - 1)
					# print( "choose randMetalIdx: ", randMetalIdx )
					bestMetalPinInfo.append([pIdx, randMetalIdx, -999])
				else:
					print(" Wrong RPA MODE. Please put 0-2 values")
					exit(1)

			iocVal += min(bestMetalRpa - 1.0, 0)

			retStr += "  PIN %s\n" % (pinInfo.name)
			retStr += "    RPA %.04f ;\n" % (bestMetalRpa)
			retStr += "  END PIN\n"
		retStr += "  IOC %.04f ;\n" % (iocVal)
		retStr += "END MACRO\n\n"

		# for pidx, midx, rpa in bestMetalPinInfo:
		#  print( "Best Solution: ", pidx, midx, rpa )

		numTotalMacros += 1
		if isMultiMetal1Cell:
			numMultiMetal1Macros += 1

		if "DFF" in cellName:
			numMultiMetal1MacrosDFF += 1

		##### Print RPA report!!
		# if iocVal < 0:
		#   print("CellName:", cellName)
		#   PrintGrid(grid, gridX, gridY)
		#   print("ApCnt: ",  netIdToNumAp )
		#   print("DInt: ", techInfo.getDInt())

		#   print("[RPA List]")

		#   for pinInfo in sorted(pinInfos):
		#     # for each metal segment in each pin
		#     for mIdx, metal in enumerate(sorted(pinInfo.metal1s)):
		#       print(pinInfo.name, GetHashId(pinInfo.netID, mIdx), rpaStor[GetHashId(pinInfo.netID, mIdx)])

		#   print("IOC:", iocVal)
		#   print()
		######

		# cell name --> cell pin candidates.
		retPinInfo = {}
		retPinInfo[cellName] = bestMetalPinInfo

		return retStr, retPinInfo

	@staticmethod
	def GetMacroInfoStr(conv, cellName, outputDir, techInfo):
		insts = []
		metals = []
		vias = []
		extpins = []

		for curLine in conv.split("\n"):
			words = curLine.split(" ")
			if words[0] == "COST":
				# techInfo.numCpp = int(words[1])//120 + 1
				# techInfo.numCpp = int(words[1])//120 original
				techInfo.numCpp = int(words[1]) // (2 * techInfo.cp_factor)
			elif words[0] == "TRACK":
				techInfo.numTrack = int(words[2])
			elif words[0] == "INST":
				insts.append(
					Instance(
						words[1], words[2], words[3], words[4], words[5], words[6], words[7]
					)
				)
			elif words[0] == "METAL":
				metals.append(
					Metal(words[1], words[2], words[3], words[4], words[5], words[6])
				)
			elif words[0] == "VIA":
				vias.append(Via(words[1], words[2], words[3], words[4], words[5]))
			elif words[0] == "EXTPIN":
				extpins.append(
					ExtPin(words[2], words[3], words[4], words[1], words[5], words[6])
				)

		# Prevent maxCellWidth if
		# cellName has "DFFHQ"

		# isMaxCellWidthUpdate = (not ("DFFHQ" in cellName))
		# techInfo.update( isMaxCellWidthUpdate )
		techInfo.update(False)
		# techInfo.dump()

		# for metal in metals:
		#  metal.dump()

		# for via in vias:
		#  via.dump()

		# for extpin in extpins:
		#  extpin.dump()

		pinInfos = []
		pinNetId = set()
		for extpin in sorted(extpins):
			pinNetId.add(extpin.netID)
			via0Arr = [
				via for via in vias if via.netID == extpin.netID and via.fromMetal == 2
			]
			metal1Arr = [
				metal
				for metal in metals
				if metal.netID == extpin.netID and metal.layer == 3
			]
			via1Arr = [
				via for via in vias if via.netID == extpin.netID and via.fromMetal == 3
			]
			metal2Arr = [
				metal
				for metal in metals
				if metal.netID == extpin.netID and metal.layer == 4
			]

			pinInfos.append(
				PinInfo(
					extpin.pinName,
					extpin.netID,
					via0Arr,
					metal1Arr,
					via1Arr,
					metal2Arr,
					extpin.isInput,
				)
			)

		# for pinInfo in sorted(pinInfos):
		#  pinInfo.dump()

		cellName = techInfo.getCellName(cellName)
		cellWidth = techInfo.cellWidth

		infoStr = ""
		infoStr += "%s " % (cellName)

		for pinInfo in sorted(pinInfos):
			if pinInfo.isInput == False:
				infoStr += "%s " % (pinInfo.name)

		for pinInfo in sorted(pinInfos):
			if pinInfo.isInput == True:
				infoStr += "%s " % (pinInfo.name)

		infoStr += "\n"
		infoStr += "%g %g\n" % (techInfo.cellWidth / 1000.0, techInfo.cellHeight / 1000.0)
		return infoStr

