#include "header.hh"

void foo(Cards * c, Buffer<Value> * buf) {
  //Cards * c = new Cards();
  mvprintw(2, 3, "foo() created");
  Value v;
  int bar = 0;
  while (1) {
    //sleep(2);
    mvprintw(2, 3, "foo() waiting");
    //consumer.lock();
    //v = q.front();
    //q.pop();
    v = buf->consume();
    //producer.unlock();

    //while (1) ;
    
    mvprintw(2, 0, "%d foo() printing %c/%d\n", bar++, v.type, v.type);
    c->printCard(10, 10, v.type % 52);
  }
  
  mvprintw(0, 0, "foo() done\n");
}

int main(int argc, char** argv) {
  if (argc < 2) {
    exit(1);
  }

  //consumer.lock();

  //Networking *net = new Localhost(argv[1]);
  //Paxos *pax = new Paxos(*net);
  Cards * c = new Cards();
  //mvprintw(35, 0, "# threads %d", std::thread::hardware_concurrency());
  Buffer<Value> * buf = new SVBuffer<Value>(1);

  //std::thread output = std::thread(&foo, &pax->getBuffer());
  std::thread output = std::thread(&foo, c, buf);

  mvprintw(0, 0, "main() ready\n");
  Value v;
  v.type = 'a';
  int count = 0;
  while (v.type != 'q') {
    //std::this_thread::yield();
    //producer.lock();
    v.type = getch();

    if (v.type == 'r') {
      mvprintw(11, 0, "sz:%d", buf->count());
      //std::this_thread::yield();
      continue;
    } else if (v.type == 't') {
      c->toggleUnicode();
      c->toggleColor();
      continue;
    }
    mvprintw(0, 0, "%d main() req(%c/%d)\n", count++, v.type, v.type);
    //pax->makeRequest(v);
    buf->produce(v);
    //std::this_thread::yield();
    //q.push(v);
    //consumer.unlock();
  }

  mvprintw(0, 0, "main() done\n");

  return 0;
}
