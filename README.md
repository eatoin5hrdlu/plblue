# plblue
Bluetooth API for SWI-Prolog

Mostly working code to scan, connect, send/receive text with devices supporting SPP protocol.
File bluelib.c is compiled with swipl-ld into a shared object file to be loaded by load_foreign_function/3.

Not handling pairing at the moment since there are usually tools for that on every platform.

API:   bt_scan(-List)         - returns MAC addresses of all available Bluetooth devices

       bt_open(+MAC, -Index)  - returns integer index of open channel to Bluetooth device
       
       bt_close(+Index)       - closes connection to Bluetooth device
       
       bt_converse(+Index, +Send, ?Receive) -- Sends text in +Send to Bluetooth device and returns the response.
       
         Send can be an atom, list of atoms, (NYI: list of character codes, or quoted string )
         Receive will be a 'string' of all data received from Bluetooth device up to the string "end_of_data\r\n"
        NB: Calls to bt_converse/3 will not return until the Bluetooth device sends these characters.
     
      Not Yet Implemented:
        (To re-implement bt_converse/3 as the following two calls)
        
       bt_send(+Index, +Send)  -- Sends text to Bluetooth device
       
       bt_receive(+Index, -Receive) -- Read data from device until EndOfTransmission pattern is received
       
       bt_set_eot(+EndOfTransmission)   -- Change the default "end_of_data\r\n" marker to the characters in EndOfTransmission.
       
        
