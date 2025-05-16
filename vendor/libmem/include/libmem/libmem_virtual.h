#pragma once

#include <libmem/libmem.hpp>

#define GET_VIRTUAL(pInstance, idx)        vmt::GetVMethod(idx, pInstance)
#define SET_VIRTUAL(pInstance, idx, pFunc) vmt::SetVMethod(idx, pInstance, pFunc)
#define CALL_VIRTUAL(retType, idx, ...)    vmt::CallVirtual<retType>(idx, __VA_ARGS__)

#ifdef _WIN32
#define WIN_LINUX(win, linux) win
#define THISCALL __thiscall
#else
#define WIN_LINUX(win, linux) linux
#define THISCALL
#endif

namespace vmt {
	template<typename T = void*>
	inline T GetVMethod(uint32_t uIndex, void* pInstance) {
		if (!pInstance) {
			printf("vmt::GetVMethod failed: invalid instance pointer\n");
			return T();
		}

		void** vtable = *static_cast<void***>(pInstance);
		if (!vtable) {
			printf("vmt::GetVMethod failed: invalid vtable pointer\n");
			return T();
		}

		return reinterpret_cast<T>(vtable[uIndex]);
	}

	// 模拟真实的虚函数调用流程
	// 自动处理返回值为非平凡类型的情况
	// 返回值类型、内存结构必须与虚函数的一致
	template<typename Ret = void, typename T, typename... Args>
	inline Ret CallVirtual(uint32_t uIndex, T pClass, Args... args) {
		auto func_ptr = GetVMethod(uIndex, pClass);
		if (!func_ptr) {
			printf("vmt::CallVirtual failed: invalid function pointer\n");
			return Ret();
		}

#ifdef _WIN32
		class VType {};
		union VConverter {
			VConverter(void* _ptr) : ptr(_ptr) {}

			void* ptr;
			Ret (__thiscall VType::*fn)(Args...);
		} v(func_ptr);

		return ((VType*)pClass->*v.fn)(args...);
#else
		return reinterpret_cast<Ret (*)(T, Args...)>(func_ptr)(pClass, args...);
#endif
	}

	template<typename T>
	inline bool SetVMethod(uint32_t uIndex, void* pInstance, T pFunc) {
		if (!pInstance) {
			printf("vmt::SetVMethod failed: invalid instance pointer\n");
			return false;
		}

		void** vtable = *static_cast<void***>(pInstance);
		if (!vtable) {
			printf("vmt::SetVMethod failed: invalid vtable pointer\n");
			return false;
		}

		libmem::Vmt vmt((libmem::Address*)vtable);
		vmt.Hook(uIndex, (libmem::Address)pFunc);

		return true;
	}

	template<typename T>
	inline bool SetVMethodEx(uint32_t uIndex, void* pInstance, T pFunc, void*& pOriginFunc) {
		if (!pInstance) {
			printf("vmt::SetVMethodEx failed: invalid instance pointer\n");
			return false;
		}

		void** vtable = *static_cast<void***>(pInstance);
		if (!vtable) {
			printf("vmt::SetVMethodEx failed: invalid vtable pointer\n");
			return false;
		}

		libmem::Vmt vmt((libmem::Address*)vtable);
		vmt.Hook(uIndex, (libmem::Address)pFunc);

		pOriginFunc = (void*)vmt.GetOriginal(uIndex);

		return true;
	}

	template<typename T>
	inline void Override(T& pOrigin, T pFunc) {
		if (!pFunc) {
			return;
		}

		pOrigin = pFunc;
	}

} // namespace vmt
