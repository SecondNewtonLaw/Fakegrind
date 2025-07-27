//
// Created by Dottik on 26/7/2025.
//

#include "ServiceManager.hpp"
#include "Services/ConfigurationService.hpp"
#include "Services/IatHookService.hpp"
#include "Services/LoggerService.hpp"
#include "Services/MemoryTrackerService.hpp"
#include "Services/ThreadManagerService.hpp"
#include <MinHook.h>
#include <Windows.h>

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwCallReason, LPVOID lpvReserved) {
    UNUSED_ARGUMENT(hModule);
    UNUSED_ARGUMENT(lpvReserved);
    auto &lpManager = Fakegrind::ServiceManager::GetSingleton();

    switch (dwCallReason) {
    case DLL_PROCESS_ATTACH: {
        MH_Initialize();
        lpManager.AddService<Fakegrind::Services::LoggerService>();
        lpManager.AddService<Fakegrind::Services::ConfigurationService>();
        lpManager.AddService<Fakegrind::Services::CommunicationService>();
        lpManager.AddService<Fakegrind::Services::ThreadManagerService>();
        lpManager.AddService<Fakegrind::Services::IATHookService>();
        lpManager.AddService<Fakegrind::Services::MemoryTrackerService>();
        break;
    }

    case DLL_THREAD_ATTACH: {
        const auto &lpThreadManagerService = lpManager.GetService<Fakegrind::Services::ThreadManagerService>();
        lpThreadManagerService->AddCurrentThread();
        break;
    }

    case DLL_THREAD_DETACH: {
        const auto &lpThreadManagerService = lpManager.GetService<Fakegrind::Services::ThreadManagerService>();
        lpThreadManagerService->RemoveThread(GetCurrentThread(), true);
        break;
    }

    case DLL_PROCESS_DETACH: {
        const auto &lpMemoryTrackerService = lpManager.GetService<Fakegrind::Services::MemoryTrackerService>();
        lpMemoryTrackerService->CheckAllocatedBlocks();
        lpManager.Uninitialize();
        break;
    }

    default:
        return FALSE;
    }

    return TRUE;
}
