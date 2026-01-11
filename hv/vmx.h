#pragma once
#include <intrin.h>
#include <ntddk.h>

#include "ia32.hpp"
#include "regions.h"


namespace paradigm
{
	class vmx final
	{
	public:
		auto detect_vmx_support() -> bool;

		auto adjust_cr0() -> cr0;

		auto adjust_cr4() -> cr4;

		auto start(vcpu* vcpu) -> void;
	};
}

extern paradigm::vmx* vmx;
