#include <SWI-Prolog.h>
#ifndef NULL
#define NULL ((void *)0)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#define MAX_SOCKETS 10
static int next_socket = 0;
static int sockets[MAX_SOCKETS] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

static char buf[1024];


// Close all open bluetooth sockets and re-initialize socket table
foreign_t
pl_bt_reset()
{ 
  int i;
  int close_rval = 0;
  for (i = 0; i<next_socket; i++) {
    if ( sockets[i] != -1) {
      close_rval |= close(sockets[i]);
      sockets[i] = -1;
    }
  }
  next_socket = 0;
  if (close_rval)
    return FALSE;
  return TRUE;
}


foreign_t
pl_bt_close(term_t t1)
{ 
  int index = -1;
  if ( PL_get_integer(t1, &index)  == FALSE )
    PL_fail;
  if (index < 0 || index  >= next_socket || sockets[index] == -1) {
    PL_fail;
  }
  close(sockets[index]);
  sockets[index] = -1;
  PL_succeed;
}

foreign_t
pl_scan(term_t t1, term_t t2)
{ 
  term_t l = PL_copy_term_ref(t1);
  term_t a = PL_new_term_ref();
  term_t l2 = PL_copy_term_ref(t2);
  term_t a2 = PL_new_term_ref();

    inquiry_info *ii = NULL;
    int max_rsp, num_rsp;
    int dev_id, sock, len, flags;
    int i;
    char addr[19] = { 0 };
    char name[248] = { 0 };

    dev_id = hci_get_route(NULL);
    sock = hci_open_dev( dev_id );
    if (dev_id < 0 || sock < 0) {
        PL_warning("problem opening socket");
        PL_fail;
    }

    len  = 8;
    max_rsp = 255;
    flags = IREQ_CACHE_FLUSH;
    ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));
    
    num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
    if( num_rsp < 0 ) perror("hci_inquiry");

    for (i = 0; i < num_rsp; i++) {
        ba2str(&(ii+i)->bdaddr, addr);
	if ( !PL_unify_list(l, a, l) || !PL_unify_atom_chars(a, addr) )
	  PL_fail;
	memset(name, 0, sizeof(name));
	if (hci_read_remote_name(sock, &(ii+i)->bdaddr, sizeof(name), name, 0) < 0)
	  strcpy(name, "[unknown]");
	if ( !PL_unify_list(l2, a2, l2) || !PL_unify_atom_chars(a2, name) )
	  PL_fail;
    }
    free( ii );
    close( sock );
    return PL_unify_nil(l);
}

// Note that this handles a stream containing lines ending in \r\n (not portable yet)

int check_for(char *buf,int total_bytes,char *eof)
{
  int len = strlen(eof);
  int rval = !strncmp(&buf[total_bytes-(len)],eof,len);
  return rval;
}

foreign_t
pl_converse(term_t s, term_t l, term_t r)
{ 
  int index = -1;
  term_t head = PL_new_term_ref();   /* the elements */
  term_t list = PL_copy_term_ref(l); /* copy (we modify list) */

  if ( PL_get_integer(s, &index)  == FALSE )
    PL_fail;

  if (index < 0 || index >= next_socket || sockets[index] == -1)
    PL_fail;

  char *cs;

  if (PL_is_atom(list)) { // Not a list!
    if ( PL_get_atom_chars(list, &cs) ) write(sockets[index], cs, strlen(cs));
    else PL_fail;
  } else {

    while( PL_get_list(list, head, list) ) { // Send everything in the list
      if ( PL_get_atom_chars(head, &cs) ) {
	write(sockets[index], cs, strlen(cs));
      }
      else
	PL_fail;
    }
    if (PL_get_nil(list) == FALSE)  /* test end for [] */
      PL_fail;
  } // End of Sending List

  int bytes_read;
  int total_bytes;
  memset(buf,0,1024);
  total_bytes = 0;

  bytes_read = read(sockets[index], &buf[total_bytes], sizeof(buf)-total_bytes);
  while (bytes_read > 0) {
    total_bytes += bytes_read;
    if (check_for(buf,total_bytes,"end_of_data\r\n")) // Quit reading
      break;
    sleep(1);  // Give the guy a chance to respond fully
    bytes_read = read(sockets[index],&buf[total_bytes], sizeof(buf)-total_bytes);
  }
  return PL_unify_string_nchars(r, total_bytes, buf);
}

foreign_t
pl_bluetooth_socket(term_t mac, term_t n)
{
  int s;
  char *dest;
  PL_get_atom_chars(mac,&dest);
  s = bluetoothSocket(dest);
  if (s == -1)
    PL_fail;
  if (PL_unify_integer(n, (intptr_t)next_socket) == FALSE)
    PL_fail;
  sockets[next_socket++] = s;
  PL_succeed;
}
/* Returns file descriptor for a Bluetooth connection */

int bluetoothSocket(char *dest) {
  int tries = 10;

  struct sockaddr_rc addr = {
    .rc_family  = AF_BLUETOOTH,
    .rc_bdaddr  = (bdaddr_t){{0xff,0xff,0xff,0xff,0xff,0xff}},
    .rc_channel = (uint8_t) 1
  };

  str2ba( dest, &addr.rc_bdaddr );

  int s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
  if (s == -1) {
    PL_warning("Failed to create a Bluetooth socket");
    PL_fail;
  }
  fprintf(stderr,"connecting...\n");
  while ( connect(s, (struct sockaddr *)&addr, sizeof(addr)) && 0 < tries-- ) {
    fprintf(stderr,"try %d\n",tries);
    close(s);
    s = -1;
    while(s == -1 && 0 < tries--) {
      sleep(0.2);
      s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    }
    //    PL_warning("Bluetooth connection failed. Retrying with new socket");
  }
  if (tries < 0) {
    if (s != -1) close(s);
    return -1;
  }
  return s;
}

static PL_extension predicates [] =
{
  { "bt_socket",    2, pl_bluetooth_socket, 0 },
  { "bt_converse",  3, pl_converse,         0 },
  { "bt_scan",      2, pl_scan,             0 },
  { "bt_close",     1, pl_bt_close,         0 },
  { "bt_reset",     0, pl_bt_reset,         0 },
  { NULL, 0, NULL, 0 } /* terminator */
};

install_t install_plblue() { PL_load_extensions(predicates); }

