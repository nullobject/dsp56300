#pragma once

// Lightweight DSP memory-write tracer for reverse-engineering.
// Enabled only when JitConfig::memoryWritesCallCpp is set (so every write
// routes through callDSPMemWrite in jitmem.cpp). Records (area,addr,value)
// for writes whose offset falls in [lo,hi) while tracing is active.

#include <cstdint>
#include <vector>

namespace dsp56k
{
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
	uint32_t  fetchProfileSize();
	uint32_t  fetchProfileUnencodable();	// blocks whose counter was out of displacement range
	void      fetchProfileCountUnencodable();

	void                              memTraceBegin(uint32_t _lo, uint32_t _hi);
	void                              memTraceEnd();
	bool                              memTraceActive();
	void                              memTraceRecord(uint8_t _area, bool _write, uint32_t _addr, uint32_t _value, uint32_t _pc);
	const std::vector<MemTraceEntry>& memTraceData();
	void                              memTraceClear();
}
