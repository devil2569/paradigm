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

		inline auto segment_base(
			segment_descriptor_register_64 const& gdtr,
			segment_selector const selector) -> uint64_t {

			if (selector.index == 0)
				return 0;

			auto const descriptor = reinterpret_cast<segment_descriptor_64*>(
				gdtr.base_address + static_cast<uint64_t>(selector.index) * 8);

			auto base_address =
				(uint64_t)descriptor->base_address_low |
				((uint64_t)descriptor->base_address_middle << 16) |
				((uint64_t)descriptor->base_address_high << 24);

			if (descriptor->descriptor_type == SEGMENT_DESCRIPTOR_TYPE_SYSTEM)
				base_address |= (uint64_t)descriptor->base_address_upper << 32;

			return base_address;
		}

		inline unsigned __int32 access_right(unsigned __int16 segment_selector)
		{
			hv_segment_selector selector;
			hv_segment_access_rights vmx_access_rights;

			selector.flags = segment_selector;

			if (selector.table == 0
				&& selector.index == 0)
			{
				vmx_access_rights.flags = 0;
				vmx_access_rights.unusable = TRUE;
				return vmx_access_rights.flags;
			}

			vmx_access_rights.flags = (__load_ar(segment_selector) >> 8);
			vmx_access_rights.unusable = 0;
			vmx_access_rights.reserved0 = 0;
			vmx_access_rights.reserved1 = 0;

			return vmx_access_rights.flags;
		}

	};
}

extern paradigm::utilities* util;
