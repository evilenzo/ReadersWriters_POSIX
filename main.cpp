/*
 * Данная программа реализует схему решения задачи
 * "читатели-писатели", используя стандарт POSIX.
 *
 * Приоритет писателя.
 * "Как только появился хоть один писатель, никого больше не пускать.
 * Все остальные могут простаивать"
 *
 * Количество читателей, писателей и повторений, задержка, а также возможность
 * случайно генерировать задержку (добавляя случайное число от 0-3
 * к задержке) чтения/записи устанавливаются директивами препроцессора.
 */



#define READERS_NUM 5  // Количество читателей
#define WRITERS_NUM 3  // Количество писателей

#define NUM_OF_REPEATS 3  // Количество повторений

#define READ_DELAY 2   // Задержка чтения
#define WRITE_DELAY 3  // Задержка записи

#define RANDOMIZE_DELAY false  // Рандомизировать задержку



#include <iostream>
#include <pthread.h>
#include <unistd.h>

/// Общий ресурс для чтения-записи
int resource = 1;

// Mutex для читателей
struct ReadMutex {
    pthread_mutex_t m = {};

    ReadMutex() { pthread_mutex_init(&m, nullptr); }
    ~ReadMutex() { pthread_mutex_destroy(&m); }
} read_mutex;

// Mutex для писателей
struct WriteMutex {
    pthread_mutex_t m = {};

    int a;

    WriteMutex() { pthread_mutex_init(&m, nullptr); }
    ~WriteMutex() { pthread_mutex_destroy(&m); }
} write_mutex;

// Mutex для вывода
struct OutputMutex {
    pthread_mutex_t m = {};

    OutputMutex() { pthread_mutex_init(&m, nullptr); }
    ~OutputMutex() { pthread_mutex_destroy(&m); }
} output_mutex;

// Количество читателей
struct ReadersCount {
    int i = 0;
    pthread_mutex_t m = {};

    ReadersCount() { pthread_mutex_init(&m, nullptr); }
    ~ReadersCount() { pthread_mutex_destroy(&m); }
} readers_count;

// Количество писателей
struct WritersCount {
    int i = 0;
    pthread_mutex_t m = {};

    WritersCount() { pthread_mutex_init(&m, nullptr); }
    ~WritersCount() { pthread_mutex_destroy(&m); }
} writers_count;

void* reader(void* args) {
    int repeats_num = NUM_OF_REPEATS;

    while (repeats_num > 0) {
        // Блокируем mutex чтения и количества читателей
        pthread_mutex_lock(&read_mutex.m);
        pthread_mutex_lock(&readers_count.m);

        // Добавляем себя в читатели
        ++readers_count.i;
        if (readers_count.i == 1) {
            // Блокируем mutex на запись
            pthread_mutex_lock(&write_mutex.m);
        }

        // Разблокируем mutex чтения и количества читателей
        pthread_mutex_unlock(&readers_count.m);
        pthread_mutex_unlock(&read_mutex.m);

        // Читаем и выводим
        pthread_mutex_lock(&output_mutex.m);
        std::cout << resource << std::endl;
        pthread_mutex_unlock(&output_mutex.m);

        // Блокируем mutex количества читателей
        pthread_mutex_lock(&readers_count.m);

        // Убираем себя из читателей
        --readers_count.i;
        if (readers_count.i == 0) {
            // Читателей нет, разрешаю писать
            pthread_mutex_unlock(&write_mutex.m);
        }

        // Разблокируем mutex количества читателей
        pthread_mutex_unlock(&readers_count.m);

        // Ожидаем
        int randomized = 0;

        if (RANDOMIZE_DELAY) {
            randomized = rand() % 4;
        }

        sleep(READ_DELAY + randomized);

        --repeats_num;
    }

    return nullptr;
}

void* writer(void* args) {
    int repeats_num = NUM_OF_REPEATS;

    while (repeats_num > 0) {
        // Блокируем mutex количества писателей
        pthread_mutex_lock(&writers_count.m);

        // Добавляем себя в писатели
        ++writers_count.i;
        if (writers_count.i == 1) {
            // Блокируем мьютекс на чтение
            pthread_mutex_lock(&read_mutex.m);
        }

        // Разблокируем mutex количества писателей
        pthread_mutex_unlock(&writers_count.m);

        // Блокируем mutex на запись
        pthread_mutex_lock(&write_mutex.m);

        // Пишем
        ++resource;

        // Разблокируем mutex на запись
        pthread_mutex_unlock(&write_mutex.m);

        // Блокируем mutex количества писателей
        pthread_mutex_lock(&writers_count.m);

        // Убираем себя из писателей
        --writers_count.i;
        if (writers_count.i == 0) {
            // Писателей нет, разрешаю читать
            pthread_mutex_unlock(&read_mutex.m);
        }

        // Разблокируем mutex количества писателей
        pthread_mutex_unlock(&writers_count.m);

        // Ожидаем
        int randomized = 0;

        if (RANDOMIZE_DELAY) {
            randomized = rand() % 4;
        }

        sleep(WRITE_DELAY + randomized);

        --repeats_num;
    }

    return nullptr;
}

int main() {
    pthread_t threads[READERS_NUM + WRITERS_NUM];

    std::size_t i = 0;

    for (; i < READERS_NUM; ++i) {
        pthread_create(&threads[i], NULL, reader, NULL);
    }

    for (int j = 0; j < WRITERS_NUM; ++j, ++i) {
        pthread_create(&threads[i], NULL, writer, NULL);
    }

    for (i = 0; i < READERS_NUM + WRITERS_NUM; ++i) {
        pthread_join(threads[i], nullptr);
    }
}