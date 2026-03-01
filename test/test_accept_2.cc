#include "header.hh"

#define N 2

/*
 * Network: 1 real node, 1 fake nodes, no message drops.
 * Action: A fake accept message is broadcast.
 * Expectaion: The real node should output the sent value to the buffer.  
 */
int main(int argc, char **argv) {
  Simnet *net = new Simnet(NULL);
  for (int i = 0; i < N-1; i++) net->addFakeNode();

  srand((unsigned long int)&net);
  Value v1 = { .raw = rand() };

  Message *msg = NEW_MSG();
  memset(msg, 0, MSG_SIZE);
  msg->sender = 0;
  msg->recipient = NET_BROADCAST;
  msg->type = ACCEPT_M;
  msg->ballotNo = N*10;
  msg->logPendingStart = 0;
  msg->logPendingEnd = 1;
  msg->vals[0] = v1;
  msg->ballots[0] = N*10;
  net->send(NET_BROADCAST, msg);
  
  Paxos *pax = new Paxos(*net);

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

