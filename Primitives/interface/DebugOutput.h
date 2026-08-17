/*  参考DiligentEngine:https://github.com/DiligentGraphics/DiligentEngine  */
#pragma once

namespace Hikali
{
    /// 描述debug信息严重程度
    enum DEBUG_MESSAGE_SEVERITY
    {
        /// Information 
        DEBUG_MESSAGE_SEVERITY_INFO = 0,

        /// Warning
        DEBUG_MESSAGE_SEVERITY_WARNING,

        /// Error - 可能修复
        DEBUG_MESSAGE_SEVERITY_ERROR,

        /// Fatal error - 不可修复
        DEBUG_MESSAGE_SEVERITY_FATAL_ERROR
    };

    typedef void(* DebugMessageCallbackType)(DEBUG_MESSAGE_SEVERITY Severity,
                                            const char*             Message,
                                            const char*             Function,
                                            const char*             File,
                                            int                     Line);
    extern DebugMessageCallbackType DebugMessageCallback;

    void SetDebugMessageCallback(DebugMessageCallbackType DbgMessageCallback);
}
