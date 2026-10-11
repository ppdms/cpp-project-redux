#include <sgg/AudioManager.h>

#ifdef __EMSCRIPTEN__
#include <cstdio>
#include <unistd.h>
#include <emscripten.h>

// The web build ships OGG Vorbis audio (see assets/*.ogg, generated from the
// WAVs) instead of the 55MB of uncompressed WAVs the desktop build uses.
// Filenames are rewritten at this single choke point so game code is unchanged.
static std::string sgg_web_audio_path(const std::string& soundfile)
{
	const std::string suffix = ".wav";
	if (soundfile.size() > suffix.size() &&
		soundfile.compare(soundfile.size() - suffix.size(), suffix.size(), suffix) == 0)
	{
		return soundfile.substr(0, soundfile.size() - suffix.size()) + ".ogg";
	}
	return soundfile;
}

struct SggPendingMusic
{
	AudioManager* mgr;
	std::string file;
	float volume;
	bool looping;
	int fade_time;
};

void sgg_music_fetched(unsigned handle, void* arg, const char* file)
{
	(void)handle;
	(void)file;
	SggPendingMusic* pending = (SggPendingMusic*)arg;
	pending->mgr->fetching.erase(pending->file);
	// The file now exists in MEMFS; play it (late, but automatically).
	pending->mgr->playMusic(pending->file, pending->volume, pending->looping, pending->fade_time);
	delete pending;
}

void sgg_music_fetch_failed(unsigned handle, void* arg, int status)
{
	(void)handle;
	SggPendingMusic* pending = (SggPendingMusic*)arg;
	printf("SGG audio: failed to download %s (status %d); music will retry on next playMusic call\n",
		pending->file.c_str(), status);
	pending->mgr->fetching.erase(pending->file);
	delete pending;
}
#else
static const std::string& sgg_web_audio_path(const std::string& soundfile)
{
	return soundfile;
}
#endif

void AudioManager::playSound(std::string soundfile, float volume, bool looping)
{
	std::string path = sgg_web_audio_path(soundfile);
	auto siter = sounds.find(path);
	if (siter == sounds.end())
	{
		Mix_Chunk * as = Mix_LoadWAV(path.c_str());
		if (as)
		{
			sounds.insert(std::pair<std::string, Mix_Chunk*>(path, as));
			Mix_VolumeChunk(as, (unsigned char)(MIX_MAX_VOLUME*volume));
			Mix_PlayChannel(-1, as, looping ? -1 : 0);
		}
#ifdef __EMSCRIPTEN__
		else
		{
			printf("SGG audio: Mix_LoadWAV failed for %s: %s\n", path.c_str(), Mix_GetError());
		}
#endif
	}
	else
	{
		Mix_VolumeChunk(siter->second, (unsigned char)(MIX_MAX_VOLUME*volume));
		Mix_PlayChannel(-1, siter->second, looping ? -1 : 0);
	}
}

void AudioManager::playMusic(std::string soundfile, float volume, bool looping, int fade_time)
{
	std::string path = sgg_web_audio_path(soundfile);
#ifdef __EMSCRIPTEN__
	// Deferred tracks (castle/ending music) are not in the preload bundle:
	// fetch them on first use. The onload callback retries the play, so the
	// game code's single playMusic call still results in music, just late.
	if (scores.find(path) == scores.end() && access(path.c_str(), R_OK) != 0)
	{
		if (fetching.find(path) == fetching.end())
		{
			fetching.insert(path);
			// NOTE: pending->file is the rewritten .ogg path (the fetching
			// key), not the caller's .wav name; playMusic re-rewrites it.
			SggPendingMusic* pending = new SggPendingMusic{ this, path, volume, looping, fade_time };
			// Deferred OGGs are served next to the page at web/assets/.
			emscripten_async_wget2(path.c_str(), path.c_str(), "GET", "", pending,
				sgg_music_fetched, sgg_music_fetch_failed, nullptr);
		}
		return;
	}
#endif
	auto siter = scores.find(path);
	if (siter == scores.end())
	{
		Mix_Music * ms = Mix_LoadMUS(path.c_str());
		if (ms)
		{
			scores.insert(std::pair<std::string, Mix_Music*>(path, ms));
			Mix_VolumeMusic((unsigned char)(MIX_MAX_VOLUME*volume));
			Mix_FadeInMusic(ms, looping ? -1 : 1, fade_time);
		}
#ifdef __EMSCRIPTEN__
		else
		{
			printf("SGG audio: Mix_LoadMUS failed for %s: %s\n", path.c_str(), Mix_GetError());
		}
#endif
	}
	else
	{
		Mix_VolumeMusic((unsigned char)(MIX_MAX_VOLUME*volume));
		Mix_FadeInMusic(siter->second, looping ? -1 : 1, fade_time);
	}
}

void AudioManager::stopMusic(int fade_time)
{
	Mix_FadeOutMusic(fade_time);
	
}

AudioManager::AudioManager()
{
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
		return;
	initialized = true;
}

AudioManager::~AudioManager()
{
	
	for (auto sound : sounds)
		Mix_FreeChunk(sound.second);
	for (auto score : scores)
		Mix_FreeMusic(score.second);

	Mix_CloseAudio();
}
