cmake_minimum_required(VERSION 3.30)
project(Fakegrind)

if (NOT WIN32)
    message(FATAL_ERROR "This project is only meant for compilation on Win32 environments")
endif ()

set(ENABLE_ASAN OFF)
set(ENABLE_UBSAN OFF) # GNU front-end only!

#[==[ Compiler Settings (Sanitizers ONLY!) ]==]

if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
    # gnu flags.
    if (ENABLE_ASAN AND ENABLE_UBSAN)
        add_compile_options(
                -fsanitize=address,undefined
        )
        add_link_options(
                -fsanitize=address,undefined
        )
    elseif (ENABLE_ASAN)
        add_compile_options(
                -fsanitize=address
        )
        add_link_options(
                -fsanitize=address
        )
    elseif (ENABLE_UBSAN)
        add_compile_options(
                -fsanitize=undefined
        )
        add_link_options(
                -fsanitize=undefined
        )
    endif ()
elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    # msvc flags.
    if (ENABLE_UBSAN)
        message(FATAL_ERROR "The Undefined Behaviour Sanitiser is not available on MSVC front-end compilers!")
    endif ()

    if (ENABLE_ASAN)
        add_compile_options(/fsanitize=address)
    endif ()

    add_compile_options(/Zc:preprocessor) # C std conformant preprocessor
endif ()

include(cmake/ccache.cmake)
include(cmake/CPM.cmake)
include(cmake/identify_compiler.cmake)

#[==[ External Dependencies ]==]

CPMAddPackage(
        NAME MinHook
        GITHUB_REPOSITORY TsudaKageyu/minhook
        VERSION 1.3.4
        OPTIONS "BUILD_SHARED_LIBS OFF"
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
        Fakegrind.Module/Services/MemoryTrackerService.cpp
        Fakegrind.Module/Services/MemoryTrackerService.hpp
        Fakegrind.Module/Services/IatHookService.cpp
        Fakegrind.Module/Services/IatHookService.hpp
        Fakegrind.Module/Services/CommunicationService.cpp
        Fakegrind.Module/Services/CommunicationService.hpp
        Fakegrind.Module/Services/ConfigurationService.cpp
        Fakegrind.Module/Services/ConfigurationService.hpp
)

#[==[ Compiler Settings ]==]

if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
    # gnu flags.
    set(CXX_COMPILE_OPTIONS -Wall -Wextra -Werror -Wmicrosoft -pedantic -g3 -static)
elseif (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    # msvc flags.
    set(CXX_COMPILE_OPTIONS /WX /Wall /permissive-)
endif ()

target_link_options(Fakegrind.Module PUBLIC -g3 -static)
target_link_options(Fakegrind.CLI PUBLIC -g3)

target_compile_options(Fakegrind.Common PRIVATE ${CXX_COMPILE_OPTIONS})
target_compile_options(Fakegrind.CLI PRIVATE ${CXX_COMPILE_OPTIONS})
target_compile_options(Fakegrind.Module PRIVATE ${CXX_COMPILE_OPTIONS})

target_precompile_headers(Fakegrind.Common PRIVATE Fakegrind.Common/pch.hpp)
target_precompile_headers(Fakegrind.Module PRIVATE Fakegrind.Common/pch.hpp)
target_precompile_headers(Fakegrind.CLI PRIVATE Fakegrind.Common/pch.hpp)

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