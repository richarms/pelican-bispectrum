#
# pelican/bispectrum/CMakeLists.txt
#

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
SUBPACKAGE(bispectrum core server emulator)

# Build example codes (to check they work!)
set(bispectrum_src
    reference/src/AdapterExample.cpp
    reference/src/ChunkerExample.cpp
    reference/src/DirectDataClientExample.cpp
    reference/src/EmulatorExample.cpp
    reference/src/ModuleExample.cpp
    reference/src/PipelineExample.cpp
    bp/src/OutputStreamExample.cpp
    bp/src/SignalAmplifier.cpp
    bp/src/SignalChunker.cpp
    bp/src/SignalData.cpp
    bp/src/SignalDataAdapter.cpp
    bp/src/SignalEmulator.cpp
    bp/src/SignalProcessingPipeline.cpp
)
add_library(bispectrum ${bispectrum_src})
target_link_libraries(bispectrum
    ${SUBPACKAGE_LIBRARIES}
)

# Create the pipeline binary.
add_executable(signalPipeline
    bp/src/mainPipeline.cpp
)
target_link_libraries(signalPipeline
    ${SUBPACKAGE_LIBRARIES}
    bispectrum
)

# Create the server binary.
add_executable(signalServer
    bp/src/mainServer.cpp
)
target_link_libraries(signalServer
    ${SUBPACKAGE_LIBRARIES}
    bispectrum
)

# Create the emulator binary.
add_executable(signalEmulator
    bp/src/mainEmulator.cpp
)
target_link_libraries(signalEmulator
    ${SUBPACKAGE_LIBRARIES}
    bispectrum
)

# Copy the XML configuration files needed for the bp.
include(CopyFiles)
copy_files(${CMAKE_CURRENT_SOURCE_DIR}/bp/data/*.xml . exampleXML)
copy_files(${CMAKE_CURRENT_SOURCE_DIR}/bp/data/plotScript.gnu . plotScript.gnu)
