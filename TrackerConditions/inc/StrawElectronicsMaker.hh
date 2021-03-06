#ifndef TrackerConditions_StrawElectronicsMaker_hh
#define TrackerConditions_StrawElectronicsMaker_hh

//
// construct a StrawElectronics conditions entity
// from fcl or database
//

#include "TrackerConditions/inc/StrawElectronics.hh"
#include "TrackerConfig/inc/StrawElectronicsConfig.hh"
#include "DAQConditions/inc/EventTiming.hh"
#include "DbTables/inc/TrkDelayPanel.hh"
#include "DbTables/inc/TrkDelayRStraw.hh"
#include "DbTables/inc/TrkPreampStraw.hh"


namespace mu2e {

  class StrawElectronicsMaker {
  public:
    StrawElectronicsMaker(StrawElectronicsConfig const& config):_config(config) {}
    StrawElectronics::ptr_t fromFcl(EventTiming::cptr_t eventTiming);
    StrawElectronics::ptr_t fromDb(TrkDelayPanel::cptr_t tdp,
				   TrkDelayRStraw::cptr_t tdrs,
				   TrkPreampStraw::cptr_t tps,
                                   EventTiming::cptr_t eventTiming);
  
  private:

    // this object needs to be thread safe, 
    // _config should only be initialized once
    const StrawElectronicsConfig _config;

  };
}


#endif

