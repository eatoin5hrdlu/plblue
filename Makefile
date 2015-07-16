#LIBS = -lbluetooth -lwiringPi  # Linux 
LIBS = -lwsock32 -lc # Windows
#LIBS=/usr/i686-pc-mingw32/sys-root/mingw/lib/libwsock32.a -lc
LAGOON=98:D3:31:70:2B:70
PUMPS=98:d3:31:40:1d:a4
MINGCC= -cc i686-pc-mingw32-gcc-4.7.3.exe
MINGCPP= -c++ i686-pc-mingw32-g++.exe
MING=$(MINGCC) $(MINGCPP)
CYGCC= -cc "c:/cygwin/bin/gcc"
CYGCPP= -c++ "C:/cygwin/bin/i686-pc-cygwin-g++.exe"
CYG=$(CYGCC)

all :	rendezvous bluetest plblue.so

rendezvous : rendezvous.cpp
	gcc -Wno-write-strings -o rendezvous rendezvous.cpp $(LIBS)

bluetest : bluetest.cpp Makefile
	gcc -Wno-write-strings -o bluetest bluetest.cpp $(LIBS)

test1 : bluetest
	./bluetest $(LAGOON)

plblue.so :	plbluelib.c
	swipl-ld -Wno-unused-result -shared -o plblue plbluelib.c -lbluetooth

plblue.dll :	plbluelib.c Makefile
	swipl-ld -Wpointer-to-int-cast -Wno-write-strings -Wno-unused-result -dll -shared -o plblue plbluelib.c $(LIBS)


test : plblue.dll
	swipl -s testplblue.pl -g main


