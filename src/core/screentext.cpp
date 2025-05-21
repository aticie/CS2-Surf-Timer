#include <core/screentext.h>
#include <core/memory.h>
#include <core/eventmanager.h>
#include <core/concmdmanager.h>
#include <utils/ctimer.h>

CScreenTextControllerManager g_ScreenTextControllerManager;

extern void SetScreenTextEntityTransmiter(CBaseEntity* pScreenEnt, CBasePlayerController* pOwner);

CScreenText::CScreenText(const ScreenTextManifest_t& manifest) : m_vecPos(manifest.m_vecPos) {
	CPointWorldText* pText = (CPointWorldText*)MEM::CALL::CreateEntityByName("point_worldtext");
	if (!pText) {
		SDK_ASSERT(false);
		return;
	}

	pText->m_Color(manifest.m_Color);
	pText->m_FontName(manifest.m_sFont.c_str());
	pText->m_flFontSize(manifest.m_fFontSize);
	pText->m_flWorldUnitsPerPx((0.25 / manifest.m_iUnits) * manifest.m_fFontSize);
	pText->m_flDepthOffset(0.0f);
	pText->m_fadeMinDist(0.0f);
	pText->m_fadeMinDist(0.0f);
	pText->m_nJustifyHorizontal(PointWorldTextJustifyHorizontal_t::POINT_WORLD_TEXT_JUSTIFY_HORIZONTAL_LEFT);
	pText->m_nJustifyVertical(PointWorldTextJustifyVertical_t::POINT_WORLD_TEXT_JUSTIFY_VERTICAL_CENTER);
	pText->m_bFullbright(true);
	pText->m_bDrawBackground(manifest.m_bBackground);
	pText->m_flBackgroundBorderHeight(manifest.m_fBackgroundBorderHeight);
	pText->m_flBackgroundBorderWidth(manifest.m_fBackgroundBorderWidth);

	pText->m_messageText("Sample Text");
	pText->m_bEnabled(manifest.m_bEnable);

	pText->DispatchSpawn();

	this->m_hScreenEnt.Set(pText);
}

CScreenText::~CScreenText() {
	if (m_hScreenEnt.IsValid()) {
		m_hScreenEnt->Kill();
		m_hScreenEnt.Term();
	}

	m_hOwner.Term();
	m_hOriginalController.Term();
}

void CScreenText::SetText(const std::string_view& text) {
	m_hScreenEnt->SetText(text.data());
}

void CScreenText::SetPos(float x, float y) {
	m_vecPos.x = x;
	m_vecPos.y = y;

	UpdatePos();
}

void CScreenText::SetColor(Color color) {
	m_hScreenEnt->m_Color(color);
}

void CScreenText::SetFont(const std::string_view& font) {
	m_hScreenEnt->m_FontName(font.data());
}

void CScreenText::SetFontSize(float fontsize) {
	m_hScreenEnt->m_flFontSize(fontsize);
}

void CScreenText::SetUnits(int unit) {
	m_hScreenEnt->m_flWorldUnitsPerPx((0.25 / unit) * m_hScreenEnt->m_flFontSize());
}

bool CScreenText::IsRendering() {
	return m_hOwner.IsValid() && m_hScreenEnt->m_bEnabled();
}

void CScreenText::Display(CBasePlayerController* pController) {
	// Already displayed
	if (IsRendering()) {
		return;
	}

	CPointWorldText* pText = this->m_hScreenEnt.Get();
	if (!pText) {
		SDK_ASSERT(false);
		return;
	}

	CCSPlayerPawnBase* pPawn = dynamic_cast<CCSPlayerPawnBase*>(pController->GetCurrentPawn());
	if (!pPawn) {
		SDK_ASSERT(false);
		return;
	}

	pText->Enable();

	UpdateTransmit(pController);
	UpdateRelation(pPawn);
	UpdatePos();
}

void CScreenText::UpdateRelation(CCSPlayerPawnBase* pPawn) {
	if (!pPawn) {
		return;
	}

	CPointWorldText* pText = this->m_hScreenEnt.Get();
	if (!pText) {
		SDK_ASSERT(false);
		return;
	}

	if (!pPawn->IsObserver()) {
		CBaseViewModel* pViewModel = pPawn->EnsureViewModel();
		if (!pViewModel) {
			SDK_ASSERT(false);
			return;
		}

		pText->SetParent(pViewModel);
		pText->m_hOwnerEntity().Set(pViewModel);
	} else {
		CBaseEntity* pTarget = pPawn;

		CPlayer_ObserverServices* pObsService = pPawn->m_pObserverServices();
		CCSPlayerPawnBase* pObsTarget = dynamic_cast<CCSPlayerPawnBase*>(pObsService->m_hObserverTarget()->Get());
		if (pObsTarget && pObsService->m_iObserverMode() == OBS_MODE_IN_EYE) {
			pTarget = pObsTarget->EnsureViewModel();
		}

		pText->SetParent(pTarget);
		pText->m_hOwnerEntity().Set(pTarget);
	}

	m_hOwner.Set(pPawn);
}

void CScreenText::UpdatePos() {
	auto pText = m_hScreenEnt.Get();

	if (!pText || !pText->m_hOwnerEntity().IsValid()) {
		return;
	}

	CBaseEntity* pParent = pText->m_hOwnerEntity().Get();
	const Vector& parentPos = pParent->GetAbsOrigin();
	Vector textPos;
	if (dynamic_cast<CBaseViewModel*>(pParent)) {
		textPos = GetRelativeVMOrigin(parentPos);
	} else {
		pParent->Teleport(nullptr, &vec3_angle, nullptr);
		textPos = GetRelativePawnOrigin(parentPos, vec3_angle);
	}

	static QAngle textAng = {0.0f, -90.0f, 90.0f};

	Vector fwd, right;
	AngleVectors(textAng, &fwd, &right, nullptr);
	fwd *= m_vecPos.x;
	right *= m_vecPos.y * -1.0f;
	textPos += fwd + right;

	pText->Teleport(&textPos, &textAng, nullptr);
}

void CScreenText::UpdateTransmit(CBasePlayerController* pOwner) {
	CPointWorldText* pText = this->m_hScreenEnt.Get();
	if (!pText) {
		SDK_ASSERT(false);
		return;
	}

	SetScreenTextEntityTransmiter(pText, pOwner);
}

Vector CScreenText::GetRelativeVMOrigin(const Vector& eyePosition, float distanceToTarget) {
	return Vector(eyePosition.x + distanceToTarget, eyePosition.y, eyePosition.z);
}

Vector CScreenText::GetRelativePawnOrigin(const Vector& eyePosition, const QAngle& eyeAngles, float distanceToTarget) {
	double pitch = eyeAngles.x * (M_PI / 180.0);
	double yaw = eyeAngles.y * (M_PI / 180.0);

	double targetX = eyePosition.x + distanceToTarget * std::cos(pitch) * std::cos(yaw);
	double targetY = eyePosition.y + distanceToTarget * std::cos(pitch) * std::sin(yaw);
	double targetZ = eyePosition.z - distanceToTarget * std::sin(pitch);

	return Vector(targetX, targetY, targetZ);
};

CScreenTextControllerManager* VGUI::GetScreenTextManager() {
	return &g_ScreenTextControllerManager;
}

std::weak_ptr<CScreenText> VGUI::CreateScreenText(CBasePlayerController* pController, std::optional<ScreenTextManifest_t> manifest) {
	CScreenTextController* pTextController = GetScreenTextManager()->ToPlayer(pController);
	if (!pTextController) {
		SDK_ASSERT(false);
		return {};
	}

	if (!manifest) {
		manifest = ScreenTextManifest_t {};
	}

	auto pText = std::make_shared<CScreenText>(manifest.value());
	pText->m_hOriginalController = pController->GetRefEHandle();
	pTextController->m_ScreenTextList.emplace_back(pText);
	return pText;
}

void VGUI::Render(const std::weak_ptr<CScreenText>& hText) {
	auto pText = hText.lock();
	if (!pText) {
		return;
	}

	if (pText->IsRendering()) {
		return;
	}

	if (!pText->m_hOriginalController.IsValid()) {
		return;
	}

	auto pController = pText->m_hOriginalController.Get();
	if (!pController) {
		return;
	}

	pText->Display(pController);
}

void VGUI::Unrender(const std::weak_ptr<CScreenText>& hText) {
	auto pText = hText.lock();
	if (!pText) {
		return;
	}

	if (!pText->IsRendering()) {
		return;
	}

	pText->Disable();
}

void VGUI::Dispose(const std::weak_ptr<CScreenText>& hText) {
	auto pText = hText.lock();
	if (!pText) {
		return;
	}

	if (!pText->m_hOriginalController.IsValid()) {
		return;
	}

	auto pController = pText->m_hOriginalController.Get();
	if (!pController) {
		return;
	}

	CScreenTextController* pTextController = GetScreenTextManager()->ToPlayer(pController);
	if (!pTextController) {
		SDK_ASSERT(false);
		return;
	}

	std::erase(pTextController->m_ScreenTextList, pText);
}

void VGUI::Cleanup(CBasePlayerController* pController) {
	CScreenTextController* pTextController = GetScreenTextManager()->ToPlayer(pController);
	if (!pTextController) {
		SDK_ASSERT(false);
		return;
	}

	pTextController->m_ScreenTextList.clear();
}

EVENT_CALLBACK_POST(OnPlayerTeam) {
	auto pController = (CCSPlayerController*)pEvent->GetPlayerController("userid");
	if (!pController) {
		return;
	}

	auto iNewTeam = pEvent->GetInt("team");
	CHandle<CCSPlayerController> hController = pController->GetRefEHandle();

	UTIL::RequestFrame([hController, iNewTeam]() {
		CCSPlayerController* pController = hController.Get();
		if (!pController) {
			return;
		}

		CBasePlayerPawn* pPawn = pController->GetCurrentPawn();
		if (!pPawn) {
			return;
		}

		auto pPlayer = VGUI::GetScreenTextManager()->ToPlayer(pController);
		if (!pPlayer) {
			return;
		}

		for (const auto& pScreenText : pPlayer->m_ScreenTextList) {
			pScreenText->UpdateRelation(dynamic_cast<CCSPlayerPawnBase*>(pPawn));
			pScreenText->UpdatePos();
		}
	});
}

void CScreenTextControllerManager::OnPluginStart() {
	EVENT::HookEvent("player_team", ::OnPlayerTeam);
}

void CScreenTextControllerManager::OnPlayerRunCmdPost(CCSPlayerPawnBase* pPawn, const CInButtonState& buttons, const float (&vec)[3], const QAngle& viewAngles, const int& weapon, const int& cmdnum, const int& tickcount, const int& seed, const int (&mouse)[2]) {
	if (pPawn->IsObserver()) {
		auto pNode = pPawn->m_CBodyComponent()->m_pSceneNode();
		auto pParent = pNode->m_pParent();
		CGameSceneNode* pChild = pNode->m_pChild();
		if (pChild) {
			//  Magically force child update physics every tick!
			auto& velZ = pPawn->m_vecVelocity().m_vecZ();
			velZ += 0.6f;
			pNode->m_angAbsRotation(viewAngles);
			if (pNode->m_angRotation() != viewAngles) {
				pNode->m_angRotation(viewAngles);
			}
		}
	}
}

void CScreenTextControllerManager::OnSetObserverTargetPost(CPlayer_ObserverServices* pService, CBaseEntity* pEnt) {
	auto iObsMode = pService->m_iObserverMode();
	if (iObsMode == OBS_MODE_NONE) {
		return;
	}

	if (!pEnt && iObsMode == OBS_MODE_IN_EYE) {
		return;
	}

	if (iObsMode == OBS_MODE_IN_EYE || iObsMode == OBS_MODE_CHASE) {
		auto pServicePawn = pService->GetPawn();
		auto pPlayer = VGUI::GetScreenTextManager()->ToPlayer(pServicePawn);
		if (!pPlayer) {
			return;
		}

		for (const auto& pScreenText : pPlayer->m_ScreenTextList) {
			pScreenText->UpdateRelation(pServicePawn);
			pScreenText->UpdatePos();
		}
	}
}
