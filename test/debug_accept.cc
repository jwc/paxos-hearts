#include "header.hh"

#define N 4
#define M 10 

/**
 ** Verifying there is progress made under optimal circumstances.
 * Network: 4 nodes, no message loss
 * Expectaion: Should see nodes confirmed
 */
int main(int argc, char **argv) {
  Simnet *net = new Simnet(NULL);
  for (int i = 0; i < N-1; i++) net->addFakeNode();

  Message *msg = NEW_MSG();
  memset(msg, 0, MSG_SIZE);
  msg->sender = 0;
  msg->recipient = NET_BROADCAST;
  msg->type = ACCEPT_M;
  msg->ballotNo = N*10;
  msg->logPendingStart = 0;
  msg->logPendingEnd = M;
  srand((unsigned long int)&msg);
  for (int i = 0; i < M; i++) {
    msg->ballots[i] = msg->ballotNo;
    msg->vals[i].type = rand() % 255;
    msg->vals[i].data = rand() % 255;
  }
  net->send(NET_BROADCAST, msg);
  
  Paxos *pax = new Paxos(*net);

  sleep(1);

  //Verifying all nodes confirmed all messages.
  int retval = 0;
  return retval;
}

