//
// Created by Dottik on 26/7/2025.
//

#include "CommunicationService.hpp"
void Fakegrind::Services::CommunicationService::NotifyDoubleFree(const AllocationInformation &mem) {

    MessageBoxA(nullptr, "Fakegrind", "Double Free Condition!", MB_OK);
}