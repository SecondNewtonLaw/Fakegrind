//
// Created by Dottik on 26/7/2025.
//


#pragma once
#include "Service.hpp"

namespace Fakegrind::Services {
    class ThreadManagerService final : public Service {
        std::vector<HANDLE> m_threads;
    public:

        void Initialize() override;
        void Uninitialize() override;

        void RemoveThread(HANDLE hThread, bool bIsThreadExit = false);
        void AddThread(HANDLE hThread);
    };
} // namespace Fakegrind::Services
