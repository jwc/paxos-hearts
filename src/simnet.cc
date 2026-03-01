#include "header.hh"

Simnet::Simnet(char * str) : Networking(str) {
  nNodes = nodeID = 0;

  for (int i = 0; i < NET_MAX_CONNECTIONS; i++)
    ids[i] = i;

  netIsLive = 1;
  for (int i = 0; i < NET_MAX_CONNECTIONS; i++)
    for (int j = 0; j < NET_MAX_CONNECTIONS; j++)
      ruleMatrix[i][j] = 0;

  //deliveryThread = std::thread(&Simnet::delivery, this);
  for (int i = 0; i < 100; i++) 
    deliveryThread[i] = std::thread(&Simnet::delivery, this);
}

void Simnet::delivery() {
  while (1) {
    Message * msg = sendQueue.consume();

    //fprintf(stderr, "delivery(%d, %d, %d)\n", 
    //    msg->recipient, msg->type, msg->val.raw);

    if (msg->recipient == NET_BROADCAST) {
      // Broadcast
      //fprintf(stderr, " broadcast\n");

      //for (int i = 0; i < nNodes; i++) {
      for (int i = 0; i < NET_MAX_CONNECTIONS; i++) {
        if (i == msg->sender) continue;
        if ((rand() % 100) < ruleMatrix[msg->sender][i]) continue;
        Message * copy = NEW_MSG();
        memcpy(copy, msg, MSG_SIZE);
        /*
        copy->sender = msg->sender;
        copy->recipient = msg->recipient;
        copy->type = msg->type;
        copy->ballotNo = msg->ballotNo;
        copy->logPendingStart = msg->logPendingStart;
        copy->logPendingEnd= msg->logPendingEnd;
        copy->slot = msg->slot;
        copy->val = msg->val;
        for (int i = 0; i < PAXOS_MAX_PENDING; i++) {
          copy->ballots[i] = msg->ballots[i];
          copy->vals[i] = msg->vals[i];
        }
        */
        assert(copy->val.raw== msg->val.raw);
        assert(copy->logPendingEnd == msg->logPendingEnd);
        buffers[i].produce(copy);
      }
      DEL_MSG(msg);
      /*
      if (msg->type == ACCEPTED_M) {
        int blank = 0;
        for (int i = 0; i < msg->logPendingEnd - msg->logPendingStart; i++)
          if (msg->ballots[i] == 0) {
            fprintf(stderr, "NET: bad ACCEPTMSG all<-%d\n", msg->sender);
            assert(0);
          }
        buffers[msg->sender].produce(msg);
      }
      */

    } else {
      // Normal send
      //fprintf(stderr, " norm send to %d\n", msg->recipient);

      buffers[msg->recipient].produce(msg);
    }
  }
}

void Simnet::addNode(char * str) { ; }

void Simnet::send(int dst, Message * msg) { sendQueue.produce(msg); }

Buffer<Message *> & Simnet::getBuffer() { 
  fprintf(stderr, "buf:%d\n", nodeID);
  return buffers[nodeID - 1]; 
}

const int & Simnet::getN() {
  fprintf(stderr, "nNodes:%d\n", nNodes);
  return nNodes;
}

const int & Simnet::getID() { 
  nNodes++; 
  assert(nNodes <= NET_MAX_CONNECTIONS);
  fprintf(stderr, "id:%d\n", ids[nodeID]);
  return ids[nodeID++];
}
