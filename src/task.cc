#include "task.hh" 

// Task:

void Task::enqueueTask() {
  if (this->type == Type::NON_BLOCKING) taskManager.enqueue(this);
}

// TaskManager: 

Task::TaskManager Task::taskManager;

Task::TaskManager::TaskManager() {
  int poolSize = std::thread::hardware_concurrency(); 
  poolSize = poolSize > maxPoolSize ? maxPoolSize : poolSize;
  for (int i = 0; i < poolSize; i++) {
    std::thread t(&TaskManager::worker, this, Type::NON_BLOCKING);
    nonblockingThreadPool.push_back(move(t));
  }
}

void Task::TaskManager::worker(Type type) {
  if (type == Type::NON_BLOCKING) {
    while (1) {
      std::unique_lock<std::mutex> lock(nonblockingLock);
      nonblockingCV.wait(lock, [&] { 
          return ( ! nonblockingTasks.empty()) || shouldEnd;
      });

      if (shouldEnd) return;

      printf("Worker\n");
      Task *task = nonblockingTasks.back();
      nonblockingTasks.pop_back();

      task->executeTask();
    }
  }
}

void Task::TaskManager::enqueue(Task *task) { 
  if (task->type == Type::NON_BLOCKING) {
    std::lock_guard<std::mutex> lock(nonblockingLock);

    nonblockingTasks.push_front(task);
    printf("Enqueued. %d\n", nonblockingThreadPool.size());

    nonblockingCV.notify_one();
  } 
}

void Task::TaskManager::end() { 
  shouldEnd = true; 
  nonblockingCV.notify_all();

  while ( ! nonblockingThreadPool.empty()) {
    nonblockingThreadPool[0].join();
    nonblockingThreadPool.pop_front();
  }
}

// Temporary: 

class OtherTask : Task {
private:
  OtherTask() : Task(Type::NON_BLOCKING) {;}

  ~OtherTask() {}

  void executeTask() override {
    printf("Ran OtherTask\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    delete this;
  } 

public:
  static OtherTask * create() { return new OtherTask(); }
};

int main() {
  //OtherTask *task = new OtherTask(Task::Type::NON_BLOCKING);
  //Task *task2 = new Task(Task::Type::NON_BLOCKING);
  //OtherTask task3 = OtherTask();
  //Task task4 = Task();
  Task *task5 = Task::create();
  OtherTask *task6 = OtherTask::create();
  
  //task->enqueueTask();
  //task2->enqueueTask();
  //task->enqueueTask();
  //task3.enqueueTask();

  //delete task;
  //delete task2;

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  printf("main ending\n");

  Task::end();
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  printf("main returning\n");
  return 0;
}

