#ifndef TASK_HH
#define TASK_HH

#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <stdio.h>
#include <chrono>

class Task {
public:
  enum Type { BLOCKING, NON_BLOCKING };

  static Task * create() { return new Task(); }

  static void end() { taskManager.end(); }

  void enqueueTask(); 

protected:
  enum Type type;

  Task() : type(Type::NON_BLOCKING) { taskManager.enqueue(this); }

  Task(Type type) : type(type) { taskManager.enqueue(this); }

  ~Task() {}

  virtual void executeTask() { 
    printf("Ran Task\n"); 
    delete this;
  }

private:
  static class TaskManager {
  public:
    TaskManager();

    void enqueue(Task *task);

    void end();

  private:
    std::deque<Task*>       nonblockingTasks;
    std::deque<std::thread> nonblockingThreadPool;
    std::mutex              nonblockingLock;
    std::condition_variable nonblockingCV;
    const static int        maxPoolSize = 16;
    bool                    shouldEnd = false;

    void worker(Type type);
  } taskManager;
};

#endif // TASK_HH

