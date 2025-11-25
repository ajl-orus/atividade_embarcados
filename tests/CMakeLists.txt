# CMakeLists.txt principal
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(radar_eletronico)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})

# CMakeLists.txt para testes
if(CONFIG_ZTEST)
    enable_testing()
    find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
    project(radar_tests)
    
    FILE(GLOB test_sources src/*.c tests/*.c)
    target_sources(testbinary PRIVATE ${test_sources})
    
    target_compile_definitions(testbinary PRIVATE TEST_MODE=1)
endif()