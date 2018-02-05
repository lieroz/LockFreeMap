#include <ConcurrentMap_Leapfrog.h>
#include <thread>
#include <vector>
#include <iostream>

#define NUM_INTS 60
#define NUM_CYCLES 1000000

// Create the map.
typedef junction::ConcurrentMap_Leapfrog<int, int> ConcurrentMap;
ConcurrentMap map;

void testMap(int m_arg)
{
    for (int j = 0; j < NUM_CYCLES; j++) {
        const int index = j % (8 * NUM_INTS);
        map.insertOrFind(index + 1);
        map.assign(index + 1, m_arg + 2);

        if (j % 10000 == 0) {
            std::cout << m_arg << " " << j << std::endl;
        }
    }
}

int main()
{
    // Create QSBR context for the main thread.
    junction::QSBR::Context context = junction::DefaultQSBR.createContext();

    std::vector<std::thread> threads(NUM_INTS);

    for (int i = 0; i < NUM_INTS; ++i) {
        threads[i] = std::thread(testMap, i + 2);
    }

    for (int i = 0; i < NUM_INTS; ++i) {
        threads[i].join();
    }
    // Update the QSBR context for this thread.
    // In a larger application, this should be called periodically, for each thread, at a moment
    // when the thread is quiescent – that is, not in the middle of any operation that uses a
    // Junction data structure.
    junction::DefaultQSBR.update(context);

    // Destroy the QSBR context for the main thread.
    junction::DefaultQSBR.destroyContext(context);
    return 0;
}
