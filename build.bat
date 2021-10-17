@echo off
rc /nologo i_love_music.rc 
cl source\i_love_music.c i_love_music.res /nologo /Wall /wd 4324 /wd 4514 /wd 4710 /wd 4711 /wd 4738 /wd 4820 /wd 5045 /WX /O2 /Ob2 /Oi /Ot /Oy /MT /GL /GF /D "NDEBUG" /link /INCREMENTAL:NO /OPT:REF /OPT:ICF /LTCG /SUBSYSTEM:WINDOWS