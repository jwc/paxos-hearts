#include "header.hh"

Cards::Cards() : Printable() {
  numCards = 0;
  visibilityBitmap = 0;
}

char Cards::add(char card) {
  assert(card >= 0);
  assert(card < 52);

  visibilityBitmap |= 1 << numCards; 
  cards[numCards] = card;
  return numCards++;
}

char Cards::remove(char index) {
  assert(index >= 0);
  assert(index < numCards);

  return 0;
}

void Cards::printCards(int y, int x) {
  printCard(x, y, 8);
}

void Cards::printSuit(int y, int x, char card) {
  switch (static_cast<Suit>(card / 13)) {
    case Spades:
      if (useUnicode()) mvprintw(y, x, "♠");
      else mvprintw(y, x, "S");
      break;
    case Hearts:
      if (useUnicode()) mvprintw(y, x, "♥");
      else mvprintw(y, x, "H");
      break;
    case Clubs:
      if (useUnicode()) mvprintw(y, x, "♣");
      else mvprintw(y, x, "C");
      break;
    case Diamonds:
      if (useUnicode()) mvprintw(y, x, "♦");
      else mvprintw(y, x, "D");
      break;
    default:
      mvprintw(y, x, "?");
  };
}

void Cards::printRank(int y, int x, char card) {
  switch (static_cast<Rank>(card%13)) {
    case 0:
      mvprintw(y, x, "2");
      break;
    case 1:
      mvprintw(y, x, "3");
      break;
    case 2:
      mvprintw(y, x, "4");
      break;
    case 3:
      mvprintw(y, x, "5");
      break;
    case 4:
      mvprintw(y, x, "6");
      break;
    case 5:
      mvprintw(y, x, "7");
      break;
    case 6:
      mvprintw(y, x, "8");
      break;
    case 7:
      mvprintw(y, x, "9");
      break;
    case 8:
      mvprintw(y, x, "T");
      break;
    case 9:
      mvprintw(y, x, "J");
      break;
    case 10:
      mvprintw(y, x, "Q");
      break;
    case 11:
      mvprintw(y, x, "K");
      break;
    case 12:
      mvprintw(y, x, "A");
      break;
    default:
      mvprintw(y, x, "?");
  };
}

void Cards::printCard(int y, int x, char card) {
  if (useUnicode()) {
    mvprintw(y-2, x-3, "╭─────╮");
    for (int i = y-1; i < y+3; i++)
      mvprintw(i, x-3, "│     │");
    mvprintw(y+2, x-3, "╰─────╯");
  } else {
    mvprintw(y-2, x-3, ",-----,");
    for (int i = y-1; i < y+3; i++)
      mvprintw(i, x-3, "|     |");
    mvprintw(y+2, x-3, "'-----'");
  }

  if (useColor() && (getSuit(card) == Diamonds || getSuit(card) == Hearts)) 
    attron(COLOR_PAIR(1));
  printSuit(y-1, x-2, card);
  printRank(y-1, x-1, card);
  printSuit(y+1, x+2, card);
  printRank(y+1, x+1, card);
  if (useColor() && (getSuit(card) == Diamonds || getSuit(card) == Hearts)) 
    attroff(COLOR_PAIR(1));

  mvprintw(0, sizeY - 1, "X");
}

void Cards::shuffledDeck() {
  clear();

  for (int i = 0; i < 52; i++) 
    cards[i] = i;

  for (int i = 0; i < 52; i++) 
    SWAP(cards, i, rand());

  visibilityBitmap = ~0;
}
