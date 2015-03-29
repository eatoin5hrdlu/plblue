
:- load_foreign_files('plout',[],[]), writeln('plout.so loaded').


open_all_bluetooth(SocketList) :-
    bt_scan(BtList),
    maplist(bt_socket, BtList, SocketList),
    writeln(SocketList).

main :-
    writeln('start test'),
    bt_scan([X|_]),
    bt_socket(X,S),
    writeln(socketEstablished(X,S)),
    bt_converse(S,['h\n'],Reply),
    writeln(reply(Reply)),
    !.

main :-
    writeln('main failed, as expected?').
