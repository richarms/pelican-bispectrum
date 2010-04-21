#ifndef ABSTRACTDATACLIENT_H
#define ABSTRACTDATACLIENT_H

#include "pelican/core/DataTypes.h"
#include "pelican/utility/Factory.h"
#include "pelican/utility/ConfigNode.h"
#include <QHash>
#include <QList>
#include <QSet>
#include <QString>
class QIODevice;

/**
 * @file AbstractDataClient.h
 */

namespace pelican {

/**
 * This macro is used to register the named client type with the factory.
 * It should be used within the global scope of the client's source file.
 *
 * @note
 * The macro expands to declare a dummy variable of the object's generator
 * type, which (when constructed), adds the type name to the creator's known
 * object types.
 *
 * The macro is placed within the global scope so that it is initialised as
 * soon as the program starts executing. It is placed within an anonymous
 * namespace so that the dummy creator variable is not accessible from outside
 * the file that instantiated it.
 */
#define PELICAN_DECLARE_CLIENT(type) namespace {Creator<type, AbstractDataClient> reg(#type);}

class AbstractAdapter;
class DataBlob;
class StreamData;
class Data;

/**
 * @class AbstractDataClient
 *
 * @brief
 * This is the abstract base class for the data client used by the pipeline
 * driver.
 *
 * @details
 * The data client fetches data from the data server and makes it available
 * to the pipelines via the pipeline driver. The data client is created by
 * the pipeline driver after determining the data requirements.
 *
 * Inherit this class and implement the getData() method to create a new data
 * client type.
 *
 * \section Configuration
 *
 * Under its root XML configuration node, each data client must have a list of
 * data types and data adapters, for example:
 *
 * \verbatim
      <data type="VisibilityData" adapter="AdapterStationVisibilities"/>
      <data type="AntennaPositions" adapter="AdapterStationAntennas"/>
 * \endverbatim
 *
 * This information is read by the abstract data client, and stored in the
 * hash accessible using the adapterNames() method.
 */
class AbstractDataClient
{
    private:
        /// The configuration node for the data client.
        ConfigNode _configNode;

        /// The DataTypes and requirements.
        DataTypes _dataReqs;

    protected:
        QSet<QString> _requireSet;

    protected: // methods
        /// Writes a message to the log.
        void log(const QString& msg);

        /// Adapts stream data.
        QHash<QString, DataBlob*> adaptStream(QIODevice& device,
                const StreamData* sd, QHash<QString, DataBlob*>& dataHash );

        /// Adapts service data.
        QHash<QString, DataBlob*> adaptService(QIODevice& device,
                const Data* d, QHash<QString, DataBlob*>& dataHash );

        /// Returns a pointer to the configuration node.
        const ConfigNode& configNode() {return _configNode;}

        /// Returns the adapter for service data of the required type.
        AbstractServiceAdapter* serviceAdapter(const QString& type) const {
            return _dataReqs.serviceAdapter(type);
        }

        /// Returns the adapter for stream data of the required type.
        AbstractStreamAdapter* streamAdapter(const QString& type) const {
            return _dataReqs.streamAdapter(type);
        }

    public:
        /// Data client constructor.
        AbstractDataClient(const ConfigNode& config);

        /// Data client destructor (virtual).
        virtual ~AbstractDataClient();

        /// Returns the list of data requirements for each pipeline.
        const QList<DataRequirements>& dataRequirements() {
            return _dataReqs.dataRequirements();
        }

        /// returns the type of data associated with the data name (stream/service/unknown)
        AbstractAdapter::AdapterType_t type(const QString& dataName) const {
            return _dataReqs.type(dataName);
        }

        /// Gets the requested data from the data server.
        /// This method gets tFills the given data hash, and returns another hash of valid data.
        virtual QHash<QString, DataBlob*> getData(QHash<QString, DataBlob*>&) = 0;

        /// Sets the data requirements.
        void setDataRequirements(const DataTypes& types);
};

} // namespace pelican

#endif // ABSTRACTDATACLIENT_H
