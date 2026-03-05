#include "header.hh"

#define N 4
#define M 10 

/*
 * Network: 4 nodes, 10% message loss.
 * Action: Make 10 requests.
 * Expectation: All confirmed values should be the same accross nodes.
 */
int main(int argc, char **argv) {
  Simnet *net = new Simnet(NULL);
  for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
      if (i != j) 
        net->setPath(i, j, 10);
  
  Paxos *pax[N];
  for (int i = 0; i < N; i++)
    pax[i] = new Paxos(*net); 

  sleep(1); // Giving time for leader election.
  Value v;
  srand((unsigned long int) &v);
  for (int i = 0; i < M; i++) { // Requesting M messages.
    v.type = rand() % 255;
    v.data = rand() % 255;
    pax[rand()%N]->makeRequest(v);
  }
  sleep(1); 

  //Verifying all nodes confirmed all messages.
  int retval = 0;
  int min = M;
  for (int i = 0; i < N; i++) 
    if (min > pax[i]->getBuffer().count()) 
      min = pax[i]->getBuffer().count();
  if (min == 0) {
    fprintf(stderr, "main: missing values.\n");
    return -1;
  }

  for (int i = 0; i < min; i++) {
    Value v1 = pax[0]->getBuffer().consume();
    for (int j = 1; j < N; j++) {
      Value v2 = pax[j]->getBuffer().consume();
      if (v1.raw != v2.raw) {
        retval++;
        fprintf(stderr, "main: not equal.\n");
      } else {
        fprintf(stderr, "main: %d:%d\n", v1.raw, v2.raw);
      }
    }
  }
  if (retval == 0) fprintf(stderr, "main: good\n");
  return retval;
}

