#pragma once

#ifdef IMPROVED_TECH_PART
#define DEFAULT_MAX_NUMBER_OF_PEDS 15.0f
#define DEFAULT_MAX_NUMBER_OF_PEDS_INTERIOR 15.0f
#define DEFAULT_MAX_NUMBER_OF_CARS 20.0f
#else
#define DEFAULT_MAX_NUMBER_OF_PEDS 25.0f
#define DEFAULT_MAX_NUMBER_OF_PEDS_INTERIOR 40.0f
#define DEFAULT_MAX_NUMBER_OF_CARS 12.0f
#endif

class CIniFile
{
public:
	static void LoadIniFile();

	static float PedNumberMultiplier;
	static float CarNumberMultiplier;
};
