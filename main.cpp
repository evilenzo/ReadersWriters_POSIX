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
#define WRITERS_AMOUNT 3

#define REPEATS_AMOUNT 3

#define READ_DELAY 2
#define WRITE_DELAY 3

#define RANDOMIZE_DELAY true  // Randomly add 0-3 to upper values



#include <pthread.h>
#include <unistd.h>

#include <iostream>
#include <random>


template <typename T>
struct SyncStorage;

template <>
struct SyncStorage<void> {
  mutable pthread_mutex_t m = {};

  SyncStorage() { pthread_mutex_init(&m, nullptr); }
  ~SyncStorage() { pthread_mutex_destroy(&m); }
};

template <typename T>
struct SyncStorage : SyncStorage<void> {
  T value = {};
};

struct ReadWriteSync {
  explicit ReadWriteSync(uint32_t value_) : value(value_) {}
  uint32_t value;

  SyncStorage<void> read;
  SyncStorage<void> write;
  SyncStorage<size_t> readers_count;
  SyncStorage<size_t> writers_count;

  SyncStorage<void> cout;
};

void* reader(void* args) {
  const auto resource = reinterpret_cast<ReadWriteSync*>(args);

  uint32_t repeats_amount = REPEATS_AMOUNT;

  std::random_device r;
  std::mt19937 eng{r()};
  std::uniform_int_distribution<int> dist(0, 3);

  while (repeats_amount > 0) {
    // Adding ourselves to readers and locking write
    pthread_mutex_lock(&resource->read.m);
    pthread_mutex_lock(&resource->readers_count.m);

    ++resource->readers_count.value;
    if (resource->readers_count.value == 1) {
      pthread_mutex_lock(&resource->write.m);
    }

    pthread_mutex_unlock(&resource->readers_count.m);
    pthread_mutex_unlock(&resource->read.m);


    // Read and output
    pthread_mutex_lock(&resource->cout.m);
    std::cout << resource->value << std::endl;
    pthread_mutex_unlock(&resource->cout.m);


    // Remove ourselves from readers and return access for writing if no more
    // readers exist
    pthread_mutex_lock(&resource->readers_count.m);

    --resource->readers_count.value;
    if (resource->readers_count.value == 0) {
      pthread_mutex_unlock(&resource->write.m);
    }

    pthread_mutex_unlock(&resource->readers_count.m);


    // Delay
    size_t randomized = 0;
    if (RANDOMIZE_DELAY) {
      randomized = dist(eng);
    }
    sleep(READ_DELAY + randomized);
    --repeats_amount;
  }

  return nullptr;
}

void* writer(void* args) {
  const auto resource = reinterpret_cast<ReadWriteSync*>(args);

  uint32_t repeats_amount = REPEATS_AMOUNT;

  std::random_device r;
  std::mt19937 eng{r()};
  std::uniform_int_distribution<int> dist(0, 3);

  while (repeats_amount > 0) {
    // Adding ourselves to writers and locking read
    pthread_mutex_lock(&resource->writers_count.m);

    ++resource->writers_count.value;
    if (resource->writers_count.value == 1) {
      pthread_mutex_lock(&resource->read.m);
    }

    pthread_mutex_unlock(&resource->writers_count.m);

    // Writing
    pthread_mutex_lock(&resource->write.m);
    ++resource->value;
    pthread_mutex_unlock(&resource->write.m);


    // Remove ourselves from writers and return access for reading if no more
    // writers exist
    pthread_mutex_lock(&resource->writers_count.m);

    --resource->writers_count.value;
    if (resource->writers_count.value == 0) {
      pthread_mutex_unlock(&resource->read.m);
    }

    pthread_mutex_unlock(&resource->writers_count.m);


    // Delay
    int randomized = 0;
    if (RANDOMIZE_DELAY) {
      randomized = dist(eng);
    }
    sleep(WRITE_DELAY + randomized);
    --repeats_amount;
  }

  return nullptr;
}

int main() {
  ReadWriteSync resource{1};
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