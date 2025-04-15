#pragma once

//// Compilation context
#if defined(__clang__)
	#define COMPILER_CLANG 1
#elif defined(__GNUC__)
	#define COMPILER_GCC 1
#elif defined(_MSC_VER)
	#define COMPILER_MSVC 1
#else
	#error "Unsupported compiler"
#endif

#if defined(__linux__)
	#define OS_LINUX 1
	#define _DEFAULT_SOURCE
#elif defined(_WIN32) || defined(_WIN64)
	#define OS_WINDOWS 1
#else
	#error "Unsupported operating system"
#endif
