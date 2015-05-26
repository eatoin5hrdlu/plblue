Server.default.options.device ="ASIO4ALL"
s = Server.default
s.boot
s.sendMsg(\dumpOSC,1);
s.sendMsg(\s_set,\volume,1);
s.volume.volume = 10;
s.sendMsg(\volumeAmp,0.01);
s.reboot;
SynthDef(\plr,{Out.ar(0,{SinOsc.ar(700,0,2,0)})}).load(s)
s.sendMsg(\s_new,\plr,2010)
s.sendMsg(\n_run,1002,1);

(
// define a noise pulse
SynthDef("mtish", { arg freq = 1200, rate = 2, amp = 5;
	var osc, trg, mrate;
	mrate = MouseX.kr(1,5);
	trg = Decay2.ar(Impulse.ar(rate,0,0.3), 0.01, 0.3);
	osc = {WhiteNoise.ar(trg)}.dup;
	Out.ar(0, osc); // send output to audio bus zero.
}).send(s);
)



(
// define an echo effect
SynthDef("echo", { arg delay = 0.03, decay = 3;
	var in, mdelay;
	in = In.ar(0,2);
	mdelay = MouseY.kr(0.01,2);
	// use ReplaceOut to overwrite the previous contents of the bus.
	ReplaceOut.ar(0, CombN.ar(in, 0.5, mdelay, decay, 1, in));
}).send(s);
)

//McCartney's Babbling Brook
(
SynthDef(\brook,{Out.ar(0,{
({RHPF.ar(OnePole.ar(BrownNoise.ar, 0.99), LPF.ar(BrownNoise.ar, 14)
* 400 + 500, 0.03, 0.003)}!2)
+ ({RHPF.ar(OnePole.ar(BrownNoise.ar, 0.99), LPF.ar(BrownNoise.ar, 20)
* 800 + 1000, 0.03, 0.005)}!2)
* 4
})}).send(s);
)
s.sendMsg("/n_free",1006);
s.sendMsg("/s_new", \brook, y = s.nextNodeID, 1, 1);
p = Synth(\brook);
s.sendMsg(\n_run,1006,1);

// start the pulse
s.sendMsg("/s_new", "mtish", x = s.nextNodeID, 1, 1, \freq, 200, \rate, 1.2);

// stop the pulse
s.sendMsg("/n_free",x);

// add an effect
s.sendMsg("/s_new", "echo", y = s.nextNodeID, 1, 1);
// stop the effect
s.sendMsg("/n_free", y);

// add an effect (time has come today.. hey!)
s.sendMsg("/s_new", "echo", z = s.nextNodeID, 1, 1, \delay, 0.1, \decay, 1);

// stop the effect
s.sendMsg("/n_free", z);
s.sendMsg("/n_free", 1008);