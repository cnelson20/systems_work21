#include "pipe_networking.h"

int strtoupper(char *string) {
  while (*string) {
    if (*string >= 'a' && *string <= 'z') {
      *string -= ('a' - 'A');
    }
    string++;
  }
}

int main() {

  int to_client;
  int from_client;

  char *data_to_process;
  int data_length;
  
  while (1) {
    from_client = server_handshake( &to_client );
    while (1) {
      if (read(from_client,&data_length,sizeof(int)) == 0) {break;}
      data_to_process = malloc(data_length);

      if (read(from_client,data_to_process,data_length) == 0) {
	free(data_to_process);
	break;
      }
      strtoupper(data_to_process);
      write(to_client,&data_length,sizeof(int));
      write(to_client,data_to_process,data_length);

      free(data_to_process);
    }
  }
}
