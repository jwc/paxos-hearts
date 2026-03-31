#include "header.hh"

char currentPlayer = 0;

//void foo(Cards * c, Buffer<Value> * buf) {
void foo(Hearts * h, Buffer<Value> * buf) {
  //Cards * c = new Cards();
  mvprintw(8, 3, "foo() created");
  Value v;
  int bar = 0;
  while (1) {
    mvprintw(8, 3, "foo() waiting");
    v = buf->consume();
    
    mvprintw(2, 0, "%d foo() printing %c/%d\n", bar++, v.type, v.type);
    //c->printCards(Printable::sizeY-3, Printable::sizeX/2, 0);
    h->print(currentPlayer);
    mvprintw(Printable::sizeY-1, Printable::sizeX/2, "X");
  }
  
  mvprintw(8, 3, "foo() done\n");
}

int main(int argc, char** argv) {
  if (argc < 2) {
    exit(1);
  }

  Networking *net = new Localhost(argv[1]);
  Paxos *pax = new Paxos(*net);
  //Cards * c = new Cards();
  Hearts * h = new Hearts(*pax);
  for (int i = 0; i < 4; i++)
    h->hands[i].add(i);
  //mvprintw(35, 0, "# threads %d", std::thread::hardware_concurrency());
  Buffer<Value> * buf = new SVBuffer<Value>(1);

  //std::jthread output = std::jthread(&foo, &pax->getBuffer());
  //std::jthread output = std::jthread(&foo, c, buf);
  std::jthread output = std::jthread(&foo, h, buf);

  mvprintw(0, 0, "main() ready\n");
  char input = 'a';
  int count = 0;
  Value val;
  char hasExited = 0;
  while ( ! hasExited) {
    //std::this_thread::yield();
    //producer.lock();
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

    mvprintw(0, 0, "%d main() req(%c/%d)\n", count++, input, input);
    buf->produce(val);
    //std::this_thread::yield();
    //consumer.unlock();
  } 

  mvprintw(0, 0, "main() done\n");
  exit(0);
  return 0;
}
