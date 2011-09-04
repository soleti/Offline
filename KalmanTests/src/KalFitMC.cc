//
// MC functions associated with KalFit
// $Id: KalFitMC.cc,v 1.7 2011/09/04 15:04:30 mu2ecvs Exp $
// $Author: mu2ecvs $ 
// $Date: 2011/09/04 15:04:30 $
//
//geometry
#include "GeometryService/inc/GeometryService.hh"
#include "GeometryService/inc/GeomHandle.hh"
#include "GeometryService/inc/getTrackerOrThrow.hh"
#include "VirtualDetectorGeom/inc/VirtualDetector.hh"
#include "GeometryService/inc/DetectorSystem.hh"
#include "BFieldGeom/inc/BFieldManager.hh"
// services
#include "ConditionsService/inc/ConditionsHandle.hh"
#include "ConditionsService/inc/ParticleDataTable.hh"
#include "art/Framework/Services/Optional/TFileService.h"
// data
#include "RecoDataProducts/inc/StrawHitCollection.hh"
#include "RecoDataProducts/inc/StrawHit.hh"
#include "MCDataProducts/inc/StrawHitMCTruth.hh"
#include "MCDataProducts/inc/PtrStepPointMCVectorCollection.hh"
#include "MCDataProducts/inc/StrawHitMCTruthCollection.hh"
#include "MCDataProducts/inc/StepPointMCCollection.hh"
// tracker
#include "TrackerGeom/inc/Tracker.hh"
#include "TrackerGeom/inc/Straw.hh"
// BaBar
#include "BaBar/BaBar.hh"
#include "BaBar/PdtPid.hh"
#include "KalmanTests/inc/TrkDef.hh"
#include "KalmanTests/inc/TrkStrawHit.hh"
#include "KalmanTests/inc/DetStrawHitElem.hh"
#include "KalmanTests/inc/KalFitMC.hh"
#include "TrkBase/TrkHelixUtils.hh"
#include "TrkBase/TrkMomCalculator.hh"
#include "BField/BFieldFixed.hh"
#include "KalmanTrack/KalHit.hh"
//CLHEP
#include "CLHEP/Units/PhysicalConstants.h"
// root 
#include "TMath.h"
#include "TFile.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "TApplication.h"
#include "TGMsgBox.h"
#include "TTree.h"
// C++
#include <iostream>
#include <string>
#include <functional>

using namespace std; 
 
 
namespace mu2e 
{
  // comparison functor for ordering step points
    struct timecomp : public binary_function<MCStepItr,MCStepItr, bool> {
      bool operator()(MCStepItr x,MCStepItr y) { return x->time() < y->time(); }
  };

 KalFitMC::~KalFitMC(){}
  
  KalFitMC::KalFitMC(fhicl::ParameterSet const& pset) :
    _mintrkmom(pset.get<double>("minTrkMom",60.0)),
    _mct0err(pset.get<double>("mcT0Err",-0.5)),
    _debug(pset.get<int>("debugLevel",0)),
    _minnhits(pset.get<unsigned>("minNHits",10)),
    _maxnhits(pset.get<unsigned>("maxNHits",200)),
    _purehits(pset.get<bool>("pureHits",true)),
    _trkdiag(0),_hitdiag(0)
  {
// define the ids of the midplane and entrance virtual detector: there must be a better way to initialize these, FIXME!!!
    _midvids.push_back(11);
    _midvids.push_back(12);
    _entvids.push_back(13);
    _entvids.push_back(14); 
  }

// Find MC truth for the given particle entering a given detector(s).  Unfortunately the same logical detector can map
// onto several actual detectors, as they are not allowed to cross volume boundaries, so we have to check against
// several ids.  The track may also pass through this detector more than once, so we return a vector, sorted by time.
  void 
  KalFitMC::findMCSteps(StepPointMCCollection const* mcsteps, cet::map_vector_key const& trkid, std::vector<int> const& vids,
  std::vector<MCStepItr>& steps) {
    steps.clear();
  // Loop over the step points, and find the one corresponding to the given detector
    for( MCStepItr imcs =mcsteps->begin();imcs!= mcsteps->end();imcs++){
      if( imcs->trackId() == trkid && std::find(vids.begin(),vids.end(),imcs->volumeId()) != vids.end()){
        steps.push_back(imcs);
      }
    }
  // sort these in time
    std::sort(steps.begin(),steps.end(),timecomp());
  }

// define seed helix, t0, and hits coming from a given particle using MC truth.  Note that the input
// trkdef object must reference a valid straw hit collection
  bool
  KalFitMC::trkFromMC(MCEvtData const& mcdata,cet::map_vector_key const& trkid,TrkDef& mytrk) {
// preset to failure
    bool retval(false);
    ConditionsHandle<ParticleDataTable> pdt("ignored");
    unsigned nstraws = mytrk.strawHitCollection()->size();
    if(nstraws >= _minnhits ){
      GeomHandle<DetectorSystem> det;
      GeomHandle<VirtualDetector> vdg;
      std::vector<size_t> indices;
// find the mcstep at the middle of the detector
      std::vector<MCStepItr> steps;
      findMCSteps(mcdata._mcvdsteps,trkid,_midvids,steps);
      if(steps.size() > 0 && vdg->exist(steps[0]->volumeId())&& steps[0]->momentum().mag() > _mintrkmom){
// take the first point
        MCStepItr imcs = steps[0];
        double t0 = imcs->time();
        double charge = pdt->particle(imcs->simParticle()->pdgId()).ref().charge();
        CLHEP::Hep3Vector mom = imcs->momentum();
// need to transform into the tracker coordinate system
        CLHEP::Hep3Vector pos = det->toDetector(imcs->position());
        if(_debug > 1)std::cout << "Defining track at virtual detector id= " << imcs->volumeId()
          << " name " << vdg->name(imcs->volumeId())
          << " position = " << pos
          << " momentum = " << mom
          << " time = " << t0 << std::endl;
// find the indices of the true primary particle straw hits
// this t0 is a calorimeter artifact introduced in the tracking code, it must be subtracted.
// eventually this part of the simulation should be moved to the calorimeter code, FIXME!!!      
        t0 += mcdata._mcstrawhits->at(0).t0();
        for(unsigned istraw=0;istraw<nstraws;istraw++){
          PtrStepPointMCVector const& mcptr(mcdata._mchitptr->at(istraw));
          unsigned nprimary(0);
          for( size_t j=0; j<mcptr.size(); ++j ) {
            StepPointMC const& mchit = *mcptr[j];
 // not sure if this works with bkgs merged FIXME!!!!  we also need to treat energy from daughters different
 // from energy from completely different particles
            if( mchit.trackId() == trkid )nprimary++;
          }
// decide if we want all hits with any contribution from this particle, or only pure hits from this particle.
          if( nprimary > 0 && (nprimary == mcptr.size() || (!_purehits) ) )indices.push_back(istraw);
        }
        if(indices.size() >= _minnhits && indices.size() <= _maxnhits){
// nominal magnetic field.
          GeomHandle<BFieldManager> bfMgr;
          HepVector parvec(5,0);
// Babar interface still uses HepPoint: FIXME!!!!
// use the z component of th enominal field to define the initial helix parameters.  Off-axis terms are
// ignored anyways by this interface
          double hflt(0.0);
          TrkHelixUtils::helixFromMom( parvec, hflt, 
            HepPoint(pos.x(),pos.y(),pos.z()),
            mom,charge,bfMgr->getDSUniformValue().z());
  // dummy covariance matrix; this should be set according to measured values, FIXME!!!!!
          HepSymMatrix dummy(5,1); 
          dummy(1,1)=1.; dummy(2,2)=0.1*0.1;dummy(3,3)=1e-2*1e-2;
          dummy(4,4)=1.; dummy(5,5)=0.1*0.1;
          mytrk = TrkDef(mytrk.strawHitCollection(),indices,parvec,dummy,t0,_mct0err);
          retval = true;
        }
      }
    }
    return retval;
  }
 
  void KalFitMC::hitDiag(MCEvtData const& mcdata,const TrkStrawHit* strawhit) {
    if(_hitdiag == 0)createHitDiag();
// straw info
    _edep = strawhit->strawHit().energyDep();
//    CLHEP::Hep3Vector dmid = strawhit->wirePosition()-strawhit->straw().getMidPoint(); 
// trkhit  info
    _rdrift = strawhit->driftRadius();
    _rdrifterr = strawhit->driftRadiusErr();
    _amb = strawhit->ambig();
    CLHEP::Hep3Vector shpos;
    strawhit->hitPosition(shpos);
    _shpos = shpos;
//    _shposs.push_back(shpos);
    _dmid = strawhit->timeDiffDist();
    _dmiderr = strawhit->timeDiffDistErr();
// residual info
    double resid,residerr;
    if(strawhit->resid(resid,residerr,true)){
      _resid = resid;
      _residerr = residerr;
    } else {
      _resid = _residerr = -100.0;
    }
    _hitt0 = strawhit->hitT0();
    _hitt0err = strawhit->hitT0Err();
    _hflt = strawhit->hitLen();
    _trkflt = strawhit->fltLen();
    _active = strawhit->isActive();
    _use = strawhit->usability();
// mc information
    unsigned istraw = strawhit->index();
    StrawHitMCTruth const& mcstrawhit = (mcdata._mcstrawhits->at(istraw));
    _mcrdrift = mcstrawhit.driftDistance();
    _mcdmid = mcstrawhit.distanceToMid();
    CLHEP::Hep3Vector mcpos;
    PtrStepPointMCVector const& mcptr(mcdata._mchitptr->at(istraw));
    _nmcsteps = mcptr.size();
    double esum(0.0);
    double mct0(0.0);
    _pdist = 0.0;
    _pperp = 0.0;
    _pmom = 0.0;
    for( size_t imc=0; imc< _nmcsteps; ++imc ) {
      StepPointMC const& mchit = *mcptr[imc];
      if( mchit.trackId().asInt() == 1 ){
        double edep = mchit.eDep();
        esum += edep;
        mcpos += mchit.position()*edep;
        mct0 += mchit.time()*edep;
        CLHEP::Hep3Vector dprod = mchit.position()-mchit.simParticle()->startPosition();
        _pdist += dprod.mag()*edep;
        _pperp += dprod.perp(Hep3Vector(0.0,0.0,1.0))*edep;
        _pmom = mchit.momentum().mag()*edep;
      }
    }
    if(esum > 0.0){
      mcpos /= esum;
      mct0 /= esum;
      _pdist /= esum;
      _pperp /= esum;
      _pmom /= esum;
    }
// must correct for straw hit t0; what does that represent physically????
    mct0 += mcstrawhit.t0();
    _mcpos = mcpos;
    _mchitt0 = mct0;
    _hitdiag->Fill();
  }
  
  void
  KalFitMC::trkDiag(MCEvtData const& mcdata,TrkDef const& mytrk, TrkKalFit const& myfit) {
    if(_trkdiag == 0)createTrkDiag();
// initial t0 value
    _t00 = myfit._t00.t0();
    _t00err = myfit._t00.t0Err();
// iterations
    _nt0iter = myfit._nt0iter;
    _nweediter = myfit._nweediter;
// final t0 value
    _t0 = myfit._t0.t0();
    _t0err = myfit._t0.t0Err();
    double loclen(0.0);
    if(myfit._krep != 0 && myfit._krep->fitCurrent()){
      _trkid = myfit._krep->parentTrack()->id();
      _nhits = myfit._krep->hotList()->nHit();
      _fitstatus = myfit._krep->fitCurrent();
      _niter = myfit._krep->iterations();
      _ndof = myfit._krep->nDof();
      _nactive = myfit._krep->nActive();
      _chisq = myfit._krep->chisq();
      // get the fit at the first hit
      const TrkStrawHit* firsthit = dynamic_cast<const TrkStrawHit*>(myfit._krep->firstHit()->kalHit()->hitOnTrack());
      double fltlen=firsthit->fltLen();
      const TrkSimpTraj* ltraj = myfit._krep->localTrajectory(fltlen,loclen);
      _fitpar = helixpar(ltraj->parameters()->parameter());
      _fiterr = helixpar(ltraj->parameters()->covariance());
      CLHEP::Hep3Vector fitmom = myfit._krep->momentum(fltlen);
      BbrVectorErr momerr = myfit._krep->momentumErr(fltlen);
      _fitmom = fitmom.mag();
      Hep3Vector momdir = fitmom.unit();
      HepVector momvec(3);
      for(int icor=0;icor<3;icor++)
        momvec[icor] = momdir[icor];
      _fitmomerr = sqrt(momerr.covMatrix().similarity(momvec));
      CLHEP::Hep3Vector seedmom = TrkMomCalculator::vecMom(*(myfit._krep->seed()),myfit._trk->bField(),0.0);
      _seedmom = seedmom.mag();
    } else {
      _fitstatus = -1;
      _nhits = -1;
      _fitmom = -1.0;
      _seedmom = -1.0;
    }
      // get MC truth at this point
    GeomHandle<BFieldManager> bfMgr;
    _tshinfo.clear();
      // loop over hits.  Order doesn't matter here
    for(std::vector<TrkStrawHit*>::const_iterator itsh = myfit._hits.begin(); itsh != myfit._hits.end(); itsh++){
      const TrkStrawHit* tsh = *itsh;
      if(tsh != 0){
        TrkStrawHitInfo tshinfo;
        tshinfo._active = tsh->isActive();
        tshinfo._usable = tsh->usability();
        PtrStepPointMCVector const& mcptr(mcdata._mchitptr->at(tsh->index()));
        tshinfo._mcn = mcptr.size();
	std::vector<trksum> mcsum;
	KalFitMC::fillMCSummary(mcptr,mcsum); 
	tshinfo._mcnunique = mcsum.size();
	tshinfo._mcpdg = mcsum[0]._pdgid;
	tshinfo._mcgen = mcsum[0]._gid;
	tshinfo._mcproc = mcsum[0]._pid;
        _tshinfo.push_back(tshinfo);
      }
    }
// find the mc info at the entrance to the detector
    std::vector<MCStepItr> steps;
    GeomHandle<VirtualDetector> vdg;
    GeomHandle<DetectorSystem> det;
    cet::map_vector_key trkid(1); // conversion electron
    findMCSteps(mcdata._mcvdsteps,trkid,_entvids,steps);
    if(steps.size() > 0 && vdg->exist(steps[0]->volumeId())){
    // take the first point; hopefully this is the entrance!
      MCStepItr imcs = steps[0];
      CLHEP::Hep3Vector mcmom = imcs->momentum();
      CLHEP::Hep3Vector mcpos = det->toDetector(imcs->position());
  // initial length estimate defines convention for flightlength, z0
      double mclen(0.0);
      HepVector mcpar(5,0);
      TrkHelixUtils::helixFromMom( mcpar, mclen,
        HepPoint(mcpos.x(),mcpos.y(),mcpos.z()),
        mcmom,-1.,bfMgr->getDSUniformValue().z());
      _mcentmom = imcs->momentum().mag();
      _mcentpar = helixpar(mcpar);
      _mcentt0 = imcs->time();
    } else {
      _mcentmom = -1;
      _mcentt0 = 0.0;
    }

// mc at midpoint of detector
    findMCSteps(mcdata._mcvdsteps,trkid,_midvids,steps);
    if(steps.size() > 0 && vdg->exist(steps[0]->volumeId())){
// take the first point; hopefully this is the entrance!
      MCStepItr imcs = steps[0];
      CLHEP::Hep3Vector mcmom = imcs->momentum();
      CLHEP::Hep3Vector mcpos = det->toDetector(imcs->position());
// initial length estimate defines convention for flightlength, z0
      double mclen(0.0);
      HepVector mcpar(5,0);
      TrkHelixUtils::helixFromMom( mcpar, mclen,
        HepPoint(mcpos.x(),mcpos.y(),mcpos.z()),
        mcmom,-1.,bfMgr->getDSUniformValue().z());
      _mcmidmom = imcs->momentum().mag();
      _mcmidpar = helixpar(mcpar);
      _mcmidt0 = imcs->time();
    } else {
      _mcmidmom = -1;
      _mcmidt0 = 0.0;
    }
    
    _trkdiag->Fill(); 
  }

// Summarize an associated set of StepPointMCs from a StrawHit according to their parents.  This assignes
// daughter contributions to their parents
  void KalFitMC::fillMCSummary(PtrStepPointMCVector const& mcptr,std::vector<trksum>& summary ){
// first, create a map from daughters to mothers
    std::map<SPPtr,SPPtr> mdmap;
    findRelatives(mcptr,mdmap);
// Loop over the step points, and fill the summary vector
    for( size_t imc=0; imc< mcptr.size(); ++imc ) {
// find the primary parent, and create a pair for this step point's energy deposition
      art::Ptr<SimParticle> sp = mcptr[imc]->simParticle();
// find it's parent
      art::Ptr<SimParticle> spp = mdmap[sp];
// create the summary
      trksum tsum(*mcptr[imc],spp);
// Add this energy to this particle, or create the entry if this is the first time this particle is seen
      std::vector<trksum>::iterator ifnd = std::find(summary.begin(),summary.end(),tsum);
      if(ifnd == summary.end())
        summary.push_back(tsum);
      else
        ifnd->append(*mcptr[imc]);
    }
// sort this according to deposited energy
    std::sort(summary.begin(),summary.end(),trksum::ecomp());
  }

// map daughters onto parents within the context of an associated set of StepPointMCs (like from a StrawHit).
  void KalFitMC::findRelatives(PtrStepPointMCVector const& mcptr,std::map<SPPtr,SPPtr>& mdmap){
    // loop over steps
    for( size_t imc=0; imc< mcptr.size(); ++imc ) {
      art::Ptr<SimParticle> spp = mcptr[imc]->simParticle();		  
      if(mdmap.find(spp) == mdmap.end()){
    // start with self-reference
        mdmap[spp] = spp;
      }
    }
    // loop over the steps again, trying to map particles to their parents
    for( size_t imc=0; imc< mcptr.size(); ++imc ) {
      art::Ptr<SimParticle> spp = mcptr[imc]->simParticle();		  
      if(mdmap[spp] == spp){
    // move up through the genealogy till we find the highest-rank parent directly contributing
    // to this strawhit
        art::Ptr<SimParticle> sppp = spp->parent();
        while(sppp){
          std::map<SPPtr,SPPtr>::iterator ifnd = mdmap.find(sppp);
          if(ifnd != mdmap.end())
    // repoint this particle to its highest-level contributing parent
            mdmap[spp] = sppp;
    // move on to the parent's parent, as there can be gaps in the direct contribution
          sppp = sppp->parent();
        }
      }
    }
  }
 
  TTree*
  KalFitMC::createTrkDiag(){
    art::ServiceHandle<art::TFileService> tfs;
    _trkdiag=tfs->make<TTree>("trkdiag","trk diagnostics");
    _trkdiag->Branch("eventid",&_eventid,"eventid/i");
    _trkdiag->Branch("trkid",&_trkid,"trkid/i");
    _trkdiag->Branch("fitstatus",&_fitstatus,"fitstatus/I");
    _trkdiag->Branch("t00",&_t00,"t00/F");
    _trkdiag->Branch("t00err",&_t0err,"t00err/F");
    _trkdiag->Branch("t0",&_t0,"t0/F");
    _trkdiag->Branch("t0err",&_t0err,"t0err/F");
    _trkdiag->Branch("nhits",&_nhits,"nhits/I");
    _trkdiag->Branch("ndof",&_ndof,"ndof/I");
    _trkdiag->Branch("niter",&_niter,"niter/i");
    _trkdiag->Branch("nt0iter",&_nt0iter,"nt0iter/i");
    _trkdiag->Branch("nweediter",&_nweediter,"nweediter/i");
    _trkdiag->Branch("nactive",&_nactive,"nactive/I");
    _trkdiag->Branch("chisq",&_chisq,"chisq/F");
    _trkdiag->Branch("fitmom",&_fitmom,"fitmom/F");
    _trkdiag->Branch("fitmomerr",&_fitmomerr,"fitmomerr/F");
    _trkdiag->Branch("seedmom",&_seedmom,"seedmom/F");
    _trkdiag->Branch("fitpar",&_fitpar,"d0/F:p0/F:om/F:z0/F:td/F");
    _trkdiag->Branch("fiterr",&_fiterr,"d0err/F:p0err/F:omerr/F:z0err/F:tderr/F");
// mc info at tracker entrance and midplane
    _trkdiag->Branch("mcentpar",&_mcentpar,"mcentd0/F:mcentp0/F:mcentom/F:mcentz0/F:mcenttd/F");
    _trkdiag->Branch("mcentt0",&_mcentt0,"mcentt0/F");
    _trkdiag->Branch("mcentmom",&_mcentmom,"mcentmom/F");
    _trkdiag->Branch("mcmidpar",&_mcmidpar,"mcmidd0/F:mcmidp0/F:mcmidom/F:mcmidz0/F:mcmidtd/F");
    _trkdiag->Branch("mcmidt0",&_mcmidt0,"mcmidt0/F");
    _trkdiag->Branch("mcmidmom",&_mcmidmom,"mcmidmom/F");
// track hit info    
    _trkdiag->Branch("tshinfo",&_tshinfo);
    return _trkdiag;
  }

  TTree*
  KalFitMC::createHitDiag(){
    art::ServiceHandle<art::TFileService> tfs;
    _hitdiag=tfs->make<TTree>("hitdiag","hit diagnostics");      
    _hitdiag->Branch("shpos",&_shpos,"x/F:y/F:z/F");
    _hitdiag->Branch("rdrift",&_rdrift,"rdrift/F");
    _hitdiag->Branch("rdrifterr",&_rdrifterr,"rdrifterr/F");
    _hitdiag->Branch("amb",&_amb,"amb/I");
    _hitdiag->Branch("hitt0",&_hitt0,"hitt0/F");
    _hitdiag->Branch("hitt0err",&_hitt0err,"hitt0err/F");
    _hitdiag->Branch("dmid",&_dmid,"dmid/F");
    _hitdiag->Branch("dmiderr",&_dmiderr,"dmiderr/F");
    _hitdiag->Branch("resid",&_resid,"resid/F");
    _hitdiag->Branch("residerr",&_residerr,"residerr/F");
    _hitdiag->Branch("hflt",&_hflt,"hflt/F");
    _hitdiag->Branch("trkflt",&_trkflt,"trkflt/F");
    _hitdiag->Branch("active",&_active,"active/B");
    _hitdiag->Branch("use",&_use,"use/I");
    _hitdiag->Branch("edep",&_edep,"edep/F");
    _hitdiag->Branch("pdist",&_pdist,"pdist/F");
    _hitdiag->Branch("pperp",&_pperp,"pperp/F");
    _hitdiag->Branch("pmom",&_pmom,"pmom/F");

    _hitdiag->Branch("nmcsteps",&_nmcsteps,"nmcsteps/i");
    _hitdiag->Branch("mcpos",&_mcpos,"x/F:y/F:z/F");
    _hitdiag->Branch("mcrdrift",&_mcrdrift,"mcrdrift/F");
    _hitdiag->Branch("mchitt0",&_mchitt0,"mchitt0/F");
    _hitdiag->Branch("mcdmid",&_mcdmid,"mcdmid/F");

//      _hitdiag->Branch("shdir","CLHEP::HepVector",&_shdir);
    return _hitdiag;
  }
}
