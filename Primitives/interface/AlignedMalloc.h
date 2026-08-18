/*  参考DiligentEngine:https://github.com/DiligentGraphics/DiligentEngine  */
#pragma once

#include <stdlib.h>

#ifdef USE_CRT_MALLOC_DBG
#    include <crtdbg.h>
#endif

#ifdef HIKALI_ALIGNED_MALLOC
#    undef HIKALI_ALIGNED_MALLOC
#endif
#ifdef ALIGNED_FREE
#    undef ALIGNED_FREE
#endif

#if defined(_MSC_VER) && defined(USE_CRT_MALLOC_DBG)
#    define HIKALI_ALIGNED_MALLOC(Size, Alignment, dbgFileName, dbgLineNumber) _aligned_malloc_dbg(Size, Alignment, dbgFileName, dbgLineNumber)
#    define HIKALI_ALIGNED_FREE(Ptr)                                           _aligned_free(Ptr)
#elif defined(_MSC_VER) || defined(__MINGW64__) || defined(__MINGW32__)
#    define HIKALI_ALIGNED_MALLOC(Size, Alignment, dbgFileName, dbgLineNumber) _aligned_malloc(Size, Alignment)
#    define HIKALI_ALIGNED_FREE(Ptr)                                           _aligned_free(Ptr)
#elif defined(USE_ALIGNED_MALLOC_FALLBACK)
#    define HIKALI_ALIGNED_MALLOC(Size, Alignment, dbgFileName, dbgLineNumber) AllocateAlignedFallback(Size, Alignment)
#    define HIKALI_ALIGNED_FREE(Ptr)                                           FreeAlignedFallback(Ptr)
#else
#    define HIKALI_ALIGNED_MALLOC(Size, Alignment, dbgFileName, dbgLineNumber) aligned_alloc(Alignment, ((Size) + (Alignment)-1) & ~((Alignment)-1))
#    define HIKALI_ALIGNED_FREE(Ptr)                                           free(Ptr)
#endif

namespace Hikali
{
    void* AllocateAlignedFallback(size_t Size, size_t Alignment);
    void  FreeAlignedFallback(void* Ptr);
}