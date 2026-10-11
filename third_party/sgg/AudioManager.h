#pragma once

#include <unordered_map>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#ifdef __EMSCRIPTEN__
#include <unordered_set>
#endif

class AudioManager
{
	std::unordered_map<std::string, Mix_Chunk*> sounds;
	std::unordered_map<std::string, Mix_Music*> scores;
	bool initialized = true;
#ifdef __EMSCRIPTEN__
	// Deferred music tracks currently being fetched from the server (see
	// AudioManager.cpp). Guards against duplicate in-flight requests.
	std::unordered_set<std::string> fetching;
	friend void sgg_music_fetched(unsigned, void*, const char*);
	friend void sgg_music_fetch_failed(unsigned, void*, int);
#endif
public:
	void playSound(std::string soundfile, float volume, bool looping = false);
	void playMusic(std::string soundfile, float volume, bool looping = true, int fade_time = 0);
	void stopMusic(int fade_time = 0);
	AudioManager();
	~AudioManager();
	
};