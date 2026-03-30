#ifndef HEARTS_HH
#define HEARTS_HH

#define REQ_START_T 0
#define START_T 1
#define SELECT_T 2
#define PLAY_T 3

//typedef Value Play;
union Voting {
  int32_t all; 
  struct {
    int8_t votes[4];
  };
};

class Hearts : public Printable {
  public:
    Hearts(Paxos & pax);

    int isPlayable(Value play);

    int play(Value move);

    void print(int playerPerspective);

    int shouldDeal() { return __builtin_popcount(voting.all) >= 4; };

  //private:
    Cards hands[8];
    char phase;
    char turn;
    char topCardPlayer; 
    Suit leadingSuit;
    Rank leadingRank;
    int scores[8];
    char gameFinished;
    std::mutex lock;

    Paxos & pax;
    Voting voting;

    std::thread listenerThread;
    void listener();
};

#endif // HEARTS_HH
 
