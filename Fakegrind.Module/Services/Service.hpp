//
// Created by Dottik on 26/7/2025.
//

#pragma once

namespace Fakegrind {
class Service {

  public:
    virtual ~Service() = default;

    virtual void Initialize() = 0;

    virtual void Uninitialize() = 0;
};
} // namespace Fakegrind
