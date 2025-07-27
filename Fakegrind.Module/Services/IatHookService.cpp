//
// Created by Dottik on 26/7/2025.
//

#include "IatHookService.hpp"

IMAGE_IMPORT_DESCRIPTOR *Fakegrind::Services::IATHookService::GetImportTable(HMODULE hModule) {
    const auto lpDOS = reinterpret_cast<IMAGE_DOS_HEADER *>(hModule);
    ASSERT(lpDOS->e_magic == IMAGE_DOS_SIGNATURE);
    const auto lpNT = reinterpret_cast<IMAGE_NT_HEADERS *>(reinterpret_cast<uintptr_t>(hModule) + lpDOS->e_lfanew);
    ASSERT(lpNT->Signature == IMAGE_NT_SIGNATURE);
    auto pImportDirectory = lpNT->OptionalHeader.DataDirectory + IMAGE_DIRECTORY_ENTRY_IMPORT;
    ASSERT(pImportDirectory != nullptr && pImportDirectory->VirtualAddress != 0);
    const auto pImportDescriptor = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR *>(reinterpret_cast<std::uintptr_t>(hModule) + pImportDirectory->VirtualAddress);
    ASSERT(pImportDescriptor != nullptr);
    return pImportDescriptor;
}

void Fakegrind::Services::IATHookService::Initialize() { /* No Initialization required*/ }

void Fakegrind::Services::IATHookService::HookByOrdinal(unsigned short ordinal, void *replaceWith) {
    auto lpLogger = ServiceManager::GetSingleton().GetService<Services::LoggerService>();
    auto hModule = GetModuleHandle(nullptr);
    auto pImportDescriptor = this->GetImportTable(hModule);
    while (pImportDescriptor->Name != 0) {
        auto lpImportNameTable = reinterpret_cast<IMAGE_THUNK_DATA *>(reinterpret_cast<std::uintptr_t>(hModule) + pImportDescriptor->OriginalFirstThunk);
        auto lpImportAddressTable = reinterpret_cast<IMAGE_THUNK_DATA *>(reinterpret_cast<std::uintptr_t>(hModule) + pImportDescriptor->FirstThunk);

        while (lpImportNameTable->u1.AddressOfData != 0) {
            if (IMAGE_ORDINAL(lpImportNameTable->u1.Ordinal) == ordinal) {
                void **pIatEntry = &reinterpret_cast<void *&>(lpImportAddressTable->u1.Function);
                void *pOriginalFunction = *pIatEntry;

                DWORD oldProtect = 0;
                if (VirtualProtect(pIatEntry, sizeof(void *), PAGE_READWRITE, &oldProtect)) {
                    *pIatEntry = replaceWith;
                    lpLogger->LogInfo("Hooked IAT entry of running executable at ordinal '{}' to point to custom implementation (instrumented)", ordinal);
                    VirtualProtect(pIatEntry, sizeof(void *), oldProtect, &oldProtect); // Restore
                    this->m_iatAddressToOriginal[pIatEntry] = pOriginalFunction;
                    this->m_hookToOriginal[replaceWith] = pOriginalFunction;
                } else {
                    lpLogger->LogWarning("IAT Hook failed for ordinal '{}'. The call will not be instrumented.", ordinal);
                }
            }
            lpImportNameTable++;
            lpImportAddressTable++;
        }
        pImportDescriptor++;
    }
}

void Fakegrind::Services::IATHookService::HookByName(const char *name, void *replaceWith) {
    auto lpLogger = ServiceManager::GetSingleton().GetService<Services::LoggerService>();
    auto hModule = GetModuleHandle(nullptr);
    auto pImportDescriptor = this->GetImportTable(hModule);
    auto hooked = false;
    while (pImportDescriptor->Name != 0) {
        auto lpImportNameTable = reinterpret_cast<IMAGE_THUNK_DATA *>(reinterpret_cast<std::uintptr_t>(hModule) + pImportDescriptor->OriginalFirstThunk);
        auto lpImportAddressTable = reinterpret_cast<IMAGE_THUNK_DATA *>(reinterpret_cast<std::uintptr_t>(hModule) + pImportDescriptor->FirstThunk);

        while (lpImportNameTable->u1.AddressOfData != 0) {
            if (!IMAGE_SNAP_BY_ORDINAL(lpImportNameTable->u1.Ordinal)) {
                auto pImportByName = reinterpret_cast<IMAGE_IMPORT_BY_NAME *>(reinterpret_cast<std::uintptr_t>(hModule) + lpImportNameTable->u1.AddressOfData);

                if (strcmp(pImportByName->Name, name) == 0) {
                    void **pIatEntry = &reinterpret_cast<void *&>(lpImportAddressTable->u1.Function);
                    void *pOriginalFunction = *pIatEntry;

                    DWORD oldProtect = 0;
                    if (VirtualProtect(pIatEntry, sizeof(void *), PAGE_READWRITE, &oldProtect)) {
                        *pIatEntry = replaceWith;
                        lpLogger->LogInfo("Hooked IAT entry of running executable '{}' to point to custom implementation (instrumented)", name);
                        VirtualProtect(pIatEntry, sizeof(void *), oldProtect, &oldProtect); // Restore
                        this->m_iatAddressToOriginal[pIatEntry] = pOriginalFunction;
                        this->m_hookToOriginal[replaceWith] = pOriginalFunction;
                        hooked = true;
                    } else {
                        lpLogger->LogWarning("IAT Hook failed for '{}'. The call will not be instrumented.", name);
                    }
                }
            }
            lpImportNameTable++;
            lpImportAddressTable++;
        }
        pImportDescriptor++;
    }

    if (!hooked) {
        lpLogger->LogWarning("IAT entry for function '{}' not found! The function may be statically linked/ not imported!", name);
    }
}

void Fakegrind::Services::IATHookService::Uninitialize() {
    for (const auto &address : this->m_iatAddressToOriginal) {
        DWORD dwOld = 0;
        VirtualProtect(address.first, sizeof(void *), PAGE_READWRITE, &dwOld);
        memcpy(address.first, &address.second, sizeof(void *)); // replace back.
        VirtualProtect(address.first, sizeof(void *), dwOld, nullptr);
    }
}