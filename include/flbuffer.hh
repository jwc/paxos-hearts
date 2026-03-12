#ifndef FLBUFFER_HH
#define FLBUFFER_HH

//#define BUFFER_SIZE 50

/**
 * Fixed Length Buffer
 *
 * Buffer implemented with a simple producer-consumer lock.
 *
 * For explanations of the functions, see "buffer.hh"
 */

template <typename T> class FLBuffer : public Buffer<T> {
private:
  std::mutex                              lock;
  std::counting_semaphore<FLBUFFER_SIZE>  consumer;
  std::counting_semaphore<FLBUFFER_SIZE>  producer;
  T                                       data[FLBUFFER_SIZE+1];
  int                                     start;
  int                                     end;

public:
  FLBuffer() : FLBuffer(0) {} 

  FLBuffer(int blocking) : Buffer<T>(blocking), 
      consumer(0), producer(FLBUFFER_SIZE) {
    start = end = 0;
  }

  void produce(T item) override {
    if (this->blocking) producer.acquire();
    else if ( ! producer.try_acquire()) return;
    lock.lock();

    if (count() + 10 > FLBUFFER_SIZE) fprintf(stderr, "FLBUFFER NEAR FULL!\n");

    data[end] = item;
    end = (end + 1) % (FLBUFFER_SIZE+1);

    lock.unlock();
    consumer.release();
    std::this_thread::sleep_for(FLBuffer::TIMEOUT);
  }

  T consume() override {
    consumer.acquire();
    lock.lock();

    T item = data[start];
    start = (start + 1) % (FLBUFFER_SIZE+1);

    lock.unlock();
    producer.release();

    return item;
  }

  int count() override {
    return end >= start ? end - start : end + FLBUFFER_SIZE - start; 
  }
};

#endif // FLBUFFER_HH
       
