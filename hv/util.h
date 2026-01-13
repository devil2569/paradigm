#pragma once
#include <ntddk.h>
#include <intrin.h>
#include <cstdint>
#include "ia32.hpp"
#include "commons.hpp"

namespace paradigm
{
	class utilities final
	{
	public:
		auto virt_to_phys(void* va) -> uint64_t;
		auto phys_to_virt(uint64_t phys) -> uint64_t;
		auto segment_base(segment_descriptor_register_64 const& gdtr, segment_selector const selector) -> uint64_t;
		auto access_right(unsigned __int16 segment_selector) -> unsigned __int32;
	};
}

extern paradigm::utilities* util;
