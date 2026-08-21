#include "wcamera.h"
#include "TMath.h"
#include "TRandom.h"
#include <set>
WCamera::WCamera()
{

}

WCamera::~WCamera()
{

}

// Updated By Chen Suhong
double WCamera::Dx[18] = {-0.24, 0.7, -0.04, -0.74, -0.18, -0.6, 0.12, 0.32, -0.18, 
				0.12, 0.3, -0.3, -0.06, 0.02, 0.06, 0.28, -0.04, 0.06 };

//*** Updated By You Zhiyong and Chen Suhong, the centerx and y are changed*** //
//gaps between subclusters are added //
void WCamera::SetSiPMMAP()
{
    double interval = 1.0; //mm, gaps between subclusters 
    int Interx,Intery;
    double centerx, centery;

    centerx = (32.5*D_ConeOut+7*interval)/2;
    centery = (32*D_ConeOut+7*interval)/2;
    for(int k=0;k<1024;k++){
        int i = k/32;
        int j = k%32;
        Intery = i/4;
        Interx = j/4;
        if(i%2==0)
            SiPMMAP[k][0] = (j+0.5)*D_ConeOut + interval*Interx-centerx;
        if(i%2==1)
            SiPMMAP[k][0]  = (j+1)*D_ConeOut + interval*Interx-centerx;
        
        SiPMMAP[k][1] =((PIX-i)*D_ConeOut + interval*(7-Intery)-centery)-D_ConeOut/2;   
       // SiPMMAP[k][0]=0.96*SiPMMAP[k][0];
       // SiPMMAP[k][1]=0.96*SiPMMAP[k][1];

        //SiPMMAP[k][0] = -SiPMMAP[k][0];
        //SiPMMAP[k][1] = (PIX-i)*D_ConeOut + interval*Intery-centery;
        // debugged by liuhu and youzhiyong  2021/1/7
        // SiPMMAP[k][1] = (PIX-i)*D_ConeOut + interval*(7-Intery)-centery;

    }

    for(int ict=0; ict<18; ict++) Dx[ict] = -1.*Dx[ict]*TMath::DegToRad()*FOCUS;

   // for(int k=0; k<1024; k++){

   //   SiPMMAP[k][0] = SiPMMAP[k][0];
   //   SiPMMAP[k][1] = SiPMMAP[k][1];
   // } 
}

/*void WCamera::SetSiPMMAP()
{
  int  k;
  for(int i=0; i<PIX; i++){
     for(int j=0; j<PIX; j++){
         k = i*PIX + j;
         if(i%2==0)
            SiPMMAP[k][0] = (j+0.5-PIX/2.0)*D_ConeOut;

         if(i%2==1)
            SiPMMAP[k][0] = (j+1-PIX/2.0)*D_ConeOut;

         SiPMMAP[k][1] = (PIX/2.0-i)*D_ConeOut - D_ConeOut/2.0;
     }
  }
}*/

double WCamera::GetSiPMX(int ict, int itube)
{
  return SiPMMAP[itube][0] - Dx[ict];
}

double WCamera::GetSiPMY(int ict, int itube)
{
  return SiPMMAP[itube][1];
}

int WCamera::GetCone(int ict, double clusterx, double clustery)
{
  double deltax, deltay;
  int  itube;

  itube = -100;
  for(int k=0; k<NSIPM; k++){
     deltax = clusterx-(SiPMMAP[k][0]-Dx[ict]);
     deltay = clustery-SiPMMAP[k][1];
     if(fabs(deltax)<D_ConeIn/2.0&&fabs(deltay)<D_ConeIn/2.0){  //the gap between PMT are considered
        itube = k;
        break;
     }
  }
  return itube;
}

int WCamera::GetTube(int ict, double clusterx, double clustery)
{
  double deltax, deltay;
  int itube;

  itube = -100; 
  for(int k=0; k<NSIPM; k++)
  {
    deltax = clusterx-(SiPMMAP[k][0]-Dx[ict]);
    deltay = clustery-SiPMMAP[k][1];
    if(fabs(deltax)<D_SiPM/2.0&&fabs(deltay)<D_SiPM/2.0)
    {  //the gap between PMT are considered
       itube = k;
       break;
    }
  }
  return itube;
}

void WCamera::SetNSB(int ict,float nsb)
{
   NSB[ict] = nsb;
printf("nsb itel %d is %f\n",NSB[ict]);
}

void WCamera::SetTriggerSigma(int ict ,float triggersigma)
{
  TriggerSigma[ict] = triggersigma;
}

void WCamera::SetFixTriggerThreshold(int ict,float innerfixtriggerthreshold, float outerfixtriggerthreshold)
{
   InnerFixTriggerThreshold[ict] = innerfixtriggerthreshold;
   OuterFixTriggerThreshold[ict] = outerfixtriggerthreshold;
}

void WCamera::SetCTNumber(int ctnumber)
{  
   CTNumber = ctnumber;
}


void WCamera::Init()
{
  //* added by lingling ma on 2020-5-13 *//
  //* to simulate the nsb for each telescope*//
  NSB = new float[CTNumber];
  InnerFixTriggerThreshold = new float[CTNumber];
  OuterFixTriggerThreshold = new float[CTNumber];
  TriggerSigma = new float[CTNumber];
 
  TubeSignal.resize(CTNumber*NSIPM);
  TubeTrigger.resize(CTNumber*NSIPM); 
  TubeSignalIntoCone.resize(CTNumber*NSIPM);
  TubeSignalAfterConeTracing.resize(CTNumber*NSIPM);
  TelTrigger.resize(CTNumber);
  MeanTime.resize(CTNumber);
  //** Added by linglingMa on 2018-1-22 **//
  //** To Get the arrive time on each SiPM **//
  PeakTime.resize(CTNumber*NSIPM);
  TubeSignalInTriggerWindow.resize(CTNumber*NSIPM);

  //**Added by linglingma on 2019-12-16**//
  //**To get the TriggerTime of each SiPM**//
  TubeTriggerTime.resize(CTNumber*NSIPM); 
}


void WCamera::ReSet()
{
   for(int i=0; i<NSIPM*CTNumber; i++){
      TubeSignal[i] = 0;
      TubeTrigger[i] = 0;
      TubeSignalIntoCone[i] = 0;
      TubeSignalAfterConeTracing[i] =0;
      //** Added by linglingMa on 2018-1-22 **//
      //** To Get the arrive time on each SiPM **//
      PeakTime[i] = 0;
      TubeSignalInTriggerWindow[i] = 0;  
      TubeTriggerTime[i] = 0;
   }
  for(int ict=0; ict<CTNumber; ict++){
    TelTrigger[ict] = 0;
    MeanTime[ict] = 0;
  }
   cell.clear();
   ArriveTime.clear();
   TubeTrigger_.clear();
   TubeID_.clear();
   TubeSignal_.clear();
   TubeSignalInTriggerWindow_.clear();
   TubeTriggerTime_.clear();
}


void WCamera::PhotonCellToTube()
{
  // TubeSignal[ict*NSIPM+itube]+= outpe ;
  int ict, itube;
  for(ct_iter=cell.begin();ct_iter!=cell.end();ct_iter++){
     ict = ct_iter->first;
     for(tube_iter=ct_iter->second.begin();tube_iter!=ct_iter->second.end();tube_iter++){
        itube = tube_iter->first;
        for(cell_iter=tube_iter->second.begin();cell_iter!=tube_iter->second.end();cell_iter++){
           TubeSignal[ict*NSIPM+itube] += cell_iter->second;
        } 
     }
  } 
}


void WCamera::GetPeakTime()
{
  //** Added by linglingMa on 2018-1-22 **//
  //** To Get the arrive time on each SiPM **//  
  int ict, itube;
  float maxpe;
  int n;
  for(ct_time_iter=ArriveTime.begin(); ct_time_iter!=ArriveTime.end(); ct_time_iter++){
     ict = ct_time_iter->first;
     for(tube_time_iter=ct_time_iter->second.begin();tube_time_iter!=ct_time_iter->second.end();tube_time_iter++){
        itube = tube_time_iter->first;
        maxpe=0;
        for(time_time_iter=tube_time_iter->second.begin();time_time_iter!=tube_time_iter->second.end();time_time_iter++){
           if(time_time_iter->second > maxpe) {
              maxpe = time_time_iter->second;
              PeakTime[ict*NSIPM+itube] = time_time_iter->first;
           }
        }  
    }
  }
  

}

void WCamera::GetPhotonInTriggerWindow(double trigger_window)
{ 
  int ict, itube;
  float delta_time;
  double Nphoton_in_triggerwindow;
  int n;
  for(ct_time_iter=ArriveTime.begin(); ct_time_iter!=ArriveTime.end(); ct_time_iter++){
     ict = ct_time_iter->first;
     for(tube_time_iter=ct_time_iter->second.begin();tube_time_iter!=ct_time_iter->second.end();tube_time_iter++){
        itube = tube_time_iter->first;
        Nphoton_in_triggerwindow = 0;
        if( PeakTime[ict*NSIPM+itube] ==0) TubeSignalInTriggerWindow[ict*NSIPM+itube] = 00;
        else{
           for(time_time_iter=tube_time_iter->second.begin();time_time_iter!=tube_time_iter->second.end();time_time_iter++){
              delta_time = time_time_iter->first - PeakTime[ict*NSIPM+itube];
              if(fabs(delta_time)<=trigger_window/2.){
                 Nphoton_in_triggerwindow+=time_time_iter->second;
              }
           }
           TubeSignalInTriggerWindow[ict*NSIPM+itube] = Nphoton_in_triggerwindow;

        }
     }
   } 

}

void WCamera::PhotonIntoCone(int ict,int itube, float outpe)
{  
   TubeSignalIntoCone[ict*NSIPM+itube]+= outpe ;
}

void WCamera::GetArriveTime(int ict, int itube,int icell, int itime,  float iphoton)
{
  if(cell[ict][itube][icell]==0) 
     ArriveTime[ict][itube][itime]+=iphoton; 
  else ArriveTime[ict][itube][itime] += 0;
}
void WCamera::PhotonIntoCell(int ict, int itube, int icell, float outpe)
{
   cell[ict][itube][icell] += outpe;
}

void WCamera::PhotonAfterConeTracing(int ict,int itube, float outpe)
{
   TubeSignalAfterConeTracing[ict*NSIPM+itube] += outpe;
}

void WCamera::AddNSB(int fadcflag)
{
  double nsb;
  for(int ict=0; ict<CTNumber; ict++){
     for(int itube=0; itube<NSIPM; itube++){
       nsb = gRandom->Poisson(NSB[ict]);
       if(fadcflag)  TubeSignalInTriggerWindow[ict*NSIPM+itube] +=int(nsb);
       else  TubeSignal[ict*NSIPM+itube] +=int(nsb);
     }
  }
}


void WCamera::GetTubeTrigger(int nsbflag, int fadcflag)
{
   float tubepe;
   if(nsbflag){
     for(int ict=0; ict<CTNumber; ict++){ 
        for(int itube=0; itube<NSIPM; itube++){
           if(fadcflag){
             tubepe = TubeSignalInTriggerWindow[ict*NSIPM+itube];
             TubeSignalInTriggerWindow[ict*NSIPM+itube] -=NSB[ict];   //Update By Lingling Ma 2019-7-1
           } 
           else {
             tubepe = TubeSignal[ict*NSIPM+itube];
             TubeSignal[ict*NSIPM+itube] -= NSB[ict];  //Update By Lingling Ma 2019-7-1
           }
           if((tubepe-NSB[ict])/sqrt(NSB[ict])>TriggerSigma[ict]) {
              TubeTrigger[ict*NSIPM+itube] = 1;
               
           }
           else TubeTrigger[ict*NSIPM+itube] = 0;
        }
     }
   }
   else{
      for(int ict=0; ict<CTNumber; ict++){
        for(int itube=0; itube<NSIPM; itube++){
           if(fadcflag){
             tubepe = TubeSignalInTriggerWindow[ict*NSIPM+itube];
             TubeSignalInTriggerWindow[ict*NSIPM+itube] -=NSB[ict];   //Update By Lingling Ma 2019-7-1         
           }
           else {
              tubepe = TubeSignal[ict*NSIPM+itube];
              TubeSignal[ict*NSIPM+itube] -=NSB[ict];   //Update By Lingling Ma 2019-7-1 
           }

           if(itube%32==31||itube%32==0||itube%32==30||itube%32==1){// edge SiPMs 
              if(tubepe>OuterFixTriggerThreshold[ict]) TubeTrigger[ict*NSIPM+itube] = 1;
              else TubeTrigger[ict*NSIPM+itube] = 0;
           }
           else{ //inner sipm
              if(tubepe>InnerFixTriggerThreshold[ict]) TubeTrigger[ict*NSIPM+itube] = 1;
              else TubeTrigger[ict*NSIPM+itube] = 0;
             
           }
        }
      }
     
   }

}

void WCamera::GetTubeTriggerTime(int fadcbins,double  trigger_bins)
{
 //  fadclength*=4;
  int n=0;
  for(int ict=0; ict<CTNumber; ict++){
      n = 0;
      MeanTime[ict] = 0;
  
      for(int itube=0; itube<NSIPM; itube++){
         if(TubeTrigger[ict*NSIPM+itube]==1){
              if(PeakTime[ict*NSIPM+itube]==0) continue;
              MeanTime[ict] += PeakTime[ict*NSIPM+itube]; 
              n++;
         }
      }
      if(n>0) MeanTime[ict]/=n;
  }
   for(int ict=0; ict<CTNumber; ict++){
      for(int itube=0; itube<NSIPM; itube++){
         if(TubeTrigger[ict*NSIPM+itube]==1){
            if(PeakTime[ict*NSIPM+itube]>0){
                 TubeTriggerTime[ict*NSIPM+itube]= int((PeakTime[ict*NSIPM+itube]-MeanTime[ict]+trigger_bins/2.));
                 //printf("%f %f\n",PeakTime[ict*NSIPM+itube],MeanTime[ict]);
                 //printf("******** %d %d \n",itube,TubeTriggerTime[ict*NSIPM+itube]);
            }
            else TubeTriggerTime[ict*NSIPM+itube]=gRandom->Rndm()*20;
         } 
         else{
            TubeTriggerTime[ict*NSIPM+itube]=gRandom->Rndm()*20;
         }
            
      }
   }
}

struct Dir{
  int ipmt;
  double u;
  double v;
  double w;

  //this line overloads '<' because the structure need be sorted in std::set!
   bool operator<(const Dir &a) const {return ipmt<a.ipmt;}
 };//This structure is only used below.


void WCamera::GetEulerMatrix(float TelZ,float TelA)
{
   float cosz, sinz, cosa, sina;
   cosz = cos(TelZ);
   sinz = sin(TelZ);
   cosa = cos(TelA);
   sina = sin(TelA);
   matrix_[0][0] = cosa*cosz;
   matrix_[0][1] = sina*cosz;
   matrix_[0][2] = -sinz;
   matrix_[1][0] = -sina;
   matrix_[1][1] = cosa;
   matrix_[1][2] = 0;
   matrix_[2][0] = cosa*sinz;
   matrix_[2][1] = sina*sinz;
   matrix_[2][2] = cosz;
}

void WCamera::InverseEuler(double x0, double y0, double z0, double *x, double *y, double *z)
{
   *x = matrix_[0][0]*x0+matrix_[1][0]*y0+matrix_[2][0]*z0;
   *y = matrix_[0][1]*x0+matrix_[1][1]*y0+matrix_[2][1]*z0;
   *z = matrix_[0][2]*x0+matrix_[1][2]*y0+matrix_[2][2]*z0;
}

void WCamera::GetTelescopeTrigger(int CTNumber,float *CT_Zen, float *CT_Azi)
{

  double u0,v0,w0;
  double u1,v1,w1;
  double x0,y0,z0;
  double COS = cos(0.6*TMath::DegToRad());
  int NTrigger;
  struct Dir direction;
  vector<struct Dir> dir_collection;
  set<struct Dir> Pattern;
  set<struct Dir>::iterator itor;
  int Trigger_single = 3;
  int Trigger_multiple = 3;
  int addpmt;

  //Get directions of all triggered tubes.   if it is triggered push (u,v,w) into the vector[itel]
  //Calculate trigger for each telescope 

  for(int iTel=0; iTel<CTNumber; iTel++)
  {
    GetEulerMatrix(CT_Zen[iTel],CT_Azi[iTel]);
    for(int ipmt = 0;ipmt<NSIPM;ipmt++)
    {
      // push all triggered pmts into a vector
      if(TubeTrigger[iTel*NSIPM+ipmt]==0) continue;
      x0=SiPMMAP[ipmt][0]-Dx[iTel];
      y0=SiPMMAP[ipmt][1];
      z0 = FOCUS;
      u0=-x0/sqrt(x0*x0+y0*y0+z0*z0);
      v0=-y0/sqrt(x0*x0+y0*y0+z0*z0);
      w0=z0/sqrt(x0*x0+y0*y0+z0*z0);
      InverseEuler(u0,v0,w0,&u1,&v1,&w1);
      direction.ipmt = iTel*NSIPM+ipmt;
      direction.u=u1;
      direction.v=v1;
      direction.w=w1;
      dir_collection.push_back(direction);

      // trigger for iTel
      Pattern.clear();
      Pattern.insert(direction);
      while(1)
      {
        addpmt = 0;
        for(int jpmt = 0;jpmt<NSIPM;jpmt++)
        {
           if(jpmt==ipmt||TubeTrigger[iTel*NSIPM+jpmt]==0) continue;
          x0=SiPMMAP[jpmt][0]-Dx[iTel];
          y0=SiPMMAP[jpmt][1];
          z0 = FOCUS;
          u0=-x0/sqrt(x0*x0+y0*y0+z0*z0);
          v0=-y0/sqrt(x0*x0+y0*y0+z0*z0);
          w0=z0/sqrt(x0*x0+y0*y0+z0*z0);
          InverseEuler(u0,v0,w0,&u1,&v1,&w1) ;
          direction.ipmt = iTel*NSIPM+jpmt;
          direction.u=u1;
          direction.v=v1;
          direction.w=w1;
          for(itor=Pattern.begin();itor!=Pattern.end();itor++)
          {
            if(itor->ipmt==jpmt) continue;
            if(itor->u*u1+itor->v*v1+itor->w*w1>COS)
            {
              if(Pattern.insert(direction).second)
              {
                addpmt++;
                break;
              }
            }
          } // for(itor=Pattern.begin();itor!=Pattern.end();itor++)
          if(Pattern.size()>=Trigger_single) break;
        }//  for(int jpmt = 0;jpmt<NPMT;jpmt++)
        if(addpmt==0||Pattern.size()>=Trigger_single) break;
      } //while(1)
     if(Pattern.size()>=Trigger_single) {TelTrigger[iTel]=1; break;}
      
    }//  for(int ipmt = 0;ipmt<NPMT;ipmt++)
  }//  for(int iTel=0; iTel<ntel; iTel++)
  for(int iTel=0;iTel<CTNumber;iTel++) if(TelTrigger[iTel]!=0) return;

  // multiple trigger mode is applied only all telescopes are not triggered

  for(int ipmt=0;ipmt<dir_collection.size();ipmt++)
  {
    Pattern.clear();
    Pattern.insert(dir_collection[ipmt]);
    while(1)
    {
      addpmt = 0;
      for(int jpmt=0;jpmt<dir_collection.size();jpmt++)
      {
        if(jpmt==ipmt) continue;
        for(itor=Pattern.begin();itor!=Pattern.end();itor++)
        {
          if(itor->u*dir_collection[ipmt].u+itor->v*dir_collection[ipmt].v+itor->w*dir_collection[ipmt].w>COS)
          {
            if(Pattern.insert(dir_collection[ipmt]).second)
            {
              addpmt++;
              break;
            }
          }
        } // for(itor=Pattern.begin();itor!=Pattern.end();itor++)
        if(Pattern.size()>=Trigger_multiple) break;
      }//  for(int jpmt = 0;jpmt<NPMT;jpmt++)

      if(addpmt==0||Pattern.size()>=Trigger_multiple) break;
    } //while(1)
    if(Pattern.size()>=Trigger_multiple) for(int iTel=0;iTel<CTNumber;iTel++) TelTrigger[iTel]=2;
  }//  for(int ipmt = 0;ipmt<NPMT;ipmt++)
}

void WCamera::GetResult( WFCTAMcEvent *wfcta)
{
  for(int i=0; i<CTNumber*NSIPM;i++){
     if(TubeTrigger[i]==1){
        TubeTrigger_.push_back(TubeTrigger[i]);
        TubeID_.push_back(i);
        TubeSignal_.push_back(TubeSignal[i]);
        TubeSignalInTriggerWindow_.push_back(TubeSignalInTriggerWindow[i]);
        TubeTriggerTime_.push_back(TubeTriggerTime[i]);
        wfcta->TubeTrigger.push_back(TubeTrigger[i]);
        //wfcta->TubeSignalInTriggerWindow.push_back(TubeSignalInTriggerWindow[i]);
	wfcta->TubeSignalInTriggerWindow.push_back(TubeSignal[i]);
        wfcta->TubeID.push_back(i);
        wfcta->TubeTriggerTime.push_back(TubeTriggerTime[i]);
     
     }
  }
  for(int iTel=0;iTel<CTNumber;iTel++){
      wfcta->TelTrigger.push_back(TelTrigger[iTel]);
  }
  
}
