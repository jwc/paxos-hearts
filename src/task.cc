#include "task.hh" 

// Task:

Task* Task::create() { return new Task(); }

void Task::enqueueTask() { taskManager.enqueue(this); }

void Task::end() { taskManager.end(); }

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
  Task *task = nullptr;

  if (type == Type::NON_BLOCKING) {
    while (1) {
      if (task != nullptr) task->executeTask();

      std::unique_lock<std::mutex> lock(nonblockingLock);
      nonblockingCV.wait(lock, [&] { 
        return ( ! nonblockingTasks.empty()) || shouldEnd;
      });

      if (shouldEnd) return;

      task = nonblockingTasks.back();
      nonblockingTasks.pop_back();
    }
  } else {
    while (1) {
      if (task != nullptr) task->executeTask();

      std::unique_lock<std::mutex> lock(blockingLock);
      blockingThreadsWaiting++;
      blockingCV.wait(lock, [&] { 
        return ( ! blockingTasks.empty()) || shouldEnd; 
      });
      blockingThreadsWaiting--;

      if (shouldEnd) return;

      task = blockingTasks.back();
      blockingTasks.pop_back();
    }
  }
}

void Task::TaskManager::enqueue(Task *task) { 
  if (task->type == Type::NON_BLOCKING) {
    std::lock_guard<std::mutex> lock(nonblockingLock);

    nonblockingTasks.push_front(task);

    nonblockingCV.notify_one();
  } else {
    std::lock_guard<std::mutex> lock(blockingLock);


    blockingTasks.push_front(task);
    
    if (blockingThreadsWaiting > 0) {
      blockingCV.notify_one();
    } else if (! shouldEnd) {
      std::thread t(&TaskManager::worker, this, Type::BLOCKING);
      blockingThreadPool.push_back(move(t));
    }
  }
}

void Task::TaskManager::end() { 
  printf("nb:%d, b:%d (%d)\n", nonblockingThreadPool.size(), blockingThreadPool.size(), blockingThreadsWaiting);

  shouldEnd = true; 
  nonblockingCV.notify_all();
  blockingCV.notify_all();

  while ( ! nonblockingThreadPool.empty()) {
    nonblockingThreadPool[0].join();
    nonblockingThreadPool.pop_front();
  }

  while ( ! blockingThreadPool.empty()) {
    blockingThreadPool[0].join();
    blockingThreadPool.pop_front();
  }
  
  while ( ! nonblockingTasks.empty()) {
    delete nonblockingTasks.back();
    nonblockingTasks.pop_back();
  }

  while ( ! blockingTasks.empty()) {
    delete blockingTasks.back();
    blockingTasks.pop_back(); 
  }
}

// Temporary: 

class OtherTask : Task {
private:
  OtherTask() : Task(Type::BLOCKING) {;}

  ~OtherTask() {}

  void executeTask() override {
    printf("Ran OtherTask\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    OtherTask::create();
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
 
  
  //for (int i = 0; i < 5; i++) {
    //OtherTask::create();
  //}
  for (int i = 0; i < 5; i++) Task::create();

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  printf("main ending\n");

  Task::end();
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  printf("main returning\n");
  return 0;
}

