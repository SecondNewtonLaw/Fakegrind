//
// Created by Dottik on 26/7/2025.
//

#include "ServiceManager.hpp"
#include "Services/LoggerService.hpp"

Fakegrind::ServiceManager::~ServiceManager() {
    auto lpLoggerService = this->GetService<Services::LoggerService>();

    lpLoggerService->LogInfo("Goodbye!");

    for (auto &service : this->m_serviceMap) {
        if (service.second.use_count() > 1) {
            // forcefully resetting here could induce a crash.
            UNREACHABLE("You should not reach here, this means bad resource management in Fakegrind.");
        }

        service.second->Uninitialize();
        service.second.reset();
    }

    this->m_serviceMap.clear();
}

Fakegrind::ServiceManager &Fakegrind::ServiceManager::GetSingleton() {
    static ServiceManager instance;
    return instance;
}
