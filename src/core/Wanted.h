#pragma once

#include "Crime.h"

#ifdef IMPROVED_TECH_PART // wanted system
#define TIME_DELAY_BEFORE_BEING_SEEN 350
#define TIME_AUTOMATIC_UPDATED_LAST_SEEN_POS_PLAYER 25000
#endif

class CEntity;
class CCopPed;

class CWanted
{
public:
	int32 m_nChaos;
	int32 m_nMinChaos;
	int32 m_nLastUpdateTime;
	uint32 m_nLastWantedLevelChange;
	uint32 m_nLastTimeSuspended;
	float m_fCrimeSensitivity;
	uint8 m_CurrentCops;
	uint8 m_MaxCops;
	uint8 m_MaximumLawEnforcerVehicles;
	uint8 m_CopsBeatingSuspect;
	int16 m_RoadblockDensity;
	uint8 m_bIgnoredByCops : 1;
	uint8 m_bIgnoredByEveryone : 1;
	uint8 m_bSwatRequired : 1;
	uint8 m_bFbiRequired : 1;
	uint8 m_bArmyRequired : 1;
	int32 m_nWantedLevel;
	int32 m_nMinWantedLevel;
	CCrimeBeingQd m_aCrimes[16];
	CCopPed *m_pCops[10];

#ifdef IMPROVED_TECH_PART // wanted system
	uint32 m_nLastTimeSeenPlayer;
	CVector m_vecLastSeenPosPlayer;
	CVehicle* m_vLastSeenPlayerVehicle;
	uint32 m_nTimeDelayBeforeBeingSeen;
	uint32 m_nTimeAutomaticUpdatedPosPlayer;

	bool m_bNextReportIsLastSeen;
	bool m_bSearchPlayerRandomly;
#endif

	static int32 MaximumWantedLevel;
	static int32 nMaximumWantedLevel;

public:
	void Initialise();
	bool AreMiamiViceRequired();
	bool AreSwatRequired();
	bool AreFbiRequired();
	bool AreArmyRequired();
	int32 NumOfHelisRequired();
	void SetWantedLevel(int32);
	void SetWantedLevelNoDrop(int32 level);
	int32 GetWantedLevel() { return m_nWantedLevel; }
	void CheatWantedLevel(int32 level);
	void RegisterCrime(eCrimeType type, const CVector &coors, uint32 id, bool policeDoesntCare);
	void RegisterCrime_Immediately(eCrimeType type, const CVector &coors, uint32 id, bool policeDoesntCare);
	void ClearQdCrimes();
	bool AddCrimeToQ(eCrimeType type, int32 id, const CVector &pos, bool reported, bool policeDoesntCare);
	void ReportCrimeNow(eCrimeType type, const CVector &coors, bool policeDoesntCare);
	void UpdateWantedLevel();
	void Reset();
	void ResetPolicePursuit();
	void UpdateCrimesQ();
	void Update();

	void Suspend();

	bool IsIgnored(void) { return m_bIgnoredByCops || m_bIgnoredByEveryone; }

#ifdef IMPROVED_TECH_PART // wanted system
	bool IsPlayerLost() const;
	bool IsPlayerHides() const;
#endif

	static int32 WorkOutPolicePresence(CVector posn, float radius);
	static void SetMaximumWantedLevel(int32 level);
};

VALIDATE_SIZE(CWanted, 0x204);
