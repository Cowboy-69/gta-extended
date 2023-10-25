#pragma once

#include "Entity.h"

class CBuilding : public CEntity
{
public:
	CBuilding(void) {
		m_type = ENTITY_TYPE_BUILDING;
		bUsesCollision = true;
	}
	static void *operator new(size_t) throw();
	static void operator delete(void*, size_t) throw();

	void ReplaceWithNewModel(int32 id);

	virtual bool GetIsATreadable(void) { return false; }

#ifdef IMPROVED_TECH_PART
	//bool SetupLighting(void);
	//void RemoveLighting(bool reset);
#endif
#ifdef EX_DISTANT_LIGHTS
	void ProcessDistantLights();
#endif
};

bool IsBuildingPointerValid(CBuilding*);
