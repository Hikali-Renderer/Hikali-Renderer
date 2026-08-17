#pragma once

#include <windows.h>

#include "AppBase.hpp"

namespace Hikali
{
	class Win32AppBase : public AppBase
	{
	public:
		virtual bool OnWindowCreated(HWND hWnd, LONG WindowWidth, LONG WindowHeight) = 0;
		virtual LRESULT HandleWin32Message(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
		{
			return 0;
		}
	};
}
