#include <SWI-Prolog.h>
#include <errno.h>
#ifndef NULL
#define NULL ((void *)0)
#endif

#define WORDS__BIGENDIAN 1
#ifdef WINDOWS
#include "plbluewindows.h"
#define close closesocket
#else
#include "plbluelinux.h"
#endif

//#include <stdio.h>
//#include <stdlib.h>
//#include <unistd.h>
//#include <sys/socket.h>
//#include <bluetooth/bluetooth.h>
//#include <bluetooth/rfcomm.h>
//#include <bluetooth/hci.h>
//#include <bluetooth/hci_lib.h>

#define MAX_SOCKETS 10
static int next_socket = 0;
static int sockets[MAX_SOCKETS] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

static char buf[1024];
static int plblueonce = 0;

//static FILE *db = (FILE *)NULL;
// Must turn off tracing after PL_warning
void notrace(void) {  
  term_t nt = PL_new_term_ref();
  PL_unify_atom_chars(nt, "notrace");
  PL_call(nt,NULL);
}
#ifdef WINDOWS
static char _hex_chars[16] = "0123456789ABCDEF";

void btad2string(unsigned long ba, char *dest) {
int i;
unsigned char bytes[6];
    for( i=0; i<6; i++ ) {
        bytes[5-i] = (unsigned char) ((ba >> (i*8)) & 0xff);
    }
    for (i=5; i>-1; i--) {
        dest[(5-i)*3] = _hex_chars[(bytes[5-i]>>4)&0x0F];
        dest[(5-i)*3+1] = _hex_chars[bytes[5-i]&0x0F];
	if ( ((5-i)*3+2)==17) dest[(5-i)*3+2] = 0;
        else                  dest[(5-i)*3+2] = ':';
    }
}

void
ba2str( BTH_ADDR ba, char *addr )
{
    int i;
    unsigned char bytes[6];
    for( i=0; i<6; i++ ) {
        bytes[5-i] = (unsigned char) ((ba >> (i*8)) & 0xff);
    }
    for (i=0; i< 6; i++)
    sprintf(addr, "%02X:%02X:%02X:%02X:%02X:%02X", 
            bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5] );
}

void mystr2ba(char *btaddr, BTH_ADDR *addr)
{
  char buff[100];
  strcpy(buf,btaddr);
  char *cp = &buf[0];
  int i,j,bitshift,byteshift;
  unsigned long long shift, value;
  unsigned long long packed = 0L;
  for(i=0;i<6;i++) {
    cp = &buf[i*3];
    buf[(i*3)+2] = 0;
    value = (0xFF & strtol(cp,NULL,16));
    byteshift = (15-(3*i))/3;
    bitshift = byteshift * 8;
    for (j=0; j<byteshift; j++) {
        value = value<<8;
    }
    packed |= value;
  }
  *addr = packed;
}
#endif

/* Send a reset string (zeros) */
#define NUMNULLS  10
foreign_t
pl_areset(term_t s)
{ 
  int index = -1;
  int i;
  char zeros[NUMNULLS];

  if ( PL_get_integer(s, &index)  == FALSE )
    PL_fail;

  if (index < 0 || index >= next_socket || sockets[index] == -1)
    PL_fail;
  for(i=0;i<NUMNULLS;i++) zeros[i] = 0;
  for(i=0;i<100;i++)
    write(sockets[index], zeros, NUMNULLS);
  PL_succeed;
}

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

foreign_t dcg_float_codes(term_t Number, term_t Codes, term_t Tail)
{
  term_t l = PL_copy_term_ref(Codes);
  term_t a = PL_new_term_ref();
   union { float asNumber; char asCodes[sizeof(float)];} val, val1;
   int j = 0;

#define WORDS_BIGENDIAN 1
#ifdef WORDS_BIGENDIAN
   j = sizeof(float) - 1;
#endif

  double tmp;
  if(PL_get_float(Number, &tmp))
    {
      val.asNumber = (float)tmp;
      int i;
      for (i=0;i<sizeof(float);i++) {
	if (    !PL_unify_list(l, a, l)
		|| !PL_unify_integer(a, (val.asCodes[i^j]&0xFF)) )
	  PL_fail;
      }
      return PL_unify(l, Tail);
    }
  PL_fail;
}

foreign_t
pl_scan(term_t t1, term_t t2)
{ 
#ifdef WINDOWS
  /*  PL_warning("bt_scan/2 not implemented on Windows"); */
  PL_fail;
#else
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
	notrace();
        PL_fail;
    }

    len  = 8;
    max_rsp = 255;
    flags = IREQ_CACHE_FLUSH;
    ii = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));
    
    num_rsp = hci_inquiry(dev_id, len, max_rsp, NULL, &ii, flags);
    if( num_rsp < 0 ) perror("hci_inquiry");

    for (i = 0; i < num_rsp; i++) {
#ifdef WINDOWS
        btad2string(&(ii+i)->bdaddr, addr);
#else
        ba2str(&(ii+i)->bdaddr, addr);
#endif
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
#endif
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
    //    fprintf(db,"atom\n");
    if ( PL_get_atom_chars(list, &cs) ) {
        write(sockets[index], cs, strlen(cs));
	//	fprintf(db,"SENDATOM[%s]\n",cs);
    }
    else PL_fail;
  } else {
    //    fprintf(db,"list\n");
    while( PL_get_list(list, head, list) ) { // Send everything in the list
      if ( PL_get_atom_chars(head, &cs) ) {
	write(sockets[index], cs, strlen(cs));
	//	fprintf(db,"SENDLIST[%s]\n",cs);
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

  sleep(3);   // Completely necessary!!!! If Arduinos slow down this will
              // need to be longer to give the 'dweeno a chance to respond
  bytes_read = read(sockets[index], &buf[total_bytes], sizeof(buf)-total_bytes);
  int tri = 3;
  while (bytes_read == 0 && tri-- > 0) { //Needs more time?
    sleep(1);
    bytes_read = read(sockets[index], &buf[total_bytes], sizeof(buf)-total_bytes);
  }
    
  while (bytes_read > 0) {
    total_bytes += bytes_read;
    if (check_for(buf,total_bytes,"end_of_data.\r\n")) // Quit reading
      break;
    if (check_for(buf,total_bytes,"end_of_data\r\n")) // Quit reading
      break;
    sleep(1);  // Give the guy a chance to respond fully
    bytes_read = read(sockets[index],&buf[total_bytes], sizeof(buf)-total_bytes);
  }
  if (bytes_read == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
    sprintf(buf,"timeout(%d).\r\nend_of_data\r\n", index);
  //  fprintf(db,"REPLY[%s]\n",buf);
  return PL_unify_string_nchars(r, total_bytes, buf);
}

foreign_t
pl_bluetooth_socket(term_t mac, term_t n)
{
  int s;
  char *dest;
  struct timeval timeout;      
  timeout.tv_sec = 10;
  timeout.tv_usec = 0;
  //  if (db == (FILE *)NULL) {
  //    db = fopen("dbg.txt","w");
  //    setbuf(db,NULL);
  //  }

  if (!PL_is_atom(mac)) PL_fail;

  PL_get_atom_chars(mac,&dest);
  s = bluetoothSocket(dest);
  if (s == -1)
    PL_fail;
  if (PL_unify_integer(n, (intptr_t)next_socket) == FALSE)
    PL_fail;
  sockets[next_socket++] = s;

  if (setsockopt (s, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
    {
      error("setsockopt read timeout failed\n");
      PL_fail;
    }
  if (setsockopt (s, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
    {
      error("setsockopt send timeout failed\n");
      PL_fail;
    }

  PL_succeed;
}
/* Returns file descriptor for a Bluetooth connection */

#define NETDOWN    10050L
#define NETUNREACH 10051L
#define NETTIMEOUT 10060L

int bluetoothSocket(char *dest) {
  int lasterror;
  char buf[100];
  int tries = 10;
  int once;
  if (plblueonce == 0) { plblueonce = 1; initialize;}
#ifdef WINDOWS
    mystr2ba( dest, &addr.rc_bdaddr );
#else
    str2ba( dest, &addr.rc_bdaddr );
#endif
  btport(1);
#ifdef WINDOWS
  int s = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
#else
  int s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
#endif

  if (s == -1) {
    PL_warning("Failed to create a Bluetooth socket");
    notrace();
    return(-1);
  }
  lasterror = 0;
  once = 1;
  while ( lasterror != NETDOWN &&
	  connect(s, (struct sockaddr *)&addr, sizeof(addr)) && 0 < tries-- ) {
#ifdef WINDOWS
    lasterror = WSAGetLastError();
    if (lasterror == NETDOWN) {
      PL_warning("NETWORK DOWN: Is your Bluetooth receiver installed?");
      notrace();
      close(s);
      s = -1;
      break;
    } else if (lasterror == NETUNREACH || lasterror == NETTIMEOUT) {
      if (once) {
	PL_warning("Bluetooth client(%s) hasn't responded.", dest);
        once = 0;
      }
   } else
      PL_warning("connect returned non zero %s %s", dest, strerror(lasterror));
#else
    PL_warning("connect to (%s) failed: %s", dest, strerror(errno));
#endif
      notrace();
      close(s);
      s = -1;
      while(s == -1 && 0 < tries--) {
	sleep(0.2);
#ifdef WINDOWS
	s = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
#else
	s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
#endif
      }
      if (lasterror != NETDOWN) {
	PL_warning("Bluetooth connection failed. Retry(%d) with new socket",
		   5-(tries/2));
	notrace();
      }
  }
  //  PL_warning("after connect");
  if (tries < 2) {
    if (s != -1) close(s);
    return -1;
  }
  return s;
}
static inline
void cp_net_order(char * to, char * from, int size)  /* must be a power of 2 */
{ 	register int i = 0, j = 0;

#ifdef WORDS_BIGENDIAN
	j = size - 1;
#endif
	for(i = 0; i < size; i++)
		to[i] = from[i ^ j];
}

foreign_t pl_float_codes(term_t Number, term_t Codes)
{
	union
		{
		float asNumber;
		char asCodes[sizeof(float)];
		} val, val1;

	char *data;
	size_t len;
	double tmp;

	if(PL_get_float(Number, &tmp))
		{ val.asNumber = (float)tmp;

		cp_net_order(val1.asCodes, val.asCodes, sizeof(val1.asCodes));

		return PL_unify_list_ncodes(Codes, sizeof(val1.asCodes), val1.asCodes);
		}

	if(PL_get_list_nchars(Codes, &len, &data, CVT_LIST)
			&& len == sizeof(val.asCodes))
		{ cp_net_order(val.asCodes, data, sizeof(val.asCodes));

		tmp = val.asNumber;

		return PL_unify_float(Number, tmp);
		}

        PL_warning("float_codes/2: Instatiation error");
        PL_fail;
}

static PL_extension predicates [] =
{
  { "bt_socket",    2, pl_bluetooth_socket, 0 },
  { "bt_converse",  3, pl_converse,         0 },
  { "bt_scan",      2, pl_scan,             0 },
  { "bt_close",     1, pl_bt_close,         0 },
  { "bt_areset",    1, pl_areset,           0 },
  { "bt_reset",     0, pl_bt_reset,         0 },
  { "float_codes",  2, pl_float_codes,      0 },
  { "float_codes",  3, dcg_float_codes,     0 },
  { NULL, 0, NULL, 0 } /* terminator */
};

install_t install_plblue() { PL_load_extensions(predicates); }
install_t install() { PL_load_extensions(predicates); }

