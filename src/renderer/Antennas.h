#pragma once

class CAntenna
{
public:
	bool active;
	bool updatedLastFrame;
	uint32 id;
	float segmentLength;
	CVector pos[6];
	CVector speed[6];
#ifdef EX_VCPD_WINTERGREEN // Antenna
	CRGBA color[6];
#endif

	void Update(CVector dir, CVector pos);
};

class CAntennas
{
	// no need to use game's array
	static CAntenna aAntennas[NUMANTENNAS];
public:
	static void Init(void);
	static void Update(void);
#ifdef EX_VCPD_WINTERGREEN // Antenna
	static void RegisterOne(uint32 id, CVector dir, CVector position, float length, CRGBA color = CRGBA(200, 200, 200, 100));
#else
	static void RegisterOne(uint32 id, CVector dir, CVector position, float length);
#endif
	static void Render(void);
};
