LIBS = -lbluetooth   # Linux
# LIBS = -lwsock32  # Windows
DEF_BT=98:D3:31:70:2B:70
all :	bluetest plblue.so

bluetest : bluetest.cpp
	gcc -Wno-write-strings -o bluetest bluetest.cpp $(LIBS)

test1 : bluetest
	./bluetest $(DEF_BT)

plout.so :	plbluelib.c
	swipl-ld -Wno-unused-result -shared -o plblue plbluelib.c -lbluetooth


test : plblue.so
	swipl -s testplblue.pl -g main


