#ifndef Mu2eG4_StrawSD_hh
#define Mu2eG4_StrawSD_hh
//
// Define a sensitive detector for Straws.
//
// $Id: StrawSD.hh,v 1.16 2013/01/07 04:05:00 kutschke Exp $
// $Author: kutschke $
// $Date: 2013/01/07 04:05:00 $
//
// Original author Rob Kutschke
//

// Mu2e includes
#include "Mu2eG4/inc/EventNumberList.hh"
#include "MCDataProducts/inc/StepPointMCCollection.hh"
#include "Mu2eG4/inc/Mu2eSensitiveDetector.hh"
#include "TTrackerGeom/inc/SupportModel.hh"

// Art includes
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Framework/Principal/Event.h"

class G4Step;
class G4HCofThisEvent;

namespace mu2e {

  class StrawSD : public Mu2eSensitiveDetector{

  public:
    StrawSD(const G4String, SimpleConfig const & config);

    G4bool ProcessHits(G4Step*, G4TouchableHistory*);

  private:

    G4ThreeVector GetTrackerOrigin(const G4TouchableHandle & touchableHandle);

    int _nStrawsPerDevice;
    int _nStrawsPerSector;
    int _TrackerVersion;

    SupportModel _supportModel;

  };

} // namespace mu2e

#endif /* Mu2eG4_StrawSD_hh */
