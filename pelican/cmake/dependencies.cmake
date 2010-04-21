#
# dependencies.cmake: Sets depencies for PELICAN.
#
# Required dependencies:
#   Qt4
#   CppUnit
#   CFitsio
#   Boost program options
#   cblas (either ATLAS or MKL)
#   lapack (either ATLAS or MKL)
#
# Optional dependencies:
#
# Sets the following variables:
#   PROJECT_LIBARRIES: Use for building libpelican.so (TO BE REMOVED)
#

# === Find Dependencies.
find_package(Boost COMPONENTS program_options REQUIRED)
find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork QtXml REQUIRED)
find_package(CppUnit REQUIRED)
find_package(CFitsio REQUIRED)

# === Find cblas and lapack from MKL if availiable, otherwise elsewhere.
if(NOT DEFINED pelican_mkl)
    set(pelican_mkl true)
endif(NOT DEFINED pelican_mkl)

if(pelican_mkl)
    find_package(MKL QUIET)
endif(pelican_mkl)

if (MKL_FOUND)
    add_definitions(-DUSING_MKL)
    set(pelican_math_libs ${MKL_LIBRARIES})
    set(pelican_mkl true)
else (MKL_FOUND)
    find_package(CBLAS REQUIRED)
    find_package(LAPACK REQUIRED)
    set(pelican_math_libs ${LAPACK_LIBRARIES} ${CBLAS_LIBRARIES})
endif (MKL_FOUND)

# === List of all libraries for single library install.
set(PROJECT_LIBRARIES
    pelican
    ${QT_QTCORE_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
    ${pelican_math_libs}
    ${CFITSIO_LIBRARIES}
)