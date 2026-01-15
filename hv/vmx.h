#pragma once
#include <intrin.h>
#include <ntddk.h>

#include "ia32.hpp"
#include "regions.h"


namespace paradigm
{
	class vmx
	{
	public:
		auto detect_vmx_support() -> bool;

		auto adjust_cr0() -> cr0;

		auto adjust_cr4() -> cr4;

		auto increment_guest_ip() -> void;

		auto start(vcpu* vcpu) -> bool;
	};
}

extern paradigm::vmx* vmx;
