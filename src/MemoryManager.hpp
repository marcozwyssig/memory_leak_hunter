#pragma once

#include <cstdlib>
#include <iostream>
#include <mutex>
#include <new> // For std::nothrow

namespace Lib 
{
    class MemoryManager
    {
    public:
        struct MemoryInfo
        {
            const char* File;
            int Line;
            unsigned int Size;
            MemoryInfo* pPrev_;
            MemoryInfo* pNext_;
        };

        MemoryManager();
        ~MemoryManager();

        void* alloc(size_t size, const char* file, int line);
        void free(void* pObject);
        void print(std::ostream &rOut) const;
        unsigned int getLeaks() const;

        class MemoryWatchIf
        {
        public:
            static void* operator new(size_t size);
            static void* operator new(size_t size, char* file, int line);
            static void* operator new[](size_t size);
            static void* operator new[](size_t size, char* file, int line);
            static void operator delete(void* pObject);
            static void operator delete(void* pObject, char* file, int line);
            static void operator delete[](void* pObject);
            static void operator delete[](void* pObject, char* file, int line);
        };

    private:
        inline static MemoryInfo* Front_ = nullptr;
        inline static MemoryInfo* Last_ = nullptr;
        mutable std::mutex cr_;
    };

    class Singleton
    {
    public:
        static MemoryManager& instance()
        {
            static MemoryManager instance;
            return instance;
        }
    };
}

// Macro to overload new operator using MemoryManager with file and line information
#define F_NEW new(__FILE__, __LINE__)

void* operator new(size_t size, const char* file, int line);

void operator delete(void* pObject);