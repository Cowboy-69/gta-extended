#include "common.h"

#include "Weapon.h"
#include "AnimBlendAssociation.h"
#include "AudioManager.h"
#include "BulletInfo.h"
#include "Camera.h"
#include "Coronas.h"
#include "DMAudio.h"
#include "Explosion.h"
#include "General.h"
#include "Glass.h"
#include "Heli.h"
#include "ModelIndices.h"
#include "Object.h"
#include "Pad.h"
#include "Particle.h"
#include "Ped.h"
#include "PointLights.h"
#include "Pools.h"
#include "ProjectileInfo.h"
#include "RpAnimBlend.h"
#include "ShotInfo.h"
#include "SpecialFX.h"
#include "Stats.h"
#include "TempColModels.h"
#include "Timer.h"
#include "Automobile.h"
#include "Boat.h"
#include "WaterLevel.h"
#include "WeaponInfo.h"
#include "World.h"
#include "SurfaceTable.h"
#include "Bike.h"
#include "Glass.h"
#include "Sprite.h"
#include "Pickups.h"
#include "SaveBuf.h"
#ifdef IMPROVED_TECH_PART // wanted system
#include "CopPed.h"
#include "Wanted.h"
#endif
#ifdef IMPROVED_MENU_AND_INPUT
#include "Frontend.h"
#endif
#ifdef IMPROVED_TECH_PART // moving Moon
#include "Timecycle.h"
#endif
#ifdef IMPROVED_TECH_PART // blood screen droplets
#include "screendroplets.h"
#endif
#ifdef IMPROVED_TECH_PART // Saving a screenshot after taking a photo
#include "main.h"
#endif
#ifdef EX_IMPROVED_WEAPONS
#include "Streaming.h"
#endif
#include "main.h"

float fReloadAnimSampleFraction[5] = {  0.5f,  0.7f,  0.75f,  0.75f,  0.7f };
float fSeaSparrowAimingAngle = 10.0f;
float fHunterAimingAngle = 30.0f;
float fPlayerAimScaleDist = 5.0f; 
float fPlayerAimScale = 2.5f;

bool CWeapon::bPhotographHasBeenTaken;
#ifdef IMPROVED_TECH_PART // Saving a screenshot after taking a photo
bool CWeapon::bTakePhoto;
#endif

#ifdef SECUROM
int32 sniperPirateCheck = 0x00797743; // 'Cwy\0' ???
#endif

#ifdef FREE_CAM
static bool
Find3rdPersonCamTargetVectorFromCachedVectors(float dist, CVector pos, CVector& source, CVector& target, CVector camSource, CVector camFront, CVector camUp)
{
	if (CPad::GetPad(0)->GetLookBehindForPed()) {
		source = pos;
		target = dist * FindPlayerPed()->GetForward() + source;
		return false;
	} else {
		float angleX = DEGTORAD((CCamera::m_f3rdPersonCHairMultX - 0.5f) * 1.8f * 0.5f * TheCamera.Cams[TheCamera.ActiveCam].FOV * CDraw::GetAspectRatio());
		float angleY = DEGTORAD((0.5f - CCamera::m_f3rdPersonCHairMultY) * 1.8f * 0.5f * TheCamera.Cams[TheCamera.ActiveCam].FOV);
		source = camSource;
		target = camFront;
		target += camUp * Tan(angleY);
		if (RwCameraGetMirror(Scene.camera))
			target += CrossProduct(camFront, camUp) * Tan(-angleX);
		else
			target += CrossProduct(camFront, camUp) * Tan(angleX);
		target.Normalise();
		source += DotProduct(pos - source, target) * target;
		target = dist * target + source;
		return true;
	}
}
#endif

CWeaponInfo *
CWeapon::GetInfo()
{
	CWeaponInfo *info = CWeaponInfo::GetWeaponInfo(m_eWeaponType);
	ASSERT(info!=nil);
	return info;
}

CWeapon::CWeapon(eWeaponType type, int32 ammo)
{
	m_eWeaponType = type;
	m_eWeaponState = WEAPONSTATE_READY;
	m_nAmmoTotal = Min(ammo, 99999);
	m_nAmmoInClip = 0;
	Reload();
	m_nTimer = 0;
	m_bAddRotOffset = false;
}

void
CWeapon::InitialiseWeapons(void)
{
	CWeaponInfo::Initialise();
	CShotInfo::Initialise();
	CExplosion::Initialise();
	CProjectileInfo::Initialise();
	CBulletInfo::Initialise();
	bPhotographHasBeenTaken = false;
}

void
CWeapon::ShutdownWeapons(void)
{
	CWeaponInfo::Shutdown();
	CShotInfo::Shutdown();
	CExplosion::Shutdown();
	CProjectileInfo::Shutdown();
	CBulletInfo::Shutdown();
}

void
CWeapon::UpdateWeapons(void)
{
	CShotInfo::Update();
	CExplosion::Update();
	CProjectileInfo::Update();
	CBulletInfo::Update();

#ifdef IMPROVED_TECH_PART // Saving a screenshot after taking a photo
	if (bTakePhoto) {
		TakeAndSaveScreenshot();
		bTakePhoto = false;
	}
#endif
}


void
CWeapon::Initialise(eWeaponType type, int32 ammo)
{
	m_eWeaponType = type;
	m_eWeaponState = WEAPONSTATE_READY;
	m_nAmmoTotal = Min(ammo, 99999);
	m_nAmmoInClip = 0;
	Reload();
	m_nTimer = 0;
	int32 modelId = CWeaponInfo::GetWeaponInfo(m_eWeaponType)->m_nModelId;
	int32 model2Id = CWeaponInfo::GetWeaponInfo(m_eWeaponType)->m_nModel2Id;
	
	if ( modelId != -1 )
		CModelInfo::GetModelInfo(modelId)->AddRef();
	if ( model2Id != -1 )
		CModelInfo::GetModelInfo(model2Id)->AddRef();

#ifdef EX_IMPROVED_WEAPONS
	CWeaponModelInfo* modelInfo = (CWeaponModelInfo*)CModelInfo::GetModelInfo(modelId);
	if (modelId != -1) {
		int animForWeaponBlock = modelInfo->GetAnimForWeaponFileIndex();
		if (animForWeaponBlock != -1) {
			CStreaming::RequestAnim(animForWeaponBlock, STREAMFLAGS_DEPENDENCY);
			CStreaming::LoadAllRequestedModels(false);
			CAnimManager::AddAnimBlockRef(animForWeaponBlock);
		}
	}
#endif
}

void
CWeapon::Shutdown()
{
	int32 modelId = CWeaponInfo::GetWeaponInfo(m_eWeaponType)->m_nModelId;
	if (modelId != -1)
		CModelInfo::GetModelInfo(modelId)->RemoveRef();

#ifdef EX_IMPROVED_WEAPONS
	CWeaponModelInfo* modelInfo = (CWeaponModelInfo*)CModelInfo::GetModelInfo(modelId);
	if (modelId != -1) {
		int animForWeaponBlock = modelInfo->GetAnimForWeaponFileIndex();
		if (animForWeaponBlock != -1) {
			CAnimManager::RemoveAnimBlockRef(animForWeaponBlock);
		}
	}
#endif

	int32 model2Id = CWeaponInfo::GetWeaponInfo(m_eWeaponType)->m_nModel2Id;
	if (model2Id != -1)
		CModelInfo::GetModelInfo(model2Id)->RemoveRef();

	m_eWeaponType = WEAPONTYPE_UNARMED;
	m_eWeaponState = WEAPONSTATE_READY;
	m_nAmmoInClip = 0;
	m_nAmmoTotal = 0;
	m_nTimer = 0;
}

bool
CWeapon::Fire(CEntity *shooter, CVector *fireSource)
{
	ASSERT(shooter!=nil);

	CVector fireOffset(0.0f, 0.0f, 0.6f);
	CVector *source = fireSource;
#ifdef FIX_BUGS
	static CVector shooterSource;
#else
	CVector shooterSource;
#endif

	if ( !fireSource )
	{
		shooterSource = shooter->GetMatrix() * fireOffset;
		source = &shooterSource;
	}
		
	if ( m_bAddRotOffset )
	{
		float heading = RADTODEG(shooter->GetForward().Heading());
		float angle   = DEGTORAD(heading);
		(*source).x += -Sin(angle) * 0.15f;
		(*source).y +=  Cos(angle) * 0.15f;
	}

	if ( m_eWeaponState != WEAPONSTATE_READY && m_eWeaponState != WEAPONSTATE_FIRING )
		return false;

	bool fired;
	bool addFireRateAsDelay = true;

	if ( GetInfo()->m_eWeaponFire != WEAPON_FIRE_MELEE )
	{
		if (m_nAmmoInClip <= 0) {
			if (m_nAmmoTotal <= 0 || m_eWeaponState == WEAPONSTATE_RELOADING)
				return false;

			Reload();
		}

#ifdef EX_IMPROVED_WEAPONS // Slide animation after a shot
		if (shooter->IsPed()) {
			CPed* ped = (CPed*)shooter;
			if (ped->m_pWeaponModel && GetInfo()->m_animForWeaponToPlay != ASSOCGRP_STD) {
				if (RpAnimBlendClumpGetAssociation(ped->m_pWeaponModel, ANIM_FOR_WEAPON_SLIDE))
					CAnimManager::BlendAnimation(ped->m_pWeaponModel, GetInfo()->m_animForWeaponToPlay, ANIM_FOR_WEAPON_SLIDE, 100.0f);
				else
					CAnimManager::AddAnimation(ped->m_pWeaponModel, GetInfo()->m_animForWeaponToPlay, ANIM_FOR_WEAPON_SLIDE);
			}
		}
#endif

		switch ( m_eWeaponType )
		{
			case WEAPONTYPE_SHOTGUN:
			case WEAPONTYPE_SPAS12_SHOTGUN:
			case WEAPONTYPE_STUBBY_SHOTGUN:
#ifdef EX_WEAPON_SHOTGUN2 // Fire
			case WEAPONTYPE_SHOTGUN2:
#endif
			{
				addFireRateAsDelay = true;
				fired = FireShotgun(shooter, source);

				break;
			}

			case WEAPONTYPE_SNIPERRIFLE:
			case WEAPONTYPE_LASERSCOPE:
			{
				if (shooter == FindPlayerPed())
					fired = FireSniper(shooter);
				else
					fired = FireInstantHit(shooter, source);
				
				break;
			}
			
			case WEAPONTYPE_COLT45:
			case WEAPONTYPE_PYTHON:
			case WEAPONTYPE_UZI:
			case WEAPONTYPE_TEC9:
			case WEAPONTYPE_SILENCED_INGRAM:
			case WEAPONTYPE_MP5:
			case WEAPONTYPE_M4:
			case WEAPONTYPE_RUGER:
			case WEAPONTYPE_M60:
			case WEAPONTYPE_MINIGUN:
			case WEAPONTYPE_HELICANNON:
#ifdef EX_WEAPON_AK47 // Fire
			case WEAPONTYPE_AK47:
#endif
#ifdef EX_WEAPON_M16 // Fire
			case WEAPONTYPE_M16:
#endif
#ifdef EX_WEAPON_UZIOLD // Fire
			case WEAPONTYPE_UZIOLD:
#endif
#ifdef EX_WEAPON_DEAGLE // Fire
			case WEAPONTYPE_DEAGLE:
#endif
#ifdef EX_WEAPON_BERETTA // Fire
			case WEAPONTYPE_BERETTA:
#endif
#ifdef EX_WEAPON_STEYR // Fire
			case WEAPONTYPE_STEYR:
#endif
			{
				if ((TheCamera.PlayerWeaponMode.Mode == CCam::MODE_HELICANNON_1STPERSON || TheCamera.PlayerWeaponMode.Mode == CCam::MODE_M16_1STPERSON)
					&& shooter == FindPlayerPed()) {
					addFireRateAsDelay = true;
					fired = FireM16_1stPerson(shooter);
				} else {
					addFireRateAsDelay = false;
					fired = FireInstantHit(shooter, source);
				}
				break;
			}

			case WEAPONTYPE_ROCKETLAUNCHER:
			{
				if ( shooter->IsPed() && ((CPed*)shooter)->m_pSeekTarget != nil )
				{
					float distToTarget = (shooter->GetPosition() - ((CPed*)shooter)->m_pSeekTarget->GetPosition()).Magnitude();

					if ( distToTarget > 8.0f || ((CPed*)shooter)->IsPlayer() )
						fired = FireProjectile(shooter, source, 0.0f);
					else
						fired = false;
				}
				else
					fired = FireProjectile(shooter, source, 0.0f);

				break;
			}

			case WEAPONTYPE_MOLOTOV:
			case WEAPONTYPE_GRENADE:
			case WEAPONTYPE_DETONATOR_GRENADE:
			case WEAPONTYPE_TEARGAS:
			{
				if ( shooter == FindPlayerPed() )
				{
					fired = FireProjectile(shooter, source, ((CPlayerPed*)shooter)->m_fAttackButtonCounter*0.0375f);
				}
#ifdef IMPROVED_TECH_PART // wanted system
				else if ( shooter->IsPed() && ((CPed*)shooter)->m_nPedType == PEDTYPE_COP && !FindPlayerPed()->m_pWanted->IsPlayerHides())
				{
					float distToTarget = (shooter->GetPosition() - FindPlayerCoors()).Magnitude();
					float power = Clamp(distToTarget * 0.04f, 0.3f, 1.0f);

					fired = FireProjectile(shooter, source, power);
				}
#endif
				else if ( shooter->IsPed() && ((CPed*)shooter)->m_pSeekTarget != nil )
				{
					float distToTarget = (shooter->GetPosition() - ((CPed*)shooter)->m_pSeekTarget->GetPosition()).Magnitude();
					float power = Clamp((distToTarget-10.0f)*0.02f, 0.2f, 1.0f);

					fired = FireProjectile(shooter, source, power);
				}
				else
					fired = FireProjectile(shooter, source, 0.3f);

				if (m_eWeaponType == WEAPONTYPE_DETONATOR_GRENADE) {
					((CPed*)shooter)->GiveWeapon(WEAPONTYPE_DETONATOR, 1, true);
					((CPed*)shooter)->GetWeapon(((CPed*)shooter)->GetWeaponSlot(WEAPONTYPE_DETONATOR)).m_eWeaponState = WEAPONSTATE_READY;
					((CPed*)shooter)->SetCurrentWeapon(WEAPONTYPE_DETONATOR);
				}
				break;
			}

			case WEAPONTYPE_FLAMETHROWER:
			{
				fired = FireAreaEffect(shooter, source);

				break;
			}

			case WEAPONTYPE_DETONATOR:
			{
				CWorld::UseDetonator(shooter);
				m_nAmmoTotal  = 1;
				m_nAmmoInClip = m_nAmmoTotal;
				fired = true;

				break;
			}
			
			case WEAPONTYPE_CAMERA:
			{
				fired = TakePhotograph(shooter);

				break;
			}

			default:
			{
				debug("Unknown weapon type, Weapon.cpp");
				break;
			}
		}

		if (fired)
		{
			bool isPlayer = false;

			if (shooter->IsPed())
			{
				CPed* shooterPed = (CPed*)shooter;
				
				if ( m_eWeaponType != WEAPONTYPE_CAMERA )
				{
					shooterPed->bIsShooting = true;
	
					if (shooterPed->IsPlayer())
						isPlayer = true;
				}
				
				DMAudio.PlayOneShot(shooterPed->m_audioEntityId, SOUND_WEAPON_SHOT_FIRED, 0.0f);

				if ( isPlayer )
				{
					CPed *aimPed = (CPed *)shooterPed->m_pSeekTarget;
					if ( aimPed )
					{
						if ( aimPed->IsPed() )
							shooterPed->Say(SOUND_PED_ON_FIRE);
					}
				}
			}
			
			switch ( m_eWeaponType )
			{
				case WEAPONTYPE_COLT45:
				case WEAPONTYPE_PYTHON:
				case WEAPONTYPE_SHOTGUN:
				case WEAPONTYPE_SPAS12_SHOTGUN:
				case WEAPONTYPE_STUBBY_SHOTGUN:
				case WEAPONTYPE_TEC9:
				case WEAPONTYPE_UZI:
				case WEAPONTYPE_SILENCED_INGRAM:
				case WEAPONTYPE_MP5:
				case WEAPONTYPE_M4:
				case WEAPONTYPE_RUGER:
				case WEAPONTYPE_SNIPERRIFLE:
				case WEAPONTYPE_LASERSCOPE:
				case WEAPONTYPE_M60:
				case WEAPONTYPE_MINIGUN:
#ifdef EX_WEAPON_AK47 // RoundsFiredByPlayer
				case WEAPONTYPE_AK47:
#endif
#ifdef EX_WEAPON_M16 // RoundsFiredByPlayer
				case WEAPONTYPE_M16:
#endif
#ifdef EX_WEAPON_SHOTGUN2 // RoundsFiredByPlayer
				case WEAPONTYPE_SHOTGUN2:
#endif
#ifdef EX_WEAPON_UZIOLD // RoundsFiredByPlayer
				case WEAPONTYPE_UZIOLD:
#endif
#ifdef EX_WEAPON_DEAGLE // RoundsFiredByPlayer
				case WEAPONTYPE_DEAGLE:
#endif
#ifdef EX_WEAPON_BERETTA // RoundsFiredByPlayer
				case WEAPONTYPE_BERETTA:
#endif
#ifdef EX_WEAPON_STEYR // RoundsFiredByPlayer
				case WEAPONTYPE_STEYR:
#endif
					CStats::RoundsFiredByPlayer++;
					break;
					
				case WEAPONTYPE_GRENADE:
				case WEAPONTYPE_DETONATOR_GRENADE:
				case WEAPONTYPE_MOLOTOV:
				case WEAPONTYPE_ROCKET:
				case WEAPONTYPE_ROCKETLAUNCHER:
				case WEAPONTYPE_DETONATOR:
				case WEAPONTYPE_HELICANNON:
					CStats::KgsOfExplosivesUsed++;
					break;
			}
	  

			if (m_nAmmoInClip > 0)
				m_nAmmoInClip--;

			if (m_nAmmoTotal > 0 && (m_nAmmoTotal < 25000 || isPlayer) && (!isPlayer || CStats::GetPercentageProgress() < 100.0f || m_eWeaponType == WEAPONTYPE_DETONATOR))
				m_nAmmoTotal--;

			if (m_eWeaponState == WEAPONSTATE_READY && m_eWeaponType == WEAPONTYPE_FLAMETHROWER)
				DMAudio.PlayOneShot(((CPhysical*)shooter)->m_audioEntityId, SOUND_WEAPON_FLAMETHROWER_FIRE, 0.0f);

			m_eWeaponState = WEAPONSTATE_FIRING;

			if (m_nAmmoInClip == 0)
			{
				if (m_nAmmoTotal == 0) {
					if (TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_CAMERA)
						CPad::GetPad(0)->Clear(false);

					return true;
				}

				m_eWeaponState = WEAPONSTATE_RELOADING;
				m_nTimer = CTimer::GetTimeInMilliseconds() + GetInfo()->m_nReload;

				if (shooter == FindPlayerPed())
				{
					if (CWorld::Players[CWorld::PlayerInFocus].m_bFastReload)
						m_nTimer = CTimer::GetTimeInMilliseconds() + GetInfo()->m_nReload / 4;
				}

				return true;
			}

			if ( addFireRateAsDelay )
				m_nTimer = CTimer::GetTimeInMilliseconds() + GetInfo()->m_nFiringRate;
			else
				m_nTimer = CTimer::GetTimeInMilliseconds();
		}
	}
	else
	{
		if ( m_eWeaponState != WEAPONSTATE_FIRING )
		{
			m_nTimer = CTimer::GetTimeInMilliseconds() + GetInfo()->m_nReload;
			m_eWeaponState = WEAPONSTATE_FIRING;

			if (shooter->IsPed() && m_eWeaponType != WEAPONTYPE_CHAINSAW)
			{
				DMAudio.PlayOneShot(((CPed*)shooter)->m_audioEntityId, SOUND_MELEE_ATTACK_START, m_eWeaponType << 8);
			}
		}

		fired = FireMelee(shooter, *source);
	}

	if ( m_eWeaponType == WEAPONTYPE_UNARMED || m_eWeaponType == WEAPONTYPE_BASEBALLBAT )
		return true;
	else
		return fired;
}

bool
CWeapon::FireFromCar(CVehicle *shooter, bool left, bool right)
{
	ASSERT(shooter!=nil);

	if ( m_eWeaponState != WEAPONSTATE_READY && m_eWeaponState != WEAPONSTATE_FIRING )
		return false;

	if ( m_nAmmoInClip <= 0 )
		return false;

	if ( FireInstantHitFromCar(shooter, left, right) )
	{
		DMAudio.PlayOneShot(shooter->m_audioEntityId, SOUND_WEAPON_SHOT_FIRED, 0.0f);

		if ( m_nAmmoInClip > 0 )
			m_nAmmoInClip--;

		if ( m_nAmmoTotal < 25000 && m_nAmmoTotal > 0 && (!shooter || shooter->GetStatus() != STATUS_PLAYER || CStats::GetPercentageProgress() < 100.f))
			m_nAmmoTotal--;

		m_eWeaponState = WEAPONSTATE_FIRING;

		if ( m_nAmmoInClip == 0 )
		{
			if ( m_nAmmoTotal == 0 )
				return true;

			m_eWeaponState = WEAPONSTATE_RELOADING;
			m_nTimer = CTimer::GetTimeInMilliseconds() + GetInfo()->m_nReload;

			return true;
		}

		m_nTimer = CTimer::GetTimeInMilliseconds() + 1000;
	}

	return true;
}

bool
CWeapon::FireMelee(CEntity *shooter, CVector &fireSource)
{
	ASSERT(shooter!=nil);

	CWeaponInfo *info = GetInfo();

	bool anim2Playing = false;
	
	if ( CPed::GetFireAnimGround(info, false) != (AnimationId)0 )
	{
		if ( RpAnimBlendClumpGetAssociation(shooter->GetClump(), CPed::GetFireAnimGround(info, false)) )
			anim2Playing = true;
	}
	
	ASSERT(shooter->IsPed());

	CPed *shooterPed = (CPed*)shooter;

	if (shooterPed == FindPlayerPed())
	{
		if (m_eWeaponType == WEAPONTYPE_GOLFCLUB || m_eWeaponType == WEAPONTYPE_NIGHTSTICK ||
			(m_eWeaponType >= WEAPONTYPE_BASEBALLBAT && m_eWeaponType <= WEAPONTYPE_CHAINSAW))
		{	
			CGlass::BreakGlassPhysically(fireSource, info->m_fRadius);
			
			if (m_eWeaponType == WEAPONTYPE_CHAINSAW)
				CEventList::RegisterEvent(EVENT_GUNSHOT, EVENT_ENTITY_PED, FindPlayerPed(), FindPlayerPed(), 1000);
		}
	}

	int damageEntityRegistered = 0;

	for ( int32 i = 0; i < shooterPed->m_numNearPeds; i++ )
	{
		CPed *victimPed = shooterPed->m_nearPeds[i];
		ASSERT(victimPed!=nil);

		if ( (victimPed->m_nPedType != shooterPed->m_nPedType || victimPed == shooterPed->m_pSeekTarget)
				&& victimPed != shooterPed->m_leader || !(CGeneral::GetRandomNumber() & 31)
				&& (!shooterPed->IsGangMember() || victimPed->CanBeDamagedByThisGangMember(shooterPed)) )
		{
			bool collided = false;

			if (victimPed->m_nPedState == PED_DRIVING && (m_eWeaponType == WEAPONTYPE_UNARMED || m_eWeaponType == WEAPONTYPE_BRASSKNUCKLE
				|| info->IsFlagSet(WEAPONFLAG_FIGHTMODE)))
				continue;

			float victimPedRadius = victimPed->GetBoundRadius() + info->m_fRadius;
			if ( victimPed->bUsesCollision || victimPed->Dead() || victimPed->Driving() )
			{
				CVector victimPedPos = victimPed->GetPosition();
				if ( SQR(victimPedRadius) > (victimPedPos-fireSource).MagnitudeSqr() )
				{
					CVector collisionDist;
					CColModel* victimPedCol = &CTempColModels::ms_colModelPed1;
					bool useLocalPos = false;
					if (victimPed->m_nPedState == PED_FALL
						|| victimPed->m_nPedState == PED_DIE && victimPed->bIsPedDieAnimPlaying
						|| victimPed->m_nWaitState == WAITSTATE_SIT_IDLE
						|| victimPed->m_nWaitState == WAITSTATE_SUN_BATHE_IDLE)
					{
						useLocalPos = true;
						victimPedCol = ((CPedModelInfo*)CModelInfo::GetModelInfo(victimPed->GetModelIndex()))->AnimatePedColModelSkinnedWorld(victimPed->GetClump());
					} else if (victimPed->DyingOrDead()) {
						victimPedCol = &CTempColModels::ms_colModelPedGroundHit;
					}

					int32 s = 0;
					while ( s < victimPedCol->numSpheres )
					{
						CColSphere *sphere = &victimPedCol->spheres[s];

						if (useLocalPos)
							collisionDist = sphere->center - fireSource;
						else
							collisionDist = victimPedPos + sphere->center - fireSource;

						if ( SQR(sphere->radius + info->m_fRadius) > collisionDist.MagnitudeSqr() )
						{
							collided = true;
							break;
						}
						s++;
					}

					if ( !(victimPed->IsPlayer() && victimPed->GetPedState() == PED_GETUP) )
					{
						if ( collided )
						{
							float victimPedHealth = victimPed->m_fHealth;
							CVector bloodPos = fireSource + (collisionDist*0.7f);

							CVector2D posOffset(shooterPed->GetPosition().x-victimPedPos.x, shooterPed->GetPosition().y-victimPedPos.y);

							int32 localDir = victimPed->GetLocalDirection(posOffset);

							bool isHeavy = m_eWeaponType >= WEAPONTYPE_GOLFCLUB && m_eWeaponType <= WEAPONTYPE_KATANA && m_eWeaponType != WEAPONTYPE_HAMMER;

							if (shooterPed->m_fDamageImpulse == 0.0f)
							{
								shooterPed->m_pDamageEntity = victimPed;
								victimPed->RegisterReference(&shooterPed->m_pDamageEntity);
							}

							damageEntityRegistered = 3;
							if (victimPed->bInVehicle)
							{
								CVehicle *victimVeh = victimPed->m_pMyVehicle;
								if (victimVeh)
								{
									if (victimVeh->IsBike())
									{
										CBike *victimBike = (CBike*)victimVeh;
										victimBike->KnockOffRider(m_eWeaponType, localDir, victimPed, false);
										if (victimBike->pDriver)
											victimBike->pDriver->ReactToAttack(shooterPed);
										else
										{
											if (victimVeh->pPassengers[0])
												victimVeh->pPassengers[0]->ReactToAttack(shooterPed);
										}
										continue;
									}
								}
							}

							if ( !victimPed->DyingOrDead() )
								victimPed->ReactToAttack(shooterPed);

							uint8 hitLevel = HITLEVEL_HIGH;
							if ( isHeavy && (victimPed->OnGround() || victimPed->m_nWaitState == WAITSTATE_SUN_BATHE_IDLE))
								hitLevel = HITLEVEL_GROUND;

							victimPed->StartFightDefend(localDir, hitLevel, 10);

							if ( !victimPed->DyingOrDead() )
							{
								if ( shooterPed->IsPlayer() && isHeavy && anim2Playing )
									victimPed->InflictDamage(shooterPed, m_eWeaponType, 100.0f, PEDPIECE_TORSO, localDir);
								else if ( shooterPed->IsPlayer() && ((CPlayerPed*)shooterPed)->m_bAdrenalineActive )
									victimPed->InflictDamage(shooterPed, m_eWeaponType, 3.5f*info->m_nDamage, PEDPIECE_TORSO, localDir);
								else
								{
									if ( victimPed->IsPlayer() && isHeavy ) // wtf, it's not fair
										victimPed->InflictDamage(shooterPed, m_eWeaponType, 2.0f*info->m_nDamage, PEDPIECE_TORSO, localDir);
									else
										victimPed->InflictDamage(shooterPed, m_eWeaponType,      info->m_nDamage, PEDPIECE_TORSO, localDir);
								}
							}

							if ( CGame::nastyGame && victimPed->GetIsOnScreen() )
							{
								CVector dir = collisionDist * RecipSqrt(1.0f, 10.0f*collisionDist.MagnitudeSqr());

								CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, dir);
								CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, dir);
								CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, dir);

								if ( isHeavy )
								{
									dir.x += CGeneral::GetRandomNumberInRange(-0.05f, 0.05f);
									dir.y += CGeneral::GetRandomNumberInRange(-0.05f, 0.05f);
									CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, dir);

									dir.x += CGeneral::GetRandomNumberInRange(-0.05f, 0.05f);
									dir.y += CGeneral::GetRandomNumberInRange(-0.05f, 0.05f);
									CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, dir);
								}

								if (m_eWeaponType == WEAPONTYPE_CHAINSAW)
								{
									if (victimPed->m_nPedState != PED_DEAD && !((CTimer::GetFrameCounter() + 17) & 1)
										|| victimPed->m_nPedState == PED_DEAD && !((CTimer::GetFrameCounter() + 17) & 3))
									{
										CParticle::AddParticle(PARTICLE_TEST, bloodPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.2f);
									}
									CVector newDir(dir);
									newDir.z += 0.2f;
									CParticle::AddParticle(PARTICLE_BLOOD_SMALL, bloodPos, newDir);
									CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, newDir);
									newDir.z = dir.z + 0.1f;
									CParticle::AddParticle(PARTICLE_BLOOD, bloodPos, newDir);
									newDir.x = 0.0f;
									newDir.y = 0.0f;
									newDir.z = 0.01f;
									CParticle::AddParticle(PARTICLE_DEBRIS2, bloodPos, newDir);

									CVector dropDir(CGeneral::GetRandomNumberInRange(-0.15f, 0.15f), CGeneral::GetRandomNumberInRange(0.1f, 0.35f), 0.f);
									CVector dropPos(CGeneral::GetRandomNumberInRange(SCREEN_STRETCH_X(50.0f), SCREEN_STRETCH_FROM_RIGHT(50.0f)),
										CGeneral::GetRandomNumberInRange(SCREEN_STRETCH_Y(50.0f), SCREEN_STRETCH_FROM_BOTTOM(50.0f)), 1.f);
									CParticle::AddParticle(PARTICLE_BLOODDROP, dropPos, dropDir, nil, CGeneral::GetRandomNumberInRange(0.1f, 0.15f),
										CRGBA(0, 0, 0, 0), 0, 0, CGeneral::GetRandomNumber() & 1, 0);

#ifdef IMPROVED_TECH_PART // blood screen droplets
									ScreenDroplets::FillScreenMoving(1.0f, true);
#endif
								}
								if (info->m_AnimToPlay == ASSOCGRP_KNIFE)
								{
									dir += 0.1f * shooterPed->GetUp() + 0.05f * shooterPed->GetRight();
									CParticle::AddParticle(PARTICLE_BLOOD_SPURT, bloodPos, dir);
									CParticle::AddParticle(PARTICLE_BLOOD_SPURT, bloodPos, dir);
									CParticle::AddParticle(PARTICLE_BLOOD_SPURT, bloodPos, dir);
								}
							}

							if ( !victimPed->OnGround() )
							{
								if ( victimPed->m_fHealth > 0.0f
									&& (victimPed->m_fHealth < 30.0f && victimPedHealth > 30.0f ||
										(isHeavy || m_eWeaponType == WEAPONTYPE_BRASSKNUCKLE) && !victimPed->IsPlayer()) )
								{
									posOffset.Normalise();
									victimPed->bIsStanding = false;
									if(m_eWeaponType == WEAPONTYPE_CHAINSAW)
										victimPed->ApplyMoveForce(posOffset.x*-2.0f, posOffset.y*-2.0f, 2.0f);
									else
										victimPed->ApplyMoveForce(posOffset.x*-5.0f, posOffset.y*-5.0f, 3.0f);

									if ( isHeavy && victimPed->IsPlayer() )
										victimPed->SetFall(3000, AnimationId(ANIM_STD_HIGHIMPACT_FRONT + localDir), false);
									else
										victimPed->SetFall(1500, AnimationId(ANIM_STD_HIGHIMPACT_FRONT + localDir), false);

									shooterPed->m_pSeekTarget = victimPed;
									shooterPed->m_pSeekTarget->RegisterReference(&shooterPed->m_pSeekTarget);
								}
							}
							else if (victimPed->Dying() && !anim2Playing)
							{
								posOffset.Normalise();
								victimPed->bIsStanding = false;
								if(m_eWeaponType == WEAPONTYPE_CHAINSAW)
									victimPed->ApplyMoveForce(posOffset.x*-1.0f, posOffset.y*-1.0f, 1.0f);
								else
									victimPed->ApplyMoveForce(posOffset.x*-5.0f, posOffset.y*-5.0f, 3.0f);
							}

							m_eWeaponState = WEAPONSTATE_MELEE_MADECONTACT;

							if (m_eWeaponType != WEAPONTYPE_KNIFE && m_eWeaponType != WEAPONTYPE_MACHETE
								&& m_eWeaponType != WEAPONTYPE_KATANA && m_eWeaponType != WEAPONTYPE_CHAINSAW) {

								if (victimPed->m_nPedType == PEDTYPE_COP)
									CEventList::RegisterEvent(EVENT_ASSAULT_POLICE, EVENT_ENTITY_PED, victimPed, shooterPed, 2000);
								else
									CEventList::RegisterEvent(EVENT_ASSAULT, EVENT_ENTITY_PED, victimPed, shooterPed, 2000);
							} else {
								if (victimPed->m_nPedType == PEDTYPE_COP)
									CEventList::RegisterEvent(EVENT_ASSAULT_NASTYWEAPON_POLICE, EVENT_ENTITY_PED, victimPed, shooterPed, 2000);
								else
									CEventList::RegisterEvent(EVENT_ASSAULT_NASTYWEAPON, EVENT_ENTITY_PED, victimPed, shooterPed, 2000);
							}
						}
					}
				}
			}
		}
	}
	CVehicle *nearVeh = (CVehicle*)CWorld::TestSphereAgainstWorld(fireSource, info->m_fRadius, nil, false, true, false, false, false, false);
	if (nearVeh && nearVeh->IsCar())
	{
		CAutomobile *nearCar = (CAutomobile*)nearVeh;
		m_eWeaponState = WEAPONSTATE_MELEE_MADECONTACT;
		if (shooterPed == FindPlayerPed())
		{
			if (nearCar->IsLawEnforcementVehicle())
			{
				FindPlayerPed()->SetWantedLevelNoDrop(1);
			}
			CEventList::RegisterEvent(EVENT_ASSAULT, EVENT_ENTITY_VEHICLE, nearCar, shooterPed, 2000);
		}
		float oldHealth = nearCar->m_fHealth;
		if (m_eWeaponType == WEAPONTYPE_CHAINSAW)
		{
			for( int32 i=0; i<4; i++ )
			{
				CParticle::AddParticle(PARTICLE_SPARK_SMALL, gaTempSphereColPoints[0].point, CVector(0.0f, 0.0f, 0.3f));
				CParticle::AddParticle(PARTICLE_SPARK, gaTempSphereColPoints[0].point, gaTempSphereColPoints[0].normal * 0.1f);
			}
		}
		if (m_eWeaponType == WEAPONTYPE_CHAINSAW)
		{
			nearCar->VehicleDamage(info->m_nDamage * (0.00075f * nearCar->pHandling->fMass), gaTempSphereColPoints[0].pieceB);

			CParticle::AddParticle(PARTICLE_HEATHAZE, gaTempSphereColPoints[0].point, CVector(0.0f, 0.0f, 0.0f), 0, 0.0f, 0, 0, 0, 0);
		}
		else
		{
			nearCar->VehicleDamage(info->m_nDamage* (0.01f * nearCar->pHandling->fMass), gaTempSphereColPoints[0].pieceB);
		}
		if (nearCar->m_fHealth < oldHealth)
		{
			nearCar->m_nLastWeaponDamage = m_eWeaponType;
			nearCar->m_pLastDamageEntity = shooterPed;
		}
		if (shooterPed->m_fDamageImpulse == 0.0f)
		{
			shooterPed->m_pDamageEntity = nearCar;
			nearCar->RegisterReference(&shooterPed->m_pDamageEntity);
		}
		damageEntityRegistered = 2;
		if (FindPlayerPed()->GetWeapon() == this && nearCar->VehicleCreatedBy != MISSION_VEHICLE)
		{
			if (nearCar->AutoPilot.m_nDrivingStyle != DRIVINGSTYLE_PLOUGH_THROUGH
				&& (CGeneral::GetRandomTrueFalse() || nearCar->AutoPilot.m_nCarMission != MISSION_CRUISE))
			{
				int leaveCarDelay = 200;
				CPed *driver = nearCar->pDriver;
				if (driver && driver->CharCreatedBy != MISSION_CHAR)
				{
					if (driver->m_pedStats->m_temper <= driver->m_pedStats->m_fear)
					{
						driver->SetObjective(OBJECTIVE_FLEE_ON_FOOT_TILL_SAFE);
					}
					else
					{
						driver->SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, FindPlayerPed());
						driver->m_objectiveTimer = CTimer::GetTimeInMilliseconds() + 10000;
						driver->m_prevObjective = OBJECTIVE_KILL_CHAR_ON_FOOT;
					}
					driver->m_leaveCarTimer = CTimer::GetTimeInMilliseconds() + 200;
					leaveCarDelay = 400;
				}
				for (int j = 0; j < nearCar->m_nNumPassengers; ++j)
				{
					CPed *passenger = nearCar->pPassengers[j];
					if (passenger && passenger->CharCreatedBy != MISSION_CHAR)
					{
						nearCar->pPassengers[j]->SetObjective(OBJECTIVE_FLEE_ON_FOOT_TILL_SAFE);
						passenger->m_leaveCarTimer = CTimer::GetTimeInMilliseconds() + leaveCarDelay;
						leaveCarDelay += 200;
					}
				}
			}
			else
			{
				CPed *driver = nearCar->pDriver;
				if (driver)
				{
					if (driver->m_objective != OBJECTIVE_LEAVE_CAR && driver->m_objective != OBJECTIVE_KILL_CHAR_ON_FOOT &&
						driver->m_objective != OBJECTIVE_FLEE_ON_FOOT_TILL_SAFE)
					{
						if (nearCar->AutoPilot.m_nDrivingStyle != DRIVINGSTYLE_PLOUGH_THROUGH)
							nearCar->AutoPilot.m_nCruiseSpeed = nearCar->AutoPilot.m_nCruiseSpeed * 1.5f;

						nearCar->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_PLOUGH_THROUGH;
					}
				}
			}
		}
	}
	if (m_eWeaponType == WEAPONTYPE_CHAINSAW)
	{
		CEntity *nearStatic = (CObject*)CWorld::TestSphereAgainstWorld(fireSource, info->m_fRadius, nil, true, false, false, true, false, false);
		if (nearStatic)
		{
			for(int i=0; i < 4; i++) {
				CParticle::AddParticle(PARTICLE_SPARK_SMALL, gaTempSphereColPoints[0].point, CVector(0.0f, 0.0f, 0.3f), 0, 0.0f, 0, 0, 0, 0);
				CParticle::AddParticle(PARTICLE_SPARK, gaTempSphereColPoints[0].point, 0.1f * gaTempSphereColPoints[0].normal, 0, 0.0f, 0, 0, 0, 0);
			}

			CParticle::AddParticle(PARTICLE_HEATHAZE, gaTempSphereColPoints[0].point, CVector(0.0f, 0.0f, 0.0f), 0, 0.0f, 0, 0, 0, 0);
			
			if (!damageEntityRegistered)
			{
				m_eWeaponState = WEAPONSTATE_MELEE_MADECONTACT;
				if (shooterPed->m_fDamageImpulse == 0.0f)
				{
					shooterPed->m_pDamageEntity = nearStatic;
					nearStatic->RegisterReference(&shooterPed->m_pDamageEntity);
				}
			}
			if (nearStatic->IsObject() && ((CObject*)nearStatic)->m_nCollisionDamageEffect >= DAMAGE_EFFECT_SMASH_COMPLETELY)
				((CObject*)nearStatic)->ObjectDamage(200.0f);
		}
	}

	return true;
}

bool
CWeapon::FireInstantHit(CEntity *shooter, CVector *fireSource)
{
	ASSERT(shooter!=nil);
	ASSERT(fireSource!=nil);

	CWeaponInfo *info = GetInfo();

	CVector source, target;
	CColPoint point;
	CEntity *victim = nil;

	float heading = RADTODEG(shooter->GetForward().Heading());
	float angle   = DEGTORAD(heading);

	CVector2D ahead(-Sin(angle), Cos(angle));
	ahead.Normalise();

	CVector vel = ((CPed *)shooter)->m_vecMoveSpeed;
	int32 shooterMoving = false;
	if ( Abs(vel.x) > 0.0f && Abs(vel.y) > 0.0f )
		shooterMoving = true;

	if ( shooter == FindPlayerPed() )
	{
		static float prev_heading = 0.0f;
		prev_heading = ((CPed*)shooter)->m_fRotationCur;
	}

#ifdef IMPROVED_MENU_AND_INPUT
	if ( shooter->IsPed() && ((CPed *)shooter)->m_pPointGunAt && !((CPed*)shooter)->IsPlayer() ||
		(shooter->IsPed() && ((CPed*)shooter)->IsPlayer() && ((CPed*)shooter)->m_pPointGunAt && CPad::GetPad(0)->IsAffectedByController && FrontEndMenuManager.m_PrefsAutoaim))
#else
	if ( shooter->IsPed() && ((CPed *)shooter)->m_pPointGunAt )
#endif
	{
		CPed *shooterPed = (CPed *)shooter;
		if ( shooterPed->m_pedIK.m_flags & CPedIK::GUN_POINTED_SUCCESSFULLY )
		{
			int32 accuracy   = shooterPed->m_wepAccuracy;
			int32 inaccuracy = 100-accuracy;

			CPed *threatAttack = (CPed*)shooterPed->m_pPointGunAt;
			if ( threatAttack->IsPed() )
			{
				threatAttack->m_pedIK.GetComponentPosition(target, PED_MID);
				threatAttack->ReactToPointGun(shooter);
			}
			else
				target = threatAttack->GetPosition();

			target -= *fireSource;
			float distToTarget = Max(target.Magnitude(), 0.01f);
			target *= info->m_fRange / distToTarget;
			target += *fireSource;

			if (shooter == FindPlayerPed() && inaccuracy != 0.f)
			{
				float newInaccuracy = fPlayerAimScale * FindPlayerPed()->m_fAttackButtonCounter * (inaccuracy * Min(1.f, fPlayerAimScaleDist / distToTarget));
				if (FindPlayerPed()->bIsDucking)
					newInaccuracy *= 0.4f;

				target.x += CGeneral::GetRandomNumberInRange(-0.15f, 0.15f) * newInaccuracy;
				target.y += CGeneral::GetRandomNumberInRange(-0.15f, 0.15f) * newInaccuracy;
				target.z += CGeneral::GetRandomNumberInRange(-0.2f, 0.2f) * newInaccuracy;
				FindPlayerPed()->m_fAttackButtonCounter += info->m_nDamage * 0.04f;
			}
			else if (inaccuracy > 0.f)
			{
				if (threatAttack == FindPlayerPed())
				{
					float speed = Min(0.33f, FindPlayerPed()->m_vecMoveSpeed.Magnitude());
					inaccuracy *= (0.3f * speed * 100.f / 33.f + 0.8f);
				}
				target.x += CGeneral::GetRandomNumberInRange(-0.2f, 0.2f) * inaccuracy;
				target.y += CGeneral::GetRandomNumberInRange(-0.2f, 0.2f) * inaccuracy;
				target.z += CGeneral::GetRandomNumberInRange(-0.1f, 0.1f) * inaccuracy;
			}

			if (shooter == FindPlayerPed())
				CWorld::bIncludeDeadPeds = true;

			CWorld::bIncludeBikers = true;
			ProcessLineOfSight(*fireSource, target, point, victim, m_eWeaponType, shooter, true, true, true, true, true, false, false);
			CWorld::bIncludeDeadPeds = false;
			CWorld::bIncludeBikers = false;
		}
		else
		{

			target.x = info->m_fRange;
			target.y = 0.0f;
			target.z = 0.0f;

			shooterPed->TransformToNode(target, PED_HANDR);

			CWorld::bIncludeBikers = true;
			ProcessLineOfSight(*fireSource, target, point, victim, m_eWeaponType, shooter, true, true, true, true, true, false, false);
			CWorld::bIncludeBikers = false;
		}
#ifdef FIX_BUGS
		// fix muzzleflash rotation
		heading = CGeneral::GetAngleBetweenPoints(source.x, source.y, target.x, target.y);
		angle = DEGTORAD(heading);

		ahead = CVector2D(-Sin(angle), Cos(angle));
		ahead.Normalise();
#endif
	}
#ifdef IMPROVED_MENU_AND_INPUT
	else if ( shooter == FindPlayerPed() )
#else
	else if ( shooter == FindPlayerPed() && TheCamera.Cams[0].Using3rdPersonMouseCam()  )
#endif
	{
#ifdef FREE_CAM
		if (CCamera::bFreeCam) {
			CPlayerPed* shooterPed = (CPlayerPed*)shooter;
			Find3rdPersonCamTargetVectorFromCachedVectors(info->m_fRange, *fireSource, source, target, shooterPed->m_cachedCamSource, shooterPed->m_cachedCamFront, shooterPed->m_cachedCamUp);

#ifdef FEATURES_INI // RecoilWhenFiring
			if (bRecoilWhenFiring) {
				float recoilMult = 1.0f;

				if (TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_REAL_1ST_PERSON)
					recoilMult += 0.6f;

#ifdef EX_WEAPON_AK47
				if (m_eWeaponType == WEAPONTYPE_AK47)
					recoilMult += 0.5f;
#endif

				if (m_eWeaponType == WEAPONTYPE_SILENCED_INGRAM)
					recoilMult += 0.4f;

				if (shooterPed->bIsDucking)
					recoilMult -= 0.5f;

				TheCamera.Cams[TheCamera.ActiveCam].Alpha += CGeneral::GetRandomNumberInRange(0.001f * recoilMult, 0.005f * recoilMult);
				TheCamera.Cams[TheCamera.ActiveCam].Beta += CGeneral::GetRandomNumberInRange(-0.005f * recoilMult, 0.005f * recoilMult);
			}
#endif

			if ((shooterPed->m_pedIK.m_flags & CPedIK::GUN_POINTED_SUCCESSFULLY) == 0) {
				target.x = info->m_fRange;
				target.y = 0.0f;
				target.z = 0.0f;

				shooterPed->TransformToNode(target, PED_HANDR);
			}
		} else
#endif
		{
			TheCamera.Find3rdPersonCamTargetVector(info->m_fRange, *fireSource, source, target);
		}

#ifdef FIX_BUGS
		// fix muzzleflash rotation
		heading = CGeneral::GetAngleBetweenPoints(source.x, source.y, target.x, target.y);
		angle = DEGTORAD(heading);

		ahead = CVector2D(-Sin(angle), Cos(angle));
		ahead.Normalise();
#endif
		CWorld::bIncludeBikers = true;
		CWorld::bIncludeDeadPeds = true;
		CWorld::bIncludeCarTyres = true;
#ifdef FIRING_AND_AIMING // we can't hit our vehicle while driveby
		CWorld::pIgnoreEntity = FindPlayerVehicle();
		ProcessLineOfSight(source, target, point, victim, m_eWeaponType, shooter, true, true, true, true, true, false, false);
		CWorld::pIgnoreEntity = NULL;
#else
		ProcessLineOfSight(source, target, point, victim, m_eWeaponType, shooter, true, true, true, true, true, false, false);
#endif
		CWorld::bIncludeBikers = false;
		CWorld::bIncludeDeadPeds = false;
		CWorld::bIncludeCarTyres = false;

#ifdef FIRING_AND_AIMING // make it possible to break the windshield during driveby
		if (FindPlayerVehicle() && FindPlayerPed()->bIsPlayerAiming && FindPlayerVehicle()->IsCar()) {
			if (DotProduct(FindPlayerVehicle()->GetForward(), TheCamera.Cams[TheCamera.ActiveCam].Front) > 0.9f) {
				// from CheckForShootingVehicleOccupant
				CAutomobile* car = (CAutomobile*)FindPlayerVehicle();
				if (car->Damage.ProgressPanelDamage(VEHPANEL_WINDSCREEN)) {
					if (car->Damage.GetPanelStatus(VEHPANEL_WINDSCREEN) == PANEL_STATUS_SMASHED2)
						car->Damage.ProgressPanelDamage(VEHPANEL_WINDSCREEN);

					car->SetPanelDamage(CAR_WINDSCREEN, VEHPANEL_WINDSCREEN, true);
					DMAudio.PlayOneShot(FindPlayerVehicle()->m_audioEntityId, SOUND_CAR_WINDSHIELD_CRACK, 0.f);
				}
			}
		}
#endif

		if (victim)
			CheckForShootingVehicleOccupant(&victim, &point, m_eWeaponType, source, target);

		int32 rotSpeed = 1;
		if ( m_eWeaponType == WEAPONTYPE_M4 
#ifdef EX_WEAPON_AK47 // FireInstantHit
			|| m_eWeaponType == WEAPONTYPE_AK47
#endif
#ifdef EX_WEAPON_M16 // FireInstantHit
			|| m_eWeaponType == WEAPONTYPE_M16
#endif
#ifdef EX_WEAPON_STEYR // FireInstantHit
			|| m_eWeaponType == WEAPONTYPE_STEYR
#endif
			)
			rotSpeed = 4;

		CVector bulletPos;
#ifdef IMPROVED_TECH_PART // damage
		if ( CHeli::TestBulletCollision(&source, &target, &bulletPos, info->m_nDamage / 4) )
#else
		if ( CHeli::TestBulletCollision(&source, &target, &bulletPos, 4) )
#endif
		{
			for ( int32 i = 0; i < 16; i++ )
				CParticle::AddParticle(PARTICLE_SPARK, bulletPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.0f, rotSpeed);
		}
	}
	else
	{
		uint32 model = shooter->GetModelIndex();
		if (model == MI_HUNTER || model == MI_SEASPAR || model == MI_SPARROW)
		{
			float inaccuracyMult = 0.6f;
			target = shooter->GetForward();
			if (shooter->GetStatus() == STATUS_PLAYER)
			{
				target *= info->m_fRange;
				target += *fireSource;
				CWeapon::DoDriveByAutoAiming(FindPlayerPed(), (CVehicle*)shooter, fireSource, &target);
				target -= *fireSource;
				target.Normalise();
				if (model == MI_SEASPAR || model == MI_SPARROW)
					inaccuracyMult = 0.1f;
				else
					inaccuracyMult = 0.3f;
			}
			target.x += CGeneral::GetRandomNumberInRange(-0.2f, 0.2f) * inaccuracyMult;
			target.y += CGeneral::GetRandomNumberInRange(-0.2f, 0.2f) * inaccuracyMult;
			target.z += CGeneral::GetRandomNumberInRange(-0.1f, 0.1f) * inaccuracyMult;

			target.Normalise();
			target *= info->m_fRange;
			target += *fireSource;
			CWorld::pIgnoreEntity = shooter;
			ProcessLineOfSight(*fireSource, target, point, victim, m_eWeaponType, shooter, true, true, true, true, true, true, false);
			CWorld::pIgnoreEntity = nil;

			int32 rotSpeed = 1;
			if (m_eWeaponType == WEAPONTYPE_M4
#ifdef EX_WEAPON_AK47 // FireInstantHit
				|| m_eWeaponType == WEAPONTYPE_AK47
#endif
#ifdef EX_WEAPON_M16 // FireInstantHit
				|| m_eWeaponType == WEAPONTYPE_M16
#endif
#ifdef EX_WEAPON_STEYR // FireInstantHit
				|| m_eWeaponType == WEAPONTYPE_STEYR
#endif
				)
				rotSpeed = 4;

			CVector bulletPos;
#ifdef IMPROVED_TECH_PART // damage
			if (CHeli::TestBulletCollision(fireSource, &target, &bulletPos, info->m_nDamage / 4))
#else
			if (CHeli::TestBulletCollision(fireSource, &target, &bulletPos, 4))
#endif
			{
				for (int32 i = 0; i < 16; i++)
					CParticle::AddParticle(PARTICLE_SPARK, bulletPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.0f, rotSpeed);
			}
		}
		else
		{
			float shooterHeading = RADTODEG(shooter->GetForward().Heading());
			float shooterAngle = DEGTORAD(shooterHeading);

			CVector2D rotOffset(-Sin(shooterAngle), Cos(shooterAngle));
			rotOffset.Normalise();

			target = *fireSource;
			target.x += rotOffset.x * info->m_fRange;
			target.y += rotOffset.y * info->m_fRange;

			CParticle::HandleShootableBirdsStuff(shooter, *fireSource);
			if (shooter->IsPed() && ((CPed*)shooter)->bDoomAim && (shooter != FindPlayerPed() || !info->IsFlagSet(WEAPONFLAG_CANAIM)))
			{
				CWeapon::DoDoomAiming(shooter, fireSource, &target);
			}

			CWorld::bIncludeBikers = true;
			ProcessLineOfSight(*fireSource, target, point, victim, m_eWeaponType, shooter, true, true, true, true, true, false, false);
			CWorld::bIncludeBikers = false;

			int32 rotSpeed = 1;
			if (m_eWeaponType == WEAPONTYPE_M4
#ifdef EX_WEAPON_AK47 // FireInstantHit
				|| m_eWeaponType == WEAPONTYPE_AK47
#endif
#ifdef EX_WEAPON_M16 // FireInstantHit
				|| m_eWeaponType == WEAPONTYPE_M16
#endif
#ifdef EX_WEAPON_STEYR // FireInstantHit
				|| m_eWeaponType == WEAPONTYPE_STEYR
#endif
				)
				rotSpeed = 4;

			CVector bulletPos;
#ifdef IMPROVED_TECH_PART // damage
			if (CHeli::TestBulletCollision(fireSource, &target, &bulletPos, info->m_nDamage / 4))
#else
			if (CHeli::TestBulletCollision(fireSource, &target, &bulletPos, 4))
#endif
			{
				for (int32 i = 0; i < 16; i++)
					CParticle::AddParticle(PARTICLE_SPARK, bulletPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.0f, rotSpeed);
			}
		}
	}

#ifdef IMPROVED_VEHICLES_2 // check for shooting vehicle lights and petrolcap
	if (victim) {
		CheckForShootingVehicleLights(victim, point);
		CheckForShootingVehiclePetrolCap(shooter, victim, point);
	}
#endif

	if ( shooter->IsPed() && victim)
	{
		if (victim == ((CPed*)shooter)->m_leader)
			return false;

		if (victim->IsPed() && ((CPed*)shooter)->IsGangMember() && !((CPed*)victim)->CanBeDamagedByThisGangMember((CPed*)shooter))
			return false;
	}

	if (shooter->IsPed())
		CEventList::RegisterEvent(EVENT_GUNSHOT, EVENT_ENTITY_PED, shooter, (CPed*)shooter, 1000);
	else if (shooter->IsVehicle() && ((CVehicle*)shooter)->pDriver)
		CEventList::RegisterEvent(EVENT_GUNSHOT, EVENT_ENTITY_VEHICLE, shooter, ((CVehicle*)shooter)->pDriver, 1000);

	if ( shooter == FindPlayerPed() )
	{
		if ( !(CTimer::GetFrameCounter() & 3) )
			MakePedsJumpAtShot((CPhysical*)shooter, fireSource, &target);
	}

	switch ( m_eWeaponType )
	{
		case WEAPONTYPE_M4:
		case WEAPONTYPE_RUGER:
		case WEAPONTYPE_M60:
		case WEAPONTYPE_MINIGUN:
		case WEAPONTYPE_HELICANNON:
#ifdef EX_WEAPON_AK47 // FireInstantHit
		case WEAPONTYPE_AK47:
#endif
#ifdef EX_WEAPON_M16 // FireInstantHit
		case WEAPONTYPE_M16:
#endif
#ifdef EX_WEAPON_STEYR // FireInstantHit
		case WEAPONTYPE_STEYR:
#endif
		{
			static uint8 counter = 0;

			if ( info->m_nFiringRate >= 50 || !(++counter & 1) )
			{
#ifdef FIX_BUGS
				AddGunFlashBigGuns(*fireSource, target);
#else
				AddGunFlashBigGuns(*fireSource, *fireSource + target);
#endif

				CVector gunshellPos = *fireSource;
				gunshellPos -= CVector(0.65f*ahead.x, 0.65f*ahead.y, 0.0f);
				CVector dir = CrossProduct(CVector(ahead.x, ahead.y, 0.0f), CVector(0.0f, 0.0f, 5.0f));
				dir.Normalise2D();
				AddGunshell(shooter, gunshellPos, CVector2D(dir.x, dir.y), 0.02f);
			}

			break;
		}

		case WEAPONTYPE_UZI:
		case WEAPONTYPE_TEC9:
		case WEAPONTYPE_SILENCED_INGRAM:
		case WEAPONTYPE_MP5:
#ifdef EX_WEAPON_UZIOLD // FireInstantHit
		case WEAPONTYPE_UZIOLD:
#endif
		{
			CPointLights::AddLight(CPointLights::LIGHT_POINT,
					*fireSource, CVector(0.0f, 0.0f, 0.0f), 5.0f,
					1.0f, 0.8f, 0.0f, CPointLights::FOG_NONE, false);

			CVector gunflashPos = *fireSource;

			if ( shooterMoving )
				gunflashPos += CVector(1.5f*vel.x, 1.5f*vel.y, 0.0f);

			CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.07f);
			gunflashPos += CVector(0.06f*ahead.x, 0.06f*ahead.y, 0.0f);
			CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.05f);
			gunflashPos += CVector(0.04f*ahead.x, 0.04f*ahead.y, 0.0f);
			CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.04f);
			gunflashPos += CVector(0.04f*ahead.x, 0.04f*ahead.y, 0.0f);
			CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.03f);
			gunflashPos += CVector(0.03f*ahead.x, 0.03f*ahead.y, 0.0f);
			CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.03f);
			gunflashPos += CVector(0.03f*ahead.x, 0.03f*ahead.y, 0.0f);
			CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.02f);
			gunflashPos += CVector(0.02f*ahead.x, 0.02f*ahead.y, 0.0f);
			CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.01f);

			CVector gunsmokePos = *fireSource;
			float rnd = CGeneral::GetRandomNumberInRange(0.05f, 0.25f);
			CParticle::AddParticle(PARTICLE_GUNSMOKE2, gunsmokePos, CVector(ahead.x*rnd, ahead.y*rnd, 0.0f));

			CVector gunshellPos = *fireSource;
			gunshellPos -= CVector(0.2f*ahead.x, 0.2f*ahead.y, 0.0f);
			CVector dir = CrossProduct(CVector(ahead.x, ahead.y, 0.0f), CVector(0.0f, 0.0f, 5.0f));
			dir.Normalise2D();
			AddGunshell(shooter, gunshellPos, CVector2D(dir.x, dir.y), 0.015f);

			break;
		}

		case WEAPONTYPE_COLT45:
		case WEAPONTYPE_PYTHON:
		case WEAPONTYPE_SNIPERRIFLE:
		case WEAPONTYPE_LASERSCOPE:
#ifdef EX_WEAPON_DEAGLE // FireInstantHit
		case WEAPONTYPE_DEAGLE:
#endif
#ifdef EX_WEAPON_BERETTA // FireInstantHit
		case WEAPONTYPE_BERETTA:
#endif
		{
			CPointLights::AddLight(CPointLights::LIGHT_POINT,
					*fireSource, CVector(0.0f, 0.0f, 0.0f), 5.0f,
					1.0f, 0.8f, 0.0f, CPointLights::FOG_NONE, false);

			CVector gunflashPos = *fireSource;

			if ( shooterMoving )
				gunflashPos += CVector(1.5f*vel.x, 1.5f*vel.y, 0.0f);

			CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.06f);
			gunflashPos += CVector(0.06f*ahead.x, 0.06f*ahead.y, 0.0f);
			CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.04f);
			gunflashPos += CVector(0.04f*ahead.x, 0.04f*ahead.y, 0.0f);
			CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.02f);

			CVector gunsmokePos = *fireSource;
			CParticle::AddParticle(PARTICLE_GUNSMOKE2, gunsmokePos, CVector(ahead.x*0.10f, ahead.y*0.10f, 0.0f), nil, 0.005f);
			CParticle::AddParticle(PARTICLE_GUNSMOKE2, gunsmokePos, CVector(ahead.x*0.15f, ahead.y*0.15f, 0.0f), nil, 0.015f);
			CParticle::AddParticle(PARTICLE_GUNSMOKE2, gunsmokePos, CVector(ahead.x*0.20f, ahead.y*0.20f, 0.0f), nil, 0.025f);

			CVector gunshellPos = *fireSource;
			gunshellPos -= CVector(0.2f*ahead.x, 0.2f*ahead.y, 0.0f);
			CVector dir = CrossProduct(CVector(ahead.x, ahead.y, 0.0f), CVector(0.0f, 0.0f, 5.0f));
			dir.Normalise2D();
			AddGunshell(shooter, gunshellPos, CVector2D(dir.x, dir.y), 0.015f);

			break;
		}
		default: break;
	}

	DoBulletImpact(shooter, victim, fireSource, &target, &point, ahead);

	return true;
}

void
CWeapon::AddGunFlashBigGuns(CVector start, CVector end)
{
	CPointLights::AddLight(CPointLights::LIGHT_POINT,
		start, CVector(0.0f, 0.0f, 0.0f), 5.0f,
		1.0f, 0.8f, 0.0f, CPointLights::FOG_NONE, false);
	CVector gunflashPos = start;

	CVector shootVec = end - start;

	// Wtf did you do there R*?
	shootVec.Normalise();
	CVector2D ahead = shootVec;
	ahead.Normalise();

	CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.08f);
	gunflashPos += CVector(0.06f * ahead.x, 0.06f * ahead.y, 0.0f);
	CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.06f);
	gunflashPos += CVector(0.06f * ahead.x, 0.06f * ahead.y, 0.0f);
	CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.06f);

	gunflashPos = start;
	gunflashPos += CVector(-0.1f * ahead.x, -0.1f * ahead.y, 0.0f);
	gunflashPos.z += 0.04f;
	CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.04f);
	gunflashPos.z += 0.04f;
	CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.02f);
	gunflashPos.z += 0.03f;
	CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.02f);

	gunflashPos = start;
	gunflashPos += CVector(-0.1f * ahead.x, -0.1f * ahead.y, 0.0f);
	gunflashPos.z -= 0.04f;
	CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.04f);
	gunflashPos.z -= 0.04f;
	CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.02f);
	gunflashPos.z -= 0.03f;
	CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.02f);

	CVector offset = CrossProduct(CVector(ahead.x, ahead.y, 0.0f), CVector(0.0f, 0.0f, 5.0f));
	offset.Normalise2D();

	gunflashPos = start;
	gunflashPos += CVector(-0.1f * ahead.x, -0.1f * ahead.y, 0.0f);
	gunflashPos += CVector(0.06f * offset.x, 0.06f * offset.y, 0.0f);
	CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.04f);
	gunflashPos += CVector(0.04f * offset.x, 0.04f * offset.y, 0.0f);
	CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.03f);
	gunflashPos += CVector(0.03f * offset.x, 0.03f * offset.y, 0.0f);
	CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.02f);

	gunflashPos = start;
	gunflashPos += CVector(-0.1f * ahead.x, -0.1f * ahead.y, 0.0f);
	gunflashPos -= CVector(0.06f * offset.x, 0.06f * offset.y, 0.0f);
	CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.04f);
	gunflashPos -= CVector(0.04f * offset.x, 0.04f * offset.y, 0.0f);
	CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.03f);
	gunflashPos -= CVector(0.03f * offset.x, 0.03f * offset.y, 0.0f);
	CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.02f);

	CVector gunsmokePos = start;
	float rnd = CGeneral::GetRandomNumberInRange(0.05f, 0.25f);
	CParticle::AddParticle(PARTICLE_GUNSMOKE2, gunsmokePos, CVector(ahead.x * rnd, ahead.y * rnd, 0.0f));
}

void
CWeapon::AddGunshell(CEntity *shooter, CVector const &source, CVector2D const &direction, float size)
{
	ASSERT(shooter!=nil);

	if ( shooter == nil)
		return;

	CVector dir(direction.x*0.05f, direction.y*0.05f, CGeneral::GetRandomNumberInRange(0.02f, 0.08f));

	static CVector prevEntityPosition(0.0f, 0.0f, 0.0f);
	CVector entityPosition = shooter->GetPosition();

	CVector diff = entityPosition - prevEntityPosition;

	if ( Abs(diff.x)+Abs(diff.y)+Abs(diff.z) > 1.5f )
	{
		prevEntityPosition = entityPosition;

		CParticle::AddParticle(PARTICLE_GUNSHELL_FIRST,
			source, dir, nil, size, CGeneral::GetRandomNumberInRange(-20.0f, 20.0f));
	}
	else
	{
		CParticle::AddParticle(PARTICLE_GUNSHELL,
			source, dir, nil, size, CGeneral::GetRandomNumberInRange(-20.0f, 20.0f));
	}
}


void
CWeapon::DoBulletImpact(CEntity *shooter, CEntity *victim,
		CVector *source, CVector *target, CColPoint *point, CVector2D ahead)
{
	ASSERT(shooter!=nil);
	ASSERT(source!=nil);
	ASSERT(target!=nil);
	ASSERT(point!=nil);

	CWeaponInfo *info = GetInfo();

	if ( victim )
	{
		if (shooter)
		{
			if (shooter && shooter->IsPed() && ((CPed*)shooter)->m_attachedTo == victim)
				return;

			if (shooter->IsPed() && !((CPed*)shooter)->IsPlayer())
			{
				CPed* shooterPed = (CPed*)shooter;
				CEntity* guyWePointGun = shooterPed->m_pPointGunAt;
				if (guyWePointGun)
				{
					if (victim != guyWePointGun)
					{
						float distWithAim = (guyWePointGun->GetPosition() - shooter->GetPosition()).Magnitude();
						float distWithBullet = (point->point - shooter->GetPosition()).Magnitude();
						if (distWithAim > 0.1f && distWithBullet > 0.1f)
						{
							// Normalize
							CVector aimDir = (guyWePointGun->GetPosition() - shooter->GetPosition()) * (1.0f / distWithAim);
							CVector bulletDir = (point->point - shooter->GetPosition()) * (1.0f / distWithBullet);

							float dotProd = DotProduct(aimDir, bulletDir);
							float aimAndBulletAngle;
							if (dotProd <= 0.35f)
								aimAndBulletAngle = PI;
							else
								aimAndBulletAngle = Acos(dotProd);

							if (aimAndBulletAngle <= DEGTORAD(45.0f) && (aimAndBulletAngle <= DEGTORAD(15.0f) || distWithBullet / distWithAim >= 0.75f) && distWithBullet / distWithAim >= 0.99f)
							{
								shooterPed->bObstacleShowedUpDuringKillObjective = false;
								shooterPed->m_shotTime = 0;
							}
							else
							{
								shooterPed->bObstacleShowedUpDuringKillObjective = true;
								shooterPed->m_shootTimer = 0;
								shooterPed->m_shotTime = CTimer::GetTimeInMilliseconds();
								if (distWithAim < 10.0f)
									shooterPed->SetAttackTimer(1500);
								else
									shooterPed->SetAttackTimer(3000);
							}
						}
					}
				}
			}
		}
		CGlass::WasGlassHitByBullet(victim, point->point);

		CVector traceTarget = point->point;
		CBulletTraces::AddTrace(source, &traceTarget, m_eWeaponType, shooter);

		if (victim->IsPed() && shooter->IsVehicle() && ((CVehicle*)shooter)->pDriver)
			shooter = ((CVehicle*)shooter)->pDriver;

		if ( victim->IsPed() && shooter->IsPed() &&
			(((CPed*)shooter)->m_nPedType != ((CPed*)victim)->m_nPedType || ((CPed*)shooter)->m_nPedType == PEDTYPE_PLAYER2 ||
				!((CPed*)shooter)->IsGangMember() && ((CPed*)shooter)->m_nPedType != PEDTYPE_COP))
		{
			CPed *victimPed = (CPed *)victim;
			if ( !victimPed->DyingOrDead() && victim != shooter )
			{
				CVector pos = victimPed->GetPosition();

				CVector2D posOffset(source->x-pos.x, source->y-pos.y);
				int32 localDir = victimPed->GetLocalDirection(posOffset);

				victimPed->ReactToAttack(shooter);

				if ( !victimPed->IsPedInControl() || victimPed->bIsDucking )
				{
					victimPed->InflictDamage(shooter, m_eWeaponType, info->m_nDamage, (ePedPieceTypes)point->pieceB, localDir);
				}
				else
				{
					if ( victimPed->bCanBeShotInVehicle && (IsShotgun(m_eWeaponType) ||
						(!victimPed->IsPlayer() && (m_eWeaponType == WEAPONTYPE_HELICANNON || m_eWeaponType == WEAPONTYPE_M60 || m_eWeaponType == WEAPONTYPE_PYTHON
#ifdef EX_WEAPON_DEAGLE // DoBulletImpact - Apply force
						|| m_eWeaponType == WEAPONTYPE_DEAGLE
#endif	
						))))
					{
						posOffset.Normalise();
						victimPed->bIsStanding = false;

						victimPed->ApplyMoveForce(posOffset.x*-5.0f, posOffset.y*-5.0f, 5.0f);
						victimPed->SetFall(1500, AnimationId(ANIM_STD_HIGHIMPACT_FRONT + localDir), false);

						victimPed->InflictDamage(shooter, m_eWeaponType, info->m_nDamage, (ePedPieceTypes)point->pieceB, localDir);
					}
					else
					{
						if ( victimPed->IsPlayer() )
						{
							CPlayerPed *victimPlayer = (CPlayerPed *)victimPed;
#ifdef FIRST_PERSON
							if ( victimPlayer->m_nHitAnimDelayTimer < CTimer::GetTimeInMilliseconds() && victimPed->m_nPedState != PED_DRIVING &&
								 TheCamera.Cams[TheCamera.ActiveCam].Mode != CCam::MODE_REAL_1ST_PERSON)
#else
							if ( victimPlayer->m_nHitAnimDelayTimer < CTimer::GetTimeInMilliseconds() && victimPed->m_nPedState != PED_DRIVING )
#endif
							{
								victimPed->ClearAttackByRemovingAnim();

								CAnimBlendAssociation *asoc = CAnimManager::AddAnimation(victimPed->GetClump(), ASSOCGRP_STD, AnimationId(ANIM_STD_HITBYGUN_FRONT + localDir));
								ASSERT(asoc!=nil);

								asoc->blendAmount = 0.0f;
								asoc->blendDelta  = 8.0f;

								if ( m_eWeaponType == WEAPONTYPE_M4 
#ifdef EX_WEAPON_AK47 // ANIM_STD_HITBYGUN_FRONT
									|| m_eWeaponType == WEAPONTYPE_AK47
#endif
#ifdef EX_WEAPON_M16 // ANIM_STD_HITBYGUN_FRONT
									|| m_eWeaponType == WEAPONTYPE_M16
#endif
#ifdef EX_WEAPON_STEYR // ANIM_STD_HITBYGUN_FRONT
									|| m_eWeaponType == WEAPONTYPE_STEYR
#endif
									)
									victimPlayer->m_nHitAnimDelayTimer = CTimer::GetTimeInMilliseconds() + 2500;
								else
									victimPlayer->m_nHitAnimDelayTimer = CTimer::GetTimeInMilliseconds() + 1000;
							}
						}
						else
						{
							victimPed->ClearAttackByRemovingAnim();

							CAnimBlendAssociation *asoc = CAnimManager::AddAnimation(victimPed->GetClump(), ASSOCGRP_STD, AnimationId(ANIM_STD_HITBYGUN_FRONT + localDir));
							ASSERT(asoc!=nil);

							asoc->blendAmount = 0.0f;
							asoc->blendDelta  = 8.0f;
						}

						victimPed->InflictDamage(shooter, m_eWeaponType, info->m_nDamage, (ePedPieceTypes)point->pieceB, localDir);
					}
				}

				if ( victimPed->m_nPedType == PEDTYPE_COP )
					CEventList::RegisterEvent(EVENT_SHOOT_COP, EVENT_ENTITY_PED, victim, (CPed*)shooter, 10000);
				else
					CEventList::RegisterEvent(EVENT_SHOOT_PED, EVENT_ENTITY_PED, victim, (CPed*)shooter, 10000);

				if ( CGame::nastyGame )
				{
					uint8 bloodAmount = 8;
					if ( IsShotgun(m_eWeaponType) || m_eWeaponType == WEAPONTYPE_HELICANNON )
						bloodAmount = 32;

					CVector dir = (point->point - victim->GetPosition()) * 0.01f;
					dir.z = 0.01f;

					if ( victimPed->GetIsOnScreen() )
					{
						for ( uint8 i = 0; i < bloodAmount; i++ )
							CParticle::AddParticle(PARTICLE_BLOOD_SMALL, point->point, dir);
					}

					if (m_eWeaponType == WEAPONTYPE_MINIGUN)
					{
						CParticle::AddParticle(PARTICLE_TEST, point->point, CVector(0.f, 0.f, 0.f), nil, 0.f, 0, 0, 0, 0);
						CParticle::AddParticle(PARTICLE_TEST, point->point + CVector(0.2f, -0.2f, 0.f), CVector(0.f, 0.f, 0.f), nil, 0.f, 0, 0, 0, 0);
						CParticle::AddParticle(PARTICLE_TEST, point->point + CVector(-0.2f, 0.2f, 0.f), CVector(0.f, 0.f, 0.f), nil, 0.f, 0, 0, 0, 0);
					}
				}
			}
			else
			{
				if ( CGame::nastyGame )
				{
					CVector dir = (point->point - victim->GetPosition()) * 0.01f;
					dir.z = 0.01f;

					if ( victim->GetIsOnScreen() )
					{
						for ( int32 i = 0; i < 8; i++ )
							CParticle::AddParticle(PARTICLE_BLOOD_SMALL, point->point + CVector(0.0f, 0.0f, 0.15f), dir);
					}

					if ( victimPed->Dead() )
					{
						CAnimBlendAssociation *asoc = nullptr;
						if ( RpAnimBlendClumpGetFirstAssociation(victimPed->GetClump(), ASSOC_FRONTAL) )
							asoc = CAnimManager::BlendAnimation(victimPed->GetClump(), ASSOCGRP_STD, ANIM_STD_HIT_FLOOR_FRONT, 8.0f);
#ifdef EX_PED_ANIMS_IN_CAR
						else if ( !victimPed->InVehicle() )
#else
						else
#endif
							asoc = CAnimManager::BlendAnimation(victimPed->GetClump(), ASSOCGRP_STD, ANIM_STD_HIT_FLOOR,   8.0f);

						if ( asoc )
						{
							asoc->SetCurrentTime(0.0f);
							asoc->flags |= ASSOC_RUNNING;
							asoc->flags &= ~ASSOC_FADEOUTWHENDONE;
						}
					}
				}
			}
		}
		else
		{
			switch ( victim->GetType() )
			{
				case ENTITY_TYPE_BUILDING:
				{
					for ( int32 i = 0; i < 16; i++ )
						CParticle::AddParticle(PARTICLE_SPARK, point->point, point->normal*0.05f);

#ifndef FIX_BUGS
					CVector dist = point->point - (*source);
					float distMagnitude = dist.Magnitude();
					CVector smokePos = point->point - Max(distMagnitude / 10.0f, 0.2f) * dist / distMagnitude;
#else
				    CVector smokePos = point->point;
#endif // !FIX_BUGS

					smokePos.x += CGeneral::GetRandomNumberInRange(-0.2f, 0.2f);
					smokePos.y += CGeneral::GetRandomNumberInRange(-0.2f, 0.2f);
					smokePos.z += CGeneral::GetRandomNumberInRange(-0.2f, 0.2f);

					CParticle::AddParticle(PARTICLE_BULLETHIT_SMOKE, smokePos, CVector(0.0f, 0.0f, 0.0f));

					break;
				}
				case ENTITY_TYPE_VEHICLE:
				{
					if (point->pieceB >= CAR_PIECE_WHEEL_LF && point->pieceB <= CAR_PIECE_WHEEL_RR) {
						((CVehicle*)victim)->BurstTyre(point->pieceB, true);

						for (int32 i = 0; i < 4; i++)
							CParticle::AddParticle(PARTICLE_BULLETHIT_SMOKE, point->point, point->normal * 0.05f);
					}
					else
					{
						((CVehicle*)victim)->InflictDamage(shooter, m_eWeaponType, info->m_nDamage);

						for (int32 i = 0; i < 16; i++)
							CParticle::AddParticle(PARTICLE_SPARK, point->point, point->normal * 0.05f);

#ifndef FIX_BUGS
						CVector dist = point->point - (*source);
						CVector offset = dist - Max(0.2f * dist.Magnitude(), 0.5f) * CVector(ahead.x, ahead.y, 0.0f);
						CVector smokePos = *source + offset;
#else
						CVector smokePos = point->point;
#endif

						CParticle::AddParticle(PARTICLE_BULLETHIT_SMOKE, smokePos, CVector(0.0f, 0.0f, 0.0f));
					}


					if ( shooter->IsPed() )
					{
						CPed *shooterPed = (CPed *)shooter;

						if ( shooterPed->bNotAllowedToDuck )
						{
							if ( shooterPed->bKindaStayInSamePlace && victim != shooterPed->m_pPointGunAt )
							{
								shooterPed->bKindaStayInSamePlace = false;
								shooterPed->m_duckAndCoverTimer   = CTimer::GetTimeInMilliseconds() + 15000;
							}
						}
					}

					break;
				}
				case ENTITY_TYPE_OBJECT:
				{
					for ( int32 i = 0; i < 8; i++ )
						CParticle::AddParticle(PARTICLE_SPARK, point->point, point->normal*0.05f);

					CObject *victimObject = (CObject *)victim;

					if ( !victimObject->bInfiniteMass && victimObject->m_fCollisionDamageMultiplier < 99.9f)
					{
						bool notStatic = !victimObject->GetIsStatic();
						if (notStatic && victimObject->m_fUprootLimit <= 0.0f)
						{
							victimObject->SetIsStatic(false);
							victimObject->AddToMovingList();
						}

						notStatic = !victimObject->GetIsStatic();
						if (!notStatic)
						{
							CVector moveForce = point->normal * -4.0f;
							victimObject->ApplyMoveForce(moveForce.x, moveForce.y, moveForce.z);
						}

#ifdef IMPROVED_TECH_PART // When a projectile is fired, the projectile explodes (FireInstantHit)
#ifdef EX_WEAPON_RCGRENADE // When a projectile is fired, the projectile explodes (FireInstantHit)
						if (victimObject->m_modelIndex == MI_GRENADE || victimObject->m_modelIndex == MI_MOLOTOV || victimObject->m_modelIndex == MI_MISSILE ||
							victimObject->m_modelIndex == MI_RCGRENADE) {
#else
						if (victimObject->m_modelIndex == MI_GRENADE || victimObject->m_modelIndex == MI_MOLOTOV || victimObject->m_modelIndex == MI_MISSILE) {
#endif
							for (uint8 i = 0; i < NUM_PROJECTILES; i++) {
								if (!CProjectileInfo::GetProjectileInfo(i)->ms_apProjectile[i])
									continue;

								if (CProjectileInfo::GetProjectileInfo(i)->ms_apProjectile[i] != (CProjectile*)victimObject)
									continue;

								if (gaProjectileInfo[i].m_eWeaponType == WEAPONTYPE_DETONATOR_GRENADE) {
									CProjectileInfo::RemoveDetonatorProjectile(i);
								} else {
									gaProjectileInfo[i].m_nExplosionTime = CTimer::GetTimeInMilliseconds();
								}
							}
						}
#endif
					} else if (victimObject->m_nCollisionDamageEffect >= DAMAGE_EFFECT_SMASH_COMPLETELY) {
						victimObject->ObjectDamage(50.f);
					}

					break;
				}
				default: break;
			}
		}

		switch ( victim->GetType() )
		{
			case ENTITY_TYPE_BUILDING:
			{
				PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_GROUND_1, point->point);
				break;
			}
			case ENTITY_TYPE_VEHICLE:
			{
				CStats::BulletsThatHit++;
				DMAudio.PlayOneShot(((CPhysical*)victim)->m_audioEntityId, SOUND_WEAPON_HIT_VEHICLE, 1.0f);
				break;
			}
			case ENTITY_TYPE_PED:
			{
				CStats::BulletsThatHit++;
				DMAudio.PlayOneShot(((CPhysical*)victim)->m_audioEntityId, SOUND_WEAPON_HIT_PED, 1.0f);
				((CPed*)victim)->Say(SOUND_PED_BULLET_HIT);
				break;
			}
			case ENTITY_TYPE_OBJECT:
			{
				CStats::BulletsThatHit++;
				PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_GROUND_2, point->point);
				break;
			}
			case ENTITY_TYPE_DUMMY:
			{
				PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_GROUND_3, point->point);
				break;
			}
			default: break;
		}
	}
	else
		CBulletTraces::AddTrace(source, target, m_eWeaponType, shooter);

	if ( shooter == FindPlayerPed() )
#ifdef IMPROVED_MENU_AND_INPUT
		CPad::GetPad(0)->StartShake_Distance(100, 0, 128, FindPlayerPed()->GetPosition().x, FindPlayerPed()->GetPosition().y, FindPlayerPed()->GetPosition().z);
#else
		CPad::GetPad(0)->StartShake_Distance(240, 128, FindPlayerPed()->GetPosition().x, FindPlayerPed()->GetPosition().y, FindPlayerPed()->GetPosition().z);
#endif

	BlowUpExplosiveThings(victim);
}


bool
CWeapon::FireShotgun(CEntity *shooter, CVector *fireSource)
{
	ASSERT(shooter!=nil);
	ASSERT(fireSource!=nil);

	CWeaponInfo *info = GetInfo();

	float heading = RADTODEG(shooter->GetForward().Heading());
	float angle   = DEGTORAD(heading);

	CVector2D rotOffset(-Sin(angle), Cos(angle));
	rotOffset.Normalise();

	CVector gunflashPos = *fireSource;
	gunflashPos += CVector(rotOffset.x*0.1f, rotOffset.y*0.1f, 0.0f);
	CParticle::AddParticle(PARTICLE_GUNFLASH, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.0f);
	gunflashPos += CVector(rotOffset.x*0.1f, rotOffset.y*0.1f, 0.0f);
	CParticle::AddParticle(PARTICLE_GUNFLASH, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.15f);
	gunflashPos += CVector(rotOffset.x*0.1f, rotOffset.y*0.1f, 0.0f);
	CParticle::AddParticle(PARTICLE_GUNFLASH, gunflashPos, CVector(0.0f, 0.0f, 0.0f), nil, 0.2f);
	CParticle::AddParticle(PARTICLE_GUNFLASH, *fireSource, CVector(0.0f, 0.0f, 0.0f), nil, 0.0f);

	CVector gunsmokePos = *fireSource;
	CParticle::AddParticle(PARTICLE_GUNSMOKE2, gunsmokePos, CVector(rotOffset.x*0.10f, rotOffset.y*0.10f, 0.0f), nil, 0.1f);
	CParticle::AddParticle(PARTICLE_GUNSMOKE2, gunsmokePos, CVector(rotOffset.x*0.15f, rotOffset.y*0.15f, 0.0f), nil, 0.1f);
	CParticle::AddParticle(PARTICLE_GUNSMOKE2, gunsmokePos, CVector(rotOffset.x*0.20f, rotOffset.y*0.20f, 0.0f), nil, 0.1f);
	CParticle::AddParticle(PARTICLE_GUNSMOKE2, gunsmokePos, CVector(rotOffset.x*0.25f, rotOffset.y*0.25f, 0.0f), nil, 0.1f);

	CEventList::RegisterEvent(EVENT_GUNSHOT, EVENT_ENTITY_PED, shooter, (CPed*)shooter, 1000);

	CPointLights::AddLight(CPointLights::LIGHT_POINT, *fireSource, CVector(0.0, 0.0, 0.0), 5.0f,
							1.0f, 0.8f, 0.0f, CPointLights::FOG_NONE, false);

	float shooterAngle;

	if ( shooter->IsPed() && ((CPed*)shooter)->m_pPointGunAt != nil )
	{
		CEntity *threatAttack = ((CPed*)shooter)->m_pPointGunAt;
		shooterAngle = CGeneral::GetAngleBetweenPoints(threatAttack->GetPosition().x, threatAttack->GetPosition().y,
														(*fireSource).x, (*fireSource).y);
	}
	else
		shooterAngle = RADTODEG(shooter->GetForward().Heading());

	int shootsAtOnce;
	int checkObstacleOnShootNo;
	float angleRange;
	switch (m_eWeaponType) {
		case WEAPONTYPE_SHOTGUN:
			angleRange = DEGTORAD(9.0f);
			checkObstacleOnShootNo = 1;
			shootsAtOnce = 3;
			break;
		case WEAPONTYPE_SPAS12_SHOTGUN:
			angleRange = DEGTORAD(6.0f);
			checkObstacleOnShootNo = 1;
			shootsAtOnce = 3;
			break;
		case WEAPONTYPE_STUBBY_SHOTGUN:
			angleRange = DEGTORAD(18.0f);
			checkObstacleOnShootNo = 2;
			shootsAtOnce = 5;
			break;
#ifdef EX_WEAPON_SHOTGUN2 // FireShotgun
		case WEAPONTYPE_SHOTGUN2:
			angleRange = DEGTORAD(18.0f);
			checkObstacleOnShootNo = 2;
			shootsAtOnce = 5;
			break;
#endif
		default:
			break;
	}
	bool statUpdated = false;
	float halfAngleRange = angleRange / 2.f;
	float angleBetweenTwoShot = angleRange / (shootsAtOnce - 1.f);

	for ( int32 i = 0; i < shootsAtOnce; i++ )
	{
		float shootAngle = DEGTORAD(RADTODEG(halfAngleRange - angleBetweenTwoShot * i) + shooterAngle);
		CVector2D shootRot(-Sin(shootAngle), Cos(shootAngle));
		shootRot.Normalise();

		CVector source, target;
		CColPoint point;
		CEntity *victim;

#if defined IMPROVED_MENU_AND_INPUT && defined FIRING_AND_AIMING
		if ( shooter == FindPlayerPed() && !FindPlayerPed()->bIsAutoAiming)
#else
		if ( shooter == FindPlayerPed() && TheCamera.Cams[0].Using3rdPersonMouseCam() )
#endif
		{
			CVector Left;
#ifdef FREE_CAM
			if (CCamera::bFreeCam) {
				CPlayerPed* shooterPed = (CPlayerPed*)shooter;
				Find3rdPersonCamTargetVectorFromCachedVectors(1.0f, *fireSource, source, target, shooterPed->m_cachedCamSource, shooterPed->m_cachedCamFront, shooterPed->m_cachedCamUp);
				Left = CrossProduct(shooterPed->m_cachedCamFront, shooterPed->m_cachedCamUp);
			}
			else
#endif
			{
				TheCamera.Find3rdPersonCamTargetVector(1.0f, *fireSource, source, target);
				Left = CrossProduct(TheCamera.Cams[TheCamera.ActiveCam].Front, TheCamera.Cams[TheCamera.ActiveCam].Up);
			}

			float f = (i - (shootsAtOnce / 2)) * angleBetweenTwoShot;
			target  = f * Left + target - source;
			target *= info->m_fRange;
			target += source;
			CWorld::bIncludeCarTyres = true;
			CWorld::bIncludeBikers = true;
			CWorld::bIncludeDeadPeds = true;
			ProcessLineOfSight(source, target, point, victim, m_eWeaponType, shooter, true, true, true, true, true, false, false);
			CWorld::bIncludeDeadPeds = false;
			CWorld::bIncludeCarTyres = false;
		}
		else
		{
			target = *fireSource;
			target.x += shootRot.x * info->m_fRange;
			target.y += shootRot.y * info->m_fRange;

			if ( shooter->IsPed() )
			{
				CPed *shooterPed = (CPed *)shooter;

				if ( shooterPed->m_pPointGunAt == nil )
					DoDoomAiming(shooter, fireSource, &target);
				else
				{
					CVector pos;
					if (shooterPed->m_pPointGunAt->IsPed()) {
						((CPed*)shooterPed->m_pPointGunAt)->m_pedIK.GetComponentPosition(pos, PED_MID);
					} else {
						pos = ((CPed*)shooterPed->m_pPointGunAt)->GetPosition();
					}

					float distToTarget = (pos - (*fireSource)).Magnitude2D();
					target.z += info->m_fRange / distToTarget * (pos.z - target.z);
				}
			}
			if (shooter == FindPlayerPed())
				CWorld::bIncludeDeadPeds = true;

			CWorld::bIncludeBikers = true;
			ProcessLineOfSight(*fireSource, target, point, victim, m_eWeaponType, shooter, true, true, true, true, true, false, false);
			CWorld::bIncludeDeadPeds = false;
		}
		CWorld::bIncludeBikers = false;

		if ( victim )
		{
			CGlass::WasGlassHitByBullet(victim, point.point);
			CWeapon::BlowUpExplosiveThings(victim);
			if (i == checkObstacleOnShootNo)
			{
				if (shooter)
				{
					if (shooter->IsPed() && !((CPed*)shooter)->IsPlayer())
					{
						CPed *shooterPed = (CPed*)shooter;
						CEntity *guyWePointGun = shooterPed->m_pPointGunAt;
						if (guyWePointGun)
						{
							if (victim != guyWePointGun)
							{
								float distWithAim = (guyWePointGun->GetPosition() - shooter->GetPosition()).Magnitude();
								float distWithBullet = (point.point - shooter->GetPosition()).Magnitude();
								if (distWithAim > 0.1f && distWithBullet > 0.1f)
								{
									// Normalize
									CVector aimDir = (guyWePointGun->GetPosition() - shooter->GetPosition()) * (1.0f / distWithAim);
									CVector bulletDir = (point.point - shooter->GetPosition()) * (1.0f / distWithBullet);

									float dotProd = DotProduct(aimDir, bulletDir);
									float aimAndBulletAngle;
									if (dotProd <= 0.35f)
										aimAndBulletAngle = PI;
									else
										aimAndBulletAngle = Acos(dotProd);

									if (aimAndBulletAngle <= DEGTORAD(45.0f) && (aimAndBulletAngle <= DEGTORAD(15.0f) || distWithBullet / distWithAim >= 0.75f) && distWithBullet / distWithAim >= 0.99f)
									{
										shooterPed->bObstacleShowedUpDuringKillObjective = false;
										shooterPed->m_shotTime = 0;
									}
									else
									{
										shooterPed->bObstacleShowedUpDuringKillObjective = true;
										shooterPed->m_shootTimer = 0;
										shooterPed->m_shotTime = CTimer::GetTimeInMilliseconds();
										if (distWithAim < 10.0f)
											shooterPed->SetAttackTimer(1500);
										else
											shooterPed->SetAttackTimer(3000);
									}
								}
							}
						}
					}
				}
			}
			CBulletTraces::AddTrace(fireSource, &point.point, m_eWeaponType, shooter);

			if ( victim->IsPed() )
			{
				CPed *victimPed = (CPed *)victim;
				if ( !victimPed->DyingOrDead() && victim != shooter )
				{
					bool cantStandup = true;

					CVector pos = victimPed->GetPosition();

					CVector2D posOffset((*fireSource).x-pos.x, (*fireSource).y-pos.y);
					int32 localDir = victimPed->GetLocalDirection(posOffset);

					victimPed->ReactToAttack(FindPlayerPed());

					posOffset.Normalise();

					if ( victimPed->m_getUpTimer > (CTimer::GetTimeInMilliseconds() - 3000) || 
						!victimPed->bCanBeShotInVehicle)
						cantStandup = false;

					if ( victimPed->bIsStanding && cantStandup )
					{
						victimPed->bIsStanding = false;

						victimPed->ApplyMoveForce(posOffset.x*-6.0f, posOffset.y*-6.0f, 5.0f);
					}
					else
						victimPed->ApplyMoveForce(posOffset.x*-2.0f, posOffset.y*-2.0f, 0.0f);

					if ( cantStandup )
						victimPed->SetFall(1500, AnimationId(ANIM_STD_HIGHIMPACT_FRONT + localDir), false);

					victimPed->InflictDamage(shooter, m_eWeaponType, info->m_nDamage, (ePedPieceTypes)point.pieceB, localDir);

					if ( victimPed->m_nPedType == PEDTYPE_COP )
						CEventList::RegisterEvent(EVENT_SHOOT_COP, EVENT_ENTITY_PED, victim, (CPed*)shooter, 10000);
					else
						CEventList::RegisterEvent(EVENT_SHOOT_PED, EVENT_ENTITY_PED, victim, (CPed*)shooter, 10000);

					if ( CGame::nastyGame )
					{
						uint8 bloodAmount = 8;
						if ( IsShotgun(m_eWeaponType) )
							bloodAmount = 32;

						CVector dir = (point.point - victim->GetPosition()) * 0.01f;
						dir.z = 0.01f;

						if ( victimPed->GetIsOnScreen() )
						{
							for ( uint8 i = 0; i < bloodAmount; i++ )
								CParticle::AddParticle(PARTICLE_BLOOD_SMALL, point.point, dir);
						}
					}
				} else {
					if (CGame::nastyGame)
					{
						CVector dir = (point.point - victim->GetPosition()) * 0.01f;
						dir.z = 0.01f;

						if (victimPed->GetIsOnScreen())
						{
							for (uint8 i = 0; i < 8; i++)
								CParticle::AddParticle(PARTICLE_BLOOD_SMALL, point.point + CVector(0.0f, 0.0f, 0.15f), dir);
						}
						if (victimPed->Dead())
						{
							CAnimBlendAssociation *hitAssoc;
							if (RpAnimBlendClumpGetFirstAssociation(victimPed->GetClump(), ASSOC_FRONTAL))
							{
								hitAssoc = CAnimManager::BlendAnimation(victimPed->GetClump(), ASSOCGRP_STD, ANIM_STD_HIT_FLOOR_FRONT, 8.0f);
							}
							else
							{
								hitAssoc = CAnimManager::BlendAnimation(victimPed->GetClump(), ASSOCGRP_STD, ANIM_STD_HIT_FLOOR, 8.0f);
							}
							if (hitAssoc)
							{
								hitAssoc->SetCurrentTime(0.0f);
								hitAssoc->SetRun();
								hitAssoc->flags &= ~ASSOC_DELETEFADEDOUT;
							}
						}
					}
				}
			}
			else
			{
				switch ( victim->GetType() )
				{
					case ENTITY_TYPE_VEHICLE:
					{
#ifdef IMPROVED_VEHICLES_2 // check for shooting vehicle lights and petrolcap
						CheckForShootingVehicleLights(victim, point);
						CheckForShootingVehiclePetrolCap(shooter, victim, point);
#endif

						if (point.pieceB >= CAR_PIECE_WHEEL_LF && point.pieceB <= CAR_PIECE_WHEEL_RR) {
							((CVehicle*)victim)->BurstTyre(point.pieceB, true);

							for (int32 i = 0; i < 4; i++)
								CParticle::AddParticle(PARTICLE_BULLETHIT_SMOKE, point.point, point.normal * 0.05f);
						}
						else
						{
							((CVehicle*)victim)->InflictDamage(shooter, m_eWeaponType, info->m_nDamage);

							for (int32 i = 0; i < 16; i++)
								CParticle::AddParticle(PARTICLE_SPARK, point.point, point.normal * 0.05f);

#ifndef FIX_BUGS
							CVector dist = point.point - (*fireSource);
							CVector offset = dist - Max(0.2f * dist.Magnitude(), 2.0f) * CVector(shootRot.x, shootRot.y, 0.0f);
							CVector smokePos = *fireSource + offset;
#else
							CVector smokePos = point.point;
#endif

							CParticle::AddParticle(PARTICLE_BULLETHIT_SMOKE, smokePos, CVector(0.0f, 0.0f, 0.0f));
						}
						break;
					}

					case ENTITY_TYPE_BUILDING:
					case ENTITY_TYPE_OBJECT:
					{
						for ( int32 i = 0; i < 16; i++ )
							CParticle::AddParticle(PARTICLE_SPARK, point.point, point.normal*0.05f);

#ifndef FIX_BUGS
						CVector dist = point.point - (*fireSource);
						CVector offset = dist - Max(0.2f*dist.Magnitude(), 2.0f) * CVector(shootRot.x, shootRot.y, 0.0f);
					    CVector smokePos = *fireSource + offset;
#else
					    CVector smokePos = point.point;
#endif

						smokePos.x += CGeneral::GetRandomNumberInRange(-0.2f, 0.2f);
						smokePos.y += CGeneral::GetRandomNumberInRange(-0.2f, 0.2f);
						smokePos.z += CGeneral::GetRandomNumberInRange(-0.2f, 0.2f);

						CParticle::AddParticle(PARTICLE_BULLETHIT_SMOKE, smokePos, CVector(0.0f, 0.0f, 0.0f));

						if ( victim->IsObject() )
						{
							CObject *victimObject = (CObject *)victim;

							if ( !victimObject->bInfiniteMass )
							{
								bool notStatic = !victimObject->GetIsStatic();
								if ( notStatic && victimObject->m_fUprootLimit <= 0.0f )
								{
									victimObject->SetIsStatic(false);
									victimObject->AddToMovingList();
								}

								notStatic = !victimObject->GetIsStatic();
								if ( !notStatic )
								{
									CVector moveForce = point.normal*-5.0f;
									victimObject->ApplyMoveForce(moveForce.x, moveForce.y, moveForce.z);
								}

#ifdef IMPROVED_TECH_PART // When a projectile is fired, the projectile explodes (shotgun)
#ifdef EX_WEAPON_RCGRENADE // When a projectile is fired, the projectile explodes (shotgun)
								if (victimObject->m_modelIndex == MI_GRENADE || victimObject->m_modelIndex == MI_MOLOTOV || victimObject->m_modelIndex == MI_MISSILE ||
									victimObject->m_modelIndex == MI_RCGRENADE) {
#else
								if (victimObject->m_modelIndex == MI_GRENADE || victimObject->m_modelIndex == MI_MOLOTOV || victimObject->m_modelIndex == MI_MISSILE) {
#endif
									for (uint8 i = 0; i < NUM_PROJECTILES; i++) {
										if (!CProjectileInfo::GetProjectileInfo(i)->ms_apProjectile[i])
											continue;

										if (CProjectileInfo::GetProjectileInfo(i)->ms_apProjectile[i] != (CProjectile*)victimObject)
											continue;

										if (gaProjectileInfo[i].m_eWeaponType == WEAPONTYPE_DETONATOR_GRENADE) {
											CProjectileInfo::RemoveDetonatorProjectile(i);
										} else {
											gaProjectileInfo[i].m_nExplosionTime = CTimer::GetTimeInMilliseconds();
										}
									}
								}
#endif
							}
						}

						break;
					}
					default: break;
				}
			}

			switch ( victim->GetType() )
			{
				case ENTITY_TYPE_BUILDING:
				{
					PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_GROUND_1, point.point);
					break;
				}
				case ENTITY_TYPE_VEHICLE:
				{
					if (!statUpdated) {
						CStats::BulletsThatHit++;
						statUpdated = true;
					}
					DMAudio.PlayOneShot(((CPhysical*)victim)->m_audioEntityId, SOUND_WEAPON_HIT_VEHICLE, 1.0f);
					break;
				}
				case ENTITY_TYPE_PED:
				{
					if (!statUpdated) {
						CStats::BulletsThatHit++;
						statUpdated = true;
					}
					DMAudio.PlayOneShot(((CPhysical*)victim)->m_audioEntityId, SOUND_WEAPON_HIT_PED, 1.0f);
					((CPed*)victim)->Say(SOUND_PED_BULLET_HIT);
					break;
				}
				case ENTITY_TYPE_OBJECT:
				{
					if (!statUpdated) {
						CStats::BulletsThatHit++;
						statUpdated = true;
					}
					PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_GROUND_2, point.point);
					break;
				}
				case ENTITY_TYPE_DUMMY:
				{
					PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_GROUND_3, point.point);
					break;
				}
				default: break;
			}
		}
		else
		{
			CVector traceTarget = *fireSource;
			traceTarget += (target - (*fireSource)) * Min(info->m_fRange, 30.0f) / info->m_fRange;
			CBulletTraces::AddTrace(fireSource, &traceTarget, m_eWeaponType, shooter);
		}
	}

	if ( shooter == FindPlayerPed() ) {
#ifdef FEATURES_INI // RecoilWhenFiring
		if (bRecoilWhenFiring) {
			TheCamera.Cams[TheCamera.ActiveCam].Alpha += CGeneral::GetRandomNumberInRange(0.005f, 0.01f);
			TheCamera.Cams[TheCamera.ActiveCam].Beta += CGeneral::GetRandomNumberInRange(-0.01f, 0.01f);
		}
#endif

#ifdef IMPROVED_MENU_AND_INPUT
		CPad::GetPad(0)->StartShake_Distance(240, 0, 128, FindPlayerPed()->GetPosition().x, FindPlayerPed()->GetPosition().y, FindPlayerPed()->GetPosition().z);
#else
		CPad::GetPad(0)->StartShake_Distance(240, 128, FindPlayerPed()->GetPosition().x, FindPlayerPed()->GetPosition().y, FindPlayerPed()->GetPosition().z);
#endif
	}

	return true;
}

bool
CWeapon::FireProjectile(CEntity *shooter, CVector *fireSource, float power)
{
	ASSERT(shooter!=nil);
	ASSERT(fireSource!=nil);

	CVector source, target;
	eWeaponType projectileType = m_eWeaponType;

	if ( m_eWeaponType == WEAPONTYPE_ROCKETLAUNCHER )
	{
		source = *fireSource;
		projectileType = WEAPONTYPE_ROCKET;

		if ( shooter->IsPed() && ((CPed*)shooter)->IsPlayer() )
		{
#ifndef FIRING_AND_AIMING
			int16 mode = TheCamera.Cams[TheCamera.ActiveCam].Mode;
			if (!( mode == CCam::MODE_M16_1STPERSON
				|| mode == CCam::MODE_SNIPER
				|| mode == CCam::MODE_ROCKETLAUNCHER
				|| mode == CCam::MODE_M16_1STPERSON_RUNABOUT
				|| mode == CCam::MODE_SNIPER_RUNABOUT
				|| mode == CCam::MODE_ROCKETLAUNCHER_RUNABOUT) )
			{
				return false;
			}
#endif

			*fireSource += TheCamera.Cams[TheCamera.ActiveCam].Front;
		}
		else
			*fireSource += shooter->GetForward();

		target = *fireSource;
	}
	else
	{
		float dot = DotProduct(*fireSource-shooter->GetPosition(), shooter->GetForward());

		if ( dot < 0.3f )
			*fireSource += (0.3f-dot) * shooter->GetForward();

		target = *fireSource;

		if ( target.z - shooter->GetPosition().z > 0.0f )
			target += 0.6f*shooter->GetForward();

		source = *fireSource - shooter->GetPosition();

		source = *fireSource - DotProduct(source, shooter->GetForward()) * shooter->GetForward();
	}

	if ( !CWorld::GetIsLineOfSightClear(source, target, true, true, false, true, false, false, false) )
	{
		if ( m_eWeaponType != WEAPONTYPE_GRENADE )
			CProjectileInfo::RemoveNotAdd(shooter, projectileType, *fireSource);
		else
		{
			if ( shooter->IsPed() )
			{
				source    = shooter->GetPosition() - shooter->GetForward();
				source.z -= 0.4f;

				if ( !CWorld::TestSphereAgainstWorld(source, 0.5f, nil, false, false, true, false, false, false) )
					CProjectileInfo::AddProjectile(shooter, WEAPONTYPE_GRENADE, source, 0.0f);
				else
					CProjectileInfo::RemoveNotAdd(shooter, WEAPONTYPE_GRENADE, *fireSource);
			}
		}
	}
	else
		CProjectileInfo::AddProjectile(shooter, projectileType, *fireSource, power);
	
			
	CWorld::pIgnoreEntity = nil;
		
	if ( shooter->IsPed() )
		CEventList::RegisterEvent(EVENT_GUNSHOT, EVENT_ENTITY_PED,     shooter, (CPed *)shooter,                1000);
	else if ( shooter->IsVehicle() && ((CVehicle*)shooter)->pDriver )
		CEventList::RegisterEvent(EVENT_GUNSHOT, EVENT_ENTITY_VEHICLE, shooter, ((CVehicle*)shooter)->pDriver,  1000);

	return true;
}

void
CWeapon::GenerateFlameThrowerParticles(CVector pos, CVector dir)
{
	dir *= 0.7f;
	CParticle::AddParticle(PARTICLE_FIREBALL, pos, dir);

	dir *= 0.7f;
	CParticle::AddParticle(PARTICLE_FIREBALL, pos, dir);

	dir *= 0.7f;
	CParticle::AddParticle(PARTICLE_FIREBALL, pos, dir);

	dir *= 0.7f;
	CParticle::AddParticle(PARTICLE_FIREBALL, pos, dir);

	dir *= 0.7f;
	CParticle::AddParticle(PARTICLE_FIREBALL, pos, dir);
}

bool
CWeapon::FireAreaEffect(CEntity *shooter, CVector *fireSource)
{
	ASSERT(shooter!=nil);
	ASSERT(fireSource!=nil);

	CWeaponInfo *info = GetInfo();

	float heading = RADTODEG(shooter->GetForward().Heading());

	CVector source;
	CVector target;
	CVector dir;

	if ( shooter == FindPlayerPed() && TheCamera.Cams[0].Using3rdPersonMouseCam() )
	{
#ifdef FREE_CAM
		if (CCamera::bFreeCam) {
			CPlayerPed* shooterPed = (CPlayerPed*)shooter;
			Find3rdPersonCamTargetVectorFromCachedVectors(info->m_fRange, *fireSource, source, target, shooterPed->m_cachedCamSource, shooterPed->m_cachedCamFront, shooterPed->m_cachedCamUp);
		}
		else
#endif
		{
			TheCamera.Find3rdPersonCamTargetVector(info->m_fRange, *fireSource, source, target);
		}
		float norm = (1.0f / info->m_fRange);
		dir = (target - source) * norm;
	}
	else
	{
		float angle = DEGTORAD(heading);
		dir         = CVector(-Sin(angle)*0.5f, Cos(angle)*0.5f, 0.0f);
		target      = *fireSource + dir;
	}

	CShotInfo::AddShot(shooter, m_eWeaponType, *fireSource, target);
	CWeapon::GenerateFlameThrowerParticles(*fireSource, dir);
		
	if ( shooter == (CEntity *)FindPlayerPed() )
	{
		for ( int32 i = 0; i < FindPlayerPed()->m_numNearPeds; i++ )
		{
			if ( FindPlayerPed()->m_nearPeds[i]->CharCreatedBy == RANDOM_CHAR )
			{
				if ( FindPlayerPed()->m_nearPeds[i]->IsPedInControl() && FindPlayerPed()->m_nearPeds[i]->m_nPedState != PED_FLEE_ENTITY )
					FindPlayerPed()->m_nearPeds[i]->SetFlee(shooter, 10000);
			}
		}
	}

	return true;
}

bool
CWeapon::LaserScopeDot(CVector *pOutPos, float *pOutSize)
{
	CWeaponInfo *info = GetInfo();
	
	float range = info->m_fRange;
	
	CVector source, target;
	CEntity *foundEnt = nil;
	CColPoint foundCol;
	
	source = 0.5f * TheCamera.Cams[TheCamera.ActiveCam].Front + TheCamera.Cams[TheCamera.ActiveCam].Source;
	target = TheCamera.Cams[TheCamera.ActiveCam].Front;
	target.Normalise();
	target *= range;
	target += source;
	
	if ( CWorld::ProcessLineOfSight(source, target, foundCol, foundEnt, true, true, true, true, false, false, false) )
	{
		CVector pos = foundCol.point;
		float w, h;
		
		if ( CSprite::CalcScreenCoors(foundCol.point, &pos, &w, &h, true) )
		{
			*pOutPos = pos;
			*pOutSize = w * 0.05f;
			
			CCoronas::RegisterCorona((uintptr)this + 7, 128, 0, 0, 255, pos, 1.2f, 50.0f, CCoronas::TYPE_STAR, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
			
			return true;
		}
	}
		
	return false;
}

bool
CWeapon::FireSniper(CEntity *shooter)
{
	ASSERT(shooter!=nil);
	
#ifndef FIRING_AND_AIMING
	if ( (CEntity *)FindPlayerPed() == shooter )
	{
		int16 mode = TheCamera.Cams[TheCamera.ActiveCam].Mode;
		if (!( mode == CCam::MODE_M16_1STPERSON
			|| mode == CCam::MODE_SNIPER
			|| mode == CCam::MODE_CAMERA
			|| mode == CCam::MODE_ROCKETLAUNCHER
			|| mode == CCam::MODE_M16_1STPERSON_RUNABOUT
			|| mode == CCam::MODE_SNIPER_RUNABOUT
			|| mode == CCam::MODE_ROCKETLAUNCHER_RUNABOUT) )
		{
			return false;
		}
	}
#endif

#ifdef SECUROM
	if (sniperPirateCheck){
		// if not pirated game
		// sniperPirateCheck = 0;
	}
#endif

#ifndef FIX_BUGS
	CWeaponInfo *info = GetInfo(); //unused
#endif

	CCam *cam = &TheCamera.Cams[TheCamera.ActiveCam];
	ASSERT(cam!=nil);

	CVector source = cam->Source;
	CVector dir    = cam->Front;

#ifdef IMPROVED_TECH_PART // moving Moon
	CVector moonDirection = -CTimeCycle::GetSunDirection();
	if ( DotProduct(dir, CVector(moonDirection.x, moonDirection.y, moonDirection.z)) > 0.997f )
#else
	if ( DotProduct(dir, CVector(0.0f, -0.9894f, 0.145f)) > 0.997f )
#endif
		CCoronas::MoonSize = (CCoronas::MoonSize+1) & 7;

	dir.Normalise();
	dir *= 16.0f;

#ifdef SECUROM
	if (sniperPirateCheck) return true;
#endif

	CBulletInfo::AddBullet(shooter, m_eWeaponType, source, dir);

	if ( shooter == FindPlayerPed() )
	{
#ifdef FEATURES_INI // RecoilWhenFiring
		if (bRecoilWhenFiring) {
			TheCamera.Cams[TheCamera.ActiveCam].Alpha += CGeneral::GetRandomNumberInRange(0.001f, 0.005f);
			TheCamera.Cams[TheCamera.ActiveCam].Beta += CGeneral::GetRandomNumberInRange(-0.005f, 0.005f);
		}
#endif

#ifdef IMPROVED_MENU_AND_INPUT
		CPad::GetPad(0)->StartShake_Distance(240, 0, 128,
			FindPlayerPed()->GetPosition().x,
			FindPlayerPed()->GetPosition().y,
			FindPlayerPed()->GetPosition().z);
#else
		CPad::GetPad(0)->StartShake_Distance(240, 128,
			FindPlayerPed()->GetPosition().x,
			FindPlayerPed()->GetPosition().y,
			FindPlayerPed()->GetPosition().z);
#endif
			
		CParticle::HandleShootableBirdsStuff(shooter, source);

#ifdef FIRING_AND_AIMING
		if (TheCamera.Using1stPersonWeaponMode())
#endif
		CamShakeNoPos(&TheCamera, 0.2f);
	}

	if ( shooter->IsPed() )
		CEventList::RegisterEvent(EVENT_GUNSHOT, EVENT_ENTITY_PED,     shooter, (CPed*)shooter,                1000);
	else if ( shooter->IsVehicle() && ((CVehicle*)shooter)->pDriver )
		CEventList::RegisterEvent(EVENT_GUNSHOT, EVENT_ENTITY_VEHICLE, shooter, ((CVehicle*)shooter)->pDriver, 1000);

	return true;
}

bool
CWeapon::TakePhotograph(CEntity *shooter)
{
	if ( TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_CAMERA )
	{
		CSpecialFX::bSnapShotActive = true;
		CSpecialFX::SnapShotFrames = 0;
		CStats::PhotosTaken++;
		bPhotographHasBeenTaken = true;

#ifdef IMPROVED_TECH_PART // Saving a screenshot after taking a photo
		bTakePhoto = FrontEndMenuManager.m_PrefsStoreGalleryPhotos;
#endif
		
		for ( int32 i = CPools::GetPedPool()->GetSize() - 1; i >= 0; i--)
		{
			CPed *ped = CPools::GetPedPool()->GetSlot(i);
			if ( ped )
			{
				if ( (ped->GetPosition() - TheCamera.GetPosition()).Magnitude() < 125.0f )
				{
					CVector pedPos = ped->GetPosition();
					pedPos.z += 0.8f;
					
					CVector pos;
					float w, h;
					
					if ( CSprite::CalcScreenCoors(pedPos, &pos, &w, &h, false) )
					{
						if (   SCREEN_WIDTH  * 0.1f < pos.x && SCREEN_WIDTH  * 0.9f > pos.x
							&& SCREEN_HEIGHT * 0.1f < pos.y && SCREEN_HEIGHT * 0.9f > pos.y )
						{
							CVector source, target;
							CEntity *foundEnt = nil;
							CColPoint foundCol;
	
							target = pedPos;
							source = TheCamera.GetForward() * 2.0f + TheCamera.GetPosition();
							
							if ( CWorld::ProcessLineOfSight(source, target, foundCol, foundEnt, true, true, true, true, true, true, false) )
							{
								if ( foundEnt != (CEntity*)ped )
									continue;
							}
							
							ped->bHasBeenPhotographed = true;
						}
					}
				}
			}
		}
		
		return true;
	}
	
	return false;
}

bool
CWeapon::FireM16_1stPerson(CEntity *shooter)
{
	ASSERT(shooter!=nil);

	int16 mode = TheCamera.Cams[TheCamera.ActiveCam].Mode;

	if (!( mode == CCam::MODE_M16_1STPERSON
		|| mode == CCam::MODE_SNIPER
		|| mode == CCam::MODE_CAMERA
		|| mode == CCam::MODE_ROCKETLAUNCHER
		|| mode == CCam::MODE_M16_1STPERSON_RUNABOUT
		|| mode == CCam::MODE_SNIPER_RUNABOUT
		|| mode == CCam::MODE_ROCKETLAUNCHER_RUNABOUT
		|| mode == CCam::MODE_HELICANNON_1STPERSON) )
	{
		return false;
	}

	CWeaponInfo *info = GetInfo();

	CWorld::bIncludeCarTyres = true;
	CWorld::bIncludeBikers   = true;

	CColPoint point;
	CEntity *victim;

	CWorld::pIgnoreEntity = shooter;
	CWorld::bIncludeDeadPeds = true;

	CCam *cam = &TheCamera.Cams[TheCamera.ActiveCam];
	ASSERT(cam!=nil);

	CVector source = cam->Source;
	CVector target = cam->Front*info->m_fRange + source;

	if (ProcessLineOfSight(source, target, point, victim, m_eWeaponType, shooter, true, true, true, true, true, true, false)) {
		CheckForShootingVehicleOccupant(&victim, &point, m_eWeaponType, source, target);
	}
	CWorld::pIgnoreEntity = nil;
	CWorld::bIncludeDeadPeds = false;
	CWorld::bIncludeBikers = false;
	CWorld::bIncludeCarTyres = false;

	CVector2D front(cam->Front.x, cam->Front.y);
	front.Normalise();

	DoBulletImpact(shooter, victim, &source, &target, &point, front);

	CVector bulletPos;

	if ( CHeli::TestBulletCollision(&source, &target, &bulletPos, (m_eWeaponType == WEAPONTYPE_M60 || m_eWeaponType == WEAPONTYPE_HELICANNON ? 20 : 4)) )
	{
		for ( int32 i = 0; i < 16; i++ )
			CParticle::AddParticle(PARTICLE_SPARK, bulletPos, CVector(0.0f, 0.0f, 0.0f));
	}

	if ( shooter == FindPlayerPed() )
	{
		float mult;
		switch (m_eWeaponType) {
			case WEAPONTYPE_M4:
				mult = 0.0003f;
				break;
			case WEAPONTYPE_RUGER:
				mult = 0.00015f;
				break;
			case WEAPONTYPE_HELICANNON:
			case WEAPONTYPE_M60:
				mult = 0.0003f;
				break;
			default:
				mult = 0.0002f;
				break;
		}

		if (FindPlayerPed()->bIsDucking || FindPlayerPed()->m_attachedTo)
			mult *= 0.3f;

		TheCamera.Cams[TheCamera.ActiveCam].Beta  += float((CGeneral::GetRandomNumber() & 127) - 64) * mult;
		TheCamera.Cams[TheCamera.ActiveCam].Alpha += float((CGeneral::GetRandomNumber() & 127) - 64) * mult;

		// yes, double
		double notFiringRate = (20.0 - info->m_nFiringRate) / 80.0;
		double raisedNotFiringRate = Max(1.0, Max(0.0, notFiringRate));

		uint8 shakeFreq = 80.0 * raisedNotFiringRate + 130.0;
#ifdef IMPROVED_MENU_AND_INPUT
		CPad::GetPad(0)->StartShake(20000.0f * CTimer::GetTimeStep() / shakeFreq, 0, shakeFreq);
#else
		CPad::GetPad(0)->StartShake(20000.0f * CTimer::GetTimeStep() / shakeFreq, shakeFreq);
#endif
	}

	return true;
}

bool
CWeapon::FireInstantHitFromCar(CVehicle *shooter, bool left, bool right)
{
	CWeaponInfo *info = GetInfo();

	CVehicleModelInfo *modelInfo = shooter->GetModelInfo();
	
	CVector source, target;
	
	if ( shooter->IsBike() )
	{
		if ( shooter->pDriver )
		{
			source = info->m_vecFireOffset;
			
			shooter->pDriver->TransformToNode(source, PED_HANDR);
			source += CTimer::GetTimeStep() * shooter->m_vecMoveSpeed;
			
			if ( left )
				target = source - info->m_fRange * shooter->GetRight();
			else if ( right )
				target = source + info->m_fRange * shooter->GetRight();
			else
				target = source + info->m_fRange * shooter->GetForward();
				
		}
		else if ( left )
		{
			source = shooter->GetMatrix() * CVector(-shooter->GetColModel()->boundingBox.max.x + -0.25f,
													float(CGeneral::GetRandomNumber() & 255) * 0.001f + modelInfo->GetFrontSeatPosn().y - 0.05f,
													modelInfo->GetFrontSeatPosn().z + 0.63f);
			source += CTimer::GetTimeStep() * shooter->m_vecMoveSpeed;
	
	
			target = shooter->GetMatrix() * CVector(-info->m_fRange,
														modelInfo->GetFrontSeatPosn().y,
														modelInfo->GetFrontSeatPosn().z + 0.6f);
		}
		else if ( right )
		{
			source = shooter->GetMatrix() * CVector(shooter->GetColModel()->boundingBox.max.x + 0.25f,
													float(CGeneral::GetRandomNumber() & 255) * 0.001f + modelInfo->GetFrontSeatPosn().y - 0.18f,
													modelInfo->GetFrontSeatPosn().z + 0.52f);
			source += CTimer::GetTimeStep() * shooter->m_vecMoveSpeed;
	
			target = shooter->GetMatrix() * CVector(info->m_fRange,
														modelInfo->GetFrontSeatPosn().y,
														modelInfo->GetFrontSeatPosn().z + 0.5f);
		}
		else
		{
			source = shooter->GetMatrix() * CVector(float(CGeneral::GetRandomNumber() & 255) * 0.001f + -0.4f,
													modelInfo->GetFrontSeatPosn().y + shooter->GetColModel()->boundingBox.max.y + 0.2f,
													modelInfo->GetFrontSeatPosn().z + 0.55f);
			source += CTimer::GetTimeStep() * shooter->m_vecMoveSpeed;
	
			target = shooter->GetMatrix() * CVector(0.0f,
													info->m_fRange,
													modelInfo->GetFrontSeatPosn().z + 0.5f);
		}
	}
	else
	{
		if ( left )
			source = info->m_vecFireOffset;
		else
		{
			source    = 1.8f * info->m_vecFireOffset;
			source.z -= 0.1f;
		}
		
		shooter->pDriver->TransformToNode(source, PED_HANDR);
		source += CTimer::GetTimeStep() * shooter->m_vecMoveSpeed;
		
		if ( left )
			target = source - info->m_fRange * shooter->GetRight();
		else
			target = source + info->m_fRange * shooter->GetRight();
	}

	target += CVector(float(CGeneral::GetRandomNumber()&255)*0.01f-1.28f,
						float(CGeneral::GetRandomNumber()&255)*0.01f-1.28f,
						float(CGeneral::GetRandomNumber()&255)*0.01f-1.28f);

	DoDriveByAutoAiming(FindPlayerPed(), shooter, &source, &target);

	CEventList::RegisterEvent(EVENT_GUNSHOT, EVENT_ENTITY_PED, FindPlayerPed(), FindPlayerPed(), 1000);

	if ( !TheCamera.GetLookingLRBFirstPerson() )
	{
		if ( !shooter->IsBike() )
			CParticle::AddParticle(PARTICLE_GUNFLASH, source, CVector(0.0f, 0.0f, 0.0f));
		else
			CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, source, 1.4f*shooter->m_vecMoveSpeed);
	}
	else
		CParticle::AddParticle(PARTICLE_GUNFLASH_NOANIM, source, 1.6f*shooter->m_vecMoveSpeed, nil, 0.18f);

	CEventList::RegisterEvent(EVENT_GUNSHOT, EVENT_ENTITY_VEHICLE, shooter, FindPlayerPed(), 1000);

	CPointLights::AddLight(CPointLights::LIGHT_POINT, source, CVector(0.0f, 0.0f, 0.0f), 5.0f,
		1.0f, 0.8f, 0.0f, CPointLights::FOG_NONE, false);

	CColPoint point;
	CEntity *victim;
	
	CWorld::bIncludeBikers = true;
	CWorld::pIgnoreEntity = shooter;  
	ProcessLineOfSight(source, target, point, victim, m_eWeaponType, shooter, true, true, true, true, true, true, false);
	CWorld::pIgnoreEntity = NULL;
	CWorld::bIncludeBikers = false;

	if ( !(CTimer::GetFrameCounter() & 3) )
		MakePedsJumpAtShot(shooter, &source, &target);

	if ( victim )
	{
		CVector traceTarget = point.point;
		CBulletTraces::AddTrace(&source, &traceTarget, m_eWeaponType, shooter);

		if ( victim->IsPed() )
		{
			CPed *victimPed = (CPed*)victim;

			if ( !victimPed->DyingOrDead() && victim != (CEntity *)shooter )
			{
				CVector pos = victimPed->GetPosition();

				CVector2D posOffset(source.x-pos.x, source.y-pos.y);
				int32 localDir = victimPed->GetLocalDirection(posOffset);

				victimPed->ReactToAttack(FindPlayerPed());
				victimPed->ClearAttackByRemovingAnim();

				CAnimBlendAssociation *asoc = CAnimManager::AddAnimation(victimPed->GetClump(), ASSOCGRP_STD, AnimationId(ANIM_STD_HITBYGUN_FRONT + localDir));
				ASSERT(asoc!=nil);
				asoc->blendAmount = 0.0f;
				asoc->blendDelta  = 8.0f;

				victimPed->InflictDamage(shooter, WEAPONTYPE_UZI_DRIVEBY, 3*info->m_nDamage, (ePedPieceTypes)point.pieceB, localDir);

				pos.z += 0.8f;

				if ( victimPed->GetIsOnScreen() )
				{
					if ( CGame::nastyGame )
					{
						for ( int32 i = 0; i < 4; i++ )
						{
							CVector dir;
							dir.x = CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);
							dir.y = CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);
							dir.z = CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);

							CParticle::AddParticle(PARTICLE_BLOOD, pos, dir);
						}
					}
				}

				if ( victimPed->m_nPedType == PEDTYPE_COP )
					CEventList::RegisterEvent(EVENT_SHOOT_COP, EVENT_ENTITY_PED, victimPed, FindPlayerPed(), 10000);
				else
					CEventList::RegisterEvent(EVENT_SHOOT_PED, EVENT_ENTITY_PED, victimPed, FindPlayerPed(), 10000);
			}
		}
		else if ( victim->IsVehicle() )
			((CVehicle *)victim)->InflictDamage(FindPlayerPed(), WEAPONTYPE_UZI_DRIVEBY, info->m_nDamage);
		else
			CGlass::WasGlassHitByBullet(victim, point.point);

		switch ( victim->GetType() )
		{
			case ENTITY_TYPE_BUILDING:
			{
				PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_GROUND_1, point.point);
				break;
			}
			case ENTITY_TYPE_VEHICLE:
			{
				DMAudio.PlayOneShot(((CPhysical*)victim)->m_audioEntityId, SOUND_WEAPON_HIT_VEHICLE, 1.0f);
				break;
			}
			case ENTITY_TYPE_PED:
			{
				DMAudio.PlayOneShot(((CPhysical*)victim)->m_audioEntityId, SOUND_WEAPON_HIT_PED, 1.0f);
				((CPed*)victim)->Say(SOUND_PED_BULLET_HIT);
				break;
			}
			case ENTITY_TYPE_OBJECT:
			{
				PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_GROUND_2, point.point);
				break;
			}
			case ENTITY_TYPE_DUMMY:
			{
				PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_GROUND_3, point.point);
				break;
			}
			default: break;
		}
	}
	else
	{
		float norm = 30.0f/info->m_fRange;
		CVector traceTarget = (target-source)*norm + source;
		CBulletTraces::AddTrace(&source, &traceTarget, m_eWeaponType, shooter);
	}

	if ( shooter == FindPlayerVehicle() )
#ifdef IMPROVED_MENU_AND_INPUT
		CPad::GetPad(0)->StartShake_Distance(240, 0, 128, FindPlayerVehicle()->GetPosition().x, FindPlayerVehicle()->GetPosition().y, FindPlayerVehicle()->GetPosition().z);
#else
		CPad::GetPad(0)->StartShake_Distance(240, 128, FindPlayerVehicle()->GetPosition().x, FindPlayerVehicle()->GetPosition().y, FindPlayerVehicle()->GetPosition().z);
#endif

	return true;
}

void
CWeapon::DoDoomAiming(CEntity *shooter, CVector *source, CVector *target)
{
	ASSERT(shooter!=nil);
	ASSERT(source!=nil);
	ASSERT(target !=nil);

#ifndef FIX_BUGS
	CEntity entity; // unused
#endif

	CPed *shooterPed = (CPed*)shooter;

	int16 lastEntity;
	CEntity *entities[16];
	CWorld::FindObjectsInRange(*source, (*target-*source).Magnitude(), true, &lastEntity, 15, entities, false, true, true, false, false);

	float closestEntityDist = 10000.0f;
	int16 closestEntity;

	for ( int32 i = 0; i < lastEntity; i++ )
	{
		CEntity *victim = entities[i];
		ASSERT(victim!=nil);

		if ( (CEntity*)shooterPed != victim && shooterPed->CanSeeEntity(victim, DEGTORAD(22.5f)) )
		{
			if ( !(victim->GetStatus() == STATUS_TRAIN_MOVING
				|| victim->GetStatus() == STATUS_TRAIN_NOT_MOVING
				|| victim->GetStatus() == STATUS_HELI
				|| victim->GetStatus() == STATUS_PLANE
				|| victim->GetStatus() == STATUS_WRECKED) )
			{
				float distToVictim   = (shooterPed->GetPosition()-victim->GetPosition()).Magnitude2D();
				float distToVictimZ  = Abs(shooterPed->GetPosition().z-victim->GetPosition().z);

				if ( 1.5f*distToVictimZ < distToVictim )
				{
					float entityDist = Sqrt(SQR(distToVictim) + SQR(distToVictimZ));

					if ( entityDist < closestEntityDist )
					{
						closestEntityDist = entityDist;
						closestEntity = i;
					}
				}
			}
		}
	}

	CColPoint foundCol;
	CEntity *foundEnt;
	if (closestEntityDist < DOOMAUTOAIMING_MAXDIST
		&& !CWorld::ProcessLineOfSight(*source, entities[closestEntity]->GetPosition(), foundCol, foundEnt, true, false, false, false, false, false, false, true))
	{
		CEntity *victim = entities[closestEntity];
		ASSERT(victim !=nil);

		float distToTarget = (*target - *source).Magnitude2D();
		float distToSource = (victim->GetPosition() - *source).Magnitude2D();

		float victimZ = victim->GetPosition().z + 0.3f;
		if ( victim->IsPed() )
		{
			if ( ((CPed*)victim)->bIsDucking )
				victimZ -= 0.8f;
		}

		(*target).z = (distToTarget / distToSource) * (victimZ - (*source).z) + (*source).z;
	}
}

void
CWeapon::DoTankDoomAiming(CEntity *shooter, CEntity *driver, CVector *source, CVector *target)
{
	ASSERT(shooter!=nil);
	ASSERT(driver!=nil);
	ASSERT(source!=nil);
	ASSERT(target!=nil);

#ifndef FIX_BUGS
	CEntity entity; // unused
#endif

	int16 lastEntity;
	CEntity *entities[16];
	CWorld::FindObjectsInRange(*source, (*target-*source).Magnitude(), true, &lastEntity, 15, entities, false, true, false, false, false);

	float closestEntityDist = 10000.0f;
	int16 closestEntity;

	float normZ = (target->z - source->z) / (*target-*source).Magnitude();

	for ( int32 i = 0; i < lastEntity; i++ )
	{
		CEntity *victim = entities[i];

		ASSERT(victim!=nil);

		if ( shooter != victim && driver != victim )
		{
			if ( !(victim->GetStatus() == STATUS_TRAIN_MOVING
				|| victim->GetStatus() == STATUS_TRAIN_NOT_MOVING
				|| victim->GetStatus() == STATUS_HELI
				|| victim->GetStatus() == STATUS_PLANE) )
			{
				if ( !(victim->IsVehicle() && victim->bRenderScorched) )
				{
					float distToVictim  = (shooter->GetPosition()-victim->GetPosition()).Magnitude2D();
					float distToVictimZ = Abs(shooter->GetPosition().z - (distToVictim*normZ + victim->GetPosition().z));

					if ( 3.0f*distToVictimZ < distToVictim )
					{
						CVector tmp = CVector(victim->GetPosition().x, victim->GetPosition().y, 0.0f);
						if ( CCollision::DistToLine(source, target,
								&tmp) < victim->GetBoundRadius()*3.0f )
						{
							float vehicleDist = Sqrt(SQR(distToVictim) + SQR(distToVictimZ));
							if ( vehicleDist < closestEntityDist )
							{
								closestEntityDist = vehicleDist;
								closestEntity = i;
							}
						}
					}
				}
			}
		}
	}

	if ( closestEntityDist < DOOMAUTOAIMING_MAXDIST )
	{
		CEntity *victim = entities[closestEntity];
		ASSERT(victim!=nil);

		float distToTarget = (*target - *source).Magnitude2D();
		float distToSource = (victim->GetPosition() - *source).Magnitude2D();

		(*target).z = (distToTarget / distToSource) * (0.3f + victim->GetPosition().z - (*source).z) + (*source).z;
	}
}


void
CWeapon::DoDriveByAutoAiming(CEntity *driver, CVehicle *vehicle, CVector *source, CVector *target)
{
	ASSERT(driver!=nil);
	ASSERT(source!=nil);
	ASSERT(target!=nil);

#ifndef FIX_BUGS
	CEntity entity; // unused
#endif

	CPed *shooterPed = (CPed*)driver;

	int16 lastEntity;
	CEntity *peds[16];
	CWorld::FindObjectsInRange(*source, (*target-*source).Magnitude(), true, &lastEntity, 15, peds, false, false, true, false, false);

	float closestEntityDist = 10000.0f;
	int16 closestEntity;

	for ( int32 i = 0; i < lastEntity; i++ )
	{
		CPed *victim = (CPed*)peds[i];
		ASSERT(victim!=nil);

		if (driver != victim && !victim->DyingOrDead() && victim->m_attachedTo != vehicle)
		{
			float lineDist = CCollision::DistToLine(source, target, &victim->GetPosition());

			uint32 model = vehicle->GetModelIndex();
			float pedDist;
			if (model == MI_HUNTER || model == MI_SEASPAR || model == MI_SPARROW)
			{
				float distToVictim = (victim->GetPosition() - vehicle->GetPosition()).Magnitude();
				pedDist = lineDist / Max(5.f, distToVictim);
			}
			else
			{
				float distToVictim = (victim->GetPosition() - driver->GetPosition()).Magnitude();
				pedDist = 0.15f * distToVictim + lineDist;
			}

			if ( DotProduct((*target-*source), victim->GetPosition()-*source) > 0.0f && pedDist < closestEntityDist)
			{
				closestEntity = i;
				closestEntityDist = pedDist;
			}
		}
	}
	uint32 model = vehicle->GetModelIndex();
	float maxAimDistance = CAR_DRIVEBYAUTOAIMING_MAXDIST;
	if (model == MI_HUNTER)
	{
		maxAimDistance = Tan(DEGTORAD(fHunterAimingAngle));
	}
	else if (model == MI_SEASPAR || model == MI_SPARROW)
	{
		maxAimDistance = Tan(DEGTORAD(fSeaSparrowAimingAngle));
	}

	if ( closestEntityDist < maxAimDistance )
	{
		CEntity *victim = peds[closestEntity];
		ASSERT(victim!=nil);

		float distToTarget = (*source - *target).Magnitude();
		float distToSource = (*source - victim->GetPosition()).Magnitude();
		*target = (distToTarget / distToSource) * (victim->GetPosition() - *source) + *source;
	}
}

void
CWeapon::Reload(void)
{
	if (m_nAmmoTotal == 0)
		return;

	CWeaponInfo *info = GetInfo();

	if (m_nAmmoTotal >= info->m_nAmountofAmmunition)
		m_nAmmoInClip = info->m_nAmountofAmmunition;
	else
		m_nAmmoInClip = m_nAmmoTotal;
}

void
CWeapon::Update(int32 audioEntity, CPed *pedToAdjustSound)
{
	CWeaponInfo *info = GetInfo();

#ifdef EX_IMPROVED_WEAPONS // After the last shot is fired, the slide moves backwards
	if (pedToAdjustSound && pedToAdjustSound->m_pWeaponModel) {
		CAnimBlendAssociation* animForWeaponAssoc = RpAnimBlendClumpGetAssociation(pedToAdjustSound->m_pWeaponModel, ANIM_FOR_WEAPON_SLIDE);
		if (m_eWeaponState == WEAPONSTATE_RELOADING && m_nAmmoInClip == 0) {
			if (animForWeaponAssoc) {
				float maxSlideOffsetTime = animForWeaponAssoc->hierarchy->totalLength / 2.5f;
				if (animForWeaponAssoc->speed != 0.75f && animForWeaponAssoc->IsRunning() &&
					animForWeaponAssoc->currentTime >= maxSlideOffsetTime) {

					animForWeaponAssoc->SetCurrentTime(maxSlideOffsetTime);
					animForWeaponAssoc->flags &= ~ASSOC_RUNNING;
				}
			}
		}
	}
#endif

	switch ( m_eWeaponState )
	{
		case WEAPONSTATE_MELEE_MADECONTACT:
		{
			m_eWeaponState = WEAPONSTATE_READY;
			break;
		}

		case WEAPONSTATE_FIRING:
		{
			if ( IsShotgun(m_eWeaponType) && AEHANDLE_IS_OK(audioEntity) )
			{
				uint32 timePassed = m_nTimer - CWeaponInfo::ms_aReloadSampleTime[m_eWeaponType];
				if ( CTimer::GetPreviousTimeInMilliseconds() < timePassed && CTimer::GetTimeInMilliseconds() >= timePassed )
					DMAudio.PlayOneShot(audioEntity, SOUND_WEAPON_RELOAD, 0.0f);
			}

			if ( CTimer::GetTimeInMilliseconds() > m_nTimer )
			{
				if ( GetInfo()->m_eWeaponFire != WEAPON_FIRE_MELEE && m_nAmmoTotal == 0 ) {
					m_eWeaponState = WEAPONSTATE_OUT_OF_AMMO;
					CPickups::RemoveAllPickupsOfACertainWeaponGroupWithNoAmmo(m_eWeaponType);
				} else
					m_eWeaponState = WEAPONSTATE_READY;
			}

			break;
		}

		case WEAPONSTATE_RELOADING:
		{
			if  ( AEHANDLE_IS_OK(audioEntity) && m_eWeaponType < WEAPONTYPE_TOTALWEAPONS)
			{
				CAnimBlendAssociation *reloadAssoc = nil;
				if (pedToAdjustSound) {
					if (CPed::GetReloadAnim(info) && (!CWorld::Players[CWorld::PlayerInFocus].m_bFastReload || !pedToAdjustSound->IsPlayer())) {
#ifdef FIRING_AND_AIMING // shotgun sound
						if (m_eWeaponType == WEAPONTYPE_SHOTGUN) {
							reloadAssoc = RpAnimBlendClumpGetAssociation(pedToAdjustSound->GetClump(), ANIM_WEAPON_FIRE);
							if (!reloadAssoc)
								reloadAssoc = RpAnimBlendClumpGetAssociation(pedToAdjustSound->GetClump(), ANIM_WEAPON_CROUCHFIRE);
						} else {
							reloadAssoc = RpAnimBlendClumpGetAssociation(pedToAdjustSound->GetClump(), CPed::GetReloadAnim(info));
							if (!reloadAssoc)
								reloadAssoc = RpAnimBlendClumpGetAssociation(pedToAdjustSound->GetClump(), CPed::GetCrouchReloadAnim(info));
						}
#else
						reloadAssoc = RpAnimBlendClumpGetAssociation(pedToAdjustSound->GetClump(), CPed::GetReloadAnim(info));
						if (!reloadAssoc) {
							reloadAssoc = RpAnimBlendClumpGetAssociation(pedToAdjustSound->GetClump(), CPed::GetCrouchReloadAnim(info));
						}
#endif
					}
				}
				if (reloadAssoc && reloadAssoc->IsRunning() && reloadAssoc->blendAmount > 0.2f) {
					float soundStart = 0.75f;
					switch (info->m_AnimToPlay) {
						case ASSOCGRP_PYTHON:
							soundStart = fReloadAnimSampleFraction[0];
							break;
						case ASSOCGRP_COLT:
						case ASSOCGRP_TEC:
#ifdef EX_WEAPON_DEAGLE // Anims
						case ASSOCGRP_DEAGLE:
#endif
							soundStart = fReloadAnimSampleFraction[1];
							break;
						case ASSOCGRP_UZI:
							soundStart = fReloadAnimSampleFraction[2];
							break;
						case ASSOCGRP_RIFLE:
#ifdef EX_WEAPON_STEYR // Anims
						case ASSOCGRP_STEYR:
#endif
							soundStart = fReloadAnimSampleFraction[3];
							break;
						case ASSOCGRP_M60:
							soundStart = fReloadAnimSampleFraction[4];
							break;
						default:
							break;
					}
					if (reloadAssoc->GetProgress() >= soundStart && (reloadAssoc->currentTime - reloadAssoc->timeStep) / reloadAssoc->hierarchy->totalLength < soundStart) {
						DMAudio.PlayOneShot(audioEntity, SOUND_WEAPON_RELOAD, m_eWeaponType);

#ifdef EX_IMPROVED_WEAPONS // After the last shot is fired, the slide moves backwards
						CAnimBlendAssociation* animForWeaponAssoc = RpAnimBlendClumpGetAssociation(pedToAdjustSound->m_pWeaponModel, ANIM_FOR_WEAPON_SLIDE);
						if (animForWeaponAssoc && !animForWeaponAssoc->IsRunning()) {
							animForWeaponAssoc->SetRun();
							animForWeaponAssoc->speed = 0.75f;
						}
#endif
					}
					if (CTimer::GetTimeInMilliseconds() > m_nTimer && reloadAssoc->GetProgress() < 0.9f) {
						m_nTimer = CTimer::GetTimeInMilliseconds();
					}
				}
#ifndef IMPROVED_TECH_PART // double reload sound fix
				else {
					uint32 timePassed = m_nTimer - CWeaponInfo::ms_aReloadSampleTime[m_eWeaponType];
					if (CTimer::GetPreviousTimeInMilliseconds() < timePassed && CTimer::GetTimeInMilliseconds() >= timePassed)
						DMAudio.PlayOneShot(audioEntity, SOUND_WEAPON_RELOAD, m_eWeaponType);
				}
#endif
			}

			if ( CTimer::GetTimeInMilliseconds() > m_nTimer )
			{
				Reload();
				m_eWeaponState = WEAPONSTATE_READY;

#ifdef FIRING_AND_AIMING // After reloading
				if (pedToAdjustSound && pedToAdjustSound->IsPlayer()) {
					CPlayerPed* playerPed = (CPlayerPed*)pedToAdjustSound;
					if (!playerPed->bIsPlayerAiming && !CPad::GetPad(0)->GetWeapon()) {
						playerPed->ClearWeaponTarget();
						playerPed->StopAimingAnims();
					}
				}
#endif
			}

			break;
		}
		default: break;
	}
}

void
FireOneInstantHitRound(CVector *source, CVector *target, int32 damage)
{
	ASSERT(source!=nil);
	ASSERT(target!=nil);

	CParticle::AddParticle(PARTICLE_GUNFLASH, *source, CVector(0.0f, 0.0f, 0.0f));

	CPointLights::AddLight(CPointLights::LIGHT_POINT,
		*source, CVector(0.0f, 0.0f, 0.0f), 5.0f,
		1.0f, 0.8f, 0.0f, CPointLights::FOG_NONE, false);

	CColPoint point;
	CEntity *victim;
	CWorld::ProcessLineOfSight(*source, *target, point, victim, true, true, true, true, true, true, false);

	CParticle::AddParticle(PARTICLE_HELI_ATTACK, *source, ((*target) - (*source)) * 0.15f);

	if ( victim )
	{
		if ( victim->IsPed() )
		{
			CPed *victimPed = (CPed *)victim;
			if ( !victimPed->DyingOrDead() )
			{
				CVector pos = victimPed->GetPosition();

				CVector2D posOffset((*source).x-pos.x, (*source).y-pos.y);
				int32 localDir = victimPed->GetLocalDirection(posOffset);

				victimPed->ClearAttackByRemovingAnim();

				CAnimBlendAssociation *asoc = CAnimManager::AddAnimation(victimPed->GetClump(), ASSOCGRP_STD, AnimationId(ANIM_STD_HITBYGUN_FRONT + localDir));
				ASSERT(asoc!=nil);
				asoc->blendAmount = 0.0f;
				asoc->blendDelta  = 8.0f;

				victimPed->InflictDamage(nil, WEAPONTYPE_UZI, damage, (ePedPieceTypes)point.pieceB, localDir);

				pos.z += 0.8f;

				if ( victimPed->GetIsOnScreen() )
				{
					if ( CGame::nastyGame )
					{
						for ( int32 i = 0; i < 4; i++ )
						{
							CVector dir;
							dir.x = CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);
							dir.y = CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);
							dir.z = CGeneral::GetRandomNumberInRange(-0.1f, 0.1f);

							CParticle::AddParticle(PARTICLE_BLOOD, pos, dir);
						}
					}
				}
			}
		}
		else if ( victim->IsVehicle() )
			((CVehicle *)victim)->InflictDamage(nil, WEAPONTYPE_UZI, damage);
		//BUG ? no CGlass::WasGlassHitByBullet

		switch ( victim->GetType() )
		{
			case ENTITY_TYPE_BUILDING:
			{
				PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_GROUND_1, point.point);
				CParticle::AddParticle(PARTICLE_SMOKE, point.point, CVector(0.0f, 0.0f, 0.01f));
				break;
			}
			case ENTITY_TYPE_VEHICLE:
			{
				CStats::BulletsThatHit++;
				DMAudio.PlayOneShot(((CPhysical*)victim)->m_audioEntityId, SOUND_WEAPON_HIT_VEHICLE, 1.0f);
				break;
			}
			case ENTITY_TYPE_PED:
			{
				CStats::BulletsThatHit++;
				DMAudio.PlayOneShot(((CPhysical*)victim)->m_audioEntityId, SOUND_WEAPON_HIT_PED, 1.0f);
				((CPed*)victim)->Say(SOUND_PED_BULLET_HIT);
				break;
			}
			case ENTITY_TYPE_OBJECT:
			{
				CStats::BulletsThatHit++;
				PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_GROUND_2, point.point);
				break;
			}
			case ENTITY_TYPE_DUMMY:
			{
				PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_GROUND_3, point.point);
				break;
			}
			default: break;
		}
	}
	else
	{
		float waterLevel;
		if ( CWaterLevel::GetWaterLevel((*target).x, (*target).y, (*target).z + 10.0f, &waterLevel, false) )
		{
			CParticle::AddParticle(PARTICLE_BOAT_SPLASH, CVector((*target).x, (*target).y, waterLevel), CVector(0.0f, 0.0f, 0.01f));
			PlayOneShotScriptObject(SCRIPT_SOUND_BULLET_HIT_WATER, point.point); // no sound(empty)
		}
	}
}

bool
CWeapon::IsTypeMelee(void)
{
	return CWeaponInfo::GetWeaponInfo(m_eWeaponType)->m_eWeaponFire == WEAPON_FIRE_MELEE;
}

bool
CWeapon::IsType2Handed(void)
{
	return m_eWeaponType == WEAPONTYPE_FLAMETHROWER || m_eWeaponType == WEAPONTYPE_HELICANNON || m_eWeaponType == WEAPONTYPE_M60 ||
		m_eWeaponType == WEAPONTYPE_M4 || IsShotgun(m_eWeaponType) ||
#ifdef EX_WEAPON_AK47 // IsType2Handed
		m_eWeaponType == WEAPONTYPE_AK47 ||
#endif
#ifdef EX_WEAPON_M16 // IsType2Handed
		m_eWeaponType == WEAPONTYPE_M16 ||
#endif
#ifdef EX_WEAPON_STEYR // IsType2Handed
		m_eWeaponType == WEAPONTYPE_STEYR ||
#endif
		m_eWeaponType == WEAPONTYPE_RUGER || m_eWeaponType == WEAPONTYPE_SNIPERRIFLE || m_eWeaponType == WEAPONTYPE_LASERSCOPE;
}

void
CWeapon::MakePedsJumpAtShot(CPhysical *shooter, CVector *source, CVector *target)
{
	ASSERT(shooter!=nil);
	ASSERT(source!=nil);
	ASSERT(target!=nil);

	float minx = Min(source->x, target->x) - 2.0f;
	float maxx = Max(source->x, target->x) + 2.0f;
	float miny = Min(source->y, target->y) - 2.0f;
	float maxy = Max(source->y, target->y) + 2.0f;
	float minz = Min(source->z, target->z) - 2.0f;
	float maxz = Max(source->z, target->z) + 2.0f;

	for ( int32 i = CPools::GetPedPool()->GetSize() - 1; i >= 0; i--)
	{
		CPed *ped = CPools::GetPedPool()->GetSlot(i);

		if ( ped )
		{
			if (   ped->GetPosition().x > minx && ped->GetPosition().x < maxx
				&& ped->GetPosition().y > miny && ped->GetPosition().y < maxy
				&& ped->GetPosition().z > minz && ped->GetPosition().z < maxz )
			{
				if ( ped != FindPlayerPed() && !((uint8)(ped->m_randomSeed ^ CGeneral::GetRandomNumber()) & 31) )
					ped->SetEvasiveDive(shooter, 1);
			}
		}
	}
}

bool
CWeapon::HitsGround(CEntity *holder, CVector *fireSource, CEntity *aimingTo)
{
	ASSERT(holder!=nil);
	ASSERT(aimingTo!=nil);

	if (!holder->IsPed() || !((CPed*)holder)->m_pSeekTarget)
		return false;

	CWeaponInfo *info = GetInfo();

	CVector adjustedOffset = info->m_vecFireOffset;
	adjustedOffset.z += 0.6f;

	CVector source, target;
	CEntity *foundEnt = nil;
	CColPoint foundCol;

	if (fireSource)
		source = *fireSource;
	else
		source = holder->GetMatrix() * adjustedOffset;

	CEntity *aimEntity = aimingTo ? aimingTo : ((CPed*)holder)->m_pSeekTarget;
	ASSERT(aimEntity!=nil);

	target = aimEntity->GetPosition();
	target.z += 0.6f;

	CWorld::ProcessLineOfSight(source, target, foundCol, foundEnt, true, false, false, false, false, false, false);
	if (foundEnt && foundEnt->IsBuilding()) {
		// That was supposed to be Magnitude, according to leftover code in assembly
		float diff = (foundCol.point.z - source.z);
		if (diff < 0.0f && diff > -3.0f)
			return true;
	}

	return false;
}

void
CWeapon::BlowUpExplosiveThings(CEntity *thing)
{
#ifdef FIX_BUGS
	if ( thing && thing->IsObject() )
#else
	if ( thing )
#endif
	{
		CObject *object = (CObject*)thing;
		int32 mi = object->GetModelIndex();
		if ( IsExplosiveThingModel(mi) && !object->bHasBeenDamaged && object->IsObject() )
		{
			object->bHasBeenDamaged = true;

			CExplosion::AddExplosion(object, FindPlayerPed(), EXPLOSION_BARREL, object->GetPosition()+CVector(0.0f,0.0f,0.5f), 100);

			if ( MI_EXPLODINGBARREL == mi )
				object->m_vecMoveSpeed.z += 0.55f;
			else
				object->m_vecMoveSpeed.z += 0.45f;

			object->m_vecMoveSpeed.x += float((CGeneral::GetRandomNumber()&255) - 128) * 0.0002f;
			object->m_vecMoveSpeed.y += float((CGeneral::GetRandomNumber()&255) - 128) * 0.0002f;

			if ( object->GetIsStatic())
			{
				object->SetIsStatic(false);
				object->AddToMovingList();
			}
		}
	}
}

bool
CWeapon::HasWeaponAmmoToBeUsed(void)
{
	// FIX: This is better (not bug tho)
//#if 0
	if (m_eWeaponType <= WEAPONTYPE_CHAINSAW)
//#else
//	if (CWeaponInfo::GetWeaponInfo(m_eWeaponType)->m_eWeaponFire == WEAPON_FIRE_MELEE)
//#endif
		return true;
	else
		return m_nAmmoTotal != 0;
}

bool
CPed::IsPedDoingDriveByShooting(void)
{
#ifdef FIX_BUGS
	if (FindPlayerPed() == this && CWeaponInfo::GetWeaponInfo(GetWeapon()->m_eWeaponType)->m_nWeaponSlot == WEAPONSLOT_SUBMACHINEGUN) {
#else
	if (FindPlayerPed() == this && GetWeapon()->m_eWeaponType == WEAPONTYPE_UZI) {
#endif
		if (TheCamera.Cams[TheCamera.ActiveCam].LookingLeft || TheCamera.Cams[TheCamera.ActiveCam].LookingRight)
			return true;
	}
	return false;
}

bool
CWeapon::ProcessLineOfSight(CVector const &point1, CVector const &point2, CColPoint &point, CEntity *&entity, eWeaponType type, CEntity *shooter, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, bool ignoreSomeObjects)
{
	return CWorld::ProcessLineOfSight(point1, point2, point, entity, checkBuildings, checkVehicles, checkPeds, checkObjects, checkDummies, false, ignoreSomeObjects, true);
}


void
CWeapon::CheckForShootingVehicleOccupant(CEntity **victim, CColPoint *point, eWeaponType weapon, CVector const& source, CVector const& target)
{
	if (!(*victim)->IsVehicle())
		return;

	CColSphere headSphere;

	CVehicle *veh = (CVehicle*)*victim;
	CColPoint origPoint(*point);
	float radius = 1.0f;
	bool found = false;
	CColLine shootLine(source, target);

	if (veh->pDriver && veh->pDriver->bCanBeShotInVehicle) {
		CVector pos(0.f, 0.f, 0.f);
		veh->pDriver->TransformToNode(pos, PED_HEAD);
		headSphere.Set(0.2f, pos + CVector(0.f, 0.f, 0.1f), 0, PEDPIECE_HEAD);
		if (CCollision::ProcessLineSphere(shootLine, headSphere, *point, radius)) {
			*victim = veh->pDriver;
			found = true;
		}
	}
	
	for(int i = 0; i < ARRAY_SIZE(veh->pPassengers); i++) {
		CPed *passenger = veh->pPassengers[i];
		if (passenger && passenger->bCanBeShotInVehicle) {
			CVector pos(0.f, 0.f, 0.f);
			passenger->TransformToNode(pos, PED_HEAD);
			headSphere.Set(0.2f, pos + CVector(0.f, 0.f, 0.1f), 0, PEDPIECE_HEAD);
			if (CCollision::ProcessLineSphere(shootLine, headSphere, *point, radius)) {
				*victim = passenger;
				found = true;
			}
		}
	}
	if (veh->IsCar()) {
		CVector distVec = target - source;
		if (DotProduct(distVec, veh->GetForward()) < 0.0f && DotProduct(distVec, veh->GetUp()) <= 0.0f) {
			CColModel *colModel = veh->GetColModel();
			if (colModel->numTriangles > 0) {
				bool passesGlass = false;
				CMatrix invVehMat;
				Invert(veh->GetMatrix(), invVehMat);
				shootLine.p0 = invVehMat * shootLine.p0;
				shootLine.p1 = invVehMat * shootLine.p1;
				CCollision::CalculateTrianglePlanes(colModel);
				for (int i = 0; i < colModel->numTriangles; i++) {
					if (colModel->triangles[i].surface == SURFACE_GLASS &&
						CCollision::TestLineTriangle(shootLine, colModel->vertices, colModel->triangles[i], colModel->trianglePlanes[i])) {
						passesGlass = true;
						break;
					}
				}
				CAutomobile *car = (CAutomobile*)veh;

				// No need to damage windscreen if there isn't one.
				if (passesGlass && car->Damage.ProgressPanelDamage(VEHPANEL_WINDSCREEN)) {
					if (car->Damage.GetPanelStatus(VEHPANEL_WINDSCREEN) == PANEL_STATUS_SMASHED2)
						car->Damage.ProgressPanelDamage(VEHPANEL_WINDSCREEN);

					car->SetPanelDamage(CAR_WINDSCREEN, VEHPANEL_WINDSCREEN, true);
					DMAudio.PlayOneShot(veh->m_audioEntityId, SOUND_CAR_WINDSHIELD_CRACK, 0.f);
				}
			}
		}
	}

	if (!found) {
		*victim = veh;
		*point = origPoint;
	}
}

#ifdef IMPROVED_VEHICLES_2 // check for shooting vehicle lights and petrolcap
void CWeapon::CheckForShootingVehicleLights(CEntity* victim, CColPoint point)
{
	if (!victim->IsVehicle())
		return;

	CVehicle* vehicle = (CVehicle*)victim;

	if (!vehicle)
		return;

	if (!vehicle->IsCar() && !vehicle->IsBike())
		return;

	if (vehicle->IsCar()) {
		CAutomobile* automobile = (CAutomobile*)vehicle;
		for (int frameID = CAR_HEADLIGHT_L; frameID < NUM_CAR_NODES; frameID++) {
			RwFrame* lightFrame = automobile->m_aCarNodes[frameID];
			if (!lightFrame)
				continue;

			if (automobile->GetFrameLightStatus((eCarNodes)frameID) == LIGHT_STATUS_BROKEN)
				continue;

			float radius = 0.035f;
			float distanceSqr = (point.point - lightFrame->getLTM()->pos).MagnitudeSqr();
			if (distanceSqr > radius || point.normal.z > 0.9f)
				continue;

			automobile->SetFrameLightStatus((eCarNodes)frameID, LIGHT_STATUS_BROKEN);
			DMAudio.PlayOneShot(automobile->m_audioEntityId, SOUND_CAR_LIGHT_BREAK, 1.0f);

			break;
		}
	} else {
		CBike* bike = (CBike*)vehicle;
		for (int frameID = BIKE_HEADLIGHT_L; frameID < BIKE_NUM_NODES; frameID++) {
			RwFrame* lightFrame = bike->m_aBikeNodes[frameID];
			if (!lightFrame)
				continue;

			if (bike->GetFrameLightStatus((eBikeNodes)frameID) == LIGHT_STATUS_BROKEN)
				continue;

			float radius = 0.035f;
			float distanceSqr = (point.point - lightFrame->getLTM()->pos).MagnitudeSqr();
			if (distanceSqr > radius || point.normal.z > 0.9f)
				continue;

			bike->SetFrameLightStatus((eBikeNodes)frameID, LIGHT_STATUS_BROKEN);
			DMAudio.PlayOneShot(bike->m_audioEntityId, SOUND_CAR_LIGHT_BREAK, 1.0f);

			break;
		}
	}
}
void CWeapon::CheckForShootingVehiclePetrolCap(CEntity *shooter, CEntity* victim, CColPoint point)
{
	if (!victim->IsVehicle())
		return;

	CVehicle* vehicle = (CVehicle*)victim;

	if (!vehicle)
		return;

	if (vehicle->bRenderScorched)
		return;

	if (vehicle == FindPlayerVehicle())
		return;

	if (!vehicle->IsCar() && !vehicle->IsBike())
		return;

	CVehicleModelInfo* mi = (CVehicleModelInfo*)CModelInfo::GetModelInfo(vehicle->GetModelIndex());

	CAutomobile* automobile = (CAutomobile*)vehicle;
	CVector petrolCapPos = mi->m_positions[CAR_POS_PETROLCAP];
	petrolCapPos = vehicle->GetMatrix() * petrolCapPos;

	float radius = 0.03f;
	float distanceSqr = (point.point - petrolCapPos).MagnitudeSqr();
	if (distanceSqr > radius)
		return;

	vehicle->BlowUpCar(shooter);
}
#endif

#ifdef COMPATIBLE_SAVES
#define CopyFromBuf(buf, data) memcpy(&data, buf, sizeof(data)); SkipSaveBuf(buf, sizeof(data));
#define CopyToBuf(buf, data) memcpy(buf, &data, sizeof(data)); SkipSaveBuf(buf, sizeof(data));
void
CWeapon::Save(uint8*& buf)
{
	CopyToBuf(buf, m_eWeaponType);
	CopyToBuf(buf, m_eWeaponState);
	CopyToBuf(buf, m_nAmmoInClip);
	CopyToBuf(buf, m_nAmmoTotal);
	CopyToBuf(buf, m_nTimer);
	CopyToBuf(buf, m_bAddRotOffset);
	ZeroSaveBuf(buf, 3);
}

void
CWeapon::Load(uint8*& buf)
{
	CopyFromBuf(buf, m_eWeaponType);
	CopyFromBuf(buf, m_eWeaponState);
	CopyFromBuf(buf, m_nAmmoInClip);
	CopyFromBuf(buf, m_nAmmoTotal);
	CopyFromBuf(buf, m_nTimer);
	CopyFromBuf(buf, m_bAddRotOffset);
	SkipSaveBuf(buf, 3);
}

#undef CopyFromBuf
#undef CopyToBuf
#endif
