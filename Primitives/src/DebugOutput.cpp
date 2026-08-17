#include "DebugOutput.h"

namespace Hikali
{
    void SetDebugMessageCallback(DebugMessageCallbackType DbgMessageCallback)
    {
        DebugMessageCallback = DbgMessageCallback;
    }
}