//
// Created by Dottik on 26/7/2025.
//

#pragma once
#include "MemoryTrackerService.hpp"
#include "Service.hpp"

namespace Fakegrind::Services {
class CommunicationService final : public Service {
  public:
    void Initialize() override {}

    void Uninitialize() override {}

    void NotifyDoubleFree(const AllocationInformation &mem) {

        MessageBoxA(nullptr, "Fakegrind", "Double Free Condition!", MB_OK);
    }
};
} // namespace Fakegrind::Services