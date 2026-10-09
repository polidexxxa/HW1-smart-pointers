#include <iostream>
#include <iomanip>
#include <chrono>
#include <memory>
#include <string>

#include "UnqPtr.hpp"
#include "ShrdPtr.hpp"

struct PhaseTiming {
    double rawTimeMs = 0.0;
    double customUnqMs = 0.0;
    double stdUnqMs = 0.0;
    double customShrdMs = 0.0;
    double stdShrdMs = 0.0;
};

struct DualBenchmarkResult {
    PhaseTiming allocPhase;
    PhaseTiming deallocPhase;
};

//одиночные объекты
DualBenchmarkResult RunSingleBenchmark(unsigned int count) {
    DualBenchmarkResult res;

    //Сырые указатели
    {
        int** storage = new int*[count];

        auto startAlloc = std::chrono::high_resolution_clock::now();
        for (unsigned int i = 0; i < count; i++) {
            storage[i] = new int(i);
        }
        auto endAlloc = std::chrono::high_resolution_clock::now();
        res.allocPhase.rawTimeMs = std::chrono::duration<double, std::milli>(endAlloc - startAlloc).count();

        auto startDealloc = std::chrono::high_resolution_clock::now();
        for (unsigned int i = 0; i < count; i++) {
            delete storage[i];
        }
        auto endDealloc = std::chrono::high_resolution_clock::now();
        res.deallocPhase.rawTimeMs = std::chrono::duration<double, std::milli>(endDealloc - startDealloc).count();

        delete[] storage;
    }

    // UnqPtr
    {
        UnqPtr<int>* storage = new UnqPtr<int>[count];

        auto startAlloc = std::chrono::high_resolution_clock::now();
        for (unsigned int i = 0; i < count; i++) {
            storage[i] = MakeUnq<int>(i);
        }
        auto endAlloc = std::chrono::high_resolution_clock::now();
        res.allocPhase.customUnqMs = std::chrono::duration<double, std::milli>(endAlloc - startAlloc).count();

        auto startDealloc = std::chrono::high_resolution_clock::now();
        for (unsigned int i = 0; i < count; i++) {
            storage[i].Reset();
        }
        auto endDealloc = std::chrono::high_resolution_clock::now();
        res.deallocPhase.customUnqMs = std::chrono::duration<double, std::milli>(endDealloc - startDealloc).count();

        delete[] storage;
    }

    //std::unique_ptr
    {
        std::unique_ptr<int>* storage = new std::unique_ptr<int>[count];

        auto startAlloc = std::chrono::high_resolution_clock::now();
        for (unsigned int i = 0; i < count; i++) {
            storage[i] = std::make_unique<int>(i);
        }
        auto endAlloc = std::chrono::high_resolution_clock::now();
        res.allocPhase.stdUnqMs = std::chrono::duration<double, std::milli>(endAlloc - startAlloc).count();

        auto startDealloc = std::chrono::high_resolution_clock::now();
        for (unsigned int i = 0; i < count; i++) {
            storage[i].reset();
        }
        auto endDealloc = std::chrono::high_resolution_clock::now();
        res.deallocPhase.stdUnqMs = std::chrono::duration<double, std::milli>(endDealloc - startDealloc).count();

        delete[] storage;
    }

    // ShrdPtr
    {
        ShrdPtr<int>* storage = new ShrdPtr<int>[count];

        auto startAlloc = std::chrono::high_resolution_clock::now();
        for (unsigned int i = 0; i < count; i++) {
            storage[i] = MakeShrd<int>(i);
        }
        auto endAlloc = std::chrono::high_resolution_clock::now();
        res.allocPhase.customShrdMs = std::chrono::duration<double, std::milli>(endAlloc - startAlloc).count();

        auto startDealloc = std::chrono::high_resolution_clock::now();
        for (unsigned int i = 0; i < count; i++) {
            storage[i].Reset();
        }
        auto endDealloc = std::chrono::high_resolution_clock::now();
        res.deallocPhase.customShrdMs = std::chrono::duration<double, std::milli>(endDealloc - startDealloc).count();

        delete[] storage;
    }

    //std::shared_ptr
    {
        std::shared_ptr<int>* storage = new std::shared_ptr<int>[count];

        auto startAlloc = std::chrono::high_resolution_clock::now();
        for (unsigned int i = 0; i < count; i++) {
            storage[i] = std::make_shared<int>(i);
        }
        auto endAlloc = std::chrono::high_resolution_clock::now();
        res.allocPhase.stdShrdMs = std::chrono::duration<double, std::milli>(endAlloc - startAlloc).count();

        auto startDealloc = std::chrono::high_resolution_clock::now();
        for (unsigned int i = 0; i < count; i++) {
            storage[i].reset();
        }
        auto endDealloc = std::chrono::high_resolution_clock::now();
        res.deallocPhase.stdShrdMs = std::chrono::duration<double, std::milli>(endDealloc - startDealloc).count();

        delete[] storage;
    }

    return res;
}

//массивы
DualBenchmarkResult RunArrayBenchmark(unsigned int arrayCount, unsigned int arrayLen = 64) {
    DualBenchmarkResult res;

    //сырые
    {
        int** storage = new int*[arrayCount];

        auto startAlloc = std::chrono::high_resolution_clock::now();
        for (unsigned int i = 0; i < arrayCount; i++) {
            storage[i] = new int[arrayLen]();
        }
        auto endAlloc = std::chrono::high_resolution_clock::now();
        res.allocPhase.rawTimeMs = std::chrono::duration<double, std::milli>(endAlloc - startAlloc).count();

        auto startDealloc = std::chrono::high_resolution_clock::now();
        for (unsigned int i = 0; i < arrayCount; i++) {
            delete[] storage[i];
        }
        auto endDealloc = std::chrono::high_resolution_clock::now();
        res.deallocPhase.rawTimeMs = std::chrono::duration<double, std::milli>(endDealloc - startDealloc).count();

        delete[] storage;
    }

    // UnqPtr<T[]>
    {
        UnqPtr<int[]>* storage = new UnqPtr<int[]>[arrayCount];

        auto startAlloc = std::chrono::high_resolution_clock::now();
        for (unsigned int i = 0; i < arrayCount; i++) {
            storage[i] = MakeUnq<int[]>(arrayLen);
        }
        auto endAlloc = std::chrono::high_resolution_clock::now();
        res.allocPhase.customUnqMs = std::chrono::duration<double, std::milli>(endAlloc - startAlloc).count();

        auto startDealloc = std::chrono::high_resolution_clock::now();
        for (unsigned int i = 0; i < arrayCount; i++) {
            storage[i].Reset();
        }
        auto endDealloc = std::chrono::high_resolution_clock::now();
        res.deallocPhase.customUnqMs = std::chrono::duration<double, std::milli>(endDealloc - startDealloc).count();

        delete[] storage;
    }

    //std::unique_ptr<T[]>
    {
        std::unique_ptr<int[]>* storage = new std::unique_ptr<int[]>[arrayCount];

        auto startAlloc = std::chrono::high_resolution_clock::now();
        for (unsigned int i = 0; i < arrayCount; i++) {
            storage[i] = std::make_unique<int[]>(arrayLen);
        }
        auto endAlloc = std::chrono::high_resolution_clock::now();
        res.allocPhase.stdUnqMs = std::chrono::duration<double, std::milli>(endAlloc - startAlloc).count();

        auto startDealloc = std::chrono::high_resolution_clock::now();
        for (unsigned int i = 0; i < arrayCount; i++) {
            storage[i].reset();
        }
        auto endDealloc = std::chrono::high_resolution_clock::now();
        res.deallocPhase.stdUnqMs = std::chrono::duration<double, std::milli>(endDealloc - startDealloc).count();

        delete[] storage;
    }

    // ShrdPtr<T[]>
    {
        ShrdPtr<int[]>* storage = new ShrdPtr<int[]>[arrayCount];

        auto startAlloc = std::chrono::high_resolution_clock::now();
        for (unsigned int i = 0; i < arrayCount; i++) {
            storage[i] = MakeShrd<int[]>(arrayLen);
        }
        auto endAlloc = std::chrono::high_resolution_clock::now();
        res.allocPhase.customShrdMs = std::chrono::duration<double, std::milli>(endAlloc - startAlloc).count();

        auto startDealloc = std::chrono::high_resolution_clock::now();
        for (unsigned int i = 0; i < arrayCount; i++) {
            storage[i].Reset();
        }
        auto endDealloc = std::chrono::high_resolution_clock::now();
        res.deallocPhase.customShrdMs = std::chrono::duration<double, std::milli>(endDealloc - startDealloc).count();

        delete[] storage;
    }

    //std::shared_ptr<T[]>
    {
        std::shared_ptr<int[]>* storage = new std::shared_ptr<int[]>[arrayCount];

        auto startAlloc = std::chrono::high_resolution_clock::now();
        for (unsigned int i = 0; i < arrayCount; i++) {
            storage[i] = std::shared_ptr<int[]>(new int[arrayLen]());
        }
        auto endAlloc = std::chrono::high_resolution_clock::now();
        res.allocPhase.stdShrdMs = std::chrono::duration<double, std::milli>(endAlloc - startAlloc).count();

        auto startDealloc = std::chrono::high_resolution_clock::now();
        for (unsigned int i = 0; i < arrayCount; i++) {
            storage[i].reset();
        }
        auto endDealloc = std::chrono::high_resolution_clock::now();
        res.deallocPhase.stdShrdMs = std::chrono::duration<double, std::milli>(endDealloc - startDealloc).count();

        delete[] storage;
    }

    return res;
}

//таблицы
void PrintTable(const std::string& title, 
                const unsigned int* counts, 
                unsigned int numTests,
                const PhaseTiming* timings, 
                const std::string& colName = "Count (N)") {
    std::cout << "\n=========================================================================================\n";
    std::cout << "  " << title << "\n";
    std::cout << "=========================================================================================\n";

    std::cout << std::left 
              << std::setw(15) << colName
              << std::setw(15) << "Raw Pointer"
              << std::setw(15) << "Custom UnqPtr"
              << std::setw(15) << "std::unique"
              << std::setw(15) << "Custom Shrd"
              << std::setw(15) << "std::shared"
              << "\n";
    std::cout << std::string(90, '-') << "\n";

    for (unsigned int i = 0; i < numTests; i++) {
        std::cout << std::left
                  << std::setw(15) << counts[i]
                  << std::fixed << std::setprecision(3)
                  << std::setw(15) << timings[i].rawTimeMs
                  << std::setw(15) << timings[i].customUnqMs
                  << std::setw(15) << timings[i].stdUnqMs
                  << std::setw(15) << timings[i].customShrdMs
                  << std::setw(15) << timings[i].stdShrdMs
                  << "\n";
    }
}

int main() {
    const unsigned int singleCounts[] = { 1000, 10000, 100000, 1000000 };
    const unsigned int numSingle = sizeof(singleCounts) / sizeof(singleCounts[0]);
    PhaseTiming singleAllocs[numSingle];
    PhaseTiming singleDeallocs[numSingle];

    for (unsigned int i = 0; i < numSingle; i++) {
        DualBenchmarkResult r = RunSingleBenchmark(singleCounts[i]);
        singleAllocs[i] = r.allocPhase;
        singleDeallocs[i] = r.deallocPhase;
    }

    PrintTable("ALLOCATION TIME - SINGLE OBJECTS (ms)", singleCounts, numSingle, singleAllocs, "Objects (N)");
    PrintTable("DEALLOCATION TIME - SINGLE OBJECTS (ms)", singleCounts, numSingle, singleDeallocs, "Objects (N)");

    const unsigned int arrayCounts[] = { 1000, 10000, 50000, 200000 };
    const unsigned int numArrays = sizeof(arrayCounts) / sizeof(arrayCounts[0]);
    PhaseTiming arrayAllocs[numArrays];
    PhaseTiming arrayDeallocs[numArrays];

    for (unsigned int i = 0; i < numArrays; i++) {
        DualBenchmarkResult r = RunArrayBenchmark(arrayCounts[i], 64);
        arrayAllocs[i] = r.allocPhase;
        arrayDeallocs[i] = r.deallocPhase;
    }

    PrintTable("ALLOCATION TIME - ARRAYS T[64] (ms)", arrayCounts, numArrays, arrayAllocs, "Arrays (N)");
    PrintTable("DEALLOCATION TIME - ARRAYS T[64] (ms)", arrayCounts, numArrays, arrayDeallocs, "Arrays (N)");

    return 0;
}
