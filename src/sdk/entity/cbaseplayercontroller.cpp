#include "cbaseplayercontroller.h"
#include <core/memory.h>
#include <utils/utils.h>

void CBasePlayerController::SetPlayerName(const char* pszName) {
	this->m_iszPlayerName(pszName);

	CServerSideClient* pClient = UTIL::GetClientBySlot(GetPlayerSlot());
	pClient->SetName(pszName);
}

bool CBasePlayerController::IsObserver() {
	auto pPawn = GetCurrentPawn();
	if (!pPawn) {
		SDK_ASSERT(false);
	}

	return pPawn->IsObserver();
}

void CBasePlayerController::SetPawn(CCSPlayerPawn* pawn) {
	MEM::CALL::SetPawn(this, pawn, true, false, false);
}

void CBasePlayerController::SendFullUpdate() {
	CServerSideClient* pClient = UTIL::GetClientBySlot(GetPlayerSlot());
	if (pClient) {
		pClient->ForceFullUpdate();
	}
}
