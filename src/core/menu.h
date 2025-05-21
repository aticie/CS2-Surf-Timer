#pragma once

#include <pch.h>
#include <core/playermanager.h>
#include <core/screentext.h>
#include <movement/movement.h>
#include <deque>

constexpr auto MENU_SND_SELECT = "UIPanorama.submenu_select";
constexpr auto MENU_SND_EXIT = "UIPanorama.submenu_slidein";

enum class EMenuType {
	Unknown = 0,
	ScreenText
};

enum class EMenuAction {
	SelectItem = 0,
	SwitchPage,
	Cancel,
	Exit
};

class CMenuHandle : public CWeakHandle<class CBaseMenu> {
public:
	using CWeakHandle::CWeakHandle;

	// Close all menu by default
	// Mostly we will close all menu
	// And doesn't care about previous one
	// Otherwise you should use CloseCurrent()
	virtual bool Close() override {
		return this->CloseAll();
	}

	virtual bool CloseAll();

	// Close current menu and display previous
	virtual bool CloseCurrent();
};

#define MENU_CALLBACK_ARGS   CMenuHandle &hMenu, EMenuAction action, CBasePlayerController *pController, uint iItem
#define MENU_CALLBACK(fn)    static void fn(MENU_CALLBACK_ARGS)
#define MENU_CALLBACK_L(...) [__VA_ARGS__](MENU_CALLBACK_ARGS) -> void
using MenuHandler = std::function<void(MENU_CALLBACK_ARGS)>;

class CBaseMenu {
public:
	using MenuItemType = std::pair<std::string, std::any>;
	static inline const MenuItemType NULL_ITEM = {};
	static const size_t PAGE_SIZE = 6;

public:
	CBaseMenu(CBasePlayerController* pController, MenuHandler pFnHandler, std::string sTitle = "") : m_hController(pController->GetRefEHandle()), m_pFnMenuHandler(pFnHandler), m_sTitle(sTitle), m_bExitBack(false) {
		AllocatePage();
	}

	virtual ~CBaseMenu() {}

	virtual EMenuType GetType() {
		return EMenuType::Unknown;
	}

	virtual void Display(int iPageIndex = 0) = 0;
	virtual void Enable() = 0;
	virtual void Disable() = 0;
	virtual bool Close() = 0;

public:
	void SetTitle(const std::string_view& sTitle) {
		m_sTitle = sTitle;
	}

	void SetExitback(bool bExitback) {
		m_bExitBack = bExitback;
	}

	void AddItem(std::string sItem, std::optional<std::any> data = std::nullopt);

	size_t GetPageLength() const {
		return m_vPage.size();
	}

	size_t GetItemLength() const {
		if (m_vPage.size() == 0) {
			return 0;
		}

		return ((m_vPage.size() - 1) * PAGE_SIZE) + m_vPage.back().size();
	}

	const MenuItemType& GetItem(int iPageIndex, int iItemIndex) const;
	const MenuItemType& GetItem(int iItemIndex) const;

private:
	void AllocatePage() {
		std::vector<MenuItemType> nullItems;
		nullItems.reserve(PAGE_SIZE);
		m_vPage.emplace_back(nullItems);
	}

public:
	std::string m_sTitle;
	std::vector<std::vector<MenuItemType>> m_vPage;
	MenuHandler m_pFnMenuHandler;
	CHandle<CBasePlayerController> m_hController;
	bool m_bExitBack;
};

class CScreenTextMenu : public CBaseMenu {
public:
	CScreenTextMenu(CBasePlayerController* pController, MenuHandler pFnHandler, std::string sTitle = "");

	virtual EMenuType GetType() override {
		return EMenuType::ScreenText;
	}

public:
	virtual ~CScreenTextMenu() override;
	virtual void Display(int iPageIndex = 0) override;
	virtual void Enable() override;
	virtual void Disable() override;
	virtual bool Close() override;

public:
	std::weak_ptr<CScreenText> m_wpScreenText;
};

class CMenuPlayer : public CPlayer {
private:
	virtual void Reset() override {
		CPlayer::Reset();

		Cleanup();
		ResetMenu(true);
	}

public:
	using CPlayer::CPlayer;

	void Cleanup();
	void ResetMenu(bool bResetMode = false);
	void SelectMenu();
	void SwitchMode(bool bRedraw = false);
	void DisplayPagePrev();
	void DisplayPageNext();
	void Refresh();
	void Exit();
	void CloseCurrentMenu();
	void CloseAllMenu();

	std::shared_ptr<CBaseMenu>& GetPreviousMenu();
	std::shared_ptr<CBaseMenu>& GetCurrentMenu();
	bool IsDisplaying();

private:
	friend class CScreenTextMenu;
	void ClampItem();

public:
	std::deque<std::shared_ptr<CBaseMenu>> m_MenuQueue;
	uint m_nCurrentPage;
	uint m_nCurrentItem;
	bool m_bWSADMenu;
};

class CMenuManager : CPlayerManager, CMovementForward {
public:
	CMenuManager() {
		for (int i = 0; i < MAXPLAYERS; i++) {
			m_pPlayers[i] = std::make_unique<CMenuPlayer>(i);
		}
	}

	virtual CMenuPlayer* ToPlayer(CBasePlayerController* controller) const override {
		return static_cast<CMenuPlayer*>(CPlayerManager::ToPlayer(controller));
	}

	virtual CMenuPlayer* ToPlayer(CBasePlayerPawn* pawn) const override {
		return static_cast<CMenuPlayer*>(CPlayerManager::ToPlayer(pawn));
	}

private:
	virtual void OnPluginStart() override;
	virtual void OnPlayerRunCmdPost(CCSPlayerPawnBase* pPawn, const CInButtonState& buttons, const float (&vec)[3], const QAngle& viewAngles, const int& weapon, const int& cmdnum, const int& tickcount, const int& seed, const int (&mouse)[2]) override;
};

namespace MENU {
	extern CMenuManager* GetManager();

	[[nodiscard]] std::weak_ptr<CBaseMenu> Create(CBasePlayerController* pController, MenuHandler pFnMenuHandler, EMenuType eMenuType = EMenuType::ScreenText);
	bool CloseCurrent(CBasePlayerController* pController);
	bool CloseAll(CBasePlayerController* pController);
} // namespace MENU
