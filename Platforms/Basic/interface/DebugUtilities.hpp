/*  参考DiligentEngine:https://github.com/DiligentGraphics/DiligentEngine  */
#pragma once

#include "FormatString.hpp"
#include "Error.hpp"
#include "BasicPlatformDebug.hpp"

#ifdef _DEBUG

#    include <typeinfo>

#    define ASSERTION_FAILED(Message, ...)                                                 \
        do                                                                                 \
        {                                                                                  \
            auto msg = Hikali::FormatString(Message, ##__VA_ARGS__);                     \
            Hikali::DebugAssertionFailed(msg.c_str(), __FUNCTION__, __FILE__, __LINE__); \
        } while (false)

#    define VERIFY(Expr, Message, ...)                    \
        do                                                \
        {                                                 \
            if (!(Expr))                                  \
            {                                             \
                ASSERTION_FAILED(Message, ##__VA_ARGS__); \
            }                                             \
        } while (false)

#    define UNEXPECTED  ASSERTION_FAILED
#    define UNSUPPORTED ASSERTION_FAILED

#    define VERIFY_EXPR(Expr) VERIFY((Expr), "Debug expression failed:\n", #    Expr)


template <typename DstType, typename SrcType>
void CheckDynamicType(SrcType* pSrcPtr)
{
    VERIFY(pSrcPtr == nullptr || dynamic_cast<DstType*>(pSrcPtr) != nullptr, "Dynamic type cast failed. Src typeid: \'", typeid(*pSrcPtr).name(), "\' Dst typeid: \'", typeid(DstType).name(), '\'');
}
#    define CHECK_DYNAMIC_TYPE(DstType, pSrcPtr) \
        do                                       \
        {                                        \
            CheckDynamicType<DstType>(pSrcPtr);  \
        } while (false)


#else

// clang-format off
#    define CHECK_DYNAMIC_TYPE(...) do{}while(false)
#    define VERIFY(...)do{}while(false)
#    define UNEXPECTED(...)do{}while(false)
#    define UNSUPPORTED(...)do{}while(false)
#    define VERIFY_EXPR(...)do{}while(false)
// clang-format on

#endif

#if defined(_DEBUG)
#    define DEV_CHECK_ERR VERIFY
#else
// clang-format off
#    define DEV_CHECK_ERR(...)do{}while(false)
// clang-format on
#endif

#define DEV_ERROR(...) DEV_CHECK_ERR(false, __VA_ARGS__)

#ifdef _DEBUG

#    define DEV_CHECK_WARN CHECK_WARN
#    define DEV_CHECK_INFO CHECK_INFO

#else

// clang-format off
#    define DEV_CHECK_WARN(...)do{}while(false)
#    define DEV_CHECK_INFO(...)do{}while(false)
// clang-format on

#endif