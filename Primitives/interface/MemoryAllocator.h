/*  参考DiligentEngine:https://github.com/DiligentGraphics/DiligentEngine  */
#pragma once

#include <cstdint>

namespace Hikali
{
    /// Base interface for a raw memory allocator
    struct IMemoryAllocator
    {
        /// Allocates block of memory
        virtual void* Allocate(size_t Size, const char* dbgDescription, const char* dbgFileName, const int32_t dbgLineNumber) = 0;

        /// Releases memory
        virtual void Free(void* Ptr) = 0;

        /// Allocates block of memory with specified alignment
        virtual void* AllocateAligned(size_t Size, size_t Alignment, const char* dbgDescription, const char* dbgFileName, const int32_t dbgLineNumber) = 0;

        /// Releases memory allocated with AllocateAligned
        virtual void FreeAligned(void* Ptr) = 0;
    };
}