#include "header.hh"

int currentPlayer = 0;

//void foo(Cards * c, Buffer<Value> * buf) {
void foo(Hearts * h, Buffer<Value> * buf) {
  //Cards * c = new Cards();
  mvprintw(2, 3, "foo() created");
  Value v;
  int bar = 0;
  while (1) {
    mvprintw(2, 3, "foo() waiting");
    v = buf->consume();
    
    mvprintw(2, 0, "%d foo() printing %c/%d\n", bar++, v.type, v.type);
    //c->printCards(Printable::sizeY-3, Printable::sizeX/2, 0);
    h->print(currentPlayer);
    mvprintw(Printable::sizeY-1, Printable::sizeX/2, "X");
  }
  
  mvprintw(0, 0, "foo() done\n");
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
  Value v;
  v.type = 'a';
  int count = 0;
  while (v.type != 'q') {
    //std::this_thread::yield();
    //producer.lock();
    v.type = getch();
    fprintf(stderr, "input: %c\n", v.type);

    if (v.type == 'r') {
      mvprintw(11, 0, "sz:%d", buf->count());
      //std::this_thread::yield();
    } else if (v.type == 't') {
      h->toggleUnicode();
      h->toggleColor();
    } else if (v.type == 'a') {
      h->hands[rand() % 4].add(rand() % 52);
    } else if (v.type == 'R') {
      fprintf(stderr, "calling play()\n");
      for (char i = 0; i < 4; i++) {
        Value val = { .type = REQ_DEAL_T, .player = i, .data = (int16_t) rand() };
        h->play(val);
      }
    } else if (v.type == '0') {
      currentPlayer = 0;
    } else if (v.type == '1') {
      currentPlayer = 1;
    } else if (v.type == '2') {
      currentPlayer = 2;
    } else if (v.type == '3') {
      currentPlayer = 3;
    } else if (v.type == '4') {
      currentPlayer = 4; 
    }
    mvprintw(0, 0, "%d main() req(%c/%d)\n", count++, v.type, v.type);
    //pax->makeRequest(v);
    buf->produce(v);
    //std::this_thread::yield();
    //q.push(v);
    //consumer.unlock();
  }

  mvprintw(0, 0, "main() done\n");
  exit(0);
  return 0;
}
