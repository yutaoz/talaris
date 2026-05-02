cmake_minimum_required(VERSION 3.30.4)

project(talaris-demo)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(talaris-demo)

target_sources(talaris-demo
  PRIVATE
    main.cpp
)

target_include_directories(talaris-demo
  PRIVATE
    src
)

target_link_libraries(talaris-demo
  PRIVATE
    Boost::headers
    OpenSSL::SSL
    OpenSSL::Crypto
)
