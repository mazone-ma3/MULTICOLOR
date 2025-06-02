# VABMP.EXE makefile for PC-88VA OpenWatcom

#マクロ
CC= wcl

CFLAGS = -ms -bt=dos -ox -0

# Cソ－スからobj作成

.SUFFIXES: .obj .c
.c.obj:
	$(CC) $< $(CFLAGS) -c


#生成手順

all:VABMP.EXE

OBJFILES =  VABMP.obj

VABMP.EXE : $(OBJFILES)
	wlink  system Dos option quiet name VABMP file { $(OBJFILES) }

VABMP.obj : VABMP.c
