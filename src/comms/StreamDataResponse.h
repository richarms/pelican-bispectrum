#ifndef STREAMDATARESPONSE_H
#define STREAMDATARESPONSE_H

#include "ServerResponse.h"
#include "StreamData.h"

/**
 * @file StreamDataResponse.h
 */

namespace pelican {

/**
 * @class StreamDataResponse
 *  
 * @brief
 * Class to represent a set of stream data returned from the server.
 *
 * @details
 * 
 */
class StreamDataResponse : public ServerResponse
{
    private:
        pelican::StreamData* _data;

    public:
        /// Constructs a StreamDataResponse object.
        StreamDataResponse();

        /// Destroys the StreamDataResponse object.
        ~StreamDataResponse();

        /// Sets the pointer to the StreamData object.
        void setStreamData(pelican::StreamData* data) {_data = data;}

        /// Returns the pointer to the StreamData object.
        pelican::StreamData* streamData() {return _data;}
};

} // namespace pelican

#endif // STREAMDATARESPONSE_H 
