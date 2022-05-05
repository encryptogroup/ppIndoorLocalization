cmake_minimum_required(VERSION 3.4.1)
if ("${ANDROID}")
else ()
    set(SERVER_SRCS
            main.cpp
            )
    foreach (filename ${SERVER_SRCS})
        get_filename_component(testname ${filename} NAME_WE)
        add_executable(${testname} ${filename})
        target_link_libraries(${testname} droidcrypto pthread)
    endforeach ()
endif ()