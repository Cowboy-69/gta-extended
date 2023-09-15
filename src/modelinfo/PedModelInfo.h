#pragma once

#include "ClumpModelInfo.h"
#include "ColModel.h"
#include "PedType.h"

#ifdef EX_PED_VARIATIONS
#define MAX_VARIATIONS_TEXTURES 30
#endif

enum PedNode {
	PED_TORSO = 0,	// has no bone!
	PED_MID,
	PED_HEAD,
	PED_UPPERARML,
	PED_UPPERARMR,
	PED_HANDL,
	PED_HANDR,
	PED_UPPERLEGL,
	PED_UPPERLEGR,
	PED_FOOTL,
	PED_FOOTR,
	PED_LOWERLEGR,
	PED_LOWERLEGL,

	PED_FOREARML,
	PED_FOREARMR,
	PED_CLAVICLEL,
	PED_CLAVICLER,
	PED_NECK,

	PED_NODE_MAX
};

class CPedModelInfo : public CClumpModelInfo
{
public:
	uint32 m_animGroup;
	ePedType m_pedType;
	ePedStats m_pedStatType;
	uint32 m_carsCanDrive;
	CColModel *m_hitColModel;
	int8 radio1, radio2;
#ifdef EX_PED_VARIATIONS
	RwTexture* textureClothingVariations[MAX_VARIATIONS_TEXTURES];
	RwTexture* textureShadeVariations[MAX_VARIATIONS_TEXTURES];
	int currentClothingVariation;
	int currentShadeVariation;
#endif

	static RwObjectNameIdAssocation m_pPedIds[PED_NODE_MAX];

#ifdef EX_PED_VARIATIONS
	CPedModelInfo(void) : CClumpModelInfo(MITYPE_PED) { 
		m_hitColModel = nil; 
		currentClothingVariation = 0;
		currentShadeVariation = 0;

		for (int i = 0; i < MAX_VARIATIONS_TEXTURES; i++)
			textureClothingVariations[i] = nullptr;

		for (int i = 0; i < MAX_VARIATIONS_TEXTURES; i++)
			textureShadeVariations[i] = nullptr;
	}
#else
	CPedModelInfo(void) : CClumpModelInfo(MITYPE_PED) { m_hitColModel = nil; }
#endif
	~CPedModelInfo(void) { delete m_hitColModel; }
	void DeleteRwObject(void);
	void SetClump(RpClump *);

	void CreateHitColModelSkinned(RpClump *clump);
	CColModel *GetHitColModel(void) { return m_hitColModel; }
	CColModel *AnimatePedColModelSkinned(RpClump *clump);
	CColModel *AnimatePedColModelSkinnedWorld(RpClump *clump);
};
