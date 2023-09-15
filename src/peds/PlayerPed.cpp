#include "common.h"

#include "RwHelper.h"
#include "PlayerPed.h"
#include "Wanted.h"
#include "Fire.h"
#include "DMAudio.h"
#include "Pad.h"
#include "Camera.h"
#include "WeaponEffects.h"
#include "ModelIndices.h"
#include "World.h"
#include "RpAnimBlend.h"
#include "AnimBlendAssociation.h"
#include "General.h"
#include "Pools.h"
#include "Darkel.h"
#include "CarCtrl.h"
#include "MBlur.h"
#include "Streaming.h"
#include "Population.h"
#include "Script.h"
#include "Replay.h"
#include "PedPlacement.h"
#include "VarConsole.h"
#include "SaveBuf.h"
#ifdef SWIMMING
#include "Floater.h"
#include "Particle.h"
#endif
#ifdef IMPROVED_MENU_AND_INPUT
#include "Frontend.h"
#endif
#ifdef IMPROVED_TECH_PART // skiped phone calls
#include "AudioManager.h"
#endif
#ifdef AUTOSAVE_AND_SAVE_ANYWHERE // TEMPORARY
#include "GenericGameStorage.h"
#endif

#define PAD_MOVE_TO_GAME_WORLD_MOVE 60.0f

bool CPlayerPed::bDontAllowWeaponChange;
#ifndef MASTER
bool CPlayerPed::bDebugPlayerInfo;
#endif

#ifdef NEW_CHEATS // init
bool CPlayerPed::bInvincibleCheat;
bool CPlayerPed::bNoWantedCheat;
bool CPlayerPed::bRCRocketCheat;
#endif

const uint32 CPlayerPed::nSaveStructSize =
#ifdef COMPATIBLE_SAVES
	1752;
#else
	sizeof(CPlayerPed);
#endif

int32 idleAnimBlockIndex;

CPad*
GetPadFromPlayer(CPlayerPed*)
{
	return CPad::GetPad(0);
}

CPlayerPed::~CPlayerPed()
{
	delete m_pWanted;
}

CPlayerPed::CPlayerPed(void) : CPed(PEDTYPE_PLAYER1)
{
	m_fMoveSpeed = 0.0f;
	SetModelIndex(MI_PLAYER);
#ifdef FIX_BUGS
	m_fCurrentStamina = m_fMaxStamina = 150.0f;
#endif
	SetInitialState();

	m_pWanted = new CWanted();
	m_pWanted->Initialise();
	m_pArrestingCop = nil;
	m_currentWeapon = WEAPONTYPE_UNARMED;
	m_nSelectedWepSlot = WEAPONSLOT_UNARMED;
	m_nSpeedTimer = 0;
	m_bSpeedTimerFlag = false;
	SetWeaponLockOnTarget(nil);
	SetPedState(PED_IDLE);
#ifndef FIX_BUGS
	m_fCurrentStamina = m_fMaxStamina = 150.0f;
#endif
	m_fStaminaProgress = 0.0f;
	m_nEvadeAmount = 0;
	m_pEvadingFrom = nil;
	m_nHitAnimDelayTimer = 0;
	m_fAttackButtonCounter = 0.0f;
	m_bHaveTargetSelected = false;
	m_bHasLockOnTarget = false;
	m_bCanBeDamaged = true;
	m_bNoPosForMeleeAttack = false;
	m_fWalkAngle = 0.0f;
	m_fFPSMoveHeading = 0.0f;
	m_pMinigunTopAtomic = nil;
	m_fGunSpinSpeed = 0.0;
	m_fGunSpinAngle = 0.0;
	m_nPadDownPressedInMilliseconds = 0;
	m_nTargettableObjects[0] = m_nTargettableObjects[1] = m_nTargettableObjects[2] = m_nTargettableObjects[3] = -1;
	unk1 = false;
	for (int i = 0; i < 6; i++) {
		m_vecSafePos[i] = CVector(0.0f, 0.0f, 0.0f);
		m_pPedAtSafePos[i] = nil;
		m_pMeleeList[i] = nil;
	}
	m_nAttackDirToCheck = 0;
	m_nLastBusFareCollected = 0;
	idleAnimBlockIndex = CAnimManager::GetAnimationBlockIndex("playidles");
#ifdef FREE_CAM
	m_bFreeAimActive = false;
#endif

#ifdef FIRING_AND_AIMING
	bIsPlayerAiming = false;
#endif

#ifdef SWIMMING
	bIsTiredSwimmingFast = false;
#endif

#ifdef IMPROVED_MENU_AND_INPUT
	bIsAutoAiming = false;
#endif

#ifdef IMPROVED_TECH_PART // skiped phone calls
	m_bSkipPhoneCall = false;
#endif

#ifdef FEATURES_INI // HealthRegenerationUpToHalf
	m_nHealthRegenerationTime = 0;
#endif
}

void
CPlayerPed::ClearWeaponTarget()
{
#ifdef IMPROVED_MENU_AND_INPUT
	bIsAutoAiming = false;
#endif
	if (m_nPedType == PEDTYPE_PLAYER1) {
		SetWeaponLockOnTarget(nil);
		TheCamera.ClearPlayerWeaponMode();
		CWeaponEffects::ClearCrossHair();
	}
 	ClearPointGunAt();
}

void
CPlayerPed::SetWantedLevel(int32 level)
{
	m_pWanted->SetWantedLevel(level);
}

void
CPlayerPed::SetWantedLevelNoDrop(int32 level)
{
	m_pWanted->SetWantedLevelNoDrop(level);
}

void
CPlayerPed::MakeObjectTargettable(int32 handle)
{
	for (int i = 0; i < ARRAY_SIZE(m_nTargettableObjects); i++) {
		if (CPools::GetObjectPool()->GetAt(m_nTargettableObjects[i]) == nil) {
			m_nTargettableObjects[i] = handle;
			return;
		}
	}
}

// I don't know the actual purpose of parameter
void
CPlayerPed::AnnoyPlayerPed(bool annoyedByPassingEntity)
{
	if (m_pedStats->m_temper < 52) {
		m_pedStats->m_temper++;
	} else if (annoyedByPassingEntity && m_pedStats->m_temper < 55) {
		m_pedStats->m_temper++;
	} else if (annoyedByPassingEntity) {
		m_pedStats->m_temper = 46;
	}
}

void
CPlayerPed::ClearAdrenaline(void)
{
	if (m_bAdrenalineActive && m_nAdrenalineTime != 0) {
		m_nAdrenalineTime = 0;
		CTimer::SetTimeScale(1.0f);
	}
}

CPlayerInfo *
CPlayerPed::GetPlayerInfoForThisPlayerPed()
{
	if (CWorld::Players[0].m_pPed == this)
		return &CWorld::Players[0];

	return nil;
}

void
CPlayerPed::SetupPlayerPed(int32 index)
{
	CPlayerPed *player = new CPlayerPed();
	CWorld::Players[index].m_pPed = player;
#ifdef FIX_BUGS
	player->RegisterReference((CEntity**)&CWorld::Players[index].m_pPed);
#endif

	player->SetOrientation(0.0f, 0.0f, 0.0f);

	CWorld::Add(player);
	player->m_wepAccuracy = 100;

#ifndef MASTER
	VarConsole.Add("Debug PlayerPed", &CPlayerPed::bDebugPlayerInfo, true);
	VarConsole.Add("Tweak Vehicle Handling", &CVehicle::m_bDisplayHandlingInfo, true);
#endif
}

void
CPlayerPed::DeactivatePlayerPed(int32 index)
{
	CWorld::Remove(CWorld::Players[index].m_pPed);
}

void
CPlayerPed::ReactivatePlayerPed(int32 index)
{
	CWorld::Add(CWorld::Players[index].m_pPed);
}

#ifdef CROUCH
void CPlayerPed::ProcessCrouch(void)
{
	if (!bIsDucking)
		return;

	CAnimBlendAssociation* curCrouchIdleAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CROUCH_IDLE);
	CAnimBlendAssociation* curCrouchForwardAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CROUCH_FORWARD);
	CAnimBlendAssociation* curCrouchBackwardAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CROUCH_BACKWARD);
	CAnimBlendAssociation* curCrouchRollLeftAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CROUCH_ROLL_L);
	CAnimBlendAssociation* curCrouchRollRightAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CROUCH_ROLL_R);
	bool bIsCurRollAnim = curCrouchRollLeftAssoc || curCrouchRollRightAssoc;
	bool bIsCurMoveAnim = curCrouchForwardAssoc || curCrouchBackwardAssoc;

	CAnimBlendAssociation* curCrouchFireAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_CROUCHFIRE);

	CPad* pad = GetPadFromPlayer(this);
	int16 padUpDown = pad->GetPedWalkUpDown();
	int16 padLeftRight = pad->GetPedWalkLeftRight();

	if (curCrouchForwardAssoc)
		curCrouchForwardAssoc->speed = Clamp(m_fMoveSpeed - 0.5f, 0.5f, 1.0f);

	if (curCrouchBackwardAssoc)
		curCrouchBackwardAssoc->speed = Clamp(m_fMoveSpeed - 0.5f, 0.5f, 1.0f);

	if (m_nPedState == PED_ROLL)
	{
		if (!bIsCurRollAnim)
			RestorePreviousState();

		return;
	}

	if (bIsPlayerAiming && GetWeapon()->GetInfo()->m_nWeaponSlot == WEAPONSLOT_SNIPER)
		return;

	if (bIsPlayerAiming || TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_REAL_1ST_PERSON) {
		TheCamera.Cams[TheCamera.ActiveCam].m_fTargetCameraPosZ = 0.4f;

		if ((m_fMoveSpeed < 0.1f || (padLeftRight != 0 && padUpDown == 0)) && !curCrouchIdleAssoc && !bIsCurRollAnim || m_nPedState == PED_ROLL)
			curCrouchIdleAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CROUCH_IDLE, 4.0f);

		if (pad->GetTarget() && m_fMoveSpeed > 0.1f && (!curCrouchIdleAssoc || curCrouchIdleAssoc && curCrouchIdleAssoc->blendAmount >= 1.0f) && !bIsCurMoveAnim && !bIsCurRollAnim && 
			(padLeftRight < -20 || padLeftRight > 20)) {

			if (curCrouchForwardAssoc)
				delete curCrouchForwardAssoc;

			if (curCrouchBackwardAssoc)
				delete curCrouchBackwardAssoc;

			if (curCrouchFireAssoc) {
				delete curCrouchFireAssoc;
				ClearWeaponTarget();
			}

			float angleToFace;
			angleToFace = CGeneral::GetRadianAngleBetweenPoints(
				GetPosition().x - TheCamera.GetRight().x, GetPosition().y - TheCamera.GetRight().y,
				GetPosition().x, GetPosition().y);
			angleToFace = CGeneral::LimitRadianAngle(angleToFace);
			m_fRotationCur = CGeneral::LimitRadianAngle(m_fRotationCur);
			m_fRotationCur = angleToFace;

			bDontAllowWeaponChange = true;

			SetPedState(PED_ROLL);

			delete curCrouchIdleAssoc;

			if (padLeftRight < 0) {
				curCrouchRollLeftAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CROUCH_ROLL_L, 4.0f);
				curCrouchRollLeftAssoc->SetFinishCallback(FinishRollCB, this);
			} else if (padLeftRight > 0) {
				curCrouchRollRightAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CROUCH_ROLL_R, 4.0f);
				curCrouchRollRightAssoc->SetFinishCallback(FinishRollCB, this);
			}

			return;
		}

		if (m_fMoveSpeed > 0.1 && !bIsCurRollAnim) {
			if (padUpDown != 0) {
				if (padUpDown < 0) {
					if (!curCrouchForwardAssoc)
						curCrouchForwardAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CROUCH_FORWARD, 4.0f);
				} else if (padUpDown > 0) {
					if (!curCrouchBackwardAssoc)
						curCrouchBackwardAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CROUCH_BACKWARD, 4.0f);
				}
			}
		}
	} else if (pad) {
		if (m_fMoveSpeed < 0.1f) {
			TheCamera.Cams[TheCamera.ActiveCam].m_fTargetCameraPosZ = 0.6f;
		
			if (!curCrouchIdleAssoc)
				curCrouchIdleAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CROUCH_IDLE, 4.0f);
		}

		if (m_fMoveSpeed > 0.1f) {
			TheCamera.Cams[TheCamera.ActiveCam].m_fTargetCameraPosZ = 0.3f;

			if (curCrouchIdleAssoc && !curCrouchForwardAssoc || curCrouchBackwardAssoc)
				curCrouchForwardAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CROUCH_FORWARD, 4.0f);
		}
	}
}

void CPlayerPed::FinishRollCB(CAnimBlendAssociation* assoc, void* arg)
{
	assoc->blendDelta = -1000.0f;

	bDontAllowWeaponChange = false;

	CPlayerPed* ped = (CPlayerPed*)arg;

	CAnimManager::BlendAnimation(ped->GetClump(), ASSOCGRP_STD, ANIM_STD_CROUCH_IDLE, 1000.0f);
}
#endif

#ifdef SWIMMING
void CPlayerPed::ProcessSwimming(void)
{
	if (!bIsSwimming)
		return;

	if (bIsPlayerAiming)
		bIsPlayerAiming = false;

	if (m_pFire)
		m_pFire->Extinguish();

	AnimationId otherAnims[] = { ANIM_STD_ROLLOUT_LHS, ANIM_STD_ROLLOUT_RHS, ANIM_STD_FALL, ANIM_STD_FALL_GLIDE, 
								   ANIM_STD_JUMP_GLIDE, ANIM_STD_JUMP_LAUNCH, ANIM_STD_FALL_ONBACK, ANIM_STD_FALL_ONBACK };
	for (int i = 0; i < ARRAY_SIZE(otherAnims); i++) {
		CAnimBlendAssociation* curAnimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), otherAnims[i]);
		if (curAnimAssoc) {
			curAnimAssoc->flags |= ASSOC_DELETEFADEDOUT;
			curAnimAssoc->blendDelta = -4.0f;
		}
	}

	CAnimBlendAssociation* curSwimBreastAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_SWIM_BREAST);
	CAnimBlendAssociation* curSwimCrawlAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_SWIM_CRAWL);
	CAnimBlendAssociation* curSwimTreadAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_SWIM_TREAD);

	SetPosition(CVector(GetPosition().x, GetPosition().y, mod_Buoyancy.m_waterlevel - 0.41f));

	CPad* pad = GetPadFromPlayer(this);
	int16 padUpDown = pad->GetPedWalkUpDown();
	int16 padLeftRight = pad->GetPedWalkLeftRight();

	if (m_fMoveSpeed < 0.1f && !curSwimTreadAssoc)
		curSwimTreadAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_SWIM_TREAD, 4.0f);

	float magnitude = 0.05f;
	if (pad->GetSprint() && (padLeftRight != 0 || padUpDown != 0)) {
		if (m_fCurrentStamina <= -150.0f)
			bIsTiredSwimmingFast = true;

		if (!bIsTiredSwimmingFast) {
			UseSprintEnergy();

			magnitude = 0.1f;
		}
	}
	
	if ((!pad->GetSprint() || curSwimTreadAssoc) && m_fCurrentStamina < 0.0f)
		bIsTiredSwimmingFast = true;

	if (m_fMoveSpeed > 0.1f && (padLeftRight != 0 || padUpDown != 0)) {
		if (pad->GetSprint() && !bIsTiredSwimmingFast && !curSwimCrawlAssoc)
			curSwimCrawlAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_SWIM_CRAWL, 4.0f);
		else if ((!pad->GetSprint() || bIsTiredSwimmingFast) && !curSwimBreastAssoc)
			curSwimBreastAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_SWIM_BREAST, 4.0f);

		if (m_vecMoveSpeed.Magnitude2D() < magnitude)
			ApplyMoveForce(GetForward() * 0.7f);
	}

	ApplyMoveForce(CVector(-m_vecMoveSpeed.x * 3.0f, -m_vecMoveSpeed.y * 3.0f, 0.0f));

	if (!curSwimBreastAssoc && curSwimCrawlAssoc) {
		DMAudio.PlayOneShot(m_audioEntityId, SOUND_MOVING_IN_WATER, 0.0f);

		RwRGBA waterSprayCol = { 48, 48, 64, 0 };
		CVector waterSprayPos = GetPosition() + GetForward() * 0.25f + CVector(0.0f, 0.0f, 1.0f);
		CParticle::AddParticle(PARTICLE_WATERSPRAY, waterSprayPos, -m_vecMoveSpeed, nil, 0.6f, waterSprayCol);
	}

	if (pad->JumpJustDown()) {
		CEntity* hitEntity;
		CColPoint hitPoint;
		bool isHit = CWorld::ProcessLineOfSight(GetPosition() + CVector(0.0f, 0.0f, 0.5f), GetPosition() + CVector(0.0f, 0.0f, 0.5f) + GetForward() * 1.5f, hitPoint, hitEntity, false, true, false, false, false, true);
		if (!isHit)
			isHit = CWorld::ProcessLineOfSight(GetPosition() + CVector(0.0f, 0.0f, 0.75f), GetPosition() + CVector(0.0f, 0.0f, 0.75f) + GetForward() * 1.5f, hitPoint, hitEntity, false, true, false, false, false, true);
		
		if (isHit && CWorld::GetIsLineOfSightClear(GetPosition(), GetPosition() + CVector(0.0f, 0.0f, 3.0f), false, true, false, false, false, true)) {
			CVehicle* hitVehicle = (CVehicle*)hitEntity;
			if (hitVehicle->IsBoat()) {
				CVector startPos = hitPoint.point + GetForward() * 0.5f + CVector(0.0f, 0.0f, 3.0f);
				CVector endPos = startPos - CVector(0.0f, 0.0f, 3.0f);
				if (CWorld::ProcessLineOfSight(startPos, endPos, hitPoint, hitEntity, false, true, false, false, false, true)) {
					bIsSwimming = false;
					bAffectedByGravity = true;

					SetPosition(hitPoint.point);

					AddWeaponModel(GetWeapon()->GetInfo()->m_nModelId);

					return;
				}
			}
		}

		CColPoint hitForwardPoint;
		CColPoint hitBackwardPoint;
		CColPoint hitJumpBPoint;
		if (CanPedClimbingThis(hitForwardPoint, hitBackwardPoint, hitJumpBPoint)) {
			if (curSwimBreastAssoc) {
				delete curSwimBreastAssoc;
			}

			if (curSwimCrawlAssoc) {
				delete curSwimCrawlAssoc;
			}

			if (curSwimTreadAssoc) {
				delete curSwimTreadAssoc;
			}

			bIsSwimming = false;
			bAffectedByGravity = true;

			AddWeaponModel(GetWeapon()->GetInfo()->m_nModelId);

			StartClimbing(hitForwardPoint, hitBackwardPoint, hitJumpBPoint);
		}
	}
}
#endif

#ifdef FIRING_AND_AIMING
void CPlayerPed::ProcessAiming(void)
{
	if (RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_STARTWALK))
		delete RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_STARTWALK);

	CAnimBlendAssociation* curGunMoveBackwardAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_GUNMOVE_BWD);
	CAnimBlendAssociation* curGunMoveForwardAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_GUNMOVE_FWD);
	CAnimBlendAssociation* curGunMoveLeftAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_GUNMOVE_L);
	CAnimBlendAssociation* curGunMoveRightAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_GUNMOVE_R);
	CAnimBlendAssociation* curGunStandAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_GUN_STAND);

	CPad* pad = GetPadFromPlayer(this);
	int16 padUpDown = pad->GetPedWalkUpDown();
	int16 padLeftRight = pad->GetPedWalkLeftRight();

	if (padLeftRight == 0 && padUpDown == 0 && !curGunStandAssoc)
		curGunStandAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_GUN_STAND, 4.0f);

	float animSpeed;

	if (pad->GetSprint() && (padLeftRight != 0 || padUpDown != 0))
		animSpeed = 1.4f;
	else if (IsDoomMode())
		animSpeed = Clamp((m_fMoveSpeed * 3.5f) - 0.1f, 0.1f, 3.5f);
	else
		animSpeed = Clamp(m_fMoveSpeed - 0.1f, 0.1f, 1.0f);

	if (curGunMoveBackwardAssoc)
		curGunMoveBackwardAssoc->speed = animSpeed;

	if (curGunMoveForwardAssoc)
		curGunMoveForwardAssoc->speed = animSpeed;

	if (curGunMoveLeftAssoc)
		curGunMoveLeftAssoc->speed = animSpeed;

	if (curGunMoveRightAssoc)
		curGunMoveRightAssoc->speed = animSpeed;

	if (padLeftRight < 0 && (Abs(padLeftRight) > 64 || Abs(padUpDown) < 64))
		curGunMoveLeftAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_GUNMOVE_L, 16.0f);
	else if (padLeftRight > 0 && (Abs(padLeftRight) > 64 || Abs(padUpDown) < 64))
		curGunMoveRightAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_GUNMOVE_R, 16.0f);

	if (padUpDown != 0) {
		if (padUpDown < 0) {
			if (!curGunMoveForwardAssoc)
				curGunMoveForwardAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_GUNMOVE_FWD, 16.0f);
		} else if (padUpDown > 0) {
			if (!curGunMoveBackwardAssoc)
				curGunMoveBackwardAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_GUNMOVE_BWD, 16.0f);
		}
	}
}

void CPlayerPed::StopAimingAnims(void)
{
	AnimationId gunAnims[] = { ANIM_STD_GUNMOVE_BWD, ANIM_STD_GUNMOVE_FWD, ANIM_STD_GUNMOVE_L, ANIM_STD_GUNMOVE_R };
	for (int i = 0; i < ARRAY_SIZE(gunAnims); i++) {
		CAnimBlendAssociation* curGunAnimAssoc = RpAnimBlendClumpGetAssociation(GetClump(), gunAnims[i]);
		if (curGunAnimAssoc) {
			curGunAnimAssoc->flags |= ASSOC_DELETEFADEDOUT;
			curGunAnimAssoc->blendDelta = -4.0f;
		}
	}
}

bool CPlayerPed::CanUseDriveBy(void)
{
	if (!FindPlayerVehicle())
		return false;

	CVehicle* vehicle = FindPlayerVehicle();

	bool bSuitableVehicle = !vehicle->IsRealHeli() && !vehicle->IsRealPlane() &&
		vehicle->GetModelIndex() != MI_RHINO && vehicle->GetModelIndex() != MI_FIRETRUCK;

	CCam currentCam = TheCamera.Cams[TheCamera.ActiveCam];
	bool bSuitableCamera = currentCam.DirectionWasLooking == LOOKING_FORWARD &&
		(currentCam.Mode == CCam::MODE_REAL_1ST_PERSON || currentCam.Mode == CCam::MODE_CAM_ON_A_STRING || currentCam.Mode == CCam::MODE_BEHINDBOAT);

	int8 currentWeaponSlot = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_nWeaponSlot;
	bool bHaveWeapon = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_nWeaponSlot > 2 &&
		(GetWeapon()->m_eWeaponType == WEAPONTYPE_COLT45 || currentWeaponSlot == WEAPONSLOT_SUBMACHINEGUN);

	return bSuitableVehicle && bHaveWeapon && GetUp().z >= 0.2f && bSuitableCamera;
}

bool CPlayerPed::IsDoomMode(void)
{
	return TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_REAL_1ST_PERSON && FrontEndMenuManager.m_PrefsDoomMode_FP && !InVehicle();
}
#endif

void
CPlayerPed::UseSprintEnergy(void)
{
	if (m_fCurrentStamina > -150.0f && !CWorld::Players[CWorld::PlayerInFocus].m_bInfiniteSprint
		&& !m_bAdrenalineActive) {
		m_fCurrentStamina = m_fCurrentStamina - CTimer::GetTimeStep();
		m_fStaminaProgress = m_fStaminaProgress + CTimer::GetTimeStep();
	}

	if (m_fStaminaProgress >= 500.0f) {
		m_fStaminaProgress = 0;
		if (m_fMaxStamina < 1000.0f)
			m_fMaxStamina += 10.0f;
	}
}

void
CPlayerPed::MakeChangesForNewWeapon(eWeaponType weapon)
{
	if (m_nPedState == PED_SNIPER_MODE) {
		RestorePreviousState();
		TheCamera.ClearPlayerWeaponMode();
	}
	SetCurrentWeapon(weapon);
	m_nSelectedWepSlot = m_currentWeapon;

#ifndef IMPROVED_MENU_AND_INPUT // Disabling automatic reloading when changing weapons (thanks to VitalRus)
	GetWeapon()->m_nAmmoInClip = Min(GetWeapon()->m_nAmmoTotal, CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_nAmountofAmmunition);
#endif

#if defined FIRING_AND_AIMING && defined FIRST_PERSON
	if (IsDoomMode() && GetWeapon()->GetInfo()->IsFlagSet(WEAPONFLAG_CANAIM_WITHARM))
		StopAimingAnims();

	if (CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->IsFlagSet(WEAPONFLAG_CANAIM) || IsDoomMode())
#else
	if (CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->IsFlagSet(WEAPONFLAG_CANAIM))
#endif
		ClearWeaponTarget();

	// WEAPONTYPE_SNIPERRIFLE? Wut?
	CAnimBlendAssociation* weaponAnim = RpAnimBlendClumpGetAssociation(GetClump(), GetPrimaryFireAnim(CWeaponInfo::GetWeaponInfo(WEAPONTYPE_SNIPERRIFLE)));
	if (weaponAnim) {
		weaponAnim->SetRun();
		weaponAnim->flags |= ASSOC_FADEOUTWHENDONE;
	}
	TheCamera.ClearPlayerWeaponMode();
}

void
CPlayerPed::MakeChangesForNewWeapon(int32 slot)
{
	if(slot != -1)
		MakeChangesForNewWeapon(m_weapons[slot].m_eWeaponType);
}

void
CPlayerPed::ReApplyMoveAnims(void)
{
	static AnimationId moveAnims[] = { ANIM_STD_WALK, ANIM_STD_RUN, ANIM_STD_RUNFAST, ANIM_STD_IDLE, ANIM_STD_STARTWALK };

	for(int i = 0; i < ARRAY_SIZE(moveAnims); i++) {
		CAnimBlendAssociation *curMoveAssoc = RpAnimBlendClumpGetAssociation(GetClump(), moveAnims[i]);
		if (curMoveAssoc) {
			if (CGeneral::faststrcmp(CAnimManager::GetAnimAssociation(m_animGroup, moveAnims[i])->hierarchy->name, curMoveAssoc->hierarchy->name)) {
				CAnimBlendAssociation *newMoveAssoc = CAnimManager::AddAnimation(GetClump(), m_animGroup, moveAnims[i]);
				newMoveAssoc->blendDelta = curMoveAssoc->blendDelta;
				newMoveAssoc->blendAmount = curMoveAssoc->blendAmount;
				curMoveAssoc->blendDelta = -1000.0f;
				curMoveAssoc->flags |= ASSOC_DELETEFADEDOUT;
			}
		}
	}
}

void
CPlayerPed::SetInitialState(void)
{
	m_nDrunkenness = 0;
	m_nFadeDrunkenness = 0;
	CMBlur::ClearDrunkBlur();
	m_nDrunkCountdown = 0;
	m_bAdrenalineActive = false;
	m_nAdrenalineTime = 0;
	CTimer::SetTimeScale(1.0f);
	m_pSeekTarget = nil;
	m_vecSeekPos = CVector(0.0f, 0.0f, 0.0f);
	m_fleeFromPos = CVector2D(0.0f, 0.0f);
	m_fleeFrom = nil;
	m_fleeTimer = 0;
	m_objective = OBJECTIVE_NONE;
	m_prevObjective = OBJECTIVE_NONE;
	bUsesCollision = true;
	ClearAimFlag();
	ClearLookFlag();
	bIsPointingGunAt = false;
	bRenderPedInCar = true;
	if (m_pFire)
		m_pFire->Extinguish();

	RpAnimBlendClumpRemoveAllAssociations(GetClump());
	SetPedState(PED_IDLE);
	SetMoveState(PEDMOVE_STILL);
	m_nLastPedState = PED_NONE;
	m_animGroup = ASSOCGRP_PLAYER;
	m_fMoveSpeed = 0.0f;
	m_nSelectedWepSlot = WEAPONSLOT_UNARMED;
	m_nEvadeAmount = 0;
	m_pEvadingFrom = nil;
	bIsPedDieAnimPlaying = false;
	SetRealMoveAnim();
	m_bCanBeDamaged = true;
	m_pedStats->m_temper = 50;
	m_fWalkAngle = 0.0f;
	if (m_attachedTo && !bUsesCollision)
		bUsesCollision = true;

	m_attachedTo = nil;
	m_attachWepAmmo = 0;
}

void
CPlayerPed::SetRealMoveAnim(void)
{
#ifdef CROUCH
	if (bIsDucking) {
		ProcessCrouch();

		return;
	}
#endif

#ifdef SWIMMING
	if (bIsSwimming) {
		ProcessSwimming();

		return;
	}
#endif

#ifdef FIRING_AND_AIMING
	if ((m_nPedState == PED_ATTACK || m_nPedState == PED_AIM_GUN || bIsPlayerAiming) && GetWeapon()->GetInfo()->m_nWeaponSlot > 2 && !CanWeRunAndFireWithWeapon()) {
		ProcessAiming();
		
		return;
	}
#endif

	CAnimBlendAssociation *curWalkAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_WALK);
	CAnimBlendAssociation *curRunAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_RUN);
	CAnimBlendAssociation *curSprintAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_RUNFAST);
	CAnimBlendAssociation *curWalkStartAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_STARTWALK);
	CAnimBlendAssociation *curIdleAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_IDLE);
	CAnimBlendAssociation *curRunStopAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_RUNSTOP1);
	CAnimBlendAssociation *curRunStopRAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_RUNSTOP2);
	if (bResetWalkAnims) {
		if (curWalkAssoc)
			curWalkAssoc->SetCurrentTime(0.0f);
		if (curRunAssoc)
			curRunAssoc->SetCurrentTime(0.0f);
		if (curSprintAssoc)
			curSprintAssoc->SetCurrentTime(0.0f);
		bResetWalkAnims = false;
	}

	if (!curIdleAssoc)
		curIdleAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_IDLE_TIRED);
	if (!curIdleAssoc)
		curIdleAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_FIGHT_IDLE);
	if (!curIdleAssoc)
		curIdleAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_MELEE_IDLE_FIGHTMODE);

	if (!((curRunStopAssoc && curRunStopAssoc->IsRunning()) || (curRunStopRAssoc && curRunStopRAssoc->IsRunning()))) {

		if (curRunStopAssoc && curRunStopAssoc->blendDelta >= 0.0f || curRunStopRAssoc && curRunStopRAssoc->blendDelta >= 0.0f) {
			if (curRunStopAssoc) {
				curRunStopAssoc->flags |= ASSOC_DELETEFADEDOUT;
				curRunStopAssoc->blendAmount = 1.0f;
				curRunStopAssoc->blendDelta = -8.0f;
			} else if (curRunStopRAssoc) {
				curRunStopRAssoc->flags |= ASSOC_DELETEFADEDOUT;
				curRunStopRAssoc->blendAmount = 1.0f;
				curRunStopRAssoc->blendDelta = -8.0f;
			}
			
			RestoreHeadingRate();
			if (!curIdleAssoc) {
				if (m_fCurrentStamina < 0.0f && !bIsAimingGun && !CWorld::TestSphereAgainstWorld(GetPosition(), 0.5f,
						nil, true, false, false, false, false, false)) {
					curIdleAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_IDLE_TIRED, 8.0f);

				} else {
					curIdleAssoc = CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_STD_IDLE, 8.0f);
				}
				m_nWaitTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(2500, 4000);
			}
			curIdleAssoc->blendAmount = 0.0f;
			curIdleAssoc->blendDelta = 8.0f;

		} else if (m_fMoveSpeed == 0.0f && !curSprintAssoc) {
			if (!curIdleAssoc) {
				if (m_fCurrentStamina < 0.0f && !bIsAimingGun && !CWorld::TestSphereAgainstWorld(GetPosition(), 0.5f,
						nil, true, false, false, false, false, false)) {
					curIdleAssoc = CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_IDLE_TIRED, 4.0f);
					
				} else {
					curIdleAssoc = CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_STD_IDLE, 4.0f);
				}

				m_nWaitTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(2500, 4000);
			}

			if ((m_fCurrentStamina > 0.0f || bIsAimingGun) && curIdleAssoc->animId == ANIM_STD_IDLE_TIRED) {
				CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_STD_IDLE, 4.0f);

			} else if (m_nPedState != PED_FIGHT) {
				if (m_fCurrentStamina < 0.0f && !bIsAimingGun && curIdleAssoc->animId != ANIM_STD_IDLE_TIRED
					&& !CWorld::TestSphereAgainstWorld(GetPosition(), 0.5f, nil, true, false, false, false, false, false)) {
					CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_IDLE_TIRED, 4.0f);

				} else if (curIdleAssoc->animId != ANIM_STD_IDLE) {
					CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_STD_IDLE, 4.0f);
				}
			}
			m_nMoveState = PEDMOVE_STILL;

		} else {
			if (curIdleAssoc) {
				if (curWalkStartAssoc) {
					curWalkStartAssoc->blendAmount = 1.0f;
					curWalkStartAssoc->blendDelta = 0.0f;
				} else {
					curWalkStartAssoc = CAnimManager::AddAnimation(GetClump(), m_animGroup, ANIM_STD_STARTWALK);
				}
				if (curWalkAssoc)
					curWalkAssoc->SetCurrentTime(0.0f);
				if (curRunAssoc)
					curRunAssoc->SetCurrentTime(0.0f);

				delete curIdleAssoc;
				delete RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_IDLE_TIRED);
				CAnimBlendAssociation *fightIdleAnim = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_FIGHT_IDLE);
				if (!fightIdleAnim)
					fightIdleAnim = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_MELEE_IDLE_FIGHTMODE);
				delete fightIdleAnim;
				delete curSprintAssoc;

				curSprintAssoc = nil;
				m_nMoveState = PEDMOVE_WALK;
			}
			if (curRunStopAssoc) {
				delete curRunStopAssoc;
				RestoreHeadingRate();
			}
			if (curRunStopRAssoc) {
				delete curRunStopRAssoc;
				RestoreHeadingRate();
			}
			if (!curWalkAssoc) {
				curWalkAssoc = CAnimManager::AddAnimation(GetClump(), m_animGroup, ANIM_STD_WALK);
				curWalkAssoc->blendAmount = 0.0f;
			}
			if (!curRunAssoc) {
				curRunAssoc = CAnimManager::AddAnimation(GetClump(), m_animGroup, ANIM_STD_RUN);
				curRunAssoc->blendAmount = 0.0f;
			}
			if (curWalkStartAssoc && !(curWalkStartAssoc->IsRunning())) {
				delete curWalkStartAssoc;
				curWalkStartAssoc = nil;
				curWalkAssoc->SetRun();
				curRunAssoc->SetRun();
			}
			if (m_nMoveState == PEDMOVE_SPRINT) {
				if (m_fCurrentStamina < 0.0f && (m_fCurrentStamina <= -150.0f || !curSprintAssoc || curSprintAssoc->blendDelta < 0.0f))
					m_nMoveState = PEDMOVE_STILL;

				if (curWalkStartAssoc)
					m_nMoveState = PEDMOVE_STILL;
			}

			if (curSprintAssoc && (m_nMoveState != PEDMOVE_SPRINT || m_fMoveSpeed < 0.4f)) {
				// Stop sprinting in various conditions
				if (curSprintAssoc->blendAmount == 0.0f) {
					curSprintAssoc->blendDelta = -1000.0f;
					curSprintAssoc->flags |= ASSOC_DELETEFADEDOUT;

				} else if (curSprintAssoc->blendDelta >= 0.0f || curSprintAssoc->blendAmount >= 0.8f) {
					if (m_fMoveSpeed < 0.4f) {
						AnimationId runStopAnim;
						if (curSprintAssoc->GetProgress() < 0.5) // double
							runStopAnim = ANIM_STD_RUNSTOP1;
						else
							runStopAnim = ANIM_STD_RUNSTOP2;
						CAnimBlendAssociation* newRunStopAssoc = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, runStopAnim);
						newRunStopAssoc->blendAmount = 1.0f;
						newRunStopAssoc->SetDeleteCallback(RestoreHeadingRateCB, this);
						m_headingRate = 0.0f;
						curSprintAssoc->flags |= ASSOC_DELETEFADEDOUT;
						curSprintAssoc->blendDelta = -1000.0f;
						curWalkAssoc->flags &= ~ASSOC_RUNNING;
						curWalkAssoc->blendAmount = 0.0f;
						curWalkAssoc->blendDelta = 0.0f;
						curRunAssoc->flags &= ~ASSOC_RUNNING;
						curRunAssoc->blendAmount = 0.0f;
						curRunAssoc->blendDelta = 0.0f;

					} else if (curSprintAssoc->blendDelta >= 0.0f) { // this condition is absent on mobile
						// Stop sprinting when tired
						curSprintAssoc->flags |= ASSOC_DELETEFADEDOUT;
						curSprintAssoc->blendDelta = -1.0f;
						curRunAssoc->blendDelta = 1.0f;
					}
				} else if (m_fMoveSpeed < 1.0f) {
					curSprintAssoc->blendDelta = -8.0f;
					curRunAssoc->blendDelta = 8.0f;
				}

			} else if (curWalkStartAssoc) {
				// Walk start and walk/run shouldn't run at the same time
				curWalkAssoc->flags &= ~ASSOC_RUNNING;
				curRunAssoc->flags &= ~ASSOC_RUNNING;
				curWalkAssoc->blendAmount = 0.0f;
				curRunAssoc->blendAmount = 0.0f;

			} else if (m_nMoveState == PEDMOVE_SPRINT) {
				if (curSprintAssoc) {
					// We have anim, do it
					if (curSprintAssoc->blendDelta < 0.0f) {
						curSprintAssoc->blendDelta = 2.0f;
						curRunAssoc->blendDelta = -2.0f;
					}
				} else {
					// Transition between run-sprint
					curWalkAssoc->blendAmount = 0.0f;
					curRunAssoc->blendAmount = 1.0f;
					curSprintAssoc = CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_STD_RUNFAST, 2.0f);
				}
				UseSprintEnergy();
			} else {
				if (m_fMoveSpeed < 1.0f) {
					curWalkAssoc->blendAmount = 1.0f;
					curRunAssoc->blendAmount = 0.0f;
					m_nMoveState = PEDMOVE_WALK;
				} else if (m_fMoveSpeed < 2.0f) {
					curWalkAssoc->blendAmount = 2.0f - m_fMoveSpeed;
					curRunAssoc->blendAmount = m_fMoveSpeed - 1.0f;
					m_nMoveState = PEDMOVE_RUN;
				} else {
					curWalkAssoc->blendAmount = 0.0f;
					curRunAssoc->blendAmount = 1.0f;
					m_nMoveState = PEDMOVE_RUN;
				}
				curWalkAssoc->blendDelta = 0.0f;
				curRunAssoc->blendDelta = 0.0f;
#ifdef IMPROVED_TECH_PART // run with heavy weapon
				if (GetWeapon()->m_eWeaponType == WEAPONTYPE_CHAINSAW ||
					GetWeapon()->m_eWeaponType == WEAPONTYPE_MINIGUN ||
					GetWeapon()->m_eWeaponType == WEAPONTYPE_FLAMETHROWER) {
					curRunAssoc->speed = 0.75f;
				}
#endif
			}
		}
	}
	if (m_bAdrenalineActive) {
		if (CTimer::GetTimeInMilliseconds() > m_nAdrenalineTime) {
			m_bAdrenalineActive = false;
			CTimer::SetTimeScale(1.0f);
			if (curWalkStartAssoc)
				curWalkStartAssoc->speed = 1.0f;
			if (curWalkAssoc)
				curWalkAssoc->speed = 1.0f;
			if (curRunAssoc)
				curRunAssoc->speed = 1.0f;
			if (curSprintAssoc)
				curSprintAssoc->speed = 1.0f;
		} else {
			CTimer::SetTimeScale(1.0f / 3);
			if (curWalkStartAssoc)
				curWalkStartAssoc->speed = 2.0f;
			if (curWalkAssoc)
				curWalkAssoc->speed = 2.0f;
			if (curRunAssoc)
				curRunAssoc->speed = 2.0f;
			if (curSprintAssoc)
				curSprintAssoc->speed = 2.0f;
		}
	} else if (curSprintAssoc) {
		if (TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_FIXED) {
			curSprintAssoc->speed = 0.7f;
#ifdef IMPROVED_TECH_PART // sprint with armed weapon, rocket and m60
		} else if (GetWeapon()->GetInfo()->m_nWeaponSlot == WEAPONSLOT_SHOTGUN || 
				   GetWeapon()->GetInfo()->m_nWeaponSlot == WEAPONSLOT_RIFLE ||
				   GetWeapon()->GetInfo()->m_nWeaponSlot == WEAPONSLOT_SNIPER) {
			curSprintAssoc->speed = 0.85f;
		} else if (GetWeapon()->m_eWeaponType == WEAPONTYPE_M60 || 
				   GetWeapon()->m_eWeaponType == WEAPONTYPE_ROCKETLAUNCHER) {
			curSprintAssoc->speed = 0.75f;
#endif
		} else
			curSprintAssoc->speed = 1.0f;
	}
}

void
CPlayerPed::RestoreSprintEnergy(float restoreSpeed)
{
	if (m_fCurrentStamina < m_fMaxStamina)
		m_fCurrentStamina += restoreSpeed * CTimer::GetTimeStep() * 0.5f;
}

float
CPlayerPed::DoWeaponSmoothSpray(void)
{
#ifdef FIRING_AND_AIMING // it's need for right movement during shooting
	return -1.0f;
#endif
	if (m_nPedState == PED_ATTACK && !m_pPointGunAt) {
		CWeaponInfo *weaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
		switch (GetWeapon()->m_eWeaponType) {
			case WEAPONTYPE_GOLFCLUB:
			case WEAPONTYPE_NIGHTSTICK:
			case WEAPONTYPE_BASEBALLBAT:
				if (GetFireAnimGround(weaponInfo, false) && RpAnimBlendClumpGetAssociation(GetClump(), GetFireAnimGround(weaponInfo, false)))
					return PI / 176.f;
				else
					return -1.0f;

			case WEAPONTYPE_CHAINSAW:
				if (GetMeleeStartAnim(weaponInfo) && RpAnimBlendClumpGetAssociation(GetClump(), GetMeleeStartAnim(weaponInfo))) {
#ifdef FREE_CAM
					if (TheCamera.Cams[0].Using3rdPersonMouseCam()) return -1.0f;
#endif
					return PI / 128.0f;
				}
				else if (GetFireAnimGround(weaponInfo, false) && RpAnimBlendClumpGetAssociation(GetClump(), GetFireAnimGround(weaponInfo, false)))
					return PI / 176.f;
				else
					return PI / 80.f;

			case WEAPONTYPE_PYTHON:
				return PI / 112.f;
			case WEAPONTYPE_SHOTGUN:
			case WEAPONTYPE_SPAS12_SHOTGUN:
			case WEAPONTYPE_STUBBY_SHOTGUN:
				return PI / 112.f;
			case WEAPONTYPE_UZI:
			case WEAPONTYPE_MP5:
				return PI / 112.f;
			case WEAPONTYPE_M4:
			case WEAPONTYPE_RUGER:
				return PI / 112.f;
			case WEAPONTYPE_FLAMETHROWER:
				return PI / 80.f;
			case WEAPONTYPE_M60:
			case WEAPONTYPE_MINIGUN:
			case WEAPONTYPE_HELICANNON:
				return PI / 176.f;
			default:
				return -1.0f;
		}
	} else if (bIsDucking)
		return PI / 112.f;
	else
		return -1.0f;
}

void
CPlayerPed::DoStuffToGoOnFire(void)
{
	if (m_nPedState == PED_SNIPER_MODE)
		TheCamera.ClearPlayerWeaponMode();
}

bool
CPlayerPed::DoesTargetHaveToBeBroken(CVector target, CWeapon *weaponUsed)
{
	CVector distVec = target - GetPosition();

	if (distVec.Magnitude() > CWeaponInfo::GetWeaponInfo(weaponUsed->m_eWeaponType)->m_fRange)
		return true;

	return false;
}

// Cancels landing anim while running & jumping? I think
void
CPlayerPed::RunningLand(CPad *padUsed)
{
	CAnimBlendAssociation *landAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_FALL_LAND);
	if (landAssoc && landAssoc->currentTime == 0.0f && m_fMoveSpeed > 1.5f
		&& padUsed && (padUsed->GetPedWalkLeftRight() != 0.0f || padUsed->GetPedWalkUpDown() != 0.0f)) {

		landAssoc->blendDelta = -1000.0f;
		landAssoc->flags |= ASSOC_DELETEFADEDOUT;

		CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_JUMP_LAND)->SetFinishCallback(FinishJumpCB, this);

		if (m_nPedState == PED_JUMP)
			RestorePreviousState();
	}
}

bool
CPlayerPed::IsThisPedAnAimingPriority(CPed *suspect)
{
	if (!suspect->bIsPlayerFriend)
		return true;

	if (suspect->m_pPointGunAt == this)
		return true;

	switch (suspect->m_objective) {
		case OBJECTIVE_KILL_CHAR_ON_FOOT:
		case OBJECTIVE_KILL_CHAR_ANY_MEANS:
			if (suspect->m_pedInObjective == this)
				return true;

			break;
		default:
			break;
	}
	return suspect->m_nPedState == PED_ABSEIL;
}

void
CPlayerPed::PlayerControlSniper(CPad *padUsed)
{
	ProcessWeaponSwitch(padUsed);
	TheCamera.PlayerExhaustion = (1.0f - (m_fCurrentStamina - -150.0f) / 300.0f) * 0.9f + 0.1f;

#ifndef FIRING_AND_AIMING
	if (padUsed->DuckJustDown() && !bIsDucking && m_nMoveState != PEDMOVE_SPRINT) {
		bCrouchWhenShooting = true;
		SetDuck(60000, true);
	} else if (bIsDucking && (padUsed->DuckJustDown() || m_nMoveState == PEDMOVE_SPRINT)) {
		ClearDuck(true);
		bCrouchWhenShooting = false;
	}
#endif

	if (!padUsed->GetTarget() && !m_attachedTo) {
		RestorePreviousState();
		TheCamera.ClearPlayerWeaponMode();
		return;
	}

	int firingRate = GetWeapon()->m_eWeaponType == WEAPONTYPE_LASERSCOPE ? 333 : 266;
	if (padUsed->WeaponJustDown() && CTimer::GetTimeInMilliseconds() > GetWeapon()->m_nTimer) {
		CVector firePos(0.0f, 0.0f, 0.6f);
		firePos = GetMatrix() * firePos;
		GetWeapon()->Fire(this, &firePos);
		m_nPadDownPressedInMilliseconds = CTimer::GetTimeInMilliseconds();

#ifdef FIRING_AND_AIMING // sniper rifle echo
		if (GetWeapon()->m_nAmmoTotal > 0)
			DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_AK47_BULLET_ECHO, GetWeapon()->m_eWeaponType);
#endif
	} else if (CTimer::GetTimeInMilliseconds() > m_nPadDownPressedInMilliseconds + firingRate &&
		CTimer::GetTimeInMilliseconds() - CTimer::GetTimeStepInMilliseconds() < m_nPadDownPressedInMilliseconds + firingRate && padUsed->GetWeapon()) {
		
		if (GetWeapon()->m_nAmmoTotal > 0) {
			DMAudio.PlayFrontEndSound(SOUND_WEAPON_AK47_BULLET_ECHO, GetWeapon()->m_eWeaponType);
		}
	}
	GetWeapon()->Update(m_audioEntityId, nil);

#ifdef FIRING_AND_AIMING
	PlayerControlZelda(padUsed);
#endif
}

// I think R* also used goto in here.
void
CPlayerPed::ProcessWeaponSwitch(CPad *padUsed)
{
	if (CDarkel::FrenzyOnGoing() || m_attachedTo)
		goto switchDetectDone;

#ifdef IMPROVED_MENU_AND_INPUT // You cannot switch weapons during reloading, but you can with the detonator
	if (!m_pPointGunAt && !bDontAllowWeaponChange && GetWeapon()->m_eWeaponState != WEAPONSTATE_RELOADING) { 
#else
	if (!m_pPointGunAt && !bDontAllowWeaponChange && GetWeapon()->m_eWeaponType != WEAPONTYPE_DETONATOR) {
#endif
		if (padUsed->CycleWeaponRightJustDown()) {

			if (TheCamera.PlayerWeaponMode.Mode != CCam::MODE_M16_1STPERSON
				&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_M16_1STPERSON_RUNABOUT
				&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_SNIPER
				&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_SNIPER_RUNABOUT
				&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_ROCKETLAUNCHER
				&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_ROCKETLAUNCHER_RUNABOUT
				&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_CAMERA) {

				for (m_nSelectedWepSlot = m_currentWeapon + 1; m_nSelectedWepSlot < TOTAL_WEAPON_SLOTS; ++m_nSelectedWepSlot) {
					if (HasWeaponSlot(m_nSelectedWepSlot) && GetWeapon(m_nSelectedWepSlot).HasWeaponAmmoToBeUsed()) {
#ifdef FIX_BUGS
						goto switchDetectDone;
#else
						goto spentAmmoCheck;
#endif
					}
				}
				m_nSelectedWepSlot = 0;
#ifdef FIX_BUGS
				goto switchDetectDone;
#endif
			}
		} else if (padUsed->CycleWeaponLeftJustDown()) {
			if (TheCamera.PlayerWeaponMode.Mode != CCam::MODE_M16_1STPERSON
				&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_SNIPER
				&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_ROCKETLAUNCHER
				&& TheCamera.PlayerWeaponMode.Mode != CCam::MODE_CAMERA) {

				// I don't know what kind of loop that was
				m_nSelectedWepSlot = m_currentWeapon - 1;
				do {
					if (m_nSelectedWepSlot < 0)
						m_nSelectedWepSlot = TOTAL_WEAPON_SLOTS - 1;

					if (m_nSelectedWepSlot == WEAPONSLOT_UNARMED)
						break;

					if (HasWeaponSlot(m_nSelectedWepSlot) && GetWeapon(m_nSelectedWepSlot).HasWeaponAmmoToBeUsed())
						break;
					
					--m_nSelectedWepSlot;
				} while (m_nSelectedWepSlot != WEAPONSLOT_UNARMED);
#ifdef FIX_BUGS
				goto switchDetectDone;
#endif

			}
		}
	}
	
spentAmmoCheck:
	if (CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_eWeaponFire != WEAPON_FIRE_MELEE
		&& (!padUsed->GetWeapon() || GetWeapon()->m_eWeaponType != WEAPONTYPE_MINIGUN)) {
		if (GetWeapon()->m_nAmmoTotal <= 0) {
			if (TheCamera.PlayerWeaponMode.Mode == CCam::MODE_M16_1STPERSON
				|| TheCamera.PlayerWeaponMode.Mode == CCam::MODE_SNIPER
				|| TheCamera.PlayerWeaponMode.Mode == CCam::MODE_ROCKETLAUNCHER)
				return;

			if (GetWeapon()->m_eWeaponType == WEAPONTYPE_DETONATOR
				&& GetWeapon(WEAPONSLOT_PROJECTILE).m_eWeaponType == WEAPONTYPE_DETONATOR_GRENADE)
				m_nSelectedWepSlot = WEAPONSLOT_PROJECTILE;
			else
				m_nSelectedWepSlot = m_currentWeapon - 1;

			for (; m_nSelectedWepSlot >= WEAPONSLOT_UNARMED; --m_nSelectedWepSlot) {

				// BUG: m_nSelectedWepSlot and GetWeapon(..) takes slot in VC but they compared them against weapon types in whole condition! jeez
#ifdef FIX_BUGS
				if (m_nSelectedWepSlot == WEAPONSLOT_MELEE ||
					GetWeapon(m_nSelectedWepSlot).m_nAmmoTotal > 0 && (m_nSelectedWepSlot != WEAPONSLOT_PROJECTILE || GetWeapon(WEAPONSLOT_PROJECTILE).m_eWeaponType == WEAPONTYPE_DETONATOR_GRENADE)) {
#else
				if (m_nSelectedWepSlot == WEAPONTYPE_BASEBALLBAT && GetWeapon(WEAPONTYPE_BASEBALLBAT).m_eWeaponType == WEAPONTYPE_BASEBALLBAT
					|| GetWeapon(m_nSelectedWepSlot).m_nAmmoTotal > 0
					&& m_nSelectedWepSlot != WEAPONTYPE_MOLOTOV && m_nSelectedWepSlot != WEAPONTYPE_GRENADE && m_nSelectedWepSlot != WEAPONTYPE_TEARGAS) {
#endif
					goto switchDetectDone;
				}
			}
			m_nSelectedWepSlot = WEAPONSLOT_UNARMED;
		}
	}

switchDetectDone:
	if (m_nSelectedWepSlot != m_currentWeapon) {
#ifdef FIRST_PERSON
		if (m_nPedState != PED_ATTACK && (m_nPedState != PED_AIM_GUN || IsDoomMode()) && m_nPedState != PED_FIGHT) {
#else
		if (m_nPedState != PED_ATTACK && m_nPedState != PED_AIM_GUN && m_nPedState != PED_FIGHT) {
#endif
			RemoveWeaponAnims(m_currentWeapon, -1000.0f);
			MakeChangesForNewWeapon(m_nSelectedWepSlot);
		}
	}
}

void
CPlayerPed::PlayerControlM16(CPad *padUsed)
{
	ProcessWeaponSwitch(padUsed);
	TheCamera.PlayerExhaustion = (1.0f - (m_fCurrentStamina - -150.0f) / 300.0f) * 0.9f + 0.1f;

#ifndef FIRING_AND_AIMING // The player will not be able to crouch while in turret mode
	if (padUsed->DuckJustDown() && !bIsDucking && m_nMoveState != PEDMOVE_SPRINT) {
		bCrouchWhenShooting = true;
		SetDuck(60000, true);
	} else if (bIsDucking && (padUsed->DuckJustDown() || m_nMoveState == PEDMOVE_SPRINT)) {
		ClearDuck(true);
		bCrouchWhenShooting = false;
	}
#endif

	if (!padUsed->GetTarget() && !m_attachedTo) {
		RestorePreviousState();
		TheCamera.ClearPlayerWeaponMode();
	}

	if (padUsed->GetWeapon() && CTimer::GetTimeInMilliseconds() > GetWeapon()->m_nTimer) {
		if (GetWeapon()->m_eWeaponState == WEAPONSTATE_OUT_OF_AMMO) {
			DMAudio.PlayFrontEndSound(SOUND_WEAPON_SNIPER_SHOT_NO_ZOOM, 0.f);
			GetWeapon()->m_nTimer = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_nFiringRate + CTimer::GetTimeInMilliseconds();
		} else {
			CVector firePos(0.0f, 0.0f, 0.6f);
			firePos = GetMatrix() * firePos;
			GetWeapon()->Fire(this, &firePos);
			m_nPadDownPressedInMilliseconds = CTimer::GetTimeInMilliseconds();
		}
	} else if (CTimer::GetTimeInMilliseconds() > GetWeapon()->m_nTimer &&
		CTimer::GetTimeInMilliseconds() - CTimer::GetTimeStepInMilliseconds() < GetWeapon()->m_nTimer && GetWeapon()->m_eWeaponState != WEAPONSTATE_OUT_OF_AMMO) {
		DMAudio.PlayFrontEndSound(SOUND_WEAPON_AK47_BULLET_ECHO, GetWeapon()->m_eWeaponType);
	}
	GetWeapon()->Update(m_audioEntityId, nil);
}

void
CPlayerPed::PlayerControlFighter(CPad *padUsed)
{
	float leftRight = padUsed->GetPedWalkLeftRight();
	float upDown = padUsed->GetPedWalkUpDown();
	float padMove = CVector2D(leftRight, upDown).Magnitude();

	if (padMove > 0.0f) {
		m_fRotationDest = CGeneral::GetRadianAngleBetweenPoints(0.0f, 0.0f, -leftRight, upDown) - TheCamera.Orientation;
		m_takeAStepAfterAttack = padMove > (2 * PAD_MOVE_TO_GAME_WORLD_MOVE);
		if (padUsed->GetSprint() && padMove > (1 * PAD_MOVE_TO_GAME_WORLD_MOVE))
			bIsAttacking = false;
	}

	if (!CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->IsFlagSet(WEAPONFLAG_HEAVY) && padUsed->JumpJustDown()) {
		if (m_nEvadeAmount != 0 && m_pEvadingFrom) {
			SetEvasiveDive((CPhysical*)m_pEvadingFrom, 1);
			m_nEvadeAmount = 0;
			m_pEvadingFrom = nil;
		} else {
			SetJump();
		}
	}
}

void
CPlayerPed::PlayerControl1stPersonRunAround(CPad *padUsed)
{
	float leftRight = padUsed->GetPedWalkLeftRight();
	float upDown = padUsed->GetPedWalkUpDown();
	float padMove = CVector2D(leftRight, upDown).Magnitude();
	float padMoveInGameUnit = padMove / PAD_MOVE_TO_GAME_WORLD_MOVE;
	if (padMoveInGameUnit > 0.0f) {
		m_fRotationDest = CGeneral::LimitRadianAngle(TheCamera.Orientation);
		m_fMoveSpeed = Min(padMoveInGameUnit, 0.07f * CTimer::GetTimeStep() + m_fMoveSpeed);
	} else {
		m_fMoveSpeed = 0.0f;
	}

	if (m_nPedState == PED_JUMP) {
		if (bIsInTheAir) {
			if (bUsesCollision && !bHitSteepSlope && (!bHitSomethingLastFrame || m_vecDamageNormal.z > 0.6f)
				&& m_fDistanceTravelled < CTimer::GetTimeStepInSeconds() && m_vecMoveSpeed.MagnitudeSqr() < 0.01f) {

				float angleSin = Sin(m_fRotationCur); // originally sin(DEGTORAD(RADTODEG(m_fRotationCur))) o_O
				float angleCos = Cos(m_fRotationCur);
				ApplyMoveForce(-angleSin * 3.0f, 3.0f * angleCos, 0.05f);
			}
		} else if (bIsLanding) {
			m_fMoveSpeed = 0.0f;
		}
	}

	if (m_nPedState == PED_ANSWER_MOBILE) {
		SetRealMoveAnim();
		return;
	}

	if (!CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->IsFlagSet(WEAPONFLAG_HEAVY) && padUsed->GetSprint()) {
		m_nMoveState = PEDMOVE_SPRINT;
	}
	if (m_nPedState != PED_FIGHT)
		SetRealMoveAnim();

	if (!bIsInTheAir && !(CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->IsFlagSet(WEAPONFLAG_HEAVY)) &&
		padUsed->JumpJustDown() && m_nPedState != PED_JUMP) {

		ClearAttack();
		ClearWeaponTarget();
		if (m_nEvadeAmount != 0 && m_pEvadingFrom) {
			SetEvasiveDive((CPhysical*)m_pEvadingFrom, 1);
			m_nEvadeAmount = 0;
			m_pEvadingFrom = nil;
		} else {
			SetJump();
		}
	}

	// FIX: Fact that PlayIdleAnimations only called through PlayerControlZelda was making it visible to only Classic control players. This isn't fair!
#ifdef FIX_BUGS
	if (m_nPedState != PED_FIGHT)
		PlayIdleAnimations(padUsed);
#endif
}

void
CPlayerPed::KeepAreaAroundPlayerClear(void)
{
	BuildPedLists();
	for (int i = 0; i < m_numNearPeds; ++i) {
		CPed *nearPed = m_nearPeds[i];
		if (nearPed->CharCreatedBy == RANDOM_CHAR && nearPed->m_nPedState != PED_DRIVING && !nearPed->DyingOrDead()) {
			if (nearPed->GetIsOnScreen()) {
				if (nearPed->m_objective == OBJECTIVE_NONE) {
					nearPed->SetFindPathAndFlee(this, 5000, true);
				} else {
					if (nearPed->EnteringCar())
						nearPed->QuitEnteringCar();

					nearPed->ClearObjective();
				}
			} else {
				nearPed->FlagToDestroyWhenNextProcessed();
			}
		}
	}
	CVector playerPos = (InVehicle() ? m_pMyVehicle->GetPosition() : GetPosition());

	CVector pos = GetPosition();
	int16 lastVehicle;
	CEntity *vehicles[8];
	CWorld::FindObjectsInRange(pos, CHECK_NEARBY_THINGS_MAX_DIST, true, &lastVehicle, 6, vehicles, false, true, false, false, false);

	for (int i = 0; i < lastVehicle; i++) {
		CVehicle *veh = (CVehicle*)vehicles[i];
		if (veh->VehicleCreatedBy != MISSION_VEHICLE) {
			if (veh->GetStatus() != STATUS_PLAYER && veh->GetStatus() != STATUS_PLAYER_DISABLED) {
				if ((veh->GetPosition() - playerPos).MagnitudeSqr() > 25.0f) {
					veh->AutoPilot.m_nTempAction = TEMPACT_WAIT;
					veh->AutoPilot.m_nTimeTempAction = CTimer::GetTimeInMilliseconds() + 5000;
				} else {
					if (DotProduct2D(playerPos - veh->GetPosition(), veh->GetForward()) > 0.0f)
						veh->AutoPilot.m_nTempAction = TEMPACT_REVERSE;
					else
						veh->AutoPilot.m_nTempAction = TEMPACT_GOFORWARD;

					veh->AutoPilot.m_nTimeTempAction = CTimer::GetTimeInMilliseconds() + 2000;
				}
				CCarCtrl::PossiblyRemoveVehicle(veh);
			}
		}
	}
}

void
CPlayerPed::EvaluateNeighbouringTarget(CEntity *candidate, CEntity **targetPtr, float *lastCloseness, float distLimit, float angleOffset, bool lookToLeft, bool priority)
{
	// priority param is unused
	CVector distVec = candidate->GetPosition() - GetPosition();
	if (distVec.Magnitude2D() <= distLimit) {
		if (!DoesTargetHaveToBeBroken(candidate->GetPosition(), GetWeapon())) {
			float angleBetweenUs = CGeneral::GetATanOfXY(candidate->GetPosition().x - TheCamera.GetPosition().x,
				candidate->GetPosition().y - TheCamera.GetPosition().y);

			angleBetweenUs = CGeneral::LimitAngle(angleBetweenUs - angleOffset);
			float closeness;
			if (lookToLeft) {
				closeness = angleBetweenUs > 0.0f ? -Abs(angleBetweenUs) : -100000.0f;
			} else {
				closeness = angleBetweenUs > 0.0f ? -100000.0f : -Abs(angleBetweenUs);
			}

			if (closeness > *lastCloseness) {
				*targetPtr = candidate;
				*lastCloseness = closeness;
			}
		}
	}
}

void
CPlayerPed::EvaluateTarget(CEntity *candidate, CEntity **targetPtr, float *lastCloseness, float distLimit, float angleOffset, bool priority)
{
	CVector distVec = candidate->GetPosition() - GetPosition();
	float dist = distVec.Magnitude2D();
	if (dist <= distLimit) {
		if (!DoesTargetHaveToBeBroken(candidate->GetPosition(), GetWeapon())) {
			float angleBetweenUs = CGeneral::GetATanOfXY(distVec.x, distVec.y);
			angleBetweenUs = CGeneral::LimitAngle(angleBetweenUs - angleOffset);

			float closeness = -dist - 5.0f * Abs(angleBetweenUs);
			if (priority) {
				closeness += 30.0f;
			}

			if (closeness > *lastCloseness) {
				*targetPtr = candidate;
				*lastCloseness = closeness;
			}
		}
	}
}

bool
CPlayerPed::CanIKReachThisTarget(CVector target, CWeapon* weapon, bool zRotImportant)
{
	float angleToFace = CGeneral::GetRadianAngleBetweenPoints(target.x, target.y, GetPosition().x, GetPosition().y);
	float angleDiff = CGeneral::LimitRadianAngle(angleToFace - m_fRotationCur);

#ifdef FIRING_AND_AIMING // decrease the auto-aim angle
	return (!zRotImportant || CWeaponInfo::GetWeaponInfo(weapon->m_eWeaponType)->IsFlagSet(WEAPONFLAG_CANAIM_WITHARM) || Abs(angleDiff) <= HALFPI) && TheCamera.IsSphereVisible(target, 1.0f) &&
#else
	return (!zRotImportant || CWeaponInfo::GetWeaponInfo(weapon->m_eWeaponType)->IsFlagSet(WEAPONFLAG_CANAIM_WITHARM) || Abs(angleDiff) <= HALFPI) &&
#endif
		(CWeaponInfo::GetWeaponInfo(weapon->m_eWeaponType)->IsFlagSet(WEAPONFLAG_CANAIM_WITHARM) || Abs(target.z - GetPosition().z) <= (target - GetPosition()).Magnitude2D());
}

void
CPlayerPed::RotatePlayerToTrackTarget(void)
{
	if (CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->IsFlagSet(WEAPONFLAG_CANAIM_WITHARM))
		return;

	float angleToFace = CGeneral::GetRadianAngleBetweenPoints(
		m_pPointGunAt->GetPosition().x, m_pPointGunAt->GetPosition().y,
		GetPosition().x, GetPosition().y);

	float angleDiff = CGeneral::LimitRadianAngle(m_fRotationCur - angleToFace);
	if (angleDiff < -DEGTORAD(25.0f)) {
		m_fRotationCur -= angleDiff + DEGTORAD(25.0f);
		m_fRotationDest -= angleDiff + DEGTORAD(25.0f);

	} else if (angleDiff > DEGTORAD(25.0f)) {
		m_fRotationCur -= angleDiff - DEGTORAD(25.0f);
		m_fRotationDest -= angleDiff - DEGTORAD(25.0f);
	}
}

bool
CPlayerPed::FindNextWeaponLockOnTarget(CEntity *previousTarget, bool lookToLeft)
{
	CEntity *nextTarget = nil;
	float weaponRange = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_fRange;
	// nextTarget = nil; // duplicate
	float lastCloseness = -10000.0f;
	// CGeneral::GetATanOfXY(GetForward().x, GetForward().y); // unused
	CVector distVec = previousTarget->GetPosition() - TheCamera.GetPosition();
	float referenceBeta = CGeneral::GetATanOfXY(distVec.x, distVec.y);

	for (int h = CPools::GetPedPool()->GetSize() - 1; h >= 0; h--) {
		CPed *pedToCheck = CPools::GetPedPool()->GetSlot(h);
		if (pedToCheck) {
			if (pedToCheck != this && pedToCheck != previousTarget) {
				if (!pedToCheck->DyingOrDead()
#ifndef AIMING_VEHICLE_OCCUPANTS // Mobile thing
					&& (!pedToCheck->bInVehicle || (pedToCheck->m_pMyVehicle && pedToCheck->m_pMyVehicle->IsBike()))
#endif
					&& pedToCheck->m_leader != this && !pedToCheck->bNeverEverTargetThisPed
					&& OurPedCanSeeThisOne(pedToCheck, true) && CanIKReachThisTarget(pedToCheck->GetPosition(), GetWeapon(), true)) {

					EvaluateNeighbouringTarget(pedToCheck, &nextTarget, &lastCloseness,
						weaponRange, referenceBeta, lookToLeft, IsThisPedAnAimingPriority(pedToCheck));
				}
			}
		}
	}
	for (int i = 0; i < ARRAY_SIZE(m_nTargettableObjects); i++) {
		CObject *obj = CPools::GetObjectPool()->GetAt(m_nTargettableObjects[i]);
		if (obj && !obj->bHasBeenDamaged && CanIKReachThisTarget(obj->GetPosition(), GetWeapon(), true))
			EvaluateNeighbouringTarget(obj, &nextTarget, &lastCloseness, weaponRange, referenceBeta, lookToLeft, true);
	}
	if (!nextTarget)
		return false;

	SetWeaponLockOnTarget(nextTarget);
	bDontAllowWeaponChange = true;
	SetPointGunAt(nextTarget);
	return true;
}

bool
CPlayerPed::FindWeaponLockOnTarget(void)
{
	CEntity *nextTarget = nil;
	float weaponRange = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_fRange;

	if (m_pPointGunAt) {
		CVector distVec = m_pPointGunAt->GetPosition() - GetPosition();
		if (distVec.Magnitude2D() > weaponRange) {
			SetWeaponLockOnTarget(nil);
			return false;
		} else {
			return true;
		}
	}

	// nextTarget = nil; // duplicate
	float lastCloseness = -10000.0f;
	float referenceBeta = CGeneral::GetATanOfXY(GetForward().x, GetForward().y);
	for (int h = CPools::GetPedPool()->GetSize() - 1; h >= 0; h--) {
		CPed *pedToCheck = CPools::GetPedPool()->GetSlot(h);
		if (pedToCheck) {
			if (pedToCheck != this) {
				if (!pedToCheck->DyingOrDead()
#ifndef AIMING_VEHICLE_OCCUPANTS // Mobile thing
					&& (!pedToCheck->bInVehicle || (pedToCheck->m_pMyVehicle && pedToCheck->m_pMyVehicle->IsBike()))
#endif
					&& pedToCheck->m_leader != this && !pedToCheck->bNeverEverTargetThisPed
					&& OurPedCanSeeThisOne(pedToCheck) && CanIKReachThisTarget(pedToCheck->GetPosition(), GetWeapon(), true)) {

					EvaluateTarget(pedToCheck, &nextTarget, &lastCloseness,
						weaponRange, referenceBeta, IsThisPedAnAimingPriority(pedToCheck));
				}
			}
		}
	}
	for (int i = 0; i < ARRAY_SIZE(m_nTargettableObjects); i++) {
		CObject *obj = CPools::GetObjectPool()->GetAt(m_nTargettableObjects[i]);
		if (obj && !obj->bHasBeenDamaged && CanIKReachThisTarget(obj->GetPosition(), GetWeapon(), true))
			EvaluateTarget(obj, &nextTarget, &lastCloseness, weaponRange, referenceBeta, true);
	}
	if (!nextTarget)
		return false;

#ifdef IMPROVED_MENU_AND_INPUT
	bIsAutoAiming = true;
#endif
	SetWeaponLockOnTarget(nextTarget);
	bDontAllowWeaponChange = true;
	SetPointGunAt(nextTarget);
	Say(SOUND_PED_AIMING);
	return true;
}

void
CPlayerPed::ProcessAnimGroups(void)
{
	AssocGroupId groupToSet;
#ifdef PC_PLAYER_CONTROLS
	if ((m_fWalkAngle <= -DEGTORAD(50.0f) || m_fWalkAngle >= DEGTORAD(50.0f))
#ifndef IMPROVED_MENU_AND_INPUT	
		&& TheCamera.Cams[TheCamera.ActiveCam].Using3rdPersonMouseCam()
#endif
		&& CanStrafeOrMouseControl()) {

		if (m_fWalkAngle >= -DEGTORAD(130.0f) && m_fWalkAngle <= DEGTORAD(130.0f)) {
			if (m_fWalkAngle > 0.0f) {
				if (GetWeapon()->m_eWeaponType == WEAPONTYPE_ROCKETLAUNCHER)
					groupToSet = ASSOCGRP_ROCKETLEFT;
				else if (GetWeapon()->m_eWeaponType == WEAPONTYPE_CHAINSAW ||
					GetWeapon()->m_eWeaponType == WEAPONTYPE_FLAMETHROWER ||
					GetWeapon()->m_eWeaponType == WEAPONTYPE_MINIGUN)
					groupToSet = ASSOCGRP_CHAINSAWLEFT;
				else
					groupToSet = ASSOCGRP_PLAYERLEFT;
			} else {
				if (GetWeapon()->m_eWeaponType == WEAPONTYPE_ROCKETLAUNCHER)
					groupToSet = ASSOCGRP_ROCKETRIGHT;
				else if (GetWeapon()->m_eWeaponType == WEAPONTYPE_CHAINSAW ||
					GetWeapon()->m_eWeaponType == WEAPONTYPE_FLAMETHROWER ||
					GetWeapon()->m_eWeaponType == WEAPONTYPE_MINIGUN)
					groupToSet = ASSOCGRP_CHAINSAWRIGHT;
				else
					groupToSet = ASSOCGRP_PLAYERRIGHT;
			}
		} else {
			if (GetWeapon()->m_eWeaponType == WEAPONTYPE_ROCKETLAUNCHER)
				groupToSet = ASSOCGRP_ROCKETBACK;
			else if (GetWeapon()->m_eWeaponType == WEAPONTYPE_CHAINSAW ||
				GetWeapon()->m_eWeaponType == WEAPONTYPE_FLAMETHROWER ||
				GetWeapon()->m_eWeaponType == WEAPONTYPE_MINIGUN)
				groupToSet = ASSOCGRP_CHAINSAWBACK;
			else
				groupToSet = ASSOCGRP_PLAYERBACK;
		}
	} else
#endif
	{
		if (GetWeapon()->m_eWeaponType == WEAPONTYPE_ROCKETLAUNCHER) {
			groupToSet = ASSOCGRP_PLAYERROCKET;
		} else {
			if (GetWeapon()->m_eWeaponType == WEAPONTYPE_BASEBALLBAT
				 || GetWeapon()->m_eWeaponType == WEAPONTYPE_MACHETE)
				groupToSet = ASSOCGRP_PLAYERBBBAT;
			else if (GetWeapon()->m_eWeaponType == WEAPONTYPE_CHAINSAW ||
				GetWeapon()->m_eWeaponType == WEAPONTYPE_FLAMETHROWER ||
				GetWeapon()->m_eWeaponType == WEAPONTYPE_MINIGUN)
				groupToSet = ASSOCGRP_PLAYERCHAINSAW;
			else if (GetWeapon()->m_eWeaponType != WEAPONTYPE_COLT45 && GetWeapon()->m_eWeaponType != WEAPONTYPE_UZI
				// I hope this is an inlined function...
				&& GetWeapon()->m_eWeaponType != WEAPONTYPE_PYTHON && GetWeapon()->m_eWeaponType != WEAPONTYPE_TEC9
				&& GetWeapon()->m_eWeaponType != WEAPONTYPE_SILENCED_INGRAM && GetWeapon()->m_eWeaponType != WEAPONTYPE_MP5
				&& GetWeapon()->m_eWeaponType != WEAPONTYPE_GOLFCLUB && GetWeapon()->m_eWeaponType != WEAPONTYPE_KATANA
				&& GetWeapon()->m_eWeaponType != WEAPONTYPE_CAMERA) {
				if (!GetWeapon()->IsType2Handed()) {
					groupToSet = ASSOCGRP_PLAYER;
				} else {
					groupToSet = ASSOCGRP_PLAYER2ARMED;
				}
			} else {
				groupToSet = ASSOCGRP_PLAYER1ARMED;
			}
		}
	}

	if (m_animGroup != groupToSet) {
		m_animGroup = groupToSet;
		ReApplyMoveAnims();
	}
}

void
CPlayerPed::ProcessPlayerWeapon(CPad *padUsed)
{
#ifdef SWIMMING
	if (bIsSwimming)
		return;
#endif

	CWeaponInfo *weaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
	if (m_bHasLockOnTarget && !m_pPointGunAt) {
		TheCamera.ClearPlayerWeaponMode();
		CWeaponEffects::ClearCrossHair();
		ClearPointGunAt();
	}

#ifdef IMPROVED_TECH_PART
	if (padUsed->DuckJustDown() && !bIsDucking && m_nMoveState != PEDMOVE_SPRINT && weaponInfo->m_nWeaponSlot != WEAPONSLOT_HEAVY && GetWeapon()->m_eWeaponType != WEAPONTYPE_CHAINSAW) {
#else
	if (padUsed->DuckJustDown() && !bIsDucking && m_nMoveState != PEDMOVE_SPRINT) {
#endif

#ifdef FIRING_AND_AIMING
		ClearPointGunAt();
#else
#ifdef FIX_BUGS
		// fix tommy being locked into looking at the same spot if you duck just after starting to shoot
		if(!m_pPointGunAt)
			ClearPointGunAt();
#endif
#endif
		bCrouchWhenShooting = true;
		SetDuck(60000, true);
	} else if (bIsDucking && (padUsed->DuckJustDown() || m_nMoveState == PEDMOVE_SPRINT ||
#ifdef IMPROVED_TECH_PART // When aiming from a sniper or camera, the player doesn't stand up when changing the zoom while sitting
		((padUsed->GetSprint() || padUsed->ExitVehicleJustDown() || padUsed->JumpJustDown()) && 
		TheCamera.PlayerWeaponMode.Mode != CCam::MODE_SNIPER && TheCamera.PlayerWeaponMode.Mode != CCam::MODE_CAMERA))) {
#else
		padUsed->GetSprint() || padUsed->JumpJustDown() || padUsed->ExitVehicleJustDown())) {
#endif

#ifdef FIRING_AND_AIMING
		ClearPointGunAt();
#else
#ifdef FIX_BUGS
		// same fix as above except for standing up
		if(!m_pPointGunAt)
			ClearPointGunAt();
#endif
#endif
		ClearDuck(true);
		bCrouchWhenShooting = false;
	}

	if(weaponInfo->IsFlagSet(WEAPONFLAG_CANAIM))
		m_wepAccuracy = 95;
	else
		m_wepAccuracy = 100;

#ifdef FIRING_AND_AIMING // make it possible to switch between a pistol and a submachine gun while aiming in vehicle
	if (InVehicle() && bIsPlayerAiming && (padUsed->CycleWeaponLeftJustDown() || padUsed->CycleWeaponRightJustDown())) {
		if (HasWeaponSlot(5) && GetWeapon(5).m_nAmmoTotal > 0 && GetWeapon()->m_eWeaponType == WEAPONTYPE_COLT45) {
			if (m_storedWeapon == WEAPONTYPE_UNIDENTIFIED)
				m_storedWeapon = GetWeapon()->m_eWeaponType;
			SetCurrentWeapon(GetWeapon(5).m_eWeaponType);
		}
		else if (HasWeaponSlot(3) && GetWeapon(3).m_nAmmoTotal > 0 && GetWeapon(3).m_eWeaponType == WEAPONTYPE_COLT45) {
			if (m_storedWeapon == WEAPONTYPE_UNIDENTIFIED)
				m_storedWeapon = GetWeapon()->m_eWeaponType;
			SetCurrentWeapon(GetWeapon(3).m_eWeaponType);
		}
	}
#endif

	if (!m_pFire) {
		eWeaponType weapon = GetWeapon()->m_eWeaponType;
#if defined FIRING_AND_AIMING && defined FIRST_PERSON
		bool bSniperCamera = weapon == WEAPONTYPE_ROCKETLAUNCHER || weapon == WEAPONTYPE_SNIPERRIFLE || weapon == WEAPONTYPE_LASERSCOPE || weapon == WEAPONTYPE_CAMERA;

		uint32 slot = CWeaponInfo::GetWeaponInfo(weapon)->m_nWeaponSlot;
		if ((slot == WEAPONSLOT_SHOTGUN || slot == WEAPONSLOT_HANDGUN ||
			slot == WEAPONSLOT_RIFLE || slot == WEAPONSLOT_SUBMACHINEGUN ||
			weapon == WEAPONTYPE_M60 || weapon == WEAPONTYPE_MINIGUN ||
			weapon == WEAPONTYPE_FLAMETHROWER || bSniperCamera) && m_nMoveState != PEDMOVE_SPRINT) {

			bool isFrontObstacle = false;
			if (padUsed->GetTarget()) {
				// from CPed::SetAttack
				CVector aimPos = GetRight() * 0.1f + GetForward() * 0.2f + GetPosition();
				aimPos += GetUp() * 0.35f;
				CEntity* obstacle = CWorld::TestSphereAgainstWorld(aimPos, 0.2f, nil, true, false, false, true, false, false);
				if (obstacle) {
					if (gaTempSphereColPoints[0].surfaceB != SURFACE_TRANSPARENT_CLOTH && gaTempSphereColPoints[0].surfaceB != SURFACE_METAL_CHAIN_FENCE &&
						gaTempSphereColPoints[0].surfaceB != SURFACE_WOOD_BENCH && gaTempSphereColPoints[0].surfaceB != SURFACE_SCAFFOLD_POLE) {

						isFrontObstacle = true;
					}
				}
			}

			bool cantFire = isFrontObstacle || m_fMoveSpeed != 0.0f && bIsDucking;

			if (cantFire) {
				ClearAimFlag();
				ClearWeaponTarget();
			}

			if (padUsed->GetWeapon() && bIsPlayerAiming && weapon != WEAPONTYPE_CAMERA && (weapon != WEAPONTYPE_LASERSCOPE || IsDoomMode())) {
				if (m_nPedState == PED_ATTACK) {
					m_fAttackButtonCounter *= Pow(0.94f, CTimer::GetTimeStep());
				}
				else {
					m_fAttackButtonCounter = 0.0f;
				}
				SetAttack(nil);
			}

			if ((padUsed->GetTarget() || IsDoomMode()) && bIsPlayerAiming && !cantFire && GetWeapon()->m_eWeaponState != WEAPONSTATE_RELOADING && !bIsAutoAiming) {
				if (!IsDoomMode()) {
					if (weapon == WEAPONTYPE_ROCKETLAUNCHER)
						TheCamera.SetNewPlayerWeaponMode(CCam::MODE_ROCKETLAUNCHER, 0, 0);
					else if (weapon == WEAPONTYPE_SNIPERRIFLE || weapon == WEAPONTYPE_LASERSCOPE)
						TheCamera.SetNewPlayerWeaponMode(CCam::MODE_SNIPER, 0, 0);
					else if (weapon == WEAPONTYPE_CAMERA)
						TheCamera.SetNewPlayerWeaponMode(CCam::MODE_CAMERA, 0, 0);
				}

				bDontAllowWeaponChange = !IsDoomMode();

				SetAimFlag(m_fRotationCur);
				m_fFPSMoveHeading = TheCamera.Find3rdPersonQuickAimPitch();

				if (m_nPedState != PED_ATTACK && weapon != WEAPONTYPE_CAMERA) {
					SetPointGunAt(nil);

					// ped reaction to aiming
					float multiplier = 1.0f;
					for (int i = 1; i < 60; i++) {
						// first check
						CWorld::pIgnoreEntity = this;
						CEntity* hitEntityRaycast;
						if (CWorld::ProcessLineOfSight(TheCamera.GetPosition(), TheCamera.GetPosition() + TheCamera.GetForward() * 20.0f, CColPoint{}, hitEntityRaycast, true, false, true, false, false, true)) {
							if (hitEntityRaycast->IsPed()) {
								CPed* hitPed = (CPed*)hitEntityRaycast;
								if ((hitPed->CanSeeEntity(this, DEGTORAD(80.0f)) && hitPed->m_nPedType != PEDTYPE_COP) ||
									(hitPed->CanSeeEntity(this, DEGTORAD(80.0f)) && hitPed->m_nPedType == PEDTYPE_COP && !FindPlayerPed()->m_pWanted->IsPlayerHides())) {
									m_pPointGunAt = hitPed;

									Say(SOUND_PED_AIMING);

									break;
								}
							}
						}
						CWorld::pIgnoreEntity = NULL;

						// second check
						CVector spherePos = GetPosition() + CVector(0.0f, 0.0f, 0.6f) + (TheCamera.GetForward() * 0.5f) * multiplier;
						CEntity* hitEntityShapecast = CWorld::TestSphereAgainstWorld(spherePos, 0.25f, this, true, false, true, false, false, false);
						if (!hitEntityShapecast) {
							if (i == 59 && m_pPointGunAt)
								m_pPointGunAt = nullptr;

							multiplier++;
							continue;
						}

						if (hitEntityShapecast->IsPed()) {
							CPed* hitPed = (CPed*)hitEntityShapecast;
							if ((hitPed->CanSeeEntity(this, DEGTORAD(80.0f)) && hitPed->m_nPedType != PEDTYPE_COP) ||
								(hitPed->CanSeeEntity(this, DEGTORAD(80.0f)) && hitPed->m_nPedType == PEDTYPE_COP && !FindPlayerPed()->m_pWanted->IsPlayerHides())) {
								m_pPointGunAt = hitPed;

								Say(SOUND_PED_AIMING);

								break;
							}
						} else if (m_pPointGunAt) {
							m_pPointGunAt = nullptr;

							break;
						}
					}
				}
			}

			if ((padUsed->GetTarget() || IsDoomMode()) && m_nPedState != PED_FIGHT) {
				bIsPlayerAiming = true;
			} else if ((!padUsed->GetTarget() || IsDoomMode()) && bIsPlayerAiming) {
				bIsPlayerAiming = false;

				if (GetWeapon()->m_eWeaponState != WEAPONSTATE_RELOADING)
					ClearWeaponTarget();

				StopAimingAnims();

				if (bIsDucking) {
					TheCamera.Cams[TheCamera.ActiveCam].m_fTargetCameraPosZ = 0.55f;
				} else if (weapon != WEAPONTYPE_COLT45 && weapon != WEAPONTYPE_TEC9 && weapon != WEAPONTYPE_SILENCED_INGRAM) {
					if (m_fMoveSpeed > 0.0f)
						CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_STD_RUN, 4.0f);
					else
						CAnimManager::BlendAnimation(GetClump(), m_animGroup, ANIM_STD_IDLE, 4.0f);
				}
			}

			if (weapon == WEAPONTYPE_CAMERA || (weapon == WEAPONTYPE_LASERSCOPE && bIsPlayerAiming))
				return;

			float diff = DefaultFOV - TheCamera.Cams[TheCamera.ActiveCam].AimingFOV;

			if (bIsPlayerAiming) {
				if (slot == WEAPONSLOT_RIFLE || weapon == WEAPONTYPE_M60)
					TheCamera.Cams[TheCamera.ActiveCam].AimingFOV = 40.0f;
				else
					TheCamera.Cams[TheCamera.ActiveCam].AimingFOV = 55.0f;
			}
		} else {
			bIsPlayerAiming = false;

			StopAimingAnims();
		}
#else
		if (weapon == WEAPONTYPE_ROCKETLAUNCHER || weapon == WEAPONTYPE_SNIPERRIFLE ||
			weapon == WEAPONTYPE_LASERSCOPE || weapon == WEAPONTYPE_M4 ||
			weapon == WEAPONTYPE_RUGER || weapon == WEAPONTYPE_M60 ||
			weapon == WEAPONTYPE_CAMERA) {

			if (padUsed->TargetJustDown() || TheCamera.m_bJustJumpedOutOf1stPersonBecauseOfTarget) {
		#ifdef FREE_CAM
				if (CCamera::bFreeCam && TheCamera.Cams[0].Using3rdPersonMouseCam()) {
					m_fRotationCur = CGeneral::LimitRadianAngle(-TheCamera.Orientation);
					SetHeading(m_fRotationCur);
				}
		#endif
				if (weapon == WEAPONTYPE_ROCKETLAUNCHER)
					TheCamera.SetNewPlayerWeaponMode(CCam::MODE_ROCKETLAUNCHER, 0, 0);
				else if (weapon == WEAPONTYPE_SNIPERRIFLE || weapon == WEAPONTYPE_LASERSCOPE)
					TheCamera.SetNewPlayerWeaponMode(CCam::MODE_SNIPER, 0, 0);
				else if (weapon == WEAPONTYPE_CAMERA)
					TheCamera.SetNewPlayerWeaponMode(CCam::MODE_CAMERA, 0, 0);
				else
					TheCamera.SetNewPlayerWeaponMode(CCam::MODE_M16_1STPERSON, 0, 0);

				m_fMoveSpeed = 0.0f;

				CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_IDLE, 1000.0f);
				SetPedState(PED_SNIPER_MODE);
				return;
			}
			if (!TheCamera.Using1stPersonWeaponMode())
				if (weapon == WEAPONTYPE_ROCKETLAUNCHER || weapon == WEAPONTYPE_SNIPERRIFLE || weapon == WEAPONTYPE_LASERSCOPE || weapon == WEAPONTYPE_CAMERA)
					return;
		}
#endif
	}

#ifdef IMPROVED_MENU_AND_INPUT
	if (padUsed->GetMeleeWeapon() && weaponInfo->m_nWeaponSlot <= WEAPONSLOT_MELEE && m_nMoveState != PEDMOVE_SPRINT) {
		if (m_nSelectedWepSlot == m_currentWeapon) {
			if (m_nPedState == PED_ATTACK) {
				if (padUsed->MeleeWeaponJustDown()) {
					m_bHaveTargetSelected = true;
				} else if (!m_bHaveTargetSelected) {
					m_fAttackButtonCounter += CTimer::GetTimeStepNonClipped();
				}
			} else {
				m_fAttackButtonCounter = 0.0f;
				m_bHaveTargetSelected = false;
			}

			if (padUsed->MeleeWeaponJustDown()) {
				if ((GetWeapon()->m_eWeaponType == WEAPONTYPE_UNARMED || GetWeapon()->m_eWeaponType == WEAPONTYPE_BRASSKNUCKLE || weaponInfo->IsFlagSet(WEAPONFLAG_FIGHTMODE)) &&
					(m_fMoveSpeed < 1.0f || m_nPedState == PED_FIGHT)) {
					
					StartFightAttack(padUsed->GetWeapon());
				} else {
					SetAttack(nil);
				}
			}
		}
	} else if (padUsed->GetWeapon() && m_nMoveState != PEDMOVE_SPRINT) {
		if (m_nSelectedWepSlot == m_currentWeapon) {
			if (m_pPointGunAt && FrontEndMenuManager.m_PrefsAutoaim) {
				if (m_nPedState == PED_ATTACK) {
					m_fAttackButtonCounter *= Pow(0.94f, CTimer::GetTimeStep());
				} else {
					m_fAttackButtonCounter = 0.0f;
				}
				SetAttack(m_pPointGunAt);
			} else {
				if (m_nPedState == PED_ATTACK) {
					if (padUsed->WeaponJustDown()) {
						m_bHaveTargetSelected = true;
					} else if (!m_bHaveTargetSelected) {
						m_fAttackButtonCounter += CTimer::GetTimeStepNonClipped();
					}
				} else {
					m_fAttackButtonCounter = 0.0f;
					m_bHaveTargetSelected = false;
				}

				if (GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED && GetWeapon()->m_eWeaponType != WEAPONTYPE_BRASSKNUCKLE &&
					!weaponInfo->IsFlagSet(WEAPONFLAG_FIGHTMODE)) {

					if (GetWeapon()->m_eWeaponType != WEAPONTYPE_DETONATOR && GetWeapon()->m_eWeaponType != WEAPONTYPE_DETONATOR_GRENADE ||
						padUsed->WeaponJustDown())

						SetAttack(nil);
				}
			}
		}
	} else {
		m_pedIK.m_flags &= ~CPedIK::LOOKAROUND_HEAD_ONLY;
		if (m_nPedState == PED_ATTACK) {
			m_bHaveTargetSelected = true;
			bIsAttacking = false;
		}
	}

	if (CPad::GetPad(0)->WeaponReloadJustDown() && weaponInfo->IsFlagSet(WEAPONFLAG_RELOAD)) {
		CWeapon* weapon = GetWeapon();
		if (weapon->m_eWeaponState == WEAPONSTATE_READY && weapon->m_nAmmoInClip > 0 && 
			weapon->m_nAmmoInClip < weaponInfo->m_nAmountofAmmunition && weapon->m_nAmmoTotal != weapon->m_nAmmoInClip) {

			weapon->m_eWeaponState = WEAPONSTATE_RELOADING;
			weapon->m_nTimer = CTimer::GetTimeInMilliseconds() + weapon->GetInfo()->m_nReload;

			CAnimBlendAssociation* newReloadAssoc = CAnimManager::BlendAnimation(
				GetClump(), weaponInfo->m_AnimToPlay,
				bIsDucking && GetCrouchReloadAnim(weaponInfo) ? GetCrouchReloadAnim(weaponInfo) : GetReloadAnim(weaponInfo),
				8.0f);
			newReloadAssoc->SetFinishCallback(FinishedReloadCB, this);

			ClearLookFlag();
			ClearAimFlag();
			bIsAttacking = false;
			bIsPointingGunAt = false;
			m_shootTimer = CTimer::GetTimeInMilliseconds();
		}
	}
#else
	if (padUsed->GetWeapon() && m_nMoveState != PEDMOVE_SPRINT) {
		if (m_nSelectedWepSlot == m_currentWeapon) {
			if (m_pPointGunAt) {
				if (m_nPedState == PED_ATTACK) {
					m_fAttackButtonCounter *= Pow(0.94f, CTimer::GetTimeStep());
				} else {
					m_fAttackButtonCounter = 0.0f;
				}
				SetAttack(m_pPointGunAt);
			} else {
				if (m_nPedState == PED_ATTACK) {
					if (padUsed->WeaponJustDown()) {
						m_bHaveTargetSelected = true;
					} else if (!m_bHaveTargetSelected) {
						m_fAttackButtonCounter += CTimer::GetTimeStepNonClipped();
					}
				} else {
					m_fAttackButtonCounter = 0.0f;
					m_bHaveTargetSelected = false;
				}

				if (GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED && GetWeapon()->m_eWeaponType != WEAPONTYPE_BRASSKNUCKLE &&
					!weaponInfo->IsFlagSet(WEAPONFLAG_FIGHTMODE)) {

					if (GetWeapon()->m_eWeaponType != WEAPONTYPE_DETONATOR && GetWeapon()->m_eWeaponType != WEAPONTYPE_DETONATOR_GRENADE ||
						padUsed->WeaponJustDown())
						SetAttack(nil);
				} else if (padUsed->WeaponJustDown()) {
					if (m_fMoveSpeed < 1.0f || m_nPedState == PED_FIGHT)
						StartFightAttack(padUsed->GetWeapon());
					else
						SetAttack(nil);
				}
			}
		}
	} else {
		m_pedIK.m_flags &= ~CPedIK::LOOKAROUND_HEAD_ONLY;
		if (m_nPedState == PED_ATTACK) {
			m_bHaveTargetSelected = true;
			bIsAttacking = false;
		}
	}
#endif

#ifdef FREE_CAM
	static int8 changedHeadingRate = 0;
	static int8 pointedGun = 0;
	if (changedHeadingRate == 2) changedHeadingRate = 1;
	if (pointedGun == 2) pointedGun = 1;

	// Rotate player/arm when shooting. We don't have auto-rotation anymore
#ifdef IMPROVED_MENU_AND_INPUT
	if (CCamera::bFreeCam &&
#else
	if (CCamera::m_bUseMouse3rdPerson && CCamera::bFreeCam &&
#endif
#ifdef FIRING_AND_AIMING // change hand position while driveby
		(m_nSelectedWepSlot == m_currentWeapon || InVehicle()) && m_nMoveState != PEDMOVE_SPRINT) {
#else
		m_nSelectedWepSlot == m_currentWeapon && m_nMoveState != PEDMOVE_SPRINT) {
#endif

#ifdef FIRING_AND_AIMING // change hand position while driveby
#define CAN_AIM_WITH_ARM ((weaponInfo->IsFlagSet(WEAPONFLAG_CANAIM_WITHARM) || InVehicle()) && !bIsDucking && !bCrouchWhenShooting)
#else
#define CAN_AIM_WITH_ARM (weaponInfo->IsFlagSet(WEAPONFLAG_CANAIM_WITHARM) && !bIsDucking && !bCrouchWhenShooting)
#endif
		// Weapons except throwable and melee ones
		if (weaponInfo->m_nWeaponSlot > 2) {
#ifdef FIRING_AND_AIMING
			if ((padUsed->GetTarget() && CAN_AIM_WITH_ARM) || padUsed->GetWeapon() || (m_nPedState == PED_ATTACK && weaponInfo->m_nWeaponSlot > WEAPONSLOT_PROJECTILE)) {
#else
			if ((padUsed->GetTarget() && CAN_AIM_WITH_ARM) || padUsed->GetWeapon()) {
#endif
				float limitedCam = CGeneral::LimitRadianAngle(-TheCamera.Orientation);

				m_cachedCamSource = TheCamera.Cams[TheCamera.ActiveCam].Source;
				m_cachedCamFront = TheCamera.Cams[TheCamera.ActiveCam].Front;
				m_cachedCamUp = TheCamera.Cams[TheCamera.ActiveCam].Up;
				
				// On this one we can rotate arm.
				if (CAN_AIM_WITH_ARM) {
					pointedGun = 2;
					m_bFreeAimActive = true;
					SetLookFlag(limitedCam, true, true);
					SetAimFlag(limitedCam);
					SetLookTimer(INT32_MAX);
					((CPlayerPed*)this)->m_fFPSMoveHeading = TheCamera.Find3rdPersonQuickAimPitch();
					if (m_nPedState != PED_ATTACK && m_nPedState != PED_AIM_GUN) {
						// This is a seperate ped state just for pointing gun. Used for target button
						SetPointGunAt(nil);
					}
				} else {
					m_fRotationDest = limitedCam;
					changedHeadingRate = 2;
					m_headingRate = 12.5f;

					// Anim. fix for shotgun, ak47 and m16 (we must finish rot. it quickly)
					if (weaponInfo->IsFlagSet(WEAPONFLAG_CANAIM) && padUsed->WeaponJustDown()) {
						m_fRotationCur = CGeneral::LimitRadianAngle(m_fRotationCur);
						float limitedRotDest = m_fRotationDest;

						if (m_fRotationCur - PI > m_fRotationDest) {
							limitedRotDest += 2 * PI;
						} else if (PI + m_fRotationCur < m_fRotationDest) {
							limitedRotDest -= 2 * PI;
						}

						m_fRotationCur += (limitedRotDest - m_fRotationCur) / 2;
					}
				}
			}
		}
#undef CAN_AIM_WITH_ARM
	}

	if (changedHeadingRate == 1) {
		changedHeadingRate = 0;
		RestoreHeadingRate();
	}
	if (pointedGun == 1) {
		if (m_nPedState == PED_ATTACK) {
			if (!padUsed->GetWeapon() && (m_pedIK.m_flags & CPedIK::GUN_POINTED_SUCCESSFULLY) == 0) {
				float limitedCam = CGeneral::LimitRadianAngle(-TheCamera.Orientation);

				SetAimFlag(limitedCam);
				((CPlayerPed*)this)->m_fFPSMoveHeading = TheCamera.Find3rdPersonQuickAimPitch();
				m_bFreeAimActive = true;
			}
		} else {
			pointedGun = 0;
			ClearPointGunAt();
		}
	}
#endif

#if defined IMPROVED_MENU_AND_INPUT && defined FIRING_AND_AIMING
	if (!FrontEndMenuManager.m_PrefsAutoaim)
		return;
	else {
		if (CPad::GetPad(0)->GetTarget()) {
			bool bCantAim = CPad::GetPad(0)->NewState.RightShoulder1 == 0 && CPad::GetPad(0)->Mode == 2 ||
							CPad::GetPad(0)->NewState.LeftShoulder2 < 128 && CPad::GetPad(0)->Mode == 1;

			if (bCantAim && !bIsAutoAiming || GetWeaponSlot(GetWeapon()->m_eWeaponType) == WEAPONSLOT_SNIPER || InVehicle())
				return;
		}
	}
#endif

	if (padUsed->GetTarget() && m_nSelectedWepSlot == m_currentWeapon && m_nMoveState != PEDMOVE_SPRINT && !TheCamera.Using1stPersonWeaponMode() && weaponInfo->IsFlagSet(WEAPONFLAG_CANAIM)) {
		if (m_pPointGunAt) {
			// what??
			if (!m_pPointGunAt
#ifdef FREE_CAM
				|| (!CCamera::bFreeCam && CCamera::m_bUseMouse3rdPerson)
#else
				|| CCamera::m_bUseMouse3rdPerson
#endif		
			) {
				ClearWeaponTarget();
				return;
			}

			if (m_pPointGunAt->IsPed() && (
#ifndef AIMING_VEHICLE_OCCUPANTS
				(((CPed*)m_pPointGunAt)->bInVehicle && (!((CPed*)m_pPointGunAt)->m_pMyVehicle || !((CPed*)m_pPointGunAt)->m_pMyVehicle->IsBike())) ||
#endif
				!CGame::nastyGame && ((CPed*)m_pPointGunAt)->DyingOrDead())) {
				ClearWeaponTarget();
				return;
			}
			if (CPlayerPed::DoesTargetHaveToBeBroken(m_pPointGunAt->GetPosition(), GetWeapon()) || 
				(!bCanPointGunAtTarget && !weaponInfo->IsFlagSet(WEAPONFLAG_CANAIM_WITHARM))) { // this line isn't on Mobile, idk why
				ClearWeaponTarget();
				return;
			}

			if (m_pPointGunAt) {
				RotatePlayerToTrackTarget();
			}

			if (m_pPointGunAt) {
				if (padUsed->ShiftTargetLeftJustDown())
					FindNextWeaponLockOnTarget(m_pPointGunAt, true);
				if (padUsed->ShiftTargetRightJustDown())
					FindNextWeaponLockOnTarget(m_pPointGunAt, false);
			}

#if defined IMPROVED_MENU_AND_INPUT && defined FIRING_AND_AIMING && defined FIRST_PERSON // we turn the camera to the target during autoaim
			CCam& camera = TheCamera.Cams[0];

			CVector distForBeta = m_pPointGunAt->GetPosition() - GetPosition();
			if (camera.Mode == CCam::MODE_REAL_1ST_PERSON)
				camera.Beta = distForBeta.Heading() - DEGTORAD(90.0f);
			else
				camera.Beta = distForBeta.Heading() + DEGTORAD(92.5f);

			CVector dist = GetPosition() - m_pPointGunAt->GetPosition();
			float groundDist = dist.Magnitude2D();
			camera.Alpha = CGeneral::GetATanOfXY(groundDist, -dist.z);
			while (camera.Alpha >= PI) camera.Alpha -= 2.0f * PI;
			while (camera.Alpha < -PI) camera.Alpha += 2.0f * PI;
#else
			TheCamera.SetNewPlayerWeaponMode(CCam::MODE_SYPHON, 0, 0);
			TheCamera.UpdateAimingCoors(m_pPointGunAt->GetPosition());
#endif
			
		} else if (!CCamera::m_bUseMouse3rdPerson) {
#ifdef IMPROVED_MENU_AND_INPUT
			if (!bIsAutoAiming || TheCamera.m_bJustJumpedOutOf1stPersonBecauseOfTarget)
#else
			if (padUsed->TargetJustDown() || TheCamera.m_bJustJumpedOutOf1stPersonBecauseOfTarget)
#endif
				FindWeaponLockOnTarget();
		}
	} else if (m_pPointGunAt) {
		ClearWeaponTarget();
	}

	if (m_pPointGunAt) {
		CVector markPos;
		if (m_pPointGunAt->IsPed()) {
			((CPed*)m_pPointGunAt)->m_pedIK.GetComponentPosition(markPos, PED_MID);
		} else {
			markPos = m_pPointGunAt->GetPosition();
		}

		if (bCanPointGunAtTarget) {
			CWeaponEffects::MarkTarget(markPos, 64, 0, 0, 255, 0.8f);
		} else {
			CWeaponEffects::MarkTarget(markPos, 64, 32, 0, 255, 0.8f);
		}
	}
	m_bHasLockOnTarget = m_pPointGunAt != nil;
}

bool
CPlayerPed::MovementDisabledBecauseOfTargeting(void)
{
	return m_pPointGunAt && !CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->IsFlagSet(WEAPONFLAG_CANAIM_WITHARM);
}

void
CPlayerPed::PlayerControlZelda(CPad *padUsed)
{
	float smoothSprayRate = DoWeaponSmoothSpray();
	float camOrientation = TheCamera.Orientation;
	float leftRight = padUsed->GetPedWalkLeftRight();
	float upDown = padUsed->GetPedWalkUpDown();
	float padMoveInGameUnit;
	bool smoothSprayWithoutMove = false;
#ifndef FIRING_AND_AIMING // it's need for right movement during aiming
	if (MovementDisabledBecauseOfTargeting()) {
		upDown = 0.0f;
		leftRight = 0.0f;
	}
#endif

	if (smoothSprayRate > 0.0f && upDown > 0.0f) {
		padMoveInGameUnit = 0.0f;
		smoothSprayWithoutMove = true;
	} else {
		padMoveInGameUnit = CVector2D(leftRight, upDown).Magnitude() / PAD_MOVE_TO_GAME_WORLD_MOVE;
	}

#ifdef FREE_CAM
	if (TheCamera.Cams[0].Using3rdPersonMouseCam() && smoothSprayRate > 0.0f) {
		padMoveInGameUnit = 0.0f;
		smoothSprayWithoutMove = false;
	}
#endif

	if (padMoveInGameUnit > 0.0f || smoothSprayWithoutMove) {
		float padHeading = CGeneral::GetRadianAngleBetweenPoints(0.0f, 0.0f, -leftRight, upDown);
		float neededTurn = CGeneral::LimitRadianAngle(padHeading - camOrientation);
		if (smoothSprayRate > 0.0f) {
			m_fRotationDest = m_fRotationCur - leftRight / 128.0f * smoothSprayRate * CTimer::GetTimeStep();
		} else {
			m_fRotationDest = neededTurn;
		}

#ifdef IMPROVED_MENU_AND_INPUT // walking on the key
		if (CPad::GetPad(0)->GetPedWalk() && !bIsPlayerAiming)
			m_fMoveSpeed /= 2;
#endif
		float maxAcc = 0.07f * CTimer::GetTimeStep();
		m_fMoveSpeed = Min(padMoveInGameUnit, m_fMoveSpeed + maxAcc);

	} else {
		m_fMoveSpeed = 0.0f;
	}

	if (m_nPedState == PED_JUMP) {
		if (bIsInTheAir) {
			if (bUsesCollision && !bHitSteepSlope && (!bHitSomethingLastFrame || m_vecDamageNormal.z > 0.6f)
				&& m_fDistanceTravelled < CTimer::GetTimeStepInSeconds() && m_vecMoveSpeed.MagnitudeSqr() < 0.01f) {

				float angleSin = Sin(m_fRotationCur); // originally sin(DEGTORAD(RADTODEG(m_fRotationCur))) o_O
				float angleCos = Cos(m_fRotationCur);
				ApplyMoveForce(-angleSin * 3.0f, 3.0f * angleCos, 0.05f);
			}
		} else if (bIsLanding) {
			m_fMoveSpeed = 0.0f;
		}
	}

	if (m_nPedState == PED_ANSWER_MOBILE) {
		SetRealMoveAnim();
		return;
	}

#ifdef SWIMMING
	if (!CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->IsFlagSet(WEAPONFLAG_HEAVY) && padUsed->GetSprint() && !bIsTiredSwimmingFast) {
#else
	if (!CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->IsFlagSet(WEAPONFLAG_HEAVY) && padUsed->GetSprint()) {
#endif
#if defined CROUCH && defined SWIMMING && defined FIRING_AND_AIMING
		if ((!m_pCurrentPhysSurface || (!m_pCurrentPhysSurface->bInfiniteMass || m_pCurrentPhysSurface->m_phy_flagA08)) && !bIsPlayerAiming && !RpAnimBlendClumpGetAssociation(GetClump(), ANIM_WEAPON_FIRE) &&
			!bIsDucking && !RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CROUCH_IDLE) && !RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CROUCH_FORWARD) && !bIsSwimming ||
			bIsSwimming && !RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_SWIM_TREAD))
			m_nMoveState = PEDMOVE_SPRINT;
		else if (bIsDucking && !bIsPlayerAiming) {
			ClearDuck();
			bCrouchWhenShooting = false;
		}
#else
		if (!m_pCurrentPhysSurface || (!m_pCurrentPhysSurface->bInfiniteMass || m_pCurrentPhysSurface->m_phy_flagA08))
			m_nMoveState = PEDMOVE_SPRINT;
#endif
	}

	if (m_nPedState != PED_FIGHT)
		SetRealMoveAnim();

#ifdef FIRING_AND_AIMING
	if (bIsPlayerAiming)
		return;
#endif

	if (!bIsInTheAir && !CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->IsFlagSet(WEAPONFLAG_HEAVY)
#ifdef SWIMMING
		&& padUsed->JumpJustDown() && m_nPedState != PED_JUMP && m_nPedState != PED_SWIM) {
#else
		&& padUsed->JumpJustDown() && m_nPedState != PED_JUMP) {
#endif
		ClearAttack();
		ClearWeaponTarget();
#ifdef CROUCH
		if (bIsDucking) {
			ClearDuck();

			return;
		}
#endif

#ifdef FEATURES_INI // PlayerDoesntBounceAwayFromMovingCar
		if (!bPlayerDoesntBounceAwayFromMovingCar && m_nEvadeAmount != 0 && m_pEvadingFrom) {
#else
		if (m_nEvadeAmount != 0 && m_pEvadingFrom) {
#endif
			SetEvasiveDive((CPhysical*)m_pEvadingFrom, 1);
			m_nEvadeAmount = 0;
			m_pEvadingFrom = nil;
		} else {
			SetJump();
		}
	}
	PlayIdleAnimations(padUsed);
}

// Finds nice positions for peds to duck and shoot player. And it's inside PlayerPed, this is treachery!
void
CPlayerPed::FindNewAttackPoints(void)
{
	for (int i=0; i<ARRAY_SIZE(m_pPedAtSafePos); i++) {
		CPed *safeNeighbour = m_pPedAtSafePos[i];
		if (safeNeighbour) {
			if (safeNeighbour->m_nPedState == PED_DEAD || safeNeighbour->m_pedInObjective != this) {
				m_vecSafePos[i].x = 0.0f;
				m_vecSafePos[i].y = 0.0f;
				m_vecSafePos[i].z = 0.0f;
				m_pPedAtSafePos[i] = nil;
			}
		} else {
			m_vecSafePos[i].x = 0.0f;
			m_vecSafePos[i].y = 0.0f;
			m_vecSafePos[i].z = 0.0f;
		}
	}
	CEntity *entities[6];
	int16 numEnts;
	float rightMult, fwdMult;
	CWorld::FindObjectsInRange(GetPosition(), 18.0f, true, &numEnts, 6, entities, true, false, false, true, false);
	for (int i = 0; i < numEnts; ++i) {
		CEntity *ent = entities[i];
		int16 mi = ent->GetModelIndex();
		if (!ent->IsObject() || ((CObject*)ent)->m_nSpecialCollisionResponseCases == COLLRESPONSE_FENCEPART)
			if (!IsTreeModel(mi))
				continue;

		if (mi == MI_TRAFFICLIGHTS) {
			rightMult = 2.957f;
			fwdMult = 0.147f;

		} else if (mi == MI_SINGLESTREETLIGHTS1) {
			rightMult = 0.744f;
			fwdMult = 0.0f;

		} else if (mi == MI_SINGLESTREETLIGHTS2) {
			rightMult = 0.043f;
			fwdMult = 0.0f;

		} else if (mi == MI_SINGLESTREETLIGHTS3) {
			rightMult = 1.143f;
			fwdMult = 0.145f;

		} else if (mi == MI_DOUBLESTREETLIGHTS) {
			rightMult = 0.744f;
			fwdMult = 0.0f;

		} else if (mi == MI_LAMPPOST1) {
			rightMult = 0.744f;
			fwdMult = 0.0f;

		} else if (mi == MI_TRAFFICLIGHT01) {
			rightMult = 2.957f;
			fwdMult = 0.147f;

		} else if (mi == MI_LITTLEHA_POLICE) {
			rightMult = 0.0f;
			fwdMult = 0.0f;

		} else if (mi == MI_PARKBENCH) {
			rightMult = 0.0f;
			fwdMult = 0.0f;

		} else if (IsTreeModel(mi)) {
			rightMult = 0.0f;
			fwdMult = 0.0f;
		} else
			continue;

		CVector entAttackPoint(rightMult * ent->GetRight().x + fwdMult * ent->GetForward().x + ent->GetPosition().x,
			rightMult * ent->GetRight().y + fwdMult * ent->GetForward().y + ent->GetPosition().y,
			ent->GetPosition().z);
		CVector attackerPos = GetPosition() - entAttackPoint; // for now it's dist, not attackerPos
		CVector dirTowardsUs = attackerPos;
		dirTowardsUs.Normalise();
		dirTowardsUs *= 2.0f;
		attackerPos = entAttackPoint - dirTowardsUs; // to make cop farther from us
		CPedPlacement::FindZCoorForPed(&attackerPos);
		if (CPedPlacement::IsPositionClearForPed(attackerPos))
			m_vecSafePos[i] = attackerPos;
	}
}

void
CPlayerPed::ProcessControl(void)
{
#if defined DEBUG && defined WANTED_PATHS
	CPad* pad = CPad::GetPad(0);
	if (pad->GetCharJustDown(0x31)) { // 1
		if (bIsPathRecording) {
			re3_debug("stop recording");

			char pos[10];
			int result = snprintf(pos, sizeof pos, "%i nodes", countRecords);
			CFileMgr::Write(folderRecord, pos, result);

			countRecords = 0;

			CFileMgr::CloseFile(folderRecord);
		} else {
			re3_debug("start recording");
			
			folderRecord = CFileMgr::OpenFileForWriting("data\\maps\\wanted_paths.txt");
		}

		bIsPathRecording = !bIsPathRecording;
	} else if (pad->GetCharJustDown(0x32)) { // 2
		if (bIsPathRecording) {
			char pos[100];
			int a = 0;
			if (InVehicle())
				a = snprintf(pos, sizeof pos, "\t%f %f %f %s", FindPlayerVehicle()->GetPosition().x, FindPlayerVehicle()->GetPosition().y, FindPlayerVehicle()->GetPosition().z + 1.0f, "\n");
			else
				a = snprintf(pos, sizeof pos, "\t%f %f %f %s", GetPosition().x, GetPosition().y, GetPosition().z + 1.0f, "\n");
			CFileMgr::Write(folderRecord, pos, a);

			countRecords++;
			
			re3_debug("node %i saving", countRecords);

			if (countRecords == 20) {
				re3_debug("stop recording");

				char pos[10];
				int result = snprintf(pos, sizeof pos, "%i nodes", countRecords);
				CFileMgr::Write(folderRecord, pos, result);

				countRecords = 0;
				
				CFileMgr::CloseFile(folderRecord);

				bIsPathRecording = false;
			}
		}
	}
#endif

#ifdef FIRING_AND_AIMING
	if (bIsPlayerAiming && (!IsPedInControl() && m_nPedState != PED_ROLL || m_nPedState == PED_FIGHT)) {
		bIsPlayerAiming = false;
		ClearWeaponTarget();
	}
#endif

#ifdef SWIMMING
	if (m_fCurrentStamina >= 0.0f && bIsTiredSwimmingFast)
		bIsTiredSwimmingFast = false;
#endif

#ifdef CROUCH
	if (!bIsDucking && TheCamera.Cams[TheCamera.ActiveCam].m_fTargetCameraPosZ != 0.0f)
		TheCamera.Cams[TheCamera.ActiveCam].m_fTargetCameraPosZ = 0.0f;
#endif

#if defined DEBUG && defined AUTOSAVE_AND_SAVE_ANYWHERE
	if (CPad::GetPad(0)->GetCharJustDown(0x33)) { // 3
		DoAutoSave();
	}

	if (CPad::GetPad(0)->GetCharJustDown(0x34) && PlayerCanMakeQuickSave()) { // 4
		*(int32*)&CTheScripts::ScriptSpace[CTheScripts::OnAMissionFlag] = 1;
		CTheScripts::Process();

		bAutoSave = false;
		IsQuickSave = false;
		bSaveAnywhere = true;
		FrontEndMenuManager.m_bActivateSaveMenu = true;
	}
#endif

	// Mobile has some debug/abandoned cheat thing in here: "gbFrankenTommy"

	if (m_nEvadeAmount != 0)
		--m_nEvadeAmount;

	if (m_nEvadeAmount == 0)
		m_pEvadingFrom = nil;

	if (m_pWanted->GetWantedLevel() > 0)
		FindNewAttackPoints();
	
	UpdateMeleeAttackers();

	if (m_pCurrentPhysSurface && m_pCurrentPhysSurface->IsVehicle() && ((CVehicle*)m_pCurrentPhysSurface)->IsBoat()) {
		bTryingToReachDryLand = true;

	} else if (!(((uint8)CTimer::GetFrameCounter() + m_randomSeed) & 0xF)) {
		CVehicle *nearVeh = (CVehicle*)CWorld::TestSphereAgainstWorld(GetPosition(), 7.0f, nil, false, true, false, false, false, false);
		if (nearVeh && nearVeh->IsBoat())
			bTryingToReachDryLand = true;
		else
			bTryingToReachDryLand = false;
	}

	if (m_nFadeDrunkenness) {
		if (m_nDrunkenness - 1 > 0) {
			--m_nDrunkenness;
		} else {
			m_nDrunkenness = 0;
			CMBlur::ClearDrunkBlur();
			m_nFadeDrunkenness = 0;
		}
	}
	if (m_nDrunkenness != 0) {
		CMBlur::SetDrunkBlur(m_nDrunkenness / 255.f);
	}
	CPed::ProcessControl();
	SetNearbyPedsToInteractWithPlayer();
	if (bWasPostponed)
		return;

	CPad *padUsed = GetPadFromPlayer(this);
	m_pWanted->Update();
	PruneReferences();

	if (GetWeapon()->m_eWeaponType == WEAPONTYPE_MINIGUN) {
		CWeaponInfo *weaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
		CAnimBlendAssociation *fireAnim = RpAnimBlendClumpGetAssociation(GetClump(), GetPrimaryFireAnim(weaponInfo));
		if (fireAnim && fireAnim->currentTime - fireAnim->timeStep < weaponInfo->m_fAnimLoopEnd && m_nPedState == PED_ATTACK) {
			if (m_fGunSpinSpeed < 0.45f) {
				m_fGunSpinSpeed = Min(0.45f, m_fGunSpinSpeed + CTimer::GetTimeStep() * 0.013f);
			}

			if (padUsed->GetWeapon() && GetWeapon()->m_nAmmoTotal > 0 && fireAnim->currentTime >= weaponInfo->m_fAnimLoopStart) {
				DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_MINIGUN_ATTACK, 0.0f);
			} else {
				DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_MINIGUN_2, m_fGunSpinSpeed * (20.f / 9));
			}
		} else {
			if (m_fGunSpinSpeed > 0.0f) {
				if (m_fGunSpinSpeed >= 0.45f) {
					DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_MINIGUN_3,  0.0f);
				}
				m_fGunSpinSpeed = Max(0.0f, m_fGunSpinSpeed - CTimer::GetTimeStep() * 0.003f);
			}
		}
	}
	if (GetWeapon()->m_eWeaponType == WEAPONTYPE_CHAINSAW && m_nPedState != PED_ATTACK && !bInVehicle) {
		DMAudio.PlayOneShot(m_audioEntityId, SOUND_WEAPON_CHAINSAW_IDLE, 0.0f);
	}

	if (m_nMoveState != PEDMOVE_RUN && m_nMoveState != PEDMOVE_SPRINT)
		RestoreSprintEnergy(1.0f);
	else if (m_nMoveState == PEDMOVE_RUN)
		RestoreSprintEnergy(0.3f);

	if (m_nPedState == PED_DEAD) {
		ClearWeaponTarget();
		return;
	}
	if (m_nPedState == PED_DIE) {
		ClearWeaponTarget();
		if (CTimer::GetTimeInMilliseconds() > m_bloodyFootprintCountOrDeathTime + 4000)
			SetDead();
		return;
	}
	if (m_nPedState == PED_DRIVING && m_objective != OBJECTIVE_LEAVE_CAR) {
		if (!CReplay::IsPlayingBack() || m_pMyVehicle) {
			if (m_pMyVehicle->IsCar() && ((CAutomobile*)m_pMyVehicle)->Damage.GetDoorStatus(DOOR_FRONT_LEFT) == DOOR_STATUS_SWINGING) {
				CAnimBlendAssociation *rollDoorAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_CLOSE_DOOR_ROLLING_LHS);

				if (m_pMyVehicle->m_nGettingOutFlags & CAR_DOOR_FLAG_LF || rollDoorAssoc || (rollDoorAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CAR_CLOSE_DOOR_ROLLING_LO_LHS))) {
					if (rollDoorAssoc)
						m_pMyVehicle->ProcessOpenDoor(CAR_DOOR_LF, ANIM_STD_CAR_CLOSE_DOOR_ROLLING_LHS, rollDoorAssoc->currentTime);

				} else {
					// These comparisons are wrong, they return uint16
					if (padUsed && (padUsed->GetAccelerate() != 0.0f || padUsed->GetSteeringLeftRight() != 0.0f || padUsed->GetBrake() != 0.0f)) {
						if (rollDoorAssoc)
							m_pMyVehicle->ProcessOpenDoor(CAR_DOOR_LF, ANIM_STD_CAR_CLOSE_DOOR_ROLLING_LHS, rollDoorAssoc->currentTime);

					} else {
						m_pMyVehicle->m_nGettingOutFlags |= CAR_DOOR_FLAG_LF;
						if (m_pMyVehicle->bLowVehicle)
							rollDoorAssoc = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_CLOSE_DOOR_ROLLING_LO_LHS);
						else
							rollDoorAssoc = CAnimManager::AddAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_CAR_CLOSE_DOOR_ROLLING_LHS);

						rollDoorAssoc->SetFinishCallback(PedAnimDoorCloseRollingCB, this);
					}
				}
			}
		}
		return;
	}
	if (m_objective == OBJECTIVE_NONE)
		m_nMoveState = PEDMOVE_STILL;
	if (bIsLanding)
		RunningLand(padUsed);

#ifndef FIRING_AND_AIMING
	if (padUsed && padUsed->WeaponJustDown() && !TheCamera.Using1stPersonWeaponMode()) {
		// ...Really?
		eWeaponType playerWeapon = FindPlayerPed()->GetWeapon()->m_eWeaponType;
		if (playerWeapon == WEAPONTYPE_SNIPERRIFLE || playerWeapon == WEAPONTYPE_LASERSCOPE) {
			DMAudio.PlayFrontEndSound(SOUND_WEAPON_SNIPER_SHOT_NO_ZOOM, 0);
		} else if (playerWeapon == WEAPONTYPE_ROCKETLAUNCHER) {
			DMAudio.PlayFrontEndSound(SOUND_WEAPON_ROCKET_SHOT_NO_ZOOM, 0);
		}
	}
#endif

#ifdef CROUCH
	if (m_nPedState == PED_ROLL && !bIsStanding) {
		RestorePreviousState();

		ClearDuck();
	}

	if (m_nPedState == PED_ROLL && !RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CROUCH_ROLL_L) && !RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_CROUCH_ROLL_R)) {
		SetRealMoveAnim();
	}
#endif

#ifdef FIRING_AND_AIMING
	if (InVehicle() && bIsPlayerAiming && m_pMyVehicle->GetUp().z < 0.2f)
		ClearWeaponTarget();
#endif

#if defined FIRING_AND_AIMING && defined FIRST_PERSON
	if (IsDoomMode() && (!IsPedInControl() || m_nPedState == PED_ANSWER_MOBILE || m_nPedState == PED_SEEK_CAR || m_nPedState == PED_SEEK_IN_BOAT)) {
		bIsPlayerAiming = false;
		ClearWeaponTarget();

		StopAimingAnims();
	}
#endif

#ifdef IMPROVED_TECH_PART // skip a phone call
	if (m_nPedState == PED_ANSWER_MOBILE && m_bSkipPhoneCall) {
		for (int slot = 0; slot < MISSION_AUDIO_SLOTS; slot++)
			AudioManager.FinishMissionAudioPhoneDialogue(slot);
	} else if (m_nPedState == PED_ANSWER_MOBILE && !m_bSkipPhoneCall && CPad::GetPad(0)->ExitVehicleJustDown()) {
		m_bSkipPhoneCall = true;
	}
#endif

	switch (m_nPedState) {
		case PED_NONE:
		case PED_IDLE:
		case PED_FLEE_POS:
		case PED_FLEE_ENTITY:
		case PED_ATTACK:
		case PED_FIGHT:
		case PED_AIM_GUN:
		case PED_ANSWER_MOBILE:
#ifdef SWIMMING
		case PED_SWIM:
#endif
#ifdef FIRING_AND_AIMING
			if (!RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_BLOCK) && !m_attachedTo && !InVehicle()) {
#else
			if (!RpAnimBlendClumpGetFirstAssociation(GetClump(), ASSOC_BLOCK) && !m_attachedTo) {
#endif
				if (TheCamera.Using1stPersonWeaponMode()) {
					if (padUsed)
						PlayerControlSniper(padUsed);

				} else if (TheCamera.Cams[0].Using3rdPersonMouseCam()
#ifdef FREE_CAM
					&& !CCamera::bFreeCam
#endif
					) {
					if (padUsed)
						PlayerControl1stPersonRunAround(padUsed);

				} else if (m_nPedState == PED_FIGHT) {
					if (padUsed)
						PlayerControlFighter(padUsed);

				} else if (padUsed) {
					PlayerControlZelda(padUsed);
				}
			}
			if (IsPedInControl() && m_nPedState != PED_ANSWER_MOBILE && padUsed)
				ProcessPlayerWeapon(padUsed);
			break;
		case PED_SEEK_ENTITY:
			m_vecSeekPos = m_pSeekTarget->GetPosition();

			// fall through
		case PED_SEEK_POS:
			switch (m_nMoveState) {
				case PEDMOVE_WALK:
					m_fMoveSpeed = 1.0f;
					break;
				case PEDMOVE_RUN:
					m_fMoveSpeed = 1.8f;
					break;
				case PEDMOVE_SPRINT:
					m_fMoveSpeed = 2.5f;
					break;
				default:
					m_fMoveSpeed = 0.0f;
					break;
			}
			SetRealMoveAnim();
			if (Seek()) {
				RestorePreviousState();
				SetMoveState(PEDMOVE_STILL);
			}
			break;
		case PED_SNIPER_MODE:
			if (GetWeapon()->m_eWeaponType == WEAPONTYPE_SNIPERRIFLE || GetWeapon()->m_eWeaponType == WEAPONTYPE_LASERSCOPE) {
				if (padUsed)
					PlayerControlSniper(padUsed);

			} else if (padUsed) {
				PlayerControlM16(padUsed);
			}
			break;
		case PED_SEEK_CAR:
		case PED_SEEK_IN_BOAT:
			if (bVehEnterDoorIsBlocked || bKindaStayInSamePlace) {
				m_fMoveSpeed = 0.0f;
			} else {
				m_fMoveSpeed = Min(2.0f, 2.0f * (m_vecSeekPos - GetPosition()).Magnitude2D());
			}
			if (padUsed && !padUsed->ArePlayerControlsDisabled()) {
				if (padUsed->GetTarget() || padUsed->GetLeftStickXJustDown() || padUsed->GetLeftStickYJustDown() ||
					padUsed->GetDPadUpJustDown() || padUsed->GetDPadDownJustDown() || padUsed->GetDPadLeftJustDown() ||
					padUsed->GetDPadRightJustDown()) {

					RestorePreviousState();
					if (m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER || m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER) {
						RestorePreviousObjective();
					}
				}
			}
			if (padUsed && padUsed->GetSprint())
				m_nMoveState = PEDMOVE_SPRINT;
			SetRealMoveAnim();
			break;
		case PED_JUMP:
			if (padUsed)
				PlayerControlZelda(padUsed);
			if (bIsLanding)
				break;

			// This has been added later it seems
			return;
		case PED_FALL:
		case PED_GETUP:
		case PED_ENTER_TRAIN:
		case PED_EXIT_TRAIN:
		case PED_CARJACK:
		case PED_DRAG_FROM_CAR:
		case PED_ENTER_CAR:
		case PED_STEAL_CAR:
		case PED_EXIT_CAR:
			ClearWeaponTarget();
			break;
		case PED_ARRESTED:
			if (m_nLastPedState == PED_DRAG_FROM_CAR && m_pVehicleAnim)
				BeingDraggedFromCar();
			break;
		default:
			break;
	}
	if (padUsed && IsPedShootable() && m_nPedState != PED_ANSWER_MOBILE && m_nLastPedState != PED_ANSWER_MOBILE) {
		ProcessWeaponSwitch(padUsed);
		GetWeapon()->Update(m_audioEntityId, this);
	}
	ProcessAnimGroups();
	if (padUsed) {
		if (TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_FOLLOWPED
			&& TheCamera.Cams[TheCamera.ActiveCam].DirectionWasLooking == LOOKING_BEHIND) {

			m_lookTimer = 0;
			float camAngle = CGeneral::LimitRadianAngle(TheCamera.Cams[TheCamera.ActiveCam].Front.Heading());
			float angleBetweenPlayerAndCam = Abs(camAngle - m_fRotationCur);

			if (m_nPedState != PED_ATTACK && angleBetweenPlayerAndCam > DEGTORAD(30.0f) && angleBetweenPlayerAndCam < DEGTORAD(330.0f)) {
				if (angleBetweenPlayerAndCam > DEGTORAD(150.0f) && angleBetweenPlayerAndCam < DEGTORAD(210.0f)) {
					float rightTurnAngle = CGeneral::LimitRadianAngle(m_fRotationCur - DEGTORAD(150.0f));
					float leftTurnAngle = CGeneral::LimitRadianAngle(DEGTORAD(150.0f) + m_fRotationCur);

					if (m_fLookDirection == 999999.0f || bIsDucking)
						camAngle = rightTurnAngle;
					else if (Abs(rightTurnAngle - m_fLookDirection) < Abs(leftTurnAngle - m_fLookDirection))
						camAngle = rightTurnAngle;
					else
						camAngle = leftTurnAngle;
				}
				SetLookFlag(camAngle, true);
				SetLookTimer(CTimer::GetTimeStepInMilliseconds() * 5.0f);
			} else {
				ClearLookFlag();
			}
		}
	}
	if (m_nMoveState == PEDMOVE_SPRINT && bIsLooking) {
		ClearLookFlag();
		SetLookTimer(250);
	}

	if (m_vecMoveSpeed.Magnitude2D() < 0.1f) {
		if (m_nSpeedTimer) {
			if (CTimer::GetTimeInMilliseconds() > m_nSpeedTimer)
				m_bSpeedTimerFlag = true;
		} else {
			m_nSpeedTimer = CTimer::GetTimeInMilliseconds() + 500;
		}
	} else {
		m_nSpeedTimer = 0;
		m_bSpeedTimerFlag = false;
	}

	if (bDontAllowWeaponChange && FindPlayerPed() == this) {
#ifdef CROUCH
		if (!CPad::GetPad(0)->GetTarget() && m_nPedState != PED_ROLL)
#else
		if (!CPad::GetPad(0)->GetTarget())
#endif
			bDontAllowWeaponChange = false;
	}

	if (m_nPedState != PED_SNIPER_MODE && (GetWeapon()->m_eWeaponState == WEAPONSTATE_FIRING || m_nPedState == PED_ATTACK))
		m_nPadDownPressedInMilliseconds = CTimer::GetTimeInMilliseconds();

	if (!bIsVisible)
		UpdateRpHAnim();
}

bool
CPlayerPed::DoesPlayerWantNewWeapon(eWeaponType weapon, bool onlyIfSlotIsEmpty)
{
	// GetPadFromPlayer(); // unused
	uint32 slot = CWeaponInfo::GetWeaponInfo(weapon)->m_nWeaponSlot;

	if (!HasWeaponSlot(slot) || GetWeapon(slot).m_eWeaponType == weapon)
		return true;

	if (onlyIfSlotIsEmpty)
		return false;

	// Check if he's using that slot right now.
	return m_nPedState != PED_ATTACK && m_nPedState != PED_AIM_GUN || slot != m_currentWeapon;
}

void
CPlayerPed::PlayIdleAnimations(CPad *padUsed)
{
	CAnimBlendAssociation* assoc;

#ifdef SWIMMING
	if (bIsSwimming)
		return;
#endif

#ifdef CLIMBING
	if (bIsClimbing)
		return;
#endif

	if (TheCamera.m_WideScreenOn || bIsDucking)
		return;

	struct animAndGroup {
		AnimationId animId;
		AssocGroupId groupId;
	};

	const animAndGroup idleAnims[] = {
		{ANIM_PLAYER_IDLE1, ASSOCGRP_PLAYER_IDLE},
		{ANIM_PLAYER_IDLE2, ASSOCGRP_PLAYER_IDLE},
		{ANIM_PLAYER_IDLE3, ASSOCGRP_PLAYER_IDLE},
		{ANIM_PLAYER_IDLE4, ASSOCGRP_PLAYER_IDLE},
		{ANIM_STD_XPRESS_SCRATCH, ASSOCGRP_STD},
	};

	static int32 lastTime = 0;
	static int32 lastAnim = -1;

	bool hasIdleAnim = false;
	CAnimBlock *idleAnimBlock = CAnimManager::GetAnimationBlock(idleAnimBlockIndex);
	uint32 sinceLastInput = padUsed->InputHowLongAgo();
	if (sinceLastInput <= 30000) {
		if (idleAnimBlock->isLoaded) {
			for (assoc = RpAnimBlendClumpGetFirstAssociation(GetClump()); assoc; assoc = RpAnimBlendGetNextAssociation(assoc)) {
				if (assoc->flags & ASSOC_IDLE) {
					hasIdleAnim = true;
					assoc->blendDelta = -8.0f;
				}
			}
			if (!hasIdleAnim)
				CStreaming::RemoveAnim(idleAnimBlockIndex);
		} else {
			lastTime = 0;
		}
	} else {
		CStreaming::RequestAnim(idleAnimBlockIndex, STREAMFLAGS_DONT_REMOVE);
		if (idleAnimBlock->isLoaded) {
			for(CAnimBlendAssociation *assoc = RpAnimBlendClumpGetFirstAssociation(GetClump()); assoc; assoc = RpAnimBlendGetNextAssociation(assoc)) {
				int firstIdle = idleAnimBlock->firstIndex;
				int index = assoc->hierarchy - CAnimManager::GetAnimation(0);
				if (index >= firstIdle && index < firstIdle + idleAnimBlock->numAnims) {
					hasIdleAnim = true;
					break;
				}
			}

			if (!hasIdleAnim && !bIsLooking && !bIsRestoringLook && sinceLastInput - lastTime > 25000) {
				int anim;
				do
					anim = CGeneral::GetRandomNumberInRange(0, ARRAY_SIZE(idleAnims));
				while (lastAnim == anim);

				assoc = CAnimManager::BlendAnimation(GetClump(), idleAnims[anim].groupId, idleAnims[anim].animId, 8.0f);
				assoc->flags |= ASSOC_IDLE;
				lastAnim = anim;
				lastTime = sinceLastInput;
			}
		}
	}
}

void
CPlayerPed::SetNearbyPedsToInteractWithPlayer(void)
{
	if (CGame::noProstitutes)
		return;

	for (int i = 0; i < m_numNearPeds; ++i) {
		CPed *nearPed = m_nearPeds[i];
		if (nearPed && nearPed->m_objectiveTimer < CTimer::GetTimeInMilliseconds() && !CTheScripts::IsPlayerOnAMission()) {
			int mi = nearPed->GetModelIndex();
			if (CPopulation::CanSolicitPlayerOnFoot(mi)) {
				CVector distToMe = nearPed->GetPosition() - GetPosition();
				CVector dirToMe = GetPosition() - nearPed->GetPosition();
				dirToMe.Normalise();
				if (DotProduct(dirToMe, nearPed->GetForward()) > 0.707 && DotProduct(GetForward(), nearPed->GetForward()) < -0.707 // those are double
					&& distToMe.MagnitudeSqr() < 9.0f && nearPed->m_objective == OBJECTIVE_NONE) {
					nearPed->SetObjective(OBJECTIVE_SOLICIT_FOOT, this);
					nearPed->m_objectiveTimer = CTimer::GetTimeInMilliseconds() + 10000;
					nearPed->Say(SOUND_PED_SOLICIT);
				}
			} else if (CPopulation::CanSolicitPlayerInCar(mi)) {
				if (InVehicle() && m_pMyVehicle->IsVehicleNormal()) {
					if (m_pMyVehicle->IsCar()) {
						CVector distToVeh = nearPed->GetPosition() - m_pMyVehicle->GetPosition();
						if (distToVeh.MagnitudeSqr() < 25.0f && m_pMyVehicle->IsRoomForPedToLeaveCar(CAR_DOOR_LF, nil) && nearPed->m_objective == OBJECTIVE_NONE) {
							nearPed->SetObjective(OBJECTIVE_SOLICIT_VEHICLE, m_pMyVehicle);
						}
					}
				}
			}
		}
	}
}

void
CPlayerPed::UpdateMeleeAttackers(void)
{
	CVector attackCoord;
	if (((CTimer::GetFrameCounter() + m_randomSeed + 7) & 3) == 0) {
		GetMeleeAttackCoords(attackCoord, m_nAttackDirToCheck, 2.0f);

		// Check if there is any vehicle/building inbetween us and m_nAttackDirToCheck. Peds will be able to attack us from those available directions.
		if (CWorld::GetIsLineOfSightClear(GetPosition(), attackCoord, true, true, false, true, false, false, false)
			&& !CWorld::TestSphereAgainstWorld(attackCoord, 0.4f, m_pMeleeList[m_nAttackDirToCheck], true, true, false, true, false, false)) {
			if (m_pMeleeList[m_nAttackDirToCheck] == this)
				m_pMeleeList[m_nAttackDirToCheck] = nil; // mark it as available
		} else {
			m_pMeleeList[m_nAttackDirToCheck] = this; // slot not available. useful for m_bNoPosForMeleeAttack
		}
		if (++m_nAttackDirToCheck >= ARRAY_SIZE(m_pMeleeList))
			m_nAttackDirToCheck = 0;
	}
	// 6 directions
	for (int i = 0; i < ARRAY_SIZE(m_pMeleeList); ++i) {
		CPed *victim = m_pMeleeList[i];
		if (victim && victim != this) {
			if (victim->m_nPedState != PED_DEAD && victim->m_pedInObjective == this)  {
				if (victim->m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT || victim->m_objective == OBJECTIVE_KILL_CHAR_ANY_MEANS || victim->m_objective == OBJECTIVE_KILL_CHAR_ON_BOAT) {
					GetMeleeAttackCoords(attackCoord, i, 2.0f);
					if ((attackCoord - GetPosition()).MagnitudeSqr() > 12.25f)
						m_pMeleeList[i] = nil;
				} else {
					m_pMeleeList[i] = nil;
				}
			} else {
				m_pMeleeList[i] = nil;
			}
		}
	}
	m_bNoPosForMeleeAttack = m_pMeleeList[0] == this && m_pMeleeList[1] == this && m_pMeleeList[2] == this
#ifdef FIX_BUGS
		&& m_pMeleeList[3] == this
#endif
		&& m_pMeleeList[4] == this && m_pMeleeList[5] == this;
}

void
CPlayerPed::RemovePedFromMeleeList(CPed *ped)
{
	for (uint16 i = 0; i < ARRAY_SIZE(m_pMeleeList); i++) {
		if (m_pMeleeList[i] == ped) {
			m_pMeleeList[i] = nil;
			ped->m_attackTimer = 0;
			return;
		}
	}
}

void
CPlayerPed::GetMeleeAttackCoords(CVector& coords, int8 dir, float dist)
{
	coords = GetPosition();
	switch (dir) {
		case 0:
			coords.y += dist;
			break;
		case 1:
			coords.x += Sqrt(3.f / 4.f) * dist;
			coords.y += 0.5f * dist;
			break;
		case 2:
			coords.x += Sqrt(3.f / 4.f) * dist;
			coords.y -= 0.5f * dist;
			break;
		case 3:
			coords.y -= dist;
			break;
		case 4:
			coords.x -= Sqrt(3.f / 4.f) * dist;
			coords.y -= 0.5f * dist;
			break;
		case 5:
			coords.x -= Sqrt(3.f / 4.f) * dist;
			coords.y += 0.5f * dist;
			break;
		default:
			break;
	}
}

int32
CPlayerPed::FindMeleeAttackPoint(CPed *victim, CVector &dist, uint32 &endOfAttackOut)
{
	endOfAttackOut = 0;
	bool thereIsAnEmptySlot = false;
	int dirToAttack = -1;
	for (int i = 0; i < ARRAY_SIZE(m_pMeleeList); i++) {
		CPed* pedAtThisDir = m_pMeleeList[i];
		if (pedAtThisDir) {
			if (pedAtThisDir == victim) {
				dirToAttack = i;
			} else {
				if (pedAtThisDir->m_attackTimer > endOfAttackOut)
					endOfAttackOut = pedAtThisDir->m_attackTimer;
			}
		} else {
			thereIsAnEmptySlot = true;
		}
	}

	// We don't have victim ped in our melee list
	if (dirToAttack == -1 && thereIsAnEmptySlot) {
		float angle = Atan2(-dist.x, -dist.y);
		float adjustedAngle = angle + DEGTORAD(30.0f);
		if (adjustedAngle < 0.f)
			adjustedAngle += TWOPI;

		int wantedDir = Floor(adjustedAngle / DEGTORAD(60.0f));

		// And we have another ped at the direction of victim ped, so store victim to next empty direction to it's real direction. (Bollocks)
		if (m_pMeleeList[wantedDir]) {
			int closestDirToPreferred = -99;
			int preferredDir = wantedDir;

			for (int i = 0; i < ARRAY_SIZE(m_pMeleeList); i++) {
				if (!m_pMeleeList[i]) {
					if (Abs(i - preferredDir) < Abs(closestDirToPreferred - preferredDir))
						closestDirToPreferred = i;
				}
			}
			if (closestDirToPreferred > 0)
				dirToAttack = closestDirToPreferred;
		} else {

			// Luckily the direction of victim ped is already empty, good
			dirToAttack = wantedDir;
		}

		if (dirToAttack != -1) {
			m_pMeleeList[dirToAttack] = victim;
			victim->RegisterReference((CEntity**) &m_pMeleeList[dirToAttack]);
			if (endOfAttackOut > CTimer::GetTimeInMilliseconds())
				victim->m_attackTimer = endOfAttackOut + CGeneral::GetRandomNumberInRange(1000, 2000);
			else
				victim->m_attackTimer = CTimer::GetTimeInMilliseconds() + CGeneral::GetRandomNumberInRange(500, 1000);
		}
	}
	return dirToAttack;
}

#ifdef COMPATIBLE_SAVES
#define CopyFromBuf(buf, data) memcpy(&data, buf, sizeof(data)); SkipSaveBuf(buf, sizeof(data));
#define CopyToBuf(buf, data) memcpy(buf, &data, sizeof(data)); SkipSaveBuf(buf, sizeof(data));
void
CPlayerPed::Save(uint8*& buf)
{
	CPed::Save(buf);
	ZeroSaveBuf(buf, 16);
	CopyToBuf(buf, m_fMaxStamina);
	ZeroSaveBuf(buf, 28);
	CopyToBuf(buf, m_nTargettableObjects[0]);
	CopyToBuf(buf, m_nTargettableObjects[1]);
	CopyToBuf(buf, m_nTargettableObjects[2]);
	CopyToBuf(buf, m_nTargettableObjects[3]);
	ZeroSaveBuf(buf, 164);
}

void
CPlayerPed::Load(uint8*& buf)
{
	CPed::Load(buf);
	SkipSaveBuf(buf, 16);
	CopyFromBuf(buf, m_fMaxStamina);
	SkipSaveBuf(buf, 28);
	CopyFromBuf(buf, m_nTargettableObjects[0]);
	CopyFromBuf(buf, m_nTargettableObjects[1]);
	CopyFromBuf(buf, m_nTargettableObjects[2]);
	CopyFromBuf(buf, m_nTargettableObjects[3]);
	SkipSaveBuf(buf, 164);
}
#undef CopyFromBuf
#undef CopyToBuf
#endif
