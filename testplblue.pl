:- load_foreign_files('plblue',[],[]), writeln('plblue.so loaded').
:- dynamic lagoon_socket/1.

open_all_bluetooth(SocketList) :-
    bt_scan(BtList),
    maplist(bt_socket, BtList, SocketList),
    writeln(SocketList).

talk(X) :-
    lagoon_socket(S),
    bt_converse(S,X,R),
    writeln(reply(R)).

allon :-
    lagoon_socket(S),
    bt_converse(S,['a1\n'],_),
    bt_converse(S,['l1\n'],_),
    bt_converse(S,['m1\n'],_).


alloff :-
    lagoon_socket(S),
    bt_converse(S,['m0\n'],_),
    bt_converse(S,['l0\n'],_),
    bt_converse(S,['a0\n'],_).


main :-
    writeln('start test'),
    bt_scan([X|_],[N|_]),
    writeln(scanned(X,N)),
    bt_socket(X,S),
    writeln(socketEstablished(X,S,N)),
    ( N == 'Lagoon' ->
	assert(lagoon_socket(S))
    ;
	true
    ),
    bt_converse(S,['h\n'],Reply),
    writeln(reply(Reply)),
    alloff,
    !.
