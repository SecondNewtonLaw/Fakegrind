//
// Created by Dottik on 26/7/2025.
//

#include <MinHook.h>
#include <Windows.h>
#include "ServiceManager.hpp"
#include "Services/LoggerService.hpp"
#include "Services/MemoryTrackerService.hpp"
#include "Services/ThreadManagerService.hpp"
#include "../Fakegrind.Common/pch.hpp"


BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwCallReason, LPVOID lpvReserved) {
    UNUSED_ARGUMENT(hModule);
    UNUSED_ARGUMENT(lpvReserved);
    auto &lpManager = Fakegrind::ServiceManager::GetSingleton();

    switch (dwCallReason) {
    case DLL_PROCESS_ATTACH: {
        MH_Initialize();
        lpManager.AddService<Fakegrind::Services::LoggerService>();
        lpManager.AddService<Fakegrind::Services::MemoryTrackerService>();
        lpManager.AddService<Fakegrind::Services::ThreadManagerService>();
        break;
    }

    case DLL_THREAD_ATTACH: {
        auto lpThreadManagerService = lpManager.GetService<Fakegrind::Services::ThreadManagerService>();
        lpThreadManagerService->AddCurrentThread();
        break;
    }

    case DLL_THREAD_DETACH: {
        auto lpThreadManagerService = lpManager.GetService<Fakegrind::Services::ThreadManagerService>();
        lpThreadManagerService->RemoveThread(GetCurrentThread(), true);
        break;
    }

    case DLL_PROCESS_DETACH: {
        lpManager.GetSingleton().Uninitialize();
        break;
    }

    default:
        return FALSE;
    }

    return TRUE;
}
