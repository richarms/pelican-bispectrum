#include "Session.h"
#include "comms/AbstractProtocol.h"
#include "DataManager.h"
#include "LockedData.h"
#include "LockableStreamData.h"
#include "LockableData.h"
#include "comms/StreamData.h"
#include "comms/ServerRequest.h"
#include "comms/StreamDataRequest.h"
#include "comms/ServiceDataRequest.h"
#include <QTcpSocket>
#include <QString>
#include <QHash>
#include <QTime>

#include <iostream>

#include "utility/memCheck.h"

using std::cout;
using std::endl;

namespace pelican {

// class Session
Session::Session(int socketDescriptor, AbstractProtocol* proto, DataManager* data, QObject* parent)
    : QThread(parent), _dataManager(data)
{
    _protocol = proto;
    _socketDescriptor = socketDescriptor;
}

Session::~Session()
{
    if (isRunning()) wait();
}


/**
 * @details
 *
 */
void Session::run()
{
    QTcpSocket socket;
    if (!socket.setSocketDescriptor(_socketDescriptor)) {
        emit error(socket.error());
        return;
    }

    boost::shared_ptr<ServerRequest> req = _protocol->request(socket);

    processRequest(*req, socket);
    socket.disconnectFromHost();
    if (socket.state() != QAbstractSocket::UnconnectedState)
        socket.waitForDisconnected();
}

/**
 * @details
 * This routine processes a general ServerRequest, calling the appropriate
 * working and then passes this on to the protocol to be returned to the client
 */
void Session::processRequest(const ServerRequest& req, QIODevice& out)
{
    try {
        switch(req.type()) {
            case ServerRequest::Acknowledge:
                _protocol->send(out,"ACK");
                cout << "Sent acknowledgement" << endl;
                break;
            case ServerRequest::StreamData:
                {
                    QList<LockedData> dataList = processStreamDataRequest(static_cast<const StreamDataRequest&>(req) );
                    cout << "List size: " << dataList.size() << endl;
                    if( dataList.size() > 0 ) {
                        AbstractProtocol::StreamData_t data;
                        for( int i=0; i < dataList.size(); ++i ) {
                            LockableStreamData* lockedData = static_cast<LockableStreamData*>(dataList[i].object());
                            data.append(static_cast<StreamData*>(lockedData->streamData()));
                        }
                        _protocol->send( out, data );

                        // Mark as being served so can delete.
                        foreach (LockedData d, dataList) {
                            static_cast<LockableStreamData*>(d.object())->served() = true;
                        }
                        cout << "Sent stream data" << endl;
                    }
                    cout << "Finished stream data request" << endl;
                }
                break;
            case ServerRequest::ServiceData:
                {
                    QList<LockedData> d = processServiceDataRequest(static_cast<const ServiceDataRequest&>(req) );
                    if( d.size() > 0 ) {
                        AbstractProtocol::ServiceData_t data;
                        for( int i=0; i < d.size(); ++i ) {
                            LockableData* lockedData = static_cast<LockableData*>(d[i].object());
                            data.append(lockedData->data().get());
                        }
                        _protocol->send( out, data );
                        cout << "Sent service data" << endl;
                    }
                }
                break;
            default:
                _protocol->sendError( out, req.message());
        }
    }
    catch( const QString& e )
    {
        _protocol->sendError( out, e );
        cout << "Sent error: " << e.toStdString() << endl;
    }
}

/**
 * @details
 * Iterates over the list of data options (requirements) provided in the request
 * until a valid data object is returned.
 *
 * Returns the first data set available (streams and associated service data)
 * that match the requirements.
 *
 * An empty request will return immediately with an empty list.
 *
 * WARNING: This function will block until valid data matching the request can
 * made.
 *
 * The data will be returned as a locked container to ensure access by other
 * threads will be blocked. This is achived by a signal emitted when the
 * LockedData object goes out of scope.
 *
 * @param[in] req       StreamDataRequest object containing a data requirements
 *                      iterator.
 * @param[in] timeout   Timeout in milliseconds. (0 = dont timeout)
 *
 * @return A list of locked data containing streams data object and their
 *         associated service data for the first request that can be fully
 *         satisfied.
 */
QList<LockedData> Session::processStreamDataRequest(const StreamDataRequest& req,
                                                    const unsigned timeout)
{
    // Return an empty list if there are no data requirements in the request.
    QList<LockedData> dataList;
    if (req.isEmpty()) {
        return dataList;
    }

    // Start a timer to handle the timeout.
    QTime time;
    time.start();

    // Iterate until the data requirments can be satisfied.
    while (dataList.size() == 0) { // keep spinning until we either get data or time out
        if ((unsigned int)time.elapsed() > timeout && timeout > 0) {
            throw QString("Session::processStreamDataRequest():"
            " Request timed out after %1 ms.").arg(time.elapsed());
        }
        DataRequirementsIterator it = req.begin();
        while(it != req.end() && dataList.size() == 0) {
            dataList = _dataManager->getDataRequirements( *it );
            ++it;
        }
        usleep(1);
    }
    return dataList;
}


/**
 * @details
 * Returns all the data sets mentioned in the list (or throws if any one is missing).
 * The data will be returned as a locked container to ensure access
 * by other threads will be blocked.
 */
QList<LockedData> Session::processServiceDataRequest(const ServiceDataRequest& req )
{
    QList<LockedData> data;
    foreach( QString type, req.types() ) {
        LockedData d = _dataManager->getServiceData(type, req.version(type));
        if( ! d.isValid() )
        {
            throw QString("Data Requested does not exist :" + type + " " + req.version(type));
        }
        data.append(d);
    }
    return data;
}

} // namespace pelican
