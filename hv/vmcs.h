#pragma once
#include <ntddk.h>
#include <intrin.h>

#include "ia32.hpp"
#include "commons.hpp"
#include "util.h"

namespace paradigm
{
	class vmcs final
	{
	public:
		auto clear_vmcs(vcpu* vcpu) -> bool;

		auto load_vmcs(vcpu* vcpu) -> bool;

		auto adjust_ctrl_value(uint32_t msr, uint64_t req_val) -> uint32_t;

		auto adjust_pinbased_reg(ia32_vmx_pinbased_ctls_register* req_ctls, ia32_vmx_basic_register basic_msr) -> void;

		auto adjust_entry_ctrls(ia32_vmx_entry_ctls_register* req_ctls, ia32_vmx_basic_register basic_msr) -> void;

		auto adjust_exit_ctrls(ia32_vmx_exit_ctls_register* req_ctls, ia32_vmx_basic_register basic_msr) -> void;

		auto adjust_cpu_primary_based_ctrls(ia32_vmx_procbased_ctls_register* req_ctls, ia32_vmx_basic_register basic_msr) -> void; 

		auto adjust_cpu_secondary_based_ctrls(ia32_vmx_procbased_ctls2_register* req_ctls) -> void;

		auto setup_vmcs(vcpu* vcpu) -> bool;
	};
}

extern paradigm::vmcs* vmcs;
