#include "TemperatureData.h"

namespace SunsHeight
{
	TemperatureData::TemperatureData(const Input& a_input) :
		bodyTemp(a_input.bodyTemp),
		ambientTemp(a_input.ambientTemp),
		ambientTargetTemp(a_input.ambientTargetTemp),
		clothingWarmth(a_input.clothingWarmth),
		clothingCoverage(a_input.clothingCoverage),
		wetness(a_input.wetness),
		fireHeat(a_input.fireHeat),
		isRaining(a_input.isRaining),
		isSnowing(a_input.isSnowing),
		isSandstorm(a_input.isSandstorm),
		isInside(a_input.isInside),
		nearFire(a_input.nearFire),
		weatherID(a_input.weatherID),
		lastGameHour(a_input.lastGameHour),
		lastHeatLevel(a_input.lastHeatLevel),
		lastColdLevel(a_input.lastColdLevel),
		lastWetLevel(a_input.lastWetLevel),
		heatLevel(a_input.heatLevel),
		coldLevel(a_input.coldLevel),
		wetLevel(a_input.wetLevel)
	{

	}
}