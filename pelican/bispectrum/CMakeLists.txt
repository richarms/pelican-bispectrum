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
    src/OutputStreamExample.cpp
    src/SignalAmplifier.cpp
    src/SignalChunker.cpp
    src/SignalData.cpp
    src/SignalDataAdapter.cpp
    src/SignalEmulator.cpp
    src/SignalProcessingPipeline.cpp
)
add_library(bispectrum ${bispectrum_src})
target_link_libraries(bispectrum
    ${SUBPACKAGE_LIBRARIES}
)

# Create the pipeline binary.
add_executable(signalPipeline
    src/mainPipeline.cpp
)
target_link_libraries(signalPipeline
    ${SUBPACKAGE_LIBRARIES}
    bispectrum
)

# Create the server binary.
add_executable(signalServer
    src/mainServer.cpp
)
target_link_libraries(signalServer
    ${SUBPACKAGE_LIBRARIES}
    bispectrum
)

# Create the emulator binary.
add_executable(signalEmulator
    src/mainEmulator.cpp
)
target_link_libraries(signalEmulator
    ${SUBPACKAGE_LIBRARIES}
    bispectrum
)

# Copy the XML configuration files needed for the bp.
include(CopyFiles)
copy_files(${CMAKE_CURRENT_SOURCE_DIR}/data/*.xml . exampleXML)
copy_files(${CMAKE_CURRENT_SOURCE_DIR}/data/plotScript.gnu . plotScript.gnu)
