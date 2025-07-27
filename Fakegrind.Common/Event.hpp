//
// Created by Dottik on 26/7/2025.
//
#pragma once

#include <cstdint>
#include <functional>
#include <map>

namespace Fakegrind {
template <typename... Y> class ListenableFireableObject final {
    std::function<void(Y &&...)> m_dispatcher;

  public:
    explicit ListenableFireableObject(std::function<void(Y &&...)> func) { this->m_dispatcher = func; }

    void Fire(Y &&...arg) { this->m_dispatcher(arg...); }
};

template <typename... Y> class ListenableEvent final {
    std::map<std::uintptr_t, std::function<void(Y &&...)>> m_functionList;

    std::uintptr_t CreateAttachId() {
        std::uintptr_t id{};

        for (auto &f : this->m_functionList)
            if (id <= f.first) [[likely]]
                id = f.first + 1;

        return id;
    }

  public:
    ListenableEvent() = default;

    ListenableFireableObject<Y...> GetFirableObject() {
        return ListenableFireableObject<Y...>([this](Y &&...arg) {
            if (this->m_functionList.empty()) [[unlikely]]
                return;

            for (auto &f : this->m_functionList)
                f.second(std::forward<Y>(arg)...);
        });
    }

    void Fire(Y &&...arg) {
        if (this->m_functionList.empty()) [[unlikely]]
            return;
        for (auto &f : this->m_functionList)
            f.second(std::forward<Y>(arg)...);
    }

    bool IsAttached(std::uintptr_t attachId) { return this->m_functionList.contains(attachId); }

    std::uintptr_t AttachFunction(std::function<void(Y &&...)> func) {
        std::uintptr_t attachId = this->CreateAttachId();
        this->m_functionList[attachId] = func;
        return attachId;
    }

    void Destroy() { this->m_functionList.clear(); }
};
} // namespace Fakegrind
