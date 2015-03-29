
plout.so :	bluelib.c
	swipl-ld -Wno-unused-result -shared -o plout bluelib.c -lbluetooth

test : plout.so
	swipl -s testplblue.pl -g main


