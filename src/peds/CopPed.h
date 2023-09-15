#pragma once
#include "Ped.h"
#ifdef IMPROVED_TECH_PART // wanted system
#include "PlayerPed.h"
#endif

enum eCopType
{
	COP_STREET = 0,
	COP_FBI = 1,
	COP_SWAT = 2,
	COP_HELI_SWAT = 3,
	COP_ARMY = 4,
	COP_MIAMIVICE = 5
};

class CCopPed : public CPed
{
public:
	CVehicle* m_nRoadblockVeh;
	float m_fDistanceToTarget;
	bool m_bIsInPursuit;
	bool m_bIsDisabledCop;
	int8 field_5FE;
	bool m_bBeatingSuspect;
	bool m_bStopAndShootDisabledZone;
	bool m_bDragsPlayerFromCar;
	bool m_bZoneDisabled;
	float m_fAbseilPos;
	eCopType m_nCopType;
	bool m_bThrowsSpikeTrap;
	CEntity *m_pRopeEntity;	// CHeli or 1
	uintptr m_nRopeID;
	uint32 m_nHassleTimer;
	uint32 field_61C;
	class CStinger *m_pStinger;
	int32 field_624;
	int8 field_628;

#ifdef IMPROVED_TECH_PART // wanted system
	CVector m_vecLastPosPlayerBeforeHiding;
	CVector m_vecPotentialPlayerSearchCoordinates;
	bool m_bLookingForPlayer;
	bool m_bMovesToLastPlayerPosition;

	bool m_bThrowingTeargas;
	bool m_bUsedTeargas;
#endif

	CCopPed(eCopType, int32 modifier = 0);
	~CCopPed();

	void ClearPursuit(void);
	void ProcessControl(void);
	void SetArrestPlayer(CPed*);
	void SetPursuit(bool);
	void ArrestPlayer(void);
	void ScanForCrimes(void);
	void CopAI(void);
	void ProcessHeliSwat(void);
	void ProcessStingerCop(void);
#ifdef IMPROVED_TECH_PART // wanted system
	void ProcessSearchPlayer(CPlayerPed* playerPed);
	void FindPotentialPlayerPos(void);
	bool CheckIfPositionIsUnderWater(CVector checkPos, bool bCheckMiddlePos);
#endif
};

VALIDATE_SIZE(CCopPed, 0x62C);
