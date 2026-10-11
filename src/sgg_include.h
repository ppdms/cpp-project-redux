#pragma once
// Single point of entry for the SGG graphics header across platforms.
// Desktop builds keep using the prebuilt headers in lib/<platform>/include,
// while the Emscripten build uses the vendored sources in third_party/sgg.
#ifdef __EMSCRIPTEN__
#include <sgg/graphics.h>
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#ifdef _WIN64
#include "sgg/graphics.h"
#endif
#elif __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_MAC
#include "graphics.h"
#endif
#elif __linux__
#include "graphics.h"
#else
#error "Unknown compiler"
#endif
