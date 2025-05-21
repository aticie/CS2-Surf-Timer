#pragma once

#include <core/forwards.h>

using CEdictBitVec = CBitVec<MAX_EDICTS>;

class CHidePlugin : CFeatureForward, CCoreForward {
private:
	virtual void OnPluginStart() override;
	virtual void OnClientSendSnapshotBefore(CServerSideClient* pClient) override;

public:
	void Set(CBasePlayerController* pOwner, CBaseEntity* pTarget, bool bShouldHide);
	void SetExclude(CBasePlayerController* pExcludedOwner, CBaseEntity* pTarget, bool bShouldHide);
	void Remove(CBasePlayerController* pOwner, CBaseEntity* pTarget);
	void Reset(CBasePlayerController* pOwner);

private:
	void ClearEntityTransmitInfo(CEdictBitVec* pTransmitEntity, CBaseEntity* pEntity);
	void ClearEntityChildEntities(CEdictBitVec* pTransmitEntity, CBaseEntity* pEntity);

private:
	static void OnPlayerDeath(IGameEvent* pEvent, const char* szName, bool bDontBroadcast);

private:
	std::array<CEdictBitVec, MAXPLAYERS> m_vBlockTransmit;
};

namespace SURF {
	namespace MISC {
		extern CHidePlugin* HidePlugin();
	} // namespace MISC
} // namespace SURF
