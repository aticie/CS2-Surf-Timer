#pragma once

#include <pch.h>
#include <surf/api.h>
#include <surf/surf_bot.h>
#include <utils/ctimer.h>

using ReplayArray_t = std::vector<replay_frame_data_t>;

struct replay_run_info_t {
	std::time_t timestamp;
	uint64 steamid;
	f64 time;
	TimerStyle_t style;
	TimerTrack_t track;
	TimerStage_t stage;
	size_t framelength;
	// ReplayArray_t frames; // Not POD, write it on somewhere else
};

static_assert(std::is_trivial_v<replay_run_info_t> && std::is_standard_layout_v<replay_run_info_t>);

struct replay_file_header_t {
	std::string format = SURF_REPLAY_FORMAT;
	u8 version = SURF_REPLAY_VERSION;
	std::string map;
	f32 tickrate;
	replay_run_info_t info;

	void ReadFromStream(std::ifstream& in);
	void WriteToStream(std::ofstream& out) const;
};

struct replay_extraframe_t {
	size_t iPreStart {};
	size_t iPreEnd {};
	bool bGrabEnd {};
	size_t iEndStart {};
};

class CSurfReplayService : CSurfPlayerService {
private:
	virtual void OnInit() override;
	virtual void OnReset() override;

public:
	using CSurfPlayerService::CSurfPlayerService;

	void Init();
	void OnEnterStart_Recording();
	void OnStart_Recording();
	void OnTimerFinishPost_SaveRecording();
	void FinishGrabbingPostFrames(bool bStage = false);

	void DoRecord(CCSPlayerPawn* pawn, const CPlayerButton& buttons, const QAngle& viewAngles);
	void SaveRecord(bool bStageReplay, ReplayArray_t* out = nullptr);
	void ClearFrames();

public:
	bool m_bEnabled {};
	ReplayArray_t m_vCurrentFrames;
	size_t m_iCurrentFrame {};
	replay_extraframe_t m_ExtraTrackFrame;
	replay_extraframe_t m_ExtraStageFrame;

	CTimerHandle m_hTrackPostFrameTimer;
	CTimerHandle m_hStagePostFrameTimer;
};

class CSurfBotReplayService : CSurfBotService {
public:
	using CSurfBotService::CSurfBotService;

	virtual void OnInit() override;
	virtual void OnReset() override;

public:
	void Init();
	void DoPlayback(CCSPlayerPawn* pPawn, CInButtonState& buttons);

	bool IsReplayBot() const {
		return m_bReplayBot;
	}

	bool IsStageBot() const {
		return m_iCurrentStage != -1;
	}

	bool IsTrackBot() const {
		return m_iCurrentTrack != -1;
	}

public:
	static inline const ReplayArray_t NULL_REPLAY_ARRAY = {};

	bool m_bReplayBot {};
	TimerStage_t m_iCurrentStage = -1;
	TimerTrack_t m_iCurrentTrack = -1;
	size_t m_iCurrentTick {};
};

class CSurfReplayPlugin : CSurfForward, CMovementForward, CCoreForward {
private:
	virtual void OnPluginStart() override;
	virtual void OnEntitySpawned(CEntityInstance* pEntity) override;

	virtual bool OnPlayerRunCmd(CCSPlayerPawn* pPawn, CInButtonState& buttons, float (&vec)[3], QAngle& viewAngles, int& weapon, int& cmdnum, int& tickcount, int& seed, int (&mouse)[2]) override;
	virtual void OnPlayerRunCmdPost(CCSPlayerPawn* pPawn, const CInButtonState& buttons, const float (&vec)[3], const QAngle& viewAngles, const int& weapon, const int& cmdnum, const int& tickcount, const int& seed, const int (&mouse)[2]) override;

	virtual bool OnEnterZone(const ZoneCache_t& zone, CSurfPlayer* pPlayer) override;
	virtual bool OnStayZone(const ZoneCache_t& zone, CSurfPlayer* pPlayer) override;
	virtual bool OnLeaveZone(const ZoneCache_t& zone, CSurfPlayer* pPlayer) override;
	virtual void OnTimerFinishPost(CSurfPlayer* pPlayer) override;

public:
	std::string BuildReplayPath(const i8 style, const TimerTrack_t track, const i8 stage, const std::string_view map);
	void AsyncWriteReplayFile(const replay_run_info_t& info, const ReplayArray_t& vFrames);
	bool ReadReplayFile(const std::string_view path, ReplayArray_t& out);

private:
	void HookEvents();

public:
	ConVarRefAbstract* m_cvarTrackPreRunTime;
	ConVarRefAbstract* m_cvarTrackPostRunTime;
	ConVarRefAbstract* m_cvarStagePreRunTime;
	ConVarRefAbstract* m_cvarStagePostRunTime;
	ConVarRefAbstract* m_cvarPreRunAlways;

	std::array<ReplayArray_t, SURF_MAX_TRACK> m_aTrackReplays;
	std::array<ReplayArray_t, SURF_MAX_STAGE> m_aStageReplays;
};

namespace SURF {
	extern CSurfReplayPlugin* ReplayPlugin();

	namespace REPLAY {
		namespace HOOK {
			bool OnBotTrigger(CBaseEntity* pSelf, CBaseEntity* pOther);
		} // namespace HOOK
	} // namespace REPLAY
} // namespace SURF
