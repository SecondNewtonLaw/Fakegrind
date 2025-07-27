//
// Created by Dottik on 26/7/2025.
//

#pragma once

/*  This file is a precompiled header!
 *      - Files which do not change frequently should be included here to speed up compile times!
 *      - This file should NOT and must NOT include project files! If they change, the CMake project cache must be wiped
 * and re-generated.
 */

#include <algorithm>
#include <any>
#include <array>
#include <atomic>
#include <bitset>
#include <chrono>
#include <concepts>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <queue>
#include <ranges>
#include <set>
#include <span>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include <cmath>

#include <libassert/assert.hpp>

#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif

#define UNUSED_ARGUMENT(x) ((void)(x))

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>
#include <dbghelp.h>
#include <intrin.h>

#else
#error 'This platform is not supported for building; if you want to ignore the cries of help of this, then go ahead and remove me :<'
#endif
