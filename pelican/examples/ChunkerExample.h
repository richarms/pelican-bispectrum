#ifndef CHUNKEREXAMPLE_H
#define CHUNKEREXAMPLE_H

#include "pelican/server/AbstractChunker.h"
#include <QtCore/QByteArray>

/**
 * @file ChunkerExample.h
 */

using namespace pelican;

/**
 * @class ChunkerExample
 *  
 * @brief
 * A simple example to demonstrate how to write a data chunker.
 */
class ChunkerExample : public AbstractChunker
{
    public:
        ChunkerExample(const ConfigNode& config);
        virtual QIODevice* newDevice();
        virtual void next(QIODevice*);

    private:
        QByteArray buffer;
        int chunkSize;
};

#endif // CHUNKEREXAMPLE_H 
