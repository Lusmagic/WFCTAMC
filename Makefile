inc_dir = include/
src_dir = src/
obj_dir = obj/
bin_dir = bin/
tmp_dir = tmp/
CCOMPILER  = g++
DEBUG = -ggdb  #-g -gstabs -gstabs+ -ggdb -glevel

object = $(obj_dir)WFCTA-MC.o \
         ${obj_dir}wtelescope.o ${obj_dir}wcamera.o ${obj_dir}SquareCone.o ${obj_dir}creadparam.o $(obj_dir)atm.o $(obj_dir)attenu.o $(obj_dir)cer_event.o $(obj_dir)WFCTAMcEventDict.o $(obj_dir)WFCTAMcEvent.o

WFCTA-MC: $(obj_dir)WFCTA-MC.a
	$(CCOMPILER) -o WFCTA-MC $(obj_dir)WFCTA-MC.a `root-config --cflags --libs` 

$(obj_dir)WFCTA-MC.a: $(object)
	ar -r $(obj_dir)WFCTA-MC.a $(object)


$(obj_dir)WFCTA-MC.o: WFCTA-MC.C Makefile $(inc_dir)
	$(CCOMPILER) $(DEBUG) -c $< -o $@ -DSCAN -I $(inc_dir) `root-config --cflags --libs`

$(obj_dir)wtelescope.o: $(src_dir)wtelescope.cc Makefile $(inc_dir)
	$(CCOMPILER) $(DEBUG) -c $< -o $@ -DSCAN -I $(inc_dir) `root-config --cflags --libs`

$(obj_dir)wcamera.o: $(src_dir)wcamera.cc Makefile $(inc_dir)
	$(CCOMPILER) $(DEBUG) -c $< -o $@ -DSCAN -I $(inc_dir) `root-config --cflags --libs`

$(obj_dir)SquareCone.o: $(src_dir)SquareCone.cc Makefile $(inc_dir)
	$(CCOMPILER) $(DEBUG) -c $< -o $@ -DSCAN -I $(inc_dir) `root-config --cflags --libs`

$(obj_dir)creadparam.o: $(src_dir)creadparam.cc Makefile $(inc_dir)
	$(CCOMPILER) $(DEBUG) -c $< -o $@ -DSCAN -I $(inc_dir) `root-config --cflags --libs`

$(obj_dir)atm.o: $(src_dir)atm.cc Makefile $(inc_dir)
	$(CCOMPILER) $(DEBUG) -c $< -o $@ -DSCAN -I $(inc_dir) `root-config --cflags --libs`

$(obj_dir)attenu.o: $(src_dir)attenu.cc Makefile $(inc_dir)
	$(CCOMPILER) $(DEBUG) -c $< -o $@ -DSCAN -I $(inc_dir) `root-config --cflags --libs`

$(obj_dir)cer_event.o: $(src_dir)cer_event.cc Makefile $(inc_dir)
	$(CCOMPILER) $(DEBUG) -c $< -o $@ -DSCAN -I $(inc_dir) `root-config --cflags --libs`

$(obj_dir)WFCTAMcEvent.o: $(src_dir)WFCTAMcEvent.cc Makefile $(inc_dir)
	$(CCOMPILER) $(DEBUG) -c $< -o $@ -DSCAN -I $(inc_dir) `root-config --cflags --libs`

$(obj_dir)WFCTAMcEventDict.o: $(src_dir)WFCTAMcEventDict.cc Makefile $(inc_dir)
	$(CCOMPILER) $(DEBUG) -c $< -o $@ -DSCAN -I $(inc_dir) `root-config --cflags --libs`
.PHONY : clean
 clean :
	rm WFCTA-MC $(object)
