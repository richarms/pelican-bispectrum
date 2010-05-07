#include "pelican/core/DirectStreamDataClient.h"
#include "pelican/utility/Config.h"
#include "pelican/utility/Factory.h"
#include "pelican/core/DataClientFactory.h"
#include "pelican/adapters/AbstractAdapter.h"
#include "pelican/data/DataBlob.h"
#include "pelican/data/VisibilityData.h"
#include "pelican/server/test/TestUdpChunker.h"

#include <iostream>
#include <QCoreApplication>
#include <QString>
#include <QList>
#include <QHash>

using namespace pelican;

void printVisibilities(VisibilityData* visData)
{
    unsigned nChan = visData->nChannels();
    unsigned nAnt = visData->nAntennas();
    unsigned nPol = visData->nPolarisations();
    for (unsigned p = 0; p < nPol; p++) {
        for (unsigned c = 0; c < nChan; c++) {
            complex_t* ptr = visData->ptr(c, p);
            for (unsigned j = 0; j < nAnt; j++) {
                for (unsigned i = 0; i < nAnt; i++) {
                    std::cout << "VisData: " << ptr[i + j * nAnt] << std::endl;
                }
            }
        }
    }
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    // Get command line arguments.
    if (argc != 4) {
        std::cerr << "Usage: directClient <port> <host> <chunk size, bytes>" << std::endl;
        return 1;
    }

    int port = atoi(argv[1]);
    QString host(argv[2]);
    int chunkSize = atoi(argv[3]);

    QString pipelineXml = ""
            "<buffers>"
            "   <VisibilityData>"
            "       <buffer maxSize=\"2000\" maxChunkSize=\"2000\"/>"
            "   </VisibilityData>"
            "</buffers>"
            "<chunkers>"
            "    <TestUdpChunker name=\"a\">"
            "       <connection host=\"" + host + "\" port=\"" + QString::number(port) + "\"/>"
            "       <data type=\"VisibilityData\" chunkSize=\"" + QString::number(chunkSize) + "\"/>"
            "    </TestUdpChunker>"
            "</chunkers>"
            "<clients>"
            "    <DirectStreamDataClient>"
            "        <data type=\"VisibilityData\" adapter=\"AdapterLofarStationVisibilities\"/>"
            "    </DirectStreamDataClient>"
            "</clients>"
            "<adapters>"
            "   <AdapterLofarStationVisibilities>"
            "       <antennas number=\"2\"/>"
            "       <channels start=\"0\" end=\"1\"/>"
            "       <polarisation value=\"both\"/>"
            "       <dataBytes number=\"8\"/>"
            "   </AdapterLofarStationVisibilities>"
            "</adapters>";
    Config config;
    config.setFromString(pipelineXml);

    try {
    // Create the adapter factory.
    Factory<AbstractAdapter> adapterFactory(&config, "pipeline", "adapters");

    // Create the data client factory.
    DataClientFactory clientFactory(&config, "pipeline", "clients", &adapterFactory);

    // Create the data blob factory.
    Factory<DataBlob> blobFactory;

    // Create a list of data requirements.
    QString dataType = "VisibilityData";
    DataRequirements req;
    QList<DataRequirements> requirements;
    req.addStreamData(dataType);
    requirements.append(req);

    // Create the client.
    DirectStreamDataClient* client = static_cast<DirectStreamDataClient*>(
            clientFactory.create("DirectStreamDataClient", requirements));
    client->addStreamChunker("TestUdpChunker", "a");
    client->start();

    // Set up the data hash.
    QHash<QString, DataBlob*> dataHash;
    dataHash.insert(dataType, blobFactory.create(dataType));

    for (int i = 0; i < 2000; i++) {
        std::cout << "---------------------------------- " << i << std::endl;
        // Get the data.
        QHash<QString, DataBlob*> validData = client->getData(dataHash);

        // Check the content of the data blob.
        VisibilityData* visData = (VisibilityData*)validData.value("VisibilityData");
        printVisibilities(visData);
    }
    } catch (QString error) {
        std::cerr << error.toStdString() << std::endl;
    }

    return 0;
}
