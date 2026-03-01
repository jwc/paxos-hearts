#include "header.hh"

Localhost::Localhost(char * port) : IPv4() {
  nNodes = nodeID = 0;
  addNode(port);
  lock.lock();

  for (int i = 0; i < NET_MAX_CONNECTIONS + 1; i++) sockets[i] = -1;

  memset(&listenAddress, 0, sizeof(listenAddress));
  listenAddress.sin_family = AF_INET;
  listenAddress.sin_addr.s_addr = ips[0]; // TODO: BIND TO LOCAL
  listenAddress.sin_port = ports[0];
  int listenSocket = socket(PF_INET, SOCK_STREAM, 0);

  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
    printf("Signal\n");
    exit(1);
  }
      
  int optVal = 1;
  if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(int)) < 0) {
    printf("Setsockopt-1\n");
    exit(1);
  }
      
  if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEPORT, &optVal, sizeof(int)) < 0) {
    printf("Setsockopt-2\n");
    exit(1);
  }
      
  if (listenSocket < 0) {
    printf("Socket\n");
    exit(1);
  }

  if (bind(listenSocket, (struct sockaddr *)&listenAddress, sizeof(listenAddress))) {
    printf("Bind\n");
    exit(1);
  }

  if (listen(listenSocket, 3)) {
    printf("Listen\n");
    exit(1);
  }

  listenThread = std::thread(&Localhost::listener, this, listenSocket); 
  if(debug)fprintf(stderr, "{%d} Listener Created\n", nodeID);

  lock.unlock();
}

void Localhost::addNode(char * port) {
  lock.lock();

  // Check for space.
  if (nNodes == NET_MAX_CONNECTIONS) { 
    lock.unlock(); 
    return;
  }

  // Place at end of list. 
  ports[nNodes] = atoi(port);
  ips[nNodes] = INADDR_ANY;
  sockets[nNodes] = -1;

  // Verify atoi() was successful.
  if (ports[nNodes] == 0) {
    lock.unlock();
    fprintf(stderr, "{%d} addNode() atoi() failed.\n", nodeID);
    exit(1);
    return;
  }

  // Check for duplicates.
  for (int i = 0; i < nNodes; i++) {
    if (ports[i] == ports[nNodes] && ips[i] == ips[nNodes]) {
      lock.unlock();
      fprintf(stderr, "{%d} addNode() dup.\n", nodeID);
      exit(3);
      return;
    }
  }

  // Resort array.
  for (int i = 0; i < nNodes; i++) {
    if (ips[i] == ips[nNodes] && ports[i] == ports[nNodes]) {
      //Error
      if(debug)fprintf(stderr, "{%d} UNREACHABLE\n", nodeID);
      lock.unlock();
      return;

    } else if (ips[i] > ips[nNodes] 
        || (ips[i] == ips[nNodes] && ports[i] > ports[nNodes])) {
      ips[i] ^= ips[nNodes];
      ips[nNodes] ^= ips[i];
      ips[i] ^= ips[nNodes];

      ports[i] ^= ports[nNodes];
      ports[nNodes] ^= ports[i];
      ports[i] ^= ports[nNodes];

      sockets[i] ^= sockets[nNodes];
      sockets[nNodes] ^= sockets[i];
      sockets[i] ^= sockets[nNodes];

      if (i == nodeID)
        nodeID = nNodes;
      else if (nNodes == nodeID)
        nodeID = i;
    } 
  }

  nNodes++;
  if(debug)fprintf(stderr, "{%d} Inserted.\n", nodeID);
  lock.unlock();
  return;
}

