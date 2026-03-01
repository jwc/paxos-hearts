#ifndef SVBUFFER_HH
#define SVBUFFER_HH

// This macro is used to quickly toggle debug code that output the ratio of
// time spent full to time spent empty. 
//#define BUFFER_DEBUG

/**
 * Single Value Buffer
 *
 * A buffer that only can hold a single value. 
 *
 * For explanations of the functions, see "buffer.hh"
 */

template <typename T> class SVBuffer : public Buffer<T> {
private:
  std::mutex  consumer;
  std::mutex  producer;
  T           data;
  char        full;

  #ifdef BUFFER_DEBUG
  double timeConsumer, timeProducer;
  std::chrono::time_point<std::chrono::high_resolution_clock> timeA, timeB;
  #endif // BUFFER_DEBUG

public:
  SVBuffer() : SVBuffer(0) {}

  SVBuffer(int blocking) : Buffer<T>(blocking) {
    #ifdef BUFFER_DEBUG
    timeConsumer = timeProducer = 0;
    timeA = std::chrono::high_resolution_clock::now();
    #endif // BUFFER_DEBUG
         
    consumer.lock();
    full = 0;
  }

  void produce(T data) override {
    if (this->blocking) producer.lock();
    else if ( ! producer.try_lock()) return;

    #ifdef BUFFER_DEBUG
    timeB = std::chrono::high_resolution_clock::now();
    timeConsumer += std::chrono::duration_cast<std::chrono::nanoseconds>(
      timeB - timeA).count();
    double sum = timeConsumer + timeProducer;
    fprintf(stderr, "\tBuf: E:%0.9lf, F:%0.9lf\n", 
      timeConsumer/sum, timeProducer/sum);
    timeA = std::chrono::high_resolution_clock::now();
    #endif // BUFFER_DEBUG

    this->data = data;
    full = 1;
    consumer.unlock();
  }

  T consume() override {
    consumer.lock();

    #ifdef BUFFER_DEBUG
    timeB = std::chrono::high_resolution_clock::now();
    timeProducer += std::chrono::duration_cast<std::chrono::nanoseconds>(
      timeB - timeA).count();
    double sum = timeConsumer + timeProducer;
    fprintf(stderr, "\tBuf: E:%0.9lf, F:%0.9lf\n", 
      timeConsumer/sum, timeProducer/sum);
    timeA = std::chrono::high_resolution_clock::now();
    #endif // BUFFER_DEBUG
         
    T data = this->data;
    full = 0;
    producer.unlock();
    return data;
  }

  int count() override { return full; }
};

#endif // SVBUFFER_HH
       
