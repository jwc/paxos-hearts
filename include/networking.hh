#ifndef NETWORKING_HH
#define NETWORKING_HH

/**
 * Interface for all networking classes.
 *
 */

class Networking {
  public:
    explicit Networking(char *) { ; };

    virtual ~Networking() { ; }

    /**
     * @brief Used to inform object of another node to connect to. 
     *
     * @param id The identifier for the other node. 
     */
    virtual void addNode(char * id) = 0;

    /**
     * @brief Sends a message.
     *
     * @param dst The node to recieve the message.
     * @param msg The message being sent.
     */
    virtual void send(int dst, Message * msg) = 0; 

    /**
     * @brief Gives a reference to the node's unique identifier on the network.
     *
     * @return A unique identifier for the network.
     */
    virtual const int & getID() { return nodeID; } 
    
    /**
     * @brief Gives a reference to count of all the nodes on the network.
     *
     * @return The number of nodes on the network.
     */
    virtual const int & getN() { return nNodes; }

    /**
     * @brief Gives a reference to where recieved messages are placed.
     *
     * @return The buffer containing incoming messages.
     */
    virtual Buffer<Message *> & getBuffer() = 0;

  protected:
    int nNodes;
    int nodeID;

    explicit Networking() { ; };
};

#endif // NETWORKING_HH

