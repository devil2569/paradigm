#pragma once
#include <ntddk.h>
#include <intrin.h>

#include "ia32.hpp"
#include "commons.hpp"
#include "util.h"

namespace paradigm
{
	class regs final {
	public:
		auto allocate_vmx_reg(vcpu* vcpu) -> bool;

		auto allocate_vmcs_reg(vcpu* vcpu) -> bool;

		auto allocate_vmm_stack(vcpu* vcpu) -> bool;

		auto allocate_regions(vcpu* vcpu) -> bool;
	};
}

extern paradigm::regs* regions;
