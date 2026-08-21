#include <stdlib.h>
#include <stdio.h>
#include "TFile.h"
#include "TTree.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TH2D.h"
#include "TBox.h"
#include "TGaxis.h"
#include "WFCTAMcEvent.h"

using namespace std;

float Focus[18] = { 2869.5, 2869, 2870, 2871, 2886, 2869, 2872, 2869, 2872,
                    2867.5, 2867.5, 2870, 2860, 2871, 2871, 2868, 2871, 2871};

float Dx[18] = { -0.24, 0.7, -0.04, -0.74, -0.18, -0.6, 0.12, 0.32,  -0.18,
                 0.12, 0.3, -0.3, -0.06, 0.02, 0.06, 0.28, -0.04, 0.06};
int itel;

void GetPMTMAP(double (*sipmmap)[2])
{
  double interval = 1.0;
  double D_ConeOut=25.8 ;
  int Interx,Intery;
  double centerx, centery;
  int PIX = 32;
  centerx = (32.5*D_ConeOut+7*interval)/2;//-D_ConeOut/2;//16*D_ConeOut+3.5*interval;//414.3;
  centery = (32*D_ConeOut+7*interval)/2;//-D_ConeOut/2;//414.3;

  double SSX = Dx[itel]*TMath::DegToRad()*Focus[itel];

  for(int k=0;k<1024;k++)
  {
    double imagex,imagey;
    int i = k/32;
    int j = k%32;
    Intery = i/4;
    Interx = j/4;
    if(i%2==0)
      imagex = -((j+0.5)*D_ConeOut + interval*Interx-centerx);
    if(i%2==1)
      imagex = -((j+1)*D_ConeOut + interval*Interx-centerx);

    imagey = ((PIX-i)*D_ConeOut + interval*(7-Intery)-centery)-D_ConeOut/2;

    sipmmap[k][0] = imagex - SSX;
    sipmmap[k][1] = imagey;
  }
}

int main(int argc, char *argv[]) {

  WFCTAMcEvent *wfctaevent = new WFCTAMcEvent();

  itel=atoi(argv[3])-1;

  TFile *hfile= TFile::Open(argv[1]);
  TTree *EventTree = (TTree *)gDirectory->Get("events");
  EventTree->SetBranchAddress("wfcta", &wfctaevent);

  EventTree->GetEntry(0);

  double sipmmap[1024][2];
  GetPMTMAP(sipmmap);
  TCanvas *cImage = new TCanvas("cImage","Cherenkov Image",700,700);
  TH2D *h_mean_cr = new TH2D("h_mean_cr","laser size (lg(pe))",50,-9,9,50,-9,9);
  h_mean_cr->SetStats(0);
  TBox *bx[1024];

  double maxtel, mintel;
  maxtel = 0;
  mintel = 1000000;
  int npix=wfctaevent->TubeID.size();
  for(int i=0;i<npix;i++){
    if(wfctaevent->TubeID.at(i)<itel*1024||wfctaevent->TubeID.at(i)>=itel*1024+1024) continue;
    if(wfctaevent->TubeSignalInTriggerWindow.at(i)<50) continue;
    if(wfctaevent->TubeSignalInTriggerWindow.at(i)>maxtel) maxtel = wfctaevent->TubeSignalInTriggerWindow.at(i);
    if(wfctaevent->TubeSignalInTriggerWindow.at(i)<mintel) mintel = wfctaevent->TubeSignalInTriggerWindow.at(i);
  }

  cImage->cd();
  h_mean_cr->Draw();
  for(int i=0;i<npix;i++){
    int itube = wfctaevent->TubeID.at(i);
    if(itube<itel*1024||itube>=itel*1024+1024) continue;
    itube -= itel*1024;
    double ipe = wfctaevent->TubeSignalInTriggerWindow.at(i);
    if(ipe<50) continue;
    double imagex = sipmmap[itube][0]/Focus[itel]*TMath::RadToDeg();
    double imagey = sipmmap[itube][1]/Focus[itel]*TMath::RadToDeg();

    bx[i] = new TBox(imagex-0.25,imagey-0.25,imagex+0.25,imagey+0.25);
    bx[i]->SetFillColor(int((log10(ipe)-log10(mintel))/(log10(maxtel)-log10(mintel))*50)+49);
    bx[i]->Draw();
  }

  TBox *bxcolor[50];
  TGaxis* axis1 = new TGaxis(9,-9,9,9,log10(mintel),log10(maxtel),20,"+LS");
  axis1->SetTickSize(0.02);
  for(int i=0;i<50;i++){
    bxcolor[i] = new TBox(9,18/50.*i+(-9),10,18./50*(i+1)+(-9));
    bxcolor[i]->SetFillColor(i+49);
    bxcolor[i]->Draw();
  }
  axis1->Draw();

  cImage->SaveAs(argv[2]);

  return 0;
}
