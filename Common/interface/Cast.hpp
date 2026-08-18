/*  参考DiligentEngine:https://github.com/DiligentGraphics/DiligentEngine  */
#pragma once

#include "Basic/interface/DebugUtilities.hpp"
#include "CompilerDefinitions.h"

namespace Hikali
{
    template <typename DstType, typename SrcType>
    NODISCARD DstType* ClassPtrCast(SrcType* Ptr)
    {
#ifdef _DEBUG
        if (Ptr != nullptr)
        {
            CHECK_DYNAMIC_TYPE(DstType, Ptr);
        }
#endif
        return static_cast<DstType*>(Ptr);
    }


    template <typename DstType, typename SrcType>
    NODISCARD DstType BitCast(const SrcType& Src)
    {
        static_assert(sizeof(DstType) >= sizeof(SrcType), "DstType size must be greater than or equal to SrcType size");
        DstType Dst = static_cast<DstType>(0);
        memcpy(&Dst, &Src, sizeof(Src));
        return Dst;
    }


    template <typename DstType, typename SrcType>
    NODISCARD DstType StaticCast(const SrcType& Src)
    {
#ifdef _DEBUG
        using MaxType = decltype(DstType{ 1 } + SrcType{ 1 });
        VERIFY(static_cast<MaxType>(Src) == static_cast<MaxType>(static_cast<DstType>(Src)), "Cast will lose data");
#endif
        return static_cast<DstType>(Src);
    }
}
