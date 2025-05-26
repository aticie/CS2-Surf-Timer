#pragma once

#include <pch.h>
#include <utils/typehelper.h>
#include <list>

// ======================== //
using SDKHookRet_Pre = bool;
using SDKHookRet_Post = void;

#define SDKHOOK_BIND(eType, fnType) \
	template<> \
	struct SDKHookBindings<eType> { \
		using Type = fnType; \
	};

#define SDKHOOK_PRE(fnType)  RebindFunction_t<fnType, SDKHookRet_Pre>
#define SDKHOOK_POST(fnType) RebindFunction_t<fnType, SDKHookRet_Post>
// ======================== //

using HookTouch_t = void (*)(CBaseEntity* pSelf, CBaseEntity* pOther);
using HookTeleport_t = void (*)(CBaseEntity* pSelf, Vector* newPosition, QAngle* newAngles, Vector* newVelocity);
using HookUse_t = void (*)(CBaseEntity* pSelf, EntityInputData_t* pInput);

enum SDKHookType {
	SDKHook_StartTouch,
	SDKHook_StartTouchPost,
	SDKHook_Touch,
	SDKHook_TouchPost,
	SDKHook_EndTouch,
	SDKHook_EndTouchPost,
	SDKHook_Teleport,
	SDKHook_TeleportPost,
	SDKHook_Use,
	SDKHook_UsePost,
	MAX_TYPE
};

namespace SDKHOOK {
	template<SDKHookType T>
	struct SDKHookBindings;

	SDKHOOK_BIND(SDKHook_StartTouch, SDKHOOK_PRE(HookTouch_t));
	SDKHOOK_BIND(SDKHook_StartTouchPost, SDKHOOK_POST(HookTouch_t));
	SDKHOOK_BIND(SDKHook_Touch, SDKHOOK_PRE(HookTouch_t));
	SDKHOOK_BIND(SDKHook_TouchPost, SDKHOOK_POST(HookTouch_t));
	SDKHOOK_BIND(SDKHook_EndTouch, SDKHOOK_PRE(HookTouch_t));
	SDKHOOK_BIND(SDKHook_EndTouchPost, SDKHOOK_POST(HookTouch_t));
	SDKHOOK_BIND(SDKHook_Teleport, SDKHOOK_PRE(HookTeleport_t));
	SDKHOOK_BIND(SDKHook_TeleportPost, SDKHOOK_POST(HookTeleport_t));
	SDKHOOK_BIND(SDKHook_Use, SDKHOOK_PRE(HookUse_t));
	SDKHOOK_BIND(SDKHook_UsePost, SDKHOOK_POST(HookUse_t));

	template<SDKHookType T>
	bool HookEntity(CBaseEntity* pEnt, typename SDKHookBindings<T>::Type pFn);

	// vtable for key
	inline std::unordered_map<void*, std::unordered_set<uint32_t>> m_umVFuncHookMarks;
	inline std::unordered_map<void*, std::list<void*>> m_umSDKHooks[SDKHookType::MAX_TYPE];
	inline std::unordered_map<void*, void*> m_umSDKHookTrampolines[SDKHookType::MAX_TYPE];
} // namespace SDKHOOK
