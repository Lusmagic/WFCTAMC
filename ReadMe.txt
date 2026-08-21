//------------------------------------------------------
2022-04-29
all reflections of tels are same.
//------------------------------------------------------
2021-09-28
the map is modified by chensuhong 

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


//------------------------------------------------------
2021-09-16
the threshold is set to 40 in input files.

//------------------------------------------------------
2021-09-15
the PDE is updated by Yang Mingjie and the data is added to reflection of mirror. In other words, the reflection and PDE are combined.
the data is from F:\work\IHEP\lhaasodata\WFCTA模拟\软件更新 in Wang Yudong PC


//------------------------------------------------------
2021-08-07
the reflection of mirror is update by wangyudong and the data is from JinMin and wangyudong.

//------------------------------------------------------
2021-05-18
1, in order to avoid the endless loop in ReadBuffer() function, 
add the judge of the return of ReadBuffer();
if(stat) break;

//------------------------------------------------------
2021-4-13
1, the function has been updated by llma in main function.

 double temp = telescope[ict]->RayTrace(MirrorSpot[ict],
                                       -ph_bunch->y_*10, ph_bunch->x_*10, ph_bunch->z_*10, //update 20210413
                                       -ph_bunch->v_, ph_bunch->u_, ph_bunch->l_,
                                        &ph_bunch->xc_, &ph_bunch->yc_,&ph_bunch->time_raytrace_, &ph_bunch->u1_, &ph_bunch->v1_);
2,  //*xc = -ycluster;
    //*yc = xcluster;
    *xc = xcluster;  //update 20210413 by llma
    *yc = ycluster;


//------------------------------------------------------
2021-4-9
1. the mirror spots in inputfile are all modified into 9 mm from 6 mm.
2. the paramter of telescope are modified： const double D_DOOR = 2410; //width of door from Geng Lisi  20210409
const double Hdoor = 2470. ; //mm, the height of the container;  from Geng Lisi  20210409
 
//------------------------------------------------------
2021-1-15
1. the contents and format of fitlers transparency are updated by Wang yudong.
   In the ./Data/Filter, the filter glasses of all telescopes are quartz, the data is from wangyudong
   (modified base on the data from Chen Suhong!!!!).

//-------------------------------------------------------
2021-1-13
1. The version inherits from Beta.
2. Some comment lines are removed.


//-------------------------------------------------------
2021-1-12
1. the contents and format of fitlers transparency are updated by Wang yudong.
   In the ./Data/Filter, the filter glasses of all telescopes are quartz, the data is from wangyudong
   (NOT from Chen Suhong!!!!).
   
   the wavelength is extended from 300-600 to 200-1000nm

   in the source file wtelescope.cc, these lines are modified:

   filter_wl_min = 200;    //300
   filter_wl_max = 1000;   //600

   Transmissivity = new TGraph2D(161*8); //61



//--------------------------------------------------------
2021-1-11
1. the WFCTAMC.C is modified to adapt to read CORSIKA file in eos disk. 
   TFile::Open() is used and the runtest.sh is the test script. 
   
  
   Many lines are change based Wang Yudong, llma, Yin liqiao and You zhiyhong.
   

//-------------------------------------------------------
2021-1-7
1. In the source file wcamera.cc, the line is modified:
 
   SiPMMAP[k][1] = (PIX-i)*D_ConeOut + interval*(7-Intery)-centery; // reported by liuhu and youzhiyong
   the old line is:
   //SiPMMAP[k][1] = (PIX-i)*D_ConeOut + interval*Intery-centery;   
  
 
//--------------------------------------------------------
2020-6-30
1: The Atmosphere model is setted in the input card by the key word "atmmodel"
   2 is for us standard model, 3 is for magic summer, 4 is for magic winter
   440000 is for the altitude of observatory (here is for DaoCheng)

2: The reflectivity of mirrors is ininited by 0.85 with wavelength from 250nm tp 1250 nm.

3: The reflectivity of winston cones is setted be to 0.9. 

4: The signal integral window is fadc_bin*fadc_length, 
   photons within the window (maxtime-fadc_bin*fadc_length/2, maxtime+fadc_bin*fadc_length/2) are counted. 

5: mirrorsize in the input card should be 500cm if the zenith angle of telescope is larger than 35 degree. 

2020-6-30
the reflectivity of the mirrors, transparency of the fitlers, spot size, nsb of each telescope can be setted individually 
the reflectivity of the mirrors, transparency of the filters can be setted by the files in the Data directory
the spot size and nsb can be setted in the input card


2020-9-16
The shelter of the wind duct and ledframe are considered during the ray tracing process 
The output of events is written to  WFCTAMcEvent class
