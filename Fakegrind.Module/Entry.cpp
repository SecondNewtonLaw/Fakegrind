//
// Created by Dottik on 26/7/2025.
//

#include <Windows.h>

#include "ServiceManager.hpp"
#include "Services/LoggerService.hpp"
#include "Services/ThreadManagerService.hpp"

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwCallReason, LPVOID lpvReserved) {
    auto &lpManager = Fakegrind::ServiceManager::GetSingleton();

    switch (dwCallReason) {
        case DLL_PROCESS_ATTACH: {
            lpManager.AddService<Fakegrind::Services::LoggerService>();
            lpManager.AddService<Fakegrind::Services::ThreadManagerService>();

            break;
        }

        case DLL_THREAD_ATTACH: {
            auto lpThreadManagerService = lpManager.GetService<Fakegrind::Services::ThreadManagerService>();
            lpThreadManagerService->AddThread(GetCurrentThread());
            break;
        }

        case DLL_THREAD_DETACH: {
            auto lpThreadManagerService = lpManager.GetService<Fakegrind::Services::ThreadManagerService>();
            lpThreadManagerService->RemoveThread(GetCurrentThread(), true);
            break;
        }

        case DLL_PROCESS_DETACH: break;
        default: return FALSE;
    }

    return TRUE;
}
