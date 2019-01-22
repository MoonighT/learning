#include <iostream>
#include <chrono>
#include <thread>
#include "thread_safe_queue.h"


thread_safe_queue<int> myqueue;
void push() {
    for(int i=0; i<10; i++) {
        std::cout << "push " << i << std::endl;
        myqueue.push(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void pop() {
    int value;
    for(int i=0; i<10; i++) {
        myqueue.wait_and_pop(value);
        std::cout << value << std::endl;
    }
}

int main() {
    std::thread t1(push);
    std::thread t2(pop);
    t1.join();
    t2.join();
    std::cout << "done" << std::endl;
    return 0;
}
