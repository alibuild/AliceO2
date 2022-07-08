// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "Framework/DataProcessorSpec.h"

using namespace o2::framework;

// we need to add workflow options before including Framework/runDataProcessing
void customize(std::vector<o2::framework::ConfigParamSpec>& workflowOptions)
{
  // option allowing to set parameters
  workflowOptions.push_back(ConfigParamSpec{"use-ccdb", o2::framework::VariantType::Bool, false, {"enable access to ccdb ZDC calibration objects"}});
  workflowOptions.push_back(ConfigParamSpec{"do-lhc-phase", o2::framework::VariantType::Bool, true, {"do LHC clock phase calibration"}});
}

// ------------------------------------------------------------------

#include "Framework/runDataProcessing.h"

WorkflowSpec defineDataProcessing(ConfigContext const& configcontext)
{
  WorkflowSpec specs;
  auto useCCDB = configcontext.options().get<bool>("use-ccdb");
  auto doLHCcalib = configcontext.options().get<bool>("do-lhc-phase");

  LOG(INFO) << "ZDC Calibration workflow: options";
  LOG(INFO) << "useCCDB = " << useCCDB;
  return specs;
}
