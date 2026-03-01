#ifndef SIMNET_HH
#define SIMNET_HH

class Simnet : public Networking {
  private: 
    int               ids[NET_MAX_CONNECTIONS]; 
    FLBuffer<Message *> buffers[NET_MAX_CONNECTIONS];
    FLBuffer<Message *> sendQueue;
    //std::thread       deliveryThread;
    std::thread       deliveryThread[100];

    char              ruleMatrix[NET_MAX_CONNECTIONS][NET_MAX_CONNECTIONS];
    int               netIsLive;

    /**
     * NOTE: Can only deliver a message if: 
     *  A) netIsLive == 1
     *  B) ruleMatrix[sender][recipient] == 1
     *  * also there's a [messageDropChance]% failure rate
     */

  public:
    Simnet(char *);

    void delivery();

    void addNode(char *);

    void send(int, Message *);

    Buffer<Message *> & getBuffer(); 

    const int & getN() override; 

    const int & getID() override; 

    Buffer<Message *> & getBuffer(int i) { return buffers[i]; }

    // Functions to alter network properties:
    
    void addFakeNode() {
      nodeID++;
      nNodes++;
    }

    void setNetStatus(int status) { netIsLive = status; }

    void setPath(int from, int to, char dropChance) { 
      if (from < 0 || from >= NET_MAX_CONNECTIONS
          || to < 0 || to >= NET_MAX_CONNECTIONS) return;
      if (dropChance < 0) dropChance = 0;
      else if (dropChance > 100) dropChance = 100;

      ruleMatrix[from][to] = dropChance; 
    }

    int getNetStatus() { return netIsLive; }

    int getPath(int from, int to) { 
      if (from < 0 || from >= NET_MAX_CONNECTIONS
          || to < 0 || to >= NET_MAX_CONNECTIONS) return 0;
      return ruleMatrix[from][to]; 
    }

    void disconnectNode(int id) {
      if (id < 0 || id >= NET_MAX_CONNECTIONS) return;
      for (int i = 0; i < NET_MAX_CONNECTIONS; i++) {
        if (i == id) continue;
        ruleMatrix[id][i] = ruleMatrix[i][id] = 0;
      }
    }

    void reconnectNode(int id) {
      if (id < 0 || id >= NET_MAX_CONNECTIONS) return;
      for (int i = 0; i < NET_MAX_CONNECTIONS; i++) {
        if (i == id) continue;
        ruleMatrix[id][i] = ruleMatrix[i][id] = 100;
      }
    }
};

#endif // SIMNET_HH

