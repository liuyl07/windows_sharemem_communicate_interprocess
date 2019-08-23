#include <boost/interprocess/windows_shared_memory.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <thread>
#include <sdkddkver.h>
#include <cstring>
#include <cstdlib>
#include <string>
#include <chrono>
#include <ratio>
#include <ctime>
#include <iostream>


#define PATCH_COUNT (4096/(8+8))

int main(int argc, char *argv[])
{
    using namespace boost::interprocess;

    using std::chrono::system_clock;
    system_clock::time_point prev, current;
    std::time_t tt;
    std::chrono::duration<double, std::ratio<1, 1000>> dtn;

     long long frameIndex = 0;
    unsigned long long flag = 0;

    size_t patchSize = sizeof(frameIndex) + sizeof(std::time_t);
    size_t patchCount = patchSize * PATCH_COUNT;
    size_t step1 = sizeof(frameIndex), offset = 0;

    windows_shared_memory shm(create_only, "MySharedMemory", read_write, patchCount);

    mapped_region region(shm, read_write);
    char* address = static_cast<char*>(region.get_address());
    printf("windows_shared_memory checker starts:\n");
    getchar();

    offset = 0;
    frameIndex = 1; // Start from 1
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(10));
        prev = system_clock::now();
        tt = system_clock::to_time_t(prev);
        std::memcpy(address + offset, &frameIndex, sizeof(frameIndex));
        std::memcpy(address + offset + step1, &tt, sizeof(tt));
        current = system_clock::now();
        dtn = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, 1000>>>(current - prev);
        tt = system_clock::to_time_t(current);
        printf("index = %d\ttime_t = %d\tMemcpy needs %fms\n", frameIndex, tt, dtn.count());
        offset += patchSize;
        if (offset >= region.get_size())
        {
            offset -= region.get_size();
        }
        frameIndex++;

        //if (!fmod(frameIndex, 200))  getchar();
    }

    getchar();
    return 0;
}