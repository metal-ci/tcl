enable_testing()

file(GLOB ALL_TEST_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(main_test STATIC main_test.cpp ${ALL_TEST_FILES})
target_link_libraries(main_test PUBLIC ${TCL_LIBRARY} ${TCL_STUB_LIBRARY} Boost::system)
add_subdirectory(modules)

foreach(test ${ALL_TEST_FILES})
    if (test STREQUAL main_test.cpp)
        continue()
    endif()
    get_filename_component(test_name ${test} NAME_WE)

    add_executable(${test_name} main_test.cpp ${test})
    target_link_libraries(${test_name} PUBLIC main_test)
    target_include_directories(${test_name} PUBLIC ../include ${TCL_INCLUDE_PATH})
    add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${test_name}>)
endforeach()

