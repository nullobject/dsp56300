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
	};

	void                              memTraceBegin(uint32_t _lo, uint32_t _hi);
	void                              memTraceEnd();
	bool                              memTraceActive();
	void                              memTraceRecord(uint8_t _area, bool _write, uint32_t _addr, uint32_t _value);
	const std::vector<MemTraceEntry>& memTraceData();
	void                              memTraceClear();
}
