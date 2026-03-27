#include "header.hh"

Hearts::Hearts(Paxos & pax) : Printable(), pax(pax), turn(0) {
  for (int i = 0; i < 8; i++) scores[i] = 0;

  voting.all = 0; 

  listenerThread = std::thread(&Hearts::listener, this);

  for (int i = 4; i < 8; i++) hands[i].selectCard(-1);
}

int Hearts::isPlayable(Value move) {
  if (move.type == DEAL_T) return 1;
  return 1;
}

int Hearts::play(Value move) {
  switch (move.type) {
    case REQ_DEAL_T:
      fprintf(stderr, "recv REQ_DEAL_T: %d\n", __builtin_popcount(voting.all));
      voting.votes[move.player] = 1;

      if (__builtin_popcount(voting.all) >= 4 && pax.getID() == 0) {
        Value v = { .type = DEAL_T, .player = 0, .data = (int16_t) rand() };
        fprintf(stderr, "send DEAL_T: %d\n", __builtin_popcount(voting.all));
        pax.makeRequest(v);
      }
      break;

    case DEAL_T:
      fprintf(stderr, "recv DEAL_T: \n");
      voting.all = 0;

      for (int i = 0; i < 8; i++) hands[i].clear();
      for (int i = 0; i < 52; i++) hands[0].add(i);

      hands[0].shuffle(move.raw);
      
      for (int i = 1; i < 4; i++) 
        for (int j = 0; j < 13; j++)
          hands[i].add(hands[0].remove());
      
      phase = 0;
      turn = 0;

      break;

    case SELECT_T:
      fprintf(stderr, "recv SELECT_T:\n");
      if (move.data < 0) move.data = 0;
      if (move.data >= hands[move.player].getNumCards()) 
        move.data = hands[move.player].getNumCards() - 1;

      hands[move.player].selectCard(move.data);
      break;

    case PLAY_T:
      fprintf(stderr, "recv PLAY_T: \n");
      if (move.data < 0) move.data = 0;
      if (move.data >= hands[move.player].getNumCards()) 
      voting.all = 0;

      if (phase % 2 == 0) {
        // Passing phase.
        if (hands[move.player + 4].getNumCards() < 3)
          hands[move.player + 4].add(hands[move.player].remove(move.data));

        int readyToPass = 1;
        for (int i = 4; i < 8; i++)
          if (hands[i].getNumCards() < 3) readyToPass = 0;

        if (readyToPass) {
          int offset = 1;
          if (phase == 2) offset = 3;
          if (phase == 4) offset = 2; 

          for (int i = 4; i < 8; i++) 
            for (int j = 0; j < 3; j++) 
              hands[(i + offset) % 4].add(hands[i].remove());

          phase++;
          turn = -1;
        }

      } else {
        // Playing phase.
       
        // First move of the trick - Must be the 2 of clubs. 
        if (turn == -1) {
          char card = hands[move.player].getCard(move.data);
          if (Cards::getSuit(card) == CLUBS && Cards::getRank(card) == Two) {
          //if (hands[move.player].getCard(move.data) == 0) {
            hands[move.player + 4].add(hands[move.player].remove(move.data));
            turn = (move.player + 1) % 4;
          }
        }

        if (turn == move.player) {
          ;
        }
      }

      break;

    default:
      fprintf(stderr, "recv UNKNOWN_T: \n");
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

  hands[(0 + playerPerspective) % 4 + 4].printCards(sizeY - 13, sizeX / 2, 0);
  hands[(1 + playerPerspective) % 4 + 4].printCards(sizeY / 2,         14, 1);
  hands[(2 + playerPerspective) % 4 + 4].printCards(        12, sizeX / 2, 2);
  hands[(3 + playerPerspective) % 4 + 4].printCards(sizeY / 2, sizeX - 15, 3);

  /*
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
  */

  mvprintw(0, 0, "Phase: %d  Turn: %d", phase, turn);

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

void Hearts::listener() {
  while (1) {
    play(pax.getBuffer().consume());
  }
}
