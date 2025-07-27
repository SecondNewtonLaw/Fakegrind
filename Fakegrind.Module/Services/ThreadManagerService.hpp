//
// Created by Dottik on 26/7/2025.
//

#pragma once
#include "../../Fakegrind.Common/Event.hpp"
#include "Service.hpp"

namespace Fakegrind::Services {

struct ThreadInformation {
    HANDLE hThread;
    DWORD dwTid;
    DWORD dwAssignedId;
};

class ThreadManagerService final : public Service {
    mutable std::mutex m_mutex;
    std::vector<ThreadInformation> m_threads;
    std::uint32_t m_currentId = 0;

  public:
    Fakegrind::ListenableEvent<ThreadInformation> OnThreadCreated;
    Fakegrind::ListenableEvent<ThreadInformation> OnThreadDestroyed;

    void Initialize() override;
    void Uninitialize() override;

    void RemoveThread(HANDLE hThread, bool bIsThreadExit = false);
    void AddThread(const ThreadInformation &thread);
    void AddCurrentThread();

    [[nodiscard]] ThreadInformation GetThread(HANDLE hThread) const;

    [[nodiscard]] ThreadInformation GetCurrentThreadInformation() const;
};
} // namespace Fakegrind::Services
