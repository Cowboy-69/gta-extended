#pragma once

#include "ClumpModelInfo.h"
#ifdef NEW_VEHICLE_LOADER
#include "HandlingMgr.h"
#endif

enum {
#ifdef IMPROVED_VEHICLES // More materials and colors
	NUM_FIRST_MATERIALS = 50,
	NUM_SECOND_MATERIALS = 50,
	NUM_THIRD_MATERIALS = 50,
	NUM_FOURTH_MATERIALS = 50,
	NUM_VEHICLE_COLOURS = 16,
#else
	NUM_FIRST_MATERIALS = 24,
	NUM_SECOND_MATERIALS = 20,
	NUM_VEHICLE_COLOURS = 8,
#endif
};

enum {
	ATOMIC_FLAG_NONE	= 0x0,
	ATOMIC_FLAG_OK		= 0x1,
	ATOMIC_FLAG_DAM		= 0x2,
	ATOMIC_FLAG_LEFT	= 0x4,
	ATOMIC_FLAG_RIGHT	= 0x8,
	ATOMIC_FLAG_FRONT	= 0x10,
	ATOMIC_FLAG_REAR	= 0x20,
	ATOMIC_FLAG_DRAWLAST	= 0x40,
	ATOMIC_FLAG_WINDSCREEN	= 0x80,
	ATOMIC_FLAG_ANGLECULL	= 0x100,
	ATOMIC_FLAG_REARDOOR	= 0x200,
	ATOMIC_FLAG_FRONTDOOR	= 0x400,
	ATOMIC_FLAG_NOCULL	= 0x800,
#ifdef IMPROVED_VEHICLES_2
	ATOMIC_FLAG_DOORWINDOW	= 0x1000,
#endif
};

enum eVehicleType {
	VEHICLE_TYPE_CAR,
	VEHICLE_TYPE_BOAT,
	VEHICLE_TYPE_TRAIN,
	VEHICLE_TYPE_HELI,
	VEHICLE_TYPE_PLANE,
	VEHICLE_TYPE_BIKE,
	NUM_VEHICLE_TYPES
};

enum eCarPositions
{
	CAR_POS_HEADLIGHTS,
	CAR_POS_TAILLIGHTS,
	CAR_POS_FRONTSEAT,
	CAR_POS_BACKSEAT,
	CAR_POS_EXHAUST,
#ifdef IMPROVED_VEHICLES_2 // dummy positions
	CAR_POS_HEADLIGHTS_2,
	CAR_POS_TAILLIGHTS_2,
	CAR_POS_REVERSINGLIGHTS,
	CAR_POS_REVERSINGLIGHTS_2,
	CAR_POS_BRAKELIGHTS,
	CAR_POS_BRAKELIGHTS_2,
	CAR_POS_INDICATORS_F,
	CAR_POS_INDICATORS_R,
	CAR_POS_INDICATORS_2_F,
	CAR_POS_INDICATORS_2_R,
	CAR_POS_ENGINE,
	CAR_POS_OVERHEAT,
	CAR_POS_OVERHEAT_2,
	CAR_POS_PETROLCAP,
	NUM_VEHICLE_POSITIONS,
#endif
};

enum eBoatPositions
{
	BOAT_POS_FRONTSEAT
};

enum eTrainPositions
{
	TRAIN_POS_LIGHT_FRONT,
	TRAIN_POS_LIGHT_REAR,
	TRAIN_POS_LEFT_ENTRY,
	TRAIN_POS_MID_ENTRY,
	TRAIN_POS_RIGHT_ENTRY
};

enum ePlanePositions
{
	PLANE_POS_LIGHT_LEFT,
	PLANE_POS_LIGHT_RIGHT,
	PLANE_POS_LIGHT_TAIL,
};

#ifndef IMPROVED_VEHICLES_2
enum {
	NUM_VEHICLE_POSITIONS = 5
};
#endif

#ifdef NEW_VEHICLE_LOADER
struct tAnotherVehicleSampleData {
	int16 m_nAccelerationSampleIndex;
	uint8 m_nBank;
	int16 m_nHornSample;
	int32 m_nHornFrequency;
	uint8 m_nSirenOrAlarmSample;
	int32 m_nSirenOrAlarmFrequency;
	uint8 m_bDoorType;
};

struct tVehicleShadowSettingData {
	float fHeightMultiplier;
	float fWidthMultiplier;
	float fSizeMultiplier;
};

struct tBoatSoundSettingData {
	uint8 volume;
	float volumeModificator;
	uint32 frequency;
	float frequencyModificator;
	bool8 bEngineType;
};
#endif

#ifdef IMPROVED_VEHICLES_2
struct tNewLightsData {
	bool bBumperHeadlights;
	bool bBumperForwardIndicators1;
	bool bBumperForwardIndicators2;
	bool bWingHeadlights;
	bool bWingIndicators1;
	bool bWingIndicators2;
	bool bBumperRearIndicators1;
	bool bBumperRearIndicators2;
	bool bBumperTaillights;
	bool bBumperReversinglights;
	bool bBumperBrakelights;
};
#endif

class CVehicleModelInfo : public CClumpModelInfo
{
public:
	uint8 m_lastColour1;
	uint8 m_lastColour2;
#ifdef IMPROVED_VEHICLES // More colors
	uint8 m_lastColour3;
	uint8 m_lastColour4;
	bool bHasManyColors;
#endif
	char m_gameName[10];
	int32 m_vehicleType;
	float m_wheelScale;
	union {
		int16 m_wheelId;
		int16 m_planeLodId;
	};
	int16 m_handlingId;
	int8 m_numDoors;
	int8 m_vehicleClass;
	int8 m_level;
	int8 m_numComps;
	int16 m_frequency;
	CVector m_positions[NUM_VEHICLE_POSITIONS];
	uint32 m_compRules;
	float m_bikeSteerAngle;
	RpMaterial *m_materials1[NUM_FIRST_MATERIALS];
	RpMaterial *m_materials2[NUM_SECOND_MATERIALS];
	uint8 m_colours1[NUM_VEHICLE_COLOURS];
	uint8 m_colours2[NUM_VEHICLE_COLOURS];
#ifdef IMPROVED_VEHICLES // More colors
	RpMaterial* m_materials3[NUM_THIRD_MATERIALS];
	RpMaterial* m_materials4[NUM_FOURTH_MATERIALS];
	uint8 m_colours3[NUM_VEHICLE_COLOURS];
	uint8 m_colours4[NUM_VEHICLE_COLOURS];
	uint8 m_currentColour3;
	uint8 m_currentColour4;
#endif
	uint8 m_numColours;
	uint8 m_lastColorVariation;
	uint8 m_currentColour1;
	uint8 m_currentColour2;
	RpAtomic *m_comps[6];
	// This is stupid, CClumpModelInfo already has it!
	union {
		int32 m_animFileIndex;
		char *m_animFileName;
	};

#ifdef IMPROVED_VEHICLES_2
	RwTexture* lightsOffTexture;
	RwTexture* lightsOnTexture;
	bool bNewLights;
	tNewLightsData newLightsData;
#endif

#ifdef VEHICLE_MODS
	CRGBA m_nDefaultWindowMaterialColor;
#endif

#ifdef NEW_VEHICLE_LOADER
	tAnotherVehicleSampleData vehicleSampleData;
	uint16 m_policeRadioIndex;
	tHandlingData handlingData;
	tBikeHandlingData bikeHandlingData;
	tBoatHandlingData boatHandlingData;
	tFlyingHandlingData flyingHandlingData;
	char m_anotherAnimFileName[6];
	tVehicleShadowSettingData vehicleShadowData;
	tBoatSoundSettingData boatSoundData;
	wchar m_fullGameName[32];
#endif

	static int8 ms_compsToUse[2];
	static int8 ms_compsUsed[2];
	static RwRGBA ms_vehicleColourTable[256];
	static RwTexture *ms_colourTextureTable[256];
	static RwObjectNameIdAssocation *ms_vehicleDescs[NUM_VEHICLE_TYPES];

	CVehicleModelInfo(void);
	void DeleteRwObject(void);
	RwObject *CreateInstance(void);
	void SetClump(RpClump *);
	void SetAnimFile(const char *file);
	void ConvertAnimFileIndex(void);
	int GetAnimFileIndex(void) { return m_animFileIndex; }

	static RwFrame *CollapseFramesCB(RwFrame *frame, void *data);
	static RwObject *MoveObjectsCB(RwObject *object, void *data);
	static RpAtomic *HideDamagedAtomicCB(RpAtomic *atomic, void *data);
	static RpAtomic *HideAllComponentsAtomicCB(RpAtomic *atomic, void *data);
	static RpMaterial *HasAlphaMaterialCB(RpMaterial *material, void *data);

	static RpAtomic *SetAtomicRendererCB(RpAtomic *atomic, void *data);
	static RpAtomic *SetAtomicRendererCB_BigVehicle(RpAtomic *atomic, void *data);
	static RpAtomic *SetAtomicRendererCB_Train(RpAtomic *atomic, void *data);
	static RpAtomic *SetAtomicRendererCB_Boat(RpAtomic *atomic, void *data);
	static RpAtomic *SetAtomicRendererCB_Heli(RpAtomic *atomic, void *data);
	static RpAtomic *SetAtomicRendererCB_RealHeli(RpAtomic *atomic, void *data);
	void SetAtomicRenderCallbacks(void);

	static RwObject *SetAtomicFlagCB(RwObject *object, void *data);
	static RwObject *ClearAtomicFlagCB(RwObject *atomic, void *data);
	void SetVehicleComponentFlags(RwFrame *frame, uint32 flags);
	void PreprocessHierarchy(void);
	void GetWheelPosn(int32 n, CVector &pos);
	const CVector &GetFrontSeatPosn(void) { return m_vehicleType == VEHICLE_TYPE_BOAT ? m_positions[BOAT_POS_FRONTSEAT] : m_positions[CAR_POS_FRONTSEAT]; }

	int32 ChooseComponent(void);
	int32 ChooseSecondComponent(void);

	static RpMaterial *GetEditableMaterialListCB(RpMaterial *material, void *data);
	static RpAtomic *GetEditableMaterialListCB(RpAtomic *atomic, void *data);
	void FindEditableMaterialList(void);
#ifdef IMPROVED_VEHICLES // More colors
	void SetVehicleColour(uint8 c1, uint8 c2, uint8 c3 = 0, uint8 c4 = 0);
	void ChooseVehicleColour(uint8& col1, uint8& col2, uint8& col3, uint8& col4);
	void AvoidSameVehicleColour(uint8* col1, uint8* col2, uint8* col3, uint8* col4);
#else
	void SetVehicleColour(uint8 c1, uint8 c2);
	void ChooseVehicleColour(uint8& col1, uint8& col2);
	void AvoidSameVehicleColour(uint8* col1, uint8* col2);
#endif
	static void LoadVehicleColours(void);
	static void DeleteVehicleColourTextures(void);

	static RpAtomic *SetEnvironmentMapCB(RpAtomic *atomic, void *data);
	static RpMaterial *SetDefaultEnvironmentMapCB(RpMaterial *material, void *data);
	static RpMaterial *GetMatFXEffectMaterialCB(RpMaterial *material, void *data);
	void SetEnvironmentMap(void);
	static void LoadEnvironmentMaps(void);
	static void ShutdownEnvironmentMaps(void);

	static int GetMaximumNumberOfPassengersFromNumberOfDoors(int id);
	static void SetComponentsToUse(int8 c1, int8 c2) { ms_compsToUse[0] = c1; ms_compsToUse[1] = c2; }
};

extern bool gbBlackCars;
extern bool gbPinkCars;
