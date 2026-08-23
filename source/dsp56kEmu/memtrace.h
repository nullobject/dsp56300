#pragma once

// Lightweight DSP memory-write tracer for reverse-engineering.
// Enabled only when JitConfig::memoryWritesCallCpp is set (so every write
// routes through callDSPMemWrite in jitmem.cpp). Records (area,addr,value)
// for writes whose offset falls in [lo,hi) while tracing is active.

#include <cstdint>
#include <vector>

namespace dsp56k
{
	class DSP;

	struct MemTraceEntry
	{
		uint8_t  area;   // EMemArea
		uint8_t  write;  // 1 = write, 0 = read
		uint32_t addr;
		uint32_t value;
		uint32_t pc;     // block-granular DSP program counter of the accessing code
	};

	// Sink mode: when a sink is installed, entries are handed to it instead of
	// being accumulated in the vector. Needed for whole-run profiling, where the
	// access count is far too large to store.
	using MemTraceSink = void (*)(uint8_t _area, bool _write, uint32_t _addr, uint32_t _value, uint32_t _pc);
	void                              memTraceSetSink(MemTraceSink _sink);

	// Instruction-fetch profiling. The JIT never fetches instructions the way the
	// DSP does - it compiles a block once and runs it - so fetch traffic has to be
	// reconstructed: count how often each block runs, times the block's length in
	// P words. Must be enabled before any block is compiled, otherwise blocks that
	// already exist carry no counter. Counters are indexed by block start PC.
	void      fetchProfileEnable(uint32_t _pMemSize);
	bool      fetchProfileActive();
	void      fetchProfileClear();
	uint64_t* fetchProfileCounts();
	uint32_t* fetchProfileSizes();
	uint32_t* fetchProfileInstrs();
	uint32_t  fetchProfileSize();
	uint32_t  fetchProfileUnencodable();	// blocks whose counter was out of displacement range
	void      fetchProfileCountUnencodable();

	// Opcode profiling. Recorded at compile time, one entry per P word, so a
	// post-run pass can weight each instruction by how often its block ran.
	// g_opcodes[instr].m_assembly gives the mnemonic.
	void      opcodeProfileEnable(uint32_t _pMemSize);
	bool      opcodeProfileActive();
	uint32_t* opcodeProfileWord();		// [pc] -> the opcode word, 0xffffffff where nothing was compiled
	uint8_t*  opcodeProfileOpSize();	// [pc] -> length in P words
	uint8_t*  opcodeProfileParallel();	// [pc] -> 1 if the opcode carries a parallel move

	// Peripheral profiling. Peripheral space is not reachable through Memory - the
	// JIT routes it through readPeriph/writePeriph - so it needs its own hooks.
	// Indexed by [area * 128 + (addr - 0xffff80)], area 0 = X, 1 = Y.
	void      periphProfileEnable();
	bool      periphProfileActive();
	void      periphProfileRecord(uint32_t _area, uint32_t _addr, bool _write);
	void      periphProfileSite(uint32_t _area, uint32_t _addr);
	uint64_t* periphProfileReads();
	uint64_t* periphProfileWrites();
	uint32_t* periphProfileSites();		// compile-time sites, for the readAsPtr fast
										// path whose reads never call back into C++
	void      periphProfileMark();		// snapshot the counts, so boot-time setup can be
										// told apart from steady-state traffic
	uint64_t* periphProfileMarkReads();
	uint64_t* periphProfileMarkWrites();

	// Per-instruction trace hook, for lockstep co-simulation against an
	// independent implementation. Emitted ahead of every op, after flushing the
	// JIT register pool, so the sink sees coherent architectural state as it
	// stood *before* the instruction runs. Pair it with a JitConfig of
	// maxInstructionsPerBlock = 1 and linkJitBlocks = false, otherwise memory
	// writes from a whole block land between two consecutive callbacks.
	// Arming is separate from installing the sink, because the hook is emitted at
	// compile time: a sink installed before boot would put the hook in every block
	// the boot code compiles and trace from DSP reset instead of from the window.
	// Arm at the window, then destroy all blocks so they are re-emitted with it.
	using InstTraceSink = void (*)(DSP* _dsp, uint32_t _pc);
	void instTraceSetSink(InstTraceSink _sink);
	void instTraceArm(bool _armed);
	bool instTraceActive();
	void callDSPInstTrace(DSP* _dsp, uint32_t _pc);

	void                              memTraceBegin(uint32_t _lo, uint32_t _hi);
	void                              memTraceEnd();
	bool                              memTraceActive();
	void                              memTraceRecord(uint8_t _area, bool _write, uint32_t _addr, uint32_t _value, uint32_t _pc);
	const std::vector<MemTraceEntry>& memTraceData();
	void                              memTraceClear();
}
