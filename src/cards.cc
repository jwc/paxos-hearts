#include "header.hh"

Cards::Cards() : Printable() {
  numCards = 0;
  visibility = 0;
  selected = 0;
}

char Cards::add(char card) {
  assert(card >= 0);
  assert(card < 52);

  cards[numCards] = card;
  return numCards++;
}

char Cards::remove() {
  assert(numCards > 0);

  numCards--;
  if (selected >= numCards) selected--;

  return cards[numCards];
}

char Cards::remove(char index) {
  assert(index >= 0);
  assert(index < numCards);

  char card = cards[index];

  numCards--;
  if (selected >= numCards || index < selected) selected--;
  for (int i = index; i < numCards; i++)
    cards[i] = cards[i + 1];

  return card;
}

void Cards::printCards(int y, int x, int orientation) {
  mvprintw(sizeY/2, sizeX/2, "orientation:%d", orientation);
  if ((orientation % 2) == 0) {
    for (int i = 0; i < numCards; i++) {
      printCard(y, x + ((i + i - numCards) * 2), orientation, cards[i]);
    }
  } else {
    for (int i = 0; i < numCards; i++) {
      printCard(y + ((i + i - numCards) * 1), x, orientation, cards[i]);
    }
  }
}

void Cards::printSuit(int y, int x, char card) {
  switch (static_cast<Suit>(card / 13)) {
    case SPADES:
      if (useUnicode()) mvaddwstr(y, x, L"♠");
      else mvaddstr(y, x, "S");
      break;
    case HEARTS:
      if (useUnicode()) mvaddwstr(y, x, L"♥");
      else mvaddstr(y, x, "H");
      break;
    case CLUBS:
      if (useUnicode()) mvaddwstr(y, x, L"♣");
      else mvaddstr(y, x, "C");
      break;
    case DIAMONDS:
      if (useUnicode()) mvaddwstr(y, x, L"♦");
      else mvaddstr(y, x, "D");
      break;
    default:
      mvaddstr(y, x, "?");
  };
}

void Cards::printRank(int y, int x, char card) {
  switch (static_cast<Rank>(card%13)) {
    case 0:
      mvaddstr(y, x, "2");
      break;
    case 1:
      mvaddstr(y, x, "3");
      break;
    case 2:
      mvaddstr(y, x, "4");
      break;
    case 3:
      mvaddstr(y, x, "5");
      break;
    case 4:
      mvaddstr(y, x, "6");
      break;
    case 5:
      mvaddstr(y, x, "7");
      break;
    case 6:
      mvaddstr(y, x, "8");
      break;
    case 7:
      mvaddstr(y, x, "9");
      break;
    case 8:
      mvaddstr(y, x, "T");
      break;
    case 9:
      mvaddstr(y, x, "J");
      break;
    case 10:
      mvaddstr(y, x, "Q");
      break;
    case 11:
      mvaddstr(y, x, "K");
      break;
    case 12:
      mvaddstr(y, x, "A");
      break;
    default:
      mvaddstr(y, x, "?");
  };
}

void Cards::printCard(int y, int x, int orientation, char card) {
  if (orientation == 0 || orientation == 2) {
    if (cards[selected] == card && orientation == 0) y -= 2;
    if (cards[selected] == card && orientation == 2) y += 2;

    if (useUnicode()) {
      mvaddwstr(y-2, x-3, L"╭─────╮");
      for (int i = y-1; i < y+3; i++)
        mvaddwstr(i, x-3, L"│     │");
      mvaddwstr(y+2, x-3, L"╰─────╯");
    } else {
      mvprintw(y-2, x-3, ",-----,");
      for (int i = y-1; i < y+3; i++)
        mvprintw(i, x-3, "|     |");
      mvprintw(y+2, x-3, "'-----'");
    }

    if (useColor() && (getSuit(card) == DIAMONDS || getSuit(card) == HEARTS)) 
      attron(COLOR_PAIR(1));
    printSuit(y-1, x-1, card);
    printRank(y-1, x-2, card);
    printSuit(y+1, x+1, card);
    printRank(y+1, x+2, card);
    if (useColor() && (getSuit(card) == DIAMONDS || getSuit(card) == HEARTS)) 
      attroff(COLOR_PAIR(1));
  } else {
    if (cards[selected] == card && orientation == 1) x += 3;
    if (cards[selected] == card && orientation == 3) x -= 3;

    if (useUnicode()) {
      mvaddwstr(y-1, x-4, L"╭───────╮");
      mvaddwstr(y+0, x-4, L"│       │");
      mvaddwstr(y+1, x-4, L"│       │");
      mvaddwstr(y+2, x-4, L"╰───────╯");
    } else {
      mvaddstr(y-1, x-4, ",-------,");
      mvaddstr(y+0, x-4, "|       |");
      mvaddstr(y+1, x-4, "|       |");
      mvaddstr(y+2, x-4, "'-------'");
    }
    
    if (useColor() && (getSuit(card) == DIAMONDS || getSuit(card) == HEARTS)) 
      attron(COLOR_PAIR(1));
    printSuit(y-0, x-2, card);
    printRank(y-0, x-3, card);
    printSuit(y+1, x+2, card);
    printRank(y+1, x+3, card);
    if (useColor() && (getSuit(card) == DIAMONDS || getSuit(card) == HEARTS)) 
      attroff(COLOR_PAIR(1));
  }

  //mvprintw(y, x, "X");
}

void Cards::shuffle(int32_t seed) {
  srand(seed);

  for (int i = 0; i < 52; i++) 
    SWAP(cards, i, rand() % 52);
}

void Cards::selectCard(char index) {
  selected = index;
}

