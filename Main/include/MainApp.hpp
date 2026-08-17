#pragma once

#include <string>

#include "NativeAppBase.hpp"

namespace Hikali
{
	class MainApp : public NativeAppBase
	{
	public:
		MainApp();
		~MainApp();

		virtual void ProcessCommandLine(RendererConfig& Config) override final;
		virtual const char* GetAppTitle() const override final { return m_AppTitle.c_str(); }

		virtual void Update(double CurrTime, double ElapsedTime) override;
		virtual void WindowResize(int16_t Width, int16_t Height) override;
		virtual void Render() override;
		virtual void Present() override;

	protected:
		std::string m_AppTitle;
	};
}