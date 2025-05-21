#include "ccsplayercontroller.h"

void CCSPlayerController::ChangeTeam(int iTeam) {
	static auto iOffset = GAMEDATA::GetOffset("ControllerChangeTeam");
	CALL_VIRTUAL(void, iOffset, this, iTeam);
}

void CCSPlayerController::SwitchTeam(int iTeam) {
	if (!IsController()) {
		return;
	}

	if (iTeam <= CS_TEAM_SPECTATOR) {
		ChangeTeam(iTeam);
	} else {
		MEM::CALL::SwitchTeam(this, iTeam);
	}
}

void CCSPlayerController::Respawn() {
	CCSPlayerPawn* pawn = GetPlayerPawn();
	if (!pawn || pawn->IsAlive()) {
		return;
	}

	SetPawn(pawn);
	if (this->m_iTeamNum() != CS_TEAM_CT && this->m_iTeamNum() != CS_TEAM_T) {
		SwitchTeam(RandomInt(CS_TEAM_T, CS_TEAM_CT));
	}

	static auto iOffset = GAMEDATA::GetOffset("ControllerRespawn");
	CALL_VIRTUAL(void, iOffset, this);
}

CBaseEntity* CCSPlayerController::GetObserverTarget() {
	auto pPawn = GetCurrentPawn();
	if (!pPawn) {
		SDK_ASSERT(false);
		return nullptr;
	}

	if (!pPawn->IsObserver()) {
		return this;
	}

	auto pObsService = pPawn->m_pObserverServices();
	auto iObsMode = pObsService->m_iObserverMode();
	if (iObsMode >= OBS_MODE_IN_EYE && iObsMode <= OBS_MODE_CHASE) {
		return pObsService->m_hObserverTarget()->Get();
	}

	return nullptr;
}
