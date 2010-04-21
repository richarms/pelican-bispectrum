#include "pelican/server/PelicanServer.h"

#include "pelican/comms/AbstractProtocol.h"
#include "pelican/server/AbstractChunker.h"
#include "pelican/server/ChunkerManager.h"
#include "pelican/server/DataManager.h"
#include "pelican/server/DataReceiver.h"
#include "pelican/comms/PelicanProtocol.h"
#include "pelican/server/PelicanPortServer.h"
#include "pelican/utility/Config.h"

#include <boost/shared_ptr.hpp>
#include <QtGlobal>
#include <QTcpSocket>

#include <iostream>

#include "pelican/utility/memCheck.h"

namespace pelican {

/**
 * @details
 * Creates a new Pelican server, which in turn creates a chunker manager.
 */
PelicanServer::PelicanServer(const Config* config, QObject* parent)
    : QThread(parent)
{
    _config = config;
    _ready = false;

    // Create the chunker manager.
    _chunkerManager = new ChunkerManager(config);
}

/**
 * @details
 * Destroys the Pelican server.
 */
PelicanServer::~PelicanServer()
{
    // Wait for the thread to finish.
    if (isRunning()) while (!isFinished()) quit();
    wait();

    // Delete the protocols.
    foreach (AbstractProtocol* protocol, _protocolPortMap)
        delete protocol;

    // Delete the chunker manager.
    delete _chunkerManager;
}

/**
 * @details
 * Adds the given \p protocol to the given \p port.
 * The class takes ownership of \p protocol.
 *
 * @param proto A pointer to the allocated protocol.
 * @param port  The incoming port number to use.
 */
void PelicanServer::addProtocol(AbstractProtocol* protocol, quint16 port)
{
    if ( _protocolPortMap.contains(port) ) {
        delete protocol;
        throw QString("Cannot map multiple protocols to port %1").arg(port);
    }
    _protocolPortMap[port] = protocol;
}

/**
 * @details
 * Adds a stream chunker of the given \p type and \p name.
 *
 * @param type The chunker type (class name).
 * @param name The optional chunker configuration name.
 */
void PelicanServer::addStreamChunker(QString type, QString name)
{
    _chunkerManager->addStreamChunker(type, name);
}

/**
 * @details
 * Adds a service chunker of the given \p type and \p name.
 *
 * @param type The chunker type (class name).
 * @param name The optional chunker configuration name.
 */
void PelicanServer::addServiceChunker(QString type, QString name)
{
    _chunkerManager->addServiceChunker(type, name);
}

/**
 * @details
 * Runs the server thread starting its event loop.
 *
 * Sets up the data manager which handles stream and service data buffers
 * which are set up on request of the chunkers.
 */
void PelicanServer::run()
{
    QVector<boost::shared_ptr<PelicanPortServer> > servers;

    // Set up the data manager.
    DataManager dataManager(_config);
    _chunkerManager->init(dataManager);

    // Set up listening servers.
    QList<quint16> ports = _protocolPortMap.keys();
    for (int i = 0; i < ports.size(); ++i) {
        boost::shared_ptr<PelicanPortServer> server(
            new PelicanPortServer(_protocolPortMap[ports[i]], &dataManager) );
        servers.append(server);
        if ( !server->listen(QHostAddress::Any, ports[i] )) {
            throw QString(QString("Unable to start PelicanServer on port='") +
                            QString().setNum(ports[i])+ QString("'"));
        }
    }
    _mutex.lock();
    _ready = true;
    _mutex.unlock();
    exec();
}

} // namespace pelican