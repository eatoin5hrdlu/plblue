#LIBS = -lbluetooth -lwiringPi  # RPi

LIBS = -lbluetooth              # Linux 
SWIPL = swipl
SHARED=plblue.so
OSDEF=-DLINUX=1

#LIBS = -lwsock32 -lc           # Windows
#SWIPL=swipl-win
#SHARED=plblue.dll
#OSDEF=-DWINDOWS=1

CELLSTAT=98:D3:31:20:23:36
LAGOON=98:D3:31:70:2B:70
PUMPS=98:d3:31:40:1d:a4
MINGCC= -cc i686-pc-mingw32-gcc-4.7.3.exe
MINGCPP= -c++ i686-pc-mingw32-g++.exe
MING=$(MINGCC) $(MINGCPP)
CYGCC= -cc "c:/cygwin/bin/gcc"
CYGCPP= -c++ "C:/cygwin/bin/i686-pc-cygwin-g++.exe"
CYG=$(CYGCC)

all :	plblue.so test test1

install: $(SHARED)
	cp $(SHARED) ../EvoStat
	cp $(SHARED) ../PACE

rendezvous : rendezvous.cpp
	gcc -Wno-write-strings -o rendezvous rendezvous.cpp $(LIBS)

bluetest : bluetest.cpp Makefile
	gcc -Wno-write-strings $(OSDEF) -o bluetest bluetest.cpp $(LIBS)

test1 : bluetest
	./bluetest $(CELLSTAT)

plblue.so :	plbluelib.c
	swipl-ld $(OSDEF) -Wno-unused-result -shared -o plblue plbluelib.c -lbluetooth

plblue.dll :	plbluelib.c Makefile
	swipl-ld -Wpointer-to-int-cast -Wno-write-strings -Wno-unused-result -dll -shared -o plblue plbluelib.c $(LIBS)

test : $(SHARED)
	$(SWIPL) -s testplblue.pl -g main


