#include "PelicanServerTest.h"

#include "PelicanServer.h"
#include "PelicanTestClient.h"
#include "TestProtocol.h"
#include "DataManager.h"
#include "TestChunker.h"
#include "data/DataRequirements.h"
#include <QCoreApplication>
#include <QThread>
#include <iostream>
#include "utility/Config.h"

#include "utility/memCheck.h"

namespace pelican {

CPPUNIT_TEST_SUITE_REGISTRATION( PelicanServerTest );

// class PelicanServerTest
PelicanServerTest::PelicanServerTest()
    : CppUnit::TestFixture()
{
//    int ac = 0;
//    _app = new QCoreApplication(ac, NULL);
}

PelicanServerTest::~PelicanServerTest()
{
//    delete _app;
}

void PelicanServerTest::setUp()
{
}

void PelicanServerTest::tearDown()
{
}

void PelicanServerTest::test_singleProtocolAcknowledge()
{
    // Use Case:
    // Attach to Server with a single protocol.
    // Expect response to the correct port
    try {
        Config config;
        PelicanServer server(&config);
        quint16 port = 2000;
        server.addProtocol(new TestProtocol(""), port);
        server.start();
        while (!server.isReady()) {}
        PelicanTestClient client(port);
        client.processAcknowledgement();
    }
    catch (QString e)
    {
        CPPUNIT_FAIL("Unexpected exception: " +  e.toStdString());
    }
}

void PelicanServerTest::test_singleProtocolStream()
{
    // Use Case:
    // Attach to Server with a single protocol.
    // Expect response to the correct port
    try {
        Config config;
        PelicanServer server(&config);
        quint16 port = 2000;
        server.addProtocol(new TestProtocol("", ServerRequest::StreamData), port);
        server.addStreamChunker(new TestChunker("TestStream"));
        server.start();
        while (!server.isReady()) {}
        PelicanTestClient client(port);
        client.processStreamData();
    }
    catch (QString e)
    {
        CPPUNIT_FAIL("Unexpected exception: " +  e.toStdString());
    }
}

void PelicanServerTest::test_multiProtocol()
{
    QString id1("1");
    QString id2("2");

    // Use Case:
    // Assign multiple protocols to the same port
    // Expect an exception
    {
        Config config;
        PelicanServer server(&config);
        quint16 port = 20000;
        CPPUNIT_ASSERT_NO_THROW(server.addProtocol(new TestProtocol(id1), port));
        CPPUNIT_ASSERT_THROW(server.addProtocol(new TestProtocol(id2), port), QString);
    }

    // Use Case:
    // Assign multiple protocols to different ports
    // Expect no exceptions and client calls to be routed through
    // to the appropriate protocol object
    {
        try {
            Config config;
            PelicanServer server(&config);
            quint16 porta = 20000;
            quint16 portb = 20001;
            server.addProtocol(new TestProtocol(id1), porta);
            server.addProtocol(new TestProtocol(id2), portb);
            server.start();
            while (!server.isReady()) {}
            PelicanTestClient client1(porta);
            PelicanTestClient client2(portb);
            client1.processAcknowledgement();
            client2.processAcknowledgement();
        }
        catch (QString e) {
            CPPUNIT_FAIL("Unexpected exception: " +  e.toStdString());
        }

        // TODO Understand what the ID means!
//        CPPUNIT_ASSERT_EQUAL(id1.toStdString(), client1.processAcknowledgement().toStdString());
//        CPPUNIT_ASSERT_EQUAL(id2.toStdString(), client2.processAcknowledgement().toStdString());
    }
}

} // namespace pelican
