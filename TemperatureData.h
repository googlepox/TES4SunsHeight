#pragma once
#include <Defs.h>

namespace SunsHeight
{
	enum class HeatLevel : int
	{
		None = 0,
		Warm = 1,
		Hot = 2,
		Sweltering = 3,
		Hyperthermia = 4
	};

	enum class ColdLevel : int
	{
		None = 0,
		Chilly = 1,
		Cold = 2,
		Freezing = 3,
		Hypothermia
	};

	enum class WetLevel : int
	{
		Dry = 0,
		Damp = 1,
		Wet = 2,
		Soaked = 3
	};

	struct TierSpell
	{
		const char* modName;
		UInt32      localID;
	};

	static SpellItem* g_heatSpellForms[5] = {};
	static SpellItem* g_coldSpellForms[5] = {};
	static SpellItem* g_wetSpellForms[4] = {};

	class TemperatureData
	{
	public:
		float bodyTemp = 100.0f;
		float ambientTemp = 100.0f;
		float ambientTargetTemp = 100.0f;
		float clothingWarmth = 0.0f;
		float clothingCoverage = 0.0f;
		float wetness = 0.0f;
		float fireHeat = 0.0f;
		bool isRaining = false;
		bool isSnowing = false;
		bool isSandstorm = false;
		bool isInside = false;
		bool nearFire = false;
		UInt32 weatherID = 0;
		float lastGameHour = -1.0f;
		HeatLevel lastHeatLevel;
		ColdLevel lastColdLevel;
		WetLevel lastWetLevel;
		HeatLevel heatLevel;
		ColdLevel coldLevel;
		WetLevel wetLevel;

		struct Input
		{
			float bodyTemp;
			float ambientTemp;
			float ambientTargetTemp;
			float clothingWarmth;
			float clothingCoverage;
			float wetness;
			float fireHeat;
			bool isRaining;
			bool isSnowing;
			bool isSandstorm;
			bool isInside;
			bool nearFire;
			UInt32 weatherID;
			float lastGameHour;
			HeatLevel lastHeatLevel;
			ColdLevel lastColdLevel;
			WetLevel lastWetLevel;
			HeatLevel heatLevel;
			ColdLevel coldLevel;
			WetLevel wetLevel;
		};

		TemperatureData() = default;
		explicit TemperatureData(const Input& a_input);

	};
}