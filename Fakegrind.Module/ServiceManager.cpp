//
// Created by Dottik on 26/7/2025.
//

#include "ServiceManager.hpp"
#include "Services/LoggerService.hpp"

void Fakegrind::ServiceManager::Uninitialize() {
    if (!this->m_serviceMap.empty())
        return;

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

Fakegrind::ServiceManager::~ServiceManager() { this->Uninitialize(); }

Fakegrind::ServiceManager &Fakegrind::ServiceManager::GetSingleton() {
    static ServiceManager instance;
    return instance;
}
