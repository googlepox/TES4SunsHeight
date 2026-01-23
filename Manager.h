#pragma once
#include "Defs.h"
#include <TemperatureData.h>

namespace SunsHeight
{
	constexpr UInt32 kSerializationVersion = 1;
	constexpr UInt32 kSerializationSignature = 'SUNH';

	static const char* kSunHUDMenu = "SunHeightHUD";


	struct SaveData
	{
		float bodyTemp;
		float wetness;
		float lastGameHour;

		UInt8 heatLevel;
		UInt8 coldLevel;
		UInt8 wetLevel;
	};

	struct OverrideKey
	{
		std::string modName;
		UInt32      localID;
	};

	struct Override
	{
		float tempOffset = 0.0f;
		float wetnessRate = 0.0f;
		float warmth = 0.0f;
		float coverage = 0.0f;
	};

	struct OverrideEntry
	{
		OverrideKey key;
		Override data;
	};

	static std::unordered_map<UInt32, Override> g_weatherOverrides;
	static std::vector<OverrideEntry> g_weatherOverrideEntries;

	static std::unordered_map<UInt32, Override> g_itemOverrides;
	static std::vector<OverrideEntry> g_itemOverrideEntries;

	static std::unordered_map<UInt32, Override> g_regionOverrides;
	static std::vector<OverrideEntry> g_regionOverrideEntries;

	static std::unordered_map<UInt32, Override> g_worldOverrides;
	static std::vector<OverrideEntry> g_worldOverrideEntries;

	class Manager
	{
	public:

		static Manager* GetSingleton()
		{
			static Manager singleton;
			return std::addressof(singleton);
		}

		void OnSaveGame();
		void OnLoadGame();

		void ShowHUD();

		void Initialize();
		void UpdateTemperature();
		void CalculateAmbientTemp(TESObjectCELL* cell);
		void UpdateAmbientTemp(bool didTimeJump, float deltaSeconds);
		float GetSlotWeight(UInt32 slot);
		TESForm* ResolveForm(const char* modName, UInt32 localID);
		float EstimateWarmthFromArmor(TESForm* armor);
		float EstimateCoverageFromArmor(TESForm* armor);
		void CalculateEquipment();
		void ResolveWeatherOverrides();
		void ResolveRegionOverrides();
		void ResolveWorldspaceOverrides();
		std::vector<TESObjectREFR*> DetectNearbyFire();
		TESLandTexture* GetLandTexture();
		void UpdateExposureLevels();
		void ApplyTierSpell(Actor* player, int newTier, int& lastTier, SpellItem** spellTable);
		void ResolveTierSpells();
		void ApplyTemperatureTierSpells();
		void WriteToGlobals();
		void ResolveGlobals();

		bool realTimeMenusLoaded;
		float tickAccumulator;
		TESObjectCELL* cell;
		TemperatureData tempData;

	private:
		Manager()
		{
			realTimeMenusLoaded = false;
			tickAccumulator = 0.0f;

			tempData = TemperatureData::TemperatureData(TemperatureData::Input{
				50.0f,  // bodyTemp
				50.0f,  // ambientTemp
				50.0f,  // ambientTargetTemp
				0.0f,   // clothingWarmth
				0.0f,   // clothingCoverage
				0.0f,   // wetness
				0.0f,   // fireHeat
				false,  // isRaining
				false,  // isSnowing
				false,  // isSandstorm
				false,  // isInside
				false,  // neatFire
				0,      // weatherID
				-1.0f,  // lastGameHour
				HeatLevel::None,  // lastHeatLevel
				ColdLevel::None,  // lastColdLevel
				WetLevel::Dry,  // lastWetLevel
				HeatLevel::None,  // heatLevel
				ColdLevel::None,  // coldLevel
				WetLevel::Dry,  // wetLevel
				});
		}
		~Manager() = default;

		Manager(const Manager&) = delete;
		Manager(Manager&&) = delete;
		Manager& operator=(const Manager&) = delete;
		Manager& operator=(Manager&&) = delete;

	};
}