SUBPACKAGE(bispectrumTest bispectrum)
set(
   bispectrumTest_src
src/bispectrumTest.cpp
)
add_executable(bispectrumTest ${bispectrumTest_src} )
target_link_libraries(bispectrumTest ${SUBPACKAGE_LIBRARIES} ${CPPUNIT_LIBRARIES})
add_test(bispectrumTest bispectrumTest )
