#include "header.hh"

#define N 4

/*
 * Network: 1 real node, 3 fake nodes, no message drops.
 * Action: A fake accepted message is broadcast.
 * Expectaion: The real node should not output the sent value to the buffer.  
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
  msg->type = ACCEPTED_M;
  msg->ballotNo = N*10;
  msg->logPendingStart = 0;
  msg->logPendingEnd = 1;
  msg->vals[0] = v1;
  msg->ballots[0] = N*10;
  net->send(NET_BROADCAST, msg);
  
  Paxos *pax = new Paxos(*net);

  sleep(1);

  if (pax->getBuffer().count() != 0) {
    fprintf(stderr, "main: value in buffer.\n");
    return 1;
  }

  fprintf(stderr, "main: good.\n");
  return 0;
}

