//
// Created by Dottik on 26/7/2025.
//

#pragma once
#include "Services/Service.hpp"

namespace Fakegrind {
    class ServiceManager final {
        std::recursive_mutex m_syncLock;
        std::map<std::type_index, std::shared_ptr<Fakegrind::Service>> m_serviceMap;

    public:
        ~ServiceManager();
        static ServiceManager &GetSingleton();

        template<typename T>
            requires std::is_base_of_v<Fakegrind::Service, T>
        std::shared_ptr<T> GetService() {
            std::lock_guard lock(this->m_syncLock);

            const auto it = m_serviceMap.find(typeid(T));
            ASSERT(it != m_serviceMap.end(), "Service not found.");

            return std::dynamic_pointer_cast<T>(it->second);
        }

        template<typename T>
            requires std::is_base_of_v<Fakegrind::Service, T>
        std::shared_ptr<T> AddService() {
            std::lock_guard lock(this->m_syncLock);
            const auto it = m_serviceMap.find(typeid(T));
            ASSERT(it == m_serviceMap.end(), "Service already exists.", typeid(T).name());

            auto lpNewService = std::make_shared<T>();
            lpNewService->Initialize();
            this->m_serviceMap[typeid(T)] = std::dynamic_pointer_cast<Fakegrind::Service>(lpNewService);
            return lpNewService;
        }

        template<typename T, typename... Args>
            requires std::is_base_of_v<Fakegrind::Service, T>
        std::shared_ptr<T> AddService(Args &...args) {
            std::lock_guard lock(this->m_syncLock);
            const auto it = m_serviceMap.find(typeid(T));
            ASSERT(it == m_serviceMap.end(), "Service already exists.", typeid(T).name());

            auto lpNewService = std::make_shared<T>(std::forward<Args>(args)...);
            lpNewService->Initialize();
            this->m_serviceMap[typeid(T)] = std::dynamic_pointer_cast<Fakegrind::Service>(lpNewService);
            return lpNewService;
        }
    };
} // namespace Fakegrind
