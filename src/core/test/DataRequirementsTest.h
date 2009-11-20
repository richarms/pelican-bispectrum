#ifndef DATAREQUIREMENTSTEST_H
#define DATAREQUIREMENTSTEST_H

#include <cppunit/extensions/HelperMacros.h>

/**
 * @file DataRequirementsTest.h
 */

/**
 * @class DataRequirementsTest
 *  
 * @brief
 * Unit testing class for the data requirements object.
 * 
 * @details
 * Performs unit tests on the DataRequirements object
 * using the CppUnit framework.
 */

namespace pelican {

class DataRequirementsTest : public CppUnit::TestFixture
{
    public:
        CPPUNIT_TEST_SUITE( DataRequirementsTest );
        CPPUNIT_TEST( test_hash );
        CPPUNIT_TEST( test_operators );
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp();
        void tearDown();

        // Test Methods
        void test_hash();
        void test_operators();

    public:
        DataRequirementsTest(  );
        ~DataRequirementsTest();

    private:
};

} // namespace pelican
#endif // DATAREQUIREMENTSTEST_H 