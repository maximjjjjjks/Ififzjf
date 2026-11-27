#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <filesystem>

using namespace geode::prelude;

// Is the level's original song available locally?
static bool isOriginalSongInstalled(GJGameLevel* level) {
	if (!level) return true;

	int songID = level->m_songID;
	// songID == 0 -> official track, always bundled with the game
	if (songID == 0) return true;

	auto path = MusicDownloadManager::sharedState()->pathForSong(songID);
	return CCFileUtils::sharedFileUtils()->isFileExist(path.c_str());
}

// Resolve fallback: 1) file picker  2) <save dir>/tracks/<track-name>  3) bundled resource
static std::filesystem::path getFallbackPath() {
	std::error_code ec;

	auto picked = Mod::get()->getSettingValue<std::filesystem::path>("song-file");
	if (!picked.empty() && std::filesystem::exists(picked, ec)) {
		return picked;
	}

	auto name = Mod::get()->getSettingValue<std::string>("track-name");
	if (!name.empty()) {
		auto p = Mod::get()->getSaveDir() / "tracks" / name;
		if (std::filesystem::exists(p, ec)) return p;
	}

	return Mod::get()->getResourcesDir() / "fallback.mp3";
}

$on_mod(Loaded) {
	// create the tracks folder so the user can drop files in it
	std::error_code ec;
	auto dir = Mod::get()->getSaveDir() / "tracks";
	std::filesystem::create_directories(dir, ec);
	log::info("Fallback tracks folder: {}", dir.string());
}

class $modify(FallbackPlayLayer, PlayLayer) {
	// Called on level start AND on every restart (resetLevel)
	void startMusic() {
		if (!Mod::get()->getSettingValue<bool>("enabled")
			|| isOriginalSongInstalled(m_level)) {
			PlayLayer::startMusic();
			return;
		}

		auto path = getFallbackPath();
		std::error_code ec;
		if (!std::filesystem::exists(path, ec)) {
			log::warn("Fallback song not found: {}", path.string());
			PlayLayer::startMusic();
			return;
		}

		auto engine = FMODAudioEngine::sharedEngine();
		bool loop = Mod::get()->getSettingValue<bool>("loop");

		engine->stopAllMusic(true);
		engine->playMusic(path.string(), loop, 0.f, 0);

		// StartPos / practice checkpoint: jump to current level time
		float t = m_gameState.m_levelTime;
		if (t > 0.f) {
			engine->setMusicTimeMMS(static_cast<unsigned int>(t * 1000.f), true, 0);
		}
	}
};
