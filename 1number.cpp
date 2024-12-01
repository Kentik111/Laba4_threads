#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <random>
#include <algorithm>
using namespace std;

// Функция для генерации случайного символа ASCII
char generateRandomChar() {
    static random_device rd; // Генератор случайных чисел
    static mt19937 gen(rd()); // Вихрь Мерсенна для генерации случайных чисел
    static uniform_int_distribution<> dis(32, 126); // Диапазон ASCII символов
    return static_cast<char>(dis(gen)); // Возвращаем случайный символ
}

// Функция для тестирования мьютекса
std::chrono::duration<double> testMutex(int numThreads) {
    mutex mutex; // Мьютекс для синхронизации доступа к общему ресурсу
    auto startTime = chrono::high_resolution_clock::now(); // Засекаем время начала

    vector<thread> threads; // Вектор для хранения потоков
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([i, &mutex]() {
            lock_guard<std::mutex> lock(mutex); // Блокируем мьютекс
            char randomChar = generateRandomChar(); // Генерируем случайный символ
            cout << "Поток " << i + 1 << ": Случайный символ ASCII: " << randomChar << endl; // Выводим символ
        });
    }

    for (auto& t : threads) {
        t.join(); // Дожидаемся завершения всех потоков
    }

    auto endTime = std::chrono::high_resolution_clock::now(); // Засекаем время окончания
    return endTime - startTime; // Возвращаем время выполнения
}

// Функция для тестирования семафора
std::chrono::duration<double> testSemaphore(int numThreads) {
    vector<thread> threads; // Вектор для хранения потоков
    mutex mutex; // Мьютекс для синхронизации доступа к общему ресурсу
    condition_variable cv; // Условная переменная для ожидания
    int count = 0; // Счетчик потоков, ожидающих доступа
    auto startTime = std::chrono::high_resolution_clock::now(); // Засекаем время начала

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([i, &mutex, &cv, &count, numThreads]() {
            unique_lock<std::mutex> lock(mutex); // Блокируем мьютекс
            cv.wait(lock, [&count, numThreads]() { return count < numThreads / 2; }); // Ожидаем, пока счетчик меньше половины потоков
            ++count; // Увеличиваем счетчик
            char randomChar = generateRandomChar(); // Генерируем случайный символ
            cout << "Поток " << i + 1 << ": Случайный символ ASCII: " << randomChar << endl; // Выводим символ
            --count; // Уменьшаем счетчик
            cv.notify_one(); // Уведомляем один из ожидающих потоков
        });
    }

    for (auto& t : threads) {
        t.join(); // Дожидаемся завершения всех потоков
    }

    auto endTime = std::chrono::high_resolution_clock::now(); // Засекаем время окончания
    return endTime - startTime; // Возвращаем время выполнения
}

// Функция для тестирования семафора с ограничением на 1 поток
std::chrono::duration<double> testSemaphoreSlim(int numThreads) {
    vector<thread> threads; // Вектор для хранения потоков
    mutex mutex; // Мьютекс для синхронизации доступа к общему ресурсу
    condition_variable cv; // Условная переменная для ожидания
    bool locked = false; // Флаг, указывающий, занят ли семафор
    auto startTime = std::chrono::high_resolution_clock::now(); // Засекаем время начала

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([i, &mutex, &cv, &locked]() {
            unique_lock<std::mutex> lock(mutex); // Блокируем мьютекс
            cv.wait(lock, [&locked]() { return !locked; }); // Ожидаем, пока семафор не будет свободен
            locked = true; // Блокируем семафор
            char randomChar = generateRandomChar(); // Генерируем случайный символ
            cout << "Поток " << i + 1 << ": Случайный символ ASCII: " << randomChar << endl; // Выводим символ
            locked = false; // Освобождаем семафор
            cv.notify_one(); // Уведомляем один из ожидающих потоков
        });
    }

    for (auto& t : threads) {
        t.join(); // Дожидаемся завершения всех потоков
    }

    auto endTime = std::chrono::high_resolution_clock::now(); // Засекаем время окончания
    return endTime - startTime; // Возвращаем время выполнения
}

// Функция для тестирования SpinWait
std::chrono::duration<double> testSpinWait(int numThreads) {
    vector<thread> threads; // Вектор для хранения потоков
    auto startTime = std::chrono::high_resolution_clock::now(); // Засекаем время начала

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([i]() {
            for (int j = 0; j < 1000000; ++j) {} // Имитация работы
            char randomChar = generateRandomChar(); // Генерируем случайный символ
            cout << "Поток " << i + 1 << ": Случайный символ ASCII: " << randomChar << endl; // Выводим символ
        });
    }

    for (auto& t : threads) {
        t.join(); // Дожидаемся завершения всех потоков
    }

    auto endTime = std::chrono::high_resolution_clock::now(); // Засекаем время окончания
    return endTime - startTime; // Возвращаем время выполнения
}

// Функция для тестирования барьера
std::chrono::duration<double> testBarrier(int numThreads) {
    vector<thread> threads; // Вектор для хранения потоков
    mutex mutex; // Мьютекс для синхронизации доступа к общему ресурсу
    condition_variable cv; // Условная переменная для ожидания
    int count = 0; // Счетчик потоков, достигших барьера
    auto startTime = std::chrono::high_resolution_clock::now(); // Засекаем время начала

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([i, &mutex, &cv, &count, numThreads]() {
            unique_lock<std::mutex> lock(mutex); // Блокируем мьютекс
            cout << "Поток " << i + 1 << " достиг барьера" << endl; // Выводим сообщение о достижении барьера
            ++count; // Увеличиваем счетчик
            if (count == numThreads) {
                cv.notify_all(); // Если все потоки достигли барьера, уведомляем их
            } else {
                cv.wait(lock, [&count, numThreads]() { return count == numThreads; }); // Ожидаем, пока все потоки достигнут барьера
            }
            char randomChar = generateRandomChar(); // Генерируем случайный символ
            cout << "Поток " << i + 1 << " продолжает выполнение: " << randomChar << endl; // Выводим символ
        });
    }

    for (auto& t : threads) {
        t.join(); // Дожидаемся завершения всех потоков
    }

    auto endTime = std::chrono::high_resolution_clock::now(); // Засекаем время окончания
    return endTime - startTime; // Возвращаем время выполнения
}

// Функция для тестирования спинлока
std::chrono::duration<double> testSpinLock(int numThreads) {
    vector<thread> threads; // Вектор для хранения потоков
    atomic<bool> spinLock(false); // Атомарный флаг для спинлока
    auto startTime = std::chrono::high_resolution_clock::now(); // Засекаем время начала

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([i, &spinLock]() {
            while (spinLock.exchange(true, memory_order_acquire)) {
                // Spin until lock is acquired
            }
            char randomChar = generateRandomChar(); // Генерируем случайный символ
            cout << "Поток " << i + 1 << ": Случайный символ ASCII: " << randomChar << endl; // Выводим символ
            spinLock.store(false, memory_order_release); // Освобождаем спинлок
        });
    }

    for (auto& t : threads) {
        t.join(); // Дожидаемся завершения всех потоков
    }

    auto endTime = std::chrono::high_resolution_clock::now(); // Засекаем время окончания
    return endTime - startTime; // Возвращаем время выполнения
}

// Функция для тестирования монитора
std::chrono::duration<double> testMonitor(int numThreads) {
    vector<thread> threads; // Вектор для хранения потоков
    mutex mutex; // Мьютекс для синхронизации доступа к общему ресурсу
    condition_variable cv; // Условная переменная для ожидания
    bool ready = false; // Флаг, указывающий, готовы ли потоки
    auto startTime = std::chrono::high_resolution_clock::now(); // Засекаем время начала

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([i, &mutex, &cv, &ready]() {
            unique_lock<std::mutex> lock(mutex); // Блокируем мьютекс
            cv.wait(lock, [&ready]() { return ready; }); // Ожидаем, пока флаг не станет истинным
            char randomChar = generateRandomChar(); // Генерируем случайный символ
            cout << "Поток " << i + 1 << ": Случайный символ ASCII: " << randomChar << endl; // Выводим символ
        });
    }

    {
        lock_guard<std::mutex> lock(mutex); // Блокируем мьютекс
        ready = true; // Устанавливаем флаг в истину
        cv.notify_all(); // Уведомляем все потоки
    }

    for (auto& t : threads) {
        t.join(); // Дожидаемся завершения всех потоков
    }

    auto endTime = std::chrono::high_resolution_clock::now(); // Засекаем время окончания
    return endTime - startTime; // Возвращаем время выполнения
}

int main() {
    int numThreads = 12; // Количество потоков
    cout << "Количество потоков: " << numThreads << endl;

    cout << "Testing Mutex..." << endl;
    auto timeMutex = testMutex(numThreads);
    cout << "Mutex time: " << timeMutex.count() << " seconds" << endl << endl;

    cout << "Testing Semaphore..." << endl;
    auto timeSemaphore = testSemaphore(numThreads);
    cout << "Semaphore time: " << timeSemaphore.count() << " seconds" << endl << endl;

    cout << "Testing SemaphoreSlim..." << endl;
    auto timeSemaphoreSlim = testSemaphoreSlim(numThreads);
    cout << "SemaphoreSlim time: " << timeSemaphoreSlim.count() << " seconds" << endl << endl;

    cout << "Testing SpinWait..." << endl;
    auto timeSpinWait = testSpinWait(numThreads);
    cout << "SpinWait time: " << timeSpinWait.count() << " seconds" << endl << endl;

    cout << "Testing Barrier..." << endl;
    auto timeBarrier = testBarrier(numThreads);
    cout << "Barrier time: " << timeBarrier.count() << " seconds" << endl << endl;

    cout << "Testing SpinLock..." << endl;
    auto timeSpinLock = testSpinLock(numThreads);
    cout << "SpinLock time: " << timeSpinLock.count() << " seconds" << endl << endl;

    cout << "Testing Monitor..." << endl;
    auto timeMonitor = testMonitor(numThreads);
    cout << "Monitor time: " << timeMonitor.count() << " seconds" << endl << endl;

    return 0;
}