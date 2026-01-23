#include "Hooks.h"
#include "Defs.h"
#include "Manager.h"
#include <SafeWrite.cpp>
#include <GameMenus.h>

namespace SunsHeight
{

	namespace GetCurrentWorldspace
	{
		static LARGE_INTEGER freq;
		static LARGE_INTEGER lastTime;
		static bool initialized = false;

		void __fastcall GetCurrentWorldspaceHook(TES* tes, void* edx)
		{
			if (!initialized)
			{
				QueryPerformanceFrequency(&freq);
				QueryPerformanceCounter(&lastTime);
				initialized = true;
				return;
			}

			LARGE_INTEGER now;
			QueryPerformanceCounter(&now);

			float deltaSeconds =
				float(now.QuadPart - lastTime.QuadPart) / freq.QuadPart;

			lastTime = now;

			Manager::GetSingleton()->tickAccumulator += deltaSeconds;

			if (Manager::GetSingleton()->tickAccumulator >= 0.5f)
			{
				Manager::GetSingleton()->tickAccumulator -= 0.5f;
				Manager::GetSingleton()->UpdateTemperature();
			}

			ThisStdCall(0x43F270, tes);
		}

		void Install()
		{
			WriteRelCall(0x40D90E, (UInt32)&GetCurrentWorldspaceHook);
			_MESSAGE("Installed GetCurrentWorldspace hook");
		}
	}

	void Install()
	{
		_MESSAGE("-HOOKS-");
		GetCurrentWorldspace::Install();
		_MESSAGE("Installed all hooks");
	}
}