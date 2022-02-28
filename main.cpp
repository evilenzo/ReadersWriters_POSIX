/*
 * Readers-Writers problem solution using POSIX API
 *
 * Writers-preference.
 * "No writer, once added to the queue, shall be kept waiting
 * longer than absolutely necessary."
 *
 * Readers and writers amount, read/write delay
 * or random delay generation option (adding 0-3 to user-defined)
 * can be changed by #define directives
 *
 * P.S. #define directives was one of conditions of this task
 */


#define READERS_AMOUNT 5
#define WRITERS_AMOUNT 4

#define REPEATS_AMOUNT 3

#define READ_DELAY 2
#define WRITE_DELAY 2

#define RANDOMIZE_DELAY true  // Randomly add 0-3 to upper values



#include <pthread.h>
#include <unistd.h>

#include <iostream>
#include <random>



class Delayer {
 public:
  Delayer() : eng(r()), dist(0, 3) {}

  void delay() {
    uint randomized = 0;

    if (RANDOMIZE_DELAY) {
      randomized = dist(eng);
    }

    sleep(READ_DELAY + randomized);
  }

 private:
  std::random_device r = {};
  std::mt19937 eng;
  std::uniform_int_distribution<uint> dist;
};

struct Mutex {
  Mutex() { pthread_mutex_init(&m, nullptr); }
  ~Mutex() { pthread_mutex_destroy(&m); }

  void lock() { pthread_mutex_lock(&m); }
  void unlock() { pthread_mutex_unlock(&m); }

 private:
  pthread_mutex_t m = {};
};

template <typename T>
struct SyncStorage {
  T value = {};
  mutable Mutex mtx;
};

struct SharedResource {
  explicit SharedResource(uint32_t value_) : value(value_) {}
  uint32_t value;

  mutable Mutex read;
  mutable Mutex write;
  mutable Mutex cout;

  mutable SyncStorage<size_t> readers_count;
  mutable SyncStorage<size_t> writers_count;
};

void* reader(void* args) {
  auto resource = reinterpret_cast<const SharedResource*>(args);

  Delayer delayer;
  uint32_t repeats_amount = REPEATS_AMOUNT;

  while (repeats_amount > 0) {
    // Adding ourselves to readers and locking write
    resource->read.lock();
    resource->readers_count.mtx.lock();

    ++resource->readers_count.value;
    if (resource->readers_count.value == 1) {
      resource->write.lock();
    }

    resource->readers_count.mtx.unlock();
    resource->read.unlock();


    // Read and output
    resource->cout.lock();
    std::cout << resource->value << std::endl;
    resource->cout.unlock();


    // Remove ourselves from readers and return access for writing if no more
    // readers exist
    resource->readers_count.mtx.lock();

    --resource->readers_count.value;
    if (resource->readers_count.value == 0) {
      resource->write.unlock();
    }

    resource->readers_count.mtx.unlock();

    delayer.delay();

    --repeats_amount;
  }

  return nullptr;
}

void* writer(void* args) {
  const auto resource = reinterpret_cast<SharedResource*>(args);

  Delayer delayer;
  uint32_t repeats_amount = REPEATS_AMOUNT;

  while (repeats_amount > 0) {
    // Adding ourselves to writers and locking read
    resource->writers_count.mtx.lock();

    ++resource->writers_count.value;
    if (resource->writers_count.value == 1) {
      resource->read.lock();
    }

    resource->writers_count.mtx.unlock();

    // Writing
    resource->write.lock();
    ++resource->value;
    resource->write.unlock();


    // Remove ourselves from writers and return access for reading if no more
    // writers exist
    resource->writers_count.mtx.lock();

    --resource->writers_count.value;
    if (resource->writers_count.value == 0) {
      resource->read.unlock();
    }

    resource->writers_count.mtx.unlock();

    delayer.delay();

    --repeats_amount;
  }

  return nullptr;
}

int main() {
  SharedResource resource{1};  // Shared resource for read-write
  pthread_t threads[READERS_AMOUNT + WRITERS_AMOUNT];
  size_t i = 0;

  for (; i < READERS_AMOUNT; ++i) {
    pthread_create(&threads[i], nullptr, reader, &resource);
  }

  for (int j = 0; j < WRITERS_AMOUNT; ++j, ++i) {
    pthread_create(&threads[i], nullptr, writer, &resource);
  }

  for (i = 0; i < READERS_AMOUNT + WRITERS_AMOUNT; ++i) {
    pthread_join(threads[i], nullptr);
  }
}