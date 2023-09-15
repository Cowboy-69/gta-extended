#pragma once

#include "AutoPilot.h"

class CVehicle;

class CCarAI
{
public:
	static float FindSwitchDistanceClose(CVehicle*);
	static float FindSwitchDistanceFarNormalVehicle(CVehicle*);
	static float FindSwitchDistanceFar(CVehicle*);
	static void BackToCruisingIfNoWantedLevel(CVehicle*);
#ifdef IMPROVED_TECH_PART // wanted system
	static void BackToCruisingIfPlayerLost(CVehicle*);
#endif
	static void UpdateCarAI(CVehicle*);
	static void CarHasReasonToStop(CVehicle*);
	static float GetCarToGoToCoors(CVehicle*, CVector*);
	static float GetCarToParkAtCoors(CVehicle*, CVector*);
	static void AddPoliceCarOccupants(CVehicle*);
	static void AddAmbulanceOccupants(CVehicle*);
	static void AddFiretruckOccupants(CVehicle*);
	static void TellOccupantsToLeaveCar(CVehicle*);
	static void TellOccupantsToFleeCar(CVehicle*);
	static void TellCarToRamOtherCar(CVehicle*, CVehicle*);
	static void TellCarToBlockOtherCar(CVehicle*, CVehicle*);
#ifdef WANTED_PATHS // wanted system
	static void TellCarToSeekWantedPaths(CVehicle*);
	static void TellOccupantsToLeaveCarImmediately(CVehicle*);
#endif
	static uint8 FindPoliceCarMissionForWantedLevel();
	static uint8 FindPoliceBoatMissionForWantedLevel();
	static int32 FindPoliceCarSpeedForWantedLevel(CVehicle*);
	static void MellowOutChaseSpeed(CVehicle*);
	static void MellowOutChaseSpeedBoat(CVehicle*);
	static void MakeWayForCarWithSiren(CVehicle *veh);
};
