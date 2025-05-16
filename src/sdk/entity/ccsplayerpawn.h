#pragma once

#include "cbaseplayerpawn.h"

class CCSPlayer_ViewModelServices;
class CCSBot;
class CCSPlayerController;

class CCSPlayerPawnBase : public CBasePlayerPawn {
public:
	DECLARE_SCHEMA_CLASS(CCSPlayerPawnBase);

	SCHEMA_FIELD(CCSPlayer_ViewModelServices*, m_pViewModelServices);
	SCHEMA_FIELD(QAngle, m_angEyeAngles);
	SCHEMA_FIELD(CHandle<CCSPlayerController>, m_hOriginalController);

	Vector GetEyePosition();

	CBaseViewModel* EnsureViewModel(int vmSlot = 1);
};

class CCSPlayerPawn : public CCSPlayerPawnBase {
public:
	DECLARE_SCHEMA_CLASS(CCSPlayerPawn);
	SCHEMA_FIELD(float, m_ignoreLadderJumpTime);
	SCHEMA_FIELD(float, m_flSlopeDropOffset);
	SCHEMA_FIELD(float, m_flSlopeDropHeight);
	SCHEMA_FIELD(float, m_flVelocityModifier);
	SCHEMA_FIELD(CCSBot*, m_pBot);
	SCHEMA_FIELD(QAngle, m_aimPunchAngle);

public:
	template<typename T = CBasePlayerController>
	T* GetController() {
		return (T*)(m_hController()->Get());
	}

	void Respawn() {
		CALL_VIRTUAL(void, GAMEDATA::GetOffset("Respawn"), this);
	}
};

class CBot {
public:
	DECLARE_SCHEMA_CLASS(CBot);

	SCHEMA_FIELD(uint64_t, m_buttonFlags);
	SCHEMA_FIELD(CCSPlayerPawn*, m_pPlayer);
	SCHEMA_FIELD(CCSPlayerController*, m_pController);
};

class CCSBot : public CBot {
public:
	DECLARE_SCHEMA_CLASS(CCSBot);
};
