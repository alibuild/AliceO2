// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file FCTSegment.cxx
/// \brief Implementation of the FCTSegment class
/// \author Mario Sitta <sitta@to.infn.it>
/// \author Chinorat Kobdaj (kobdaj@g.sut.ac.th)

#include "FCTSimulation/FCTSegment.h"
#include "FCTBase/GeometryTGeo.h"
#include "FCTSimulation/Detector.h"

#include <fairlogger/Logger.h> // for LOG

#include <TGeoManager.h> // for TGeoManager, gGeoManager
#include <TGeoMatrix.h>  // for TGeoCombiTrans, TGeoRotation, etc
#include <TGeoMedium.h>
#include <TGeoArb8.h>           // for TGeoTrap
#include <TGeoVolume.h>         // for TGeoVolume, TGeoVolumeAssembly
#include <TGeoCompositeShape.h> // for TGeoCompositeShape
#include <TGeoMatrix.h>         // for TGeoCombiTrans
#include "TMathBase.h"          // for Abs
#include <TMath.h>              // for Sin, RadToDeg, DegToRad, Cos, Tan, etc

#include <cstdio> // for snprintf

using namespace TMath;
using namespace o2::fct;
using namespace o2::itsmft;

ClassImp(FCTSegment);

FCTSegment::~FCTSegment() = default;

FCTSegment::FCTSegment(Int_t layerNumber, Int_t sectorNumber, Int_t moduleNumber, std::string segmentName, Double_t x, Double_t y, Double_t z, Double_t vertL, Double_t innerL, Double_t outerL, Double_t rotX, Double_t rotY, Double_t rotZ, Float_t segmentx2X0) : mLayerNumber(layerNumber), mSectorNumber(sectorNumber), mModuleNumber(moduleNumber), mSegmentName(segmentName), mx2X0(segmentx2X0), mX(x), mY(y), mZ(z), mRotX(rotX), mRotY(rotY), mRotZ(rotZ), mVertL(vertL), mInnerL(innerL), mOuterL(outerL)
{

  Float_t Si_X0 = 9.37; // In cm
  mChipThickness = mx2X0 * Si_X0;
  LOG(info) << "Creating FCT segment: Layer " << mLayerNumber << " Sector: " << mSectorNumber << " Module: " << mModuleNumber;
  LOG(info) << "   Using silicon X0 = " << Si_X0 << " to emulate segment radiation length.";
  LOG(info) << "   Segment z = " << mZ << " ; vertL = " << mVertL << " ; innerL = " << mInnerL << " ; outerL = " << mOuterL << " ; x2X0 = " << mx2X0 << " ; ChipThickness = " << mChipThickness;
}

void FCTSegment::createSegment(TGeoVolume* motherVolume)
{
  if (mLayerNumber < 0 || mSectorNumber < 0 || mModuleNumber < 0) {
    return;
  }
  // Create tube, set sensitive volume, add to mother volume

  std::string chipName = Form("%s_Lay_%d_Sec_%d_Mod_%d", o2::fct::GeometryTGeo::getFCTChipPattern(), mLayerNumber, mSectorNumber, mModuleNumber);
  std::string sensName = Form("%s_Lay_%d_Sec_%d_Mod_%d", GeometryTGeo::getFCTSensorPattern(), mLayerNumber, mSectorNumber, mModuleNumber);
  TGeoTrap* sensor = new TGeoTrap(mChipThickness / 2., 0., 0.,
                                  mVertL / 2., mInnerL / 2., mOuterL / 2., 0.,
                                  mVertL / 2., mInnerL / 2., mOuterL / 2., 0.);
  TGeoTrap* chip = new TGeoTrap(mChipThickness / 2., 0., 0.,
                                mVertL / 2., mInnerL / 2., mOuterL / 2., 0.,
                                mVertL / 2., mInnerL / 2., mOuterL / 2., 0.);
  TGeoTrap* segment = new TGeoTrap(mChipThickness / 2., 0., 0.,
                                   mVertL / 2., mInnerL / 2., mOuterL / 2., 0.,
                                   mVertL / 2., mInnerL / 2., mOuterL / 2., 0.);

  TGeoMedium* medSi = gGeoManager->GetMedium("FCT_SILICON$");
  TGeoMedium* medAir = gGeoManager->GetMedium("FCT_AIR$");

  TGeoVolume* sensVol = new TGeoVolume(sensName.c_str(), sensor, medSi);
  sensVol->SetLineColor(kSpring + 5);
  TGeoVolume* chipVol = new TGeoVolume(chipName.c_str(), chip, medSi);
  chipVol->SetLineColor(kSpring + 5);
  TGeoVolume* segmentVol = new TGeoVolume(mSegmentName.c_str(), segment, medAir);
  segmentVol->SetLineColor(kSpring + 5);

  LOG(info) << "Inserting " << sensVol->GetName() << " inside " << chipVol->GetName();
  chipVol->AddNode(sensVol, 1, nullptr);

  LOG(info) << "Inserting " << chipVol->GetName() << " inside " << segmentVol->GetName();
  segmentVol->AddNode(chipVol, 1, nullptr);

  // Finally put everything in the mother volume
  auto FwdSegmentRotation = new TGeoRotation("FwdSegmentRotation", mRotX, mRotY, mRotZ);
  auto FwdSegmentCombiTrans = new TGeoCombiTrans(mX, mY, mZ, FwdSegmentRotation);

  LOG(info) << "Inserting " << segmentVol->GetName() << " inside " << motherVolume->GetName();
  motherVolume->AddNode(segmentVol, 1, FwdSegmentCombiTrans);
}