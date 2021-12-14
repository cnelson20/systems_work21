#include "pipe_networking.h"


/*=========================
  server_handshake
  args: int * to_client

  Performs the client side pipe 3 way handshake.
  Sets *to_client to the file descriptor to the downstream pipe.

  returns the file descriptor for the upstream pipe.
  =========================*/
int server_handshake(int *to_client) {
  int from_client = 0;
  int dtlength;
  char *buffer;
  
  mkfifo(WKP,0600);

  int w_k_p = open(WKP,O_RDONLY);
  remove(WKP);

  read(w_k_p,&dtlength,sizeof(int));
  buffer = malloc(dtlength);
  read(w_k_p,buffer,dtlength);

  //printf("FIFO to send data to client: %s\n",buffer);

  close(w_k_p);
  
  char randname[5];
  randname[4] = '\0';
  int urandom = open("/dev/urandom",O_RDONLY);
  read(urandom,randname,4);
  for (urandom = 0; urandom < 4; urandom++) {
	  randname[urandom] = (randname[urandom] & 0x0F) + 0x41;
  }
  mkfifo(randname,0600);
  
  //printf("New  readpipe to send to client: %s\n",randname);
  
  *to_client  = open(buffer,O_WRONLY);
  dtlength = 5;
  write(*to_client,&dtlength,sizeof(int));
  write(*to_client,randname,dtlength);
  
  int spcwkp = open(randname,O_RDONLY);
  remove(randname);
  
  close(w_k_p);
  
  free(buffer);
  
  from_client = spcwkp;
  
  // do acknowledge
  int ack[2];
  LabelHere:
  read(from_client,&(ack[0]),sizeof(int));
  //printf("d from client: %d\n",ack[0]); 
  ack[0]++;
  read(urandom,&(ack[1]),sizeof(int));
  //printf("d w to client: %d %d\n",ack[0],ack[1]);
  write(*to_client,ack,2*sizeof(int));
  read(from_client,&(ack[0]),sizeof(int));
  if (ack[0] == ack[1] + 1) {
    //printf("d from client: %d\n",ack[0]); 
  } else {
	goto LabelHere;
  }
  //printf("to_client: %d\nfrom_client: %d\n",*to_client,from_client);
  return from_client;
}


/*=========================
  client_handshake
  args: int * to_server

  Performs the client side pipe 3 way handshake.
  Sets *to_server to the file descriptor for the upstream pipe.

  returns the file descriptor for the downstream pipe.
  =========================*/
int client_handshake(int *to_server) {
  int from_server = 0;
  char pidstring[12];
  int dtlength;
  
  int w_k_p = open(WKP,O_WRONLY);
  snprintf(pidstring,12,"%d",getpid());

  mkfifo(pidstring,0600);

  //printf("FIFO to send to server: %s\n",pidstring);

  dtlength = strlen(pidstring)+1;
  write(w_k_p,&dtlength,sizeof(int));
  write(w_k_p,pidstring,dtlength);

  close(w_k_p);
  
  int secp = open(pidstring,O_RDONLY);
  remove(pidstring);
  
  read(secp,&dtlength,sizeof(int));
  read(secp,pidstring,dtlength);
  
  *to_server = open(pidstring,O_WRONLY);
  
  //printf("new write to server pipe: %s\n",pidstring);
  
  from_server = secp;
  
  int urandom = open("/dev/urandom",O_RDONLY);
  // Do acknowledge
  int ack[2];
  Label:
  read(urandom,&(ack[0]),sizeof(int));
  //printf("d to server: %d\n",ack[0]);
  write(*to_server,&(ack[0]),sizeof(int));
  read(from_server,&(ack[1]),sizeof(int));
  if (ack[1] == ack[0] + 1) {
    ack[0] = ack[1];
    read(from_server,&(ack[1]),sizeof(int));
    //printf("d r from server: %d %d\n",ack[0],ack[1]);
    ack[1]++;
    write(*to_server,&(ack[1]),sizeof(int));
    //printf("d w to server p2: %d\n",ack[1]);
  } else {
    read(from_server,&(ack[0]),sizeof(int));
    goto Label;
  }
  //printf("to_server: %d\nfrom_server: %d\n",*to_server,from_server);
  return from_server;
}
