//
// Created by Dottik on 26/7/2025.
//

#pragma once
#include "../ServiceManager.hpp"
#include "LoggerService.hpp"
#include "Service.hpp"

namespace Fakegrind::Services {
class IATHookService final : public Service {
    std::unordered_map<void *, void *> m_iatAddressToOriginal; // used to reset on uninitialize.
    std::unordered_map<void *, void *> m_hookToOriginal;       // only for ease of usage.

  public:
    IMAGE_IMPORT_DESCRIPTOR *GetImportTable(HMODULE hModule);

    void Initialize() override;

    template <typename T> std::add_pointer_t<T> GetOriginalByFunctionPointer(T hook) {
        ASSERT(this->m_hookToOriginal.contains(reinterpret_cast<void *>(hook)));
        return reinterpret_cast<std::add_pointer_t<T>>(reinterpret_cast<uintptr_t>(this->m_hookToOriginal.at(reinterpret_cast<void *>(hook))));
    }

    void HookByOrdinal(unsigned short ordinal, void *replaceWith);

    void HookByName(const char *name, void *replaceWith);

    void Uninitialize() override;
};
} // namespace Fakegrind
