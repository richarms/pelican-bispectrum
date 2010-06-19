#===============================================================================
# subpackage.cmake
#
# Macros for handling sub-package creation
# 
# Sets: 
#   SUBPACKAGE_WORK_DIR: the work directory for sub-package module files.
#
#
# Defines the following public macros:
#   SUBPACKAGE: register a sub-package and set / get dependencies.
#   SUBPACKAGE_LIBRARY: Create the subpackage library.
#
#
# Defines the following utility macros:
#   SUBPACKAGE_GET_LIBS: Get the libraries required to link the specified 
#                        sub-package
#   SUBPACKAGE_GET_INCS: Get the includes required for the specified sub-package.
#
#
#===============================================================================


# Define and create the sub-package work directory.
set(SUBPACKAGE_WORK_DIR ${CMAKE_BINARY_DIR}/_subpackagesNew)
file(MAKE_DIRECTORY ${SUBPACKAGE_WORK_DIR})


#
# Macro to register and set / get sub-package dependencies.
#
macro(SUBPACKAGE_NEW name)

    # Set the current sub-package name.
    set(subpackage_current "${name}")
    
    # Set the name of the sub-package file.
    set(subpackage_file "${SUBPACKAGE_WORK_DIR}/${name}.cmake")


    # Write guard in the sub-package file to prevent double loading of this file/
    file(WRITE ${subpackage_file}
        "# Sub-package: '${name}'\n"
        "#  This file is auto-generated by the build system - do not edit.\n"
        "if(subpackage_${name}_LIBS)\n"
        "   return()\n"
        "endif(subpackage_${name}_LIBS)\n"
    )

    # Process packages that the sub-package depends on.
    # --------------------------------------------------------------------------
    set(subpackage_${name}_DEPS ${ARGN})

    # If any dependencies exist.    
    if(subpackage_${name}_DEPS)
        # Reverse the list of dependencies.
        list(REVERSE subpackage_${name}_DEPS)
        # Add the list of dependencies to the sub-package file.
        file(APPEND ${subpackage_file}
            "set(subpackage_${name}_DEPS ${subpackage_${name}_DEPS})\n"
        )
    endif(subpackage_${name}_DEPS)
    
    # Export all include directories defined before this macro
    if(COMMAND GET_PROPERTY)
        get_property(includes DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
    else(COMMAND GET_PROPERTY)
        # cmake 2.4 compatibility, just include everthing.
        set(includes ${CMAKE_INCLUDE_PATH})
    endif(COMMAND GET_PROPERTY)

    # Add the includes to the sub-package file.
    foreach(inc ${includes}
        file(APPEND ${subpackage_file} "include_directories(${inc})\n")
    endforeach(inc)

    # Add the package files of dependent sub-packages to the sub-package file.
    foreach(pack ${ARGN})
        file(APPEND ${subpackage_file} 
            "include(${SUBPACKAGE_WORK_DIR)/${pack}.cmake)\n")
    endforeach(pack)

    # Set the top level directory for includes.
    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    
    # Loop over the sub-package depenency list, loading their subpackage module
    # files to get the required librarys.
    foreach(dep ${subpackage_${name}_DEPS})
        # Load the sub-package file.
        include(${SUBPACKAGE_WORK_DIR}/${dep}.cmake)
        # Extract the required libraries setting ${SUBPACKAGE_LIBRARIES}.
        SUBPACKAGE_GET_LIBS(${dep}) # TODO refactor this macro        
    endforeach(dep)
    
    # Remove duplicates from sub-package libraries ?
    # TODO?

    # Create the install target for header files of the sub-package.
    #---------------------------------------------------------------------------
    file(GLOB public_headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h")
    install(FILES ${public_headers} DESTINATION ${INCLUDE_INSTALL_DIR}/${name})
        
    # TODO: instead of writing to the subpackge file thoughout.
    # add what is needed at the very end or just set variables and delay 
    # this to another macro.

endmacro(SUBPACKAGE_NEW)





