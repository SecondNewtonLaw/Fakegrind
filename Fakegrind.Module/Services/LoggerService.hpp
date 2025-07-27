//
// Created by Dottik on 26/7/2025.
//


#pragma once
#include "Service.hpp"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace Fakegrind::Services {
    class LoggerService final : public Service {
        std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> cout_sink = nullptr;
        std::shared_ptr<spdlog::sinks::basic_file_sink_mt> file_sink = nullptr;
        spdlog::logger m_logger;

    public:
        void Initialize() override {
            this->m_logger = spdlog::logger("Fakegrind::Injected");
            if (!std::filesystem::is_directory("Fakegrind_logs")) std::filesystem::create_directory("Fakegrind_logs");

            auto logname = std::format("log-{:%Y-%m-%d_%H.%M.%S}", std::chrono::system_clock::now());

            this->file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
                    std::filesystem::absolute(std::format("Fakegrind_logs/{}.log", logname)).string(), true);
            this->file_sink->set_level(spdlog::level::trace);

            this->cout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            this->cout_sink->set_level(spdlog::level::trace);

            this->m_logger.set_level(spdlog::level::trace);
        }

        void LogTrace(const std::string_view str) { this->m_logger.trace(str); }
        void LogDebug(const std::string_view str) { this->m_logger.debug(str); }
        void LogInfo(const std::string_view str) { this->m_logger.info(str); }
        void LogWarning(const std::string_view str) { this->m_logger.warn(str); }
        void LogError(const std::string_view str) { this->m_logger.error(str); }
        void LogCritical(const std::string_view str) { this->m_logger.critical(str); }


        template<typename... Args>
        void LogTrace(std::format_string<Args...> fmt, Args &&...args) {
            LogTrace(std::format(fmt, std::forward<Args>(args)...));
        }

        template<typename... Args>
        void LogDebug(std::format_string<Args...> fmt, Args &&...args) {
            LogDebug(std::format(fmt, std::forward<Args>(args)...));
        }

        template<typename... Args>
        void LogInfo(std::format_string<Args...> fmt, Args &&...args) {
            LogInfo(std::format(fmt, std::forward<Args>(args)...));
        }

        template<typename... Args>
        void LogWarning(std::format_string<Args...> fmt, Args &&...args) {
            LogWarning(std::format(fmt, std::forward<Args>(args)...));
        }

        template<typename... Args>
        void LogError(std::format_string<Args...> fmt, Args &&...args) {
            LogError(std::format(fmt, std::forward<Args>(args)...));
        }

        template<typename... Args>
        void LogCritical(std::format_string<Args...> fmt, Args &&...args) {
            LogCritical(std::format(fmt, std::forward<Args>(args)...));
        }
        void Uninitialize() override {
            this->m_logger.flush();
            this->file_sink->flush();
            this->cout_sink->flush();

            this->m_logger.sinks().clear();
            this->file_sink = nullptr;
            this->cout_sink = nullptr;
        }
    };
} // namespace Fakegrind::Services
