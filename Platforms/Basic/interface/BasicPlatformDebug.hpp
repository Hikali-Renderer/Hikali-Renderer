/*  参考DiligentEngine:https://github.com/DiligentGraphics/DiligentEngine  */
#pragma once

#include "Error.hpp"

namespace Hikali
{
    enum class TextColor
    {
        Auto, // 文字颜色由信息严重程度决定

        Default,

        Black,
        DarkRed,
        DarkGreen,
        DarkYellow,
        DarkBlue,
        DarkMagenta,
        DarkCyan,
        DarkGray,

        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        White,
        Gray
    };

    /// Basic platform-specific debug functions
    struct BasicPlatformDebug
    {
        static std::string FormatAssertionFailedMessage(const char* Message,
            const char* Function, // type of __FUNCTION__
            const char* File,     // type of __FILE__
            int         Line);
        static std::string FormatDebugMessage(DEBUG_MESSAGE_SEVERITY Severity,
            const char* Message,
            const char* Function, // type of __FUNCTION__
            const char* File,     // type of __FILE__
            int                    Line);

        static const char* TextColorToTextColorCode(DEBUG_MESSAGE_SEVERITY Severity, TextColor Color);

        static bool ColoredTextSupported()
        {
            return true;
        }

        static void SetBreakOnError(bool BreakOnError);
        static bool GetBreakOnError();
    };

    // platform-specific debug函数的前向声明
    void DebugAssertionFailed(const char* Message, const char* Function, const char* File, int Line);
}