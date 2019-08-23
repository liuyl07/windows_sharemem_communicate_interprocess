#include <boost/interprocess/windows_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <cstring>
#include <cstdlib>
#include <string>
#include <chrono>
#include <ratio>
#include <ctime>
#include <iostream>
#include <thread>

#define PATCH_COUNT (4096/(8+8))

int main(int argc, char *argv[])
{
    using namespace boost::interprocess;

    using std::chrono::system_clock;
    system_clock::time_point prev, current;
    std::time_t tt;
    std::chrono::duration<double, std::ratio<1, 1000>> dtn;

    unsigned long long currentIndex = 0,  expectIndex = 1, resetIndex = 0;

    size_t patchSize = sizeof(expectIndex) + sizeof(tt);
    size_t patchCount = patchSize * PATCH_COUNT;
    size_t step1 = sizeof(expectIndex), offset = 0;

    //Open already created shared memory object.
    windows_shared_memory shm(open_only, "MySharedMemory", read_write);

    //Map the whole shared memory in this process
    mapped_region region(shm, read_write);

    //Check that memory was initialized to 1
    char *mem = static_cast<char*>(region.get_address());
    printf("Sharedmem checker is running\n");

    offset = 0;
    currentIndex = 0;
    expectIndex = 1; // start from 1
    while (true)
    {
        //std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        std::memcpy(&currentIndex, mem + offset, sizeof(currentIndex));
        if (currentIndex == expectIndex)
        {
            std::memcpy(&tt, mem + offset + step1, sizeof(tt));
            prev = system_clock::from_time_t(tt);
            current = system_clock::now();
            dtn = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1000>>>(current - prev);
            printf("index = %d\ttime_t = %d\tTransfering needs %fms\n", currentIndex, tt, dtn.count());

            offset += patchSize;
            if (offset >= region.get_size()) offset -= region.get_size();
            expectIndex++;
        }
    }

    return 0;
}