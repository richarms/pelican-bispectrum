#include "pelican/comms/DataBlobResponse.h"

namespace pelican {

/**
 * @details DataBlobResponse
 */
DataBlobResponse::DataBlobResponse(const QString& blobType,
        const QString& streamName,
        quint64 dataSize, QSysInfo::Endian endianness )
    : ServerResponse( ServerResponse::Blob ),
      _type(blobType),
      _name(streamName),
      _dataSize(dataSize),
      _endianness(endianness)
{
}

/**
 * @details
 */
DataBlobResponse::~DataBlobResponse()
{
}

void DataBlobResponse::serialise( QDataStream& stream ) {
    stream <<  _type << _name;
    stream << _endianness;
    //stream << _dataSize;
}

void DataBlobResponse::deserialise( QDataStream& stream ) {
    stream >> _type >> _name;
    //stream >> _endianness >> _dataSize;
}

size_t DataBlobResponse::serialisedSize() {
    return sizeof(_type) + sizeof(_name) + sizeof(_dataSize) + sizeof(_endianness);
}
} // namespace pelican
