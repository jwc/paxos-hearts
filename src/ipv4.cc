#include "header.hh"

// Public:

IPv4::IPv4(char * addrAndPort) : Networking(addrAndPort) {
  nNodes = nodeID = 0;
  addNode(addrAndPort);
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

  listenThread = std::thread(&IPv4::listener, this, listenSocket); 
  if(debug)fprintf(stderr, "{%d} Listener Created\n", nodeID);

  lock.unlock();
}

void IPv4::addNode(char * addrAndPort) {
  lock.lock();

  // Check for space.
  if (nNodes == NET_MAX_CONNECTIONS) { 
    lock.unlock(); 
    //return -1; 
    return;
  }

  // Place at end of list. 
  MyAddress address;
  int16_t port;
  if (sscanf(addrAndPort, "%hhd.%hhd.%hhd.%hhd:%hd", &address.b1, 
                &address.b2, &address.b3, &address.b4, &port) != 5) {
    fprintf(stderr, "{%d} addNode() sscanf() fialede.\n", nodeID);
    exit(2);
  }

  fprintf(stderr, "{%d} adding %d.%d.%d.%d:%d\n", nodeID, address.b1,
      address.b2, address.b3, address.b4, ntohs(port));

  ips[nNodes] = address.address;
  ports[nNodes] = htons(port);
  sockets[nNodes] = -1;
      
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
      //return -1;
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
  //return atoiVal;
  return;
}

void IPv4::send(int dst, Message * msg) {
  lock.lock();

  if (dst == nodeID) {
    // send to self: move to incoming.
    if(debug)fprintf(stderr, "{%d} Send to Self\n", nodeID);
    incoming.produce(msg);

  } else if (dst >= 0 && dst < nNodes) {
    // simple send: send to recipient
    if(debug)fprintf(stderr, "{%d} Send to %d\n", nodeID, dst);
    send_helper(dst, (char *) msg);
    DEL_MSG(msg);

  } else {
    // broadcast: send to all, then add to incoming buffer.
    if(debug)fprintf(stderr, "{%d} Broadcast\n", nodeID);
    for (int i = 0; i < nNodes; i++) if (i != nodeID)
      send_helper(i, (char *) msg);
    incoming.produce(msg);
  }

  lock.unlock();
}

// Private:

void IPv4::receiver() {
  Message * msg    = NEW_MSG(); 
  int       socket = receiverSockets.consume();
  int       from   = -1; 
  int       duped  = 0;

  fprintf(stderr, "{%d} socket %hhd opened. \n", nodeID, socket);
  while (1) {
    if (read(socket, msg, MSG_SIZE) != MSG_SIZE) {
      close(socket);
      fprintf(stderr, "{%d} socket %hhd closed. \n", nodeID, socket);
      
      socket = receiverSockets.consume();
      duped = 0;
      fprintf(stderr, "{%d} socket %hhd opened. \n", nodeID, socket);
      
    } else {
      from = msg->sender;
      incoming.produce(msg);
      msg = NEW_MSG();

      /*
      lock.lock();
      if (duped == 0 && from > 0 && from < nNodes 
          && from != nodeID && sockets[from] != socket) {
        fprintf(stderr, "{%d} sockets %hhd %hhd combined.\n", nodeID, 
            socket, sockets[from]);
        duped = 1;

        //if (sockets[from] < 0)
          //sockets[from] = dup(socket);
        //else if (from < nodeID)
          //dup2(socket, sockets[from]);
        //else 
          //dup2(sockets[from], socket);
      }
      lock.unlock();
      */
    }
  }

  if(debug)fprintf(stderr, "{%d} receiver() unreachable point\n", nodeID);
}

void IPv4::listener(int listenSocket) {
  std::thread         receivers[NET_MAX_CONNECTIONS];
  for (int i = 0; i < NET_MAX_CONNECTIONS; i++) 
    receivers[i] = std::thread(&IPv4::receiver, this);
  
  // No locking needed. Only writing to using local variables.

  while (1) {
    struct sockaddr_in  respAddress;
    int                 alen = sizeof( respAddress );
    int                 respSocket = accept( listenSocket, 
                                (struct sockaddr *) &respAddress, 
                                (socklen_t*) &alen);
    if (respSocket < 0) {
      printf("Accept\n");
      fprintf(stderr, "EAGAIN:%d\nEBADF:%d\nECONNABORTED:%d\nEINTR:%d\nEINVAL:%d\nEMFILE:%d\nENFILE:%d\nENOBUFS:%d\nENOMEM:%d\nENOTSOCK:%d\nEOPNOTSUPP:%d\n", EAGAIN, EBADF, ECONNABORTED, EINTR, EINVAL, EMFILE, ENFILE, ENOBUFS, ENOMEM, ENOTSOCK, EOPNOTSUPP);
      fprintf(stderr, "ERRNO:%d\n", errno);
      exit(1);
    }

    fprintf(stderr, "{%d} socket %d added.\n", respSocket);
    receiverSockets.produce(respSocket);
  }
}

void IPv4::send_helper(int dst, char * msg) {
  // Assumed to already be locked

  if(debug)fprintf(stderr, "{%d} SEND:\n", nodeID);
  if (dst < 0 || dst >= nNodes) return;

  if (dst == nodeID) {
    //TODO: send to self
    return;
  }

  if(debug)fprintf(stderr, "{%d} SND: A\n", nodeID);

  if (sockets[dst] < 0) connect(dst);

  if (::send(sockets[dst], msg, MSG_SIZE, 0) != MSG_SIZE) {
    if (sockets[dst] >= 0) disconnect(dst);
    if (connect(dst) == 0) ::send(sockets[dst], msg, MSG_SIZE, 0);
  }

  return;
}

int IPv4::connect(int id) {
  if (id < 0 || id > nNodes || id == nodeID) 
    return 1;
  if (sockets[id] >= 0) 
    return 2;

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = ips[id];
  address.sin_port = ports[id];
  sockets[id] = socket(PF_INET, SOCK_STREAM, 0); // -1 on error

  if (::connect(sockets[id], (const sockaddr*) 
      &address, sizeof(address)) == -1) {
    close(sockets[id]);
    sockets[id] = -1;
    //fprintf(stderr, "{%d} connect fail %d\n", nodeID, sockets[id]);
    return 3;
  }
        
  fprintf(stderr, "{%d} connect %d\n", nodeID, sockets[id]);
  return 0;
}

int IPv4::disconnect(int id) {
  if (id < 0 || id > nNodes || id == nodeID)
    return -1;
  if (sockets[id] < 0) 
    return -2;

  fprintf(stderr, "{%d} disconnect %d\n", nodeID, sockets[id]);

  close(sockets[id]);
  sockets[id] = -1;
  return 0;
}

