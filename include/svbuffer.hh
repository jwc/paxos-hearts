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

public:
  SVBuffer() : SVBuffer(0) {}

  SVBuffer(int blocking) : Buffer<T>(blocking) {
    consumer.lock();
    full = 0;
  }

  void produce(T data) override {
    if (this->blocking) producer.lock();
    else if ( ! producer.try_lock()) return;

    this->data = data;
    full = 1;

    consumer.unlock();
    std::this_thread::sleep_for(SVBuffer::TIMEOUT);
  }

  T consume() override {
    consumer.lock();

    T data = this->data;
    full = 0;

    producer.unlock();
    return data;
  }

  int count() override { return full; }
};

#endif // SVBUFFER_HH
       
