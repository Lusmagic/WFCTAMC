#include <iostream>
#include <fstream>
#include <string>

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "TMath.h"
#include "TFile.h"
#include "TTree.h"
#include "TRandom.h"
#include "creadparam.h"
#include "wtelescope.h"
#include "wcamera.h"
#include "telescopeparameters.h"
#include "cer_event.h"
#include "SquareCone.h"
#include "WFCTAMcEvent.h"
#include "TNtuple.h"
using namespace std;
//float ALTITUDE=440000.;
void SetAtmModel(int model, float ol);
float atm(float wavelength, float height, float theta );
void attenu(float wavelength, float height, float obslev, float theta,
            int atm_model, float *tr_atmos, double *AM);
main(int argc, char *argv[])
{
  /*parameters read from the input card */
  int NSBFlag,WLFlag,WLFormat, ThinFlag, FilterFlag,MirrorPointErrorFlag, FadcFlag,MirrorGeometry;
  int CTNumber;
  float MirrorSizeX, MirrorSizeY, MirrorPointError;
  char inputfilename[PATH_MAX_LENGTH];//="root://eos01.ihep.ac.cn/";
  char outputfilename[PATH_MAX_LENGTH];

  int PIX =  32;
  int NSIPM = PIX*PIX;
  int Fadc_bins, Fadc_length; //The number of FADC bins and the bin length 

  /*spot settings of each telescope */
  float *MirrorSpot;

  /*night sky background settings of each telescope */
  float *nsb, *triggersigma;
  float *InnerFixTriggerThreshold, *OuterFixTriggerThreshold;  //the trigger threshold for SiPM
 
  /*The telescope arrays settings*/
  float *CT_X, *CT_Y, *CT_Z, *CT_Azi, *CT_Zen;

  float *timefirst, *timelast,*timemean, *ncphoton;

  /*cos directions of the pointing of telescopes */
  float *CT_m, *CT_n, *CT_l;

  /*cos directions of the primary directions of showers */
  float prim_m, prim_n, prim_l;

  /*space angle between the arriving directions of showers and the pointing of the telescopes */
  float *Space_angle;

  float zenith, azimuth, Xmax, Nmax;
  
  float *CoreX, *CoreY;
  double  mm, nn;

  // * ==== iuse: for the multiple use of CER event ==== *//
  int Nuse = 1;  ///< 20 is the max times of CER event use
  int iuse = 0;
  int iRun;

  //*=== cone tracing and SiPM simulation ====*//
  int ict, icone, itube, icell,ix, iy, iternum, flag;
  double x, y, deltax, deltay,xx;
  double hitpos[3], dircos[3], Weight;

  WFCTAMcEvent *wfcta = new WFCTAMcEvent();

  //*Get the parameters from the input card*//
  WReadConfig *readconfig = new WReadConfig();  

  readconfig->readparam(argv[1]);
  WLFlag = readconfig->GetWLFlag();
  WLFormat = readconfig->GetWLFormat();
  ThinFlag = readconfig->GetThinFlag();
  FilterFlag = readconfig->GetFilterFlag();

  MirrorGeometry = readconfig->GetMirrorGeometry();
  MirrorPointErrorFlag = readconfig->GetMirrorPointErrorFlag();
  MirrorPointError = readconfig->GetMirrorPointError();

  readconfig->GetMirrorSize(&MirrorSizeX , &MirrorSizeY);

  CTNumber =readconfig-> GetCTNumber();
  wfcta->ntel = CTNumber;

  MirrorSpot = new float[CTNumber];

  nsb = new float [CTNumber];
  triggersigma = new float [CTNumber]; 
  InnerFixTriggerThreshold = new float [CTNumber];
  OuterFixTriggerThreshold = new float [CTNumber];

  CT_X = new float[CTNumber];
  CT_Y = new float[CTNumber];
  CT_Z = new float[CTNumber];

  CT_Azi = new float[CTNumber];
  CT_Zen = new float[CTNumber];
  CT_m = new float[CTNumber];
  CT_n = new float[CTNumber];
  CT_l = new float[CTNumber];
  Space_angle = new float[CTNumber];

  CoreX = new float[CTNumber];   //added by linglingMa on 2018.03.05
  CoreY = new float[CTNumber];   //To store the core positions of each events

  timefirst = new float[CTNumber];
  timelast = new float[CTNumber];
  timemean = new float[CTNumber];
  ncphoton = new float[CTNumber];

  //for test the raytracing of c photons 
  float *cphotons_0,*cphotons_1,*cphotons_2,*cphotons_3,*cphotons_4,*cphotons_5,*cphotons_6;
  cphotons_0 = new float[CTNumber]; 
  cphotons_1 = new float[CTNumber];
  cphotons_2 = new float[CTNumber];
  cphotons_3 = new float[CTNumber];
  cphotons_4 = new float[CTNumber];
  cphotons_5 = new float[CTNumber];
  cphotons_6 = new float[CTNumber];

  float* photon_total;
  photon_total = new float[CTNumber * NSIPM];

//  memset(photon_total, 0, sizeof(float) * CTNumber * NSIPM);

  NSBFlag = readconfig->GetNSBFlag();

  for (int ict = 0; ict <CTNumber; ict++){

     CT_X[ict] =  readconfig->GetCTPosition(ict,0); 
     CT_Y[ict] =  readconfig->GetCTPosition(ict,1);
     CT_Z[ict] =  readconfig->GetCTPosition(ict,2);
     CT_Zen[ict] =  readconfig->GetCTPosition(ict,3) * TMath::DegToRad(); 
     CT_Azi[ict] =  readconfig->GetCTPosition(ict,4) * TMath::DegToRad();
     CT_m[ict] = sin(CT_Zen[ict]) * cos(CT_Azi[ict]);
     CT_n[ict] = sin(CT_Zen[ict]) * sin(CT_Azi[ict]);
     CT_l[ict] = cos(CT_Zen[ict]);
     wfcta->TelX.push_back(CT_X[ict]);
     wfcta->TelY.push_back(CT_Y[ict]);
     wfcta->TelA.push_back(CT_Azi[ict]);
     wfcta->TelZ.push_back(CT_Zen[ict]);

     /*Settings of each telescope Added on 2020-5-13 by lingling ma*/ 
     MirrorSpot[ict] = readconfig->GetMirrorSpot(ict);
     nsb[ict] = readconfig->GetNSB(ict);


     if(NSBFlag){
        triggersigma[ict] = readconfig->GetTriggerSigma(ict); 
     }
     else{
        InnerFixTriggerThreshold[ict] = readconfig->GetInnerFixTriggerThreshold(ict);
        OuterFixTriggerThreshold[ict] = readconfig->GetOuterFixTriggerThreshold(ict);

     }

  }


  //*The total intensty of nsb in the trigger window equlas Fadc_bins X Fabs_length X nsb *//
  FadcFlag = readconfig->GetFadcFlag(); 
  Fadc_bins = readconfig->GetFadcBins();
  Fadc_length = readconfig->GetFadcLength();

  int AtmModel = readconfig->GetAtmModel();
  float ALTITUDE = readconfig->GetAltitude();

 
  strcpy( inputfilename, argv[2]);
  strcpy( outputfilename, argv[3]);

  WTelescope **telescope;
  telescope = new WTelescope * [CTNumber];
 
  //* Init the telescope array *//
  for(int ict=0; ict<CTNumber; ict++){

     telescope[ict] = new WTelescope();
printf("Geometry Settings of NO. %d CT:\n\n",ict);
     telescope[ict]->SetMirrorSpot(MirrorSpot[ict]);
     telescope[ict]->SetMirrorGeometry(MirrorGeometry);
     telescope[ict]->SetMirror();
     telescope[ict]->SetPointing(CT_Zen[ict],CT_Azi[ict]);
     telescope[ict]->SetMirrorPointError(MirrorPointErrorFlag,MirrorPointError);
     telescope[ict]->SetEulerMatrix(CT_Zen[ict],CT_Azi[ict]);
    
     telescope[ict]->SetReflectivity(ict); 
     telescope[ict]->SetTransmissivity(ict);

     telescope[ict]->SetPlanePoints();

  }

  //* to get the primary information of the showers *//
  CER_Event *Event; 
  Event = new CER_Event();
  
  //* to get the information of the Cherenkov photons *//
  CER_bunch *ph_bunch;
  ph_bunch = new CER_bunch();
  
  //*define the camera*//
  WCamera *camera = new WCamera();
  camera->SetSiPMMAP();
  camera->SetCTNumber(CTNumber);
  camera->Init();

printf("\n\n");
printf("NSB Settings:\n\n");
  for(int ict=0; ict<CTNumber; ict++){
      //** the option is added on 2017-10-30 by LinglingMa **//
      camera->SetNSB(ict,nsb[ict]*Fadc_length*Fadc_bins);
      printf("The NSB level of NO. %d CT is %f \n",nsb[ict]*Fadc_length*Fadc_bins);
       if(NSBFlag) {
          camera->SetTriggerSigma(ict,triggersigma[ict]);
          printf("TriggerSigma of NO. %d is %f\n",ict,triggersigma[ict]);
       }
      else {
         camera->SetFixTriggerThreshold(ict,InnerFixTriggerThreshold[ict],OuterFixTriggerThreshold[ict]);
         printf("InnterTriggerThreshold of NO. %d is %f\n",ict,InnerFixTriggerThreshold[ict]);
         printf("OuterTriggerThreshold of NO. %d is %f\n",ict,OuterFixTriggerThreshold[ict]);
      }
  }


  SquareCone *cone = new SquareCone();

  SetAtmModel(AtmModel,ALTITUDE);


  //*the outputfile *//

  
  TFile *file = TFile::Open(outputfilename,"recreate");
  //TFile *file = new TFile(outputfilename,"recreate");

 cout<<"outputfile is created" <<endl;

  //TNtuple *nt = new TNtuple("nt","dfr","t");
  TTree *EventsTree = new TTree("events","RayTrace");

  //EventsTree->Branch("iRun",&iRun,"iRun/I");
  //EventsTree->Branch("iEvent",&Event->Event_Number_,"iEvent/I");
  //EventsTree->Branch("iUse",&iuse,"iUse/I");
  //EventsTree->Branch("id",&Event->Primary_id_,"id/D");
  //EventsTree->Branch("energy",&Event->Primary_Energy_,"energy/D");
  //EventsTree->Branch("zenith", &Event->Primary_zenith_,"zenith/D");
  //EventsTree->Branch("azimuth", &Event->Primary_azimuth_, "azimuth/D");
  //EventsTree->Branch("corex", &Event->Primary_core_x_, "corex/D");
  //EventsTree->Branch("corey", &Event->Primary_core_y_, "corey/D");
  //EventsTree->Branch("Xmax",&Event->Xmax_,"Xmax/D");
  //EventsTree->Branch("Nmax",&Event->Nmax_,"Nmax/D");

 // EventsTree->Branch("ntel", &CTNumber,"ntel/I");
 // EventsTree->Branch("TelX", CT_X, Form("TelX[%d]/F",CTNumber) );
 // EventsTree->Branch("TelY", CT_Y, Form("TelY[%d]/F",CTNumber)  );
 // EventsTree->Branch("TelZ", CT_Zen,Form("TelZ[%d]/F",CTNumber) );
 // EventsTree->Branch("TelA", CT_Azi,Form("TelA[%d]/F",CTNumber) );
  //EventsTree->Branch("CoreX",CoreX,Form("CoreX[%d]/F",CTNumber));
  //EventsTree->Branch("CoreY",CoreY,Form("CoreY[%d]/F",CTNumber));


//  EventsTree->Branch("TubeSignalAfterConeTracing",&(camera->TubeSignalAfterConeTracing));
//  EventsTree->Branch("TubeSignalIntoCone", &(camera->TubeSignalIntoCone));
  EventsTree->Branch("TubeSignal", &(camera->TubeSignal_));
 // EventsTree->Branch("TubeTrigger", &(camera->TubeTrigger_));
 // EventsTree->Branch("TubeID", &(camera->TubeID_));
 // EventsTree->Branch("TelTrigger",&(camera->TelTrigger));
 // EventsTree->Branch("TubeSignalInTriggerWindow",&(camera->TubeSignalInTriggerWindow_));
 // EventsTree->Branch("TubeTriggerTime",&(camera->TubeTriggerTime_));
   EventsTree->Branch("wfcta",&wfcta);


  //to test
  // EventsTree->Branch("cphotons_0", cphotons_0, Form("cphotons_0[%d]/F",CTNumber) );
  // EventsTree->Branch("cphotons_1", cphotons_1, Form("cphotons_1[%d]/F",CTNumber) );
  // EventsTree->Branch("cphotons_2", cphotons_2, Form("cphotons_2[%d]/F",CTNumber) );
  // EventsTree->Branch("cphotons_3", cphotons_3, Form("cphotons_3[%d]/F",CTNumber) );
  // EventsTree->Branch("cphotons_4", cphotons_4, Form("cphotons_4[%d]/F",CTNumber) );
  // EventsTree->Branch("cphotons_5", cphotons_5, Form("cphotons_5[%d]/F",CTNumber) );
  // EventsTree->Branch("cphotons_6", cphotons_6, Form("cphotons_6[%d]/F",CTNumber) );

//   EventsTree->Branch("photon_total", photon_total, Form("photon_total[%d]/F",CTNumber) );
//   EventsTree->Branch("photon_total", photon_total, Form("photon_total[%d]/F",NSIPM) );
  //* To read the corsika file *// 

  EventsTree->Branch("photon_total",photon_total,Form("photon_total[%d]/F", CTNumber*NSIPM));
  //* ==== Particle, subblock, record length set based on THIN flag ==== *//
  int particlelength;
  if(ThinFlag) 
     particlelength = PARTICLE_LENGTH_THINNING;
  else
     particlelength = PARTICLE_LENGTH_NO_THINNING;

  int subblocklength = particlelength * N_PARTICLE;
  int recordlength = subblocklength * N_SUBBLOCK;

  
  union REC {
      float f;
      char c[5];
  };
  REC *rec = new REC [recordlength];

  union {
     int i;
     char c[4];
  } padding;

  char sss[5];

  int iblock = 0;
  int jrune = 0;
  int jrunh = 0;

  TFile *fin = TFile::Open(inputfilename);
  cout<<inputfilename << endl << endl;

  if (!fin->IsOpen()) {
     cout << "Error when open CER file " << inputfilename << endl << endl;
     exit(1);
  }

  fin->Seek(0,TFile::kBeg);
  size_t filesize = fin->GetSize(); 
  cout << "The CER file size is " << filesize << " bytes" << endl << endl;
  if (!filesize) {
     cout << "Warning: The file is empty!" << endl << endl;
     fin->Close();
     return(0);
  }

  fin->Seek(0,TFile::kBeg);
  char blockbuffer[4*recordlength];
  /* loop to read the corsika file */
  while (true) {
  //while (fin.good()) {

   //There are padding words before and after a record in the fortran output
  Bool_t read_stat; 
  if(jrune!=1)  read_stat = fin->ReadBuffer(padding.c, 4);
  if(read_stat) break; //20210518
 
  //if (!fin) {
  if(jrune==1){
     if (jrunh!=1||jrune!=1||iblock==0) {
        cout << "Error in reading corsika file \"" << inputfilename << "\"" << endl;
        cout << "Is the file truncated?" << endl;
        if (iblock==0) cout << "Is the file empty?" << endl;
        if (jrunh!=2) cout << "There is no RUNH sub-block!" << endl;
        if (jrune!=1) cout << "There is no RUNE sub-block!" << endl;
        cout << endl;
        exit(1);
     }
     else {
        //A successful reading would end here
        cout << "Succeed in reading corsika file \"" << inputfilename << "\"" << endl << endl;
        break;
     }
  }

  int iflag = 0;
  if (padding.i!=4*recordlength) iflag = 1;

  
  read_stat = fin->ReadBuffer(blockbuffer,4*recordlength);
  if(read_stat) break; //20210518  

  for(int iLength = 0;iLength<recordlength;iLength++){
      memcpy(rec[iLength].c, &blockbuffer[iLength*4],4);
      rec[iLength].c[4] = '\0';
  }

  if (!fin) iflag = 1;
 
  read_stat  = fin->ReadBuffer(padding.c,4);
  if(read_stat) break; //20210518


  if (padding.i!=4*recordlength) iflag = 1;

  if (iflag) {
      cout << "Error in reading corsika file \"" << inputfilename << "\"" << endl;
      cout << "Is the file truncated?" << endl;
      exit(1);
  }

  iblock++;

  //* ====== CORSIKA CER file data sub-block read ====== *//
  for(int isubblock  = 1 ;  isubblock <= N_SUBBLOCK ; isubblock++) {
    
      int iptr = subblocklength*(isubblock-1) - 1;
      memcpy(sss,&rec[iptr+1].f,4);
      sss[4] = '\0';
      //* ================ RUN HEADER ================ *//
      if (strcmp(sss,"RUNH")==0){
        iRun = int(rec[iptr+2].f);
         if (iuse++ == 0){
            jrunh++;
         }
      }
      //* ================ RUN END =================== *//
      else if (strcmp(sss,"RUNE")==0) {
           if(iuse == Nuse){
              jrune++;
           }
           else {
              fin ->Seek(0,TFile::kBeg);
              break;
          }
      }
     //* ================ EVENT HEADER ================ *//
      else if (strcmp(sss,"EVTH")==0) {

           Event->Init();
           wfcta->InitEvent();
           Event->Event_Number_ = int(rec[iptr+2].f);
           Event->Event_use_Number_ = iuse;
         
           Event->Primary_Energy_ = rec[iptr+4].f;
           Event->Primary_id_ = rec[iptr+3].f;

           Event->Primary_zenith_ = rec[iptr+11].f;
           Event->Primary_azimuth_ = rec[iptr+12].f;

           Event->Primary_core_x_ = rec[iptr+98+iuse].f;
           Event->Primary_core_y_ = rec[iptr+118+iuse].f;
           Event->Ob_level_ = rec[iptr+47+1].f;
      // printf("%f %f\n",Event->Primary_zenith_*57.3,Event->Primary_azimuth_*57.3);  
           prim_m = sin(rec[iptr+11].f) * cos( rec[iptr+12].f);
           prim_n = sin(rec[iptr+11].f) * sin( rec[iptr+12].f);
           prim_l = cos(rec[iptr+11].f);

           wfcta->iEvent=int(rec[iptr+2].f);
           wfcta->iUse = iuse;
           wfcta->id = rec[iptr+3].f;
           wfcta->energy = rec[iptr+4].f;
           wfcta->zenith = rec[iptr+11].f;
           wfcta->azimuth = rec[iptr+12].f;
           wfcta->corex = rec[iptr+98+iuse].f;
           wfcta->corey = rec[iptr+118+iuse].f;  
              
           //cout<<"CTNumber: "<<CTNumber<<endl;
           camera->ReSet();
           for(int ict=0; ict<CTNumber; ict++){
           //cout<<"ict: "<<ict<<endl;
              Space_angle[ict] = prim_m * CT_m[ict] + prim_n * CT_n[ict] + prim_l * CT_l[ict];
              if(Space_angle[ict]>1)  Space_angle[ict] = 1;
              if(Space_angle[ict]<-1) Space_angle[ict] = -1;
              Space_angle[ict] = acos(Space_angle[ict])*TMath::RadToDeg();
              //CoreX[ict] = Event->Primary_core_x_+CT_X[ict];
              //CoreY[ict] = Event->Primary_core_y_+CT_Y[ict];
              CoreX[ict] = CT_X[ict]; //yinlq for laser sim 202406
              CoreY[ict] = CT_Y[ict];
              telescope[ict]->SetXY(CoreX[ict],CoreY[ict]);
              timefirst[ict] = 1000000000;
              timelast[ict] = 0.;
              timemean[ict] = 0.;
              ncphoton[ict] = 0.;
              cphotons_0[ict] = 0.;
              cphotons_1[ict] = 0.;
              cphotons_2[ict] = 0.;
              cphotons_3[ict] = 0.;
              cphotons_4[ict] = 0.;
              cphotons_5[ict] = 0.; 
              cphotons_6[ict] = 0.;
	      for(int icone=0 ; icone<NSIPM ;icone++){
	      photon_total[ict*NSIPM+icone] = 0.;}
              // for test the decrease of c photons
           } //for

            
       }  

       //* ================ EVENT END ================ *//
       else if (strcmp(sss,"EVTE")==0) {
           printf("The end of the event\n");
           Event->Nmax_ = rec[iptr+255+1].f;
           Event->Xmax_ = rec[iptr+255+3].f;  
           wfcta->Xmax = rec[iptr+255+3].f;
           wfcta->Nmax = rec[iptr+255+1].f;           
                        

           camera->PhotonCellToTube();

           //** Added by lingling Ma on 2018-1-22             **//
           //** to Get the PeakTime on each SiPM              **//
           camera->GetPeakTime();   
           //** Added by linglingMa on 2018-3-13              **//
           //** to Get the photon ratio in the Trigger window **//    
           camera->GetPhotonInTriggerWindow(Fadc_bins);

          // if(NSBFlag){   
             camera->AddNSB(FadcFlag);
             camera->GetTubeTrigger(NSBFlag,FadcFlag);
          // }
           //else{
           //  camera->AddNSB(FadcFlag);
           //  camera->GetTubeTrigger(NSBFlag,FadcFlag);          
          // }
           camera->GetTubeTriggerTime(4,Fadc_bins); 
           camera->GetTelescopeTrigger(CTNumber,CT_Zen, CT_Azi); 
           camera->GetResult(wfcta);
           EventsTree->Fill();
       }

       //* ================ LONG sub-block ================ *//
       else if (strcmp(sss,"LONG")==0) {
       
       }
       //* ================ CERenkov sub-block ================ *//
       else {
          for (int iptcl=1; iptcl<=N_PARTICLE; iptcl++) {
         //  cout<<"iptcl: "<< iptcl<<endl; 
            int iptrnow = (iptcl-1) * particlelength + iptr;

            ph_bunch->Init();

            if(ThinFlag)
            //    ph_bunch->weight_ = rec[iptrnow+8].f; //yinlq for laser sim 20240606
            //else
                ph_bunch->weight_ = 1; 

            if(WLFlag){
                int wavelength_and_bunch = int(rec[iptrnow+1].f);

                if(WLFormat==0){
                   ph_bunch->wavelength_ = wavelength_and_bunch%1000;
                   ph_bunch->nclight_ = wavelength_and_bunch/1000;
                }
                if(WLFormat==1){
                  ph_bunch->wavelength_ = rec[iptrnow+8].f;
                  ph_bunch->nclight_ = rec[iptrnow+1].f;
                }
                if(WLFormat==2){
                  ph_bunch->wavelength_= int(rec[iptrnow+1].f);
                  ph_bunch->nclight_=100*(rec[iptrnow+1].f-int(rec[iptrnow+1].f));
                }
            }
            else
                ph_bunch->nclight_ = rec[iptrnow+1].f;

            if (ph_bunch->nclight_) {
               
               for(int ict=0; ict<CTNumber; ict++){
              //   cout<<"ict simulation "<< ict<<endl;     
                  //if(Space_angle[ict]>30) continue;  //yinlq for laser sim 20240606
 
                  //*=== Init the ph_bunch for each telescope*===//
                  //*=== debuged by Lingling Ma 2016.7.20*===//
                  ph_bunch->SetCERBunch(rec[iptrnow+2].f,rec[iptrnow+3].f,rec[iptrnow+4].f,
                                       rec[iptrnow+5].f,rec[iptrnow+6].f,rec[iptrnow+7].f); //x,y,u,v,t,h
                 
                  // to test
                  cphotons_0[ict] += int(ph_bunch->nclight_*ph_bunch->weight_); 
                  if(telescope[ict]->IncidentTel(ph_bunch->x_,ph_bunch->y_,MirrorSizeX,MirrorSizeY)){
                     // to test
                     cphotons_1[ict] += int(ph_bunch->nclight_*ph_bunch->weight_);
                     ph_bunch->IntoTelArea(telescope[ict]->Telx_,telescope[ict]->Tely_);

                     //*=== coordinate transformation to telescope system*===//
                     //*=== debuged by LinglingMa 2016.720 *===//
                     telescope[ict]->Euler(ph_bunch);
                    // if(ph_bunch->nclight_==1) ph_bunch->nclight_ = ph_bunch->nclight_;
                    // if(ph_bunch->nclight_>1) ph_bunch->nclight_= ph_bunch->nclight_+1;
                    //printf("%f %lf %lf \n",ph_bunch->nclight_,ph_bunch->weight_,ph_bunch->wavelength_);
                    double TotalLight = ph_bunch->nclight_*ph_bunch->weight_;
                     for(int Nclight=0; Nclight<int(TotalLight)+1; Nclight++){
                      if(Nclight==int(TotalLight)) {
                         if(gRandom->Rndm()>TotalLight-int(TotalLight)) {
                            //printf("******** %d %f\n",Nclight,TotalLight-int(TotalLight));
                            continue;
                         } 
                      } 
                      //if(FilterFlag){
                      //     int Transimissivity = telescope[ict] ->GetTransmissivity( ph_bunch->wavelength_);
                      //     if(Transimissivity==0) continue;
                      //}

                       
                      //float tr_atmos;
                      //double AM;
                      //float theta = rec[iptrnow+4].f*rec[iptrnow+4].f+rec[iptrnow+5].f*rec[iptrnow+5].f; 
                      //theta = sqrt(1-theta);
                      //theta = acos(theta);
		      //printf("**********test yinlq theta:%lf\n",theta*57.3);
                      //tr_atmos = atm(ph_bunch->wavelength_, rec[iptrnow+7].f, theta );
                      //attenu(ph_bunch->wavelength_,  rec[iptrnow+7].f, 440000,  theta, 0, &tr_atmos, &AM); 
                      //if(gRandom->Rndm()>tr_atmos) continue;
                      //if(gRandom->Rndm()>0.273) continue; //quanteff, yinlq for laser sim 20240606

                      if(telescope[ict]->Reflected(ph_bunch->wavelength_)){
                           //to test
                           cphotons_2[ict] += 1;

                           double temp = telescope[ict]->RayTrace(MirrorSpot[ict],
                                       -ph_bunch->y_*10, ph_bunch->x_*10, ph_bunch->z_*10, //update 20210413
                                       -ph_bunch->v_, ph_bunch->u_, ph_bunch->l_,
                                        &ph_bunch->xc_, &ph_bunch->yc_,&ph_bunch->time_raytrace_, &ph_bunch->u1_, &ph_bunch->v1_);
                
                           if(temp < 0) continue; 
                           cphotons_3[ict] += 1;
                           if(FilterFlag){
                                double filter_m = ph_bunch->u1_;
                                double filter_n = ph_bunch->v1_;
                                double filter_l = sqrt(1-filter_m*filter_m-filter_n*filter_n);
                                //double filter_angle = acos(filter_m*0+filter_n*0+filter_l*1)*TMath::RadToDeg();
                                double filter_angle = acos(filter_l)*TMath::RadToDeg();
                                int Transimissivity = telescope[ict] ->GetTransmissivity( ph_bunch->wavelength_,filter_angle);
                                if(Transimissivity==0) continue;
                                cphotons_4[ict] += 1;
                           }
                     
                           //*Get the time information of the cphotons *//  
                           ph_bunch->time_raytrace_ += rec[iptrnow+6].f;

                           timemean[ict] += ph_bunch->time_raytrace_;

                           if(ph_bunch->time_raytrace_>timelast[ict]) 
                              timelast[ict] = ph_bunch->time_raytrace_;

                           if(ph_bunch->time_raytrace_<timefirst[ict]) 
                              timefirst[ict] = ph_bunch->time_raytrace_;
                      
                           //*The photons that can enter the wenston cone*// 
                           icone = camera->GetCone( ict, ph_bunch->xc_, ph_bunch->yc_); 	
                           photon_total[ict * NSIPM + icone] +=1;
			   
			   if(icone<0) continue;
                           //photon_total[ict * NSIPM + icone] ++;

			   camera->PhotonIntoCone(ict,icone,1);
                           // to test 
                           cphotons_5[ict] += 1;
                           //*To Get the cone coordinates*//
                           x = camera->GetSiPMX(ict,icone);
                           y = camera->GetSiPMY(ict,icone); 

                           deltax =  ph_bunch->xc_ - x;
                           deltay =  ph_bunch->yc_ - y;
                           dircos[0] = -ph_bunch->u1_;
                           dircos[1] = ph_bunch->v1_;
                           dircos[2] = -sqrt(1-ph_bunch->u1_*ph_bunch->u1_-ph_bunch->v1_*ph_bunch->v1_);

                           //*The ray trace in the cone *//
                           cone -> SetInitDir(dircos);
                           cone -> SetInitPos(deltax,deltay);
                           cone -> SquareRaytracing();
                           cone -> GetHitPos(hitpos,Weight);
                           if(!cone -> GetStrike()) continue;
                           //ConeTracing(deltax, deltay,  x,  y,  dircos, hitpos, &iternum, &flag);
                           hitpos[0] += x;
                           hitpos[1] += y;
                           itube = camera->GetTube(ict,hitpos[0],hitpos[1]);
                           if(itube<0) continue;           //Throw away the bad point after ConeTracing.
                           if(itube!=icone) continue;
                           xx = gRandom->Rndm();
                           if(xx>Weight) continue;
                           //to test
                           cphotons_6[ict] += 1;
                           camera->PhotonAfterConeTracing(ict,icone,1);

                           hitpos[0] = hitpos[0] - x;
                           hitpos[1] = hitpos[1] - y;
                           ix = int((hitpos[0]+D_SiPM/2)/D_Cell);
                           iy = int((hitpos[1]+D_SiPM/2)/D_Cell);
                           icell = iy*NCell+ix;

                           // ** Added By linglingMa to count the arrive time of photons **//
                       //    if(Nclight!=int(ph_bunch->nclight_*ph_bunch->weight_)){
                                  camera->GetArriveTime(ict,itube,icell,int(ph_bunch->time_raytrace_/Fadc_length),1);
                                  camera->PhotonIntoCell(ict,itube,icell,1);
                       //    }
                       //   if(Nclight==int(ph_bunch->nclight_*ph_bunch->weight_)){
                       //           camera->GetArriveTime(ict,itube,icell,int(ph_bunch->time_raytrace_/Fadc_length),0.5);
                       //           camera->PhotonIntoCell(ict,itube,icell,0.5);
                       //    }
//printf("%d %d %d\n",ict,itube,icell);
 
                      }
                    } //loop for Nclight
                 } //if telescope
               } //for ict 
            }//if Nclight
          }// for Nparticle
//       cout<<"partilce end"<<endl;
       }// else cherenkov block

//cout<<" cherenkov block end"<<endl;
     }//sub block
//cout<<" sub block"<<endl;
   }  //while

   double p0=0;
   for (int ict = 0; ict < CTNumber; ict++) {
     for(int icone=0; icone<NSIPM; icone++){
       int index = ict * NSIPM + icone;
       p0 += photon_total[index];
     }
   }
   printf("p0=%.2f \n",p0);

   file->cd();
   file->Write();
   file->Close();
}
