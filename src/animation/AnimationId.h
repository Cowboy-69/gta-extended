#pragma once

enum AnimationId
{
	ANIM_STD_WALK,
	ANIM_STD_RUN,
	ANIM_STD_RUNFAST,
	ANIM_STD_IDLE,
	ANIM_STD_STARTWALK,
	ANIM_STD_RUNSTOP1,
	ANIM_STD_RUNSTOP2,
	ANIM_STD_IDLE_CAM,
	ANIM_STD_IDLE_HBHB,
	ANIM_STD_IDLE_TIRED,
	ANIM_STD_IDLE_BIGGUN,
	ANIM_STD_CHAT,
	ANIM_STD_HAILTAXI,
	ANIM_STD_KO_FRONT,
	ANIM_STD_KO_LEFT,
	ANIM_STD_KO_BACK,
	ANIM_STD_KO_RIGHT,
	ANIM_STD_KO_SHOT_FACE,
	ANIM_STD_KO_SHOT_STOMACH,
	ANIM_STD_KO_SHOT_ARM_L,
	ANIM_STD_KO_SHOT_ARM_R,
	ANIM_STD_KO_SHOT_LEG_L,
	ANIM_STD_KO_SHOT_LEG_R,
	ANIM_STD_SPINFORWARD_LEFT,
	ANIM_STD_SPINFORWARD_RIGHT,
	ANIM_STD_HIGHIMPACT_FRONT,
	ANIM_STD_HIGHIMPACT_LEFT,
	ANIM_STD_HIGHIMPACT_BACK,
	ANIM_STD_HIGHIMPACT_RIGHT,
	ANIM_STD_HITBYGUN_FRONT,
	ANIM_STD_HITBYGUN_LEFT,
	ANIM_STD_HITBYGUN_BACK,
	ANIM_STD_HITBYGUN_RIGHT,
	ANIM_STD_HIT_FRONT,
	ANIM_STD_HIT_LEFT,
	ANIM_STD_HIT_BACK,
	ANIM_STD_HIT_RIGHT,
	ANIM_STD_HIT_FLOOR,

	/* names made up */
	ANIM_STD_HIT_BODYBLOW,
	ANIM_STD_HIT_CHEST,
	ANIM_STD_HIT_HEAD,
	ANIM_STD_HIT_WALK,
	/**/

	ANIM_STD_HIT_WALL,
	ANIM_STD_HIT_FLOOR_FRONT,
	ANIM_STD_HIT_BEHIND,
	ANIM_STD_FIGHT_IDLE,
	ANIM_STD_FIGHT_2IDLE,
	ANIM_STD_FIGHT_SHUFFLE_F,

	/* names made up */
	ANIM_STD_FIGHT_BODYBLOW,
	ANIM_STD_FIGHT_HEAD,
	ANIM_STD_FIGHT_KICK,
	ANIM_STD_FIGHT_KNEE,
	ANIM_STD_FIGHT_LHOOK,
	ANIM_STD_FIGHT_PUNCH,
	ANIM_STD_FIGHT_ROUNDHOUSE,
	ANIM_STD_FIGHT_LONGKICK,
	/**/

	ANIM_STD_PARTIAL_PUNCH,

	/* names made up */
	ANIM_STD_FIGHT_JAB,
	ANIM_STD_FIGHT_ELBOW_L,
	ANIM_STD_FIGHT_ELBOW_R,
	ANIM_STD_FIGHT_BKICK_L,
	ANIM_STD_FIGHT_BKICK_R,
	/**/

	ANIM_STD_DETONATE,
	ANIM_STD_PUNCH,
	ANIM_STD_PARTIALPUNCH,
	ANIM_STD_KICKGROUND,

	ANIM_STD_THROW_UNDER,
	ANIM_STD_FIGHT_SHUFFLE_B,

	ANIM_STD_JACKEDCAR_RHS,
	ANIM_STD_JACKEDCAR_LO_RHS,
	ANIM_STD_JACKEDCAR_LHS,
	ANIM_STD_JACKEDCAR_LO_LHS,
	ANIM_STD_QUICKJACK,
	ANIM_STD_QUICKJACKED,
	ANIM_STD_CAR_ALIGN_DOOR_LHS,
	ANIM_STD_CAR_ALIGNHI_DOOR_LHS,
	ANIM_STD_CAR_OPEN_DOOR_LHS,
	ANIM_STD_CARDOOR_LOCKED_LHS,
	ANIM_STD_CAR_PULL_OUT_PED_LHS,
	ANIM_STD_CAR_PULL_OUT_PED_LO_LHS,
	ANIM_STD_CAR_GET_IN_LHS,
	ANIM_STD_CAR_GET_IN_LO_LHS,
	ANIM_STD_CAR_CLOSE_DOOR_LHS,
	ANIM_STD_CAR_CLOSE_DOOR_LO_LHS,
	ANIM_STD_CAR_CLOSE_DOOR_ROLLING_LHS,
	ANIM_STD_CAR_CLOSE_DOOR_ROLLING_LO_LHS,
	ANIM_STD_CAR_JUMP_IN_LO_LHS,
	ANIM_STD_GETOUT_LHS,
	ANIM_STD_GETOUT_LO_LHS,
	ANIM_STD_CAR_CLOSE_LHS,
	ANIM_STD_CAR_ALIGN_DOOR_RHS,
	ANIM_STD_CAR_ALIGNHI_DOOR_RHS,
	ANIM_STD_CAR_OPEN_DOOR_RHS,
	ANIM_STD_CARDOOR_LOCKED_RHS,
	ANIM_STD_CAR_PULL_OUT_PED_RHS,
	ANIM_STD_CAR_PULL_OUT_PED_LO_RHS,
	ANIM_STD_CAR_GET_IN_RHS,
	ANIM_STD_CAR_GET_IN_LO_RHS,
	ANIM_STD_CAR_CLOSE_DOOR_RHS,
	ANIM_STD_CAR_CLOSE_DOOR_LO_RHS,
	ANIM_STD_CAR_SHUFFLE_RHS,
	ANIM_STD_CAR_SHUFFLE_LO_RHS,
	ANIM_STD_CAR_SIT,
	ANIM_STD_CAR_SIT_LO,
	ANIM_STD_CAR_SIT_P,
	ANIM_STD_CAR_SIT_P_LO,
	ANIM_STD_CAR_DRIVE_LEFT,
	ANIM_STD_CAR_DRIVE_RIGHT,
	ANIM_STD_CAR_DRIVE_LEFT_LO,
	ANIM_STD_CAR_DRIVE_RIGHT_LO,
	ANIM_STD_CAR_DRIVEBY_LEFT,
	ANIM_STD_CAR_DRIVEBY_RIGHT,
	ANIM_STD_CAR_DRIVEBY_LEFT_LO,
	ANIM_STD_CAR_DRIVEBY_RIGHT_LO,
	ANIM_STD_CAR_LOOKBEHIND,
	ANIM_STD_BOAT_DRIVE,
	ANIM_STD_BOAT_DRIVE_LEFT,
	ANIM_STD_BOAT_DRIVE_RIGHT,
	ANIM_STD_BOAT_LOOKBEHIND,

	ANIM_STD_BIKE_PICKUP_LHS,
	ANIM_STD_BIKE_PICKUP_RHS,
	ANIM_STD_BIKE_PULLUP_LHS,
	ANIM_STD_BIKE_PULLUP_RHS,
	ANIM_STD_BIKE_ELBOW_LHS,
	ANIM_STD_BIKE_ELBOW_RHS,
	ANIM_STD_BIKE_FALLOFF,
	ANIM_STD_BIKE_FALLBACK,

	ANIM_STD_GETOUT_RHS,
	ANIM_STD_GETOUT_LO_RHS,
	ANIM_STD_CAR_CLOSE_RHS,
	ANIM_STD_CAR_HOOKERTALK,

	ANIM_STD_TRAIN_GETIN,
	ANIM_STD_TRAIN_GETOUT,

	ANIM_STD_CRAWLOUT_LHS,
	ANIM_STD_CRAWLOUT_RHS,
	ANIM_STD_ROLLOUT_LHS,
	ANIM_STD_ROLLOUT_RHS,

	ANIM_STD_GET_UP,
	ANIM_STD_GET_UP_LEFT,
	ANIM_STD_GET_UP_RIGHT,
	ANIM_STD_GET_UP_FRONT,
	ANIM_STD_JUMP_LAUNCH,
	ANIM_STD_JUMP_GLIDE,
	ANIM_STD_JUMP_LAND,
	ANIM_STD_FALL,
	ANIM_STD_FALL_GLIDE,
	ANIM_STD_FALL_LAND,
	ANIM_STD_FALL_COLLAPSE,
	ANIM_STD_FALL_ONBACK,
	ANIM_STD_FALL_ONFRONT,

	ANIM_STD_EVADE_STEP,
	ANIM_STD_EVADE_DIVE,
	ANIM_STD_XPRESS_SCRATCH,
	ANIM_STD_ROADCROSS,
	ANIM_STD_TURN180,
	ANIM_STD_ARREST,
	ANIM_STD_DROWN,
	ANIM_STD_DUCK_DOWN,
	ANIM_STD_DUCK_LOW,

	ANIM_STD_DUCK_WEAPON,

	ANIM_STD_RBLOCK_SHOOT,
	ANIM_STD_HANDSUP,
	ANIM_STD_HANDSCOWER,
	ANIM_STD_PARTIAL_FUCKU,
	ANIM_STD_PHONE_IN,
	ANIM_STD_PHONE_OUT,
	ANIM_STD_PHONE_TALK,

	ANIM_STD_SEAT_DOWN,
	ANIM_STD_SEAT_UP,
	ANIM_STD_SEAT_IDLE,
	ANIM_STD_SEAT_RVRS,
	ANIM_STD_ATM,
	ANIM_STD_ABSEIL,

#ifdef CLIMBING // add anims
	ANIM_STD_CLIMBING_IDLE,
	ANIM_STD_CLIMBING_JUMP,
	ANIM_STD_CLIMBING_JUMP_B,
	ANIM_STD_CLIMBING_PULL,
	ANIM_STD_CLIMBING_STAND,
	ANIM_STD_CLIMBING_STAND_FINISH,
#endif

#ifdef CROUCH // add anims
	ANIM_STD_CROUCH_IDLE,
	ANIM_STD_CROUCH_FORWARD,
	ANIM_STD_CROUCH_BACKWARD,
	ANIM_STD_CROUCH_ROLL_L,
	ANIM_STD_CROUCH_ROLL_R,
#endif

#ifdef SWIMMING // add anims
	ANIM_STD_SWIM_BREAST,
	ANIM_STD_SWIM_CRAWL,
	ANIM_STD_SWIM_JUMPOUT,
	ANIM_STD_SWIM_TREAD,
#endif

#ifdef FIRING_AND_AIMING // add anims
	ANIM_STD_GUNMOVE_BWD,
	ANIM_STD_GUNMOVE_FWD,
	ANIM_STD_GUNMOVE_L,
	ANIM_STD_GUNMOVE_R,
	ANIM_STD_GUN_STAND,

	ANIM_STD_CAR_SIT_DB,
	ANIM_STD_CAR_SIT_LO_DB,
	ANIM_STD_CAR_SIT_P_DB,
	ANIM_STD_CAR_SIT_P_LO_DB,
	ANIM_STD_CAR_DRIVE_LEFT_DB,
	ANIM_STD_CAR_DRIVE_RIGHT_DB,
	ANIM_STD_CAR_DRIVE_LEFT_LO_DB,
	ANIM_STD_CAR_DRIVE_RIGHT_LO_DB,
	ANIM_STD_BOAT_DRIVE_DB,
	ANIM_STD_BOAT_DRIVE_LEFT_DB,
	ANIM_STD_BOAT_DRIVE_RIGHT_DB,
#endif

#ifdef EX_PED_ANIMS_IN_CAR
	ANIM_STD_CAR_DIE_HORN_DS,
	ANIM_STD_CAR_DIE_DS,
	ANIM_STD_CAR_DIE_PS,
#endif

	ANIM_STD_NUM,

	ANIM_STD_VAN_OPEN_DOOR_REAR_LHS,
	ANIM_STD_VAN_GET_IN_REAR_LHS,
	ANIM_STD_VAN_CLOSE_DOOR_REAR_LHS,
	ANIM_STD_VAN_GET_OUT_REAR_LHS,
	ANIM_STD_VAN_OPEN_DOOR_REAR_RHS,
	ANIM_STD_VAN_GET_IN_REAR_RHS,
	ANIM_STD_VAN_CLOSE_DOOR_REAR_RHS,
	ANIM_STD_VAN_GET_OUT_REAR_RHS,

	ANIM_STD_COACH_OPEN_LHS,
	ANIM_STD_COACH_OPEN_RHS,
	ANIM_STD_COACH_GET_IN_LHS,
	ANIM_STD_COACH_GET_IN_RHS,
	ANIM_STD_COACH_GET_OUT_LHS,

	ANIM_BIKE_RIDE,
	ANIM_BIKE_READY,
	ANIM_BIKE_LEFT,
	ANIM_BIKE_RIGHT,
	ANIM_BIKE_LEANB,
	ANIM_BIKE_LEANF,
	ANIM_BIKE_WALKBACK,
	ANIM_BIKE_JUMPON_LHS,
	ANIM_BIKE_JUMPON_RHS,
	ANIM_BIKE_KICK,
	ANIM_BIKE_HIT,
	ANIM_BIKE_GETOFF_LHS,
	ANIM_BIKE_GETOFF_RHS,
	ANIM_BIKE_GETOFF_BACK,
	ANIM_BIKE_DRIVEBY_LHS,
	ANIM_BIKE_DRIVEBY_RHS,
	ANIM_BIKE_DRIVEBY_FORWARD,
	ANIM_BIKE_RIDE_P,
#ifdef FIRING_AND_AIMING // add anims
	ANIM_BIKE_RIDE_DB,
	ANIM_BIKE_READY_DB,
	ANIM_BIKE_LEFT_DB,
	ANIM_BIKE_RIGHT_DB,
	ANIM_BIKE_WALKBACK_DB,
	ANIM_BIKE_RIDE_P_DB,
#endif

	ANIM_ATTACK_1,
	ANIM_ATTACK_2,
	ANIM_ATTACK_EXTRA1,
	ANIM_ATTACK_EXTRA2,
	ANIM_ATTACK_3,

	// our synonyms... because originals are hard to understand
	ANIM_WEAPON_FIRE = ANIM_ATTACK_1,
	ANIM_WEAPON_CROUCHFIRE,
	ANIM_WEAPON_FIRE_2ND = ANIM_WEAPON_CROUCHFIRE,
	ANIM_WEAPON_RELOAD,
	ANIM_WEAPON_CROUCHRELOAD,
	ANIM_WEAPON_FIRE_3RD,
	ANIM_THROWABLE_THROW = ANIM_ATTACK_1,
	ANIM_THROWABLE_THROWU,
	ANIM_THROWABLE_START_THROW,
	ANIM_MELEE_ATTACK = ANIM_ATTACK_1,
	ANIM_MELEE_ATTACK_2ND,
	ANIM_MELEE_ATTACK_START,
	ANIM_MELEE_IDLE_FIGHTMODE,
	ANIM_MELEE_ATTACK_FINISH,

	ANIM_SUNBATHE_IDLE,
	ANIM_SUNBATHE_DOWN,
	ANIM_SUNBATHE_UP,
	ANIM_SUNBATHE_ESCAPE,

	ANIM_MEDIC_CPR,

	ANIM_PLAYER_IDLE1,
	ANIM_PLAYER_IDLE2,
	ANIM_PLAYER_IDLE3,
	ANIM_PLAYER_IDLE4,

	ANIM_RIOT_ANGRY,
	ANIM_RIOT_ANGRY_B,
	ANIM_RIOT_CHANT,
	ANIM_RIOT_PUNCHES,
	ANIM_RIOT_SHOUT,
	ANIM_RIOT_CHALLENGE,
	ANIM_RIOT_FUCKYOU,

	ANIM_STRIP_A,
	ANIM_STRIP_B,
	ANIM_STRIP_C,
	ANIM_STRIP_D,
	ANIM_STRIP_E,
	ANIM_STRIP_F,
	ANIM_STRIP_G,

#ifdef EX_IMPROVED_WEAPONS
	ANIM_FOR_WEAPON_SLIDE,
#endif
};