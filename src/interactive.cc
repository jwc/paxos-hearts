#include "header.hh"

char currentPlayer = 0;

int main(int argc, char** argv) {
  if (argc < 2) {
    exit(1);
  }

  Networking *net = new Localhost(argv[1]);
  Paxos *pax = new Paxos(*net);
  Hearts * h = new Hearts(*pax);
  for (int i = 0; i < 4; i++)
    h->hands[i].add(i);

  mvprintw(0, 0, "main() ready\n");
  char input = 'a';
  Value val;
  char hasExited = 0;
  while ( ! hasExited) {
    input = getch();
    fprintf(stderr, "input: %c\n", input);

    switch (input) {
      case 'q':
        hasExited = 1;
        break; 

      case 't':
        h->toggleUnicode();
        h->toggleColor();
        break;

      case 'r':
        val = { .type = REQ_START_T, 
                .player = currentPlayer, 
                .data = (int16_t) rand() };
        h->play(val);
        break;

      case 'R':
        fprintf(stderr, "calling play()\n");
        for (char i = 0; i < 4; i++) {
          val = { .type = REQ_START_T, .player = i, .data = (int16_t) rand() };
          h->play(val);
        }
        break;

      case 'l':
        val = { .type=SELECT_T, 
                .player=currentPlayer, 
                .data=h->hands[currentPlayer].getSelected() + 1 };
        pax->sendStatelessValue(val);
        break;

      case 'h':
        val = { .type=SELECT_T, 
                .player=currentPlayer, 
                .data=(int16_t) h->hands[currentPlayer].getSelected() - 1 };
        pax->sendStatelessValue(val);
        break;

      case ' ':
        val = { .type=PLAY_T,
                .player=currentPlayer, 
                .data=(int16_t) h->hands[currentPlayer].getSelected() };
        pax->makeRequest(val);
        break;

      case '0':
        currentPlayer = 0;
        break;

      case '1':
        currentPlayer = 1;
        break;

      case '2':
        currentPlayer = 2;
        break;

      case '3':
        currentPlayer = 3;
        break;

      case '4':
        currentPlayer = 0;
        break;
    };

    h->print(currentPlayer);
  } 

  mvprintw(0, 0, "main() done\n");
  exit(0);
  return 0;
}
