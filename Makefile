LIBS = -lbluetooth   # Linux
# LIBS = -lwsock32  # Windows

all :	bluetest plblue.so

bluetest : bluetest.cpp
	gcc -Wno-write-strings -o bluetest bluetest.cpp $(LIBS)

plout.so :	plbluelib.c
	swipl-ld -Wno-unused-result -shared -o plblue plbluelib.c -lbluetooth

test : plblue.so
	swipl -s testplblue.pl -g main


