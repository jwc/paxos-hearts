#include "header.hh"

#define N 4

/*
 * Network: 1 real node, 3 fake nodes, no message drops.
 * Action: The real node sends a bypass message to a node.
 * Expectaion: The real node should output the sent value to the buffer.  
 */
int main(int argc, char **argv) {
  Simnet *net = new Simnet(NULL);
  for (int i = 0; i < N-1; i++) net->addFakeNode();

  srand((unsigned long int)&net);
  Value v1 = { .raw = rand() };
  Paxos *pax = new Paxos(*net);
  pax->sendStatelessValue(v1);

  sleep(1);

  if (pax->getBuffer().count() != 1) {
    fprintf(stderr, "main: value not in buffer.\n");
    return 1;
  }

  Value v2 = pax->getBuffer().consume();
  if (v1.raw != v2.raw) {
    fprintf(stderr, "main: value incorrect. (%d, %d)\n", v1.raw, v2.raw);
    return 2;
  }

  fprintf(stderr, "main: good.\n");
  return 0;
}

