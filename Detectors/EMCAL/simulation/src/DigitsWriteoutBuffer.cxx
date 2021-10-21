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

#include <unordered_map>
#include <vector>
#include <list>
#include <deque>
#include <iostream>
#include <gsl/span>
#include "EMCALSimulation/LabeledDigit.h"
#include "EMCALSimulation/DigitsWriteoutBuffer.h"

using namespace o2::emcal;

DigitsWriteoutBuffer::DigitsWriteoutBuffer(unsigned int nTimeBins, unsigned int binWidth) : mBufferSize(nTimeBins),
                                                                                            mTimeBinWidth(binWidth)
{
  //mTimedDigits.resize(nTimeBins);
  for(int itime = 0; itime < nTimeBins;itime++) mTimedDigits.push_back(std::unordered_map<int, std::list<LabeledDigit>>());
  mMarker.mReferenceTime = 0.;
  mMarker.mPositionInBuffer = mTimedDigits.begin();
}

void DigitsWriteoutBuffer::clear()
{
  std::cout << "Clearing ..." << std::endl; 
  mTimedDigits.clear();
  mMarker.mReferenceTime = 0.;
  mMarker.mPositionInBuffer = mTimedDigits.begin();
}

void DigitsWriteoutBuffer::addDigit(unsigned int towerID, LabeledDigit dig, double eventTime)
{
  auto positionMarker = mMarker.mPositionInBuffer - mTimedDigits.begin();
  int nsamples = int((eventTime - mMarker.mReferenceTime) / mTimeBinWidth);
  auto &timeEntry = *(mMarker.mPositionInBuffer + nsamples);
  auto towerEntry = timeEntry.find(towerID);
  if(towerEntry == timeEntry.end()) {
    towerEntry = timeEntry.insert(std::pair<int, std::list<o2::emcal::LabeledDigit>>(towerID, std::list<o2::emcal::LabeledDigit>())).first;
  }
  towerEntry->second.push_back(dig);
  //timeEntry->insert(std::make_pair(towerID, dig));
}

void DigitsWriteoutBuffer::forwardMarker(double eventTime)
{
  mMarker.mReferenceTime = eventTime;
  mMarker.mPositionInBuffer++;

  // Allocate new memory at the end
  mTimedDigits.push_back(std::unordered_map<int, std::list<LabeledDigit>>());

  // Drop entry at the front, because it is outside the current readout window
  // only done if we have at least 15 entries
  if (mMarker.mPositionInBuffer - mTimedDigits.begin() > mNumberReadoutSamples) {
    mTimedDigits.pop_front();
  }

}

gsl::span<std::unordered_map<int, std::list<LabeledDigit>>> DigitsWriteoutBuffer::getLastNSamples(int nsamples)
{
  return gsl::span<std::unordered_map<int, std::list<LabeledDigit>>>(&mTimedDigits[int(mMarker.mPositionInBuffer - mTimedDigits.begin() - nsamples)], nsamples);
}
