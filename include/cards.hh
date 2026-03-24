#ifndef CARDS_HH
#define CARDS_HH

#define SWAP(a, b, c) { a[b]^=a[c]; a[c]^=a[b]; a[b]^=a[c]; }

enum Suit {
  SPADES = 0, 
  HEARTS = 1, 
  CLUBS = 2, 
  DIAMONDS = 3
};

enum Rank {
  Two = 0,
  Three = 1, 
  Four = 2, 
  Five = 3, 
  Six = 4, 
  Seven = 5, 
  Eight = 6, 
  Nine = 7, 
  Ten = 8, 
  Jack = 9, 
  Queen = 10, 
  King = 11, 
  Ace = 12
};

/**
 * Class used to represent a grouping of playing cards. 
 */
class Cards : public Printable {
  public:
    Cards();

    void printCards(int y, int x, int orientation);

    char add(char card);

    char remove();

    char remove(char index);

    inline void clear() { numCards = 0; };

    inline char getNumCards() { return numCards; };

    inline char getCard(char index) { return index < numCards ? cards[index] : -1; };

    inline Suit getSuit(char card) { return static_cast<Suit>(card / 13); };

    inline Rank getRank(char card) { return static_cast<Rank>(card % 13); };

    inline void toggleVisibility() { visibility ^= 1; };

    void shuffle(int32_t seed);

    void shuffledDeck();

    void printCard(int y, int x, int orientation, char card);

    void printSuit(int y, int x, char card);

    void printRank(int y, int x, char card);

  private:
    char cards[52];
    char selected;
    char numCards;
    uint64_t visibility;

};

#endif // CARDS_HH

