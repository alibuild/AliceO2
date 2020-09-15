// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.
#include "Framework/runDataProcessing.h"
#include "Framework/AnalysisTask.h"
#include "Framework/AnalysisDataModel.h"
#include "Framework/ASoAHelpers.h"
#include "ReconstructionDataFormats/Track.h"
#include "Analysis/RecoDecay.h"
#include "Analysis/trackUtilities.h"
#include "Analysis/StrangenessTables.h"
#include "Analysis/TrackSelection.h"
#include "Analysis/TrackSelectionTables.h"

#include <TFile.h>
#include <TH2F.h>
#include <TProfile.h>
#include <TLorentzVector.h>
#include <Math/Vector4D.h>
#include <TPDGCode.h>
#include <TDatabasePDG.h>
#include <cmath>
#include <array>
#include <cstdlib>
#include "PID/PIDResponse.h"
#include "Framework/ASoAHelpers.h"

using namespace o2;
using namespace o2::framework;
using namespace o2::framework::expressions;
using std::array;

struct cascadeQA {
  //Basic checks
  OutputObj<TH1F> hMassXiMinus{TH1F("hMassXiMinus", "", 3000, 0.0, 3.0)};
  OutputObj<TH1F> hMassXiPlus{TH1F("hMassXiPlus", "", 3000, 0.0, 3.0)};
  OutputObj<TH1F> hMassOmegaMinus{TH1F("hMassOmegaMinus", "", 3000, 0.0, 3.0)};
  OutputObj<TH1F> hMassOmegaPlus{TH1F("hMassOmegaPlus", "", 3000, 0.0, 3.0)};

  OutputObj<TH1F> hV0Radius{TH1F("hV0Radius", "", 1000, 0.0, 100)};
  OutputObj<TH1F> hCascRadius{TH1F("hCascRadius", "", 1000, 0.0, 100)};
  OutputObj<TH1F> hV0CosPA{TH1F("hV0CosPA", "", 1000, 0.95, 1.0)};
  OutputObj<TH1F> hCascCosPA{TH1F("hCascCosPA", "", 1000, 0.95, 1.0)};
  OutputObj<TH1F> hDCAPosToPV{TH1F("hDCAPosToPV", "", 1000, 0.0, 10.0)};
  OutputObj<TH1F> hDCANegToPV{TH1F("hDCANegToPV", "", 1000, 0.0, 10.0)};
  OutputObj<TH1F> hDCABachToPV{TH1F("hDCABachToPV", "", 1000, 0.0, 10.0)};
  OutputObj<TH1F> hDCAV0ToPV{TH1F("hDCAV0ToPV", "", 1000, 0.0, 10.0)};
  OutputObj<TH1F> hDCAV0Dau{TH1F("hDCAV0Dau", "", 1000, 0.0, 10.0)};
  OutputObj<TH1F> hDCACascDau{TH1F("hDCACascDau", "", 1000, 0.0, 10.0)};
  OutputObj<TH1F> hLambdaMass{TH1F("hLambdaMass", "", 1000, 0.0, 10.0)};

  void process(aod::Collision const& collision, soa::Join<aod::Cascades, aod::CascDataExt> const& Cascades)
  {
    for (auto& casc : Cascades) {
      if (casc.charge() < 0) { //FIXME: could be done better...
        hMassXiMinus->Fill(casc.mXi());
        hMassOmegaMinus->Fill(casc.mOmega());
      } else {
        hMassXiPlus->Fill(casc.mXi());
        hMassOmegaPlus->Fill(casc.mOmega());
      }
      //The basic eleven!
      hV0Radius->Fill(casc.v0radius());
      hCascRadius->Fill(casc.cascradius());
      hV0CosPA->Fill(casc.v0cosPA(collision.posX(), collision.posY(), collision.posZ()));
      hCascCosPA->Fill(casc.casccosPA(collision.posX(), collision.posY(), collision.posZ()));
      hDCAPosToPV->Fill(casc.dcapostopv());
      hDCANegToPV->Fill(casc.dcanegtopv());
      hDCABachToPV->Fill(casc.dcabachtopv());
      hDCAV0ToPV->Fill(casc.dcav0topv(collision.posX(), collision.posY(), collision.posZ()));
      hDCAV0Dau->Fill(casc.dcaV0daughters());
      hDCACascDau->Fill(casc.dcacascdaughters());
      hLambdaMass->Fill(casc.mLambda());
    }
  }
};

struct cascadeconsumer {
  OutputObj<TH2F> h2dMassXiMinus{TH2F("h2dMassXiMinus", "", 200, 0, 10, 200, 1.322 - 0.100, 1.322 + 0.100)};
  OutputObj<TH2F> h2dMassXiPlus{TH2F("h2dMassXiPlus", "", 200, 0, 10, 200, 1.322 - 0.100, 1.322 + 0.100)};
  OutputObj<TH2F> h2dMassOmegaMinus{TH2F("h2dMassOmegaMinus", "", 200, 0, 10, 200, 1.672 - 0.100, 1.672 + 0.100)};
  OutputObj<TH2F> h2dMassOmegaPlus{TH2F("h2dMassOmegaPlus", "", 200, 0, 10, 200, 1.672 - 0.100, 1.672 + 0.100)};

  //Selection criteria
  Configurable<double> v0cospa{"v0cospa", 0.999, "V0 CosPA"};       //double -> N.B. dcos(x)/dx = 0 at x=0)
  Configurable<double> casccospa{"casccospa", 0.999, "Casc CosPA"}; //double -> N.B. dcos(x)/dx = 0 at x=0)
  Configurable<float> dcav0dau{"dcav0dau", 1.0, "DCA V0 Daughters"};
  Configurable<float> dcacascdau{"dcacascdau", .3, "DCA Casc Daughters"};
  Configurable<float> dcanegtopv{"dcanegtopv", .1, "DCA Neg To PV"};
  Configurable<float> dcapostopv{"dcapostopv", .1, "DCA Pos To PV"};
  Configurable<float> dcabachtopv{"dcabachtopv", .1, "DCA Bach To PV"};
  Configurable<float> dcav0topv{"dcav0topv", .1, "DCA V0 To PV"};
  Configurable<float> v0radius{"v0radius", 2.0, "v0radius"};
  Configurable<float> cascradius{"cascradius", 1.0, "cascradius"};
  Configurable<float> v0masswindow{"v0masswindow", 0.008, "v0masswindow"};

  Filter preFilterV0 =
    aod::cascdata::dcapostopv > dcapostopv&& aod::cascdata::dcanegtopv > dcanegtopv&&
                                                                           aod::cascdata::dcabachtopv > dcabachtopv&&
                                                                                                          aod::cascdata::dcaV0daughters < dcav0dau&& aod::cascdata::dcacascdaughters < dcacascdau;

  void process(aod::Collision const& collision, soa::Filtered<soa::Join<aod::Cascades, aod::CascDataExt>> const& Cascades)
  {
    for (auto& casc : Cascades) {
      //FIXME: dynamic columns cannot be filtered on?
      if (casc.v0radius() > v0radius &&
          casc.cascradius() > cascradius &&
          casc.v0cosPA(collision.posX(), collision.posY(), collision.posZ()) > v0cospa &&
          casc.casccosPA(collision.posX(), collision.posY(), collision.posZ()) > casccospa &&
          casc.dcav0topv(collision.posX(), collision.posY(), collision.posZ()) > dcav0topv) {
        if (casc.charge() < 0) { //FIXME: could be done better...
          h2dMassXiMinus->Fill(casc.pt(), casc.mXi());
          h2dMassOmegaMinus->Fill(casc.pt(), casc.mOmega());
        } else {
          h2dMassXiPlus->Fill(casc.pt(), casc.mXi());
          h2dMassOmegaPlus->Fill(casc.pt(), casc.mOmega());
        }
      }
    }
  }
};

/// Extends the v0data table with expression columns
struct cascadeinitializer {
  Spawns<aod::CascDataExt> cascdataext;
  void init(InitContext const&) {}
};

WorkflowSpec defineDataProcessing(ConfigContext const&)
{
  return WorkflowSpec{
    adaptAnalysisTask<cascadeconsumer>("lf-cascadeconsumer"),
    adaptAnalysisTask<cascadeQA>("lf-cascadeQA"),
    adaptAnalysisTask<cascadeinitializer>("lf-cascadeinitializer")};
}
