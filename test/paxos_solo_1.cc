#include "header.hh"

#define N 4
#define M 10 

/*
 * Network: 1 real nodes, no message loss.
 * Action: Have nodes make 10 requests.
 * Expectaion: Should see all real nodes confirm 10 values.
 */
int main(int argc, char **argv) {
  Simnet *net = new Simnet(NULL);
  
  Paxos *pax;
  pax = new Paxos(*net); 

  sleep(1); // Giving time for leader election.
  Value v;
  srand((unsigned long int) &v);
  for (int i = 0; i < M; i++) { // Requesting M messages.
    v.type = rand() % 255;
    v.data = rand() % 255;
    pax->makeRequest(v);
  }
  sleep(1);

  //Verifying all nodes confirmed all messages.
  if (pax->getBuffer().count() != M) {
    fprintf(stderr, "main: missing values.\n");
    return 1;
  }

  fprintf(stderr, "main: good\n");
  return 0;
}

