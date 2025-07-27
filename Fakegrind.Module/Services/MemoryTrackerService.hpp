//
// Created by Dottik on 26/7/2025.
//

#pragma once
#include "../../Fakegrind.Common/pch.hpp"
#include "../ServiceManager.hpp"
#include "CommunicationService.hpp"
#include "IatHookService.hpp"
#include "Service.hpp"
#include "ThreadManagerService.hpp"
#include "cpptrace/basic.hpp"

#include <map>

namespace Fakegrind::Services {

struct FreeInfo {
    cpptrace::stacktrace stackTrace;
    ThreadInformation FreedBy;
};

struct AllocationInfo {
    cpptrace::stacktrace stackTrace;
    std::size_t allocationSize;
    ThreadInformation AllocatedBy;
};

struct AllocationInformation {
    std::unique_ptr<FreeInfo> FreeData =
        nullptr; // Access to this variable is not guaranteed, as it may not be allocated at the time of creation of the allocation information.
    std::unique_ptr<AllocationInfo> AllocationInfo = nullptr; // everything must be at LEAST allocated to be added here; so access to this is guaranteed almost.
};

class MemoryTrackerService final : public Service {
    std::map<void *, AllocationInformation> m_allocations;

    mutable std::mutex m_memoryLock;

    std::mutex *GetMemoryLock() { return &this->m_memoryLock; }

    static void *hkmalloc(size_t size) {
        const auto &lpHookService = ServiceManager::GetSingleton().GetService<IATHookService>();
        const auto &lpMemoryTrackerService = ServiceManager::GetSingleton().GetService<MemoryTrackerService>();
        const auto &lpThreadManagerService = ServiceManager::GetSingleton().GetService<ThreadManagerService>();

        const auto lpMem = lpHookService->GetOriginalByFunctionPointer<decltype(malloc)>(hkmalloc)(size);
        auto lpAllocationData = std::make_unique<AllocationInfo>();
        lpAllocationData->AllocatedBy = lpThreadManagerService->GetCurrentThreadInformation();
        lpAllocationData->allocationSize = size;
        lpAllocationData->stackTrace = std::move(cpptrace::generate_trace(0));

        {
            std::lock_guard lg(*lpMemoryTrackerService->GetMemoryLock());
            lpMemoryTrackerService->m_allocations[lpMem] = AllocationInformation{nullptr, std::move(lpAllocationData)};
        }

        return lpMem;
    }

    static void hkfree(void *memory) {
        const auto &lpHookService = ServiceManager::GetSingleton().GetService<IATHookService>();
        const auto &lpMemoryTrackerService = ServiceManager::GetSingleton().GetService<MemoryTrackerService>();
        const auto &lpThreadManagerService = ServiceManager::GetSingleton().GetService<ThreadManagerService>();

        std::lock_guard lg(*lpMemoryTrackerService->GetMemoryLock());
        auto &mem = lpMemoryTrackerService->m_allocations.at(memory);

        if (mem.FreeData != nullptr) {
            ServiceManager::GetSingleton().GetService<CommunicationService>()->NotifyDoubleFree(mem);
        }

        lpHookService->GetOriginalByFunctionPointer<decltype(free)>(hkfree)(memory);
        mem.FreeData = std::make_unique<FreeInfo>(std::move(cpptrace::generate_trace(0)), lpThreadManagerService->GetCurrentThreadInformation());
    }

  public:
    void Initialize() override {
        // Minhook should be initialized by this point.
    }
    void Uninitialize() override {}
};
} // namespace Fakegrind::Services