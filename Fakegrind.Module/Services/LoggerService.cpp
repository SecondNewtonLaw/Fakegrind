//
// Created by Dottik on 26/7/2025.
//

#include "LoggerService.hpp"

Fakegrind::Services::LoggerService::~LoggerService() { delete m_lpLogger; }
void Fakegrind::Services::LoggerService::Initialize() {
    if (GetConsoleWindow() == nullptr) // for some reason, they decide to return nullptr instead of INVALID_HANDLE_VALUE, WinAPI consistency!
        AllocConsole();

    FILE *dummy;
    freopen_s(&dummy, "CONIN$", "r", stdin);
    freopen_s(&dummy, "CONOUT$", "w", stderr);
    freopen_s(&dummy, "CONOUT$", "w", stdout);

    std::cout.clear();
    std::clog.clear();
    std::cerr.clear();
    std::cin.clear();
    std::ios_base::sync_with_stdio(true);

    this->m_lpLogger = new spdlog::logger("Fakegrind::Injected");

    if (!std::filesystem::is_directory("Fakegrind_logs"))
        std::filesystem::create_directory("Fakegrind_logs");

    auto logName = std::format("log-{:%Y-%m-%d_%H.%M.%S}", std::chrono::system_clock::now());

    this->m_lpFileSink =
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(std::filesystem::absolute(std::format("Fakegrind_logs/{}.log", logName)).string(), true);
    this->m_lpFileSink->set_level(spdlog::level::trace);

    this->m_lpCoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    this->m_lpCoutSink->set_level(spdlog::level::trace);

    this->m_lpLogger->set_level(spdlog::level::trace);
}

void Fakegrind::Services::LoggerService::LogTrace(const std::string_view str) const { this->m_lpLogger->trace(str); }

void Fakegrind::Services::LoggerService::LogDebug(const std::string_view str) const { this->m_lpLogger->debug(str); }

void Fakegrind::Services::LoggerService::LogInfo(const std::string_view str) const { this->m_lpLogger->info(str); }

void Fakegrind::Services::LoggerService::LogWarning(const std::string_view str) const { this->m_lpLogger->warn(str); }

void Fakegrind::Services::LoggerService::LogError(const std::string_view str) const { this->m_lpLogger->error(str); }

void Fakegrind::Services::LoggerService::LogCritical(const std::string_view str) const { this->m_lpLogger->critical(str); }

void Fakegrind::Services::LoggerService::Uninitialize() {
    this->m_lpLogger->flush();
    this->m_lpFileSink->flush();
    this->m_lpCoutSink->flush();

    this->m_lpLogger->sinks().clear();
    this->m_lpFileSink = nullptr;
    this->m_lpCoutSink = nullptr;
}