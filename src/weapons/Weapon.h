#pragma once

#include "WeaponType.h"

#define DRIVEBYAUTOAIMING_MAXDIST (2.5f)
#define DOOMAUTOAIMING_MAXDIST    (9000.0f)

class CEntity;
class CPhysical;
class CAutomobile;
struct CColPoint;
class CWeaponInfo;

class CWeapon
{
public:
	eWeaponType m_eWeaponType;
	eWeaponState m_eWeaponState;
	int32 m_nAmmoInClip;
	int32 m_nAmmoTotal;
	uint32 m_nTimer;
	bool m_bAddRotOffset;

	CWeapon() {
		m_bAddRotOffset = false;
	}
	
	CWeaponInfo *GetInfo();

	static void InitialiseWeapons(void);
	static void ShutdownWeapons  (void);
	static void UpdateWeapons    (void);
	
	void Initialise(eWeaponType type, int32 ammo);
	
	bool Fire          (CEntity *shooter, CVector *fireSource);
	bool FireFromCar   (CAutomobile *shooter, bool left);
	bool FireMelee     (CEntity *shooter, CVector &fireSource);
	bool FireInstantHit(CEntity *shooter, CVector *fireSource);
	
	void AddGunshell   (CEntity *shooter, CVector const &source, CVector2D const &direction, float size);
	void DoBulletImpact(CEntity *shooter, CEntity *victim, CVector *source, CVector *target, CColPoint *point, CVector2D ahead);
	
	bool FireShotgun   (CEntity *shooter, CVector *fireSource);
	bool FireProjectile(CEntity *shooter, CVector *fireSource, float power);
	
	static void GenerateFlameThrowerParticles(CVector pos, CVector dir);
	
	bool FireAreaEffect       (CEntity *shooter, CVector *fireSource);
	bool FireSniper           (CEntity *shooter);
	bool FireM16_1stPerson    (CEntity *shooter);
	bool FireInstantHitFromCar(CAutomobile *shooter, bool left);
	
	static void DoDoomAiming       (CEntity *shooter, CVector *source, CVector *target);
	static void DoTankDoomAiming   (CEntity *shooter, CEntity *driver, CVector *source, CVector *target);
	static void DoDriveByAutoAiming(CEntity *shooter, CVector *source, CVector *target);
	
	void Reload(void);
	void Update(int32 audioEntity);
	bool IsTypeMelee  (void);
	bool IsType2Handed(void);
	
	static void MakePedsJumpAtShot(CPhysical *shooter, CVector *source, CVector *target);
	
	bool HitsGround(CEntity *holder, CVector *fireSource, CEntity *aimingTo);
	static void BlowUpExplosiveThings(CEntity *thing);
	bool HasWeaponAmmoToBeUsed(void);
	
	static bool ProcessLineOfSight(CVector const &point1, CVector const &point2, CColPoint &point, CEntity *&entity, eWeaponType type, CEntity *shooter, bool checkBuildings, bool checkVehicles, bool checkPeds, bool checkObjects, bool checkDummies, bool ignoreSeeThrough, bool ignoreSomeObjects);

#ifdef EX_BREAKABLE_WINDSHIELDS
	static void CheckForShootingVehicleOccupant(CEntity** victim, CColPoint* point, eWeaponType weapon, CVector const& source, CVector const& target);
#endif

#ifdef COMPATIBLE_SAVES
	void Save(uint8*& buf);
	void Load(uint8*& buf);
#endif
};
VALIDATE_SIZE(CWeapon, 0x18);

void FireOneInstantHitRound(CVector *source, CVector *target, int32 damage);