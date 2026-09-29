# Jigsaw -- OpenWatcom 2.0 wmake file for OS/2 32-bit PM

CC      = wcc386
LINK    = wlink
RC      = wrc

CFLAGS  = -bt=os2 -mf -5 -fpi -Oaxt -W3 -ze -d0 &
          -i=src -i=$(OS2TK)\h

LFLAGS  = system os2v2_pm &
          option stack=65536 &
          option heap=4096 &
          option map &
          option quiet

OBJS    = jigsaw.obj

all: bin\jigsaw.exe

jigsaw.obj: src\jigsaw.c src\jigsaw.h src\lang.h
	$(CC) $(CFLAGS) -fo=$@ src\jigsaw.c

jigsaw.res: src\jigsaw.rc src\jigsaw.h
	$(RC) -r -fo=jigsaw.res -i=src -i=$(OS2TK)\h src\jigsaw.rc

bin\jigsaw.exe: $(OBJS) jigsaw.res
	$(LINK) $(LFLAGS) &
	  file jigsaw.obj &
	  name bin\jigsaw.exe
	$(RC) jigsaw.res bin\jigsaw.exe

clean: .SYMBOLIC
	-rm -f $(OBJS)
	-rm -f jigsaw.res
	-rm -f jigsaw.map
	-rm -f bin\jigsaw.exe
