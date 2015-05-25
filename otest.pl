:- use_module(osc).


clicker(N) :- sendOSC('s_new',"siii",[mtish,N,0,1]).
echo(N)    :- sendOSC('s_new', "sisf", [echo,N,delay,1.0]).
kill(N)    :- sendOSC('n_free',"i",[N]).
off(N)     :- sendOSC('n_run',"ii",[N,0]).
on(N)      :- sendOSC('n_run',"ii",[N,1]).
fast(N)    :- sendOSC('n_set',"isf",[N,rate,9.98]).
medium(N)  :- sendOSC('n_set',"isf",[N,rate,3]).
slow(N)    :- sendOSC('n_set',"isf",[N,rate,1.1]).

sctest(Node) :-
        clicker(Node),
	ENode is Node + 1,
        echo(ENode),
        sleep(7),
        slow(Node),
        sleep(3),
        medium(Node),
        sleep(3),
        fast(Node),
        off(Node),
        sleep(2),
	on(Node),
        sleep(2),
        off(Node),
	halt.

