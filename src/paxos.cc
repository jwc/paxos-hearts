#include "header.hh"

// Public functions:

Paxos::Paxos(Networking & net) : net(net), incoming(net.getBuffer()),
    nNodes(net.getN()), id(net.getID()) { 
  lock.lock();

  latestBallot = nNodes;
  myBallot = id;
  logFirstFree = logPendingStart = logPendingEnd = 0;  
  intervalsFromLeader = 0;

  timerThread = std::thread(&Paxos::timer, this);
  handlerThread = std::thread(&Paxos::handler, this);

  #ifdef PAXOS_DEBUG
  timeHandler = timeTimer = timeIdle = 0;
  timeA = std::chrono::high_resolution_clock::now();
  #endif // PAXOS_DEBUG
         
  lock.unlock();
}

void Paxos::makeRequest(Value request) {
  sendMessage(getLeaderID(), REQUEST_M, request);
  return;

  // Locking not needed. 
  Message * msg = NEW_MSG();
  msg->sender = id;
  msg->recipient = latestBallot % nNodes;
  msg->type = REQUEST_M;
  msg->ballotNo = latestBallot;

  msg->val = request;
  
  net.send(msg->recipient, msg);
}

void Paxos::sendStatelessValue(Value value) {
  if(debug)fprintf(stderr, "sendStatelessValue: %d\n", value.raw);
  sendMessage(NET_BROADCAST, BYPASS_M, value);
  outgoing.produce(value);
}


// Private functions:

void Paxos::timer() {
  while (1) {
    lock.lock();

    #ifdef PAXOS_DEBUG
    timeB = std::chrono::high_resolution_clock::now();
    timeIdle += std::chrono::duration_cast<std::chrono::nanoseconds>(
    timeB - timeA ).count();
    timeA = std::chrono::high_resolution_clock::now();
    #endif // PAXOS_DEBUG

    if (latestBallot == myBallot 
        && HAS_MAJORITY(ballotVotes, nNodes)) {
      // Leader sends msg to confirm liveness to other nodes.
      for (int i = logPendingStart; i < logPendingEnd; i++) {
        if (logGetBallot(i) > 0 && logGetBallot(i) < latestBallot) {
          logSetBallot(i, latestBallot);
          logSetVote(i, 0);
          logCastVote(i, id);
        }
      }
      
      // TODO: Take any unconfirmed ballots, and repropose w/ a higher ballot
          
      if(debug)fprintf(stderr, "[%d] Send Liveness_m: %d\t\t[%d,%d)\n", id, 
        latestBallot, logPendingStart, logPendingEnd);
      sendMessage(NET_BROADCAST, ACCEPT_M, logPendingStart, logPendingEnd);
         
    } else {
      // NOT LEADER:
      intervalsFromLeader++;
      if(debug)fprintf(stderr, "[%d] L:%d TIMER: %d\t\t[%d,%d)\n", id, 
      latestBallot, intervalsFromLeader, logPendingStart, logPendingEnd);
        
      if (intervalsFromLeader >= PAXOS_MAX_INTERVALS_FROM_LEADER) {
        // leaderless; attempt to become the new leader.
        ballotVotes = 0;
        MARK(ballotVotes, id);
          
        while (myBallot <= latestBallot) myBallot += nNodes;
        latestBallot = myBallot;

        sendMessage(NET_BROADCAST, PREPARE_M, logPendingStart, logPendingEnd);

        intervalsFromLeader = 0;
      }
    }

    #ifdef PAXOS_DEBUG
    if(debug)fprintf(stderr, "    I:%0.0lfs, T:%0.0lfus, H:%0.0lfus\n", 
      timeIdle/1000000000, timeTimer/1000, timeHandler/1000);
    timeB = std::chrono::high_resolution_clock::now();
    timeTimer += std::chrono::duration_cast<std::chrono::nanoseconds>(
      timeB - timeA ).count();
    timeA = std::chrono::high_resolution_clock::now();
    #endif // PAXOS_DEBUG

    lock.unlock();

    struct timespec rqtp;
    rqtp.tv_sec = 0;
    rqtp.tv_nsec = PAXOS_TIMEOUT_NS;
    nanosleep(&rqtp, NULL);
  }
}

void Paxos::handler() {
  while (1) {
    Message * msg = incoming.consume();
    lock.lock();

    #ifdef PAXOS_DEBUG
    timeB = std::chrono::high_resolution_clock::now();
    timeIdle += std::chrono::duration_cast<std::chrono::nanoseconds>(
      timeB - timeA ).count();
    timeA = std::chrono::high_resolution_clock::now();
    #endif // PAXOS_DEBUG

    switch (msg->type) {
      case PREPARE_M:
        if(debug)fprintf(stderr, "[%d]<%d PREP(%d)\n", id, msg->sender, msg->ballotNo);
        handlePrepare(msg); 
        break;

      case PROMISE_M:
        if(debug)fprintf(stderr, "[%d]<%d PROM(%d) Votes:%o (%d)\n", 
          id, msg->sender, msg->ballotNo, ballotVotes, 
          HAS_MAJORITY(ballotVotes, nNodes));
        //fprintf(stderr, "\tlatest:%d, my:%d\n", latestBallot, myBallot);
        handlePromise(msg);
        break;

      case LIVENESS_M: 
        if(debug)fprintf(stderr, "[%d] LIVE(%d)\n", id, msg->ballotNo);
        handleLiveness(msg);
        break;

      case ACCEPT_M:
        if(debug)fprintf(stderr, "[%d]<%d ACPT\tlog:%d:[%d:%d) msg:%d:[%d:%d)\n", 
          id, msg->sender, latestBallot, logPendingStart, logPendingEnd, 
          msg->ballotNo, msg->logPendingStart, msg->logPendingEnd);
        handleAccept(msg);
        break;

      case ACCEPTED_M:
        if(debug)fprintf(stderr, "[%d]<%d ACPTED\tlog:%d:[%d:%d) msg:%d:[%d:%d)\n", 
          id, msg->sender, latestBallot, logPendingStart, logPendingEnd, 
          msg->ballotNo, msg->logPendingStart, msg->logPendingEnd);
        //fprintf(stderr, "\tmsg:%d, latest:%d, my:%d\n", 
          //msg->ballotNo, latestBallot, myBallot);
        handleAccepted(msg);
        break;

      case REQUEST_M:
        if(debug)fprintf(stderr, "[%d]<%d REQ(%d:%d)\n", 
            id, msg->sender, msg->slot, msg->val);
        handleRequest(msg);
        break;

      case CONFIRM_M:
        if(debug)fprintf(stderr, "[%d]<%d CONF b:%d s:%d e:%d\n", 
          id, msg->sender, msg->ballotNo, 
          msg->logPendingStart, msg->logPendingEnd);
        handleConfirm(msg);
        break;

      case BYPASS_M:
        handleBypass(msg);
        break;

      default:
        if(debug)fprintf(stderr, "[%d]<%d Invalid MSG Type: \"%s\"\n", 
          id, msg->sender, msg);
        assert(0);
    }
   
    assert(logPendingStart <= logPendingEnd);
    if (logPendingEnd - logPendingStart > PAXOS_MAX_PENDING) {
      if(debug)fprintf(stderr, "[%d] lPE: %d  lPS: %d  PMP: %d\n", logPendingEnd, logPendingStart, PAXOS_MAX_PENDING);
    }
    assert(logPendingEnd - logPendingStart <= PAXOS_MAX_PENDING);

    // Advancing Pending Range
    advancePendingRange();

    assert(logPendingStart <= logPendingEnd);
    assert(logPendingEnd - logPendingStart <= PAXOS_MAX_PENDING);

    #ifdef PAXOS_DEBUG
    timeB = std::chrono::high_resolution_clock::now();
    timeHandler += std::chrono::duration_cast<std::chrono::nanoseconds>(
      timeB - timeA ).count();
    timeA = std::chrono::high_resolution_clock::now();
    #endif // PAXOS_DEBUG

    lock.unlock();
    DEL_MSG(msg);
  }
}

void Paxos::handlePrepare(Message * msg) {
  if (msg->ballotNo > latestBallot) {
    latestBallot = msg->ballotNo;
    intervalsFromLeader = 0;

    sendUnknownConfirms(msg->sender, msg->logPendingStart);

    sendMessage(msg->sender, PROMISE_M, logPendingStart, logPendingEnd);
  }
}

void Paxos::handlePromise(Message * msg) {
  if (msg->ballotNo == latestBallot && latestBallot == myBallot) {
    int promiseKept = 1; 
    for (int i=0, j=msg->logPendingStart; j < msg->logPendingEnd; i++,j++) {
      if (j >= logPendingEnd) promiseKept = 0;

      if (msg->ballots[i] > 0) logAdd(j, msg->ballots[i], msg->vals[i]);
    }

    if (promiseKept) MARK(ballotVotes, msg->sender);

    //fprintf(stderr, "\tv:%o t:%d\n",ballotVotes,TALLY(ballotVotes));
  }
}

void Paxos::handleAccept(Message * msg) {
  if (msg->ballotNo < latestBallot) {
    if(debug)fprintf(stderr, "[%d]  skipping func\n", id);
  }
  if (msg->ballotNo >= latestBallot) {
    latestBallot = msg->ballotNo;
    intervalsFromLeader = 0;

    for (int i=0, j=msg->logPendingStart; j < msg->logPendingEnd; i++,j++) {
      if (msg->ballots[i] == 0) {
        if(debug)fprintf(stderr, "[%d] zero ballot\n", id);
        continue;
      }
      if ( ! isInAlterableRange(j)) {
        if(debug)fprintf(stderr, "[%d] out of range\n", id);
        continue;
      }
    
      if (logGetBallot(j) < msg->ballots[i] 
          && logAdd(j, msg->ballots[i], msg->vals[i]) > -1) {
        logCastVote(j, msg->sender);
        logCastVote(j, id);
        if(debug)fprintf(stderr, "[%d] A good\n", id);

      } else if (logGetBallot(j) == msg->ballots[i]) {
        logCastVote(j, msg->sender);
        logCastVote(j, id);
        if(debug)fprintf(stderr, "[%d] B good\n", id);

      } else {
        if(debug)fprintf(stderr, "[%d] C bad/ignore\n", id);
      }

      /*
      if (logGetBallot(j) == 0) {
        // slot not in log
        if (-1 != logAdd(j, msg->ballots[i], msg->vals[i])) {
          fprintf(stderr, "[%d] A good\n", id);

          //assert(logBallots[j%PAXOS_LOG_SIZE] == msg->ballots[i]);

          logCastVote(j, msg->sender);
          logCastVote(j, id);
        } else
          fprintf(stderr, "[%d] B bad\n", id);
      } else if (logGetBallot(j) < msg->ballots[i]) {
        // slot older in log
        if (logAdd(j, msg->ballots[i], msg->vals[i]) > -1) {

          fprintf(stderr, "[%d] C good\n", id);
          //assert(logBallots[j%PAXOS_LOG_SIZE] == msg->ballots[i]);

          logCastVote(j, msg->sender);
          logCastVote(j, id);
        } else
          fprintf(stderr, "[%d] D bad\n", id);
      } else if (logGetBallot(j) == msg->ballots[i]) {
        // msg and log same 
        if (logHaveVoted(j, id)) {
          fprintf(stderr, "[%d] E fine\n", id);
          ; // Already voted: do nothing.
            logCastVote(j, msg->sender);
        } else {
          logCastVote(j, msg->sender);
          logCastVote(j, id);
          fprintf(stderr, "[%d] F fine \n", id);
        }
      } else {
        fprintf(stderr, "[%d] G ignore\n", id);
        ; // log has higher ballot: must ignore. 
      }
      */
    }
    //fprintf(stderr, "[%d]    voted:%d\n", id, castedVote);

    //if (msg->logPendingStart < logPendingStart)
      sendUnknownConfirms(msg->sender, msg->logPendingStart);

    if (isInPendingRange(logPendingStart))
      sendMessage(NET_BROADCAST, ACCEPTED_M, logPendingStart, logPendingEnd);
  }
}

void Paxos::handleAccepted(Message * msg) {
  if (msg->ballotNo >= latestBallot) {
    latestBallot = msg->ballotNo;
  //if (msg->ballotNo == latestBallot) {
    for (int i=0, j=msg->logPendingStart; j < msg->logPendingEnd; i++,j++) {
      if ( ! isInAlterableRange(j)) continue;
      if (msg->ballots[i] == 0) {
        if(debug)fprintf(stderr, "[%d]  zero ballot\n", id);
        continue;
      } else

      if (isInPendingRange(j) && msg->ballots[i] == logGetBallot(j)) {
        logCastVote(j, msg->sender);
        if(debug)fprintf(stderr, "[%d]    prev: %d:%d/%d\n", 
                  id, j, TALLY(logGetVote(j)), nNodes);
      //} else if (isUnwritten(j) && isInAlterableRange(j) 
      } else if (isUnwritten(j) 
                && logAdd(j, msg->ballots[i], msg->vals[i]) > -1) {
        logCastVote(j, id);
        logCastVote(j, msg->sender);
        if(debug)fprintf(stderr, "[%d]    new: %d:%d/%d\n", 
                  id, j, TALLY(logGetVote(j)), nNodes);
      } else {
        if(debug)fprintf(stderr, "[%d]  rejected. mb:%d lb:%d\n", 
                  id, msg->ballots[i], logGetBallot(j));
      }

      /*else if (msg->ballots[i] > logGetBallot(j)) {
        logAdd(j, msg->ballots[i], msg->vals[i]);
        logCastVote(j, msg->sender);
      }*/
    }

    sendUnknownConfirms(msg->sender, msg->logPendingStart);
  }
}

void Paxos::handleLiveness(Message * msg) {
  if (msg->ballotNo >= latestBallot) {
    latestBallot = msg->ballotNo;
    intervalsFromLeader = 0;
  }
}

void Paxos::handleRequest(Message * msg) {
  if (msg->ballotNo == latestBallot 
      && myBallot == latestBallot 
      && HAS_MAJORITY(ballotVotes, nNodes)) {
    // Leader recieves a request. Insert into log & send Accept msg.
    int slot = logAddNew(latestBallot, msg->val);
    if (slot < 0) {
      if(debug)fprintf(stderr, "\t[%d] failed to add.\n", id);
      return;
    }
  
    assert(logGetBallot(slot) > 0);
    assert(logBallots[slot % PAXOS_LOG_SIZE] > 0);

    if(debug)fprintf(stderr, "[%d] (%d)%d:%d\n", id, logBallots[slot], slot, msg->val);
  }   
}

void Paxos::handleConfirm(Message * msg) {
  if (msg->ballotNo >= latestBallot) {
    latestBallot = msg->ballotNo;
  //if (msg->ballotNo == latestBallot) {
    for (int i=0, j=msg->logPendingStart; j < msg->logPendingEnd; i++,j++) {
      if(debug)fprintf(stderr, "[%d] lGB:%d, m>b:%d\n", id, 
          logGetBallot(j), msg->ballots[i]);
      if ( ! isInAlterableRange(j)) continue;
      if (msg->ballots[i] == 0) continue;

      if (msg->ballots[i] < logGetBallot(j)) continue;

      /*
      if (msg->ballots[i] > logGetBallots(j)) 
        logAdd(j, msg->ballots[i], msg->vals[i];

      logSetVote(j, ~0);
      */

      if (msg->ballots[i] > logGetBallot(j)) {
        logAdd(j, msg->ballots[i], msg->vals[i]); 
        logSetVote(j, ~0);
      } else if (msg->ballots[i] == logGetBallot(j)) {
        logSetVote(j, ~0); 
      }
    }
  }
}

void Paxos::handleBypass(Message * msg) { outgoing.produce(msg->val); }

