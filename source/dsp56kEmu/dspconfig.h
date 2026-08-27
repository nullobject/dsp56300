#pragma once

#include "dsp56kBase/buildconfig.h"

namespace dsp56k
{
#ifdef DSP56K_AAR_TRANSLATE
	constexpr bool g_useAARTranslate = true;
#else
	constexpr bool g_useAARTranslate = false;
#endif

#if defined(HAVE_X86_64) || defined(HAVE_ARM64)
	constexpr bool g_jitSupported = true;

	// Tracing builds run the interpreter instead. The JIT keeps architectural
	// state in host registers and writes it back at block boundaries, so a
	// per-instruction observer sees whatever the register pool last flushed --
	// which inside a REP is not the machine's state at all. The interpreter
	// works on the register struct itself, so what an observer reads is real by
	// construction rather than by flushing at the right moment.
	// See nightcapaudio/pathogen#34.
#ifndef DSP56300_USE_JIT
#define DSP56300_USE_JIT 1
#endif
#else
	constexpr bool g_jitSupported = false;
#endif
}
