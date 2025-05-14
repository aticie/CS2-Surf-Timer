#include "surf_replay.h"
#include <thread>
#include <utils/utils.h>
#include <fmt/format.h>

constexpr auto REPLAY_ARRAY_HEAD = "===ARRAY_HEAD===";
constexpr auto REPLAY_ARRAY_TAIL = "===ARRAY_TAIL===";

struct replay_compress_button_t {
	u8 attack: 1;
	u8 jump: 1;
	u8 duck: 1;
	u8 use: 1;
	u8 attack2: 1;
	u8 reload: 1;
	u8 speed: 1;
	u8 inspect: 1;

	replay_compress_button_t() = default;

	replay_compress_button_t(u64 button) {
		Compress(button);
	}

	// clang-format off

	void Compress(u64 button) {
		if (button & IN_ATTACK) attack = true;
		if (button & IN_JUMP) jump = true;
		if (button & IN_DUCK) duck = true;
		if (button & IN_USE) use = true;
		if (button & IN_ATTACK2) attack2 = true;
		if (button & IN_RELOAD) reload = true;
		if (button & IN_SPEED) speed = true;
		if (button & IN_LOOK_AT_WEAPON) inspect = true;
	}

	u64 Decompress() const {
		u64 button = {};
		if (attack) button |= IN_ATTACK;
		if (jump) button |= IN_JUMP;
		if (duck) button |= IN_DUCK;
		if (use) button |= IN_USE;
		if (attack2) button |= IN_ATTACK2;
		if (reload) button |= IN_RELOAD;
		if (speed) button |= IN_SPEED;
		if (inspect) button |= IN_LOOK_AT_WEAPON;

		return button;
	}

	// clang-format on
};

static void ReadStreamString(std::ifstream& file, std::string& outStr) {
	uint32_t len = 0;
	file.read(reinterpret_cast<char*>(&len), sizeof(len));
	outStr.resize(len);
	file.read(&outStr[0], len);
}

static void WriteStreamString(std::ofstream& file, const std::string& str) {
	uint32_t length = static_cast<uint32_t>(str.size());
	file.write(reinterpret_cast<const char*>(&length), sizeof(length));
	file.write(str.data(), length);
}

void replay_file_header_t::ReadFromStream(std::ifstream& in) {
	ReadStreamString(in, format);
	in.read(reinterpret_cast<char*>(&version), sizeof(version));
	ReadStreamString(in, map);
	in.read(reinterpret_cast<char*>(&tickrate), sizeof(tickrate));
	in.read(reinterpret_cast<char*>(&info), sizeof(info));
}

void replay_file_header_t::WriteToStream(std::ofstream& out) const {
	WriteStreamString(out, format);
	out.write(reinterpret_cast<const char*>(&version), sizeof(version));
	WriteStreamString(out, map);
	out.write(reinterpret_cast<const char*>(&tickrate), sizeof(tickrate));
	out.write(reinterpret_cast<const char*>(&info), sizeof(info));
}

std::string CSurfReplayPlugin::BuildReplayPath(const i8 style, const TimerTrack_t track, const i8 stage, const std::string_view map) {
	std::string sDirPath = UTIL::PATH::Join(UTIL::GetWorkingDirectory(), "replay", map);
	try {
		if (!std::filesystem::exists(sDirPath)) {
			std::filesystem::create_directories(sDirPath);
		}
	} catch (const std::filesystem::filesystem_error& e) {
		LOG::Error("[BuildReplayPath] Failed to create directory: %s\n", e.what());
		return "";
	}

	auto getTrackFileName = [](const TimerTrack_t track) -> std::string {
		switch (track) {
			case EZoneTrack::Track_Main:
				return "Main";
			default:
				return fmt::format("B{}", (i8)track);
		}
	};

	std::string sMaybeStage = (stage == 0) ? "" : fmt::format("-s{}", stage);
	return fmt::format("{}/{}-{}{}.replay", sDirPath, SURF::GetStyleShortName(style), getTrackFileName(track), sMaybeStage);
}

void CSurfReplayPlugin::AsyncWriteReplayFile(const replay_run_info_t& info, const ReplayArray_t& vFrames) {
	std::string sMap = UTIL::GetGlobals()->mapname.ToCStr();
	std::string sFilePath = BuildReplayPath(info.style, info.track, info.stage, sMap);
	std::thread([info, vFrames, sFilePath, sMap]() {
		std::ofstream file(sFilePath, std::ios::out | std::ios::binary);
		if (!file.good()) {
			LOG::Error("Failed to WriteReplayFile: %s", sFilePath.data());
			SDK_ASSERT(false);
			return;
		}

		replay_file_header_t header;
		header.map = sMap;
		header.tickrate = SURF_TICKRATE;
		header.info = info;
		header.WriteToStream(file);

		file.write(REPLAY_ARRAY_HEAD, std::strlen(REPLAY_ARRAY_HEAD));

		for (size_t i = 0; i < info.framelength; i++) {
			auto& frame = vFrames.at(i);

			file.write(reinterpret_cast<const char*>(&frame.ang), sizeof(Vector2D));
			file.write(reinterpret_cast<const char*>(&frame.pos), sizeof(frame.pos));
			file.write(reinterpret_cast<const char*>(&frame.flags), sizeof(frame.flags));
			file.write(reinterpret_cast<const char*>(&frame.mt), sizeof(frame.mt));

			replay_compress_button_t compressed_button(frame.buttons);
			file.write(reinterpret_cast<const char*>(&compressed_button), sizeof(compressed_button));
		}

		file.write(REPLAY_ARRAY_TAIL, std::strlen(REPLAY_ARRAY_TAIL));

		file.close();
	}).detach();
}

bool CSurfReplayPlugin::ReadReplayFile(const std::string_view path, ReplayArray_t& out) {
	std::ifstream file(path.data(), std::ios::in | std::ios::binary);
	if (!file.good()) {
		LOG::Error("Failed to read replay file: %s, Reason: file not found!", path.data());
		return false;
	}

	replay_file_header_t header;
	header.ReadFromStream(file);

	std::string sHead;
	ReadStreamString(file, sHead);

	if (sHead != std::string(REPLAY_ARRAY_HEAD)) {
		LOG::Error("Failed to read replay file: %s, Reason: array head not match!", path.data());
		file.close();
		return false;
	}

	auto iFrameLen = header.info.framelength;
	out.reserve(iFrameLen);

	for (size_t i = 0; i < iFrameLen; i++) {
		replay_frame_data_t frame;

		frame.ang.z = 0;
		file.read(reinterpret_cast<char*>(&frame.ang), sizeof(Vector2D));
		file.read(reinterpret_cast<char*>(&frame.pos), sizeof(frame.pos));
		file.read(reinterpret_cast<char*>(&frame.flags), sizeof(frame.flags));
		file.read(reinterpret_cast<char*>(&frame.mt), sizeof(frame.mt));

		replay_compress_button_t compressed_button {};
		file.read(reinterpret_cast<char*>(&compressed_button), sizeof(compressed_button));
		frame.buttons = compressed_button.Decompress();

		out.emplace_back(frame);
	}

	std::string sTail;
	ReadStreamString(file, sTail);

	file.close();

	return true;
}
