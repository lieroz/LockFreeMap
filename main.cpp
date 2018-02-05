#include <ConcurrentMap_Leapfrog.h>
#include <QRunnable>
#include <QThreadPool>
#include <QDebug>

#define NUM_INTS 60
#define NUM_CYCLES 1000000

struct Foo {

};

// Create the map.
typedef ConcurrentMap_Leapfrog<int, Foo*> ConcurrentMap;
ConcurrentMap map;

class MyRunnable : public QRunnable {
protected:
    void run() override {
        for (int j = 0; j < NUM_CYCLES; j++) {
            const int index = j % (8 * NUM_INTS);
            map.insertOrFind(index + 1);
            map.assign(index + 1, new Foo);

            if (j % 10000 == 0) {
                qDebug() << j;
            }
        }
    }
};

int main()
{
    // Create QSBR context for the main thread.
    QSBR::Context context = DefaultQSBR.createContext();

    QThreadPool *pool = new QThreadPool;
    pool->setMaxThreadCount(10);

    for (int i = 0; i < NUM_INTS; ++i) {
        MyRunnable *task = new MyRunnable;
        task->setAutoDelete(true);
        pool->start(task);
    }

    // Update the QSBR context for this thread.
    // In a larger application, this should be called periodically, for each thread, at a moment
    // when the thread is quiescent – that is, not in the middle of any operation that uses a
    // Junction data structure.
    DefaultQSBR.update(context);

    // Destroy the QSBR context for the main thread.
    DefaultQSBR.destroyContext(context);
    delete pool;

    return 0;
}
