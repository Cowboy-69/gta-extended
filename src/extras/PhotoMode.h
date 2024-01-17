#pragma once

#include "common.h"

#ifdef EX_PHOTO_MODE

#include "PlayerPed.h"
#include "Particle.h"

#define MAX_MUZZLE_PARTICLES 2

class CPhotoMode {
public:
	static bool IsPhotoModeEnabled() { return bPhotoModeEnabled; }
	static CVector GetCameraPosition() { return cameraPos; }
	static float GetSunAngle() { return sunAndMoonAngle; }
	static bool HasSunAngleBeenChanged() { return bHasSunAngleBeenChanged; }

	static void EnablePhotoMode();
	static void DisablePhotoMode();

	static void ProcessControl();

	static void DrawMenuAndEffects();

private:
	enum eTabs {
		TAB_CAMERA,
		TAB_CHARACTER,
		TAB_SKY,
		TAB_LIGHT,
		TAB_EFFECTS,
		NUM_TABS,
	};

	enum eItems {
		// Camera
		ITEM_CAMERA_FOV = 0,
		ITEM_CAMERA_TILT,
		ITEM_CAMERA_NEAR_CLIP,

		// Character
		ITEM_CHAR_VISIBILITY = 0,
		ITEM_CHAR_ANIMATION,
		ITEM_CHAR_ANIMATION_FRAME,
		ITEM_CHAR_WEAPON,
		ITEM_CHAR_MUZZLE_FLASH,
		ITEM_CHAR_ROTATE,

		// Sky
		ITEM_SKY_SUN_MOON = 0,
		ITEM_SKY_HOURS,
		ITEM_SKY_MINUTES,
		ITEM_SKY_WEATHER,

		// Light
		ITEM_LIGHT_DEFAULT_LIGHT = 0,
		ITEM_LIGHT_COLOR_RED,
		ITEM_LIGHT_COLOR_GREEN,
		ITEM_LIGHT_COLOR_BLUE,
		ITEM_LIGHT_DARK_LIGHT,

		// Effects
		ITEM_EFFECT_COLORFILTER = 0,
		ITEM_EFFECT_INTENSITY,
		ITEM_EFFECT_BRIGHTNESS,
		ITEM_EFFECT_FRAMES,

		// LIMITS
		NUM_CAMERA_ITEMS = ITEM_CAMERA_NEAR_CLIP + 1,
		NUM_CHAR_ITEMS = ITEM_CHAR_ROTATE + 1,
		NUM_SKY_ITEMS = ITEM_SKY_WEATHER + 1,
		NUM_LIGHT_ITEMS = ITEM_LIGHT_DARK_LIGHT + 1,
		NUM_EFFECT_ITEMS = ITEM_EFFECT_FRAMES + 1,
	};

	enum eFrameEffects {
		FRAME_EFFECT_NONE,
		FRAME_EFFECT_CUTSCENE,
		FRAME_EFFECT_ON_ALL_SIDES,
	};

	static void DrawHints();

	static void ProcessCamera();
	static void ProcessInput();

	static void TakePhoto();

	static void UpdateMuzzleParticle();

	static void ProcessPlayerPosition();
	static void ProcessLights();
	static void DrawLightSprite();

	static void DrawFrameEffects();

	static char* GetTabName(int tab);
	static char* GetItemName(int item);
	static char* GetItemValue(int item);

	static void InitFakePlayer();
	static void DeleteFakePlayer();

	static void AddPlayerWeapon();
	static void RemovePlayerWeapon();

	static void CreateMuzzleFlash();
	static void RemoveMuzzleParticles();

	static void ChangePlayerPosition();

	static bool bPhotoModeEnabled;
	static bool bMenuOpen;
	static bool bPhotoModeReadyToUse;
	static bool bPlayerWantsToTakePhoto;
	static float photoSavedTime;
	static bool bHideUI;

	static CVector cameraPos;
	static float betaAngleCamera;
	static float alphaAngleCamera;

	static int maxTabs;
	static int currentTab;
	static int maxItems;
	static int currentItem;

	static float cameraFOV;
	static float cameraTilt;
	static float cameraNearClip;

	static bool bCharacterVisibility;
	static int animID;
	static float animFrame;
	static int weaponID;
	static bool bMuzzleFlash;
	static float rotateAngle;

	static float sunAndMoonAngle;
	static int weatherID;
	static bool bHasSunAngleBeenChanged;

	static bool bDefaultLightEnabled;
	static float lightColorRed;
	static float lightColorGreen;
	static float lightColorBlue;
	static bool bDarkLightEnabled;

	static int frameEffectID;

	static CPed* fakePlayer;
	const static int animList[];
	static CParticle* muzzleFlashParticles[MAX_MUZZLE_PARTICLES];
	static CVector playerPos;
	static CVector lightPos;

	static CVector storedPlayerPos;
	static int storedColorfilter;
	static float storedIntensity;
	static int storedBrighntess;
};

#endif