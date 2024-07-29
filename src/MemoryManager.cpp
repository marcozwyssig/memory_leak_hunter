/* Header of this module -----------------------------------------------------*/
#include "MemoryManager.hpp"
#include <iostream>

/*-----------------------------------------------------------------------------+
|   Namespaces                                                                 |
+-----------------------------------------------------------------------------*/
namespace Lib
{
    /*-----------------------------------------------------------------------------+
    |   Methods                                                                    |
    +-----------------------------------------------------------------------------*/

    MemoryManager::MemoryManager()
    {
    }

    void *MemoryManager::alloc(size_t size, const char *file, int line)
    {
        void *paObject = malloc(size + sizeof(MemoryInfo));
        if (paObject)
        {
            MemoryInfo *paInfo = (MemoryInfo *)paObject;
            paInfo->File = file;
            paInfo->Line = line;
            paInfo->Size = static_cast<unsigned int>(size);
            std::lock_guard<std::mutex> l(cr_);
            if (Front_)
            {
                paInfo->pPrev_ = Last_;
                paInfo->pNext_ = 0;
                Last_ = paInfo;
                paInfo->pPrev_->pNext_ = paInfo;
            }
            else
            {
                Front_ = paInfo;
                Last_ = paInfo;
                paInfo->pNext_ = 0;
                paInfo->pPrev_ = 0;
            }
        }
        return (char *)paObject + sizeof(MemoryInfo);
    }

    void MemoryManager::free(void *pObject)
    {
        if (pObject)
        {
            MemoryInfo *paInfo = (MemoryInfo *)((char *)pObject - sizeof(MemoryInfo));

            std::lock_guard<std::mutex> l(cr_);
            if (paInfo->pPrev_)
            {
                paInfo->pPrev_->pNext_ = paInfo->pNext_;
            }
            else
            {
                Front_ = paInfo->pNext_;
            }

            if (paInfo->pNext_)
            {
                paInfo->pNext_->pPrev_ = paInfo->pPrev_;
            }
            else
            {
                Last_ = paInfo->pPrev_;
            }

            paInfo->pNext_ = 0;
            paInfo->pPrev_ = 0;

            ::free(paInfo);
        }
    }

    void MemoryManager::print(std::ostream &rOut) const
    {
        std::lock_guard<std::mutex> l(cr_);
        if (Last_)
        {
            int i = 0;

            rOut << "-->  Reporting MemoryLeaks " << std::endl;
            unsigned int nBytes = 0;
            for (MemoryInfo *paInfo = Front_; paInfo != 0; paInfo = paInfo->pNext_)
            {
                rOut << "Object lost @ " << paInfo->File << ":" << paInfo->Line << "; Bytes:" << paInfo->Size << std::endl;
                ++i;
                nBytes += paInfo->Size;
            }
            rOut << "--> MemoryManager: Total Leaks: " << i << "; Total Bytes: " << nBytes << std::endl;
        }
    }

    unsigned int MemoryManager::getLeaks() const
    {
        std::lock_guard<std::mutex> l(cr_);
        unsigned int leaks = 0;
        if (Last_)
        {
            for (MemoryInfo *paInfo = Front_; paInfo != 0; paInfo = paInfo->pNext_)
                ++leaks;
        }
        return leaks;
    }

    MemoryManager::~MemoryManager()
    {
    }

    void *MemoryManager::MemoryWatchIf::operator new(size_t size)
    {
        return Singleton::instance().alloc(size, "file is unknown", 0);
    }

    void *MemoryManager::MemoryWatchIf::operator new(size_t size, char *file, int line)
    {
        return Singleton::instance().alloc(size, file, line);
    }

    void *MemoryManager::MemoryWatchIf::operator new[](size_t size)
    {
        return Singleton::instance().alloc(size, "file is unknown", 0);
    }

    void *MemoryManager::MemoryWatchIf::operator new[](size_t size, char *file, int line)
    {
        return Singleton::instance().alloc(size, file, line);
    }

    void MemoryManager::MemoryWatchIf::operator delete(void *pObject)
    {
        Singleton::instance().free(pObject);
    }

    void MemoryManager::MemoryWatchIf::operator delete(void *pObject, char *file, int line)
    {
        Singleton::instance().free(pObject);
    }

    void MemoryManager::MemoryWatchIf::operator delete[](void *pObject)
    {
        Singleton::instance().free(pObject);
    }

    void MemoryManager::MemoryWatchIf::operator delete[](void *pObject, char *file, int line)
    {
        Singleton::instance().free(pObject);
    }
}

void *operator new(size_t size, const char *file, int line)
{
    return Lib::Singleton::instance().alloc(size, const_cast<char *>(file), line);
}

void operator delete(void *pObject)
{
    Lib::Singleton::instance().free(pObject);
}