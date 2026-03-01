#ifndef MESSAGE_HH
#define MESSAGE_HH

/**
 * Prepare  - Sent out by node to try and become leader.
 * Promise  - Response to Prepare, including relevant info about state.
 * Accept   - Sent by leader to propose change to state.
 * Accepted - Response to Accept. 
 * Liveness - 
 * Request  - 
 * Bypass   - Not a part of actual Paxos. This is for messages that do not 
 *            impact the state, and can therefore be sent w/o acheiving consensus.
 */

/*
enum msgType {
    PREPARE,
    PROMISE, 
    ACCEPT,
    ACCEPTED,
    LIVENESS,
    REQUEST,
    BYPASS
};
*/

#define PREPARE_M   0
#define PROMISE_M   1
#define ACCEPT_M    2
#define ACCEPTED_M  3
#define LIVENESS_M  4
#define REQUEST_M   5
#define CONFIRM_M   6
#define BYPASS_M    7

#define MSG_SIZE sizeof(Message)
//#define NEW_MSG() new Message()
//#define DEL_MSG(x) delete x

//#define MSG_SIZE 100
#define NEW_MSG() (Message *) malloc(MSG_SIZE) 
//#define DEL_MSG(x) free(x) 
#define DEL_MSG(x) ;
//#define PAXOS_MAX_PENDING 5

// Maximum allowed number of promised but not confirmed messages.
union Value {
  int32_t raw;
  struct {
    int8_t type;
    int16_t data;
  };
};

struct Message {
    //Networking:
    int8_t sender;
    int8_t recipient;

    //Paxos:
    int8_t type;
    short int ballotNo;

    short int logPendingStart;
    short int logPendingEnd;
   

    // LIVENESS 
    union {
        ; // PREPARE

        struct { // REQUEST BYPASS
            //Hearts:
            short int slot;
            Value val;
        };

        struct { // PROMISE ACCEPT ACCEPTED CONFIRM
            //short int slots[MAX_PENDING];
            short int ballots[PAXOS_MAX_PENDING];
            Value vals[PAXOS_MAX_PENDING];
        };
    };
};

static_assert(sizeof(struct Message) <= MSG_SIZE, "Message STruct too big.");
static_assert(sizeof(union Value) <= sizeof(long int), "Value struct too big.");

#endif // MESSAGE_HH

