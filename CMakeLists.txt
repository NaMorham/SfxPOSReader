cmake_minimum_required(VERSION 2.8)

project(SfxPOSReader)

set( CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++11 )

set(hdrs
)
source_group("Headers" FILES ${hdrs})

set(srcs
    src/main.cpp
)
source_group("Source" FILES ${srcs})

include_directories(
#    ${IDZip_INCLUDE_DIRS}
#    ${Netcode_INCLUDE_DIRS}
    ${BinIO_INCLUDE_DIRS}
    ${RoamesLogger_INCLUDE_DIRS}
    ${tinyformat_INCLUDE_DIRS}
    ${CommandLineUtil_INCLUDE_DIRS}
    ${RoamesLogger_INCLUDE_DIRS}
    ${MathsLibrary_INCLUDE_DIRS}
    ${Boost_IOSTREAMS_LIBRARY}
#    ${zlib_INCLUDE_DIRS}
)
add_executable(SfxPOSReader
    ${hdrs}
    ${srcs}    
)

target_link_libraries(SfxPOSReader
#    Netcode
#    IDZip
    ${Boost_IOSTREAMS_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    RoamesLogger
    CommandLineUtil
#    ${ZLIB_LIBRARIES}
#    ${OPENSSL_LIBRARIES}
)

# roames_install_targets(SfxPOSReader COMPONENT dataprep)
