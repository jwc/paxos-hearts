#ifndef IPV4_HH
#define IPV4_HH

union MyAddress {
  uint32_t address;
  struct {
    uint8_t b1, b2, b3, b4;
  };
};

/**
 * A network object for connecting with other nodes via IPv4. 
 */

class IPv4 : public Networking {
  public:
    /**
     * @brief The constructor for a IPv4 network node. 
     *
     * @param addrAndPort The IP address and port to be used for accepting 
     *          incoming connections. Must be in the format "#.#.#.#:#"
     */
    explicit IPv4(char * addrAndPort); 

    ~IPv4() { ; };

    /**
     * @brief Used for adding nodes to the network.
     *
     * @param addrAndPort The IP address and port of the other node. 
     *          Must be in the format "#.#.#.#:#"
     */
    virtual void addNode(char * addrAndPort);

    /**
     * @brief Used to send messages to other nodes on the network.
     */
    void send(int, Message *);

    Buffer<Message *> & getBuffer() { return incoming; }

    void printSocks() {
      lock.lock();
      if(debug)fprintf(stderr, "{%d} Socks(%d):\n", nodeID, nNodes);
      for (int i = 0; i < nNodes + 1; i++) {
        if(debug)fprintf(stderr, "\t%d. %d, %d\n", i, ports[i], sockets[i]);
      }
      lock.unlock();
   }


  protected:
    std::mutex          lock;
    struct sockaddr_in  listenAddress;
    std::thread         listenThread;
    FLBuffer<int>       receiverSockets{1};
    FLBuffer<Message *> incoming{0};
    int                 ports[NET_MAX_CONNECTIONS + 1];
    int                 ips[NET_MAX_CONNECTIONS + 1];
    int                 sockets[NET_MAX_CONNECTIONS + 1];
    const int           debug = 0;

    explicit IPv4() : Networking() { ; };

    void receiver();

    void listener(int);

    void send_helper(int, char *);

    int connect(int id);

    int disconnect(int id);
};

#endif // IPV4_HH

