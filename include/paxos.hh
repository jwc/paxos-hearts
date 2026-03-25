#ifndef PAXOS_HH
#define PAXOS_HH

// For use w/ votes. 
#define MARK(x, y)          x |= 1 << y
#define TALLY(x)            __builtin_popcount(x)
#define IS_MARKED(x, y)     x & (1 << y)
#define HAS_MAJORITY(x, y)  ( TALLY(x) * 2 ) > y   

//#define PAXOS_DEBUG

class Paxos {
  public:
    /**
     * @brief Creates a Paxos node, connected to the specified network.
     *
     * @param net The networking object for communicating with the other nodes.
     */
    Paxos(Networking & net);

    /**
     * @brief Passes a value for Paxos to attempt to confirm.
     *
     * If/when the value is confirmed, it will be found in the output buffer.
     *
     * @param request The value to be confirmed.
     */
    void makeRequest(Value request);

    /**
     * @brief Broadcasts value to all nodes, without consensus. 
     *
     * Value may be delivered out of order, or be dropped entirely. 
     *
     * @param value The value to be broadcast.
     */
    void sendStatelessValue(Value value);

    /**
     * @brief Gives a reference to the buffer of confirmed Values.
     *
     * @return A reference to the buffer of confirmed Values.
     */
    inline Buffer<Value> & getBuffer() { return outgoing; }

    /**
     * @brief Gives a reference to the paxos node id.
     *
     * @return A reference to the node id.
     */
    inline const int & getID() { return id; };


  private:
    Networking &        net;
    const int &         nNodes;
    const int &         id;
    Buffer<Message *> & incoming;
    FLBuffer<Value>     outgoing{1};

    std::mutex          lock;
    std::thread         timerThread;
    std::thread         handlerThread;

    int                 latestBallot;
    int                 intervalsFromLeader;
    int                 myBallot;
    int                 ballotVotes;

    Value               logValues[PAXOS_LOG_SIZE];
    int                 logVotes[PAXOS_LOG_SIZE];
    int                 logBallots[PAXOS_LOG_SIZE];
    int                 logPendingStart;      // first pending
    int                 logPendingEnd;        // first non-pending
    int                 logFirstFree;         // 

    const int           debug = 0;

    //FLBuffer<Message> outgoing;

#ifdef PAXOS_DEBUG
    double              timeHandler, timeTimer, timeIdle;
    std::chrono::time_point<std::chrono::high_resolution_clock> timeA, timeB;
#endif // PAXOS_DEBUG


    // Functions safe to call whenever:


    int nextFreeSlot() {
      for (int i = logPendingStart; i < logPendingStart + PAXOS_LOG_SIZE - 1; i++) 
        if (logBallots[i % PAXOS_LOG_SIZE] == 0) 
          return i;
      return -1;
    }
    int logAdd(int slot, int ballot, Value value) {
      if ( ! isInAlterableRange(slot)) {
        if(debug)fprintf(stderr, "[%d] (A)\n", id);
        return -1;
      }
      if (ballot == 0) {
        if(debug)fprintf(stderr, "[%d] ballot=0.\n", id);
        assert(0);
      }
      /*
      if (slot < logPendingStart) {
        fprintf(stderr, "[%d] (A)\n", id);
        return -1; 
      }
      if (slot >= logPendingStart + PAXOS_MAX_PENDING) {
        fprintf(stderr, "[%d] (B)\n", id);
        return -1; 
      }
      */

      if (logGetBallot(slot) > ballot) {
        if(debug)fprintf(stderr, "[%d] (C) lGB():%d b:%d\n", id, logGetBallot(slot), ballot);
        return -1; 
      }
      if (HAS_MAJORITY(logGetVote(slot), nNodes)) {
        if(debug)fprintf(stderr, "[%d] (D:%d)\n", id, slot);
        return -1; 
      }

      // One of three places that logPendingEnd is modified.
      while (logPendingEnd <= slot) {
        logVotes[logPendingEnd % PAXOS_LOG_SIZE] = 0;
        logBallots[logPendingEnd++ % PAXOS_LOG_SIZE] = 0;
      }
      if ( ! isInPendingRange(slot)) { 
        if(debug)fprintf(stderr, "[%d] OOR\n", id);
        assert(0);
      }

      logSetBallot(slot, ballot);
      logSetValue(slot, value);
      logSetVote(slot, 0);
      //logCastVote(slot, id);

      if (logBallots[slot] == 0) {
        if(debug)fprintf(stderr, "[%d] bad logAdd().\n", id);
        assert(logBallots[slot%PAXOS_LOG_SIZE] > 0);
      }

      return 0;
    }
    int logAddNew(int ballot, Value value) {
      if (ballot <= 0) return -1;
      if (logFirstFree < logPendingStart) logFirstFree = logPendingStart;
      while (logGetBallot(logFirstFree) > 0) logFirstFree++;
      if (logPendingStart + PAXOS_MAX_PENDING <= logFirstFree) return -1;

      // One of three places that logPendingEnd is modified.
      while (logPendingEnd <= logFirstFree) 
        logBallots[logPendingEnd++ % PAXOS_LOG_SIZE] = 0;

      logBallots[logFirstFree % PAXOS_LOG_SIZE] = ballot;
      logValues[logFirstFree % PAXOS_LOG_SIZE] = value;
      logVotes[logFirstFree % PAXOS_LOG_SIZE] = 0;
      logCastVote(logFirstFree, id);

      //if (logBallots[logFirstFree % PAXOS_LOG_SIZE] == 0) {
      //  fprintf(stderr, "[%d] bad logAddNew() ballot:%d\n", id, logBallots[logFirstFree]);
      //  assert(0);
      //}
      return logFirstFree;
    }
    int isOverwritten(int i)
    {return i + PAXOS_LOG_SIZE - 1 < logPendingEnd;}
    int isUnwritten(int i) { return i >= logPendingEnd; }
    int isConfirmed(int i) 
    { return logGetBallot(i) > 0 && HAS_MAJORITY(i, nNodes); }
    int isPending(int i) {
      if (logGetBallot(i) > 0 && ! HAS_MAJORITY(i, nNodes)) return 1;
      return 0;
    }
    int isInPendingRange(int i) 
    { return i >= logPendingStart && i < logPendingEnd; }
    int isInAlterableRange(int i) 
    { return i >= logPendingStart && i < logPendingStart + PAXOS_LOG_SIZE; }
    int isInLogRange(int i) 
      { return ! (isUnwritten(i) || isOverwritten(i)); }
    int logGetBallot(int i) { 
      if (isOverwritten(i)) { 
        if(debug)fprintf(stderr, "[%d]\t\tOvrwrttn BALLOT %d : %d\n", id, i, ~0);
        return ~0;
      }
      if (isUnwritten(i)) {
        if(debug)fprintf(stderr, "[%d]\t\tNEW BALLOT %d : 0\n", id, i);
        return 0;
      }
      if(debug)fprintf(stderr, "[%d]\t\tRAW BALLOT %d : %d\n", id, i, logBallots[i]);
      return logBallots[i % PAXOS_LOG_SIZE];
    }
    Value logGetValue(int i) {
      //if (isOverwritten(i)) return (Value) 0;
      //else if (isUnwritten(i)) return NULL;
      return logValues[i];
    }
    int logGetVote(int i) { 
      if (isOverwritten(i)) return ~0;
      else if (isUnwritten(i)) return 0;
      else return logVotes[i % PAXOS_LOG_SIZE];
    }
    void logSetBallot(int i, int ballot) {
      if (logGetBallot(i) > ballot) {
        if(debug)fprintf(stderr, "[%d] bad logSetBallot() log:%d bal:%d\n", id, logGetBallot(i), ballot);
        assert(0);
      }
      if (isInPendingRange(i)) logBallots[i % PAXOS_LOG_SIZE] = ballot;
      /*if (i < logPendingStart)
        ; // No longer in storage
      else
        logBallots[i % PAXOS_LOG_SIZE] = ballot;*/
    }
    void logSetValue(int i, Value c) {
      if (isInPendingRange(i)) logValues[i % PAXOS_LOG_SIZE] = c;
      /*if (i < logPendingStart)
        ; // No longer in storage
      else
        logValues[i % PAXOS_LOG_SIZE] = c;*/
    }
    void logSetVote(int i, int votes) {
      if (isInPendingRange(i)) logVotes[i % PAXOS_LOG_SIZE] = votes;
      /*if (i < logPendingStart)
        ; // No longer in storage
      else  
        logVotes[i % PAXOS_LOG_SIZE] = votes;*/
    }
    void logCastVote(int i, int id) {
      if (isInPendingRange(i)) logVotes[i % PAXOS_LOG_SIZE] |= 1 << id;
    }
    int logHaveVoted(int i, int id) {
      if (isOverwritten(i)) return 1;
      if (isUnwritten(i)) return 0;
      return logVotes[i % PAXOS_LOG_SIZE] | (1 << id); 
    }

    int advancePendingRange() {
      int retval = 0;
      while (isInPendingRange(logPendingStart) 
          && HAS_MAJORITY(logGetVote(logPendingStart), nNodes)) {
        //Value * val = (Value *) malloc(sizeof(Value));
        //memcpy(val, &logValues[logPendingStart % PAXOS_LOG_SIZE], sizeof(Value));
        Value val = logValues[logPendingStart % PAXOS_LOG_SIZE];
        outgoing.produce(val);

        // The only place that logPendingStart is written to after init.
        logPendingStart++;
        retval = 1;
      }
      return retval;
    }

    void timer();

    void handler();

    void handlePrepare(Message *);

    void handlePromise(Message *);

    void handleAccept(Message *);

    void handleAccepted(Message *);

    void handleLiveness(Message *);

    void handleRequest(Message *);
    
    void handleConfirm(Message *);

    void handleBypass(Message *);

    void sendUnknownConfirms(int address, int rangeStart) {
      //return;
      // Updating Leader of any unknown confirmed msgs.
      if (id == address) return;

      if(debug)fprintf(stderr, "[%d] sUC(%d, %d) lPS:%d lPE:%d\n", id, address, 
          rangeStart, logPendingStart, logPendingEnd);

      for (int logi = rangeStart; logi < logPendingStart;) {
        if(debug)fprintf(stderr, "[%d] sending Confirms to %d\n", id, address);

        Message * confirmMsg = NEW_MSG();
        confirmMsg->sender = id;
        confirmMsg->recipient = address;
        confirmMsg->type = CONFIRM_M;
        confirmMsg->ballotNo = latestBallot;
        //confirmMsg->logPendingStart = logPendingStart;
        //confirmMsg->logPendingEnd = logPendingEnd;
        confirmMsg->logPendingStart = logi;
        confirmMsg->logPendingEnd = logi;
        for (int msgi = 0; msgi < PAXOS_MAX_PENDING && logi < logPendingEnd; 
            logi++, msgi++) {
          if (HAS_MAJORITY(logGetVote(logi), nNodes)) {
            confirmMsg->ballots[msgi] = logGetBallot(logi);
            confirmMsg->vals[msgi] = logGetValue(logi);
            confirmMsg->logPendingEnd++;
          } else {
            confirmMsg->ballots[msgi] = 0;
          }
        }
        net.send(confirmMsg->recipient, confirmMsg);
      }
    }

    int getLeaderID() { return latestBallot % nNodes; }
    
    int sendMessage(int address, int type, Value value) { 
      if (type != REQUEST_M && type != BYPASS_M) return 1;

      Message * msg = NEW_MSG();
      msg->sender = id;
      msg->recipient = address;
      msg->type = type;
      msg->ballotNo = latestBallot;

      msg->logPendingStart = logPendingStart;
      msg->logPendingStart = logPendingEnd;

      msg->val = value;
      assert(msg->val.raw = value.raw);

      if(debug)fprintf(stderr, "sendMessage(%d, %d, %d)\n", address, type, value.raw);
      net.send(address, msg);
      return 0; 
    }
    
    int sendMessage(int address, int type, int start, int end) {
      //if (isUnwritten(start) || isUnwritten(end)) return 1;
      if (end - start > PAXOS_MAX_PENDING) return 2;
      //if (type == PREPARE_M || type == LIVENESS_M) end = start;

      Message * msg = NEW_MSG();
      memset(msg, 0, MSG_SIZE);
      msg->sender = id;
      msg->recipient = address;
      msg->type = type;
      msg->ballotNo = latestBallot;
      //msg->logPendingStart = logPendingStart;
      //msg->logPendingEnd= logPendingEnd;
      msg->logPendingStart = start;
      msg->logPendingEnd = end;

      int full = 1;
      int nonblank = 0;
      int badballot = 0;
      int checkVotes = 0;
      switch (type) {
        case REQUEST_M:
        case BYPASS_M:
          return 3;

        case CONFIRM_M:
          checkVotes = 1;
        case PROMISE_M:
        case ACCEPT_M:
        case ACCEPTED_M:
          // There can be multiple items.
          for (int i = 0, j = start; j < end; i++, j++) {
            //if (checkVotes && ! isConfirmed(j)) {msg->ballots[i]=0;continue;}
            msg->ballots[i] = logGetBallot(j);
            msg->vals[i] = logGetValue(j);
            if (msg->ballots[i] == 0) full = 0;
            if (msg->ballots[i] > 0) nonblank = 1;
            if (msg->ballots[i] > latestBallot) badballot = 1;
          }
          if ( ! full) {
            if(debug)fprintf(stderr, "[%d] blank in log\n", id);
            //assert(full);
          }
          if (!(nonblank || end == start)) {
            if(debug)fprintf(stderr, "[%d] nb:%d, strt:%d, end:%d\n", id, nonblank, start, end);
            if(debug)fprintf(stderr, "[%d] type:%d\n", id, type);
            assert(nonblank || end == start);
            return 1;
          }
          if (badballot) {
            if(debug)fprintf(stderr, "[%d]  tried sending bad ballot\n", id); 
            assert(0);
          }

        case PREPARE_M:
        case LIVENESS_M:
          // There can be no items.
          ;
      }


      net.send(address, msg);
      return 0;
    }

};

#endif // PAXOS_HH

