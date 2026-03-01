#include "header.hh"

int main(int argc, char **argv) {
  if (argc != 5) {
    printf("TODO: Add msg\n");
    exit(1);
  }
  printf("argc:%d\n", argc);

  //Buffer<Message> * incoming = new Buffer<Message>();
  //Buffer<Message> * outgoing = new Buffer<Message>();

  //Networking * net = new Localhost(argv[1]);
  Networking * net = new Lan(argv[1]);
  net->addNode(argv[2]);
  net->addNode(argv[3]);
  net->addNode(argv[4]);
   
  fprintf(stderr, "net N: %d\n", net->getN());
  //sleep(1);
  //Paxos * pax = new Paxos(net->getN(), net->getID(), incoming, outgoing);
  //Paxos * pax = new Paxos(1, net->getID(), *net, net->getBuffer());
  Paxos * pax = new Paxos(*net);

  //sleep(5);
  Value v;
  srand((unsigned long int) &v);

  while (1) {
    sleep(1);
    v.type = rand() % 255;
    v.data = rand() % 255;
    pax->makeRequest(v);
  }
  /*
  sleep(5);
  net->printSocks();
  //char str[MSG_SIZE] = "HI";
  //net->send(1, str);
  sleep(5);
  net->printSocks();
  */
  return 0;
}


