#pragma once

#include <cstdint>

#include "Config.h"

namespace Hikali
{
	class AppBase
	{
	public:
		virtual ~AppBase() {}

		virtual void ProcessCommandLine(RendererConfig& Config) = 0;
		virtual const char* GetAppTitle() const = 0;
		virtual void Update(double CurrTime, double ElapsedTime) = 0;
		virtual void Render() = 0;
		virtual void Present() = 0;
		virtual void WindowResize(int16_t Width, int16_t Height) = 0;
		virtual bool IsReady() const
		{
			return false;
		}
	};
}