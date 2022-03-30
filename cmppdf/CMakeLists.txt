cmake_minimum_required(VERSION 3.0.0)
project(cmppdf VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(cmppdf main.cpp file_image.cpp pdf.cpp pdf_xref.cpp pdf_object.cpp pdf_array.cpp pdf_dictionary.cpp pdf_stream.cpp)
target_compile_features(cmppdf PUBLIC cxx_std_17)
if(MSVC)
	target_compile_options(cmppdf PUBLIC /source-charset:utf-8)
endif(MSVC)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
