//
// Created by Dottik on 26/7/2025.
//

#include "MemoryTrackerService.hpp"
#include "LoggerService.hpp"

void Fakegrind::Services::MemoryTrackerService::Initialize() {
    // Minhook should be initialized by this point.
    const auto &lpIATHookService = ServiceManager::GetSingleton().GetService<IATHookService>();

    lpIATHookService->HookByName("_alloca", (void *)hkmalloc);
    lpIATHookService->HookByName("malloc", (void *)hkmalloc);

    lpIATHookService->HookByName("free", (void *)hkfree);

    lpIATHookService->HookByName("realloc", (void *)hkrealloc);
    lpIATHookService->HookByName("calloc", (void *)hkcalloc);
}

void Fakegrind::Services::MemoryTrackerService::ForEachEntry(
    const std::function<void(const std::pair<void *const, std::shared_ptr<AllocationInformation>> &)> &perEntryCallback
) const {
    std::vector<std::unique_lock<std::mutex>> locks;
    for (const auto &shard : m_shards)
        locks.emplace_back(shard.memoryLock);

    for (const auto &shard : m_shards)
        for (const auto &entry : shard.allocations)
            perEntryCallback(entry);
}

void Fakegrind::Services::MemoryTrackerService::LogAllocationEntries() const {
    auto qwTotalLeak = 0ull;
    auto qwTotalAllocated = 0ull;
    auto qwAllocationCount = 0ull;
    auto qwDeallocationCount = 0ull;

    this->ForEachEntry([&](const auto &entry) {
        const auto &allocationInfo = entry.second;

        if (allocationInfo->FreeData == nullptr) {
            qwTotalLeak += allocationInfo->AllocationInfo->allocationSize;
        } else {
            qwDeallocationCount++;
        }

        qwAllocationCount++;
        qwTotalAllocated += allocationInfo->AllocationInfo->allocationSize;
    });

    const auto &lpLogger = ServiceManager::GetSingleton().GetService<LoggerService>();
    lpLogger->LogWarning(
        "==== Fakegrind Memory Allocation Report ====\n"
        "\t- Total Allocations: {}\n"
        "\t- Total Frees: {}\n"
        "\t- Leaked Blocks: {}\n"
        "\t- Total Memory Allocated: {} bytes\n"
        "\t- Leaked Memory: {} bytes\n"
        "============================================",
        qwAllocationCount, qwDeallocationCount, (qwAllocationCount - qwDeallocationCount), qwTotalAllocated, qwTotalLeak
    );
}

void Fakegrind::Services::MemoryTrackerService::CheckAllocatedBlocks() const {
    const auto &lpLogger = ServiceManager::GetSingleton().GetService<LoggerService>();
    auto leakedBlocks = 0ull;
    auto totalLeakedBytes = 0ull;

    lpLogger->LogWarning("==== Fakegrind Checking Allocated blocks ====");

    this->ForEachEntry([&](const auto &entry) {
        if (const auto &allocationInfo = entry.second; allocationInfo->FreeData == nullptr) {
            lpLogger->LogPossibleLeak(allocationInfo.get());
            leakedBlocks++;
            totalLeakedBytes += allocationInfo->AllocationInfo->allocationSize;
        }
    });

    if (leakedBlocks > 0) {
        lpLogger->LogWarning("==== Fakegrind Checked All Allocated Blocks. Found {} leaked block(s) totaling {} bytes. ====", leakedBlocks, totalLeakedBytes);
    } else {
        lpLogger->LogInfo("==== Fakegrind Checked All Allocated Blocks. No memory leaks detected. ====");
    }
}
