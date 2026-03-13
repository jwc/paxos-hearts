#ifndef HEARTS_HH
#define HEARTS_HH

#define DEAL_T 0
#define SELECT_T 1
#define PLAY_T 2

//typedef Value Play;

class Hearts : public Printable {
  public:
    Hearts();

    int isPlayable(Value play);

    int play(Value move);

    void print(int playerPerspective);

  //private:
    Cards hands[8];
    int turn;
    int scores[8];

};

#endif // HEARTS_HH
 
