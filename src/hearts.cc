#include "header.hh"

Hearts::Hearts(Paxos & pax) : Printable(), pax(pax), turn(0) {
  for (int i = 0; i < 8; i++) scores[i] = 0;

  voting.all = 0; 
}

int Hearts::isPlayable(Value move) {
  if (move.type == DEAL_T) return 1;
  return 1;
}

int Hearts::play(Value move) {
  fprintf(stderr, "play()\n");
  switch (move.type) {
    case REQ_DEAL_T:
      voting.votes[move.player] = 1;
      fprintf(stderr, "req vote : %d\n", __builtin_popcount(voting.all));

      if (__builtin_popcount(voting.all) >= 4 && pax.getID() == 0) {
        Value v = { .type = DEAL_T, .player = 0, .data = (int16_t) rand() };
        pax.makeRequest(v);
      }
      break;

    case DEAL_T:
      //voting.all = 0;

      for (int i = 0; i < 8; i++) hands[i].clear();
      for (int i = 0; i < 52; i++) hands[0].add(i);

      hands[0].shuffle(move.raw);

      for (int i = 1; i < 4; i++) 
        for (int j = 0; j < 13; j++)
          hands[i].add(hands[0].remove());

      break;

    case SELECT_T:
      hands[move.player].selectCard(move.data);
      break;

    case PLAY_T:
      //voting.all = 0;
      break;

    default:
      ;
  } 

  return 1;
}

void Hearts::print(int playerPerspective) {
  lock.lock();
  Hearts::clear();

  hands[(0 + playerPerspective) % 4].printCards(sizeY - 3, sizeX / 2, 0);
  hands[(1 + playerPerspective) % 4].printCards(sizeY / 2,         4, 1);
  hands[(2 + playerPerspective) % 4].printCards(        2, sizeX / 2, 2);
  hands[(3 + playerPerspective) % 4].printCards(sizeY / 2, sizeX - 5, 3);

  mvprintw(sizeY - 6, sizeX / 2, "Score: %d (%d)", 
      scores[(0 + playerPerspective) % 4], 
      scores[((0 + playerPerspective) % 4) + 4]);

  mvprintw(sizeY / 2,         7, "Score: %d (%d)", 
      scores[(1 + playerPerspective) % 4], 
      scores[((1 + playerPerspective) % 4) + 4]);

  mvprintw(        5, sizeX / 2, "Score: %d (%d)", 
      scores[(2 + playerPerspective) % 4], 
      scores[((2 + playerPerspective) % 4) + 4]);

  mvprintw(sizeY / 2, sizeX - 20, "Score: %d (%d)", 
      scores[(3 + playerPerspective) % 4], 
      scores[((3 + playerPerspective) % 4) + 4]);

  /*
  for (int i = 0; i < 4; i++) {
    int index = (i + playerPerspective) % 4;
    ;
    hands[index].printCards(
  }
  */

  mvprintw(sizeY/2, sizeX/2, "votes:%d", __builtin_popcount(voting.all));

  Hearts::refresh();
  lock.unlock();
}
