#include "RecoDataProducts/inc/KalSeed.hh"
#include "RecoDataProducts/inc/HelixSeed.hh"
#include "canvas/Persistency/Common/Assns.h"
namespace mu2e {
  // Assns between a KalSeed and the HelixSeed it was created from
  typedef art::Assns<KalSeed,HelixSeed> KalHelixAssns;
}
