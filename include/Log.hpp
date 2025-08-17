#pragma once

// Direct integration with Pandemonium-Log. We know its public umbrella header
// is located at Include/PandemoniumLog.h (added to include dirs by CMake).
// This header pulls in Logger.h and related components.

#if defined(RTVK_LOG_EXTERNAL_HEADER)
  #pragma message("[rtvk] Using external log header override: " STRINGIFY(RTVK_LOG_EXTERNAL_HEADER))
  #include RTVK_LOG_EXTERNAL_HEADER
#else
  #if __has_include(<PandemoniumLog.h>)
    #include <PandemoniumLog.h>
  #elif __has_include("PandemoniumLog.h")
    #include "PandemoniumLog.h"
  #else
    #error "PandemoniumLog.h not found in include path. Ensure CMake added Pandemonium-Log/Include directory or define RTVK_LOG_EXTERNAL_HEADER."
  #endif
#endif

// Ensure essential macros exist
#if !defined(LOG_INFO) || !defined(LOG_WARN) || !defined(LOG_ERROR)
  #error "Pandemonium-Log expected to define LOG_INFO / LOG_WARN / LOG_ERROR macros. Adjust Log.hpp if API differs."
#endif

#ifndef LOG_TRACE
  #define LOG_TRACE(...) LOG_INFO(__VA_ARGS__)
#endif

#ifndef LOG_FATAL
  #define LOG_FATAL(...) LOG_ERROR(__VA_ARGS__)
#endif

#define RTVK_LOG_TRACE(...) LOG_TRACE(__VA_ARGS__)
#define RTVK_LOG_INFO(...)  LOG_INFO(__VA_ARGS__)
#define RTVK_LOG_WARN(...)  LOG_WARN(__VA_ARGS__)
#define RTVK_LOG_ERROR(...) LOG_ERROR(__VA_ARGS__)
#define RTVK_LOG_FATAL(...) LOG_FATAL(__VA_ARGS__)
