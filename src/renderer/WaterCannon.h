#pragma once

#define WATERCANNON_GRAVITY (0.009f)
#define WATERCANNON_LIFETIME (150)

class CWaterCannon
{
public:
	enum
	{
		NUM_SEGMENTPOINTS = 16,
	};
	
	int32 m_nId;
	int16 m_nCur;
	uint32 m_nTimeCreated;
	CVector m_avecPos[NUM_SEGMENTPOINTS];
	CVector m_avecVelocity[NUM_SEGMENTPOINTS];
	bool m_abUsed[NUM_SEGMENTPOINTS];
#ifdef EX_FIRE_TRUCK_WATER_CANNON // A player can get a wanted level when using the water cannon on peds
	bool m_bPlayerUsesCannon;
#endif
	
	void Init(void);
	void Update_OncePerFrame(int16 index);
#ifdef EX_FIRE_TRUCK_WATER_CANNON // A player can get a wanted level when using the water cannon on peds
	void Update_NewInput(CVector *pos, CVector *dir, bool playerUsesCannon);
#else
	void Update_NewInput(CVector *pos, CVector *dir);
#endif
	void Render(void);
	void PushPeds(void);
};

VALIDATE_SIZE(CWaterCannon, 412);

class CWaterCannons
{
public:
	static CWaterCannon aCannons[NUM_WATERCANNONS];
	
	static void Init(void);
#ifdef EX_FIRE_TRUCK_WATER_CANNON // A player can get a wanted level when using the water cannon on peds
	static void UpdateOne(uint32 id, CVector *pos, CVector *dir, bool playerUsesCannon);
#else
	static void UpdateOne(uint32 id, CVector *pos, CVector *dir);
#endif
	static void Update();
	static void Render(void);
};