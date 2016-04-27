@echo off
if exist cfg\%1.cfg move cfg\%1.cfg cfg\%1.bak
if exist hi\%1.hi move hi\%1.hi hi\%1.bak
if exist nvram\%1.nv move nvram\%1.nv nvram\%1.bak
if exist diff\%1.dif move diff\%1.dif diff\%1.bak

mamearcade %1 -playback %2.inp %3 %4 %5 %6 %7 %8 %9 -nvram_directory NUL

if exist cfg\%1.bak move cfg\%1.bak cfg\%1.cfg
if exist hi\%1.bak move hi\%1.bak hi\%1.hi
if exist nvram\%1.bak move nvram\%1.bak nvram\%1.nv
if exist diff\%1.bak move diff\%1.bak diff\%1.dif
