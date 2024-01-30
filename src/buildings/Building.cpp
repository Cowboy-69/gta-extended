#include "common.h"

#include "Building.h"
#include "Streaming.h"
#include "Pools.h"
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

#ifdef EX_DISTANT_LIGHTS
void CBuilding::ProcessDistantLights()
{
	if (m_modelIndex == 1233 || m_modelIndex == 1259 || m_modelIndex == 1258 || m_modelIndex == 2802 ||
		m_modelIndex == 2174 || m_modelIndex == 1241 || m_modelIndex == 1459 || m_modelIndex == 2173 || 
		m_modelIndex == 2172 || m_modelIndex == 1244 || m_modelIndex == 412 || m_modelIndex == 1243 ||
		m_modelIndex == 1550 || m_modelIndex == 2249 || m_modelIndex == 290) {

		return;
	}
	
	uint32 flashTimer1, flashTimer2, flashTimer3;
	flashTimer1 = 0;
	flashTimer2 = 0;
	flashTimer3 = 0;

	int numEffects = CModelInfo::GetModelInfo(GetModelIndex())->GetNum2dEffects();
	for(int i = 0; i < numEffects; i++, flashTimer1 += 0x80, flashTimer2 += 0x100, flashTimer3 += 0x200){
		C2dEffect* effect = CModelInfo::GetModelInfo(GetModelIndex())->Get2dEffect(i);

		if(effect->type != EFFECT_LIGHT)
			continue;

		CVector pos = GetMatrix() * effect->pos;

		if (pos.z < 0.0f || pos.z > 1030.0f)
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
				pos, fRadius, CoronaFarClip,
				effect->light.corona, effect->light.flareType, effect->light.roadReflection,
				effect->light.flags&LIGHTFLAG_LOSCHECK, CCoronas::STREAK_OFF, 0.0f);
		else if(lightFlickering)
			CCoronas::RegisterCorona((uintptr)this + i + numEffects,
				0, 0, 0, 255,
				pos, fRadius, CoronaFarClip,
				effect->light.corona, effect->light.flareType, effect->light.roadReflection,
				effect->light.flags&LIGHTFLAG_LOSCHECK, CCoronas::STREAK_OFF, 0.0f);
	}
}
#endif