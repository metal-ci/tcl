cmake_minimum_required(VERSION 3.20)

set(TCLBIND_VERSION 1)
if(BOOST_SUPERPROJECT_VERSION)
    set(TCLBIND_VERSION ${BOOST_SUPERPROJECT_VERSION})
endif()

project(boost_tcl VERSION "${TCLBIND_VERSION}" LANGUAGES CXX)

set(TCLBIND_IS_ROOT OFF)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(TCLBIND_IS_ROOT ON)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../../boost.css)
    set(TCLBIND_SHOULD_BE_INLINE ON)
else()
    set(TCLBIND_SHOULD_BE_INLINE OFF)
endif()

option(TCLBIND_BUILD_INLINE "Configure as if part of the boost source tree" ${TCLBIND_SHOULD_BE_INLINE})


if(TCLBIND_IS_ROOT)
    include(CTest)
endif()
if(NOT BOOST_SUPERPROJECT_VERSION)
    option(TCLBIND_INSTALL "Install tclbind files" ON)
    option(TCLBIND_BUILD_TESTS "Build tclbind tests" ${BUILD_TESTING})
    option(TCLBIND_BUILD_EXAMPLES "Build tclbind examples" ${TCLBIND_IS_ROOT})
else()
    set(TCLBIND_BUILD_TESTS ${BUILD_TESTING})
endif()


if(TCLBIND_IS_ROOT AND TCLBIND_BUILD_INLINE)
    #
    # Building inside Boost tree, but as a separate project e.g. on Travis or
    # other CI, or when producing Visual Studio Solution and Projects.

    set(BOOST_INCLUDE_LIBRARIES TCL)
    set(BOOST_EXCLUDE_LIBRARIES TCL)

    set(CMAKE_FOLDER _deps)
    add_subdirectory(../.. _deps/boost EXCLUDE_FROM_ALL)
    unset(CMAKE_FOLDER)
endif()

if (NOT TCLBIND_BUILD_INLINE)
    find_package(Boost REQUIRED system json)
    include_directories(include)
endif()

find_package(TCL)
find_package(TclStub)

add_library(boost_tcl INTERFACE)
target_include_directories(boost_tcl INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
add_library(Boost::tcl ALIAS boost_tcl)

if(TCLBIND_INSTALL AND NOT BOOST_SUPERPROJECT_VERSION)
    install(TARGETS boost_tcl
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            )
endif()


if(TCLBIND_BUILD_TESTS)
    add_subdirectory(test)
endif()


if(TCLBIND_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()