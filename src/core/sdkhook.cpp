#include "sdkhook.h"
#include <libmem/libmem_helper.h>

#include <unordered_set>
#include <utility>
#include <list>

namespace SDKHOOK {
	template<SDKHookType T>
	void InstantiateHookType() {
		using PreFunc = bool (*)(CBaseEntity*, typename SDKHookBindings<T>::Pre);
		using PostFunc = bool (*)(CBaseEntity*, typename SDKHookBindings<T>::Post);

		volatile PreFunc pre_ptr = static_cast<PreFunc>(&SDKHOOK::HookEntity<T>);
		volatile PostFunc post_ptr = static_cast<PostFunc>(&SDKHOOK::HookEntity<T>);
	}

	template<int... Is>
	void ForceInstantiation(std::integer_sequence<int, Is...>) {
		int dummy[] = {(InstantiateHookType<static_cast<SDKHookType>(Is)>(), 0)...};
	}

	static auto HookInstantiator = (ForceInstantiation(std::make_integer_sequence<int, SDKHookType::MAX_TYPE> {}), 0);

	// vtable -> hooked vfuncs
	std::unordered_map<void*, std::unordered_set<uint32_t>> s_umVFuncHookMarks {};

	// [type][pre or post]::vtable -> registered callbacks
	std::unordered_map<void*, std::list<void*>> s_umSDKHooks[SDKHookType::MAX_TYPE][2] {};

	// [type]::vtable -> trampoline
	std::unordered_map<void*, void*> s_umSDKHookTrampolines[SDKHookType::MAX_TYPE] {};

	static bool IsVMTHooked(void* pVtable, uint32_t iOffset) {
		if (auto it = SDKHOOK::s_umVFuncHookMarks.find(pVtable); it != SDKHOOK::s_umVFuncHookMarks.end()) {
			return it->second.contains(iOffset);
		}
		return false;
	}

	static void AddHooks(std::list<void*>& hookList, void* pCallback) {
		if (std::find(hookList.begin(), hookList.end(), pCallback) == hookList.end()) {
			hookList.emplace_back(pCallback);
		}
	}

	template<SDKHookType T>
	static void InstallHook(CBaseEntity* pEnt, void* pCallback, bool post) {
		using Traits = SDKHookBindings<T>;
		using HookType_t = typename Traits::HookType;

		HookType_t pHook = [](CBaseEntity* pSelf, auto... args) {
			void* vtable = *(void**)pSelf;
			HookType_t pTrampoline = (HookType_t)SDKHOOK::s_umSDKHookTrampolines[T][vtable];
			SDK_ASSERT(pTrampoline);

			bool block = false;
			const auto& preHooks = SDKHOOK::s_umSDKHooks[T][0][vtable];
			for (const auto& pFnPre : preHooks) {
				if (!reinterpret_cast<SDKHOOK_PRE(HookType_t)*>(pFnPre)(pSelf, args...)) {
					block = true;
				}
			}
			if (block) {
				return;
			}

			pTrampoline(pSelf, args...);

			const auto& postHooks = SDKHOOK::s_umSDKHooks[T][1][vtable];
			for (const auto& pFnPost : postHooks) {
				reinterpret_cast<SDKHOOK_POST(HookType_t)*>(pFnPost)(pSelf, args...);
			}
		};

		static int iOffset = GAMEDATA::GetOffset(Traits::OffsetName);
		void* pVtable = *(void**)pEnt;
		AddHooks(SDKHOOK::s_umSDKHooks[T][post][pVtable], pCallback);

		if (!IsVMTHooked(pVtable, iOffset)) {
			libmem::VmtHookEx(pEnt, iOffset, pHook, SDKHOOK::s_umSDKHookTrampolines[T][pVtable]);
			SDKHOOK::s_umVFuncHookMarks[pVtable].insert(iOffset);
		}
	}

	template<SDKHookType T>
	bool HookEntity(CBaseEntity* pEnt, typename SDKHookBindings<T>::Pre pCallback) {
		InstallHook<T>(pEnt, (void*)pCallback, false);
		return true;
	}

	template<SDKHookType T>
	bool HookEntity(CBaseEntity* pEnt, typename SDKHookBindings<T>::Post pCallback) {
		InstallHook<T>(pEnt, (void*)pCallback, true);
		return true;
	}
} // namespace SDKHOOK
