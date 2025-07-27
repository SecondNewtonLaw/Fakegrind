//
// Created by Dottik on 26/7/2025.
//

#include "LoggerService.hpp"

#include "MemoryTrackerService.hpp"

LPTOP_LEVEL_EXCEPTION_FILTER oldFilter;

LONG WINAPI seh(_In_ struct _EXCEPTION_POINTERS *ExceptionInfo) {
    // We are possibly about to explode. We must flush without question.
    Fakegrind::ServiceManager::GetSingleton().GetService<Fakegrind::Services::LoggerService>()->LogCritical("!!! CRITICAL UNHANDLED EXCEPTION CAUGHT !!!");
    Fakegrind::ServiceManager::GetSingleton().GetService<Fakegrind::Services::LoggerService>()->LogCritical(
        "    Exception Code: {:#x}", ExceptionInfo->ExceptionRecord->ExceptionCode
    );
    Fakegrind::ServiceManager::GetSingleton().GetService<Fakegrind::Services::LoggerService>()->LogCritical(
        "    Exception Address: {}", ExceptionInfo->ExceptionRecord->ExceptionAddress
    );
    const auto &lpMemoryTrackerService = Fakegrind::ServiceManager::GetSingleton().GetService<Fakegrind::Services::MemoryTrackerService>();
    lpMemoryTrackerService->CheckAllocatedBlocks();
    Fakegrind::ServiceManager::GetSingleton().GetService<Fakegrind::Services::LoggerService>()->Flush();
    return oldFilter(ExceptionInfo);
}

Fakegrind::Services::LoggerService::~LoggerService() { delete m_lpLogger; }
void Fakegrind::Services::LoggerService::Initialize() {
    if (GetConsoleWindow() == nullptr) { // for some reason, they decide to return nullptr instead of INVALID_HANDLE_VALUE, WinAPI consistency!
        AllocConsole();

        FILE *dummy;
        freopen_s(&dummy, "CONIN$", "r", stdin);
        freopen_s(&dummy, "CONOUT$", "w", stderr);
        freopen_s(&dummy, "CONOUT$", "w", stdout);

        std::cout.clear();
        std::clog.clear();
        std::cerr.clear();
        std::cin.clear();
    }

    std::ios_base::sync_with_stdio(true);

    if (!std::filesystem::is_directory("Fakegrind_logs"))
        std::filesystem::create_directory("Fakegrind_logs");

    auto logName = std::format("log-{:%Y-%m-%d_%H.%M.%S}", std::chrono::system_clock::now());

    this->m_lpFileSink =
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(std::filesystem::absolute(std::format("Fakegrind_logs/{}.log", logName)).string(), true);
    this->m_lpFileSink->set_level(spdlog::level::trace);

    this->m_lpCoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    this->m_lpCoutSink->set_level(spdlog::level::trace);

    this->m_lpLogger = new spdlog::logger("Fakegrind.Module", {this->m_lpFileSink, this->m_lpCoutSink});
    this->m_lpLogger->flush_on(spdlog::level::info);
    this->m_lpLogger->set_level(spdlog::level::trace);

    oldFilter = SetUnhandledExceptionFilter(seh);
}

void Fakegrind::Services::LoggerService::LogTrace(const std::string_view str) const { this->m_lpLogger->trace(str); }

void Fakegrind::Services::LoggerService::LogDebug(const std::string_view str) const { this->m_lpLogger->debug(str); }

void Fakegrind::Services::LoggerService::LogInfo(const std::string_view str) const { this->m_lpLogger->info(str); }

void Fakegrind::Services::LoggerService::LogWarning(const std::string_view str) const { this->m_lpLogger->warn(str); }

void Fakegrind::Services::LoggerService::LogError(const std::string_view str) const { this->m_lpLogger->error(str); }

void Fakegrind::Services::LoggerService::LogCritical(const std::string_view str) const { this->m_lpLogger->critical(str); }

void Fakegrind::Services::LoggerService::LogPossibleLeak(const AllocationInformation *lpAllocationInformation) const {
    this->LogWarning(
        "Possible Memory Leak Detected!\nReason: Allocated block was not freed by the end of the program's execution.\nAllocated by thread T{} (tid: {}; "
        "hThread: 0x{:x}) HERE:\n{}"
        "\nChunk Information:\n"
        "\t- Chunk Size: {}",
        lpAllocationInformation->AllocationInfo->AllocatedBy.dwAssignedId, lpAllocationInformation->AllocationInfo->AllocatedBy.dwTid,
        reinterpret_cast<uintptr_t>(lpAllocationInformation->AllocationInfo->AllocatedBy.hThread),
        lpAllocationInformation->AllocationInfo->stackTrace.resolve().to_string(false), lpAllocationInformation->AllocationInfo->allocationSize
    );
}

void Fakegrind::Services::LoggerService::LogDoubleFree(
    const AllocationInformation *lpAllocationInformation, const ThreadInformation *lpThreadInformation, const cpptrace::stacktrace *lpCurrentStackTrace
) {
    this->LogCritical(
        "Heap corruption detected!\nReason: A previously freed memory block was freed again!\n"
        "Allocated by thread T{} (tid: {}; hThread: {:x}) HERE:\n"
        "{}\n"
        "Freed (originally) by thread T{} (tid: {}; hThread: {:x}) HERE:\n"
        "{}\n"
        "Freed again by thread T{} (tid: {}; hThread: {:x}) HERE:\n"
        "{}\n"
        "Chunk Information:\n"
        "\t- Chunk Size: {}",
        lpAllocationInformation->AllocationInfo->AllocatedBy.dwAssignedId, lpAllocationInformation->AllocationInfo->AllocatedBy.dwTid,
        reinterpret_cast<uintptr_t>(lpAllocationInformation->AllocationInfo->AllocatedBy.hThread),
        lpAllocationInformation->AllocationInfo->stackTrace.resolve().to_string(false), lpAllocationInformation->FreeData->FreedBy.dwAssignedId,
        lpAllocationInformation->FreeData->FreedBy.dwTid, reinterpret_cast<uintptr_t>(lpAllocationInformation->FreeData->FreedBy.hThread),
        lpAllocationInformation->FreeData->stackTrace.resolve().to_string(false), lpThreadInformation->dwAssignedId, lpThreadInformation->dwTid,
        reinterpret_cast<uintptr_t>(lpThreadInformation->hThread), lpCurrentStackTrace->to_string(false),
        lpAllocationInformation->AllocationInfo->allocationSize
    );
}

void Fakegrind::Services::LoggerService::LogLostAllocation(
    const AllocationInformation *lpLostAllocation, const ThreadInformation *lpNewAllocThreadInfo, const cpptrace::stacktrace *lpNewAllocStackTrace,
    size_t newAllocationSize
) {
    this->LogCritical(
        "Heap corruption detected!\n"
        "Reason: An active memory block was overwritten by a new allocation. (Heap reused an address that was never freed)\n"
        "Original (Lost) Allocation by thread T{} (tid: {}; hThread: {:x}) HERE:\n"
        "{}\n"
        "Overwritten by new allocation from thread T{} (tid: {}; hThread: {:x}) HERE:\n"
        "{}\n"
        "Chunk Information:\n"
        "\t- Original (Lost) Chunk Size: {}\n"
        "\t- New (Overwriting) Chunk Size: {}",

        lpLostAllocation->AllocationInfo->AllocatedBy.dwAssignedId, lpLostAllocation->AllocationInfo->AllocatedBy.dwTid,
        reinterpret_cast<uintptr_t>(lpLostAllocation->AllocationInfo->AllocatedBy.hThread),
        lpLostAllocation->AllocationInfo->stackTrace.resolve().to_string(false),

        lpNewAllocThreadInfo->dwAssignedId, lpNewAllocThreadInfo->dwTid, reinterpret_cast<uintptr_t>(lpNewAllocThreadInfo->hThread),
        lpNewAllocStackTrace->to_string(false),

        lpLostAllocation->AllocationInfo->allocationSize, newAllocationSize
    );
}
void Fakegrind::Services::LoggerService::Flush() const { this->m_lpLogger->flush(); }

void Fakegrind::Services::LoggerService::Uninitialize() {
    this->m_lpLogger->flush();
    this->m_lpFileSink->flush();
    this->m_lpCoutSink->flush();

    this->m_lpLogger->sinks().clear();
    this->m_lpFileSink = nullptr;
    this->m_lpCoutSink = nullptr;
}