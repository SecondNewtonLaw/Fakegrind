//
// Created by Dottik on 26/7/2025.
//

#pragma once
#include "Service.hpp"
#include "ThreadManagerService.hpp"

#include <complex>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace Fakegrind::Services {
struct AllocationInformation;
class LoggerService final : public Service {
    std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> m_lpCoutSink = nullptr;
    std::shared_ptr<spdlog::sinks::basic_file_sink_mt> m_lpFileSink = nullptr;
    spdlog::logger *m_lpLogger = nullptr;

  public:
    ~LoggerService() override;

    void Initialize() override;

    void Uninitialize() override;

    void LogTrace(const std::string_view str) const;

    void LogDebug(const std::string_view str) const;

    void LogInfo(const std::string_view str) const;

    void LogWarning(const std::string_view str) const;

    void LogError(const std::string_view str) const;

    void LogCritical(const std::string_view str) const;

    template <typename... Args> void LogTrace(std::format_string<Args...> fmt, Args &&...args) const {
        this->LogTrace(std::format(fmt, std::forward<Args>(args)...));
    }

    template <typename... Args> void LogDebug(std::format_string<Args...> fmt, Args &&...args) const {
        this->LogDebug(std::format(fmt, std::forward<Args>(args)...));
    }

    template <typename... Args> void LogInfo(std::format_string<Args...> fmt, Args &&...args) const {
        this->LogInfo(std::format(fmt, std::forward<Args>(args)...));
    }

    template <typename... Args> void LogWarning(std::format_string<Args...> fmt, Args &&...args) const {
        this->LogWarning(std::format(fmt, std::forward<Args>(args)...));
    }

    template <typename... Args> void LogError(std::format_string<Args...> fmt, Args &&...args) const {
        this->LogError(std::format(fmt, std::forward<Args>(args)...));
    }

    template <typename... Args> void LogCritical(std::format_string<Args...> fmt, Args &&...args) const {
        this->LogCritical(std::format(fmt, std::forward<Args>(args)...));
    }

    void LogPossibleLeak(const AllocationInformation *lpAllocationInformation) const;
    void LogDoubleFree(
        const AllocationInformation *lpAllocationInformation, const ThreadInformation *lpThreadInformation, const cpptrace::stacktrace *lpCurrentStackTrace
    );
    void LogLostAllocation(
        const AllocationInformation *lpLostAllocation, const ThreadInformation *lpNewAllocThreadInfo, const cpptrace::stacktrace *lpNewAllocStackTrace,
        size_t newAllocationSize
    );
    void Flush() const;
};
} // namespace Fakegrind::Services
