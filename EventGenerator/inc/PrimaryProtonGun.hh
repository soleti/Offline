#ifndef EventGenerator_PrimaryProtonGun_hh
#define EventGenerator_PrimaryProtonGun_hh
//
// Generate a proton with the primary proton energy
//
// $Id: PrimaryProtonGun.hh,v 1.15 2013/05/02 19:18:24 rhbob Exp $
// $Author: rhbob $
// $Date: 2013/05/02 19:18:24 $
//
//
// The coordinate system used in this class is:
//
// 1) The origin is at the upstream face of the production target.
//    (upstream in the sense of the proton beam).
//
// 2) The positive z direction is along the cylinder axis of the production target,
//    in the usual Mu2e sense (from PS toward DS); ie the ideal beam direction
//    is in the -z direction.  The target is slightly tilted from the Mu2e z axis.
//    The x and y axes are in the plane of the upstream face of the production target;
//    the sense of these axes is defined by the rotation matrix that was used to position
//    the production target inside the PS vacuum volume; see the section titled
//    "Production Target" inside Mu2e-doc-938.
//
// 3) The transformation from this coordinate system to the G4 world coordinate
//    system is done in Mu2eG4/src/PrimaryGeneratorAction.cc .
//

// Mu2e includes
#include "EventGenerator/inc/GeneratorBase.hh"

// Framework Includes
#include "art/Framework/Principal/Run.h"

// Forward references outside of namespace mu2e
class TH1D;
namespace art {
  class Run;
}

namespace mu2e {

  // Forward reference.
  class SimpleConfig;

  class PrimaryProtonGun: public GeneratorBase{

  public:
    PrimaryProtonGun( art::Run& run, const SimpleConfig& config );
    virtual ~PrimaryProtonGun();

    virtual void generate( GenParticleCollection&  );

  private:

    double _proton_mass;

    // Start parameters from the run time configuration.

    // Momentum of the generated proton; in MeV.
    double _p;

    // Offset of production point relative to the origin described above; in mm.
    CLHEP::Hep3Vector _beamDisplacementOnTarget;

    // Rotation of beam direction wrt to target angle; in deg.
    double _beamRotationTheta;
    double _beamRotationPhi;
    double _beamRotationPsi;

    // Beamspot is a 2D gaussian with this sigma in both x and y.
    double _beamSpotSigma;

    // Limits on the generated direction; generated over a unit sphere within these limits.
    double _czmin;
    double _czmax;
    double _phimin;
    double _phimax;

    // Time of generation is a flat distribution within these limits. Time in ns.
    double _tmin;
    double _tmax;

    // Shape
    std::string _shape;

    // radius max, for flat distribution
    double _rmax;

    // Make histograms or not.
    bool _doHistograms;

    // End parameters from the run time configuration.

    // Histograms.
    TH1D* _hKE;
    TH1D* _hKEZoom;
    TH1D* _hmomentum;
    TH1D* _hposx;
    TH1D* _hposy;
    TH1D* _hposz;
    TH1D* _hcosTheta;
    TH1D* _htime;
  };

} // end namespace mu2e,

#endif /* EventGenerator_PrimaryProtonGun_hh */


