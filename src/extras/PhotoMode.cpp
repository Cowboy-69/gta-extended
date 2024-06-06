#include "PhotoMode.h"

#ifdef EX_PHOTO_MODE

#include "main.h"
#include "Camera.h"
#include "main.h"
#include "Pad.h"
#include "ControllerConfig.h"
#include "Camera.h"
#include "General.h"
#include "Lists.h"
#include "PlayerInfo.h"
#include "Frontend.h"
#include "Hud.h"
#include "Timer.h"
#include "Clock.h"
#include "DMAudio.h"
#include "Font.h"
#include "Text.h"
#include "Weather.h"
#include "World.h"
#include "AnimBlendAssociation.h"
#include "RpAnimBlend.h"
#include "WeaponType.h"
#include "PointLights.h"
#include "Sprite.h"
#include "Radar.h"
#include "postfx.h"
#include "Streaming.h"

#define KEYDOWN(k) ControlsManager.GetIsKeyboardKeyDown((RsKeyCodes)k)
#define KEYJUSTDOWN(k) ControlsManager.GetIsKeyboardKeyJustDown((RsKeyCodes)k)

#define DEFAULT_TEXT_COLOR 255, 150, 225, 255
#define SELECTED_TEXT_COLOR 255, 170, 225, 255
#define HINT_TEXT_COLOR 255, 255, 255, 255

const char* CPhotoMode::clothesList[] = { "player", "player2", "player3", "player4", "player5", "player6", "player7", "player8", "player9", "play10", "play11", "play12" };

const int CPhotoMode::animList[][2] = {
										{ ASSOCGRP_STD, ANIM_STD_IDLE },
										{ ASSOCGRP_STD, ANIM_STD_IDLE_BIGGUN },
										{ ASSOCGRP_STD, ANIM_STD_IDLE_CAM },
										{ ASSOCGRP_STD, ANIM_STD_CHAT },
										{ ASSOCGRP_STD, ANIM_STD_SEAT_UP },
										{ ASSOCGRP_STD, ANIM_STD_EVADE_DIVE },
										{ ASSOCGRP_STD, ANIM_STD_GET_UP },
										{ ASSOCGRP_STD, ANIM_STD_GET_UP_FRONT },
										{ ASSOCGRP_STD, ANIM_STD_ARREST },
										{ ASSOCGRP_STD, ANIM_STD_CRAWLOUT_LHS },
										{ ASSOCGRP_STD, ANIM_STD_CAR_SIT },
										{ ASSOCGRP_STD, ANIM_STD_CAR_SIT_P },
										{ ASSOCGRP_STD, ANIM_STD_PARTIAL_FUCKU },
										{ ASSOCGRP_PLAYER, ANIM_STD_WALK },
										{ ASSOCGRP_PLAYER, ANIM_STD_RUN },
										{ ASSOCGRP_PLAYER, ANIM_STD_RUNFAST },
#ifdef CLIMBING
										{ ASSOCGRP_STD, ANIM_STD_CLIMBING_PULL },
#endif
#ifdef CROUCH
										{ ASSOCGRP_STD, ANIM_STD_CROUCH_FORWARD },
										{ ASSOCGRP_STD, ANIM_STD_CROUCH_ROLL_L },
#endif
#ifdef SWIMMING
										{ ASSOCGRP_STD, ANIM_STD_SWIM_BREAST },
#endif
										{ ASSOCGRP_SCREWDRIVER, ANIM_MELEE_ATTACK },
										{ ASSOCGRP_KNIFE, ANIM_MELEE_ATTACK },
										{ ASSOCGRP_BASEBALLBAT, ANIM_MELEE_ATTACK },
										{ ASSOCGRP_CHAINSAW, ANIM_MELEE_ATTACK_2ND },
										{ ASSOCGRP_COLT, ANIM_WEAPON_FIRE },
										{ ASSOCGRP_COLT, ANIM_MELEE_ATTACK_2ND },
										{ ASSOCGRP_PYTHON, ANIM_WEAPON_FIRE },
										{ ASSOCGRP_PYTHON, ANIM_MELEE_ATTACK_2ND },
										{ ASSOCGRP_SHOTGUN, ANIM_WEAPON_FIRE },
										{ ASSOCGRP_SHOTGUN, ANIM_MELEE_ATTACK_2ND },
#ifdef EX_WEAPON_STEYR // Animations in photo mode
										{ ASSOCGRP_STEYR, ANIM_WEAPON_FIRE },
#endif
										{ ASSOCGRP_BUDDY, ANIM_WEAPON_FIRE },
										{ ASSOCGRP_BUDDY, ANIM_MELEE_ATTACK_2ND },
										{ ASSOCGRP_UZI, ANIM_WEAPON_FIRE },
										{ ASSOCGRP_UZI, ANIM_MELEE_ATTACK_2ND },
										{ ASSOCGRP_RIFLE, ANIM_WEAPON_FIRE },
										{ ASSOCGRP_RIFLE, ANIM_MELEE_ATTACK_2ND },
										{ ASSOCGRP_THROW, ANIM_WEAPON_FIRE },
										{ ASSOCGRP_FLAMETHROWER, ANIM_WEAPON_FIRE },
#ifdef FIRING_AND_AIMING // Animations in photo mode
										{ ASSOCGRP_ROCKET, ANIM_WEAPON_FIRE },
#endif
									  };

bool CPhotoMode::bPhotoModeEnabled = false;
bool CPhotoMode::bMenuOpen = false;
bool CPhotoMode::bPhotoModeReadyToUse = false;
bool CPhotoMode::bPlayerWantsToTakePhoto = false;
float CPhotoMode::photoSavedTime = 0.0f;
bool CPhotoMode::bHideUI = false;

float CPhotoMode::betaAngleCamera = 0.0f;
float CPhotoMode::alphaAngleCamera = 0.0f;
CVector CPhotoMode::cameraPos = CVector(0.0f, 0.0f, 0.0f);

int CPhotoMode::maxTabs = NUM_TABS;
int CPhotoMode::currentTab = TAB_CAMERA;
int CPhotoMode::maxItems = NUM_CAMERA_ITEMS;
int CPhotoMode::currentItem = ITEM_CAMERA_FOV;

float CPhotoMode::cameraFOV = 70.0f;
float CPhotoMode::cameraTilt = 0.0f;
float CPhotoMode::cameraNearClip = DEFAULT_NEAR;

bool CPhotoMode::bCharacterVisibility = true;
int CPhotoMode::clothesID = -1;
int CPhotoMode::animID = -1;
float CPhotoMode::animFrame = 0.0f;
int CPhotoMode::weaponID = WEAPONTYPE_UNARMED;
bool CPhotoMode::bMuzzleFlash = false;
float CPhotoMode::rotateAngle = 0.0f;

float CPhotoMode::sunAndMoonAngle = 0.0f;
int CPhotoMode::weatherID = -1;
bool CPhotoMode::bHasSunAngleBeenChanged = false;

bool CPhotoMode::bDefaultLightEnabled = false;
float CPhotoMode::lightColorRed = 1.0f;
float CPhotoMode::lightColorGreen = 1.0f;
float CPhotoMode::lightColorBlue = 1.0f;
bool CPhotoMode::bDarkLightEnabled = false;

int CPhotoMode::frameEffectID = FRAME_EFFECT_NONE;

CPed* CPhotoMode::fakePlayer = nullptr;
CParticle* CPhotoMode::muzzleFlashParticles[MAX_MUZZLE_PARTICLES] = {};
CVector CPhotoMode::playerPos = CVector(0.0f, 0.0f, 0.0f);
CVector CPhotoMode::lightPos = CVector(0.0f, 0.0f, 0.0f);

CVector CPhotoMode::storedPlayerPos = CVector(0.0f, 0.0f, 0.0f);
int CPhotoMode::storedColorfilter = 0;
float CPhotoMode::storedIntensity = 1.0f;
int CPhotoMode::storedBrighntess = 0;

float CPhotoMode::storedInterpolationValue = 0.0f;
int16 CPhotoMode::storedOldWeatherType = 0;
int16 CPhotoMode::storedNewWeatherType = 0;
int16 CPhotoMode::storedForcedWeatherType = WEATHER_RANDOM;
float CPhotoMode::storedRain = 0.0f;

void CPhotoMode::EnablePhotoMode()
{
	DMAudio.SetEffectsFadeVol(0);
	DMAudio.SetMusicFadeVol(0);

	if (IsPhotoModeEnabled())
		return;

	RequestAnims();

	CClock::StoreClock();
	StoreWeatherState();

	FindPlayerPed()->bIsVisible = true;

	cameraFOV = 70.0f;
	cameraTilt = 0.0f;
	cameraNearClip = DEFAULT_NEAR;

	bCharacterVisibility = true;
	clothesID = -1;
	animID = -1;
	animFrame = 0.0f;
	weaponID = WEAPONTYPE_UNARMED;
	bMuzzleFlash = false;
	rotateAngle = FindPlayerPed()->m_fRotationCur;
	storedPlayerPos = FindPlayerCoors();
	storedColorfilter = CPostFX::EffectSwitch;
	storedIntensity = CPostFX::Intensity;
	storedBrighntess = FrontEndMenuManager.m_PrefsBrightness;

	float angleOfSun = 2 * PI * (CClock::GetSeconds() / 60.0f + CClock::GetMinutes() + CClock::GetHours() * 60) / (24 * 60);
	if (angleOfSun > PI)
		sunAndMoonAngle = PI / angleOfSun;
	else
		sunAndMoonAngle = angleOfSun / PI;

	weatherID = CWeather::OldWeatherType;
	bHasSunAngleBeenChanged = false;

	bDefaultLightEnabled = false;
	lightColorRed = 1.0f;
	lightColorGreen = 1.0f;
	lightColorBlue = 1.0f;
	bDarkLightEnabled = false;
	playerPos = FindPlayerCoors();
	lightPos = FindPlayerCoors() + FindPlayerPed()->GetForward();

	frameEffectID = FRAME_EFFECT_NONE;

	alphaAngleCamera = TheCamera.Cams[TheCamera.ActiveCam].Alpha;
	betaAngleCamera = CGeneral::GetATanOfXY(FindPlayerCoors().y - TheCamera.GetMatrix().GetPosition().y, FindPlayerCoors().x - TheCamera.GetMatrix().GetPosition().x);
	cameraPos = TheCamera.Cams[TheCamera.ActiveCam].Source;

	currentTab = TAB_CAMERA;
	maxItems = NUM_CAMERA_ITEMS;
	currentItem = ITEM_CAMERA_FOV;

	bMenuOpen = false;
	bPhotoModeEnabled = true;
	bPhotoModeReadyToUse = false;
	bPlayerWantsToTakePhoto = false;
	photoSavedTime = 0.0f;
	bHideUI = false;

#ifdef FIRST_PERSON // Restoring the player's head in first-person mode when switching to photo mode
	if (TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_REAL_1ST_PERSON)
		RpAnimBlendClumpUpdateAnimations(FindPlayerPed()->GetClump(), CTimer::GetTimeStepInSeconds());
#endif
}

void CPhotoMode::DisablePhotoMode()
{
	if (!IsPhotoModeEnabled())
		return;

	DMAudio.SetEffectsFadeVol(127);
	DMAudio.SetMusicFadeVol(127);

	RemoveAnims();

	CClock::RestoreClock();
	RestoreWeatherState();
	CPostFX::EffectSwitch = storedColorfilter;
	CPostFX::Intensity = storedIntensity;
	FrontEndMenuManager.m_PrefsBrightness = storedBrighntess;
	FindPlayerPed()->SetHeading(FindPlayerPed()->m_fRotationCur);
	FindPlayerPed()->SetPosition(storedPlayerPos);

	bHasSunAngleBeenChanged = false;

	DeleteFakePlayer();

	RemovePlayerWeapon();

	CWeaponInfo* weaponInfo = FindPlayerPed()->GetWeapon()->GetInfo();
	if (weaponInfo->m_nModelId != -1) {
		FindPlayerPed()->AddWeaponModel(weaponInfo->m_nModelId);
	}
	
	bMenuOpen = false;
	bPhotoModeEnabled = false;
}

void CPhotoMode::ProcessControl()
{
	if (CTimer::GetIsPaused())
		return;

	if (!bPhotoModeReadyToUse) {
		if (CPad::GetPad(0)->IsAffectedByController) {
			// Avoid creating a photo after the player has entered photo mode on the gamepad
			
			CPad::GetPad(0)->AffectFromXinput(0);
			
			if (CPad::GetPad(0)->PCTempJoyState.Cross)
				return;
			else
				bPhotoModeReadyToUse = true;
		} else {
			bPhotoModeReadyToUse = true;
		}
	}

	if (bPlayerWantsToTakePhoto)
		TakePhoto();

	if (photoSavedTime > 0.0f)
		photoSavedTime -= 1.0f * CTimer::GetTimeStep();

	ProcessCamera();
	ProcessInput();

	UpdateMuzzleParticles();

	ProcessPlayerPosition();
	ProcessLights();
}

void CPhotoMode::DrawHints()
{
	if (photoSavedTime > 0.0f) {
		CSprite2d::DrawRect(CRect(0.0f, SCREEN_SCALE_Y(20.0f), SCREEN_WIDTH, 0.0f), CRGBA(25, 25, 25, 100));

		CFont::SetPropOn();
		CFont::SetBackgroundOff();
		CFont::SetScale(SCREEN_SCALE_X(0.8f * 0.6f), SCREEN_SCALE_Y(1.35f * 0.6f));
		CFont::SetRightJustifyOff();
		CFont::SetRightJustifyWrap(0.0f);
		CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
		CFont::SetBackGroundOnlyTextOn();
		CFont::SetCentreOff();
		CFont::SetColor(CRGBA(DEFAULT_TEXT_COLOR));
		CFont::PrintString(SCREEN_SCALE_X(5.0f), SCREEN_SCALE_Y(1.5f), TheText.Get("PM_SAVE"));
	}

	if (bHideUI && !bMenuOpen)
		return;

	CPad* pad = CPad::GetPad(0);

	CSprite2d::DrawRect(CRect(0.0f, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_SCALE_FROM_BOTTOM(20.0f)), CRGBA(25, 25, 25, 100));
	
	const float textScaleX = 0.8f * 0.475f;
	const float textScaleY = 1.35f * 0.475f;

	CFont::RenderState.scaleX = SCREEN_SCALE_X(0.8f * 0.75f);
	CFont::RenderState.scaleY = SCREEN_SCALE_Y(1.35f * 0.75f);

	float offsetX = 3.0f;
	const float buttonY = 20.5f;
	const float textY = 17.0f;

	CFont::SetPropOn();
	CFont::SetBackgroundOff();
	CFont::SetRightJustifyOff();
	CFont::SetRightJustifyWrap(0.0f);
	CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
	CFont::SetBackGroundOnlyTextOn();
	CFont::SetCentreOff();
	CFont::SetColor(CRGBA(HINT_TEXT_COLOR));

	if (pad->IsAffectedByController) {
		// Toggle menu

		CFont::PS2Symbol = BUTTON_SQUARE;
		CFont::DrawButton(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
		CFont::PS2Symbol = BUTTON_NONE;

		offsetX += 20.0f;

		CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
		CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_TOGL"));

		offsetX += 55.0f;

		// Exit photo mode

		CFont::PS2Symbol = BUTTON_CIRCLE;
		CFont::DrawButton(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
		CFont::PS2Symbol = BUTTON_NONE;

		offsetX += 20.0f;

		CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
		CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_EXIT"));

		offsetX += 50.0f;

		if (bMenuOpen) {
			// Adjust

			CFont::PS2Symbol = BUTTON_UP_DOWN;
			CFont::DrawButton(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
			CFont::PS2Symbol = BUTTON_NONE;

			offsetX += 20.0f;

			CFont::PS2Symbol = BUTTON_LEFT_RIGHT;
			CFont::DrawButton(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
			CFont::PS2Symbol = BUTTON_NONE;
		
			offsetX += 20.0f;

			CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
			CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_ADJS"));

			offsetX += 75.0f;

			if (currentTab == TAB_LIGHT && (bDefaultLightEnabled || bDarkLightEnabled)) {
				// Move light
			
				CFont::PS2Symbol = BUTTON_CROSS;
				CFont::DrawButton(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
				CFont::PS2Symbol = BUTTON_NONE;

				offsetX += 20.0f;

				// +
				wchar sPrint[2];
				char sTemp[2];
				sprintf(sTemp, "+");
				AsciiToUnicode(sTemp, sPrint);
				CFont::SetScale(SCREEN_SCALE_X(0.45f), SCREEN_SCALE_Y(0.85f));
				CFont::PrintString(SCREEN_SCALE_X(offsetX - 2.0f), SCREEN_SCALE_FROM_BOTTOM(textY + 2.0f), sPrint);

				offsetX += 11.0f;

				CFont::PS2Symbol = BUTTON_LSTICK;
				CFont::DrawButton(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
				CFont::PS2Symbol = BUTTON_NONE;

				offsetX += 15.0f;

				CFont::PS2Symbol = BUTTON_L2;
				CFont::DrawButton(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
				CFont::PS2Symbol = BUTTON_NONE;

				offsetX += 15.0f;

				CFont::PS2Symbol = BUTTON_R2;
				CFont::DrawButton(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
				CFont::PS2Symbol = BUTTON_NONE;

				offsetX += 20.0f;

				CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
				CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_MVLI"));
			} else if (currentTab == TAB_CHARACTER) {
				// Move character

				CFont::PS2Symbol = BUTTON_CROSS;
				CFont::DrawButton(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
				CFont::PS2Symbol = BUTTON_NONE;

				offsetX += 20.0f;

				// +
				wchar sPrint[2];
				char sTemp[2];
				sprintf(sTemp, "+");
				AsciiToUnicode(sTemp, sPrint);
				CFont::SetScale(SCREEN_SCALE_X(0.45f), SCREEN_SCALE_Y(0.85f));
				CFont::PrintString(SCREEN_SCALE_X(offsetX - 2.0f), SCREEN_SCALE_FROM_BOTTOM(textY + 2.0f), sPrint);

				offsetX += 11.0f;

				CFont::PS2Symbol = BUTTON_LSTICK;
				CFont::DrawButton(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
				CFont::PS2Symbol = BUTTON_NONE;

				offsetX += 15.0f;

				CFont::PS2Symbol = BUTTON_L2;
				CFont::DrawButton(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
				CFont::PS2Symbol = BUTTON_NONE;

				offsetX += 15.0f;

				CFont::PS2Symbol = BUTTON_R2;
				CFont::DrawButton(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
				CFont::PS2Symbol = BUTTON_NONE;

				offsetX += 20.0f;

				CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
				CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_MVCH"));
			}
		} else {
			// Move camera

			CFont::PS2Symbol = BUTTON_LSTICK;
			CFont::DrawButton(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
			CFont::PS2Symbol = BUTTON_NONE;

			offsetX += 20.0f;

			CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
			CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_MOVE"));

			offsetX += 60.0f;

			// Rotate camera

			CFont::PS2Symbol = BUTTON_RSTICK;
			CFont::DrawButton(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
			CFont::PS2Symbol = BUTTON_NONE;

			offsetX += 20.0f;

			CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
			CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_ROT"));

			offsetX += 70.0f;

			// Zoom

			CFont::PS2Symbol = BUTTON_UP_DOWN;
			CFont::DrawButton(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
			CFont::PS2Symbol = BUTTON_NONE;

			offsetX += 20.0f;

			CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
			CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_ZOOM"));

			offsetX += 55.0f;

			// Tilt

			CFont::PS2Symbol = BUTTON_LEFT_RIGHT;
			CFont::DrawButton(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
			CFont::PS2Symbol = BUTTON_NONE;

			offsetX += 20.0f;

			CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
			CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_TILT"));

			offsetX += 60.0f;

			// Speed

			CFont::PS2Symbol = BUTTON_L1;
			CFont::DrawButton(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
			CFont::PS2Symbol = BUTTON_NONE;

			offsetX += 20.0f;

			CFont::PS2Symbol = BUTTON_R1;
			CFont::DrawButton(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
			CFont::PS2Symbol = BUTTON_NONE;

			offsetX += 20.0f;

			CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
			CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_SPED"));

			offsetX += 70.0f;

			// Take photo

			CFont::PS2Symbol = BUTTON_CROSS;
			CFont::DrawButton(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
			CFont::PS2Symbol = BUTTON_NONE;

			offsetX += 20.0f;

			CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
			CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_PHTO"));

			offsetX += 65.0f;

			// Hide UI

			CFont::PS2Symbol = BUTTON_TRIANGLE;
			CFont::DrawButton(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
			CFont::PS2Symbol = BUTTON_NONE;

			offsetX += 20.0f;

			CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
			CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_UI"));
		}
	} else {
		// PC Keys

		// Toggle menu

		CFont::PCSymbol = VK_TAB - 1;
		CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
		CFont::PCSymbol = BUTTON_NONE;

		offsetX += 27.5f;

		CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
		CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_TOGL"));

		offsetX += 55.0f;

		// Exit photo mode

		CFont::PCSymbol = VK_ESCAPE - 1;
		CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
		CFont::PCSymbol = BUTTON_NONE;

		offsetX += 20.0f;

		CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
		CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_EXIT"));

		offsetX += 50.0f;

		if (bMenuOpen) {
			// Adjust

			CFont::PCSymbol = 'W' - 1;
			CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
			CFont::PCSymbol = BUTTON_NONE;

			offsetX += 17.5f;

			CFont::PCSymbol = 'A' - 1;
			CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
			CFont::PCSymbol = BUTTON_NONE;

			offsetX += 17.5f;

			CFont::PCSymbol = 'S' - 1;
			CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
			CFont::PCSymbol = BUTTON_NONE;

			offsetX += 17.5f;

			CFont::PCSymbol = 'D' - 1;
			CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
			CFont::PCSymbol = BUTTON_NONE;
		
			offsetX += 20.0f;

			CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
			CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_ADJS"));

			offsetX += 75.0f;

			if (currentTab == TAB_LIGHT && (bDefaultLightEnabled || bDarkLightEnabled)) {
				// Move light
			
				CFont::PCSymbol = VK_LBUTTON - 1;
				CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
				CFont::PCSymbol = BUTTON_NONE;

				offsetX += 19.0f;

				// +
				wchar sPrint[2];
				char sTemp[2];
				sprintf(sTemp, "+");
				AsciiToUnicode(sTemp, sPrint);
				CFont::SetScale(SCREEN_SCALE_X(0.45f), SCREEN_SCALE_Y(0.85f));
				CFont::PrintString(SCREEN_SCALE_X(offsetX - 2.0f), SCREEN_SCALE_FROM_BOTTOM(textY + 2.0f), sPrint);

				offsetX += 9.5f;

				CFont::PCSymbol = SPRITE_MOUSE;
				CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
				CFont::PCSymbol = BUTTON_NONE;

				offsetX += 15.0f;

				CFont::PCSymbol = VK_RBUTTON - 1;
				CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
				CFont::PCSymbol = BUTTON_NONE;

				offsetX += 19.0f;

				CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
				CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_MVLI"));
			} else if (currentTab == TAB_CHARACTER) {
				// Move character

				CFont::PCSymbol = VK_LBUTTON - 1;
				CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
				CFont::PCSymbol = BUTTON_NONE;

				offsetX += 19.0f;

				// +
				wchar sPrint[2];
				char sTemp[2];
				sprintf(sTemp, "+");
				AsciiToUnicode(sTemp, sPrint);
				CFont::SetScale(SCREEN_SCALE_X(0.45f), SCREEN_SCALE_Y(0.85f));
				CFont::PrintString(SCREEN_SCALE_X(offsetX - 2.0f), SCREEN_SCALE_FROM_BOTTOM(textY + 2.0f), sPrint);

				offsetX += 9.5f;

				CFont::PCSymbol = SPRITE_MOUSE;
				CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
				CFont::PCSymbol = BUTTON_NONE;

				offsetX += 15.0f;

				CFont::PCSymbol = VK_RBUTTON - 1;
				CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
				CFont::PCSymbol = BUTTON_NONE;

				offsetX += 19.0f;

				CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
				CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_MVCH"));
			}
		} else {
			if (pad->GetRightMouse()) {
				// Zoom

				CFont::PCSymbol = 'W' - 1;
				CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
				CFont::PCSymbol = BUTTON_NONE;

				offsetX += 17.5f;

				CFont::PCSymbol = 'A' - 1;
				CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
				CFont::PCSymbol = BUTTON_NONE;

				offsetX += 20.0f;

				CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
				CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_ZOOM"));

				offsetX += 55.0f;

				// Tilt

				CFont::PCSymbol = 'A' - 1;
				CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
				CFont::PCSymbol = BUTTON_NONE;

				offsetX += 17.5f;

				CFont::PCSymbol = 'D' - 1;
				CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
				CFont::PCSymbol = BUTTON_NONE;

				offsetX += 20.0f;

				CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
				CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_TILT"));

				offsetX += 50.0f;
			} else {
				// Move camera

				CFont::PCSymbol = 'W' - 1;
				CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
				CFont::PCSymbol = BUTTON_NONE;

				offsetX += 17.5f;

				CFont::PCSymbol = 'A' - 1;
				CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
				CFont::PCSymbol = BUTTON_NONE;

				offsetX += 17.5f;

				CFont::PCSymbol = 'S' - 1;
				CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
				CFont::PCSymbol = BUTTON_NONE;

				offsetX += 17.5f;

				CFont::PCSymbol = 'D' - 1;
				CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
				CFont::PCSymbol = BUTTON_NONE;

				offsetX += 17.5f;

				CFont::PCSymbol = 'Q' - 1;
				CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
				CFont::PCSymbol = BUTTON_NONE;

				offsetX += 17.5f;

				CFont::PCSymbol = 'E' - 1;
				CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
				CFont::PCSymbol = BUTTON_NONE;

				offsetX += 20.0f;

				CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
				CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_MOVE"));

				offsetX += 60.0f;
			}

			// Rotate camera

			CFont::PCSymbol = VK_LBUTTON - 1;
			CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
			CFont::PCSymbol = BUTTON_NONE;

			offsetX += 19.0f;

			// +
			wchar sPrint[2];
			char sTemp[2];
			sprintf(sTemp, "+");
			AsciiToUnicode(sTemp, sPrint);
			CFont::SetScale(SCREEN_SCALE_X(0.45f), SCREEN_SCALE_Y(0.85f));
			CFont::PrintString(SCREEN_SCALE_X(offsetX - 2.0f), SCREEN_SCALE_FROM_BOTTOM(textY + 2.0f), sPrint);

			offsetX += 9.5f;

			CFont::PCSymbol = SPRITE_MOUSE;
			CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
			CFont::PCSymbol = BUTTON_NONE;

			offsetX += 20.0f;

			CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
			CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_ROT"));

			offsetX += 70.0f;

			// Zoom and Tilt

			CFont::PCSymbol = VK_RBUTTON - 1;
			CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
			CFont::PCSymbol = BUTTON_NONE;

			offsetX += 20.0f;

			CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
			CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_ZOTI"));

			offsetX += 115.0f;

			// Speed

			CFont::PCSymbol = VK_LSHIFT - 1;
			CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
			CFont::PCSymbol = BUTTON_NONE;

			offsetX += 26.0f;

			CFont::PCSymbol = VK_LCONTROL - 1;
			CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
			CFont::PCSymbol = BUTTON_NONE;

			offsetX += 28.0f;

			CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
			CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_SPED"));

			offsetX += 70.0f;

			// Take photo

			CFont::PCSymbol = 'T' - 1;
			CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
			CFont::PCSymbol = BUTTON_NONE;

			offsetX += 20.0f;

			CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
			CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_PHTO"));

			offsetX += 65.0f;

			// Hide UI

			CFont::PCSymbol = 'H' - 1;
			CFont::DrawPCKey(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(buttonY));
			CFont::PCSymbol = BUTTON_NONE;

			offsetX += 20.0f;

			CFont::SetScale(SCREEN_SCALE_X(textScaleX), SCREEN_SCALE_Y(textScaleY));
			CFont::PrintString(SCREEN_SCALE_X(offsetX), SCREEN_SCALE_FROM_BOTTOM(textY), TheText.Get("PM_UI"));

			offsetX += 30.0f;
		}
	}
}

void CPhotoMode::ProcessCamera()
{
	CPad* pad = CPad::GetPad(0);

	if (!bMenuOpen && (pad->GetRightMouse() || pad->IsAffectedByController)) {
		if (KEYDOWN('W') || pad->IsAffectedByController && pad->GetDPadUp()) {
			if (pad->GetLeftShift() || pad->IsAffectedByController && pad->GetLeftShoulder1())
				cameraFOV -= 2.0f * CTimer::GetTimeStepFix();
			else if (pad->GetLeftCtrl() || pad->IsAffectedByController && pad->GetRightShoulder1())
				cameraFOV -= 0.25f * CTimer::GetTimeStepFix();
			else
				cameraFOV -= 1.0f * CTimer::GetTimeStepFix();

			if (cameraFOV <= 10.0f)
				cameraFOV = 10.0f;
		} else if (KEYDOWN('S') || pad->IsAffectedByController && pad->GetDPadDown()) {
			if (pad->GetLeftShift() || pad->IsAffectedByController && pad->GetLeftShoulder1())
				cameraFOV += 2.0f * CTimer::GetTimeStepFix();
			else if (pad->GetLeftCtrl() || pad->IsAffectedByController && pad->GetRightShoulder1())
				cameraFOV += 0.25f * CTimer::GetTimeStepFix();
			else
				cameraFOV += 1.0f * CTimer::GetTimeStepFix();

			if (cameraFOV >= 120.0f)
				cameraFOV = 120.0f;
		}

		if (KEYDOWN('A') || pad->IsAffectedByController && pad->GetDPadLeft()) {
			if (pad->GetLeftShift() || pad->IsAffectedByController && pad->GetLeftShoulder1())
				cameraTilt -= 0.04f * CTimer::GetTimeStepFix();
			else if (pad->GetLeftCtrl() || pad->IsAffectedByController && pad->GetRightShoulder1())
				cameraTilt -= 0.005f * CTimer::GetTimeStepFix();
			else
				cameraTilt -= 0.02f * CTimer::GetTimeStepFix();

			if (cameraTilt <= -2.0f)
				cameraTilt = -2.0f;
		} else if (KEYDOWN('D') || pad->IsAffectedByController && pad->GetDPadRight()) {
			if (pad->GetLeftShift() || pad->IsAffectedByController && pad->GetLeftShoulder1())
				cameraTilt += 0.04f * CTimer::GetTimeStepFix();
			else if (pad->GetLeftCtrl() || pad->IsAffectedByController && pad->GetRightShoulder1())
				cameraTilt += 0.005f * CTimer::GetTimeStepFix();
			else
				cameraTilt += 0.02f * CTimer::GetTimeStepFix();

			if (cameraTilt >= 2.0f)
				cameraTilt = 2.0f;
		}
	}

	RwCameraSetNearClipPlane(TheCamera.m_pRwCamera, cameraNearClip);
	CDraw::SetNearClipZ(cameraNearClip);
	
	CDraw::SetFOV(cameraFOV);

	float curSenseX;
	float curSenseY;

	if (pad->IsAffectedByController) {
		curSenseX = FrontEndMenuManager.m_PrefsPadLookSensX;
		curSenseY = FrontEndMenuManager.m_PrefsPadLookSensY;
	} else {
		curSenseX = FrontEndMenuManager.m_PrefsMouseLookSensX / 2;
		curSenseY = FrontEndMenuManager.m_PrefsMouseLookSensY / 2;
	}

	if (FrontEndMenuManager.m_PrefsInvertVertically)
		curSenseY = -curSenseY;

	TheCamera.m_fMouseAccelHorzntl = curSenseX;
	TheCamera.m_fMouseAccelVertical = curSenseY;

	float MouseX = pad->GetMouseX();
	float MouseY = pad->GetMouseY();
	float LookLeftRight = 0.0f;
	float LookUpDown = 0.0f;
	if (!bMenuOpen && (pad->GetLeftMouse() || pad->IsAffectedByController)) {
		if (pad->IsAffectedByController) {
			LookLeftRight = pad->LookAroundLeftRight();
			LookUpDown = pad->LookAroundUpDown();
		} else {
			LookLeftRight = 2.5f * MouseX;
			LookUpDown = 4.0f * MouseY;
		}
	}
	float AlphaOffset, BetaOffset;
	BetaOffset = LookLeftRight * TheCamera.m_fMouseAccelHorzntl * 70.0f / 80.0f;
	AlphaOffset = LookUpDown * TheCamera.m_fMouseAccelVertical * 70.0f / 80.0f;

	alphaAngleCamera += AlphaOffset;
	betaAngleCamera += BetaOffset;

	if (alphaAngleCamera > DEGTORAD(89.5f)) alphaAngleCamera = DEGTORAD(89.5f);
	else if (alphaAngleCamera < DEGTORAD(-89.5f)) alphaAngleCamera = DEGTORAD(-89.5f);

	CVector TargetCoors;

	TargetCoors.x = cameraPos.x + Cos(alphaAngleCamera) * Sin(betaAngleCamera) * 7.0f;
	TargetCoors.y = cameraPos.y + Cos(alphaAngleCamera) * Cos(betaAngleCamera) * 7.0f;
	TargetCoors.z = cameraPos.z + Sin(alphaAngleCamera) * 3.0f;

	static float Speed = 0.0f;
	static float PanSpeedX = 0.0f;
	static float PanSpeedY = 0.0f;

	if (bMenuOpen) {
		Speed = 0.0f;
		PanSpeedX = 0.0f;
		PanSpeedY = 0.0f;
	} else {
		if (pad->IsAffectedByController) {
			float SpeedMultiplier = 0.0015f;
			if (pad->GetLeftShoulder1())
				SpeedMultiplier = 0.0075f;
			else if (pad->GetRightShoulder1())
				SpeedMultiplier = 0.00045f;

			Speed = -pad->GetLeftStickY() * SpeedMultiplier * CTimer::GetTimeStepFix();

			PanSpeedX = -pad->GetLeftStickX() * SpeedMultiplier * CTimer::GetTimeStepFix();

			PanSpeedY = -pad->NewState.LeftShoulder2 / 255.0f * 0.25f + pad->NewState.RightShoulder2 / 255.0f * 0.25f;
		} else {
			float SpeedMultiplier = 0.02f;
			if (pad->GetLeftShift())
				SpeedMultiplier = 0.04f;
			else if (pad->GetLeftCtrl())
				SpeedMultiplier = 0.005f;

			if (KEYDOWN('W') && !pad->GetRightMouse())
				Speed += 1.0f * SpeedMultiplier * CTimer::GetTimeStepFix();
			else if (KEYDOWN('S') && !pad->GetRightMouse())
				Speed += -1.0f * SpeedMultiplier * CTimer::GetTimeStepFix();
			else
				Speed = 0.0f;

			if (KEYDOWN('A') && !pad->GetRightMouse())
				PanSpeedX += 1.0f * SpeedMultiplier * CTimer::GetTimeStepFix();
			else if (KEYDOWN('D') && !pad->GetRightMouse())
				PanSpeedX += -1.0f * SpeedMultiplier * CTimer::GetTimeStepFix();
			else
				PanSpeedX = 0.0f;

			if (KEYDOWN('Q') && !pad->GetRightMouse())
				PanSpeedY += -1.0f * SpeedMultiplier * CTimer::GetTimeStepFix();
			else if (KEYDOWN('E') && !pad->GetRightMouse())
				PanSpeedY += 1.0f * SpeedMultiplier * CTimer::GetTimeStepFix();
			else
				PanSpeedY = 0.0f;
		}
	}

	CVector forward = TargetCoors - cameraPos;
	forward.Normalise();
	CVector right = CrossProduct(CVector(forward.y * cameraTilt, -forward.x * cameraTilt, 1.0f), forward);
	right.Normalise();
	CVector up = CrossProduct(forward, right);
	up.Normalise();

	CVector source = cameraPos + forward * Speed + up * PanSpeedY + right * PanSpeedX;

	cameraPos = source;

	TheCamera.GetForward() = forward;
	TheCamera.GetUp() = up;
	TheCamera.GetRight() = right;
	TheCamera.SetPosition(source);

	RwMatrix* pm = RwFrameGetMatrix(RwCameraGetFrame(TheCamera.m_pRwCamera));
	pm->pos = TheCamera.GetPosition();
	pm->at = TheCamera.GetForward();
	pm->up = TheCamera.GetUp();
	pm->right = TheCamera.GetRight();

	TheCamera.CalculateDerivedValues();
	RwMatrixUpdate(RwFrameGetMatrix(RwCameraGetFrame(TheCamera.m_pRwCamera)));
	RwFrameUpdateObjects(RwCameraGetFrame(TheCamera.m_pRwCamera));
}

void CPhotoMode::ProcessInput()
{
	CPad* pad = CPad::GetPad(0);

	if (!bMenuOpen && !bPlayerWantsToTakePhoto && (KEYJUSTDOWN('T') || pad->IsAffectedByController && pad->GetCrossJustDown()))
		bPlayerWantsToTakePhoto = true;

	if (!bMenuOpen && (KEYJUSTDOWN('H') || pad->IsAffectedByController && pad->GetTriangleJustDown()))
		bHideUI = !bHideUI;

	if (pad->GetTabJustDown() || pad->IsAffectedByController && pad->GetSquareJustDown())
		bMenuOpen = !bMenuOpen;

	if (!bMenuOpen)
		return;

	if (KEYJUSTDOWN('Q') || pad->IsAffectedByController && pad->GetLeftShoulder1JustDown()) {
		if (currentTab == TAB_EFFECTS) {
			currentTab = TAB_LIGHT;
			maxItems = NUM_LIGHT_ITEMS;
			currentItem = ITEM_LIGHT_DEFAULT_LIGHT;
		} else if (currentTab == TAB_LIGHT) {
			currentTab = TAB_SKY;
			maxItems = NUM_SKY_ITEMS;
			currentItem = ITEM_SKY_SUN_MOON;
		} else if (currentTab == TAB_SKY) {
			if (FindPlayerPed()->InVehicle()) {
				currentTab = TAB_CAMERA;
				maxItems = NUM_CAMERA_ITEMS;
				currentItem = ITEM_CAMERA_FOV;
			} else {
				currentTab = TAB_CHARACTER;
				maxItems = NUM_CHAR_ITEMS;
				currentItem = ITEM_CHAR_VISIBILITY;
			}
		} else if (currentTab == TAB_CHARACTER) {
			currentTab = TAB_CAMERA;
			maxItems = NUM_CAMERA_ITEMS;
			currentItem = ITEM_CAMERA_FOV;
		} else if (currentTab == TAB_CAMERA) {
			currentTab = TAB_EFFECTS;
			maxItems = NUM_EFFECT_ITEMS;
			currentItem = ITEM_EFFECT_COLORFILTER;
		}
	} else if (KEYJUSTDOWN('E') || pad->IsAffectedByController && pad->GetRightShoulder1JustDown()) {
		if (currentTab == TAB_CAMERA) {
			if (FindPlayerPed()->InVehicle()) {
				currentTab = TAB_SKY;
				maxItems = NUM_SKY_ITEMS;
				currentItem = ITEM_SKY_SUN_MOON;
			} else {
				currentTab = TAB_CHARACTER;
				maxItems = NUM_CHAR_ITEMS;
				currentItem = ITEM_CHAR_VISIBILITY;
			}
		} else if (currentTab == TAB_CHARACTER) {
			currentTab = TAB_SKY;
			maxItems = NUM_SKY_ITEMS;
			currentItem = ITEM_SKY_SUN_MOON;
		} else if (currentTab == TAB_SKY) {
			currentTab = TAB_LIGHT;
			maxItems = NUM_LIGHT_ITEMS;
			currentItem = ITEM_LIGHT_DEFAULT_LIGHT;
		} else if (currentTab == TAB_LIGHT) {
			currentTab = TAB_EFFECTS;
			maxItems = NUM_EFFECT_ITEMS;
			currentItem = ITEM_EFFECT_COLORFILTER;
		} else if (currentTab == TAB_EFFECTS) {
			currentTab = TAB_CAMERA;
			maxItems = NUM_CAMERA_ITEMS;
			currentItem = ITEM_CAMERA_FOV;
		}
	}

	if (KEYJUSTDOWN('W') || pad->IsAffectedByController && pad->GetDPadUpJustDown()) {
		if (currentItem == 0)
			currentItem = maxItems - 1;
		else
			currentItem--;
	} else if (KEYJUSTDOWN('S') || pad->IsAffectedByController && pad->GetDPadDownJustDown()) {
		if (currentItem >= (maxItems - 1))
			currentItem = 0;
		else
			currentItem++;
	}

	if (KEYDOWN('A') || pad->IsAffectedByController && pad->GetDPadLeft()) {
		if (currentTab == TAB_CAMERA) {
			if (currentItem == ITEM_CAMERA_FOV) {
				cameraFOV--;

				if (cameraFOV <= 10)
					cameraFOV = 10;
			} else if (currentItem == ITEM_CAMERA_TILT) {
				cameraTilt -= 0.02f;;

				if (cameraTilt <= -2.0f)
					cameraTilt = -2.0f;
			}
		} else if (currentTab == TAB_CHARACTER) {
			if (currentItem == ITEM_CHAR_ANIMATION_FRAME) {
				if (fakePlayer) {
					CAnimBlendAssociation* currentAnim = RpAnimBlendClumpGetFirstAssociation(fakePlayer->GetClump());
					if (currentAnim) {
						animFrame -= 0.01f;

						if (animFrame < 0.0f)
							animFrame = 0.0f;

						currentAnim->SetCurrentTime(animFrame);
						RpAnimBlendClumpUpdateAnimations(fakePlayer->GetClump(), CTimer::GetTimeStepInSeconds());
					}
				}
			} else if (currentItem == ITEM_CHAR_ROTATE) {
				rotateAngle -= 0.05f;

				if (rotateAngle <= -PI)
					rotateAngle = PI;

				FindPlayerPed()->SetHeading(rotateAngle);
				ChangePlayerPosition();
				FindPlayerPed()->GetMatrix().UpdateRW();
				FindPlayerPed()->UpdateRwFrame();

				if (fakePlayer) {
					fakePlayer->SetHeading(rotateAngle);
					ChangePlayerPosition();
					fakePlayer->GetMatrix().UpdateRW();
					fakePlayer->UpdateRwFrame();
				}
			}
		} else if (currentTab == TAB_SKY) {
			if (currentItem == ITEM_SKY_SUN_MOON) {
				bHasSunAngleBeenChanged = true;

				sunAndMoonAngle += 0.02f;

				if (sunAndMoonAngle > PI * 2)
					sunAndMoonAngle -= PI * 2;
			}
		}
	} else if (KEYDOWN('D') || pad->IsAffectedByController && pad->GetDPadRight()) {
		if (currentTab == TAB_CAMERA) {
			if (currentItem == ITEM_CAMERA_FOV) {
				cameraFOV++;

				if (cameraFOV >= 120)
					cameraFOV = 120;
			} else if (currentItem == ITEM_CAMERA_TILT) {
				cameraTilt += 0.02f;

				if (cameraTilt >= 2.0f)
					cameraTilt = 2.0f;
			}
		} else if (currentTab == TAB_CHARACTER) {
			if (currentItem == ITEM_CHAR_ANIMATION_FRAME) {
				if (fakePlayer) {
					CAnimBlendAssociation* currentAnim = RpAnimBlendClumpGetFirstAssociation(fakePlayer->GetClump());
					if (currentAnim) {
						animFrame += 0.01f;

						if (animFrame > currentAnim->hierarchy->totalLength - 0.05f)
							animFrame = currentAnim->hierarchy->totalLength - 0.05f;

						currentAnim->SetCurrentTime(animFrame);
						RpAnimBlendClumpUpdateAnimations(fakePlayer->GetClump(), CTimer::GetTimeStepInSeconds());
					}
				}
			} else if (currentItem == ITEM_CHAR_ROTATE) {
				rotateAngle += 0.05f;

				if (rotateAngle >= PI)
					rotateAngle = -PI;

				FindPlayerPed()->SetHeading(rotateAngle);
				ChangePlayerPosition();
				FindPlayerPed()->GetMatrix().UpdateRW();
				FindPlayerPed()->UpdateRwFrame();

				if (fakePlayer) {
					fakePlayer->SetHeading(rotateAngle);
					ChangePlayerPosition();
					fakePlayer->GetMatrix().UpdateRW();
					fakePlayer->UpdateRwFrame();
				}
			}
		} else if (currentTab == TAB_SKY) {
			if (currentItem == ITEM_SKY_SUN_MOON) {
				bHasSunAngleBeenChanged = true;

				sunAndMoonAngle -= 0.02f;

				if (sunAndMoonAngle < 0.0f)
					sunAndMoonAngle += PI * 2;
			}
		}
	}

	if (KEYJUSTDOWN('A') || pad->IsAffectedByController && pad->GetDPadLeftJustDown()) {
		if (currentTab == TAB_CAMERA) {
			if (currentItem == ITEM_CAMERA_NEAR_CLIP) {
				cameraNearClip -= 0.1f;

				if (cameraNearClip <= 0.1f)
					cameraNearClip = 0.1f;
			}
		} else if (currentTab == TAB_CHARACTER) {
			if (currentItem == ITEM_CHAR_VISIBILITY) {
				bCharacterVisibility = !bCharacterVisibility;
				
				if (fakePlayer) {
					FindPlayerPed()->bIsVisible = false;
					fakePlayer->bIsVisible = bCharacterVisibility;
				} else {
					FindPlayerPed()->bIsVisible = bCharacterVisibility;
				}
			} else if (currentItem == ITEM_CHAR_CLOTHES) {
				int maxClothes = sizeof(clothesList) / 4 - 1;
				if (clothesID <= -1)
					clothesID = maxClothes;
				else
					clothesID--;

				if (clothesID == -1 && animID == -1) {
					RemoveMuzzleParticles();
					bMuzzleFlash = false;
					FindPlayerPed()->bIsVisible = bCharacterVisibility;
					animFrame = 0.0f;

					DeleteFakePlayer();
				} else {
					FindPlayerPed()->bIsVisible = false;

					InitFakePlayer();
				}

				UpdateMuzzleParticles();
			} else if (currentItem == ITEM_CHAR_ANIMATION) {
				int maxAnimations = sizeof(animList) / 8 - 1;
				if (animID <= -1)
					animID = maxAnimations;
				else
					animID--;

				if (animID == -1 && clothesID == -1) {
					RemoveMuzzleParticles();
					bMuzzleFlash = false;
					FindPlayerPed()->bIsVisible = bCharacterVisibility;
					animFrame = 0.0f;

					DeleteFakePlayer();
				} else {
					FindPlayerPed()->bIsVisible = false;

					InitFakePlayer();
				}

				UpdateMuzzleParticles();
			} else if (currentItem == ITEM_CHAR_WEAPON) {
				RemovePlayerWeapon();

				if (weaponID == WEAPONTYPE_TEARGAS)
					weaponID = WEAPONTYPE_GRENADE; // skip WEAPONTYPE_DETONATOR_GRENADE
				else if (weaponID == WEAPONTYPE_COLT45)
					weaponID = WEAPONTYPE_MOLOTOV; // skip WEAPONTYPE_ROCKET
				else if (weaponID == (WEAPONTYPE_TOTALWEAPONS - 1))
					weaponID = WEAPONTYPE_TOTALWEAPONS - 2; // skip WEAPONTYPE_HELICANNON
				else if (weaponID == WEAPONTYPE_UNARMED)
					weaponID = WEAPONTYPE_TOTALWEAPONS - 1;
				else
					weaponID--;

				for (int i = 0; i < MAX_MUZZLE_PARTICLES; i++) {
					if (muzzleFlashParticles[i] && (weaponID <= WEAPONTYPE_MOLOTOV || weaponID >= WEAPONTYPE_ROCKETLAUNCHER)) {
						muzzleFlashParticles[i]->m_nTimeWhenWillBeDestroyed = 0;
						muzzleFlashParticles[i]->m_nAlpha = 0;
						bMuzzleFlash = false;
					}
				}

				AddPlayerWeapon();
			} else if (currentItem == ITEM_CHAR_MUZZLE_FLASH) {
				if (fakePlayer) {
					bMuzzleFlash = !bMuzzleFlash;
				} else {
					bMuzzleFlash = false;
				}

				if (bMuzzleFlash) {
					CreateMuzzleFlash();
				} else {
					RemoveMuzzleParticles();
				}
			}
		} else if (currentTab == TAB_SKY) {
			if (currentItem == ITEM_SKY_HOURS) {
				if (CClock::GetHours() == 0)
					CClock::SetGameClock(23, CClock::GetMinutes());
				else
					CClock::SetGameClock(CClock::GetHours() - 1, CClock::GetMinutes());
			} else if (currentItem == ITEM_SKY_MINUTES) {
				if (CClock::GetMinutes() == 0)
					CClock::SetGameClock(CClock::GetHours(), 59);
				else
					CClock::SetGameClock(CClock::GetHours(), CClock::GetMinutes() - 1);
			} else if (currentItem == ITEM_SKY_WEATHER) {
				if (weatherID == 0)
					weatherID = WEATHER_HURRICANE;
				else
					weatherID--;

				CWeather::ForceWeatherNow(weatherID);
				CWeather::Update();
			}
		} else if (currentTab == TAB_LIGHT) {
			if (currentItem == ITEM_LIGHT_DEFAULT_LIGHT) {
				bDefaultLightEnabled = !bDefaultLightEnabled;
			} else if (currentItem == ITEM_LIGHT_COLOR_RED) {
				lightColorRed -= 0.1f;

				if (lightColorRed <= 0.0f)
					lightColorRed = 0.0f;
			} else if (currentItem == ITEM_LIGHT_COLOR_GREEN) {
				lightColorGreen -= 0.1f;

				if (lightColorGreen <= 0.0f)
					lightColorGreen = 0.0f;
			} else if (currentItem == ITEM_LIGHT_COLOR_BLUE) {
				lightColorBlue -= 0.1f;

				if (lightColorBlue <= 0.0f)
					lightColorBlue = 0.0f;
			} else if (currentItem == ITEM_LIGHT_DARK_LIGHT) {
				bDarkLightEnabled = !bDarkLightEnabled;
			}
		} else if (currentTab == TAB_EFFECTS) {
			if (currentItem == ITEM_EFFECT_COLORFILTER) {
				if (CPostFX::EffectSwitch == CPostFX::POSTFX_OFF)
					CPostFX::EffectSwitch = CPostFX::POSTFX_MOBILE;
				else
					CPostFX::EffectSwitch--;
			} else if (currentItem == ITEM_EFFECT_INTENSITY) {
				CPostFX::Intensity -= 0.1f;

				if (CPostFX::Intensity < 0.0f)
					CPostFX::Intensity = 6.0f;
			} else if (currentItem == ITEM_EFFECT_BRIGHTNESS) {
				FrontEndMenuManager.m_PrefsBrightness -= (511.0f * 0.1f);

				if (FrontEndMenuManager.m_PrefsBrightness < 0)
					FrontEndMenuManager.m_PrefsBrightness = 0;
			} else if (currentItem == ITEM_EFFECT_FRAMES) {
				if (frameEffectID == FRAME_EFFECT_NONE)
					frameEffectID = FRAME_EFFECT_ON_ALL_SIDES;
				else
					frameEffectID--;
			}
		}
	} else if (KEYJUSTDOWN('D') || pad->IsAffectedByController && pad->GetDPadRightJustDown()) {
		if (currentTab == TAB_CAMERA) {
			if (currentItem == ITEM_CAMERA_NEAR_CLIP) {
				cameraNearClip += 0.1f;

				if (cameraNearClip >= DEFAULT_NEAR)
					cameraNearClip = DEFAULT_NEAR;
			}
		} else if (currentTab == TAB_CHARACTER) {
			if (currentItem == ITEM_CHAR_VISIBILITY) {
				bCharacterVisibility = !bCharacterVisibility;
				
				if (fakePlayer) {
					FindPlayerPed()->bIsVisible = false;
					fakePlayer->bIsVisible = bCharacterVisibility;
				} else {
					FindPlayerPed()->bIsVisible = bCharacterVisibility;
				}
			} else if (currentItem == ITEM_CHAR_CLOTHES) {
				int maxClothes = sizeof(clothesList) / 4 - 1;
				if (clothesID >= maxClothes)
					clothesID = -1;
				else
					clothesID++;

				if (clothesID == -1 && animID == -1) {
					RemoveMuzzleParticles();
					bMuzzleFlash = false;
					FindPlayerPed()->bIsVisible = bCharacterVisibility;
					animFrame = 0.0f;

					DeleteFakePlayer();
				} else {
					FindPlayerPed()->bIsVisible = false;

					InitFakePlayer();
				}

				UpdateMuzzleParticles();
			} else if (currentItem == ITEM_CHAR_ANIMATION) {
				int maxAnimations = sizeof(animList) / 8 - 1;
				if (animID >= maxAnimations)
					animID = -1;
				else
					animID++;

				if (animID == -1 && clothesID == -1) {
					RemoveMuzzleParticles();
					bMuzzleFlash = false;
					FindPlayerPed()->bIsVisible = bCharacterVisibility;
					animFrame = 0.0f;

					DeleteFakePlayer();
				} else {
					FindPlayerPed()->bIsVisible = false;

					InitFakePlayer();
				}

				UpdateMuzzleParticles();
			} else if (currentItem == ITEM_CHAR_WEAPON) {
				RemovePlayerWeapon();

				if (weaponID == WEAPONTYPE_GRENADE)
					weaponID = WEAPONTYPE_TEARGAS; // skip WEAPONTYPE_DETONATOR_GRENADE
				else if (weaponID == WEAPONTYPE_MOLOTOV)
					weaponID = WEAPONTYPE_COLT45; // skip WEAPONTYPE_ROCKET
				else if (weaponID == WEAPONTYPE_TOTALWEAPONS - 2)
					weaponID = WEAPONTYPE_TOTALWEAPONS - 1; // skip WEAPONTYPE_HELICANNON
				else if (weaponID >= (WEAPONTYPE_TOTALWEAPONS - 1))
					weaponID = WEAPONTYPE_UNARMED;
				else
					weaponID++;

				for (int i = 0; i < MAX_MUZZLE_PARTICLES; i++) {
					if (muzzleFlashParticles[i] && (weaponID <= WEAPONTYPE_MOLOTOV || weaponID >= WEAPONTYPE_ROCKETLAUNCHER)) {
						muzzleFlashParticles[i]->m_nTimeWhenWillBeDestroyed = 0;
						muzzleFlashParticles[i]->m_nAlpha = 0;
						bMuzzleFlash = false;
					}
				}

				AddPlayerWeapon();
			} else if (currentItem == ITEM_CHAR_MUZZLE_FLASH) {
				if (fakePlayer) {
					bMuzzleFlash = !bMuzzleFlash;
				} else {
					bMuzzleFlash = false;
				}

				if (bMuzzleFlash) {
					CreateMuzzleFlash();
				} else {
					RemoveMuzzleParticles();
				}
			}
		} else if (currentTab == TAB_SKY) {
			if (currentItem == ITEM_SKY_HOURS) {
				if (CClock::GetHours() >= 23)
					CClock::SetGameClock(0, CClock::GetMinutes());
				else
					CClock::SetGameClock(CClock::GetHours() + 1, CClock::GetMinutes());
			} else if (currentItem == ITEM_SKY_MINUTES) {
				if (CClock::GetMinutes() >= 59)
					CClock::SetGameClock(CClock::GetHours(), 0);
				else
					CClock::SetGameClock(CClock::GetHours(), CClock::GetMinutes() + 1);
			} else if (currentItem == ITEM_SKY_WEATHER) {
				if (weatherID >= WEATHER_HURRICANE)
					weatherID = WEATHER_SUNNY;
				else
					weatherID++;

				CWeather::ForceWeatherNow(weatherID);
				CWeather::Update();
			}
		} else if (currentTab == TAB_LIGHT) {
			if (currentItem == ITEM_LIGHT_DEFAULT_LIGHT) {
				bDefaultLightEnabled = !bDefaultLightEnabled;
			} else if (currentItem == ITEM_LIGHT_COLOR_RED) {
				lightColorRed += 0.1f;

				if (lightColorRed >= 1.0f)
					lightColorRed = 1.0f;
			} else if (currentItem == ITEM_LIGHT_COLOR_GREEN) {
				lightColorGreen += 0.1f;

				if (lightColorGreen >= 1.0f)
					lightColorGreen = 1.0f;
			} else if (currentItem == ITEM_LIGHT_COLOR_BLUE) {
				lightColorBlue += 0.1f;

				if (lightColorBlue >= 1.0f)
					lightColorBlue = 1.0f;
			} else if (currentItem == ITEM_LIGHT_DARK_LIGHT) {
				bDarkLightEnabled = !bDarkLightEnabled;
			}
		} else if (currentTab == TAB_EFFECTS) {
			if (currentItem == ITEM_EFFECT_COLORFILTER) {
				if (CPostFX::EffectSwitch == CPostFX::POSTFX_MOBILE)
					CPostFX::EffectSwitch = CPostFX::POSTFX_OFF;
				else
					CPostFX::EffectSwitch++;
			} else if (currentItem == ITEM_EFFECT_INTENSITY) {
				CPostFX::Intensity += 0.1f;

				if (CPostFX::Intensity > 6.0f)
					CPostFX::Intensity = 0.0f;
			} else if (currentItem == ITEM_EFFECT_BRIGHTNESS) {
				FrontEndMenuManager.m_PrefsBrightness += (511.0f * 0.1f);

				if (FrontEndMenuManager.m_PrefsBrightness > 511)
					FrontEndMenuManager.m_PrefsBrightness = 511;
			} else if (currentItem == ITEM_EFFECT_FRAMES) {
				if (frameEffectID == FRAME_EFFECT_ON_ALL_SIDES)
					frameEffectID = FRAME_EFFECT_NONE;
				else
					frameEffectID++;
			}
		}
	}
}

void CPhotoMode::TakePhoto()
{
	if (!bPlayerWantsToTakePhoto)
		return;

	TakeAndSaveScreenshot();

	photoSavedTime = 225.0f;

	bPlayerWantsToTakePhoto = false;
}

void CPhotoMode::UpdateMuzzleParticles()
{
	if (!bMuzzleFlash)
		return;

	if (weaponID == WEAPONTYPE_UNARMED || !fakePlayer)
		return;

	CWeaponInfo* weaponInfo = CWeaponInfo::GetWeaponInfo((eWeaponType)weaponID);
	CVector firePos = weaponInfo->m_vecFireOffset;
	fakePlayer->TransformToNode(firePos, PED_HANDR);

	CPointLights::AddLight(CPointLights::LIGHT_POINT,
						   firePos, CVector(0.0f, 0.0f, 0.0f), 5.0f,
						   1.0f, 0.8f, 0.0f, CPointLights::FOG_NONE, false);

	for (int i = 0; i < MAX_MUZZLE_PARTICLES; i++) {
		if (!muzzleFlashParticles[i] || muzzleFlashParticles[i] && muzzleFlashParticles[i]->m_nTimeWhenWillBeDestroyed <= 0)
			return;
		
		muzzleFlashParticles[i]->m_vecPosition = firePos;
	}
}

void CPhotoMode::ProcessPlayerPosition()
{
	if (!bMenuOpen || currentTab != TAB_CHARACTER)
		return;

	CPad* pad = CPad::GetPad(0);
	if (pad->GetLeftMouse() || pad->IsAffectedByController && pad->GetCross()) {
		if (pad->IsAffectedByController) {
			float stickX = pad->NewState.LeftStickX;
			if (Abs(stickX) <= FrontEndMenuManager.m_PrefsLeftStickDeadzone)
				stickX = 0;
			else
				stickX /= 128.0f * 5.0f;

			float stickY = pad->NewState.LeftStickY;
			if (Abs(stickY) <= FrontEndMenuManager.m_PrefsLeftStickDeadzone)
				stickY = 0;
			else
				stickY /= 128.0f * 5.0f;

			CVector oldPlayerPos = playerPos;

			playerPos.z += -pad->NewState.LeftShoulder2 / 255.0f * 0.25f + pad->NewState.RightShoulder2 / 255.0f * 0.25f;
			playerPos.x += TheCamera.GetRight().x * -stickX + TheCamera.GetForward().x * -stickY;
			playerPos.y += TheCamera.GetRight().y * -stickX + TheCamera.GetForward().y * -stickY;

			lightPos += playerPos - oldPlayerPos;
		} else {
			CVector oldPlayerPos = playerPos;

			float mouseX = -(float)pad->GetMouseX() / 50.0f;
			float mouseY = (float)pad->GetMouseY() / 50.0f;
			if (pad->GetRightMouse()) {
				playerPos.z += mouseY;
			} else {
				playerPos.x += TheCamera.GetRight().x * mouseX + TheCamera.GetForward().x * mouseY;
				playerPos.y += TheCamera.GetRight().y * mouseX + TheCamera.GetForward().y * mouseY;
			}

			lightPos += playerPos - oldPlayerPos;
		}

		ChangePlayerPosition();
	}
}

void CPhotoMode::ProcessLights()
{
	if (!bDefaultLightEnabled && !bDarkLightEnabled)
		return;

	// Input
	CPad* pad = CPad::GetPad(0);
	if (bMenuOpen && currentTab == TAB_LIGHT && (pad->GetLeftMouse() || pad->IsAffectedByController && pad->GetCross())) {
		if (pad->IsAffectedByController) {
			float stickX = pad->NewState.LeftStickX;
			if (Abs(stickX) <= FrontEndMenuManager.m_PrefsLeftStickDeadzone)
				stickX = 0;
			else
				stickX /= 128.0f * 5.0f;

			float stickY = pad->NewState.LeftStickY;
			if (Abs(stickY) <= FrontEndMenuManager.m_PrefsLeftStickDeadzone)
				stickY = 0;
			else
				stickY /= 128.0f * 5.0f;

			lightPos.z += -pad->NewState.LeftShoulder2 / 255.0f * 0.25f + pad->NewState.RightShoulder2 / 255.0f * 0.25f;
			lightPos.x += TheCamera.GetRight().x * -stickX + TheCamera.GetForward().x * -stickY;
			lightPos.y += TheCamera.GetRight().y * -stickX + TheCamera.GetForward().y * -stickY;
		} else {
			float mouseX = -(float)pad->GetMouseX() / 50.0f;
			float mouseY = (float)pad->GetMouseY() / 50.0f;
			if (pad->GetRightMouse()) {
				lightPos.z += mouseY;
			} else {
				lightPos.x += TheCamera.GetRight().x * mouseX + TheCamera.GetForward().x * mouseY;
				lightPos.y += TheCamera.GetRight().y * mouseX + TheCamera.GetForward().y * mouseY;
			}
		}
	}

	// Render lights
	if (bDefaultLightEnabled) {
		CPointLights::AddLight(CPointLights::LIGHT_POINT,
							   lightPos, CVector(0.0f, 0.0f, 0.0f), 10.0f,
							   lightColorRed, lightColorGreen, lightColorBlue, CPointLights::FOG_NONE, false);
	}
	if (bDarkLightEnabled) {
		CPointLights::AddLight(CPointLights::LIGHT_DARKEN,
							   lightPos, CVector(0.0f, 0.0f, 0.0f), 10.0f,
							   1.0f, 1.0f, 1.0f, CPointLights::FOG_NONE, false);
	}
}

void CPhotoMode::DrawLightSprite()
{
	if (!bDefaultLightEnabled && !bDarkLightEnabled)
		return;

	RwV3d vecOut;

	float fDistX, fDistY;
	if (!CSprite::CalcScreenCoors(lightPos, &vecOut, &fDistX, &fDistY, false))
		return;

	int spriteAlpha = CWorld::GetIsLineOfSightClear(cameraPos, lightPos, true, true, true, true, true, true) ? 255 : 50;

	int textureFilterState;
	RwRenderStateGet(rwRENDERSTATETEXTUREFILTER, &textureFilterState);
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);
	CRadar::CentreSprite.Draw(CRect(vecOut.x + 32.0f, vecOut.y + 32.0f, vecOut.x - 32.0f, vecOut.y - 32.0f), CRGBA(255, 255, 255, spriteAlpha));
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)textureFilterState);
}

void CPhotoMode::DrawFrameEffects()
{
	if (frameEffectID == FRAME_EFFECT_NONE)
		return;

	switch (frameEffectID)
	{
		case FRAME_EFFECT_CUTSCENE:
			CSprite2d::DrawRect(CRect(0.0f, (SCREEN_HEIGHT/2) * 30.0f / 100.0f - SCREEN_SCALE_Y(8.0f), SCREEN_WIDTH, 0.0f), CRGBA(0, 0, 0, 255));
			CSprite2d::DrawRect(CRect(0.0f, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - (SCREEN_HEIGHT/2) * 30.0f / 100.0f - 8.0f), CRGBA(0, 0, 0, 255));
			break;
		case FRAME_EFFECT_ON_ALL_SIDES:
			CSprite2d::DrawRect(CRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_SCALE_Y(20.0f)), CRGBA(0, 0, 0, 255));
			CSprite2d::DrawRect(CRect(0.0f, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_SCALE_FROM_BOTTOM(20.0f)), CRGBA(0, 0, 0, 255));
			CSprite2d::DrawRect(CRect(0.0f, 0.0f, SCREEN_SCALE_X(20.0f), SCREEN_HEIGHT), CRGBA(0, 0, 0, 255));
			CSprite2d::DrawRect(CRect(SCREEN_WIDTH, 0.0f, SCREEN_SCALE_FROM_RIGHT(20.0f), SCREEN_HEIGHT), CRGBA(0, 0, 0, 255));
			break;
		default:
			break;
	}
}

char* CPhotoMode::GetTabName(int tab)
{
	char sName[32];

	switch (tab) {
		case TAB_CAMERA:
			sprintf(sName, "PM_CAM");
			break;
		case TAB_CHARACTER:
			sprintf(sName, "PM_CHAR");
			break;
		case TAB_SKY:
			sprintf(sName, "PM_SKY");
			break;
		case TAB_LIGHT:
			sprintf(sName, "PM_LGHT");
			break;
		case TAB_EFFECTS:
			sprintf(sName, "PM_EFFE");
			break;
		default:
			sprintf(sName, "");
			break;
	}

	return sName;
}

char* CPhotoMode::GetItemName(int item)
{
	char sName[32];

	if (currentTab == TAB_CAMERA) {
		switch (item) {
			case ITEM_CAMERA_FOV:
				sprintf(sName, "PM_ZOOM");
				break;
			case ITEM_CAMERA_TILT:
				sprintf(sName, "PM_TILT");
				break;
			case ITEM_CAMERA_NEAR_CLIP:
				sprintf(sName, "PM_NCP");
				break;
			default:
				sprintf(sName, "");
				break;
		}
	} else if (currentTab == TAB_CHARACTER) {
		switch (item) {
			case ITEM_CHAR_VISIBILITY:
				sprintf(sName, "PM_VSBL");
				break;
			case ITEM_CHAR_CLOTHES:
				sprintf(sName, "PM_CLTH");
				break;
			case ITEM_CHAR_ANIMATION:
				sprintf(sName, "PM_ANIM");
				break;
			case ITEM_CHAR_ANIMATION_FRAME:
				sprintf(sName, "PM_ANFR");
				break;
			case ITEM_CHAR_WEAPON:
				sprintf(sName, "PM_WEAP");
				break;
			case ITEM_CHAR_MUZZLE_FLASH:
				sprintf(sName, "PM_MUZL");
				break;
			case ITEM_CHAR_ROTATE:
				sprintf(sName, "PM_ROT");
				break;
			default:
				sprintf(sName, "");
				break;
		}
	} else if (currentTab == TAB_SKY) {
		switch (item) {
			case ITEM_SKY_SUN_MOON:
				sprintf(sName, "PM_SUN");
				break;
			case ITEM_SKY_HOURS:
				sprintf(sName, "PM_HOUR");
				break;
			case ITEM_SKY_MINUTES:
				sprintf(sName, "PM_MIN");
				break;
			case ITEM_SKY_WEATHER:
				sprintf(sName, "PM_WTHR");
				break;
			default:
				sprintf(sName, "");
				break;
		}
	} else if (currentTab == TAB_LIGHT) {
		switch (item) {
			case ITEM_LIGHT_DEFAULT_LIGHT:
				sprintf(sName, "PM_DEFL");
				break;
			case ITEM_LIGHT_COLOR_RED:
				sprintf(sName, "PM_COLR");
				break;
			case ITEM_LIGHT_COLOR_GREEN:
				sprintf(sName, "PM_COLG");
				break;
			case ITEM_LIGHT_COLOR_BLUE:
				sprintf(sName, "PM_COLB");
				break;
			case ITEM_LIGHT_DARK_LIGHT:
				sprintf(sName, "PM_DARK");
				break;
			default:
				sprintf(sName, "");
				break;
		}
	} else if (currentTab == TAB_EFFECTS) {
		switch (item) {
			case ITEM_EFFECT_COLORFILTER:
				sprintf(sName, "PM_CLFL");
				break;
			case ITEM_EFFECT_INTENSITY:
				sprintf(sName, "PM_INTS");
				break;
			case ITEM_EFFECT_BRIGHTNESS:
				sprintf(sName, "PM_BRTN");
				break;
			case ITEM_EFFECT_FRAMES:
				sprintf(sName, "PM_FRMS");
				break;
			default:
				sprintf(sName, "");
				break;
		}
	}

	return sName;
}

char* CPhotoMode::GetItemValue(int item)
{
	char sValue[32];

	if (currentTab == TAB_CAMERA) {
		switch (item) {
			case ITEM_CAMERA_FOV:
				sprintf(sValue, "%i", (int)cameraFOV);
				break;
			case ITEM_CAMERA_TILT:
				sprintf(sValue, "%.1f", cameraTilt);
				break;
			case ITEM_CAMERA_NEAR_CLIP:
				sprintf(sValue, "%.1f", cameraNearClip);
				break;
			default:
				sprintf(sValue, "");
				break;
		}
	} else if (currentTab == TAB_CHARACTER) {
		switch (item) {
			case ITEM_CHAR_VISIBILITY:
				if (bCharacterVisibility)
					sprintf(sValue, UnicodeToAscii(TheText.Get("FEM_ON")));
				else
					sprintf(sValue, UnicodeToAscii(TheText.Get("FEM_OFF")));
				break;
			case ITEM_CHAR_CLOTHES:
				if (clothesID == -1)
					sprintf(sValue, UnicodeToAscii(TheText.Get("FEM_NON")));
				else
					sprintf(sValue, "%i", clothesID + 1);
				break;
			case ITEM_CHAR_ANIMATION:
				if (animID == -1)
					sprintf(sValue, UnicodeToAscii(TheText.Get("FEM_NON")));
				else
					sprintf(sValue, "%i", animID + 1);
				break;
			case ITEM_CHAR_ANIMATION_FRAME:
				sprintf(sValue, "%.2f", animFrame);
				break;
			case ITEM_CHAR_WEAPON:
				if (weaponID == WEAPONTYPE_UNARMED)
					sprintf(sValue, UnicodeToAscii(TheText.Get("FEM_NON")));
				else
					sprintf(sValue, "%i", weaponID);
				break;
			case ITEM_CHAR_MUZZLE_FLASH:
				if (bMuzzleFlash)
					sprintf(sValue, UnicodeToAscii(TheText.Get("FEM_ON")));
				else
					sprintf(sValue, UnicodeToAscii(TheText.Get("FEM_OFF")));
				break;
			case ITEM_CHAR_ROTATE:
				sprintf(sValue, "%i", (int)RADTODEG(rotateAngle));
				break;
			default:
				sprintf(sValue, "");
				break;
		}
	} else if (currentTab == TAB_SKY) {
		switch (item) {
			case ITEM_SKY_SUN_MOON:
				sprintf(sValue, "%i", (int)RADTODEG(sunAndMoonAngle));
				break;
			case ITEM_SKY_HOURS:
				sprintf(sValue, "%i", CClock::GetHours());
				break;
			case ITEM_SKY_MINUTES:
				sprintf(sValue, "%i", CClock::GetMinutes());
				break;
			case ITEM_SKY_WEATHER:
				if (weatherID == WEATHER_SUNNY)
					sprintf(sValue, UnicodeToAscii(TheText.Get("PM_SNNY")));
				else if (weatherID == WEATHER_CLOUDY)
					sprintf(sValue, UnicodeToAscii(TheText.Get("PM_CLDY")));
				else if (weatherID == WEATHER_RAINY)
					sprintf(sValue, UnicodeToAscii(TheText.Get("PM_RANY")));
				else if (weatherID == WEATHER_FOGGY)
					sprintf(sValue, UnicodeToAscii(TheText.Get("PM_FGGY")));
				else if (weatherID == WEATHER_EXTRA_SUNNY)
					sprintf(sValue, UnicodeToAscii(TheText.Get("PM_SNN2")));
				else if (weatherID == WEATHER_HURRICANE)
					sprintf(sValue, UnicodeToAscii(TheText.Get("PM_HRCN")));
				break;
			default:
				sprintf(sValue, "");
				break;
		}
	} else if (currentTab == TAB_LIGHT) {
		switch (item) {
			case ITEM_LIGHT_DEFAULT_LIGHT:
				if (bDefaultLightEnabled)
					sprintf(sValue, UnicodeToAscii(TheText.Get("FEM_ON")));
				else
					sprintf(sValue, UnicodeToAscii(TheText.Get("FEM_OFF")));
				break;
			case ITEM_LIGHT_COLOR_RED:
				sprintf(sValue, "%.1f", lightColorRed);
				break;
			case ITEM_LIGHT_COLOR_GREEN:
				sprintf(sValue, "%.1f", lightColorGreen);
				break;
			case ITEM_LIGHT_COLOR_BLUE:
				sprintf(sValue, "%.1f", lightColorBlue);
				break;
			case ITEM_LIGHT_DARK_LIGHT:
				if (bDarkLightEnabled)
					sprintf(sValue, UnicodeToAscii(TheText.Get("FEM_ON")));
				else
					sprintf(sValue, UnicodeToAscii(TheText.Get("FEM_OFF")));
				break;
			default:
				sprintf(sValue, "");
				break;
		}
	} else if (currentTab == TAB_EFFECTS) {
		switch (item) {
			case ITEM_EFFECT_COLORFILTER:
				if (CPostFX::EffectSwitch == CPostFX::POSTFX_OFF)
					sprintf(sValue, UnicodeToAscii(TheText.Get("FEM_NON")));
				else if (CPostFX::EffectSwitch == CPostFX::POSTFX_SIMPLE)
					sprintf(sValue, UnicodeToAscii(TheText.Get("FEM_SIM")));
				else if (CPostFX::EffectSwitch == CPostFX::POSTFX_NORMAL)
					sprintf(sValue, UnicodeToAscii(TheText.Get("FEM_NRM")));
				else if (CPostFX::EffectSwitch == CPostFX::POSTFX_MOBILE)
					sprintf(sValue, UnicodeToAscii(TheText.Get("FEM_MOB")));
				break;
			case ITEM_EFFECT_INTENSITY:
				sprintf(sValue, "%.1f", CPostFX::Intensity);
				break;
			case ITEM_EFFECT_BRIGHTNESS:
				sprintf(sValue, "%.1f", (float)FrontEndMenuManager.m_PrefsBrightness / 511.0f);
				break;
			case ITEM_EFFECT_FRAMES:
				if (frameEffectID == 0)
					sprintf(sValue, UnicodeToAscii(TheText.Get("FEM_NON")));
				else
					sprintf(sValue, "%i", frameEffectID);
				break;
			default:
				sprintf(sValue, "");
				break;
		}
	}

	return sValue;
}

void CPhotoMode::InitFakePlayer()
{
	DeleteFakePlayer();

	if (clothesID != -1) {
		CStreaming::RequestSpecialModel(FindPlayerPed()->GetModelIndex(), clothesList[clothesID], STREAMFLAGS_DEPENDENCY | STREAMFLAGS_SCRIPTOWNED);

		CTimer::Suspend();
		CStreaming::LoadAllRequestedModels(false);
		CTimer::Resume();
	}

	fakePlayer = new CPed(PEDTYPE_SPECIAL);
	fakePlayer->m_modelIndex = FindPlayerPed()->GetModelIndex();
	fakePlayer->CreateRwObject();
	RpAnimBlendClumpInit(fakePlayer->GetClump());
	RpAnimBlendClumpFillFrameArray(fakePlayer->GetClump(), fakePlayer->m_pFrames);
	fakePlayer->SetPosition(FindPlayerCoors());
	fakePlayer->SetOrientation(0.0f, 0.0f, 0.0f);
	fakePlayer->SetHeading(rotateAngle);
	fakePlayer->GetMatrix().UpdateRW();
	fakePlayer->UpdateRwFrame();
	CWorld::Add(fakePlayer);

	CWeaponInfo* weaponInfo = CWeaponInfo::GetWeaponInfo((eWeaponType)weaponID);

	if (animList[animID][1] != ANIM_MELEE_ATTACK_2ND &&
		(animList[animID][0] >= ASSOCGRP_SCREWDRIVER && animList[animID][0] <= ASSOCGRP_RIFLE ||
		animList[animID][0] == ASSOCGRP_FLAMETHROWER
#ifdef FIRING_AND_AIMING // Animations in photo mode
		|| animList[animID][0] == ASSOCGRP_ROCKET
#endif
#ifdef EX_WEAPON_STEYR // Animations in photo mode
		|| animList[animID][0] == ASSOCGRP_STEYR
#endif
		)) {

		if (animList[animID][0] == ASSOCGRP_COLT)
			CAnimManager::AddAnimation(fakePlayer->GetClump(), ASSOCGRP_STD, ANIM_STD_IDLE);
		else
			CAnimManager::AddAnimation(fakePlayer->GetClump(), ASSOCGRP_STD, ANIM_STD_GUN_STAND);
	} else if (animList[animID][1] == ANIM_STD_PARTIAL_FUCKU || animID == -1) {
		CAnimManager::AddAnimation(fakePlayer->GetClump(), ASSOCGRP_STD, ANIM_STD_IDLE);
	}

	if (animID != -1) {
		CAnimBlendAssociation* playerAnim = CAnimManager::AddAnimation(fakePlayer->GetClump(), (AssocGroupId)animList[animID][0], (AnimationId)animList[animID][1]);
		if (animList[animID][1] != ANIM_STD_EVADE_DIVE) {
			animFrame = playerAnim->hierarchy->totalLength / 3.0f;
			playerAnim->SetCurrentTime(animFrame);
		} else {
			animFrame = 0.0f;
			playerAnim->SetCurrentTime(0.0f);
		}
	}

	RpAnimBlendClumpUpdateAnimations(fakePlayer->GetClump(), CTimer::GetTimeStepInSeconds());

	fakePlayer->AddWeaponModel(weaponInfo->m_nModelId);

	fakePlayer->bIsVisible = bCharacterVisibility;
}

void CPhotoMode::DeleteFakePlayer()
{
	if (!fakePlayer)
		return;

	CWorld::RemoveReferencesToDeletedObject(fakePlayer);
	delete fakePlayer;
	fakePlayer = nullptr;
}

void CPhotoMode::AddPlayerWeapon()
{
	if (weaponID == WEAPONTYPE_UNARMED) {
		CWeaponInfo* weaponInfo = FindPlayerPed()->GetWeapon()->GetInfo();
		if (weaponInfo->m_nModelId != -1) {
			FindPlayerPed()->AddWeaponModel(weaponInfo->m_nModelId);
		}

		return;
	}

	CWeaponInfo* weaponInfo = CWeaponInfo::GetWeaponInfo((eWeaponType)weaponID);

	CStreaming::RequestModel(weaponInfo->m_nModelId, STREAMFLAGS_DONT_REMOVE);
	CStreaming::LoadAllRequestedModels(false);

	FindPlayerPed()->AddWeaponModel(weaponInfo->m_nModelId);

	if (fakePlayer)
		fakePlayer->AddWeaponModel(weaponInfo->m_nModelId);

	CStreaming::SetModelIsDeletable(weaponInfo->m_nModelId);
}

void CPhotoMode::RemovePlayerWeapon()
{
	CWeaponInfo* weaponInfo = CWeaponInfo::GetWeaponInfo((eWeaponType)weaponID);

	FindPlayerPed()->RemoveWeaponModel(weaponInfo->m_nModelId);

	if (fakePlayer)
		fakePlayer->RemoveWeaponModel(weaponInfo->m_nModelId);
}

void CPhotoMode::CreateMuzzleFlash()
{
	CWeaponInfo* weaponInfo = CWeaponInfo::GetWeaponInfo((eWeaponType)weaponID);
	CVector fireSource = weaponInfo->m_vecFireOffset;
	fakePlayer->TransformToNode(fireSource, PED_HANDR);

	float heading = RADTODEG(fakePlayer->GetForward().Heading());
	float angle = DEGTORAD(heading);

	CVector2D ahead(-Sin(angle), Cos(angle));
	ahead.Normalise();

	CVector gunflashPos = fireSource;
	gunflashPos += CVector(0.06f * ahead.x, 0.06f * ahead.y, 0.0f);
	muzzleFlashParticles[0] = CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.10f);
	muzzleFlashParticles[1] = CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.10f);
}

void CPhotoMode::RemoveMuzzleParticles()
{
	for (int i = 0; i < MAX_MUZZLE_PARTICLES; i++) {
		if (muzzleFlashParticles[i] && muzzleFlashParticles[i]->m_nTimeWhenWillBeDestroyed > 0) {
			muzzleFlashParticles[i]->m_nTimeWhenWillBeDestroyed = 0;
			muzzleFlashParticles[i]->m_nAlpha = 0;
		}
	}
}

void CPhotoMode::ChangePlayerPosition()
{
	FindPlayerPed()->SetPosition(playerPos);
	FindPlayerPed()->GetMatrix().UpdateRW();
	FindPlayerPed()->UpdateRwFrame();

	if (fakePlayer) {
		fakePlayer->SetPosition(playerPos);
		fakePlayer->GetMatrix().UpdateRW();
		fakePlayer->UpdateRwFrame();
	}
}

void CPhotoMode::StoreWeatherState()
{
	storedInterpolationValue = CWeather::InterpolationValue;
	storedNewWeatherType = CWeather::NewWeatherType;
	storedOldWeatherType = CWeather::OldWeatherType;
	storedForcedWeatherType = CWeather::ForcedWeatherType;
	storedRain = CWeather::Rain;
}

void CPhotoMode::RestoreWeatherState()
{
	CWeather::InterpolationValue = storedInterpolationValue;
	CWeather::NewWeatherType = storedNewWeatherType;
	CWeather::OldWeatherType = storedOldWeatherType;
	CWeather::ForcedWeatherType = storedForcedWeatherType;
	CWeather::Rain = storedRain;
}

void CPhotoMode::RequestAnims()
{
	int screwdrvBlock = CAnimManager::GetAnimationBlockIndex("screwdrv");
	int golfclubBlock = CAnimManager::GetAnimationBlockIndex("golfclub");
	int baseballBlock = CAnimManager::GetAnimationBlockIndex("baseball");
	int knifeBlock = CAnimManager::GetAnimationBlockIndex("knife");
	int chainsawBlock = CAnimManager::GetAnimationBlockIndex("chainsaw");
	int grenadeBlock = CAnimManager::GetAnimationBlockIndex("grenade");
	int colt45Block = CAnimManager::GetAnimationBlockIndex("colt45");
	int pythonBlock = CAnimManager::GetAnimationBlockIndex("python");
	int shotgunBlock = CAnimManager::GetAnimationBlockIndex("shotgun");
	int buddyBlock = CAnimManager::GetAnimationBlockIndex("buddy");
	int uziBlock = CAnimManager::GetAnimationBlockIndex("uzi");
	int rifleBlock = CAnimManager::GetAnimationBlockIndex("rifle");
	int flameBlock = CAnimManager::GetAnimationBlockIndex("flame");
	int rocketBlock = CAnimManager::GetAnimationBlockIndex("rocket");
	int manBlock = CAnimManager::GetAnimationBlockIndex("man");
	CStreaming::RequestAnim(screwdrvBlock, STREAMFLAGS_DEPENDENCY);
	CStreaming::RequestAnim(golfclubBlock, STREAMFLAGS_DEPENDENCY);
	CStreaming::RequestAnim(baseballBlock, STREAMFLAGS_DEPENDENCY);
	CStreaming::RequestAnim(knifeBlock, STREAMFLAGS_DEPENDENCY);
	CStreaming::RequestAnim(chainsawBlock, STREAMFLAGS_DEPENDENCY);
	CStreaming::RequestAnim(grenadeBlock, STREAMFLAGS_DEPENDENCY);
	CStreaming::RequestAnim(colt45Block, STREAMFLAGS_DEPENDENCY);
	CStreaming::RequestAnim(pythonBlock, STREAMFLAGS_DEPENDENCY);
	CStreaming::RequestAnim(shotgunBlock, STREAMFLAGS_DEPENDENCY);
	CStreaming::RequestAnim(buddyBlock, STREAMFLAGS_DEPENDENCY);
	CStreaming::RequestAnim(uziBlock, STREAMFLAGS_DEPENDENCY);
	CStreaming::RequestAnim(rifleBlock, STREAMFLAGS_DEPENDENCY);
	CStreaming::RequestAnim(flameBlock, STREAMFLAGS_DEPENDENCY);
	CStreaming::RequestAnim(rocketBlock, STREAMFLAGS_DEPENDENCY);
	CStreaming::RequestAnim(manBlock, STREAMFLAGS_DEPENDENCY);
	CStreaming::LoadAllRequestedModels(false);
	CAnimManager::AddAnimBlockRef(screwdrvBlock);
	CAnimManager::AddAnimBlockRef(golfclubBlock);
	CAnimManager::AddAnimBlockRef(baseballBlock);
	CAnimManager::AddAnimBlockRef(knifeBlock);
	CAnimManager::AddAnimBlockRef(chainsawBlock);
	CAnimManager::AddAnimBlockRef(grenadeBlock);
	CAnimManager::AddAnimBlockRef(colt45Block);
	CAnimManager::AddAnimBlockRef(pythonBlock);
	CAnimManager::AddAnimBlockRef(shotgunBlock);
	CAnimManager::AddAnimBlockRef(buddyBlock);
	CAnimManager::AddAnimBlockRef(uziBlock);
	CAnimManager::AddAnimBlockRef(rifleBlock);
	CAnimManager::AddAnimBlockRef(flameBlock);
	CAnimManager::AddAnimBlockRef(rocketBlock);
	CAnimManager::AddAnimBlockRef(manBlock);
}

void CPhotoMode::RemoveAnims()
{
	CAnimManager::RemoveAnimBlockRef(CAnimManager::GetAnimationBlockIndex("screwdrv"));
	CAnimManager::RemoveAnimBlockRef(CAnimManager::GetAnimationBlockIndex("golfclub"));
	CAnimManager::RemoveAnimBlockRef(CAnimManager::GetAnimationBlockIndex("baseball"));
	CAnimManager::RemoveAnimBlockRef(CAnimManager::GetAnimationBlockIndex("knife"));
	CAnimManager::RemoveAnimBlockRef(CAnimManager::GetAnimationBlockIndex("chainsaw"));
	CAnimManager::RemoveAnimBlockRef(CAnimManager::GetAnimationBlockIndex("grenade"));
	CAnimManager::RemoveAnimBlockRef(CAnimManager::GetAnimationBlockIndex("colt45"));
	CAnimManager::RemoveAnimBlockRef(CAnimManager::GetAnimationBlockIndex("python"));
	CAnimManager::RemoveAnimBlockRef(CAnimManager::GetAnimationBlockIndex("shotgun"));
	CAnimManager::RemoveAnimBlockRef(CAnimManager::GetAnimationBlockIndex("buddy"));
	CAnimManager::RemoveAnimBlockRef(CAnimManager::GetAnimationBlockIndex("uzi"));
	CAnimManager::RemoveAnimBlockRef(CAnimManager::GetAnimationBlockIndex("rifle"));
	CAnimManager::RemoveAnimBlockRef(CAnimManager::GetAnimationBlockIndex("flame"));
	CAnimManager::RemoveAnimBlockRef(CAnimManager::GetAnimationBlockIndex("rocket"));
	CAnimManager::RemoveAnimBlockRef(CAnimManager::GetAnimationBlockIndex("man"));
}

void CPhotoMode::DrawMenuAndEffects()
{
	DrawFrameEffects();

	if (bPlayerWantsToTakePhoto)
		return;

	DrawHints();

	if (!bMenuOpen)
		return;

	DrawLightSprite();

	const float startX = 25.0f;
	const float startY = 160.0f;
	const float endX = 150.0f + startY;
	const float endY = 20.0f + startY;

	// Black header
	CSprite2d::DrawRect(CRect(SCREEN_SCALE_FROM_RIGHT(startX), SCREEN_SCALE_FROM_BOTTOM(startY), SCREEN_SCALE_FROM_RIGHT(endX), SCREEN_SCALE_FROM_BOTTOM(endY)), CRGBA(0, 0, 0, 225));

	const float tabScaleX = 0.8f * 0.75f;
	const float tabScaleY = 1.35f * 0.75f;

	// Header buttons
	if (CPad::GetPad(0)->IsAffectedByController) {
		CFont::SetScale(SCREEN_SCALE_X(tabScaleX), SCREEN_SCALE_Y(tabScaleY));

		// Left button
		CFont::PS2Symbol = BUTTON_L1;
		CFont::DrawButton(SCREEN_SCALE_FROM_RIGHT(endX - 4.5f), SCREEN_SCALE_FROM_BOTTOM(endY + 1.0f));

		// Right button
		CFont::PS2Symbol = BUTTON_R1;
		CFont::DrawButton(SCREEN_SCALE_FROM_RIGHT(startX + 22.0f), SCREEN_SCALE_FROM_BOTTOM(endY + 1.0f));

		CFont::PS2Symbol = BUTTON_NONE;
	} else {
		CFont::SetScale(SCREEN_SCALE_X(0.8f * 0.75f), SCREEN_SCALE_Y(1.35f * 0.75f));

		// Left button
		CFont::PCSymbol = 'Q' - 1;
		CFont::DrawPCKey(SCREEN_SCALE_FROM_RIGHT(endX - 2.0f), SCREEN_SCALE_FROM_BOTTOM(endY + 1.0f));
		CFont::PCSymbol = BUTTON_NONE;

		// Right button
		CFont::PCSymbol = 'E' - 1;
		CFont::DrawPCKey(SCREEN_SCALE_FROM_RIGHT(startX + 19.0f), SCREEN_SCALE_FROM_BOTTOM(endY + 1.0f));
		CFont::PCSymbol = BUTTON_NONE;
	}

	char sTemp[8];
	char* sName;

	// Current tab name
	sName = GetTabName(currentTab);
	strcpy(sTemp, sName);
	CFont::SetPropOn();
	CFont::SetBackgroundOff();
	CFont::SetScale(SCREEN_SCALE_X(tabScaleX), SCREEN_SCALE_Y(tabScaleY));
	CFont::SetRightJustifyOff();
	CFont::SetRightJustifyWrap(0.0f);
	CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
	CFont::SetBackGroundOnlyTextOn();
	CFont::SetCentreOn();
	CFont::SetColor(CRGBA(DEFAULT_TEXT_COLOR));
	CFont::PrintString(SCREEN_SCALE_FROM_RIGHT((endX + startX) / 2.0f), SCREEN_SCALE_FROM_BOTTOM(endY + 2.0f), TheText.Get(sTemp));

	// Black field (under header)
	CSprite2d::DrawRect(CRect(SCREEN_SCALE_FROM_RIGHT(startX), SCREEN_SCALE_FROM_BOTTOM(endY - startY), SCREEN_SCALE_FROM_RIGHT(endX), SCREEN_SCALE_FROM_BOTTOM(startY)), CRGBA(0, 0, 0, 175));

	// Items
	const float itemScaleX = 0.8f * 0.5f;
	const float itemScaleY = 1.35f * 0.5f;
	float offsetItemY = 2.0f;
	for (int i = 0; i < maxItems; i++) {
		// Highlight
		if (currentItem == i) {
			CSprite2d::DrawRect(CRect(SCREEN_SCALE_FROM_RIGHT(endX), SCREEN_SCALE_FROM_BOTTOM(startY - (offsetItemY - 2.0f)), 
								SCREEN_SCALE_FROM_RIGHT(startX), SCREEN_SCALE_FROM_BOTTOM(startY - 20.0f - (offsetItemY - 2.0f))),
								CRGBA(50, 50, 50, 150));
		}

		// Name
		sName = GetItemName(i);
		strcpy(sTemp, sName);
		CFont::SetPropOn();
		CFont::SetBackgroundOff();
		CFont::SetScale(SCREEN_SCALE_X(itemScaleX), SCREEN_SCALE_Y(itemScaleY));
		CFont::SetRightJustifyOff();
		CFont::SetRightJustifyWrap(0.0f);
		CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
		CFont::SetBackGroundOnlyTextOn();
		CFont::SetCentreOff();
		CFont::SetColor(currentItem == i ? CRGBA(SELECTED_TEXT_COLOR) : CRGBA(DEFAULT_TEXT_COLOR));
		CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(endX - 4.0f), SCREEN_SCALE_FROM_BOTTOM(startY - offsetItemY), TheText.Get(sTemp));

		// Value
		wchar sPrint[32];
		AsciiToUnicode(GetItemValue(i), sPrint);
		CFont::SetPropOn();
		CFont::SetBackgroundOff();
		CFont::SetScale(SCREEN_SCALE_X(itemScaleX), SCREEN_SCALE_Y(itemScaleY));
		CFont::SetRightJustifyOff();
		CFont::SetRightJustifyWrap(0.0f);
		CFont::SetFontStyle(FONT_LOCALE(FONT_HEADING));
		CFont::SetBackGroundOnlyTextOn();
		CFont::SetCentreOff();
		CFont::SetColor(currentItem == i ? CRGBA(SELECTED_TEXT_COLOR) : CRGBA(DEFAULT_TEXT_COLOR));
		CFont::PrintString(SCREEN_SCALE_FROM_RIGHT(endX - 190.0f), SCREEN_SCALE_FROM_BOTTOM(startY - offsetItemY), sPrint);

		offsetItemY += 20.0f;
	}
}

#endif