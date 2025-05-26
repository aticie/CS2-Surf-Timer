#include "sdkhook.h"
#include <libmem/libmem_helper.h>

#define SDKHOOK_POSTBIND(eType, fnType) template bool SDKHOOK::HookEntity<eType>(CBaseEntity * pEnt, fnType pFn);

SDKHOOK_POSTBIND(SDKHook_StartTouch, SDKHOOK_PRE(HookTouch_t));
SDKHOOK_POSTBIND(SDKHook_StartTouchPost, SDKHOOK_POST(HookTouch_t));
SDKHOOK_POSTBIND(SDKHook_Touch, SDKHOOK_PRE(HookTouch_t));
SDKHOOK_POSTBIND(SDKHook_TouchPost, SDKHOOK_POST(HookTouch_t));
SDKHOOK_POSTBIND(SDKHook_EndTouch, SDKHOOK_PRE(HookTouch_t));
SDKHOOK_POSTBIND(SDKHook_EndTouchPost, SDKHOOK_POST(HookTouch_t));
SDKHOOK_POSTBIND(SDKHook_Teleport, SDKHOOK_PRE(HookTeleport_t));
SDKHOOK_POSTBIND(SDKHook_TeleportPost, SDKHOOK_POST(HookTeleport_t));
SDKHOOK_POSTBIND(SDKHook_Use, SDKHOOK_PRE(HookUse_t));
SDKHOOK_POSTBIND(SDKHook_UsePost, SDKHOOK_POST(HookUse_t));

static bool IsVMTHooked(void* pVtable, uint32_t iOffset) {
	if (auto it = SDKHOOK::m_umVFuncHookMarks.find(pVtable); it != SDKHOOK::m_umVFuncHookMarks.end()) {
		return it->second.contains(iOffset);
	}
	return false;
}

static void AddHooks(std::list<void*>& hookList, void* pCallback) {
	if (std::find(hookList.begin(), hookList.end(), pCallback) == hookList.end()) {
		hookList.emplace_back(pCallback);
	}
}

static void Hook_OnStartTouch(CBaseEntity* pEnt, void* pCallback, bool post) {
	HookTouch_t pHook = [](CBaseEntity* pSelf, CBaseEntity* pOther) {
		void* vtable = *(void**)pSelf;
		HookTouch_t pTrampoline = (HookTouch_t)SDKHOOK::m_umSDKHookTrampolines[SDKHook_StartTouch][vtable];
		SDK_ASSERT(pTrampoline);

		bool block = false;
		const auto& preHooks = SDKHOOK::m_umSDKHooks[SDKHook_StartTouch][vtable];
		for (const auto& pFnPre : preHooks) {
			if (!reinterpret_cast<SDKHOOK_PRE(HookTouch_t)*>(pFnPre)(pSelf, pOther)) {
				block = true;
			}
		}
		if (block) {
			return;
		}

		pTrampoline(pSelf, pOther);

		const auto& postHooks = SDKHOOK::m_umSDKHooks[SDKHook_StartTouchPost][vtable];
		for (const auto& pFnPost : postHooks) {
			reinterpret_cast<SDKHOOK_POST(HookTouch_t)*>(pFnPost)(pSelf, pOther);
		}
	};

	static int iOffset = GAMEDATA::GetOffset("CBaseEntity::StartTouch");
	void* pVtable = *(void**)pEnt;
	AddHooks(SDKHOOK::m_umSDKHooks[post ? SDKHook_StartTouchPost : SDKHook_StartTouch][pVtable], pCallback);

	if (!IsVMTHooked(pVtable, iOffset)) {
		libmem::VmtHookEx(pEnt, iOffset, pHook, SDKHOOK::m_umSDKHookTrampolines[SDKHook_StartTouch][pVtable]);
		SDKHOOK::m_umVFuncHookMarks[pVtable].insert(iOffset);
	}
}

static void Hook_OnTouch(CBaseEntity* pEnt, void* pCallback, bool post) {
	HookTouch_t pHook = [](CBaseEntity* pSelf, CBaseEntity* pOther) {
		void* vtable = *(void**)pSelf;
		HookTouch_t pTrampoline = (HookTouch_t)SDKHOOK::m_umSDKHookTrampolines[SDKHook_Touch][vtable];
		SDK_ASSERT(pTrampoline);

		bool block = false;
		const auto& preHooks = SDKHOOK::m_umSDKHooks[SDKHook_Touch][vtable];
		for (const auto& pFnPre : preHooks) {
			if (!reinterpret_cast<SDKHOOK_PRE(HookTouch_t)*>(pFnPre)(pSelf, pOther)) {
				block = true;
			}
		}
		if (block) {
			return;
		}

		pTrampoline(pSelf, pOther);

		const auto& postHooks = SDKHOOK::m_umSDKHooks[SDKHook_TouchPost][vtable];
		for (const auto& pFnPost : postHooks) {
			reinterpret_cast<SDKHOOK_POST(HookTouch_t)*>(pFnPost)(pSelf, pOther);
		}
	};

	static int iOffset = GAMEDATA::GetOffset("CBaseEntity::Touch");
	void* pVtable = *(void**)pEnt;
	AddHooks(SDKHOOK::m_umSDKHooks[post ? SDKHook_TouchPost : SDKHook_Touch][pVtable], pCallback);

	if (!IsVMTHooked(pVtable, iOffset)) {
		libmem::VmtHookEx(pEnt, iOffset, pHook, SDKHOOK::m_umSDKHookTrampolines[SDKHook_Touch][pVtable]);
		SDKHOOK::m_umVFuncHookMarks[pVtable].insert(iOffset);
	}
}

static void Hook_OnEndTouch(CBaseEntity* pEnt, void* pCallback, bool post) {
	HookTouch_t pHook = [](CBaseEntity* pSelf, CBaseEntity* pOther) {
		void* vtable = *(void**)pSelf;
		HookTouch_t pTrampoline = (HookTouch_t)SDKHOOK::m_umSDKHookTrampolines[SDKHook_EndTouch][vtable];
		SDK_ASSERT(pTrampoline);

		bool block = false;
		const auto& preHooks = SDKHOOK::m_umSDKHooks[SDKHook_EndTouch][vtable];
		for (const auto& pFnPre : preHooks) {
			if (!reinterpret_cast<SDKHOOK_PRE(HookTouch_t)*>(pFnPre)(pSelf, pOther)) {
				block = true;
			}
		}
		if (block) {
			return;
		}

		pTrampoline(pSelf, pOther);

		const auto& postHooks = SDKHOOK::m_umSDKHooks[SDKHook_EndTouchPost][vtable];
		for (const auto& pFnPost : postHooks) {
			reinterpret_cast<SDKHOOK_POST(HookTouch_t)*>(pFnPost)(pSelf, pOther);
		}
	};

	static int iOffset = GAMEDATA::GetOffset("CBaseEntity::EndTouch");
	void* pVtable = *(void**)pEnt;
	AddHooks(SDKHOOK::m_umSDKHooks[post ? SDKHook_EndTouchPost : SDKHook_EndTouch][pVtable], pCallback);

	if (!IsVMTHooked(pVtable, iOffset)) {
		libmem::VmtHookEx(pEnt, iOffset, pHook, SDKHOOK::m_umSDKHookTrampolines[SDKHook_EndTouch][pVtable]);
		SDKHOOK::m_umVFuncHookMarks[pVtable].insert(iOffset);
	}
}

static void Hook_OnTeleport(CBaseEntity* pEnt, void* pCallback, bool post) {
	HookTeleport_t pHook = [](CBaseEntity* pSelf, Vector* newPosition, QAngle* newAngles, Vector* newVelocity) {
		void* vtable = *(void**)pSelf;
		HookTeleport_t pTrampoline = (HookTeleport_t)SDKHOOK::m_umSDKHookTrampolines[SDKHook_Teleport][vtable];
		SDK_ASSERT(pTrampoline);

		bool block = false;
		const auto& preHooks = SDKHOOK::m_umSDKHooks[SDKHook_Teleport][vtable];
		for (const auto& pFnPre : preHooks) {
			if (!reinterpret_cast<SDKHOOK_PRE(HookTeleport_t)*>(pFnPre)(pSelf, newPosition, newAngles, newVelocity)) {
				block = true;
			}
		}
		if (block) {
			return;
		}

		pTrampoline(pSelf, newPosition, newAngles, newVelocity);

		const auto& postHooks = SDKHOOK::m_umSDKHooks[SDKHook_TeleportPost][vtable];
		for (const auto& pFnPost : postHooks) {
			reinterpret_cast<SDKHOOK_POST(HookTeleport_t)*>(pFnPost)(pSelf, newPosition, newAngles, newVelocity);
		}
	};

	static int iOffset = GAMEDATA::GetOffset("CBaseEntity::Teleport");
	void* pVtable = *(void**)pEnt;
	AddHooks(SDKHOOK::m_umSDKHooks[post ? SDKHook_TeleportPost : SDKHook_Teleport][pVtable], pCallback);

	if (!IsVMTHooked(pVtable, iOffset)) {
		libmem::VmtHookEx(pEnt, iOffset, pHook, SDKHOOK::m_umSDKHookTrampolines[SDKHook_Teleport][pVtable]);
		SDKHOOK::m_umVFuncHookMarks[pVtable].insert(iOffset);
	}
}

static void Hook_OnUse(CBaseEntity* pEnt, void* pCallback, bool post) {
	HookUse_t pHook = [](CBaseEntity* pSelf, EntityInputData_t* pInput) {
		void* vtable = *(void**)pSelf;
		HookUse_t pTrampoline = (HookUse_t)SDKHOOK::m_umSDKHookTrampolines[SDKHook_Use][vtable];
		SDK_ASSERT(pTrampoline);

		bool block = false;
		const auto& preHooks = SDKHOOK::m_umSDKHooks[SDKHook_Use][vtable];
		for (const auto& pFnPre : preHooks) {
			if (!reinterpret_cast<SDKHOOK_PRE(HookUse_t)*>(pFnPre)(pSelf, pInput)) {
				block = true;
			}
		}
		if (block) {
			return;
		}

		pTrampoline(pSelf, pInput);

		const auto& postHooks = SDKHOOK::m_umSDKHooks[SDKHook_UsePost][vtable];
		for (const auto& pFnPost : postHooks) {
			reinterpret_cast<SDKHOOK_POST(HookUse_t)*>(pFnPost)(pSelf, pInput);
		}
	};

	static int iOffset = GAMEDATA::GetOffset("CBaseEntity::Use");
	void* pVtable = *(void**)pEnt;
	AddHooks(SDKHOOK::m_umSDKHooks[post ? SDKHook_UsePost : SDKHook_Use][pVtable], pCallback);

	if (!IsVMTHooked(pVtable, iOffset)) {
		libmem::VmtHookEx(pEnt, iOffset, pHook, SDKHOOK::m_umSDKHookTrampolines[SDKHook_Use][pVtable]);
		SDKHOOK::m_umVFuncHookMarks[pVtable].insert(iOffset);
	}
}

template<SDKHookType T>
bool SDKHOOK::HookEntity(CBaseEntity* pEnt, typename SDKHookBindings<T>::Type pFn) {
	auto pBind = (void*&)pFn;

	switch (T) {
		case SDKHook_StartTouch:
			::Hook_OnStartTouch(pEnt, pBind, false);
			break;
		case SDKHook_StartTouchPost:
			::Hook_OnStartTouch(pEnt, pBind, true);
			break;
		case SDKHook_Touch:
			::Hook_OnTouch(pEnt, pBind, false);
			break;
		case SDKHook_TouchPost:
			::Hook_OnTouch(pEnt, pBind, true);
			break;
		case SDKHook_EndTouch:
			::Hook_OnEndTouch(pEnt, pBind, false);
			break;
		case SDKHook_EndTouchPost:
			::Hook_OnEndTouch(pEnt, pBind, true);
			break;
		case SDKHook_Teleport:
			::Hook_OnTeleport(pEnt, pBind, false);
			break;
		case SDKHook_TeleportPost:
			::Hook_OnTeleport(pEnt, pBind, true);
			break;
		case SDKHook_Use:
			::Hook_OnUse(pEnt, pBind, false);
			break;
		case SDKHook_UsePost:
			::Hook_OnUse(pEnt, pBind, true);
			break;
		default:
			return false;
	}

	return true;
}
