#include <iostream>
#include <fstream>
#include <string.h>

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <map>
#include "telescopeparameters.h"
#include "WFCTAMcEvent.h"
using namespace std;
class WCamera
{
  private:

  double SiPMMAP[NSIPM][2];
  float *NSB, *TriggerSigma;
  float *InnerFixTriggerThreshold,*OuterFixTriggerThreshold;
  //float MeanTime;

  int CTNumber;
  float matrix_[3][3];
  static double Dx[18];  

  public:

  //int **Trigger;
  //float **phe;
  vector<double> TubeSignalIntoCone;
  vector<double> TubeSignalAfterConeTracing;
  vector<double> TubeSignal;
  vector<int> TubeTrigger;
  vector<int> TelTrigger;
  vector<float>MeanTime;
  vector<double> TubeSignalInTriggerWindow;
  vector<int> TubeTriggerTime;

  vector<double> TubeSignal_;
  vector<int> TubeTrigger_;
  vector<int> TubeID_;
  vector<double> TubeSignalInTriggerWindow_;
  vector<int> TubeTriggerTime_;
  
  // ** Added by lingling Ma on 2018-1-22        ** //
  // ** to Get the mean arrive time on each SiPM ** //
  vector<float> PeakTime;
  
  map<int, map<int,map<int, bool> > > cell;
  map<int, map<int,map<int, bool> > >::iterator ct_iter;
  map<int, map<int,bool> >::iterator tube_iter;
  map<int,bool>::iterator cell_iter;
 
  // ** Added By Lingling Ma on 2018-1-22        **//
  // ** to count the arrive time of the photons  **// 
  map<int, map<int,map<int,float> > > ArriveTime;
  map<int, map<int,map<int,float> > >::iterator ct_time_iter;
  map<int, map<int,float> >::iterator tube_time_iter;
  map<int,float>::iterator time_time_iter;
  
   
  WCamera();
  ~WCamera();
  void SetSiPMMAP();
  void SetNSB(int ict, float nsb);
  void SetTriggerSigma(int ict,float triggersigma);
  //* Added on 2017-10-30 by Lingling Ma *//
  //if NSB is not simulted, the fix trigger threshold is used to trigger SiPM *//
  void SetFixTriggerThreshold( int ict,float innerfixtriggerthreshold, float outtriggerthreshold);
  void SetCTNumber(int ctnumber);
  int GetCone(int ict, double clusterx, double clustery); 
  int GetTube(int ict, double clusterx, double clustery);
  void Init();
  void ReSet();
  //void PhotonToTube(int ict, int itube, int outpe);
  void PhotonCellToTube();
  void PhotonIntoCone(int ict, int itube, float outpe);
  void PhotonAfterConeTracing(int ict, int itube, float outpe);
  void PhotonIntoCell(int ict, int itube, int icell, float outpe);
  void GetTubeTrigger(int nsbflag, int fadcflag);
  //void GetTrigger();
  void AddNSB(int fadcflag);
  double GetSiPMX(int ict, int itube);
  double GetSiPMY(int ict, int itube);
  void GetTelescopeTrigger(int CTNumber,float *CT_Zen, float *CT_AZi);
  void GetEulerMatrix(float TelZ,float TelA);
  void InverseEuler(double x0, double y0, double z0, double *x, double *y, double *z);
  // ** Added By lingling Ma on 2018-1-22        **//
  // ** to Get the mean arrive time on each SiPM **// 
  void GetArriveTime(int ict, int itube, int icell, int itime, float iphoton); 
  void GetPeakTime();
  void GetPhotonInTriggerWindow(double trigger_window);
  //void GetTubeTriggerTime(double fadclength,double trigger_window);
  void GetTubeTriggerTime(int fadcbins,double  trigger_bins);

  void GetResult(WFCTAMcEvent *wfcta);
};
