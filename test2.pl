:- current_prolog_flag(windows,true)
   -> use_foreign_library(foreign(plblue)), writeln('plblue.dll loaded')
    ; use_foreign_library(plblue), writeln('plblue.so loaded').

:- dynamic lagoon_socket/1.

bt_address(Name, Addr) :- !, % saves timea
   nth0(N,[  cellstat,
             autosampler,
             lagoon1,
	     lagoon2,
	     lagoon3,
	     lagoon4,
	     lagoon5
	  ], Name),
   nth0(N,['98:D3:31:90:29:0E',
           '98:D3:31:40:1D:D4',
           '98:D3:31:70:2A:22',
           '98:D3:31:40:31:BA',
           '98:D3:31:20:2B:EB',
           '98:D3:31:70:2B:70',
           '98:D3:31:20:23:4F'
	  ],Addr).

open_bluetooth(Name) :-
    bt_address(Name,Addr),
    bt_socket(Addr,Socket),
    Term =.. [Name,Socket],
    assert(Term),
    writeln(Term).

open_all_bluetooth :-
    bt_address(Name,_Addr),
    ( open_bluetooth(Name)
     -> writeln(success(Name))
     ;  writeln(failure(Name))
    ),
    fail.
open_all_bluetooth :- writeln(done).

talk(X) :-
    lagoon(S),
    bt_converse(S,X,R),
    writeln(reply(R)).

allon :-
    lagoon_socket(S),
    bt_converse(S,['a1\n'],_),
    bt_converse(S,['l1\n'],_),
    bt_converse(S,['m1\n'],_).


alloff :-
    lagoon_socket(S),
    bt_converse(S,['m0\n'],R1),
    writeln(reply(R1)),
    bt_converse(S,['l0\n'],R2),
    writeln(reply(R2)),
    bt_converse(S,['a0\n'],R3),
    writeln(reply(R3)).


%    bt_scan([X|_],[N|_]),
%    writeln(scanned(X,N)),
%  X = '98:D3:31:70:2B:70',

main :-
    writeln('start test'),
    ( bt_scan([X|_],[N|_]) ; true ),
    writeln('after scan'),
%    lagoon(X),
    cellstat(X),
    bt_socket(X, S),
    writeln(socketEstablished(X,S,N)),
    ( N = 'Lagoon' ->
	assert(lagoon_socket(S))
    ;
	true
    ),
    bt_converse(S,['h\n'],Reply),
    writeln(reply(Reply)),
    alloff,
    !.


