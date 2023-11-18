#include "common.h"

#include "ModelInfo.h"
#include "AnimManager.h"
#include "VisibilityPlugins.h"

void
CWeaponModelInfo::SetAnimFile(const char *file)
{
	if(strcasecmp(file, "null") == 0)
		return;

	m_animFileName = new char[strlen(file)+1];
	strcpy(m_animFileName, file);
}

void
CWeaponModelInfo::ConvertAnimFileIndex(void)
{
	if(m_animFileIndex != -1){
		// we have a string pointer in that union
		int32 index = CAnimManager::GetAnimationBlockIndex(m_animFileName);
		delete[] m_animFileName;
		m_animFileIndex = index;
	}

#ifdef EX_IMPROVED_WEAPONS
	if (m_animForWeaponFileIndex != -1) {
		int32 index = CAnimManager::GetAnimationBlockIndex(m_animForWeaponFileName);
		delete[] m_animForWeaponFileName;
		m_animForWeaponFileIndex = index;
	}
#endif
}

#ifdef EX_IMPROVED_WEAPONS
void
CWeaponModelInfo::SetAnimForWeaponFile(const char* file)
{
	if (strcasecmp(file, "null") == 0)
		return;

	m_animForWeaponFileName = new char[strlen(file) + 1];
	strcpy(m_animForWeaponFileName, file);
	m_animForWeaponFileIndex = 0;
}
#endif

#ifdef EX_CLUMP_WEAPON_MODELS
void
CWeaponModelInfo::DeleteRwObject(void)
{
	CClumpModelInfo::DeleteRwObject();
}

void
CWeaponModelInfo::SetClump(RpClump *clump)
{
	CClumpModelInfo::SetClump(clump);
	RpClumpForAllAtomics(m_clump, SetAtomicRendererCB, (void*)CVisibilityPlugins::RenderWeaponCB);
}
#endif

void
CWeaponModelInfo::Init(void)
{
#ifndef EX_CLUMP_WEAPON_MODELS
	CSimpleModelInfo::Init();
#endif
	SetWeaponInfo(0);
}

void
CWeaponModelInfo::SetWeaponInfo(int32 weaponId)
{
#ifdef EX_CLUMP_WEAPON_MODELS
	weaponType = weaponId;
#else
	m_atomics[2] = (RpAtomic*)weaponId;
#endif
}

eWeaponType
CWeaponModelInfo::GetWeaponInfo(void)
{
#ifdef EX_CLUMP_WEAPON_MODELS
	return (eWeaponType)weaponType;
#else
	return (eWeaponType)(uintptr)m_atomics[2];
#endif
}

#ifndef EX_CLUMP_WEAPON_MODELS
void
CWeaponModelInfo::SetAtomic(int n, RpAtomic *atomic)
{
	CSimpleModelInfo::SetAtomic(n, atomic);
	CVisibilityPlugins::SetAtomicRenderCallback(atomic, CVisibilityPlugins::RenderWeaponCB);
}
#endif
