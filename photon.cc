#include <stdlib.h>
#include <stdio.h>
#include "TFile.h"
#include "TTree.h"
#include "TMath.h"
#include "TCanvas.h"
#include "TH2D.h"
#include "TBox.h"
#include "TGaxis.h"
#include "TGraph.h"
#include "WFCTAMcEvent.h"

using namespace std;

int main(int argc, char *argv[]) {

  if(argc<3) {
      printf("Usage: %s infile.root ievt out.png \n", argv[0]);
      return 1;
  }

  WFCTAMcEvent *wfctaevent = new WFCTAMcEvent();

  TFile *hfile= TFile::Open(argv[1]);
  TTree *EventTree = (TTree *)gDirectory->Get("WFCTA");
  EventTree->SetBranchAddress("WFCTA", &wfctaevent);

  int PIX =  32;
  int CTNumber = 18;
  int NSIPM = PIX*PIX;
  float *photon_total;
  photon_total = new float[CTNumber*NSIPM];
  memset(photon_total, 0, sizeof(float) * CTNumber * NSIPM);
  EventTree->SetBranchAddress("photon_total", photon_total);

  int ievt = atoi(argv[2]);
  EventTree->GetEntry(ievt);
  int itel = wfctaevent->itel;


  TCanvas *cImage0 = new TCanvas("cImage0", "photon:i", 800, 600);
  TH2D *graph1  = new TH2D("photon:i", "photon:i;i;photon",
                        100, 0 , 18500 , 100, 0,80000);

  for(int i=0;i<CTNumber*NSIPM;i++){
	  double photon = photon_total[i];
	  graph1->Fill(i,photon);

  }
  graph1->SetStats(0);
  graph1->SetMarkerStyle(20);
  graph1->SetMarkerColor(kBlue);
  graph1->SetMarkerSize(0.5);

  cImage0->cd();
  graph1->Draw("prof");
  cImage0->SaveAs(argv[3]);


}
