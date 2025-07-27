//
// Created by Dottik on 26/7/2025.
//

#pragma once
#include "Service.hpp"

namespace Fakegrind::Services {
struct AllocationInformation;
class CommunicationService final : public Service {
  public:
    void Initialize() override {}

    void Uninitialize() override {}

    void NotifyDoubleFree(const AllocationInformation &mem);
};
} // namespace Fakegrind::Services