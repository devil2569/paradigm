#pragma once
#include <ntddk.h>
#include <intrin.h>
#include <cstdint>

namespace paradigm
{
	class utilities final
	{
	public:
		inline auto virt_to_phys(void* va) -> uint64_t
		{
			return MmGetPhysicalAddress(va).QuadPart;
		}

		inline auto phys_to_virt(uint64_t phys) -> uint64_t
		{
			PHYSICAL_ADDRESS physical_addr = { 0 };
			physical_addr.QuadPart = phys;
			return reinterpret_cast<uint64_t>(MmGetVirtualForPhysical(physical_addr));
		}
	};
}

extern paradigm::utilities* util;
