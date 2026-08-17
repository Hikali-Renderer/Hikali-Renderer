#pragma once

#if PLATFORM_WIN32

#include "Win32/Win32AppBase.hpp"
namespace Hikali
{
	using NativeAppBase = Win32AppBase;
}

#endif