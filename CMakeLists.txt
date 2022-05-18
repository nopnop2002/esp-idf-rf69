# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)


include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(version)


#idf_build_set_property(COMPILE_OPTIONS "-DDISABLE_CJSON" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-DDISABLE_XML" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-DDISABLE_FREQ" APPEND)
idf_build_set_property(COMPILE_OPTIONS "-DDISABLE_MALLOC" APPEND)
