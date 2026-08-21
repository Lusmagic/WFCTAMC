#ifndef __WFCTAMCEVENT_HH__
#define __WFCTAMCEVENT_HH__
#include "TObject.h"
#include <vector>
#include <map>
using namespace std;

class WFCTAMcEvent : public TObject {
public:
  int iEvent;
  int iUse;
  double id;
  double energy;
  double zenith, azimuth;
  double corex, corey;
  double Xmax, Nmax;
  int ntel;
  vector<double> TelX;
  vector<double> TelY;
  vector<double> TelA;
  vector<double> TelZ;
  vector<double> TubeSignalInTriggerWindow;
  vector<double> TubeTriggerTime;
  vector<int> TubeTrigger;
  vector<int> TelTrigger;
  vector<int> TubeID;
//  vector<float> photon_total;

//  vector<double> cphotons_4;
  WFCTAMcEvent();
  ~WFCTAMcEvent();
  void InitEvent();
  ClassDef(WFCTAMcEvent,1)
};

#endif

