#include "header.hh"

#define N 4
#define M 10 

/*
 * Network: 4 real nodes, 10% message loss.
 * Action: Make 10 requests.
 * Expectaion: Should see all nodes have 4 confirmed values. 
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
  for (int i = 0; i < N; i++) 
    if (pax[i]->getBuffer().count() == 0) {
      retval++;
      fprintf(stderr, "main: missing values.\n");
    }

  if (retval == 0) fprintf(stderr, "main: good\n");
  return retval;
}

