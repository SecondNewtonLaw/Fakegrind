//
// Created by Dottik on 26/7/2025.
//

#include "ThreadManagerService.hpp"

#include <TlHelp32.h>

#include "../ServiceManager.hpp"
#include "LoggerService.hpp"
void Fakegrind::Services::ThreadManagerService::Initialize() {
    const auto &lpLogger = ServiceManager::GetSingleton().GetService<LoggerService>();
    auto hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());

    if (hSnapshot == INVALID_HANDLE_VALUE) {
        lpLogger->LogError(
            "Failed to initialize ThreadManagerService: Could not obtain all the threads of the process we "
            "attached to!"
        );
        return;
    }

    THREADENTRY32 the{};
    the.dwSize = sizeof(THREADENTRY32);

    if (!Thread32First(hSnapshot, &the)) {
        lpLogger->LogError("Failed to iterate snapshot (cannot enumerate process threads)");
        CloseHandle(hSnapshot);
        return;
    }

    do {
        auto hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, the.th32ThreadID);

        if (hThread == INVALID_HANDLE_VALUE) {
            lpLogger->LogError("Failed to open Thread Handle to thread with id {}", the.th32ThreadID);
            continue;
        }

        this->AddThread(ThreadInformation{.hThread = hThread, .dwTid = the.th32ThreadID, .dwAssignedId = static_cast<unsigned long>(this->m_currentId++)});
    } while (Thread32Next(hSnapshot, &the));

    CloseHandle(hSnapshot);
}

void Fakegrind::Services::ThreadManagerService::Uninitialize() { this->m_threads.clear(); }

void Fakegrind::Services::ThreadManagerService::RemoveThread(const HANDLE hThread, bool bIsThreadExit) {
    const auto &lpLogger = ServiceManager::GetSingleton().GetService<LoggerService>();
    for (auto it = this->m_threads.begin(); it != this->m_threads.end(); ++it) {
        if (it->hThread == hThread) {
            auto additionalText = "Removed by Tracking";

            if (bIsThreadExit)
                additionalText = "Thread Exited";

            lpLogger->LogInfo("Thread with id {} is no longer being tracked ({})", GetThreadId(hThread), additionalText);
            auto thInfo = *it;
            this->m_threads.erase(it);
            this->OnThreadDestroyed.Fire(std::move(thInfo));
            break;
        }
    }
}

void Fakegrind::Services::ThreadManagerService::AddThread(const ThreadInformation &thread) {
    const auto &lpLogger = ServiceManager::GetSingleton().GetService<LoggerService>();
    lpLogger->LogInfo("Tracking Thread with ID {}", thread.dwTid);
    this->m_threads.emplace_back(thread);
    auto thCopy = thread;
    this->OnThreadCreated.Fire(std::move(thCopy));
}

void Fakegrind::Services::ThreadManagerService::AddCurrentThread() {
    this->AddThread(
        ThreadInformation{
            .hThread = GetCurrentThread(),
            .dwTid = GetCurrentThreadId(),
            .dwAssignedId = static_cast<unsigned long>(this->m_currentId++),
        }
    );
}

Fakegrind::Services::ThreadInformation Fakegrind::Services::ThreadManagerService::GetThread(HANDLE hThread) const {
    for (const auto &thread : m_threads) {
        if (thread.hThread == hThread)
            return thread;
    }

    UNREACHABLE("Impossible, how did this occur?"); // all threads are tracked, this is guaranteed.
}

Fakegrind::Services::ThreadInformation Fakegrind::Services::ThreadManagerService::GetCurrentThreadInformation() const {
    return this->GetThread(GetCurrentThread());
}
