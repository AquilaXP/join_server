cmake_minimum_required(VERSION 3.2)

set(PROJECT_VERSION_PATCH $ENV{TRAVIS_BUILD_NUMBER})

project(join_server VERSION 0.0.${PROJECT_VERSION_PATCH})

find_package(Boost COMPONENTS system date_time regex REQUIRED)

configure_file(version.h.in version.h @ONLY)

set(SRC
    main.cpp
    handler_connect.cpp
    server.cpp
    bd.cpp
    )

set(HDR
    get_line_raw_data.h
    handler_connect.h
    server.h
    bd.h
    )

#program join_server
add_executable(join_server ${SRC} ${HDR})

target_include_directories(join_server PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${Boost_INCLUDE_DIR} )

set_target_properties( join_server PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    COMPILE_OPTIONS -Wall
)

target_link_libraries(join_server 
    ${Boost_LIBRARIES}    
)

if(UNIX)

set_target_properties( join_server PROPERTIES
    COMPILE_OPTIONS -pthread
)
target_link_libraries( join_server pthread )

endif(UNIX)

install(TARGETS join_server RUNTIME DESTINATION bin)


set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT aquila-2004@yandex.ru)

include(CPack)
