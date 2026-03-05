#include "header.hh"

#define N 4
#define M 10 

/*
 * Network: 3 real nodes, 1 fake node, no message loss.
 * Action: Have nodes make 10 requests.
 * Expectaion: Should see all real nodes confirm 10 values.
 */
int main(int argc, char **argv) {
  Simnet *net = new Simnet(NULL);
  net->addFakeNode();
  
  Paxos *pax[N-1];
  for (int i = 0; i < N-1; i++)
    pax[i] = new Paxos(*net); 

  sleep(1); // Giving time for leader election.
  Value v;
  srand((unsigned long int) &v);
  for (int i = 0; i < M; i++) { // Requesting M messages.
    v.type = rand() % 255;
    v.data = rand() % 255;
    pax[rand()%(N-1)]->makeRequest(v);
  }
  sleep(1);

  //Verifying all nodes confirmed all messages.
  int retval = 0;
  for (int i = 0; i < N-1; i++) 
    if (pax[i]->getBuffer().count() != M) {
      retval++;
      fprintf(stderr, "main: missing values.\n");
    }
  if (retval == 0) fprintf(stderr, "main: good\n");
  return retval;
}

