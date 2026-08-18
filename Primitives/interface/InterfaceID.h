/*  参考DiligentEngine:https://github.com/DiligentGraphics/DiligentEngine  */
#pragma once

#include <string>
#include <cstdint>

namespace Hikali
{
    /// Unique interface identifier
    struct INTERFACE_ID
    {
        uint32_t Data1;
        uint16_t Data2;
        uint16_t Data3;
        uint8_t  Data4[8];

        bool operator==(const INTERFACE_ID& rhs) const noexcept
        {
            return Data1 == rhs.Data1 &&
                Data2 == rhs.Data2 &&
                Data3 == rhs.Data3 &&
                memcmp(Data4, rhs.Data4, sizeof(Data4)) == 0;
        }
        bool operator!=(const INTERFACE_ID& rhs) const noexcept
        {
            return !(*this == rhs);
        }
    };

    /// Unknown interface
    static constexpr INTERFACE_ID IID_Unknown = {0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
}