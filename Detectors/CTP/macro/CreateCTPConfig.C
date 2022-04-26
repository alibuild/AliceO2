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

/// \file CreateCTPConfig.C
/// \brief create CTP config, test it and add to database
/// \author Roman Lietava

#if !defined(__CLING__) || defined(__ROOTCLING__)

#include "FairLogger.h"
#include "CCDB/CcdbApi.h"
#include "CCDB/BasicCCDBManager.h"
#include "DataFormatsCTP/Configuration.h"
#include <string>
#include <map>
#include <iostream>
#endif
using namespace o2::ctp;
void CreateCTPConfig(long tmin = 0, long tmax = -1, std::string ccdbHost = "http://ccdb-test.cern.ch:8080")
{
  /// Demo configuration
  CTPConfiguration ctpcfg;

  // run3 config
  //
  std::string cfgRun3str =
    "cluster clu1 fv0 ft0 fdd its mft mid mch tpc zdc tst tof \n\
0 cl_ph 1 \n\
";
  // ctpcfg.loadConfiguration(cfgstr);
  ctpcfg.loadConfigurationRun3(cfgRun3str);
  ctpcfg.printStream(std::cout);
  std::cout << "Going write to db" << std::endl;
  return;
  ///
  /// add to database
  o2::ccdb::CcdbApi api;
  map<string, string> metadata; // can be empty
  api.init(ccdbHost.c_str());   // or http://localhost:8080 for a local installation
  // store abitrary user object in strongly typed manner
  try {
    //api.storeAsTFileAny(&ctpcfg, o2::ctp::CCDBPathCTPConfig, metadata, tmin, tmax);
  } catch (...) {
    std::cout << "Error: can not wrote to database." << std::endl;
    return;
  }
  std::cout << "CTP config in database" << std::endl;
  return;
  /// get frp, database
  auto& mgr = o2::ccdb::BasicCCDBManager::instance();
  mgr.setURL(ccdbHost);
  auto ctpconfigdb = mgr.get<CTPConfiguration>(CCDBPathCTPConfig);
  ctpconfigdb->printStream(std::cout);
}
