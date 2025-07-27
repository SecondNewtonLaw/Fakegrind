cmake_minimum_required(VERSION 3.30)
project(Fakegrind)

include(cmake/ccache.cmake)
include(cmake/CPM.cmake)
include(cmake/identify_compiler.cmake)

#[==[ External Dependencies ]==]

CPMAddPackage(
        NAME MinHook
        GITHUB_REPOSITORY TsudaKageyu/minhook
        VERSION 1.3.4
)
CPMAddPackage(
        NAME libassert
        GITHUB_REPOSITORY jeremy-rifkin/libassert
        GIT_TAG v2.2.0
)

CPMAddPackage(
        NAME argparse
        GITHUB_REPOSITORY p-ranav/argparse
        GIT_TAG v3.2
)

CPMAddPackage(
        NAME spdlog
        GITHUB_REPOSITORY gabime/spdlog
        GIT_TAG v1.15.3
        OPTIONS "SPDLOG_USE_STD_FORMAT ON"
)


set(CMAKE_CXX_STANDARD 23)

add_library(Fakegrind.Common STATIC
        Fakegrind.Common/Communication/SharedMemoryManager.cpp
        Fakegrind.Common/Communication/SharedMemoryManager.hpp
        Fakegrind.Common/Event.hpp

)

add_executable(Fakegrind.CLI Fakegrind.CLI/main.cpp)

add_library(Fakegrind.Module SHARED
        Fakegrind.Module/Entry.cpp
        Fakegrind.Module/Entry.cpp
        Fakegrind.Module/ServiceManager.cpp
        Fakegrind.Module/ServiceManager.hpp

        Fakegrind.Module/Services/Service.hpp
        Fakegrind.Module/Services/LoggerService.cpp
        Fakegrind.Module/Services/LoggerService.hpp
        Fakegrind.Module/Services/ThreadManagerService.cpp
        Fakegrind.Module/Services/ThreadManagerService.hpp
)

target_precompile_headers(Fakegrind.Common PUBLIC Fakegrind.Common/pch.hpp)
target_precompile_headers(Fakegrind.Module PUBLIC Fakegrind.Common/pch.hpp)
target_precompile_headers(Fakegrind.CLI PUBLIC Fakegrind.Common/pch.hpp)

target_link_libraries(Fakegrind.Common PUBLIC
        libassert::assert
        spdlog
)

target_link_libraries(Fakegrind.Module PUBLIC
        minhook
        Fakegrind.Common
)

target_link_libraries(Fakegrind.CLI PUBLIC
        argparse
        Fakegrind.Common
)