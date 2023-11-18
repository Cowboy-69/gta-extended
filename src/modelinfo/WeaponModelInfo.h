#pragma once

#include "SimpleModelInfo.h"
#include "WeaponType.h"

#ifdef EX_CLUMP_WEAPON_MODELS
class CWeaponModelInfo : public CClumpModelInfo
#else
class CWeaponModelInfo : public CSimpleModelInfo
#endif
{
#ifdef EX_CLUMP_WEAPON_MODELS
	int32 weaponType;
#endif
	union {
		int32 m_animFileIndex;
		char *m_animFileName;
	};

#ifdef EX_IMPROVED_WEAPONS
	int32 m_animForWeaponFileIndex;
	char* m_animForWeaponFileName;
#endif

public:
#ifdef EX_CLUMP_WEAPON_MODELS
	CWeaponModelInfo(void) : CClumpModelInfo(MITYPE_WEAPON) { 
		m_animFileIndex = -1;
#ifdef EX_IMPROVED_WEAPONS
		m_animForWeaponFileIndex = -1;
#endif
	}
#else
	CWeaponModelInfo(void) : CSimpleModelInfo(MITYPE_WEAPON) { m_animFileIndex = -1; }
#endif

	virtual void SetAnimFile(const char *file);
	virtual void ConvertAnimFileIndex(void);
	virtual int GetAnimFileIndex(void) { return m_animFileIndex; }
#ifdef EX_IMPROVED_WEAPONS
	virtual void SetAnimForWeaponFile(const char* file);
	virtual int GetAnimForWeaponFileIndex(void) { return m_animForWeaponFileIndex; }
#endif
#ifdef EX_CLUMP_WEAPON_MODELS
	void DeleteRwObject(void);
	void SetClump(RpClump*);
#else
	virtual void SetAtomic(int n, RpAtomic *atomic);
#endif
	
	void Init(void);
	void SetWeaponInfo(int32 weaponId);
	eWeaponType GetWeaponInfo(void);
};
