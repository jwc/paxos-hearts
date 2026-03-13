#include "header.hh"

Hearts::Hearts() : Printable(), turn(0) {
  for (int i = 0; i < 8; i++) scores[i] = 0;

  
}

int Hearts::isPlayable(Value move) {
  if (move.type == DEAL_T) return 1;
  return 1;
}

int Hearts::play(Value move) {
  if (move.type == DEAL_T) {
    for (int i = 0; i < 8; i++) hands[i].clear();
    for (int i = 0; i < 52; i++) hands[0].add(i);
   
    hands[0].shuffle(move.raw);

    for (int i = 1; i < 4; i++) 
      for (int j = 0; j < 13; j++)
        hands[i].add(hands[0].remove());

  }
  return 1;
}

void Hearts::print(int playerPerspective) {
  hands[(0 + playerPerspective) % 4].printCards(sizeY - 4, sizeX / 2, 0);
  hands[(1 + playerPerspective) % 4].printCards(sizeY / 2,         4, 1);
  hands[(2 + playerPerspective) % 4].printCards(        4, sizeX / 2, 2);
  hands[(3 + playerPerspective) % 4].printCards(sizeY / 2, sizeX - 4, 3);

  /*
  for (int i = 0; i < 4; i++) {
    int index = (i + playerPerspective) % 4;
    ;
    hands[index].printCards(
  }
  */
}
