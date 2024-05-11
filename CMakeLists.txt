cmake_minimum_required(VERSION 3.20)

set(METAL_TCL_VERSION 1)

project(metal_tcl VERSION "${METAL_TCL_VERSION}" LANGUAGES CXX)


option(METAL_TCL_USE_JIM "Use JimTcl instead of Tcl" OFF)


find_package(Boost REQUIRED system json)
include_directories(include)

add_library(metal_tcl INTERFACE)
target_include_directories(metal_tcl INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
add_library(metal::tcl ALIAS metal_tcl)

find_package(TCL)
find_package(TclStub)


if(METAL_TCL_INSTALL AND NOT BOOST_SUPERPROJECT_VERSION)
    install(TARGETS metal_tcl
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            )
endif()


if(METAL_TCL_BUILD_TESTS)
    add_subdirectory(test)
endif()


if(METAL_TCL_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()