//
// Created by Dottik on 27/7/2025.
//

#pragma once
#include "Service.hpp"

#include <atomic>
#include <new>

namespace Fakegrind::Services {
class ConfigurationService final : public Service {
  public:
    bool IsStackTraceEnabled() const { return this->m_isStackTraceEnabled; }
    void SetStackTraceEnabled(const bool enabled) { this->m_isStackTraceEnabled = enabled; }
    void Initialize() override {}
    void Uninitialize() override {}

  private:
    alignas(std::hardware_destructive_interference_size) std::atomic<bool> m_isStackTraceEnabled = true;
};
} // namespace Fakegrind::Services