//
// Created by Dottik on 26/7/2025.
//

#pragma once
#include "../ServiceManager.hpp"
#include "CommunicationService.hpp"
#include "ConfigurationService.hpp"
#include "IatHookService.hpp"
#include "Service.hpp"
#include "ThreadManagerService.hpp"
#include "cpptrace/basic.hpp"
#include "cpptrace/cpptrace.hpp"

#include <map>

namespace Fakegrind::Services {

struct FreeInfo {
    cpptrace::raw_trace stackTrace;
    ThreadInformation FreedBy;
};

struct AllocationInfo {
    cpptrace::raw_trace stackTrace;
    std::size_t allocationSize;
    ThreadInformation AllocatedBy;
    std::size_t ReassignationCount = 0; // amount of times address has been re-assigned.
};

struct AllocationInformation {
    std::unique_ptr<FreeInfo> FreeData =
        nullptr; // Access to this variable is not guaranteed, as it may not be allocated at the time of creation of the allocation information.
    std::unique_ptr<AllocationInfo> AllocationInfo = nullptr; // everything must be at LEAST allocated to be added here; so access to this is guaranteed almost.
};

struct AllocationShard {
    std::unordered_map<void *, std::shared_ptr<AllocationInformation>> allocations;
    mutable std::mutex memoryLock;
};

class MemoryTrackerService final : public Service {
    static constexpr size_t NUM_SHARDS = 512;
    std::array<AllocationShard, NUM_SHARDS> m_shards;
    AllocationShard &GetShard(void *ptr) { return m_shards[reinterpret_cast<uintptr_t>(ptr) % NUM_SHARDS]; }

    static void TrackAllocation(void *ptr, size_t size, std::shared_ptr<AllocationInformation> lpOldAllocInfo = nullptr) {
        thread_local const auto &lpMemoryTrackerService = ServiceManager::GetSingleton().GetService<MemoryTrackerService>();
        thread_local const auto &lpThreadManagerService = ServiceManager::GetSingleton().GetService<ThreadManagerService>();
        thread_local const auto &lpConfigService = ServiceManager::GetSingleton().GetService<ConfigurationService>();
        thread_local const auto &lpLoggerService = ServiceManager::GetSingleton().GetService<LoggerService>();

        auto &shard = lpMemoryTrackerService->GetShard(ptr);
        std::lock_guard lg(shard.memoryLock);

        if (auto it = shard.allocations.find(ptr); it != shard.allocations.end()) {
            auto &allocation = it->second;
            if (allocation->FreeData == nullptr) {
                cpptrace::stacktrace trace;
                auto thInfo = lpThreadManagerService->GetCurrentThreadInformation();
                if (lpConfigService->IsStackTraceEnabled()) {
                    trace = cpptrace::generate_trace(2);
                }
                lpLoggerService->LogLostAllocation(allocation.get(), &thInfo, &trace, size);
            }
            allocation->FreeData = nullptr;
            allocation->AllocationInfo->ReassignationCount++;
            allocation->AllocationInfo->allocationSize = size;
            if (lpConfigService->IsStackTraceEnabled()) {
                allocation->AllocationInfo->stackTrace = cpptrace::generate_raw_trace(2);
                allocation->AllocationInfo->AllocatedBy = lpThreadManagerService->GetCurrentThreadInformation();
            }
        } else {
            if (lpOldAllocInfo) {
                lpOldAllocInfo->AllocationInfo->allocationSize = size;
                lpOldAllocInfo->AllocationInfo->ReassignationCount++;
                if (lpConfigService->IsStackTraceEnabled()) {
                    lpOldAllocInfo->AllocationInfo->stackTrace = cpptrace::generate_raw_trace(2);
                    lpOldAllocInfo->AllocationInfo->AllocatedBy = lpThreadManagerService->GetCurrentThreadInformation();
                }
                shard.allocations[ptr] = lpOldAllocInfo;
            } else {
                auto lpAllocationData = std::make_unique<AllocationInfo>();
                lpAllocationData->allocationSize = size;
                if (lpConfigService->IsStackTraceEnabled()) {
                    lpAllocationData->stackTrace = cpptrace::generate_raw_trace(2);
                    lpAllocationData->AllocatedBy = lpThreadManagerService->GetCurrentThreadInformation();
                }
                shard.allocations[ptr] = std::make_shared<AllocationInformation>(nullptr, std::move(lpAllocationData));
            }
        }
    }

    static void *hkmalloc(const size_t size) {
        static const auto &lpHookService = ServiceManager::GetSingleton().GetService<IATHookService>();
        auto lpMem = lpHookService->GetOriginalByFunctionPointer<decltype(malloc)>(hkmalloc)(size);
        if (lpMem)
            TrackAllocation(lpMem, size);

        return lpMem;
    }

    static void *hkcalloc(const size_t count, const size_t size) {
        static const auto &lpHookService = ServiceManager::GetSingleton().GetService<IATHookService>();
        auto lpMem = lpHookService->GetOriginalByFunctionPointer<decltype(calloc)>(hkcalloc)(count, size);

        if (lpMem)
            TrackAllocation(lpMem, count * size);

        return lpMem;
    }

    static void *hkrealloc(void *ptr, const size_t nSize) {
        static const auto &lpHookService = ServiceManager::GetSingleton().GetService<IATHookService>();
        static const auto &lpMemoryTrackerService = ServiceManager::GetSingleton().GetService<MemoryTrackerService>();

        if (ptr == nullptr)
            return hkmalloc(nSize);

        if (nSize == 0) {
            hkfree(ptr);
            return nullptr;
        }

        std::shared_ptr<AllocationInformation> originalInfo = nullptr;

        {
            auto &shard = lpMemoryTrackerService->GetShard(ptr);
            std::lock_guard lg(shard.memoryLock);
            if (const auto it = shard.allocations.find(ptr); it != shard.allocations.end()) {
                originalInfo = it->second;
                shard.allocations.erase(it);
            }
        }

        const auto newPtr = lpHookService->GetOriginalByFunctionPointer<decltype(realloc)>(hkrealloc)(ptr, nSize);

        if (newPtr) {
            TrackAllocation(newPtr, nSize, originalInfo);
        } else {
            if (originalInfo)
                TrackAllocation(ptr, originalInfo->AllocationInfo->allocationSize, originalInfo);
        }

        return newPtr;
    }

    static void hkfree(void *memory) {
        if (memory == nullptr)
            return;

        auto bCallFree = false;

        static const auto lpMemoryTrackerService = ServiceManager::GetSingleton().GetService<MemoryTrackerService>();
        static const auto lpThreadManagerService = ServiceManager::GetSingleton().GetService<ThreadManagerService>();
        static const auto lpConfigService = ServiceManager::GetSingleton().GetService<ConfigurationService>();

        {
            auto &shard = lpMemoryTrackerService->GetShard(memory);
            auto &allocMap = shard.allocations;
            std::lock_guard lg(shard.memoryLock);

            if (const auto it = allocMap.find(memory); it == allocMap.end()) {
                bCallFree = true;
            } else {
                const auto &allocation = it->second;

                if (allocation->FreeData != nullptr) {
                    const auto thInfo = lpThreadManagerService->GetCurrentThreadInformation();
                    auto trace = cpptrace::stacktrace{};
                    if (lpConfigService->IsStackTraceEnabled())
                        trace = cpptrace::generate_trace(1);
                    ServiceManager::GetSingleton().GetService<LoggerService>()->LogDoubleFree(allocation.get(), &thInfo, &trace);
                } else {
                    if (lpConfigService->IsStackTraceEnabled()) {
                        auto frames = cpptrace::generate_raw_trace(1);
                        allocation->FreeData = std::make_unique<FreeInfo>(frames, lpThreadManagerService->GetCurrentThreadInformation());
                    } else {
                        allocation->FreeData = std::make_unique<FreeInfo>(cpptrace::raw_trace{}, lpThreadManagerService->GetCurrentThreadInformation());
                    }
                    bCallFree = true;
                }
            }
        }

        if (bCallFree) {
            const auto &lpHookService = ServiceManager::GetSingleton().GetService<IATHookService>();
            lpHookService->GetOriginalByFunctionPointer<decltype(free)>(hkfree)(memory);
        }
    }

  public:
    void Initialize() override;

    void ForEachEntry(const std::function<void(const std::pair<void *const, std::shared_ptr<AllocationInformation>> &)> &perEntryCallback) const;

    void Uninitialize() override {}

    void LogAllocationEntries() const;

    void CheckAllocatedBlocks() const;
};
} // namespace Fakegrind::Services