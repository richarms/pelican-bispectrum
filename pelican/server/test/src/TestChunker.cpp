#include "pelican/server/test/TestChunker.h"

#include <QCoreApplication>
#include <QBuffer>
#include <QTimer>

#include <iostream>

#include "pelican/utility/memCheck.h"

namespace pelican {

PELICAN_DECLARE_CHUNKER(TestChunker)

/**
 * @details
 * Constructs a new TestChunker.
 */
TestChunker::TestChunker(const QString& type, bool badSocket, size_t size,
        QString host, quint16 port, QObject* parent) :
            QObject(parent), AbstractChunker(type, host, port)
{
    _nextCount = 0;
    _badSocket = badSocket;
    _size = size;
    _timer = new QTimer;
    connect(_timer, SIGNAL(timeout()), this, SLOT(_start()));
}

/**
* @details
* Constructs a new TestChunker.
*/
TestChunker::TestChunker(const ConfigNode& config) :
    QObject(), AbstractChunker(config)
{
    _nextCount = 0;
    _badSocket = false;
    _size = config.getOption("data", "chunkSize", "512").toUInt();
    _timer = new QTimer;
    connect(_timer, SIGNAL(timeout()), this, SLOT(_start()));
}


/**
 * @details
 * Destroys the TestChunker.
 */
TestChunker::~TestChunker()
{
    delete _timer;
}

/**
 * @details
 * Constructs a new device.
 */
QIODevice* TestChunker::newDevice()
{
    if(_badSocket)
        return 0;

    _timer->start(1);
    QBuffer* buffer = new QBuffer;
    _device = buffer;
    return buffer;
}

/**
 * @details
 * Gets the next chunk of data from the UDP socket (if it exists).
 */
void TestChunker::next(QIODevice*)
{
    //std::cout << "TestChunker::next()" << std::endl;
    ++_nextCount;

    // Get writable data object.
    WritableData writableData = getDataStorage(_size);

    // If the writable data object is not valid, then return.
    if (!writableData.isValid())
        return;

    // ... or, block until writable data is valid.
    while (!writableData.isValid()) {
        writableData = getDataStorage(_size);
        usleep(1);
    }

    unsigned nDoubles = _size / sizeof(double);
    std::vector<double> array(nDoubles);
    for (unsigned i = 0; i < nDoubles; i++) {
//        array[i] = i;
        array[i] = _nextCount;
    }

    // Write some test data.
    writableData.write((void*)&array[0], _size, 0);
}

unsigned int TestChunker::nextCalled()
{
    unsigned int n = _nextCount;
    _nextCount = 0;
    return n;
}

/**
 * @details
 * Private slot to start and add to the data stream.
 */
void TestChunker::_start()
{
//    if (_nextCount < 10)
        next(_device);
//    else
//        _timer->stop();
}

} // namespace pelican