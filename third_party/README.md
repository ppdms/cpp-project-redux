# Vendored third-party sources (Emscripten builds only)

## `sgg/`
Snapshot of the [SGG graphics library](https://github.com/cgaueb/sgg) sources,
commit `81e50d6` (2026-08-25). The prebuilt `lib/*/libsgg.a` archives in this
repo are native desktop binaries and cannot link into WebAssembly, so the web
build compiles SGG from these sources instead.

Excluded: `audio.cpp`/`audio.h` (an older SDL audio backend nothing references;
the build uses `AudioManager.cpp` with SDL_mixer, same as desktop).

### Emscripten deltas (all behind `#ifdef __EMSCRIPTEN__`)
- `GLbackend.h`, `texture.h`, `fonts.h`, `shader.h`/`shader.cpp`: include
  `<GLES3/gl3.h>` instead of `<GL/glew.h>` (GLES3 headers provide the desktop
  entry points SGG uses: VAOs, `glTexImage2D`, `glGenerateMipmap`).
- `commonshaders.h`, `fonts.cpp`, `shader.cpp`: GLSL `#version 120`
  (`attribute`/`varying`/`texture2D`/`gl_FragColor`) translated to
  `#version 300 es` (`in`/`out`/`texture`/explicit `out vec4`).
  The `#version` line is the first byte of each shader string: ANGLE rejects
  a leading blank line, which desktop drivers silently tolerate.
- `fonts.cpp` glyph upload: sized internal format `GL_R8` instead of unsized
  `GL_RED` (WebGL2 reports `INVALID_VALUE` for unsized `GL_RED`, leaving the
  texture without storage, so no text rendered).
- `shader.h`/`shader.cpp`: `Shader` members default-initialized and the
  destructor guards `program == 0`, so a failed compile logs instead of
  crashing in `glDetachShader`.
- `GLbackend.cpp` `init()`: requests an OpenGL ES 3.0 context via
  `SDL_GL_CONTEXT_PROFILE_MASK`/`SDL_GL_CONTEXT_PROFILE_ES` *before*
  `SDL_GL_CreateContext` (desktop code sets attributes after, which SDL
  ignores); skips `glewInit`; uses `glClearDepthf`.
- `GLbackend.cpp` `drawRect`/`drawSector`, `texture.cpp`: drops desktop-only
  `glEnable(GL_TEXTURE_2D)`/`glPolygonMode` calls; texture internal format
  `4` becomes `GL_RGBA` (unsized `4` is invalid in ES).
- `shader.cpp` `setFragmentLocation()`: no-op (no callers; ES 3.0 binds
  fragment outputs via layout qualifiers).
- Resize needs no patch: Emscripten's SDL2 port reports canvas resizes via the
  regular `SDL_WINDOWEVENT`/`SDL_WINDOWEVENT_RESIZED` path, which the desktop
  `processEvent()` handler already covers.
- `GLbackend.cpp` `processEvent()`: ESC no longer quits on web (no window to
  close; exiting fullscreen also produces ESC).
- `GLbackend.cpp` `processMessages()`: skips `SDL_Delay(5)`; the browser
  paces frames via `requestAnimationFrame`.
- `texture.cpp`: skips the power-of-two upscale on web (WebGL2 supports NPOT
  with `CLAMP_TO_EDGE`; the upscale ~doubled this game's texture VRAM:
  74MB of PNG pixels became 145MB) and uses `GL_LINEAR` minification without
  mipmaps (sprites render ~1:1, so mipmaps cost 33% VRAM for no visual gain).
- `fonts.h`/`fonts.cpp`: rasterized glyphs are cached per (font, codepoint)
  on web. Desktop SGG re-rasterizes every visible character with FreeType
  every frame plus a `glTexImage2D` re-upload; cached draws reuse the stored
  pixels through the same upload + quad path (identical output).
- `graphics.cpp` `startMessageLoop()`: uses `emscripten_set_main_loop` with
  `simulate_infinite_loop=1` instead of the blocking `while` loop (a blocking
  loop would freeze the page); `stopMessageLoop()` cancels it.
- `AudioManager.h`/`AudioManager.cpp`: on web, `.wav` filenames are rewritten
  to `.ogg` at this single choke point (game code unchanged; desktop still
  uses WAV). `assets/*.ogg` are Vorbis conversions of the WAVs (q5 music,
  q3 SFX): 54.6MB -> 4.5MB. `castle_music`/`ending_theme` are excluded from
  the preload bundle and fetched with `emscripten_async_wget2` on first
  `playMusic`; the onload callback retries the play so music starts late but
  automatically, a `fetching` set prevents duplicate in-flight requests, and
  fetch failures log once without breaking the game. Load failures
  (`Mix_LoadMUS`/`Mix_LoadWAV`) also log with `Mix_GetError()` instead of
  failing silently. No mixer flag changes were needed: the Emscripten
  SDL_mixer port builds Vorbis support by default (`SDL2_MIXER_FORMATS=ogg`).

## `glm/`
Snapshot of the GLM headers SGG expects, from SGG's `3rdparty/include/glm`
(GLM 0.9.6.3). Used for the web build only; desktop builds keep using the
system/bundled GLM they already use.
