#pragma once

class CVehicle;

enum {
	// NB: not all values are allowed, check the code
#ifdef SQUEEZE_PERFORMANCE
	NUM_RUBBISH_SHEETS = 32
#else
	NUM_RUBBISH_SHEETS = 64
#endif
};

#ifdef EX_UNLOCK_MISSION_NEWS_STORY
enum eNewspaperType {
	NEWSPAPER_INTRO, // In the beginning...
	NEWSPAPER_LAWYER4, // law_4, Riot
	NEWSPAPER_GENERAL1, // col_1, Treacherous Swine
	NEWSPAPER_GENERAL2, // col_2, Mall Shootout
	NEWSPAPER_GENERAL3, // col_3, Guardian Angels
	NEWSPAPER_SERG1, // tex_1, Four Iron
	NEWSPAPER_SERG3, // tex_3, Demolition Man
	NEWSPAPER_BARON2, // cok_2, Phnom Penh '86
	NEWSPAPER_GENERAL4, // col_4, Sir, Yes Sir!
	NEWSPAPER_SERG2, // tex_2, Two Bit Hit
	NEWSPAPER_BARON3, // cok_3, The Fastest Boat
	NEWSPAPER_BARON4, // cok_4, Supply & Demand
	NEWSPAPER_BARON5, // ass_1, Rub Out
	NEWSPAPER_PROT1, // bud_1, Shakedown
	NEWSPAPER_PROT3, // bud_3, Cop Land
	NEWSPAPER_FINALE, // fin_1, Keep your Friends Close...
	NEWSPAPER_BANKJ4, // bank_4, The Job
	NEWSPAPER_COUNT2, // cnt_2, Hit the Courier
	NEWSPAPER_PORN2, // porn_2, Dildo Dodo
	NEWSPAPER_GENERAL5, // col_5, All Hands On Deck!
	NEWSPAPER_CUBAN4, // cub_4, Trojan Voodoo
	NEWSPAPER_PEACE,
	TOTAL_NEWSPAPERS,
};
#endif

class COneSheet
{
public:
	CVector m_basePos;
	CVector m_animatedPos;
	float m_targetZ;
	int8 m_state;
	int8 m_animationType;
	uint32 m_moveStart;
	uint32 m_moveDuration;
	float m_animHeight;
	float m_xDist;
	float m_yDist;
	float m_angle;
	bool m_isVisible;
	bool m_targetIsVisible;
	COneSheet *m_next;
	COneSheet *m_prev;

	void AddToList(COneSheet *list);
	void RemoveFromList(void);
};

class CRubbish
{
	static bool bRubbishInvisible;
	static int RubbishVisibility;
	static COneSheet aSheets[NUM_RUBBISH_SHEETS];
	static COneSheet StartEmptyList;
	static COneSheet EndEmptyList;
	static COneSheet StartStaticsList;
	static COneSheet EndStaticsList;
	static COneSheet StartMoversList;
	static COneSheet EndMoversList;
public:
#ifdef EX_UNLOCK_MISSION_NEWS_STORY
	static int CurrentNewspaperTextureID; // eNewspaperType
	static int NextNewspaperTextureID; // eNewspaperType
	static int NewspaperChangeTime;
	static RwTexture *NewspaperTextures[TOTAL_NEWSPAPERS];
	static void UnlockMissionNewsStory(int newsID);
#endif
	static void Render(void);
	static void StirUp(CVehicle *veh);	// CAutomobile on PS2
	static void Update(void);
	static void SetVisibility(bool visible);
	static void Init(void);
	static void Shutdown(void);
};

extern RwTexture *gpRubbishTexture[4];
