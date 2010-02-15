#include "adapters/AdapterLofarStationVisibilities.h"
#include "utility/ConfigNode.h"

#include "utility/memCheck.h"

namespace pelican {


/**
 * @details
 * Constructs a stream adapter for a LOFAR station visibility amplidude data set.
 * Extracts the default data blob dimensions from the default configuration
 * setting some defaults if configuration options cant be found.
 *
 * @param[in]   config  Pelican XML configuration node object.
 */
AdapterLofarStationVisibilities::AdapterLofarStationVisibilities(const ConfigNode& config)
: AbstractStreamAdapter(config)
{
    // Grab configuration for the adapter setting sensible defaults
    _nAnt = config.getOption("antennas", "number", "96").toUInt();
    _nChan = config.getOption("channels", "number", "512").toUInt();
    _nPol = config.getOption("polarisations", "number", "2").toUInt();
    _dataBytes = config.getOption("dataBytes", "number", "4").toUInt();
}


/**
 * @details
 */
AdapterLofarStationVisibilities::~AdapterLofarStationVisibilities()
{
}


/**
 * @details
 * Method for deserialise a LOFAR station visibility data set
 * contained in a QDataStream into a pelican visibility data blob.
 *
 * !......ONLY AN EXAMPLE AT THE MOMENT....!
 *
 * @param[in] in QDataStream containing a serialised version of a LOFAR
 *               visibility data set.
 */
void AdapterLofarStationVisibilities::deserialise(QDataStream& in)
{
    _setVisibilityData();

    complex_t* vis = _vis->ptr();

    // Assuming checkboard ant(XX/YY) per channel ?!
    for (unsigned c = 0; c < _nChan; c++) {
        for (unsigned j = 0; j < _nAnt; j++) {
            for (unsigned i = 0; i < _nAnt; i++) {
                for (unsigned p = 0; p < _nPol; c++) {
                    in >> vis[i].real();
                    in >> vis[i].imag();
                }
            }
        }
    }
}


/**
 * @details
 * Sets the size of the visibility data chunk which can be updated from
 * service data.
 * Checks the dimensions of the visibility data to be read matches the stream
 * chunk size.
 * Resizes the visibility data blob as required.
 */
void AdapterLofarStationVisibilities::_setVisibilityData()
{
    if (_chunkSize == 0) {
        throw QString("No data to read. Stream chunk size set to zero.");
    }

    if (_data == NULL) {
        throw QString("Cannot deserialise into an unallocated blob!");
    }

    // If any service data exists update the visibility dimensions from it...
    if (!_serviceData.empty()) _updateVisibilityDimensions();

    // Check the expected size due to data dimensions matches the stream chunk size
    unsigned dataSize = _nAnt * _nAnt * _nChan * _nPol * _dataBytes;
    if (dataSize != _chunkSize) {
        throw QString("Stream chunk size does not match data current dimensions");
    }

    // Resize the visibility data being read into to match the adapter dimensions
    _vis = static_cast<VisibilityData*>(_data);
    _vis->resize(_nAnt, _nChan, _nPol);
}


/**
 * @details
 * Updates the visibility data dimensions from service data passed down
 * from the adapter configuration.
 */
void AdapterLofarStationVisibilities::_updateVisibilityDimensions()
{
    // EXAMPLE ++ pseudo-code only +++
    // _nAnt = serviceData.nAnt;
    // _nChan = serviceData.nAnt;
    // _nPol = serviceData.nAnt; etc...
}


} // namespace pelican
