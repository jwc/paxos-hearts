#ifndef BUFFER_HH
#define BUFFER_HH

/**
 * Buffer Interface
 *
 * Currently implemented in SVBuffer (Single Value) and FLBuffer (Fixed Length)
 */

template <typename T> class Buffer {
  public:

    /**
     * @brief Initalizes the buffer.
     */
    Buffer() : blocking(0) { ; } 

    /**
     * @brief Initalizes the buffer.
     *
     * @param blocking Boolean for is produce() should wait when the buffer is full.
     */
    Buffer(int blocking) : blocking(blocking) { ; } 

    /**
     * @brief Puts an item into the buffer.
     *
     * @param item A pointer to the item to be added. 
     */
    virtual void produce(T item) = 0; 

    /**
     * @brief A blocking call to get an item from the buffer.
     *
     * @returns A pointer to the item retrieved.
     */
    virtual T consume() = 0;

    /**
     * @brief Gets the number of items currently in the buffer.
     *
     * @return The number of items currently in the buffer. 
     */
    virtual int count() = 0;

  protected:
    int blocking;
};

#endif // BUFFER_HH

