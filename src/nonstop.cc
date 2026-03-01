#include "header.hh"

#define N 3
#define M 3 

int main(int argc, char **argv) {
  Simnet * net = new Simnet(NULL);
  
  fprintf(stderr, "main running %d\t\n", net->getBuffer(0).count());

  Paxos * pax[N];
  for (int i = 0; i < N; i++)
    pax[i] = new Paxos(*net); 

  sleep(1);
  Value v;
  srand((unsigned long int) &v);
  int x = 0;
  while (1) {
    fprintf(stderr, "main running %d:%d\t\n", x, net->getBuffer(x).count());
    fprintf(stderr, "running %d:%d\t\n", x, pax[x]->getBuffer().count());
    x = (x + 1) % net->getN();

    for (int i = 0; i < M; i++) {
      v.type = rand() % 255;
      v.data = rand() % 255;
      pax[rand()%N]->makeRequest(v);
    }

    sleep(1);
  }

  return 0;
}


