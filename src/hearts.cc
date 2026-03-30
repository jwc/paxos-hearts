#include "header.hh"

Hearts::Hearts(Paxos & pax) : Printable(), pax(pax), turn(0), gameFinished(0) {
  for (int i = 0; i < 8; i++) scores[i] = 0;

  voting.all = 0; 

  listenerThread = std::thread(&Hearts::listener, this);

  for (int i = 4; i < 8; i++) hands[i].selectCard(-1);
}

int Hearts::isPlayable(Value move) {
  if (move.type == START_T) return 1;
  return 1;
}

int Hearts::play(Value move) {
  switch (move.type) {
    case REQ_START_T:
      fprintf(stderr, "recv REQ_START_T: %d\n", __builtin_popcount(voting.all));
      voting.votes[move.player] = 1;

      if (__builtin_popcount(voting.all) >= 4 && pax.getID() == 0) {
        Value v = { .type = START_T, .player = 0, .data = (int16_t) rand() };
        fprintf(stderr, "send START_T: %d\n", __builtin_popcount(voting.all));
        pax.makeRequest(v);
      }
      break;

    case START_T:
      fprintf(stderr, "recv START_T: \n");
      voting.all = 0;

      for (int i = 0; i < 8; i++) {
        hands[i].clear();
        scores[i] = 99;
      }

      for (int i = 0; i < 52; i++) hands[0].add(i);

      hands[0].shuffle(move.raw);
      
      for (int i = 1; i < 4; i++) 
        for (int j = 0; j < 13; j++)
          hands[i].add(hands[0].remove());
      
      phase = 0;
      turn = 0;
      topCardPlayer = -1;
      gameFinished = 0;

      break;

    case SELECT_T:
      fprintf(stderr, "recv SELECT_T:\n");
      if (gameFinished) break;
      if (move.data < 0) move.data = 0;
      if (move.data >= hands[move.player].getNumCards()) 
        move.data = hands[move.player].getNumCards() - 1;

      hands[move.player].selectCard(move.data);
      break;

    case PLAY_T:
      fprintf(stderr, "recv PLAY_T: \n");
      if (gameFinished) break;
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
        char card = hands[move.player].getCard(move.data);
       
        if (turn == -1 && hands[move.player].getNumCards() == 13) {
          // First move of the hand - Must be the 2 of clubs. 
          if (Cards::getSuit(card) == CLUBS && Cards::getRank(card) == Two) {
            hands[move.player + 4].add(hands[move.player].remove(move.data));
            turn = (move.player + 1) % 4;
            topCardPlayer = move.player;
            leadingSuit = Cards::getSuit(card);
            leadingRank = Cards::getRank(card);

          } else return 0;

        
        } else if (turn == -1 && move.player == topCardPlayer) {
          // Start of a new trick.
          // TODO: Stop point cards from being lead if hearts not yet broken.
          hands[move.player + 4].add(hands[move.player].remove(move.data));
          topCardPlayer = move.player;
          leadingSuit = Cards::getSuit(card);
          leadingRank = Cards::getRank(card);
          turn = (move.player + 1) % 4;

        } else if (turn == move.player) {
          // Must follow suit if possible. 
          // TODO: Stop point cards from being played on the 1st trick.
          if (hands[move.player].hasSuit(leadingSuit) 
              && Cards::getSuit(card) == leadingSuit) {
            hands[move.player + 4].add(hands[move.player].remove(move.data));
            turn = (move.player + 1) % 4;
            if (Cards::getRank(card) > leadingRank) {
              leadingRank = Cards::getRank(card); 
              topCardPlayer = move.player;
            }

          } else if ( ! hands[move.player].hasSuit(leadingSuit)) {
            hands[move.player + 4].add(hands[move.player].remove(move.data));
            turn = (move.player + 1) % 4;

          } else return 0;
        }

        char trickFinished = 1; 
        char pointsInTrick = 0;
        for (int i = 4; i < 8; i++) { 
          if (hands[i].getNumCards() == 0) {
            trickFinished = 0;
            break;
          }
          pointsInTrick += Cards::getPoints(hands[i].getCard(0));
        }
        if (trickFinished) {
          scores[topCardPlayer + 4] += pointsInTrick;
          for (int i = 4; i < 8; i++) hands[i].clear();
          turn = -1;

          if (hands[0].getNumCards() == 0) {
            // Hand finished

            for (int i = 0; i < 4; i++) {
              scores[i] += scores[i + 4];
              scores[i + 4] = 0;
              if (scores[i] >= 100) gameFinished = 1;
            }

            if (gameFinished) break;

            phase++;
            if (phase == 7) phase++; // Check for no pass round.

            // Re-deal cards.  
            for (char i = 0; i < 52; i++) hands[0].add(i);
            hands[0].shuffle(rand());
            for (int i = 1; i < 4; i++)
              for (int j = 0; j < 13; j++) 
                hands[i].add(hands[0].remove());
          }
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

  hands[(0 + playerPerspective) % 4 + 4].printCards(sizeY/2 + 10, sizeX/2, 0);
  hands[(1 + playerPerspective) % 4 + 4].printCards(sizeY/2, sizeX/2 - 20, 1);
  hands[(2 + playerPerspective) % 4 + 4].printCards(sizeY/2 - 10, sizeX/2, 2);
  hands[(3 + playerPerspective) % 4 + 4].printCards(sizeY/2, sizeX/2 + 20, 3);

  mvprintw(sizeY - 8, sizeX / 2, "Score: %d (%d)", 
      scores[(0 + playerPerspective) % 4], 
      scores[((0 + playerPerspective) % 4) + 4]);

  mvprintw(sizeY / 2,         9, "Score: %d (%d)", 
      scores[(1 + playerPerspective) % 4], 
      scores[((1 + playerPerspective) % 4) + 4]);

  mvprintw(        7, sizeX / 2, "Score: %d (%d)", 
      scores[(2 + playerPerspective) % 4], 
      scores[((2 + playerPerspective) % 4) + 4]);

  mvprintw(sizeY / 2, sizeX - 22, "Score: %d (%d)", 
      scores[(3 + playerPerspective) % 4], 
      scores[((3 + playerPerspective) % 4) + 4]);

  mvprintw(0, 0, "Phase: %d  Turn: %d", phase, turn);
  mvprintw(1, 0, "Current Player: %d", playerPerspective);
  mvprintw(2, 0, "Leading Player: %d", topCardPlayer);

  /*
  for (int i = 0; i < 4; i++) {
    int index = (i + playerPerspective) % 4;
    ;
    hands[index].printCards(
  }
  */

  if (voting.all) mvprintw(sizeY/2 - 1, sizeX/2, "votes:%d", __builtin_popcount(voting.all));
  if (gameFinished) mvprintw(sizeY/2, sizeX/2, "Game Over. ");

  Hearts::refresh();
  lock.unlock();
}

void Hearts::listener() {
  while (1) {
    play(pax.getBuffer().consume());
  }
}
