//---------------------------------------------------------------------------
//
// ClassName:   G4MuonMinusCapturePhysics (incorporating Kevin Lynch'es Muon Capture physics)
//
// Author: 2011 12 21 K. Genser first version based almost direcly on physicsList by Kevin Lynch
//
// Modified:
//
//----------------------------------------------------------------------------
//
// $Id: G4MuonMinusCapturePhysics.cc,v 1.2 2013/10/23 20:50:02 genser Exp $
// $Author: genser $
// $Date: 2013/10/23 20:50:02 $

#include "globals.hh"

#include "G4MuonMinusCapturePhysics.hh"

#include "G4ParticleDefinition.hh"
#include "G4ProcessManager.hh"

#include "G4Gamma.hh"
#include "G4LeptonConstructor.hh"
#include "G4BaryonConstructor.hh"

#include "G4MuAtom.hh"
#include "G4MuAtomTable.hh"
#include "G4MuAtomDecay.hh"
#include "G4MuAtomDecayTable.hh"
#include "G4GenericMuAtom.hh"
#include "G4MuonMinusAtomicCapture.hh"
#include "G4MuPStateModel.hh"
#include "G4MuAtomDIOChannel.hh"
#include "G4MuAtomNuclearCapture.hh"

#include "G4MuPCaptureChannel.hh"
#include "G4MuDCaptureChannel.hh"
#include "G4MuTCaptureChannel.hh"
#include "G4MuHe3CaptureChannels.hh"
#include "G4MuHe4CaptureChannels.hh"
#include "G4MuAtomGenericCaptureChannel.hh"

#include "G4Mu2eConversionChannel.hh"
#include "G4Mu2eOrbitalConversionChannel.hh"

#include "muPHyperfineTransition.hh"
#include "PmuPFormationChannel.hh"

#include "G4MuMolecule.hh"
#include "G4MuMoleculeNuclearCapture.hh"
#include "G4DMuDFusionChannels.hh"


G4MuonMinusCapturePhysics::G4MuonMinusCapturePhysics(G4int verbosity)
  : G4VPhysicsConstructor("MuonMinusCapture"), 
    particlesConstructed(false), processesConstructed(false)
  
{
  SetVerboseLevel(verbosity);
  if(verboseLevel > 1) G4cout << "### G4MuonMinusCapturePhysics instantiated" << G4endl;
}

G4MuonMinusCapturePhysics::G4MuonMinusCapturePhysics(G4String const & name, G4int verbosity)
  : G4VPhysicsConstructor(name), 
    particlesConstructed(false), processesConstructed(false)
{
  SetVerboseLevel(verbosity);
  if(verboseLevel > 1) G4cout << "### G4MuonMinusCapturePhysics instantiated" << G4endl;
}

G4MuonMinusCapturePhysics::~G4MuonMinusCapturePhysics()
{
  //   if(particlesConstructed) {
  //     // delete ... underlying objects should take care of it
  //   }
  
  //   if(processesConstructed) {

  //     // FIXME
  //     //    delete ...
  //     // well, ModularPhysicsList shold take care of it...

  //   }

  // FIXME delte muproc

}

G4MuonMinusAtomicCapture* G4MuonMinusCapturePhysics::GetMuonMinusAtomicCaptureProcess()
{

  if (muproc == 0 ) {
    G4Exception("G4MuonMinusCapturePhysics::GetMuonMinusAtomicCaptureProcess",
                "MMAC0001", JustWarning,
                "No mu- capture process");
  }
  return muproc;

}

void G4MuonMinusCapturePhysics::ConstructParticle()
{
  // G4cout << "G4MuonMinusCapturePhysics::ConstructParticle" << G4endl;

  // gamma
  G4Gamma::GammaDefinition();

  G4LeptonConstructor pLeptonConstructor;
  pLeptonConstructor.ConstructParticle();

  G4BaryonConstructor pBaryonConstructor;
  pBaryonConstructor.ConstructParticle();

  G4MuPStateModel* muPStateModel =  new G4MuPStateModel(); // FIXME leak
  G4MuAtomTable::GetInstance()->StateModel(1,1, muPStateModel);

  G4MuAtom* muatom;  
  // Singlet muP
  muatom = G4MuAtom::MuAtom(1,1,0);
  if (verboseLevel>0) G4cout << muatom->GetParticleName() << G4endl;
  G4MuAtomDecayTable *dt = new G4MuAtomDecayTable(); // FIXME leak
  dt->Insert(new G4MuAtomDIOChannel(muatom->GetParticleName(), 1.));
  muatom->SetMuAtomDecayTable(dt);
  G4MuAtomCaptureKineticsTable *ckt = new G4MuAtomCaptureKineticsTable(muatom); // FIXME leak
  //  ckt->Insert(new G4MuPCaptureChannel(muatom));
  //  ckt->Insert(new muPHyperfineStoT(muatom,1./(2197.*CLHEP::ns)));
  ckt->Insert(new PmuPFormationChannel(muatom,1./(2197.*CLHEP::ns)));
  muatom->CaptureKineticsTable(ckt);

  // Triplet muP
  muatom = G4MuAtom::MuAtom(1,1,2);
  if (verboseLevel>0) G4cout << muatom->GetParticleName() << G4endl;
  dt = new G4MuAtomDecayTable(); // FIXME leak
  dt->Insert(new G4MuAtomDIOChannel(muatom->GetParticleName(), 1.));
  muatom->SetMuAtomDecayTable(dt);
  ckt = new G4MuAtomCaptureKineticsTable(muatom); // FIXME leak
  //  ckt->Insert(new G4MuPCaptureChannel(muatom));
  //  ckt->Insert(new muPHyperfineTtoS(muatom, 1./(2197.*CLHEP::ns)));
  ckt->Insert(new PmuPFormationChannel(muatom,1./(2197.*CLHEP::ns)));
  muatom->CaptureKineticsTable(ckt);

  // muD
  muatom = G4MuAtom::MuAtom(1,2);
  if (verboseLevel>0) G4cout << muatom->GetParticleName() << G4endl;
  dt = new G4MuAtomDecayTable(); // FIXME leak
  dt->Insert(new G4MuAtomDIOChannel(muatom->GetParticleName(), 1.));
  muatom->SetMuAtomDecayTable(dt);
  ckt = new G4MuAtomCaptureKineticsTable(muatom); // FIXME leak
  ckt->Insert(new G4MuDCaptureChannel(muatom));
  muatom->CaptureKineticsTable(ckt);

  // muT
  muatom = G4MuAtom::MuAtom(1,3);
  if (verboseLevel>0) G4cout << muatom->GetParticleName() << G4endl;
  dt = new G4MuAtomDecayTable(); // FIXME leak
  dt->Insert(new G4MuAtomDIOChannel(muatom->GetParticleName(), 1.));
  muatom->SetMuAtomDecayTable(dt);
  ckt = new G4MuAtomCaptureKineticsTable(muatom); // FIXME leak
  ckt->Insert(new G4MuTCaptureChannel(muatom));
  muatom->CaptureKineticsTable(ckt);

  // mu_He3
  muatom = G4MuAtom::MuAtom(2,3);
  if (verboseLevel>0) G4cout << muatom->GetParticleName() << G4endl;
  dt = new G4MuAtomDecayTable(); // FIXME leak
  dt->Insert(new G4MuAtomDIOChannel(muatom->GetParticleName(), 1.));
  muatom->SetMuAtomDecayTable(dt);
  ckt = new G4MuAtomCaptureKineticsTable(muatom); // FIXME leak
  ckt->Insert(new G4MuHe3ProtonChannel(muatom, 1.e6/(2197.*CLHEP::ns)));
  ckt->Insert(new G4MuHe3DeuteronChannel(muatom, 1./(2197.*CLHEP::ns)));
  ckt->Insert(new G4MuHe3TritonChannel(muatom, 1./(2197.*CLHEP::ns)));
  muatom->CaptureKineticsTable(ckt);

  // mu_He4
  muatom = G4MuAtom::MuAtom(2,4);
  if (verboseLevel>0) G4cout << muatom->GetParticleName() << G4endl;
  dt = new G4MuAtomDecayTable(); // FIXME leak
  dt->Insert(new G4MuAtomDIOChannel(muatom->GetParticleName(), 1.));
  muatom->SetMuAtomDecayTable(dt);
  ckt = new G4MuAtomCaptureKineticsTable(muatom); // FIXME leak
  ckt->Insert(new G4MuHe4ProtonChannel(muatom));
  ckt->Insert(new G4MuHe4DeuteronChannel(muatom));
  ckt->Insert(new G4MuHe4TritonChannel(muatom));
  muatom->CaptureKineticsTable(ckt);

  // Generic MuAtom
  muatom = G4GenericMuAtom::GenericMuAtom();
  if (verboseLevel>0) G4cout << muatom->GetParticleName() << G4endl;
  dt = new G4MuAtomDecayTable(); // FIXME leak
  dt->Insert(new G4MuAtomDIOChannel("GenericMuAtom", 1.) );
  muatom->SetMuAtomDecayTable(dt);
  ckt = new G4MuAtomCaptureKineticsTable(muatom);  // FIXME leak
  ckt->Insert(new G4MuAtomGenericCaptureChannel(muatom));
  ckt->Insert(new G4Mu2eConversionChannel(muatom, 1./(2197.*CLHEP::ns))); //FIXME constant
  muatom->CaptureKineticsTable(ckt);

  // FIXME remove this crosscheck
  //   if (verboseLevel>0) {
  //     no process managers at this stage yet
  //     G4ProcessManager* pmanager = muatom->GetProcessManager();

  //     if ( pmanager == 0) {
  //       // no process manager
  //       // FIXME use #ifdef G4VERBOSE ?
  //       if (verboseLevel>0){
  //         G4cout <<"G4MuonMinusCapturePhysics::ConstructProcess"
  //                <<" : No Process Manager for "
  //                << muatom->GetParticleName() 
  //                << G4endl;
  //       }
  //     }

  //     G4ProcessVector const* pVector = pmanager->GetProcessList();

  //     G4cout << "G4MuonMinusCapturePhysics::ConstructParticle" << " " 
  //            << muatom->GetParticleName() << " processes: " << G4endl;
  //     for( G4int j=0; j<pmanager->GetProcessListLength(); ++j ) {
  //       G4VProcess* proc = (*pVector)[j];
  //       G4String const & name  = proc->GetProcessName();
  //       G4cout << "G4MuonMinusCapturePhysics::ConstructParticle " << name << G4endl;
  //     }
  //   }

  // P_mu_P
  G4MuMolecule *mumol = G4MuMolecule::Definition(1,1,1,1);
  if (verboseLevel>0) G4cout << mumol->GetParticleName() << G4endl;
  //  G4MuMoleculeCaptureKineticsTable *molckt = 
  //    new G4MuMoleculteCaptureKineticsTable(mumol);
  //  mumol->CaptureKineticsTable(molckt);

  // D_mu_D
  mumol = G4MuMolecule::Definition(1,2,1,2); // FIXME this leaks,
                                             // P_mu_P not inserted ???
  if (verboseLevel>0) G4cout << mumol->GetParticleName() << G4endl;
  G4MuMoleculeCaptureKineticsTable *molckt =
    new G4MuMoleculeCaptureKineticsTable(mumol); // FIXME leak
  molckt->Insert( new G4DMuDFusionHe3Channel(mumol,1/(2197.*CLHEP::ns)) );
  molckt->Insert( new G4DMuDFusionMuHe3Channel(mumol,1/(2197.*CLHEP::ns)) );
  molckt->Insert( new G4DMuDFusionTChannel(mumol,1/(2197.*CLHEP::ns)) );
  molckt->Insert( new G4DMuDFusionMuTChannel(mumol,1/(2197.*CLHEP::ns)) );
  molckt->Insert( new G4DMuDFusionHe4Channel(mumol,1/(2197.*CLHEP::ns)) );
  mumol->CaptureKineticsTable(molckt);

}

void G4MuonMinusCapturePhysics::ConstructProcess()
{

  //FIXME
  // use (or not) ???
  // G4PhysicsListHelper* ph = G4PhysicsListHelper::GetPhysicsListHelper();

  // FIXME use #ifdef G4VERBOSE ?
  if(verboseLevel > 1) G4cout << "### G4MuonMinusCapturePhysics::ConstructProcess " 
                              << processesConstructed << G4endl;
  if(processesConstructed) return;
  processesConstructed = true;

  // FIXME how to deal with this... ??? Assert or so??? to make sure it is created?
  // emphysics->ConstructProcess(); // does this create process managers e.g. for muons? NO

  //FIXME does one need to register those processes? is it done by/in G4TNuclearCapture

  // it looks like it is since the G4TNuclearCapture constructor calls:
  //  G4HadronicProcessStore::Instance()->RegisterExtraProcess(this);


  // FIXME use some sort of verboseLevel in SetVerboseLevel below

#if G4VERSION<4099
  if(verboseLevel > 1) G4cout << "### G4MuonMinusCapturePhysics::ConstructProcess theParticleIterator " 
                              << theParticleIterator << G4endl;

  theParticleIterator->reset();

  while( (*theParticleIterator)() ) {
    G4ParticleDefinition* particle = theParticleIterator->value();
#else
  if(verboseLevel > 1) G4cout << "### G4MuonMinusCapturePhysics::ConstructProcess aParticleIterator " 
                              << aParticleIterator << G4endl;

  aParticleIterator->reset();

  while( (*aParticleIterator)() ) {
    G4ParticleDefinition* particle = aParticleIterator->value();
#endif

    G4ProcessManager* pmanager = particle->GetProcessManager();

    G4String pNameToRemove("Decay");

    if ( pmanager == 0) {
      // no process manager
      // FIXME use #ifdef G4VERBOSE ?
      if (verboseLevel>0){
        G4cout <<"G4MuonMinusCapturePhysics::ConstructProcess"
               <<" : No Process Manager for "
               << particle->GetParticleName() 
               << G4endl;
      }

      G4Exception("G4MuonMinusCapturePhysics::ConstructProcess",
                  "MMAC0002", FatalException, 
                  "No process manager");
 
    }
    G4String const & particleName = particle->GetParticleName();

    (verboseLevel>1) &&
      G4cout <<"G4MuonMinusCapturePhysics::ConstructProcess"
             <<" : Working on  "
             << particleName
             << G4endl;
    
    if( particleName == "mu-" ){
      // remove muMinusCaptureAtRest if present
      G4String pNameToRemove("muMinusCaptureAtRest");
      G4ProcessVector* pVector = pmanager->GetProcessList();
      for( G4int j=0; j<pmanager->GetProcessListLength(); ++j ) {
        G4VProcess* proc = (*pVector)[j];
        G4String const & name  = proc->GetProcessName();
        if( name == pNameToRemove )  {
          if (verboseLevel>0) {
            G4cout << "G4MuonMinusCapturePhysics::ConstructProcess" 
                   << " Removing " << name << " from " << particleName << G4endl;
            pmanager->RemoveProcess(proc);
            break;
          }
        }
      }
      muproc = new G4MuonMinusAtomicCapture(); // FIXME leak
      pmanager->AddProcess(muproc);
      pmanager->SetProcessOrdering(muproc, idxAtRest);
    }
    
    if( particleName == "GenericMuAtom" ){
      G4MuAtomDecay* decay = new G4MuAtomDecay(); // FIXME leak
      if (verboseLevel>0) G4cout << "G4MuonMinusCapturePhysics::ConstructProcess Attach " << decay->GetProcessName() <<" to "<< particleName << G4endl;
      decay->SetVerboseLevel(verboseLevel);
      pmanager ->AddProcess(decay);
      pmanager ->SetProcessOrdering(decay, idxPostStep);
      pmanager ->SetProcessOrdering(decay, idxAtRest);
      // FIXME we need to remove Decay if present; this imposes constrait on the ordering wrt 
      // RegisterPhysics( new G4DecayPhysics(ver) )

      G4ProcessVector const* pVector = pmanager->GetProcessList();
      for( G4int j=0; j<pmanager->GetProcessListLength(); ++j ) {
        G4VProcess* proc = (*pVector)[j];
        G4String const & name  = proc->GetProcessName();
        if( name == pNameToRemove ) {
          if (verboseLevel>0) G4cout << "G4MuonMinusCapturePhysics::ConstructProcess" 
                                     << " Removing " << name << " from " 
                                     << particleName << G4endl;
          pmanager->RemoveProcess(proc);
          break;
        }          
      }

      // FIXME leak ???
      G4MuAtomNuclearCapture * capture = new G4MuAtomNuclearCapture();
      if (verboseLevel>0) G4cout << "G4MuonMinusCapturePhysics::ConstructProcess Attach " << capture->GetProcessName() <<" to "<< particleName << G4endl;
      capture->SetVerboseLevel(verboseLevel);
      pmanager->AddProcess(capture);
      pmanager->SetProcessOrdering(capture, idxPostStep);
      pmanager->SetProcessOrdering(capture, idxAtRest);

      // FIXME remove this crosscheck
      if (verboseLevel>0) {
        G4ProcessVector const* pVector = pmanager->GetProcessList();

        G4cout << "G4MuonMinusCapturePhysics::ConstructProcess" << " " 
               << particleName << " processes: " << G4endl;
        for( G4int j=0; j<pmanager->GetProcessListLength(); ++j ) {
          G4VProcess* proc = (*pVector)[j];
          G4String const & name  = proc->GetProcessName();
          G4cout << "G4MuonMinusCapturePhysics::ConstructProcess " << name << G4endl;
        }
      }

    }

    // FIXME are those leaks??? does one need to split it or have a
    // container for G4MuAtomNuclearCapture's?

    if( particleName == "mu_P" ||
        particleName == "mu_P_1" ||
        particleName == "mu_D" ||
        particleName == "mu_T" ||
        particleName == "mu_He3" ||
        particleName == "mu_He4" ){

      G4ProcessVector const* pVector = pmanager->GetProcessList();
      for( G4int j=0; j<pmanager->GetProcessListLength(); ++j ) {
        G4VProcess* proc = (*pVector)[j];
        G4String const & name  = proc->GetProcessName();
        if( name == pNameToRemove ) {
          if (verboseLevel>0) G4cout << "G4MuonMinusCapturePhysics::ConstructProcess" 
                                     << " Removing " << name << " from " 
                                     << particleName << G4endl;
          pmanager->RemoveProcess(proc);
          break;
        }          
      }

      // FIXME leak ???
      G4MuAtomNuclearCapture * capture = new G4MuAtomNuclearCapture();
      if (verboseLevel>0) G4cout << "G4MuonMinusCapturePhysics::ConstructProcess Attach " << capture->GetProcessName() <<" to "<< particleName << G4endl;
      capture->SetVerboseLevel(verboseLevel);
      pmanager->AddProcess(capture);
      pmanager->SetProcessOrdering(capture, idxPostStep);
      pmanager->SetProcessOrdering(capture, idxAtRest);
    }

    if( particleName == "D_mu_D" ){

      G4ProcessVector const* pVector      = pmanager->GetProcessList();
      for( G4int j=0; j<pmanager->GetProcessListLength(); ++j ) {
        G4VProcess* proc = (*pVector)[j];
        G4String const & name  = proc->GetProcessName();
        if( name == pNameToRemove ) {
          if (verboseLevel>0) G4cout << "G4MuonMinusCapturePhysics::ConstructProcess" 
                                     << " Removing " << name << " from " 
                                     << particleName << G4endl;
          pmanager->RemoveProcess(proc);
          break;
        }          
      }

      // FIXME leak ???
      G4MuMoleculeNuclearCapture * capture = new G4MuMoleculeNuclearCapture();
      capture->SetVerboseLevel(verboseLevel);
      if (verboseLevel>0) G4cout << "G4MuonMinusCapturePhysics::ConstructProcess Attach " << capture->GetProcessName() <<" to "<< particleName << G4endl;
      pmanager->AddProcess(capture);
      pmanager->SetProcessOrdering(capture, idxPostStep);
      pmanager->SetProcessOrdering(capture, idxAtRest);
    }
  }
}