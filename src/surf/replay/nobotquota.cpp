#include <pch.h>
#include <core/memory.h>

class CNoBotQuotaPlugin : CCoreForward {
	virtual void OnPluginStart() override;
};

CNoBotQuotaPlugin g_NoBotQuota;

void* g_fnMaintainBotQuota;

static void Hook_OnMaintainBotQuota() {}

void CNoBotQuotaPlugin::OnPluginStart() {
	HOOK_SIG("CCSBotManager::MaintainBotQuota", Hook_OnMaintainBotQuota, g_fnMaintainBotQuota);
}
