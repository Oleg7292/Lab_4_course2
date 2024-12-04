#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>

using namespace std;

class Waiter {
private:
    mutex mtx; // Для синхронизации
    condition_variable cv;
    vector<bool> forks; // Состояние вилок: true — свободна, false — занята

public:
    Waiter(int totalForks) : forks(totalForks, true) {}

    // Запрос разрешения у официанта
    void requestPermission(int leftFork, int rightFork) {
        unique_lock<mutex> lock(mtx);
        // Ждём, пока обе вилки станут свободными
        cv.wait(lock, [this, leftFork, rightFork]() { 
            return forks[leftFork] && forks[rightFork]; 
        });

        // Забираем вилки
        forks[leftFork] = false;
        forks[rightFork] = false;
    }

    // Освобождение вилок
    void releaseForks(int leftFork, int rightFork) {
        unique_lock<mutex> lock(mtx);
        // Освобождаем вилки
        forks[leftFork] = true;
        forks[rightFork] = true;

        // Уведомляем ожидающих философов
        cv.notify_all();
    }
};

class Philosopher {
private:
    int id;
    Waiter &waiter;
    mutex &leftFork;
    mutex &rightFork;

public:
    Philosopher(int id, Waiter &waiter, mutex &leftFork, mutex &rightFork)
        : id(id), waiter(waiter), leftFork(leftFork), rightFork(rightFork) {}

    void dine() {
        while (true) {
            think();
            waiter.requestPermission(id, (id + 1) % 5); // Запрос разрешения у официанта
            eat();
            waiter.releaseForks(id, (id + 1) % 5); // Освобождение вилок
        }
    }

    void think() {
        cout << "Философ " << id << " думает...\n";
        this_thread::sleep_for(chrono::milliseconds(1000 + rand() % 1000));
    }

    void eat() {
        lock(leftFork, rightFork); // Захватываем мьютексы
        lock_guard<mutex> leftLock(leftFork, adopt_lock);
        lock_guard<mutex> rightLock(rightFork, adopt_lock);
        cout << "Философ " << id << " ест.\n";
        this_thread::sleep_for(chrono::milliseconds(1000 + rand() % 1000));
        cout << "Философ " << id << " закончил есть.\n";
    }
};



int main() {
    const int numPhilosophers = 5;

    vector<mutex> forks(numPhilosophers); // Создаём мьютексы для вилок
    Waiter waiter(numPhilosophers); // Создаём официанта

    vector<thread> philosopherThreads; // Потоки для философов

    // Создаём философов и потоки
    for (int i = 0; i < numPhilosophers; ++i) {
        philosopherThreads.emplace_back(
            [i, &waiter, &forks]() {
                Philosopher philosopher(i, waiter, forks[i], forks[(i + 1) % numPhilosophers]);
                philosopher.dine(); // Запускаем метод dine() в потоке
            }
        );
    }

    // Ожидаем завершения потоков
    for (auto &thread : philosopherThreads) {
        thread.join();
    }

    return 0;
}