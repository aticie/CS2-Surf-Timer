#include "surf_replay.h"
#include <utils/utils.h>

void CSurfBotReplayService::OnInit() {
	Init();
}

void CSurfBotReplayService::OnReset() {
	Init();
}

void CSurfBotReplayService::Init() {
	m_iCurrentTick = 0;
	m_iCurrentTrack = -1;
	m_iCurrentStage = -1;
}

void CSurfBotReplayService::DoPlayback(CCSPlayerPawn* pBotPawn, CInButtonState& buttons) {
	auto& aFrames = const_cast<ReplayArray_t&>(NULL_REPLAY_ARRAY);
	if (IsTrackBot()) {
		aFrames = SURF::ReplayPlugin()->m_aTrackReplays.at(m_iCurrentTrack);
	} else if (IsStageBot()) {
		aFrames = SURF::ReplayPlugin()->m_aStageReplays.at(m_iCurrentStage);
	}

	auto iFrameSize = aFrames.size();
	if (!iFrameSize) {
		return;
	}

	if (m_iCurrentTick >= iFrameSize) {
		m_iCurrentTick = 0;
	}

	auto& frame = aFrames.at(m_iCurrentTick);
	MEM::CALL::SnapViewAngles(pBotPawn, frame.ang);

	auto botFlags = pBotPawn->m_fFlags();
	pBotPawn->m_fFlags(botFlags ^ frame.flags);

	buttons = frame.buttons;
	pBotPawn->SetMoveType(frame.mt);

	const Vector& vecCurrentPos = pBotPawn->GetAbsOrigin();
	if (m_iCurrentTick == 0 || vecCurrentPos.DistTo(frame.pos) > 15000.0) {
		pBotPawn->SetMoveType(MoveType_t::MOVETYPE_NOCLIP);
		pBotPawn->Teleport(&frame.pos, nullptr, &SURF::ZERO_VEC);
	} else {
		Vector vecCalculatedVelocity = (frame.pos - vecCurrentPos) * SURF_TICKRATE;
		pBotPawn->Teleport(nullptr, nullptr, &vecCalculatedVelocity);
	}

	m_iCurrentTick++;
}
