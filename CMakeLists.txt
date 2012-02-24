cmake_minimum_required (VERSION 2.8)
project (LWReader)
set (ADis_VERSION_MAJOR 1)
set (ADis_VERSION_MINOR 0)

set (LWReader_SOURCES
 LwoObjectData.cpp LwoReader.cpp MemFile.cpp main.cpp)

set (LWReader_HEADERS
 LwoObjectData.h LwoReader.h LwoTags.h MemFile.h)

add_executable(LWReader ${LWReader_SOURCES})

target_link_libraries(LWReader)


