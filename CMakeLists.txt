cmake_minimum_required(VERSION 3.20)
project(simpleHTTP VERSION 1.0.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_EXAMPLES "Build examples" ON)
option(ENABLE_EMBEDDED "Enable embedded system optimizations" OFF)
option(ENABLE_HEADER_ONLY "Enable header-only mode" OFF)

set(SRC_DIR src)
set(INC_DIR include)
set(EXAMPLE_DIR examples)

add_library(simpleHTTP
        ${SRC_DIR}/simpleHTTP.cpp
        ${SRC_DIR}/socket.cpp
)

target_include_directories(simpleHTTP
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if(UNIX)
    target_link_libraries(simpleHTTP PRIVATE pthread)
endif()

if(ENABLE_HEADER_ONLY)
    target_compile_definitions(simpleHTTP PUBLIC SIMPLEHTTP_HEADER_ONLY)
endif()

if(BUILD_EXAMPLES)
    add_executable(simpleHTTP_example ${EXAMPLE_DIR}/base_example.cpp)
    target_link_libraries(simpleHTTP_example PRIVATE simpleHTTP)
    target_include_directories(simpleHTTP_example
            PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/${INC_DIR}
    )
endif()

include(GNUInstallDirs)

install(TARGETS simpleHTTP
        EXPORT simpleHTTP-targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES
        ${INC_DIR}/simpleHTTP.hpp
        ${INC_DIR}/socket.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/simpleHTTP
)

install(EXPORT simpleHTTP-targets
        FILE simpleHTTP-targets.cmake
        NAMESPACE simpleHTTP::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/simpleHTTP
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/simpleHTTP-config-version.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/simpleHTTP-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/simpleHTTP-config.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/simpleHTTP
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/simpleHTTP-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/simpleHTTP-config-version.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/simpleHTTP
)