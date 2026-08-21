#include "WFCTAMcEvent.h"

ClassImp(WFCTAMcEvent)

WFCTAMcEvent::WFCTAMcEvent()
{
}

WFCTAMcEvent::~WFCTAMcEvent()  {
}
void WFCTAMcEvent::InitEvent()
{
   TelTrigger.clear();
   TubeTrigger.clear();
   TubeTriggerTime.clear();
   TubeSignalInTriggerWindow.clear();
   TubeID.clear();
//   TelZ.clear();
//   TelA.clear();
//   TelX.clear();
//   TelY.clear();

}

