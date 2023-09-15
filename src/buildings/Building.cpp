#include "common.h"

#include "Building.h"
#include "Streaming.h"
#include "Pools.h"
#ifdef IMPROVED_TECH_PART
#include "Lights.h"
#include "PointLights.h"
#endif
#ifdef EX_DISTANT_LIGHTS
#include "Clock.h"
#include "Bridge.h"
#include "Weather.h"
#include "Coronas.h"
#include "Timecycle.h"
#include "Camera.h"
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

#ifdef EX_DISTANT_LIGHTS
void CBuilding::ProcessDistantLights()
{
	if (m_area != AREA_MAIN_MAP)
		return;

	if (m_modelIndex == 2292 || m_modelIndex == 392 || m_modelIndex == 401 || m_modelIndex == 391 || 
		m_modelIndex == 1337 || m_modelIndex == 1335 || m_modelIndex == 1334) {

		return;
	}

	uint32 flashTimer1, flashTimer2, flashTimer3;
	flashTimer1 = 0;
	flashTimer2 = 0;
	flashTimer3 = 0;

	int numEffects = CModelInfo::GetModelInfo(GetModelIndex())->GetNum2dEffects();
	for(int i = 0; i < numEffects; i++, flashTimer1 += 0x80, flashTimer2 += 0x100, flashTimer3 += 0x200){
		C2dEffect* effect = CModelInfo::GetModelInfo(GetModelIndex())->Get2dEffect(i);

		if (effect->type != EFFECT_LIGHT)
			continue;

		if (effect->light.flareType == 1)
			continue;

		if (effect->light.size == 0.0f)
			continue;

		CVector pos = GetMatrix() * effect->pos;

		if (pos.z < 0.0f || pos.z > 530.0f)
			continue;

		bool lightOn = false;
		bool lightFlickering = false;
		switch(effect->light.lightType){
		case LIGHT_ON:
			lightOn = true;
			break;
		case LIGHT_ON_NIGHT:
			if(CClock::GetHours() > 18 || CClock::GetHours() < 7)
				lightOn = true;
			break;
		case LIGHT_FLICKER:
			if((CTimer::GetTimeInMilliseconds() ^ m_randomSeed) & 0x60)
				lightOn = true;
			else
				lightFlickering = true;
			if((CTimer::GetTimeInMilliseconds()>>11 ^ m_randomSeed) & 3)
				lightOn = true;
			break;
		case LIGHT_FLICKER_NIGHT:
			if(CClock::GetHours() > 18 || CClock::GetHours() < 7 || CWeather::WetRoads > 0.5f){
				if((CTimer::GetTimeInMilliseconds() ^ m_randomSeed) & 0x60)
					lightOn = true;
				else
					lightFlickering = true;
				if((CTimer::GetTimeInMilliseconds()>>11 ^ m_randomSeed) & 3)
					lightOn = true;
			}
			break;
		case LIGHT_FLASH1:
			if((CTimer::GetTimeInMilliseconds() + flashTimer1) & 0x200)
				lightOn = true;
			break;
		case LIGHT_FLASH1_NIGHT:
			if(CClock::GetHours() > 18 || CClock::GetHours() < 7)
				if((CTimer::GetTimeInMilliseconds() + flashTimer1) & 0x200)
					lightOn = true;
			break;
		case LIGHT_FLASH2:
			if((CTimer::GetTimeInMilliseconds() + flashTimer2) & 0x400)
				lightOn = true;
			break;
		case LIGHT_FLASH2_NIGHT:
			if(CClock::GetHours() > 18 || CClock::GetHours() < 7)
				if((CTimer::GetTimeInMilliseconds() + flashTimer2) & 0x400)
					lightOn = true;
			break;
		case LIGHT_FLASH3:
			if((CTimer::GetTimeInMilliseconds() + flashTimer3) & 0x800)
				lightOn = true;
			break;
		case LIGHT_FLASH3_NIGHT:
			if(CClock::GetHours() > 18 || CClock::GetHours() < 7)
				if((CTimer::GetTimeInMilliseconds() + flashTimer3) & 0x800)
					lightOn = true;
			break;
		case LIGHT_RANDOM_FLICKER:
			if(m_randomSeed > 16)
				lightOn = true;
			else{
				if((CTimer::GetTimeInMilliseconds() ^ m_randomSeed*8) & 0x60)
					lightOn = true;
				else
					lightFlickering = true;
				if((CTimer::GetTimeInMilliseconds()>>11 ^ m_randomSeed*8) & 3)
					lightOn = true;
			}
			break;
		case LIGHT_RANDOM_FLICKER_NIGHT:
			if(CClock::GetHours() > 18 || CClock::GetHours() < 7){
				if(m_randomSeed > 16)
					lightOn = true;
				else{
					if((CTimer::GetTimeInMilliseconds() ^ m_randomSeed*8) & 0x60)
						lightOn = true;
					else
						lightFlickering = true;
					if((CTimer::GetTimeInMilliseconds()>>11 ^ m_randomSeed*8) & 3)
						lightOn = true;
				}
			}
			break;
		case LIGHT_BRIDGE_FLASH1:
			if(CBridge::ShouldLightsBeFlashing() && CTimer::GetTimeInMilliseconds() & 0x200)
				lightOn = true;
			break;
		case LIGHT_BRIDGE_FLASH2:
			if(CBridge::ShouldLightsBeFlashing() && (CTimer::GetTimeInMilliseconds() & 0x1FF) < 60)
				lightOn = true;
			break;
		}

		/*if(effect->light.flags & LIGHTFLAG_HIDE_OBJECT){
			if(lightOn)
				bDoNotRender = false;
			else
				bDoNotRender = true;
			return;
		}*/

		if (!lightOn && !lightFlickering)
			continue;

		unsigned int	bAlpha;
		float	        fRadius;
		unsigned int	nTime = CClock::GetHours() * 60 + CClock::GetMinutes();
		unsigned int    curMin = CClock::GetMinutes();
		float CoronaFarClip = CTimeCycle::GetFarClip();

		if (CoronaFarClip > 1650.0f)
			CoronaFarClip = 1650.0f;

		if (nTime >= 19 * 60)
			bAlpha = (1.0f / 2.0f) * nTime - 570.0f; // http://goo.gl/3rI2tc
		else if (nTime < 3 * 60)
			bAlpha = 150;
		else
			bAlpha = (-5.0f / 8.0f) * nTime + 262.5f; // http://goo.gl/M8Dev9

		CVector camPos = TheCamera.GetPosition();

		if (Distance(pos, camPos) > CoronaFarClip)
			continue;

		if (!TheCamera.IsSphereVisible(pos, 5.0f))
			continue;

		float fDistSqr = (camPos.x - pos.x) * (camPos.x - pos.x) + (camPos.y - pos.y) * (camPos.y - pos.y) + (camPos.z - pos.z) * (camPos.z - pos.z);

		//if ((fDistSqr <= 250.0f * 250.0f || fDistSqr >= CoronaFarClip * CoronaFarClip))
		if ((fDistSqr <= effect->light.dist * effect->light.dist || fDistSqr >= CoronaFarClip * CoronaFarClip))
			continue;

		//fRadius = (fDistSqr < 300.0f * 300.0f) ? (0.07f) * sqrt(fDistSqr) - 17.5f : 3.5f;
		fRadius = (fDistSqr < (effect->light.dist - 50.0f) * (effect->light.dist - 50.0f)) ? (0.07f) * sqrt(fDistSqr) - 17.5f : 3.5f;

		fRadius *= Min((0.0025f) * sqrt(fDistSqr) + 0.25f, 4.0f); // http://goo.gl/3kDpnC

		// Corona
		if(lightOn)
			CCoronas::RegisterCorona((uintptr)this + i + numEffects,
				effect->col.r, effect->col.g, effect->col.b, bAlpha,
				pos, fRadius, effect->light.dist + CoronaFarClip,
				effect->light.corona, effect->light.flareType, effect->light.roadReflection,
				effect->light.flags & LIGHTFLAG_LOSCHECK, CCoronas::STREAK_OFF, 0.0f);
		else if(lightFlickering)
			CCoronas::RegisterCorona((uintptr)this + i + numEffects,
				0, 0, 0, 255,
				pos, fRadius, CoronaFarClip,
				effect->light.corona, effect->light.flareType, effect->light.roadReflection,
				effect->light.flags & LIGHTFLAG_LOSCHECK, CCoronas::STREAK_OFF, 0.0f);
	}
}
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
