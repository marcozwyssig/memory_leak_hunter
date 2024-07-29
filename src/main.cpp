#include "MemoryManager.hpp"

class A {
    public:
        A() {
         
        }
};   

int main() {
    A* ap = F_NEW A();
    Lib::Singleton::instance().print(std::cout);
    delete ap;
    Lib::Singleton::instance().print(std::cout);
    return 0;
}