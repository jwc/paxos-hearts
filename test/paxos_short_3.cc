#include "header.hh"

#define N 4
#define M 10 

/*
 * Network: 2 real node, 2 fake node, no message drop. 
 * Action: Have real node make requests.
 * Expectation: No values should be confirmed due to lack of consensus.
 */
int main(int argc, char **argv) {
  Simnet *net = new Simnet(NULL);
  for (int i = 0; i < N-2; i++)
    net->addFakeNode();
  
  Paxos *pax[N-2];
  for (int i = 0; i < N-2; i++)
    pax[i] = new Paxos(*net); 

  sleep(1); // Giving time for leader election.
  Value v;
  srand((unsigned long int) &v);
  for (int i = 0; i < M; i++) { // Requesting M messages.
    v.type = rand() % 255;
    v.data = rand() % 255;
    pax[rand()%(N-2)]->makeRequest(v);
  }
  sleep(1);

  //Verifying all nodes confirmed all messages.
  for (int i = 0; i < (N-2); i++) 
    if (pax[i]->getBuffer().count() != 0) {
      fprintf(stderr, "main: value confirmed by minority.\n");
      return 1;
    }
  fprintf(stderr, "main: good\n");
  return 0;
}

