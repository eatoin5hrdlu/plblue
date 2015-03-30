LIBS = -lbluetooth -lwiringPi  # Linux 
# LIBS = -lwsock32  # Windows
DEF_BT=98:D3:31:70:2B:70
all :	rendezvous bluetest plblue.so

rendezvous : rendezvous.cpp
	gcc -Wno-write-strings -o rendezvous rendezvous.cpp $(LIBS)

bluetest : bluetest.cpp
	gcc -Wno-write-strings -o bluetest bluetest.cpp $(LIBS)

test1 : bluetest
	./bluetest $(DEF_BT)

plblue.so :	plbluelib.c
	swipl-ld -Wno-unused-result -shared -o plblue plbluelib.c -lbluetooth


test : plblue.so
	swipl -s testplblue.pl -g main


