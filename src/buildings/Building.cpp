#include "common.h"

#include "Building.h"
#include "Streaming.h"
#include "Pools.h"
#ifdef IMPROVED_TECH_PART
#include "Lights.h"
#include "PointLights.h"
#endif

void *CBuilding::operator new(size_t sz) throw() { return CPools::GetBuildingPool()->New();  }
void CBuilding::operator delete(void *p, size_t sz) throw() { CPools::GetBuildingPool()->Delete((CBuilding*)p); }

void
CBuilding::ReplaceWithNewModel(int32 id)
{
	DeleteRwObject();

	if (CModelInfo::GetModelInfo(m_modelIndex)->GetNumRefs() == 0)
		CStreaming::RemoveModel(m_modelIndex);
	m_modelIndex = id;

	if(bIsBIGBuilding)
		if(m_level == LEVEL_GENERIC || m_level == CGame::currLevel)
			CStreaming::RequestModel(id, STREAMFLAGS_DONT_REMOVE);
}

#ifdef IMPROVED_TECH_PART
/*bool CBuilding::SetupLighting(void)
{
	ActivateDirectional();
	SetAmbientColoursForPedsCarsAndObjects();
	//SetAmbientAndDirectionalColours(0.5f);
	return true;
}

void CBuilding::RemoveLighting(bool reset)
{
	if (reset) {
		SetAmbientColours();
		DeActivateDirectional();
	}
}*/
#endif

bool
IsBuildingPointerValid(CBuilding* pBuilding)
{
	if (!pBuilding)
		return false;
	if (pBuilding->GetIsATreadable()) {
		int index = CPools::GetTreadablePool()->GetJustIndex_NoFreeAssert((CTreadable*)pBuilding);
#ifdef FIX_BUGS
		return index >= 0 && index < CPools::GetTreadablePool()->GetSize();
#else
		return index >= 0 && index <= CPools::GetTreadablePool()->GetSize();
#endif
	} else {
		int index = CPools::GetBuildingPool()->GetJustIndex_NoFreeAssert(pBuilding);
#ifdef FIX_BUGS
		return index >= 0 && index < CPools::GetBuildingPool()->GetSize();
#else
		return index >= 0 && index <= CPools::GetBuildingPool()->GetSize();
#endif
	}
}
