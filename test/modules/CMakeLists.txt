file(GLOB_RECURSE ALL_TCL_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.tcl)
file(GLOB_RECURSE ALL_CPP_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

foreach(module ${ALL_CPP_FILES})
    get_filename_component(stem ${module} NAME_WE)
    add_library(${stem} SHARED ${module})
    target_link_libraries(${stem} PUBLIC Boost::system)
    target_include_directories(${stem} PUBLIC ../../include)
endforeach()



foreach(script ${ALL_TCL_FILES})
    get_filename_component(stem ${script} NAME_WE)
    add_test(NAME ${stem} COMMAND
            ${TCL_TCLSH} ${CMAKE_CURRENT_SOURCE_DIR}/${script}
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endforeach()
