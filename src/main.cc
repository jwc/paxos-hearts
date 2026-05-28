#include "header.hh"

class OtherTask : Task {
private:
  OtherTask() : Task(Type::BLOCKING, 250) { ready(); }

  void executeTask() override {
    printf("Ran OtherTask\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    OtherTask::create();
  } 

public:
  static void create() { new OtherTask(); }
};

int main() {
  //Task::create(150000000);
  //Task::create(500);
  //Task::create(1500);
  printf("Created\n");
  OtherTask::create();

  IPv4 net1 = IPv4("node1", "127.0.0.1:8989");
  IPv4 net2 = IPv4("node2", "127.0.0.1:9898");

  net1.addNode("node2", "127.0.0.1:9898");
  net2.addNode("node1", "127.0.0.1:8989");
  char m1[] = "Hello World!";
  net1.sendMessage("node2", 5, m1);
  net2.sendMessage("node1", strlen(m1), m1);
  
  //for (int i = 0; i < 5; i++) { OtherTask::create(); }
  //for (int i = 0; i < 5; i++) Task::create();
  
  std::this_thread::sleep_for(std::chrono::milliseconds(6000));
  printf("main ending\n");
  Task::end();

  printf("main returning\n");
  return 0;
}

