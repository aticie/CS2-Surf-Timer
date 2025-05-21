#pragma once

#include <pch.h>

#include <sdk/common.h>
#include <sdk/datatypes.h>
#include <sdk/server/weapon.h>
#include <libmem/libmem_helper.h>
#include <libmodule/module.h>
#include <polyhook2/Detour/NatDetour.hpp>
#include <utils/vtablehelper.h>

#include <list>

class CBasePlayerController;
class CCSPlayerController;
class CCSPlayerPawn;
class CEntityInstance;
class CEntityKeyValues;
class CBaseTrigger;

class GameSessionConfiguration_t {};

namespace MEM {
	namespace CALL {
		void SwitchTeam(CCSPlayerController* controller, int team);
		void SetPawn(CBasePlayerController* controller, CCSPlayerPawn* pawn, bool a3, bool a4, bool a5);
		IGameEventListener2* GetLegacyGameEventListener(CPlayerSlot slot);
		bool TraceShape(const Ray_t& ray, const Vector& vecStart, const Vector& vecEnd, const CTraceFilter& filter, CGameTrace* tr);
		void TracePlayerBBox(const Vector& start, const Vector& end, const bbox_t& bounds, CTraceFilter* filter, trace_t& pm);
		void InitPlayerMovementTraceFilter(CTraceFilterPlayerMovementCS& pFilter, CEntityInstance* pHandleEntity, uint64 interactWith, int collisionGroup);
		void SnapViewAngles(CBasePlayerPawn* pawn, const QAngle& angle);
		void CEntityInstance_AcceptInput(CEntityInstance* pEnt, const char* pInputName, CEntityInstance* pActivator, CEntityInstance* pCaller, variant_t* value, int nOutputID);
		CBaseEntity* CreateEntityByName(const char* pszName);
		void DispatchSpawn(CBaseEntity* pEnt, CEntityKeyValues* pInitKeyValue);
		CBaseTrigger* CreateAABBTrigger(const Vector& center, const Vector& mins, const Vector& maxs);
		void SetParent(CBaseEntity* pEnt, CBaseEntity* pParent);
		void SetEntityName(CEntityIdentity* pEnt, const char* pszName);
		SndOpEventGuid_t EmitSound(IRecipientFilter& filter, CEntityIndex ent, const EmitSound_t& params);
		bool BotAddCommand(int team, bool isFromConsole = false, const char* profileName = nullptr, CSWeaponType weaponType = WEAPONTYPE_UNKNOWN, int difficulty = 0);
	} // namespace CALL

	namespace MODULE {
		inline std::shared_ptr<libmodule::CModule> engine;
		inline std::shared_ptr<libmodule::CModule> tier0;
		inline std::shared_ptr<libmodule::CModule> server;
		inline std::shared_ptr<libmodule::CModule> schemasystem;
		inline std::shared_ptr<libmodule::CModule> steamnetworkingsockets;

		void Setup();
	} // namespace MODULE

	namespace TRAMPOLINE {
		inline void* g_fnApplyGameSettings;
		inline void* g_fnGameFrame;
		inline void* g_fnClientConnect;
		inline void* g_fnClientConnected;
		inline void* g_fnClientFullyConnect;
		inline void* g_fnClientPutInServer;
		inline void* g_fnClientActive;
		inline void* g_fnClientDisconnect;
		inline void* g_fnClientVoice;
		inline void* g_fnClientCommand;
		inline void* g_fnCreateGameEvent;
		inline void* g_fnFireGameEvent;
		inline void* g_fnStartupServer;
		inline void* g_fnDispatchConCommand;
		inline void* g_fnPostEventAbstract;
		inline void* g_fnServerGamePostSimulate;
		inline void* g_fnActivateServer;
		inline void* g_fnWeaponDrop;
		inline void* g_fnTakeDamage;
		inline void* g_fnClientSendSnapshotBefore;
		inline void* g_fnSetObserverTarget;
	} // namespace TRAMPOLINE

	void SetupHooks();

	class CHookManager {
	public:
		std::list<std::pair<std::unique_ptr<PLH::NatDetour>, std::uintptr_t>> m_DetourList;
	};

	extern CHookManager* GetHookManager();

	template<typename TOriginal, typename TCallback, typename TTram>
	bool AddDetour(TOriginal pOriginal, TCallback pCallback, TTram& pTrampoline) {
		auto pDetour = std::make_unique<PLH::NatDetour>((uint64_t)pOriginal, (uint64_t)pCallback, (uint64_t*)&pTrampoline);
		if (!pDetour->hook()) {
			SDK_ASSERT(false);
			return false;
		}

		GetHookManager()->m_DetourList.emplace_back(std::pair {std::move(pDetour), (uintptr_t)pOriginal});
		return true;
	}

	template<typename TOriginal, typename TTram>
	bool RemoveDetour(TOriginal pOriginal, TTram& pTrampoline) {
		auto pTarget = reinterpret_cast<uintptr_t>(pOriginal);
		auto& pDetourList = GetHookManager()->m_DetourList;
		auto it = std::ranges::find_if(pDetourList, [pTarget](const auto& pair) { return pair.second == pTarget; });

		if (it != pDetourList.end()) {
			it->first->unHook();
			pDetourList.erase(it);
			return true;
		}

		return false;
	}

	template<typename T = void, typename... Args>
	typename std::enable_if<!std::is_void<T>::value, T>::type SDKCall(void* pAddress, Args... args) {
		auto pFn = reinterpret_cast<T (*)(Args...)>(pAddress);
		SDK_ASSERT((uintptr_t)pFn);
		return pFn(args...);
	}

	template<typename T = void, typename... Args>
	typename std::enable_if<std::is_void<T>::value, void>::type SDKCall(void* pAddress, Args... args) {
		auto pFn = reinterpret_cast<void (*)(Args...)>(pAddress);
		SDK_ASSERT((uintptr_t)pFn);
		pFn(args...);
	}

	template<typename T, typename... Args>
	auto GetFn(void* pAddress, Args... args) {
		return reinterpret_cast<T (*)(Args...)>(pAddress);
	}

	template<typename T>
	inline bool VmtHookEx(uint32_t uIndex, libmodule::CModule* pModule, const char* className, T pFunc, void*& pOriginFunc, bool allInstance = false) {
		if (!pModule) {
			return false;
		}

		void* vtable = pModule->GetVirtualTableByName(className);
		if (!vtable) {
			return false;
		}

		libmem::Vmt vmt((libmem::Address*)vtable);
		auto from = vmt.GetOriginal(uIndex);

		if (allInstance) {
			MEM::AddDetour(from, pFunc, pOriginFunc);
		} else {
			vmt.Hook(uIndex, (libmem::Address)pFunc);
			pOriginFunc = (void*)from;
		}

		return true;
	}

	template<size_t nOffset = 0>
	inline void PatchNOP(void* pAddress, size_t nLen) {
		uint8_t* adr_patch = static_cast<uint8_t*>(pAddress) + nOffset;
		auto old_mem_prot = libmem::ProtMemory((libmem::Address)adr_patch, nLen, libmem::Prot::XRW);
		if (old_mem_prot.has_value()) {
			libmem::SetMemory((libmem::Address)adr_patch, 0x90, nLen);
			libmem::ProtMemory((libmem::Address)adr_patch, nLen, old_mem_prot.value());
		}
	}
} // namespace MEM

// clang-format off

#define HOOK_SIG(sig, fnHook, fnTrampoline) \
	static auto fn##fnHook = GAMEDATA::GetMemSig(sig); \
	SDK_ASSERT(fn##fnHook); \
	if (fn##fnHook) { \
		MEM::AddDetour(fn##fnHook, fnHook, fnTrampoline); \
	}

#define HOOK_VMT(instance, vfn, fnHook, fnTrampoline) \
	SDK_ASSERT(libmem::VmtHookEx(instance, offsetof_vtablefn(vfn), fnHook, fnTrampoline));

#define HOOK_VMT_OVERRIDE(instance, classname, vfn, fnHook, fnTrampoline, ...) \
	SDK_ASSERT(libmem::VmtHookEx( \
		instance, \
		TOOLS::GetVtableIndex(static_cast<FunctionTraits<decltype(&fnHook)>::ReturnType (classname::*)(__VA_ARGS__)>(&classname::vfn)), \
		fnHook, \
		fnTrampoline));

#define HOOK_VMTEX(sClassname, vfn, pModule, fnHook, fnTrampoline) \
	SDK_ASSERT(MEM::VmtHookEx(offsetof_vtablefn(vfn), pModule.get(), sClassname, fnHook, fnTrampoline));

#define GAMEDATA_VMT(gdOffsetKey, pModule, fnHook, fnTrampoline) \
	SDK_ASSERT(MEM::VmtHookEx(GAMEDATA::GetOffset(gdOffsetKey), pModule.get(), gdOffsetKey, fnHook, fnTrampoline));

#define DETOUR_VMT(gdOffsetKey, pModule, fnHook, fnTrampoline) \
	SDK_ASSERT(MEM::VmtHookEx(GAMEDATA::GetOffset(gdOffsetKey), pModule.get(), gdOffsetKey, fnHook, fnTrampoline, true));

// clang-format on
