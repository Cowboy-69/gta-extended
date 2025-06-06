#include "common.h"

#include "World.h"
#include "PlayerPed.h"
#include "CopPed.h"
#include "Wanted.h"
#include "DMAudio.h"
#include "ModelIndices.h"
#include "Vehicle.h"
#include "RpAnimBlend.h"
#include "AnimBlendAssociation.h"
#include "General.h"
#include "ZoneCull.h"
#include "PathFind.h"
#include "RoadBlocks.h"
#include "CarCtrl.h"
#include "Renderer.h"
#include "Camera.h"
#include "PedPlacement.h"
#include "Ropes.h"
#include "Stinger.h"
#ifdef IMPROVED_TECH_PART && DEBUG // wanted system
#include "Debug.h"
#endif
#ifdef IMPROVED_TECH_PART // AI
#include "WaterLevel.h"
#endif

CCopPed::CCopPed(eCopType copType, int32 modifier) : CPed(PEDTYPE_COP)
{
	m_nCopType = copType;
	switch (copType) {
	case COP_STREET:
		SetModelIndex(MI_COP);
		GiveWeapon(WEAPONTYPE_NIGHTSTICK, 1000, true);
#ifdef EX_WEAPON_BERETTA // COP_STREET - random weapon
		if (CGeneral::GetRandomTrueFalse())
			GiveDelayedWeapon(WEAPONTYPE_BERETTA, 1000);
		else
			GiveDelayedWeapon(WEAPONTYPE_COLT45, 1000);
#else
		GiveDelayedWeapon(WEAPONTYPE_COLT45, 1000);
#endif
		m_currentWeapon = WEAPONTYPE_UNARMED;
		m_fArmour = 0.0f;
		m_wepSkills = 208; /* TODO: what is this? seems unused */
#ifdef IMPROVED_TECH_PART // AI accuracy
		m_wepAccuracy = 75;
#else
		m_wepAccuracy = 60;
#endif
		break;
	case COP_FBI:
		SetModelIndex(MI_FBI);
#ifdef EX_WEAPON_AK47 // COP_FBI - random weapon
		if (CGeneral::GetRandomTrueFalse()) {
			GiveDelayedWeapon(WEAPONTYPE_AK47, 1000);
			SetCurrentWeapon(WEAPONTYPE_AK47);
		} else {
			GiveDelayedWeapon(WEAPONTYPE_MP5, 1000);
			SetCurrentWeapon(WEAPONTYPE_MP5);
		}
#else
		GiveDelayedWeapon(WEAPONTYPE_MP5, 1000);
		SetCurrentWeapon(WEAPONTYPE_MP5);
#endif
		m_fArmour = 100.0f;
		m_wepSkills = 176; /* TODO: what is this? seems unused */
#ifdef IMPROVED_TECH_PART // AI accuracy
		m_wepAccuracy = 74;
#else
		m_wepAccuracy = 76;
#endif
		break;
	case COP_SWAT:
	case COP_HELI_SWAT:
		SetModelIndex(MI_SWAT);
#ifdef IMPROVED_TECH_PART // wanted system
		GiveDelayedWeapon(WEAPONTYPE_TEARGAS, 1);
#endif
#ifdef IMPROVED_TECH_PART // COP_SWAT - M4 weapon
		GiveDelayedWeapon(WEAPONTYPE_M4, 1000);
		SetCurrentWeapon(WEAPONTYPE_M4);
#else
		GiveDelayedWeapon(WEAPONTYPE_UZI, 1000);
		SetCurrentWeapon(WEAPONTYPE_UZI);
#endif
		m_fArmour = 50.0f;
		m_wepSkills = 32; /* TODO: what is this? seems unused */
#ifdef IMPROVED_TECH_PART // AI accuracy
		m_wepAccuracy = 72;
#else
		m_wepAccuracy = 68;
#endif
		break;
	case COP_ARMY:
		SetModelIndex(MI_ARMY);
#ifdef IMPROVED_TECH_PART // AI
#if defined EX_WEAPON_M16 && defined EX_WEAPON_STEYR // COP_ARMY - random weapon
		if (CGeneral::GetRandomTrueFalse()) {
			GiveDelayedWeapon(WEAPONTYPE_M16, 1000);
			SetCurrentWeapon(WEAPONTYPE_M16);
		} else {
			GiveDelayedWeapon(WEAPONTYPE_STEYR, 1000);
			SetCurrentWeapon(WEAPONTYPE_STEYR);
		}
#elif defined EX_WEAPON_M16 // COP_ARMY - random weapon
		GiveDelayedWeapon(WEAPONTYPE_M16, 1000);
		SetCurrentWeapon(WEAPONTYPE_M16);
#elif defined EX_WEAPON_STEYR
		GiveDelayedWeapon(WEAPONTYPE_STEYR, 1000);
		SetCurrentWeapon(WEAPONTYPE_STEYR);
#else
		GiveDelayedWeapon(WEAPONTYPE_M4, 1000);
		SetCurrentWeapon(WEAPONTYPE_M4);
#endif
#else
		GiveDelayedWeapon(WEAPONTYPE_MP5, 1000);
		SetCurrentWeapon(WEAPONTYPE_MP5);
#endif
		m_fArmour = 100.0f;
		m_wepSkills = 32; /* TODO: what is this? seems unused */
#ifdef IMPROVED_TECH_PART // AI accuracy
		m_wepAccuracy = 80;
#else
		m_wepAccuracy = 84;
#endif
		break;
	case COP_MIAMIVICE:
		switch (modifier) {
		case 0: SetModelIndex(MI_VICE1); break;
		case 1: SetModelIndex(MI_VICE2); break;
		case 2: SetModelIndex(MI_VICE3); break;
		case 3: SetModelIndex(MI_VICE4); break;
		case 4: SetModelIndex(MI_VICE5); break;
		case 5: SetModelIndex(MI_VICE6); break;
		case 6: SetModelIndex(MI_VICE7); break;
		case 7: SetModelIndex(MI_VICE8); break;
		default: assert(0); break;
		}
#ifdef EX_WEAPON_UZIOLD // COP_MIAMIVICE - random weapon
		if (CGeneral::GetRandomTrueFalse()) {
			GiveDelayedWeapon(WEAPONTYPE_UZIOLD, 1000);
			SetCurrentWeapon(WEAPONTYPE_UZIOLD);
		} else {
			GiveDelayedWeapon(WEAPONTYPE_UZI, 1000);
			SetCurrentWeapon(WEAPONTYPE_UZI);
		}
#else
		GiveDelayedWeapon(WEAPONTYPE_UZI, 1000);
		SetCurrentWeapon(WEAPONTYPE_UZI);
#endif
		m_fArmour = 100.0f;
		m_wepSkills = 176;
#ifdef IMPROVED_TECH_PART // AI accuracy
		m_wepAccuracy = 73;
#else
		m_wepAccuracy = 76;
#endif
		break;
	}
	m_bIsInPursuit = false;
	field_5FE = 1;
	m_bIsDisabledCop = false;
	m_attackTimer = 0;
	m_bBeatingSuspect = false;
	m_bStopAndShootDisabledZone = false;
	m_bDragsPlayerFromCar = false;
	m_bZoneDisabled = false;
	field_628 = -1;
	m_nRoadblockVeh = nil;
	m_bThrowsSpikeTrap = false;
	m_pRopeEntity = nil;
	m_fAbseilPos = 0.0f;
	m_nHassleTimer = 0;
	field_61C = 0;
	field_624 = 0;
	m_pStinger = new CStinger();
	SetWeaponLockOnTarget(nil);
#ifdef IMPROVED_TECH_PART // wanted system
	m_vecLastPosPlayerBeforeHiding = FindPlayerCoors();
	m_vecPotentialPlayerSearchCoordinates = CVector(0.0f, 0.0f, 0.0f);
	m_bLookingForPlayer = false;
	m_bMovesToLastPlayerPosition = false;
	m_bThrowingTeargas = false;
	m_bUsedTeargas = false;
#endif
}

CCopPed::~CCopPed()
{
	ClearPursuit();
	m_pStinger->Remove();
	delete m_pStinger;
}

// Parameter should always be CPlayerPed, but it seems they considered making civilians arrestable at some point
void
CCopPed::SetArrestPlayer(CPed *player)
{
	if (!IsPedInControl() || !player)
		return;

	player->Say(SOUND_PED_PLAYER_REACTTOCOP);
	Say(SOUND_PED_ARREST_COP);

	if (player->EnteringCar()) {
		if (CTimer::GetTimeInMilliseconds() > m_nPedStateTimer)
			return;

		// why?
		player->bGonnaKillTheCarJacker = true;

		// Genius
		FindPlayerPed()->m_bCanBeDamaged = false;
		((CPlayerPed*)player)->m_pArrestingCop = this;
		this->RegisterReference((CEntity**) &((CPlayerPed*)player)->m_pArrestingCop);

	} else if (player->m_nPedState != PED_DIE && player->m_nPedState != PED_DEAD && player->m_nPedState != PED_ARRESTED) {
		player->m_nLastPedState = player->m_nPedState;
		player->SetPedState(PED_ARRESTED);

		FindPlayerPed()->m_bCanBeDamaged = false;
		((CPlayerPed*)player)->m_pArrestingCop = this;
		this->RegisterReference((CEntity**) &((CPlayerPed*)player)->m_pArrestingCop);
	}

	SetPedState(PED_ARREST_PLAYER);
	SetObjective(OBJECTIVE_NONE);
	m_prevObjective = OBJECTIVE_NONE;
	bIsPointingGunAt = false;
	m_pSeekTarget = player;
	m_pSeekTarget->RegisterReference((CEntity**) &m_pSeekTarget);
	SetCurrentWeapon(WEAPONTYPE_COLT45);
	if (player->InVehicle()) {
		player->m_pMyVehicle->m_nNumGettingIn = 0;
		player->m_pMyVehicle->m_nGettingInFlags = 0;
		player->m_pMyVehicle->bIsHandbrakeOn = true;
		player->m_pMyVehicle->SetStatus(STATUS_PLAYER_DISABLED);
	}
	if (GetWeapon()->m_eWeaponType == WEAPONTYPE_UNARMED || GetWeapon()->m_eWeaponType == WEAPONTYPE_BRASSKNUCKLE)
		SetCurrentWeapon(WEAPONTYPE_COLT45);
}

void
CCopPed::ClearPursuit(void)
{
	CPlayerPed *player = FindPlayerPed();
	if (!player)
		return;

	CWanted *wanted = player->m_pWanted;
	int ourCopId = 0;
	bool foundMyself = false;
	int biggestCopId = 0;
	if (!m_bIsInPursuit)
		return;

	m_bIsInPursuit = false;
	for (int i = 0; i < Max(wanted->m_MaxCops, wanted->m_CurrentCops); ++i)  {
		if (!foundMyself && wanted->m_pCops[i] == this) {
			wanted->m_pCops[i] = nil;
			--wanted->m_CurrentCops;
			foundMyself = true;
			ourCopId = i;
			biggestCopId = i;
		} else {
			if (wanted->m_pCops[i])
				biggestCopId = i;
		}
	}
	if (foundMyself && biggestCopId > ourCopId) {
		wanted->m_pCops[ourCopId] = wanted->m_pCops[biggestCopId];
		wanted->m_pCops[biggestCopId] = nil;
	}
	m_objective = OBJECTIVE_NONE;
	m_prevObjective = OBJECTIVE_NONE;
	m_nLastPedState = PED_NONE;
	bIsRunning = false;
	bNotAllowedToDuck = false;
	bKindaStayInSamePlace = false;
	m_bStopAndShootDisabledZone = false;
	m_bDragsPlayerFromCar = false;
	m_bZoneDisabled = false;
	ClearObjective();
	if (IsPedInControl()) {
		if (!m_pMyVehicle || wanted->GetWantedLevel() != 0)  {
			if (m_pMyVehicle && (m_pMyVehicle->GetPosition() - GetPosition()).MagnitudeSqr() < sq(5.0f)) {
				m_nLastPedState = PED_IDLE;
				SetSeek((CEntity*)m_pMyVehicle, 2.5f);
			} else {
				m_nLastPedState = PED_WANDER_PATH;
				SetFindPathAndFlee(FindPlayerPed()->GetPosition(), 10000, true);
			}
		} else {
			SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, m_pMyVehicle);
		}
	}
}

// TODO: I don't know why they needed that parameter.
void
CCopPed::SetPursuit(bool ignoreCopLimit)
{
	if (CTimer::GetTimeInMilliseconds() < field_61C)
		return;

	CWanted *wanted = FindPlayerPed()->m_pWanted;
	if (m_bIsInPursuit || !IsPedInControl())
		return;

	if (wanted->m_CurrentCops < wanted->m_MaxCops || ignoreCopLimit) {
		for (int i = 0; i < wanted->m_MaxCops; ++i) {
			if (!wanted->m_pCops[i]) {
				m_bIsInPursuit = true;
				++wanted->m_CurrentCops;
				wanted->m_pCops[i] = this;
				break;
			}
		}
		if (m_bIsInPursuit) {
			ClearObjective();
			m_prevObjective = OBJECTIVE_NONE;
			SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, FindPlayerPed());
			SetObjectiveTimer(0);
			bNotAllowedToDuck = true;
			bIsRunning = true;
			m_bStopAndShootDisabledZone = false;
		}
	}
}

void
CCopPed::ArrestPlayer(void)
{
	m_pVehicleAnim = nil;
	CPed *suspect = (CPed*)m_pSeekTarget;
	if (suspect) {
		if (suspect->CanSetPedState())
			suspect->SetPedState(PED_ARRESTED);

		if (suspect->bInVehicle && m_pMyVehicle && suspect->m_pMyVehicle == m_pMyVehicle) {

			// BUG? I will never understand why they used LINE_UP_TO_CAR_2...
			LineUpPedWithCar(LINE_UP_TO_CAR_2);
		}

		if (suspect && (suspect->m_nPedState == PED_ARRESTED || suspect->DyingOrDead() || suspect->EnteringCar())) {

			CAnimBlendAssociation *arrestAssoc = RpAnimBlendClumpGetAssociation(GetClump(), ANIM_STD_ARREST);
			if (!arrestAssoc || arrestAssoc->blendDelta < 0.0f)
				CAnimManager::BlendAnimation(GetClump(), ASSOCGRP_STD, ANIM_STD_ARREST, 4.0f);

			CVector suspMidPos;
			suspect->m_pedIK.GetComponentPosition(suspMidPos, PED_MID);
			m_fRotationDest = CGeneral::GetRadianAngleBetweenPoints(suspMidPos.x, suspMidPos.y,
				GetPosition().x, GetPosition().y);

			m_fRotationCur = m_fRotationDest;
			SetOrientation(0.0f, 0.0f, m_fRotationCur);
		} else {
			ClearPursuit();
		}
	} else {
		ClearPursuit();
	}
}

void
CCopPed::ScanForCrimes(void)
{
	CVehicle *playerVeh = FindPlayerVehicle();

	// Look for car alarms
	if (playerVeh && playerVeh->IsCar()) {
		if (playerVeh->IsAlarmOn()) {
			if ((playerVeh->GetPosition() - GetPosition()).MagnitudeSqr() < sq(20.0f))
				FindPlayerPed()->SetWantedLevelNoDrop(1);
		}
	}

	// Look for stolen cop cars
	if (!m_bIsInPursuit) {
		CPlayerPed *player = FindPlayerPed();
		if ((m_objective == OBJECTIVE_ENTER_CAR_AS_DRIVER || m_objective == OBJECTIVE_ENTER_CAR_AS_PASSENGER)
			&& player->m_pWanted->GetWantedLevel() == 0) {

			if (player->m_pMyVehicle
#ifdef FIX_BUGS
				&& m_pMyVehicle == player->m_pMyVehicle
#endif
				&& player->m_pMyVehicle->bIsLawEnforcer)
				player->SetWantedLevelNoDrop(1);
		}
	}
}

void
CCopPed::CopAI(void)
{
	CWanted *wanted = FindPlayerPed()->m_pWanted;
	int wantedLevel = wanted->GetWantedLevel();
	CPhysical *playerOrHisVeh = FindPlayerVehicle() ? (CPhysical*)FindPlayerVehicle() : (CPhysical*)FindPlayerPed();

	if (wanted->m_bIgnoredByEveryone || wanted->m_bIgnoredByCops) {
		if (m_nPedState != PED_ARREST_PLAYER)
			ClearPursuit();

		return;
	}
	if (CCullZones::NoPolice() && m_bIsInPursuit && !m_bIsDisabledCop) {
		if (bHitSomethingLastFrame) {
			m_bZoneDisabled = true;
			m_bIsDisabledCop = true;
			bKindaStayInSamePlace = true;
			bIsRunning = false;
			bNotAllowedToDuck = false;
			bCrouchWhenShooting = false;
			SetIdle();
			ClearObjective();
			ClearPursuit();
			m_prevObjective = OBJECTIVE_NONE;
			m_nLastPedState = PED_NONE;
			SetAttackTimer(0);

			// Safe distance for disabled zone? Or to just make game easier?
			if (m_fDistanceToTarget > 15.0f)
				m_bStopAndShootDisabledZone = true;
		}
	} else if (m_bZoneDisabled && !CCullZones::NoPolice()) {
		m_bZoneDisabled = false;
		m_bIsDisabledCop = false;
		m_bStopAndShootDisabledZone = false;
		bKindaStayInSamePlace = false;
		bCrouchWhenShooting = false;
		bDuckAndCover = false;
		ClearPursuit();
	}
#ifdef IMPROVED_TECH_PART // wanted system
	if (wantedLevel > 0 && !wanted->IsPlayerLost()) {
#else
	if (wantedLevel > 0) {
#endif
		if (!m_bIsDisabledCop) {
#ifdef IMPROVED_TECH_PART // wanted system
			if (!m_bUsedTeargas && m_nCopType == COP_SWAT) {
				if (!FindPlayerVehicle() && FindPlayerSpeed().IsZero() && (CGeneral::GetRandomNumber() & 31) == 0 && bIsStanding) {
					float dist = Distance2D(GetPosition(), FindPlayerCoors());
					if (!wanted->IsPlayerHides() && dist > 30.0f && dist < 50.0f && CWorld::GetIsLineOfSightClear(GetPosition(), FindPlayerCoors(), true, false, false, false, false, false)) {
						SetCurrentWeapon(WEAPONTYPE_TEARGAS);
						m_bThrowingTeargas = true;
						m_bUsedTeargas = true;
					}
				}
			}

			if (m_bThrowingTeargas) {
				eWeaponType weapon = GetWeapon()->m_eWeaponType;
				CWeaponInfo::GetWeaponInfo(weapon)->m_nWeaponSlot == WEAPONSLOT_PROJECTILE ? SetAttack(nil) : m_bThrowingTeargas = false;
			}
#endif

			// Turn and shoot the player's vehicle, if possible
			if (!m_bIsInPursuit && !GetWeapon()->IsTypeMelee() && FindPlayerVehicle() && m_fDistanceToTarget < CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_fRange) {
				if (FindPlayerVehicle()->m_vecMoveSpeed.Magnitude2D() > 0.1f) {
					CVector2D distToVeh = GetPosition() - FindPlayerVehicle()->GetPosition();
					distToVeh.Normalise();
					CVector2D vehSpeed = FindPlayerVehicle()->m_vecMoveSpeed;
					vehSpeed.Normalise();

					if (DotProduct2D(distToVeh, vehSpeed) > 0.8f) {
						SetLookFlag(playerOrHisVeh, true);
						SetMoveState(PEDMOVE_STILL);
						if (TurnBody()) {
							SetAttack(FindPlayerVehicle());
							SetShootTimer(CGeneral::GetRandomNumberInRange(500.0f, 1000.0f));
							SetAttackTimer(CGeneral::GetRandomNumberInRange(200.0f, 300.0f));
						}
					} else if (m_nPedState == PED_ATTACK)
						RestorePreviousState();
				}
			}

			if (!m_bIsInPursuit || wanted->m_CurrentCops > wanted->m_MaxCops) {
				CCopPed *copFarthestToTarget = nil;
				float copFarthestToTargetDist = m_fDistanceToTarget;

				int oldCopNum = wanted->m_CurrentCops;
				int maxCops = wanted->m_MaxCops;
				
				for (int i = 0; i < Max(maxCops, oldCopNum); i++) {
					CCopPed *cop = wanted->m_pCops[i];
					if (cop && cop->m_fDistanceToTarget > copFarthestToTargetDist) {
						copFarthestToTargetDist = cop->m_fDistanceToTarget;
						copFarthestToTarget = wanted->m_pCops[i];
					}
				}

				if (m_bIsInPursuit) {
					if (copFarthestToTarget && oldCopNum > maxCops) {
						if (copFarthestToTarget == this && m_fDistanceToTarget > 10.0f) {
							ClearPursuit();
						} else if(copFarthestToTargetDist > 10.0f)
							copFarthestToTarget->ClearPursuit();
					}
				} else {
					if (oldCopNum < maxCops) {
						SetPursuit(true);
					} else {
						if (m_fDistanceToTarget <= 10.0f || copFarthestToTarget && m_fDistanceToTarget < copFarthestToTargetDist) {
							if (copFarthestToTarget && copFarthestToTargetDist > 10.0f)
								copFarthestToTarget->ClearPursuit();

							SetPursuit(true);
						}
					}
				}
			} else
				SetPursuit(false);

			if (!m_bIsInPursuit)
				return;

#ifdef IMPROVED_TECH_PART // wanted system
			if (wantedLevel > 1 && (GetWeapon()->m_eWeaponType == WEAPONTYPE_UNARMED || GetWeapon()->m_eWeaponType == WEAPONTYPE_NIGHTSTICK))
#else
			if (wantedLevel > 1 && GetWeapon()->m_eWeaponType == WEAPONTYPE_UNARMED)
#endif
				SetCurrentWeapon(WEAPONTYPE_COLT45);
			else if (wantedLevel == 1 && GetWeapon()->m_eWeaponType == WEAPONTYPE_UNARMED && !FindPlayerPed()->m_pCurrentPhysSurface) {
				// i.e. if player is on top of car, cop will still use colt45.
				SetCurrentWeapon(GetWeaponSlot(WEAPONTYPE_NIGHTSTICK) >= 0 ? WEAPONTYPE_NIGHTSTICK : WEAPONTYPE_UNARMED);
			}

			if (m_bBeatingSuspect && GetWeapon()->m_eWeaponType == WEAPONTYPE_NIGHTSTICK)
				Say(SOUND_PED_PULLOUTWEAPON);

			if (FindPlayerVehicle()) {
				if (m_bBeatingSuspect) {
					--wanted->m_CopsBeatingSuspect;
					m_bBeatingSuspect = false;
				}
#ifdef IMPROVED_TECH_PART // wanted system
				if (m_fDistanceToTarget * FindPlayerSpeed().Magnitude() > 40.0f && !FindPlayerPed()->m_pWanted->IsPlayerHides())
#else
				if (m_fDistanceToTarget * FindPlayerSpeed().Magnitude() > 4.0f)
#endif
					ClearPursuit();
			}
			return;
		}
#ifdef IMPROVED_TECH_PART // AI
		if (m_nCopType == COP_STREET)
			SetCurrentWeapon(WEAPONTYPE_COLT45);
#else
		SetCurrentWeapon(WEAPONTYPE_COLT45);
#endif
		CWeaponInfo *weaponInfo = CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType);
		float weaponRange = weaponInfo->m_fRange;
		SetLookFlag(playerOrHisVeh, true);
		TurnBody();
		if (!bIsDucking || bCrouchWhenShooting && GetCrouchFireAnim(weaponInfo)) {
			if (m_attackTimer >= CTimer::GetTimeInMilliseconds()) {
				if (m_nPedState != PED_ATTACK && m_nPedState != PED_FIGHT && !m_bZoneDisabled) {
					CVector targetDist = playerOrHisVeh->GetPosition() - GetPosition();
					if (m_fDistanceToTarget > 30.0f) {
						if (bIsDucking)
							ClearDuck();

						// Target is coming onto us
						if (DotProduct(playerOrHisVeh->m_vecMoveSpeed, targetDist) > 0.0f) {
							m_bIsDisabledCop = false;
							bKindaStayInSamePlace = false;
							bNotAllowedToDuck = false;
							bDuckAndCover = false;
							SetPursuit(false);
							SetObjective(OBJECTIVE_KILL_CHAR_ANY_MEANS, FindPlayerPed());
						}
					} else if (m_fDistanceToTarget < 5.0f
							&& (!FindPlayerVehicle() || FindPlayerVehicle()->m_vecMoveSpeed.MagnitudeSqr() < sq(1.f/200.f))) {
						m_bIsDisabledCop = false;
						bKindaStayInSamePlace = false;
						bNotAllowedToDuck = false;
						bDuckAndCover = false;
					} else {
						float dotProd;
						if (m_nRoadblockVeh) {
							dotProd = DotProduct2D(playerOrHisVeh->GetPosition() - m_nRoadblockVeh->GetPosition(), GetPosition() - m_nRoadblockVeh->GetPosition());
						} else
							dotProd = -1.0f;

						if(dotProd < 0.0f) {
							if (bIsDucking)
								ClearDuck();
							m_bIsDisabledCop = false;
							bKindaStayInSamePlace = false;
							bNotAllowedToDuck = false;
							bCrouchWhenShooting = false;
							bDuckAndCover = false;
							SetPursuit(false);
						} else {
							bIsPointingGunAt = true;
						}
					}
				}
			} else {
				if (m_fDistanceToTarget < weaponRange) {
					CVector gunPos = weaponInfo->m_vecFireOffset;
					TransformToNode(gunPos, PED_HANDR);

					CColPoint foundCol;
					CEntity *foundEnt;
					if (!CWorld::ProcessLineOfSight(gunPos, playerOrHisVeh->GetPosition(), foundCol, foundEnt,
						false, true, false, false, true, false, false)
						|| foundEnt && foundEnt == playerOrHisVeh) {

						SetWeaponLockOnTarget(playerOrHisVeh);
						SetAttack(playerOrHisVeh);
						SetShootTimer(CGeneral::GetRandomNumberInRange(500, 1000));
					}
					SetAttackTimer(CGeneral::GetRandomNumberInRange(200, 300));
				}
				SetMoveState(PEDMOVE_STILL);
			}
		}
	} else {
		if (!m_bIsDisabledCop || m_bZoneDisabled) {
			if (m_nPedState != PED_AIM_GUN) {
				if (m_bIsInPursuit)
					ClearPursuit();

#ifdef WANTED_PATHS
				if (IsPedInControl() && !m_bLookingForPlayer) {
#else
				if (IsPedInControl()) {
#endif
					// Entering the vehicle
					if (m_pMyVehicle && !bInVehicle) {
						if (m_pMyVehicle->IsLawEnforcementVehicle()) {
							if (m_pMyVehicle->pDriver) {
								if (m_pMyVehicle->pDriver->m_nPedType == PEDTYPE_COP) {
									if (m_objective != OBJECTIVE_ENTER_CAR_AS_PASSENGER)
										SetObjective(OBJECTIVE_ENTER_CAR_AS_PASSENGER, m_pMyVehicle);
								} else if (m_pMyVehicle->pDriver->IsPlayer()) {
									FindPlayerPed()->SetWantedLevelNoDrop(1);
								}
							} else if (m_objective != OBJECTIVE_ENTER_CAR_AS_DRIVER) {
								SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, m_pMyVehicle);
							}
						} else {
							m_pMyVehicle = nil;
							ClearObjective();
							SetWanderPath(CGeneral::GetRandomNumber() & 7);
						}
					} else {
						if (m_objective != OBJECTIVE_KILL_CHAR_ON_FOOT && m_objective != OBJECTIVE_HASSLE_CHAR && CharCreatedBy == RANDOM_CHAR) {
							for (int i = 0; i < m_numNearPeds; i++) {
								CPed *nearPed = m_nearPeds[i];
								if (nearPed->CharCreatedBy == RANDOM_CHAR) {
									if ((nearPed->m_nPedType == PEDTYPE_CRIMINAL || nearPed->IsGangMember())
										&& nearPed->IsPedInControl()) {

										bool anotherCopChasesHim = false;
										if (nearPed->m_nPedState == PED_FLEE_ENTITY) {
											if (nearPed->m_fleeFrom && nearPed->m_fleeFrom->IsPed() &&
												((CPed*)nearPed->m_fleeFrom)->m_nPedType == PEDTYPE_COP) {
												anotherCopChasesHim = true;
											}
										}
										if (!anotherCopChasesHim) {
											SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, nearPed);
											nearPed->SetObjective(OBJECTIVE_FLEE_CHAR_ON_FOOT_TILL_SAFE, this);
											nearPed->bBeingChasedByPolice = true;
											return;
										}
									} else {
										if (nearPed->m_nPedType != PEDTYPE_COP && !nearPed->IsPlayer()
											&& nearPed->IsPedInControl() && m_nHassleTimer < CTimer::GetTimeInMilliseconds()) {

											if (nearPed->m_objective == OBJECTIVE_NONE && nearPed->m_nPedState == PED_WANDER_PATH
												&& !nearPed->m_pLookTarget && nearPed->m_lookTimer < CTimer::GetTimeInMilliseconds()) {

												if ((GetPosition() - nearPed->GetPosition()).MagnitudeSqr() < sq(5.0f)) {

													if (CWorld::GetIsLineOfSightClear(GetPosition(), nearPed->GetPosition(),
														true, false, false, false, false, false, false)) {
														Say(SOUND_PED_COP_ASK_FOR_ID);
														SetObjective(OBJECTIVE_HASSLE_CHAR, nearPed);
														nearPed->SetObjective(OBJECTIVE_WAIT_ON_FOOT_FOR_COP, this);
														m_nHassleTimer = CTimer::GetTimeInMilliseconds() + 100000;
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		} else {
			if (m_bIsInPursuit && m_nPedState != PED_AIM_GUN)
				ClearPursuit();

			m_bIsDisabledCop = false;
			bKindaStayInSamePlace = false;
			bNotAllowedToDuck = false;
			bCrouchWhenShooting = false;
			bDuckAndCover = false;
			if (bIsDucking)
				ClearDuck();
			if (m_pMyVehicle)
				SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, m_pMyVehicle);
		}
	}
}

void
CCopPed::ProcessControl(void)
{
	if (m_nCopType == COP_HELI_SWAT)
		ProcessHeliSwat();

	CPed::ProcessControl();

	if (m_bThrowsSpikeTrap) {
		if (CGame::currArea != AREA_MALL)
			ProcessStingerCop();
		return;
	}

	if (m_pStinger && m_pStinger->bIsDeployed && m_pStinger->m_nSpikeState == STINGERSTATE_DEPLOYED && CGame::currArea != AREA_MALL)
		m_pStinger->Process();

	if (bWasPostponed)
		return;

	if (m_nPedState == PED_DEAD) {
		ClearPursuit();
		m_objective = OBJECTIVE_NONE;
		return;
	}
	if (m_nPedState == PED_DIE)
		return;

	if (m_nPedState == PED_ARREST_PLAYER) {
		ArrestPlayer();
		return;
	}
	GetWeapon()->Update(m_audioEntityId, nil);
	if (m_moved.Magnitude() > 0.0f)
		Avoid();

	CPhysical *playerOrHisVeh = FindPlayerVehicle() ? (CPhysical*)FindPlayerVehicle() : (CPhysical*)FindPlayerPed();
	CPlayerPed *player = FindPlayerPed();

	m_fDistanceToTarget = (playerOrHisVeh->GetPosition() - GetPosition()).Magnitude();
	if (player->m_nPedState == PED_ARRESTED || player->DyingOrDead()) {
		if (m_fDistanceToTarget < 5.0f) {
			SetArrestPlayer(player);
			return;
		}
		if (IsPedInControl())
			SetIdle();
	}

	if (m_bIsInPursuit) {
		if (player->m_nPedState != PED_ARRESTED && !player->DyingOrDead()) {
			if (player->m_pWanted->m_CurrentCops == 1) {
				Say(SOUND_PED_COP_ALONE);
			} else {
				int numCopsNear = 0;
				for (int i = 0; i < player->m_numNearPeds; ++i) {
					CPed *nearPed = player->m_nearPeds[i];
					if (nearPed->m_nPedType == PEDTYPE_COP && nearPed->m_nPedState != PED_DEAD)
						++numCopsNear;
				}
				if (numCopsNear <= 3) {
					Say(SOUND_PED_COP_LITTLECOPSAROUND);
					if (!player->bInVehicle) {
						CVector distToPlayer = player->GetPosition() - GetPosition();
						if (distToPlayer.MagnitudeSqr() < sq(20.0f)) {
#ifdef IMPROVED_TECH_PART // wanted system
							if (!player->m_pWanted->IsPlayerHides()) {
								player->Say(SOUND_PED_PLAYER_FARFROMCOPS);
								if (player->m_nPedState != PED_ATTACK && player->m_nPedState != PED_AIM_GUN) {
									player->SetLookFlag(this, false);
									player->SetLookTimer(1000);
								}
							}
#else
							player->Say(SOUND_PED_PLAYER_FARFROMCOPS);
							if (player->m_nPedState != PED_ATTACK && player->m_nPedState != PED_AIM_GUN) {
								player->SetLookFlag(this, false);
								player->SetLookTimer(1000);
							}
#endif
						}
					}
				} else if ((CGeneral::GetRandomNumber() % 16) == 1) {
					Say(SOUND_PED_COP_MANYCOPSAROUND);
				}
			}
		}
	}

#ifdef IMPROVED_TECH_PART // wanted system
	if (IsPedInControl() || FindPlayerPed()->m_pWanted->IsPlayerLost()) {
#else
	if (IsPedInControl()) {
#endif
		CopAI();
		/* switch (m_nCopType)
		{
			case COP_FBI:
				CopAI();
				break;
			case COP_SWAT:
				CopAI();
				break;
			case COP_ARMY:
				CopAI();
				break;
			default:
				CopAI();
				break;
		} */
	} else if (InVehicle()) {
		if (m_pMyVehicle->pDriver == this && m_pMyVehicle->AutoPilot.m_nCarMission == MISSION_NONE &&
			CanPedDriveOff() && m_pMyVehicle->VehicleCreatedBy != MISSION_VEHICLE) {

			CCarCtrl::JoinCarWithRoadSystem(m_pMyVehicle);
			m_pMyVehicle->AutoPilot.m_nCarMission = MISSION_CRUISE;
			m_pMyVehicle->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_STOP_FOR_CARS;
			m_pMyVehicle->AutoPilot.m_nCruiseSpeed = 17;
		}
	}
	if (IsPedInControl() || m_nPedState == PED_DRIVING)
		ScanForCrimes();

	// They may have used goto to jump here in case of PED_ATTACK.
	if (m_nPedState == PED_IDLE || m_nPedState == PED_ATTACK) {
		if (m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT &&
			player && player->EnteringCar() && m_fDistanceToTarget < 1.3f) {
			SetArrestPlayer(player);
		}
	} else {
		if (m_nPedState == PED_SEEK_POS) {
			if (player->m_nPedState == PED_ARRESTED) {
				SetIdle();
				SetLookFlag(player, false);
				SetLookTimer(1000);
				RestorePreviousObjective();
			} else {
				if (player->m_pMyVehicle && player->m_pMyVehicle->m_nNumGettingIn != 0) {
					m_distanceToCountSeekDone = 1.3f;
				}

				if (!bDuckAndCover && Seek()) {
					CVehicle *playerVeh = FindPlayerVehicle();
					if (!playerVeh && player && player->EnteringCar()) {
						SetArrestPlayer(player);
					} else if (1.5f + GetPosition().z <= m_vecSeekPos.z || GetPosition().z - 0.3f >= m_vecSeekPos.z) {
						SetMoveState(PEDMOVE_STILL);
					} else if (playerVeh && playerVeh->CanPedEnterCar() && playerVeh->m_nNumGettingIn == 0) {
						SetCarJack(playerVeh);
					}
				}
			}
		} else if (m_nPedState == PED_SEEK_ENTITY) {
			if (!m_pSeekTarget) {
				RestorePreviousState();
			} else {
#ifdef IMPROVED_TECH_PART // wanted system
				if (m_pSeekTarget == FindPlayerPed()) {
					CPlayerPed* playerPed = FindPlayerPed();
					if (!playerPed->m_pWanted->IsPlayerHides() && !playerPed->m_pWanted->IsPlayerLost() && !m_bMovesToLastPlayerPosition) {
						bool isWaterAroundCop = CheckIfPositionIsUnderWater(GetPosition() + GetForward() * 3.5f, false) ||
							CheckIfPositionIsUnderWater(GetPosition() + GetRight(), false) ||
							CheckIfPositionIsUnderWater(GetPosition() - GetRight(), false);

						if ((playerPed->bIsSwimming && isWaterAroundCop) ||
							(!playerPed->bIsSwimming && CheckIfPositionIsUnderWater(m_pSeekTarget->GetPosition(), true)))

							m_vecSeekPos = GetPosition();
						else
							m_vecSeekPos = m_pSeekTarget->GetPosition();
#ifdef DEBUG
						RwV3d copHeadPos;
						copHeadPos.x = 0.0f;
						copHeadPos.y = 0.0f;
						copHeadPos.z = 0.0f;
						m_pedIK.GetComponentPosition(copHeadPos, PED_HEAD);

						RwV3d playerHeadPos;
						playerHeadPos.x = 0.0f;
						playerHeadPos.y = 0.0f;
						playerHeadPos.z = 0.0f;
						FindPlayerPed()->m_pedIK.GetComponentPosition(playerHeadPos, PED_HEAD);

						CDebug::AddLine(copHeadPos, playerHeadPos, 0xFFFFFFFF, 0xFFFFFFFF);
#endif
					} else {
						ProcessSearchPlayer(playerPed);
#ifdef DEBUG
						RwV3d copHeadPos;
						copHeadPos.x = 0.0f;
						copHeadPos.y = 0.0f;
						copHeadPos.z = 0.0f;
						m_pedIK.GetComponentPosition(copHeadPos, PED_HEAD);

						CDebug::AddLine(copHeadPos, m_vecSeekPos, 0xff0000, 0xff0000);
#endif
					}
				} else {
					m_vecSeekPos = m_pSeekTarget->GetPosition();
				}
#else
				m_vecSeekPos = m_pSeekTarget->GetPosition();
#endif
				if (Seek()) {
					if (m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT && m_fDistanceToTarget < 2.5f && player) {
						if (player->m_nPedState == PED_ARRESTED || player->m_nPedState == PED_ENTER_CAR ||
							(player->m_nPedState == PED_CARJACK && m_fDistanceToTarget < 1.3f)) {
							SetArrestPlayer(player);
						} else
							RestorePreviousState();
					} else {
						RestorePreviousState();
					}

				}
			}
		}
	}

	if (m_pPointGunAt)
		Say(SOUND_PED_COP_TARGETING);

	if (m_bStopAndShootDisabledZone) {
		bool dontShoot = false;
		if (GetIsOnScreen()) {
			if (((CTimer::GetFrameCounter() + m_randomSeed) & 0x1F) == 17) {
				CEntity* foundBuilding = nil;
				CColPoint foundCol;
				CVector lookPos = GetPosition() + CVector(0.0f, 0.0f, 0.7f);
				CVector camPos = TheCamera.GetGameCamPosition();
				CWorld::ProcessLineOfSight(camPos, lookPos, foundCol, foundBuilding,
					true, false, false, false, false, false, false);

				// He's at least 15.0 far, in disabled zone, collided into somewhere (that's why m_bStopAndShootDisabledZone set),
				// and now has building on front of him. He's stupid, we don't need him.
				if (foundBuilding) {
					FlagToDestroyWhenNextProcessed();
					dontShoot = true;
				}
			}
		} else {
			FlagToDestroyWhenNextProcessed();
			dontShoot = true;
		}

		if (!dontShoot) {
			bStopAndShoot = true;
			bKindaStayInSamePlace = true;
			bIsPointingGunAt = true;
			SetAttack(m_pedInObjective);
		}
	}

	if (field_624 >= 2 && m_objective == OBJECTIVE_KILL_CHAR_ON_FOOT) {
		CVector centre = GetPosition() + CVector(0.f, 0.f, 0.65f);
		if (CWorld::TestSphereAgainstWorld(centre, 0.35f, this, true, false, false, false, false, false)) {
			field_624 = 0;
			m_bStopAndShootDisabledZone = true;
			ClearPursuit();
			SetObjective(OBJECTIVE_NONE);
			SetWanderPath(CGeneral::GetRandomNumberInRange(0,8));
			field_61C = CTimer::GetTimeInMilliseconds() + 30000;
		} else {
			field_624 = 0;
			if (GetWeapon()->IsTypeMelee()) {
				// TODO(Miami): enum
				for (int i = 3; i < 7; i++) {
					if (HasWeaponSlot(i)) {
						SetCurrentWeapon(i);
						break;
					}
				}
				SetMoveState(PEDMOVE_STILL);
				bStopAndShoot = true;
			}
		}
	} else if (CTimer::GetTimeStep() / 100.f <= m_fDistanceTravelled)
		field_624 = 0;
}

void
CCopPed::ProcessHeliSwat(void)
{
	CVector bestPos = GetPosition();
	SetPedState(PED_ABSEIL);
	CPedPlacement::FindZCoorForPed(&bestPos);
	if (GetPosition().z - 2.0f >= bestPos.z && m_pRopeEntity) {
		m_fAbseilPos += 0.003f * CTimer::GetTimeStep();
		m_vecMoveSpeed.z = -0.03f;
		m_vecTurnSpeed = CVector(0.f, 0.f, (m_randomSeed % 32) * 0.003f - 0.05f);
		CPhysical::ApplyTurnSpeed();
		GetMatrix().Reorthogonalise();
		CVector posOnRope;

		if (CRopes::FindCoorsAlongRope(m_nRopeID, m_fAbseilPos, &posOnRope)) {
			SetPosition(posOnRope);
		} else {
			bUsesCollision = true;
			m_vecMoveSpeed = CVector(0.f, 0.f, 0.f);
			SetPedState(PED_IDLE);
			m_nCopType = COP_SWAT;
			SetInTheAir();
			bKnockedUpIntoAir = true;
		}
		Say(SOUND_PED_COP_HELIPILOTPHRASE);
	} else {
		bUsesCollision = true;
		m_vecMoveSpeed = CVector(0.f, 0.f, 0.f);
		SetPedState(PED_IDLE);
		m_nCopType = COP_SWAT;
		SetInTheAir();
		bKnockedUpIntoAir = true;
	}
}

void
CCopPed::ProcessStingerCop(void)
{
	if (m_pStinger->bIsDeployed || FindPlayerVehicle() && (FindPlayerVehicle()->IsCar() || FindPlayerVehicle()->IsBike())) {
		if (m_pStinger->bIsDeployed) {
			m_pStinger->Process();
		} else {
#ifdef IMPROVED_TECH_PART // wanted system
			if (FindPlayerPed()->m_pWanted->IsPlayerHides())
				return;
#endif
			CVector2D vehDist = GetPosition() - FindPlayerVehicle()->GetPosition();
			CVector2D dirVehGoing = FindPlayerVehicle()->m_vecMoveSpeed;
			if (vehDist.MagnitudeSqr() < sq(30.0f)) {
				if (dirVehGoing.MagnitudeSqr() > 0.0f) {
					vehDist.Normalise();
					dirVehGoing.Normalise();
					if (DotProduct2D(vehDist, dirVehGoing) > 0.8f) {
						float angle = (CrossProduct2D(vehDist, dirVehGoing - vehDist) < 0.0f ?
							FindPlayerVehicle()->GetForward().Heading() - HALFPI :
							HALFPI + FindPlayerVehicle()->GetForward().Heading());

						SetHeading(angle);
						m_fRotationCur = angle;
						m_fRotationDest = angle;
						m_pStinger->Deploy(this);
					}
				}
			}
		}
	} else {
		ClearPursuit();
	}
}

#ifdef IMPROVED_TECH_PART // wanted system
void CCopPed::ProcessSearchPlayer(CPlayerPed* playerPed)
{
	if (playerPed->m_pWanted->IsPlayerHides() && !m_bLookingForPlayer) {
		if (CheckIfPositionIsUnderWater(m_vecLastPosPlayerBeforeHiding, true)) {
			m_bLookingForPlayer = true;
			m_bMovesToLastPlayerPosition = false;
			playerPed->m_pWanted->m_bSearchPlayerRandomly = true;

			FindPotentialPlayerPos();
		} else {
			m_vecSeekPos = m_vecLastPosPlayerBeforeHiding;
			m_bLookingForPlayer = false;
			m_bMovesToLastPlayerPosition = true;
		}
	}

	if (m_bMovesToLastPlayerPosition && !m_bLookingForPlayer && Distance2D(GetPosition(), m_vecLastPosPlayerBeforeHiding) < 2.0f) {
		if (playerPed->m_pWanted->IsPlayerHides()) {
			m_bLookingForPlayer = true;
			FindPotentialPlayerPos();
		} else {
			m_bLookingForPlayer = false;
		}
		m_bMovesToLastPlayerPosition = false;
	}

	if (playerPed->m_pWanted->IsPlayerHides() && m_bLookingForPlayer) {
		m_vecSeekPos = m_vecPotentialPlayerSearchCoordinates;
		
		if (Distance2D(GetPosition(), m_vecPotentialPlayerSearchCoordinates) < 3.0f) {
			FindPotentialPlayerPos();
		}
	}
}

void CCopPed::FindPotentialPlayerPos(void)
{
	CVector randomVector = CVector(CGeneral::GetRandomNumberInRange(-10.0f, 10.0f), CGeneral::GetRandomNumberInRange(-10.0f, 10.0f), 0.0f);
	CVector randomDirection = GetPosition() - (GetPosition() + randomVector);
	randomDirection.Normalise();
	float randomMultiple = CGeneral::GetRandomNumberInRange(10.0f, 40.0f);

	CEntity* hitEntity;
	CColPoint hitPoint;

	if (FindPlayerPed()->m_pWanted->m_bSearchPlayerRandomly) {
		// Find random position for player search
		if (CWorld::ProcessLineOfSight(GetPosition(), GetPosition() + randomDirection * randomMultiple, hitPoint, hitEntity, true, false, false, true, false, true))
			m_vecPotentialPlayerSearchCoordinates = hitPoint.point;
		else
			m_vecPotentialPlayerSearchCoordinates = GetPosition() + randomDirection * randomMultiple;

		m_vecPotentialPlayerSearchCoordinates.z = GetPosition().z;
	} else {
		// More accurate player search
		for (int i = 1; i < 150; i++) {
			if (!CWorld::ProcessLineOfSight(FindPlayerCoors(), FindPlayerCoors() + randomDirection * randomMultiple, hitPoint, hitEntity, true, false, false, true, false, true)) {
				CVector rayDirection = FindPlayerCoors() + randomDirection * randomMultiple;
				rayDirection.z = GetPosition().z;
				if (CWorld::ProcessLineOfSight(GetPosition(), rayDirection, hitPoint, hitEntity, true, false, false, true, false, true)) {
					if (Distance2D(GetPosition(), hitPoint.point) < 1.5f) {
						CVector wallDirection = CrossProduct(hitPoint.normal, CVector(0.0f, 0.0f, 1.0f));
						if (CWorld::ProcessLineOfSight(GetPosition(), GetPosition() + wallDirection * randomMultiple, hitPoint, hitEntity, true, true, false, true, false, true))
							m_vecPotentialPlayerSearchCoordinates = hitPoint.point;
						else
							m_vecPotentialPlayerSearchCoordinates = rayDirection;
					} else {
						m_vecPotentialPlayerSearchCoordinates = hitPoint.point;
					}
				} else {
					m_vecPotentialPlayerSearchCoordinates = rayDirection;
				}

				return;
			} else {
				randomVector = CVector(CGeneral::GetRandomNumberInRange(-10.0f, 10.0f), CGeneral::GetRandomNumberInRange(-10.0f, 10.0f), 0.0f);
				randomDirection = GetPosition() - (GetPosition() + randomVector);
				randomDirection.Normalise();
				randomMultiple = CGeneral::GetRandomNumberInRange(10.0f, 40.0f);
			}
		}

		// If raycasts can't find approximate position then use less accurate player search
		if (CWorld::ProcessLineOfSight(FindPlayerCoors(), FindPlayerCoors() + randomDirection * randomMultiple, hitPoint, hitEntity, true, false, false, true, false, true)) {
			CWorld::ProcessLineOfSight(GetPosition(), hitPoint.point, hitPoint, hitEntity, true, false, false, true, false, true);
			m_vecPotentialPlayerSearchCoordinates = hitPoint.point;
		} else {
			// ...else we search randomly
			if (CWorld::ProcessLineOfSight(GetPosition(), FindPlayerCoors() + randomDirection * randomMultiple, hitPoint, hitEntity, true, false, false, true, false, true))
				m_vecPotentialPlayerSearchCoordinates = hitPoint.point;
			else
				m_vecPotentialPlayerSearchCoordinates = FindPlayerCoors() + randomDirection * randomMultiple;
		}
	}

	if (CheckIfPositionIsUnderWater(m_vecPotentialPlayerSearchCoordinates, true))
		m_vecPotentialPlayerSearchCoordinates = GetPosition();
}

bool CCopPed::CheckIfPositionIsUnderWater(CVector checkPos, bool bCheckMiddlePos)
{
	if (bCheckMiddlePos) {
		CVector middlePos = (checkPos - GetPosition()) * 0.5f;
		middlePos = GetPosition() + middlePos;

		float waterLevel1;
		bool isWaterUnderMiddlePos = CWaterLevel::GetWaterLevel(middlePos, &waterLevel1, false) &&
			CWorld::GetIsLineOfSightClear(middlePos, middlePos - CVector(0.0f, 0.0f, waterLevel1), true, false, false, false, false, false);

		float waterLevel2;
		bool isWaterUnderSearchPos = CWaterLevel::GetWaterLevel(checkPos, &waterLevel2, false) &&
			CWorld::GetIsLineOfSightClear(checkPos, checkPos - CVector(0.0f, 0.0f, waterLevel2), true, false, false, false, false, false);

		if (isWaterUnderMiddlePos || isWaterUnderSearchPos) {
			return true;
		}
	} else {
		float waterLevel;
		bool isWaterUnderSearchPos = CWaterLevel::GetWaterLevel(checkPos, &waterLevel, false) &&
			CWorld::GetIsLineOfSightClear(checkPos, checkPos - CVector(0.0f, 0.0f, waterLevel), true, false, false, false, false, false);

		if (isWaterUnderSearchPos)
			return true;
	}

	return false;
}
#endif