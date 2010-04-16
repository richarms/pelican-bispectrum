#ifndef SESSION_H
#define SESSION_H

#include <QThread>
#include <QList>
#include <QTcpSocket>

/**
 * @file Session.h
 */

namespace pelican {

class ServerRequest;
class ServiceDataRequest;
class StreamDataRequest;
class LockedData;
class AbstractProtocol;
class DataManager;

/**
 * @class Session
 *  
 * @brief
 *    Class to process a single server request
 *
 * @details
 *
 */

class Session : public QThread
{
    Q_OBJECT

    public:
        /// Constructs a server session.
        Session(int socketDescriptor, AbstractProtocol* proto, DataManager* data, QObject* parent = 0);

        /// Destroys the server session.
        ~Session();

    public:
        /// Runs the session thread processing a request.
        /// (implements run method of QThread).
        void run();

        /// Process a request to the server sending the appropriate response.
        void processRequest(const ServerRequest&, QIODevice&, const unsigned timeout = 0 );

    protected:
        /// Iterates over the requirements in the stream data request (/p req) and
        /// returns the first valid stream data with valid associated service data.
        QList<LockedData> processStreamDataRequest(const StreamDataRequest& req,
                                                   const unsigned timeout = 0);

        QList<LockedData> processServiceDataRequest(const ServiceDataRequest& req);

    signals:
        void error(QTcpSocket::SocketError socketError);

    private:
        int _socketDescriptor;
        DataManager* _dataManager;
        AbstractProtocol* _protocol;
        static int _n;

    friend class SessionTest; // unit test
};

} // namespace pelican
#endif // SESSION_H 