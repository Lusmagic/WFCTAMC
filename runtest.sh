#!/bin/bash

infile=/eos/user/l/liushuo/Laser/spotrange1.5/Laser40/change0/LAS000036
outfile=/eos/user/l/liushuo/Laser/spotrange1.5/Laser40/change0/LAS000036.root

./WFCTA-MC inputcard/default.inp root://eos01.ihep.ac.cn/${infile}?filetype=raw ${outfile}

