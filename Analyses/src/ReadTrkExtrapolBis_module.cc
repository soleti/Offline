//
//
//
// $Id: ReadExtrapol_module.cc,v 1.18 2014/08/20 14:23:09 murat Exp $
// $Author: murat $
// $Date: 2014/08/20 14:23:09 $
//
// Original author G. Pezzullo
//

// Framework includes.
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Services/Optional/TFileDirectory.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "art/Framework/Principal/Handle.h"

//CLHEP includes
#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Matrix/Vector.h"
#include "CLHEP/Vector/TwoVector.h"
#include "CLHEP/Matrix/Matrix.h"
#include "CLHEP/Geometry/HepPoint.h"
#include "CLHEP/Units/PhysicalConstants.h"
#include "CLHEP/Matrix/SymMatrix.h"

#include "KalmanTests/inc/KalRepCollection.hh"

// From the art tool-chain
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

//tracker includes
#include "BaBar/BaBar.hh"
#include "BaBar/Constants.hh"
#include "TrkBase/TrkRep.hh"
#include "TrkBase/HelixParams.hh"
#include "TrkBase/HelixTraj.hh"
#include "KalmanTrack/KalRep.hh"
// conditions
#include "ConditionsService/inc/ConditionsHandle.hh"
#include "ConditionsService/inc/TrackerCalibrations.hh"
#include "GeometryService/inc/getTrackerOrThrow.hh"
#include "TTrackerGeom/inc/TTracker.hh"
// data
#include "RecoDataProducts/inc/TrkCaloIntersectCollection.hh"

// Other includes.
#include "cetlib/exception.h"

// Mu2e includes.
#include "GeometryService/inc/GeometryService.hh"
#include "GeometryService/inc/GeomHandle.hh"

//root includes
#include "TFile.h"
#include "TNtuple.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "THStack.h"
#include "TGraph.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TLatex.h"
#include "TMath.h"

// From the art tool-chain
#include <cmath>
#include <deque>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include "cetlib/pow.h"


using namespace std;

namespace mu2e {
  

  class ReadTrkExtrapolBis : public art::EDAnalyzer {

     public:
     
	 explicit ReadTrkExtrapolBis(fhicl::ParameterSet const& pset):
	   art::EDAnalyzer(pset),
	   _diagLevel(pset.get<int>("diagLevel",0)),
	   _trkExtrapolModuleLabel(pset.get<std::string>("trkExtrapolModuleLabel")),
	   _Ntup(0)
	 {}

	 virtual ~ReadTrkExtrapolBis() {}

	 void beginJob();
	 void endJob() {}

	 void analyze(art::Event const& event );



     private:

	  int          _diagLevel;
	  std::string  _trkExtrapolModuleLabel;

	  TTree* _Ntup;
	  Int_t _evt,_ntrks;
	  Float_t _caloSec[100],_trkId[100],_trkTime[100],_trkTimeErr[100], _trkPathLenghtIn[100], _trkPathLenghtInErr[100], _trkPathLenghtOut[100];
	  Float_t _trkMom[100],_trkMomX[100], _trkMomY[100],_trkMomZ[100], _trkPosX[100], _trkPosY[100], _trkPosZ[100];

  };



  void ReadTrkExtrapolBis::beginJob( ) {
 
 
      art::ServiceHandle<art::TFileService> tfs;
      _Ntup  = tfs->make<TTree>("trkExt", "Extrapolated track info");

      _Ntup->Branch("evt"    , &_evt ,   "evt/F");
      _Ntup->Branch("ntrks"  , &_ntrks , "ntrks/I");
      _Ntup->Branch("caloSec", &_caloSec, "caloSec[ntrks]/F");
      _Ntup->Branch("trkTime", &_trkTime, "trkTime[ntrks]/F");
      _Ntup->Branch("trkPathLenghtIn", &_trkPathLenghtIn, "trkPathLenghtIn[ntrks]/F");
      _Ntup->Branch("trkPathLenghtInErr", &_trkPathLenghtInErr, "trkPathLenghtInErr[ntrks]/F");
      _Ntup->Branch("trkPathLenghtOut", &_trkPathLenghtOut, "trkPathLenghtOut[ntrks]/F");
      _Ntup->Branch("trkMom", &_trkMom, "trkMom[ntrks]/F");
      _Ntup->Branch("trkMomX", &_trkMomX, "trkMomX[ntrks]/F");
      _Ntup->Branch("trkMomY", &_trkMomY, "trkMomY[ntrks]/F");
      _Ntup->Branch("trkMomZ", &_trkMomZ, "trkMomZ[ntrks]/F");
      _Ntup->Branch("trkPosX", &_trkPosX, "trkPosX[ntrks]/F");
      _Ntup->Branch("trkPosY", &_trkPosY, "trkPosY[ntrks]/F");
      _Ntup->Branch("trkPosZ", &_trkPosZ, "trkPosZ[ntrks]/F");


  }



  void ReadTrkExtrapolBis::analyze(art::Event const& event )
  {

        art::Handle<TrkCaloIntersectCollection>  trjExtrapolHandle;
        event.getByLabel(_trkExtrapolModuleLabel, trjExtrapolHandle);
        TrkCaloIntersectCollection const& trkExtrapols(*trjExtrapolHandle);
    
        _evt   = event.id().event();
        _ntrks = trkExtrapols.size();

	int it(0);
	for (auto const& extrapol: trkExtrapols)
	{
	    double pathLength       = extrapol.pathLengthEntrance();
	    _caloSec[it]            = extrapol.sectionId();
	    _trkTime[it]            = extrapol.trk()->arrivalTime(pathLength);
	    _trkPathLenghtIn[it]    = extrapol.pathLengthEntrance(); 
	    _trkPathLenghtInErr[it] = extrapol.pathLenghtEntranceErr(); 
	    _trkPathLenghtOut[it]   = extrapol.pathLengthExit();
	    _trkMom[it]             = extrapol.trk()->momentum(pathLength).mag();
	    _trkMomX[it]            = extrapol.trk()->momentum(pathLength).x();
	    _trkMomY[it]            = extrapol.trk()->momentum(pathLength).y();
	    _trkMomZ[it]            = extrapol.trk()->momentum(pathLength).z();
	    _trkPosX[it]            = extrapol.trk()->position(pathLength).x(); 
	    _trkPosY[it]            = extrapol.trk()->position(pathLength).y(); 
	    _trkPosZ[it]            = extrapol.trk()->position(pathLength).z(); 
	
	    ++it;
	}

        _Ntup->Fill();
  }


}



using mu2e::ReadTrkExtrapolBis;
DEFINE_ART_MODULE(ReadTrkExtrapolBis);