#include "header.hh"

#define N 4
#define M 1000

/*
 * Network: 4 real nodes, no message loss.
 * Action: Have nodes make 1000 requests.
 * Expectaion: Should see all real nodes confirm 1000 values.
 */
int main(int argc, char **argv) {
  Simnet *net = new Simnet(NULL);
  int count = 0;
  
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

    for (int j = 0; j < N; j++) if (pax[j]->getBuffer().count() > 0) {
      pax[j]->getBuffer().consume();
      count++;
    }

    if (i % (PAXOS_MAX_PENDING - 1) == 0) sleep(1);
  }
  sleep(1);

  for (int j = 0; j < N; j++) while (pax[j]->getBuffer().count() > 0) {
    pax[j]->getBuffer().consume();
    count++;
  }

  //Verifying all nodes confirmed all messages.
  if (count != M * N) {
    fprintf(stderr, "main: missing values (%d)\n", count);
    return 1;
  }

  fprintf(stderr, "main: good\n");
  return 0;
}

