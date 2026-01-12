#include "vmcs.h"
#include <cstdint>

paradigm::vmcs* vmcs = nullptr;

auto paradigm::vmcs::clear_vmcs(vcpu* vcpu) -> bool
{
	int stat = __vmx_vmclear(&vcpu->phys_vmcs_reg);
	if (stat) return false;

	return true;
}

auto paradigm::vmcs::load_vmcs(vcpu* vcpu) -> bool
{
	int stat = __vmx_vmptrld(&vcpu->phys_vmcs_reg);
	if (stat) return false;

	return true;
}

auto paradigm::vmcs::adjust_ctrl_value(uint32_t msr, uint64_t req_val) -> uint32_t
{
	ia32_vmx_true_ctls_register capabilities = { 0 };
	UINT32 effective_value;

	NT_ASSERT((msr == IA32_VMX_PINBASED_CTLS) ||
		(msr == IA32_VMX_PROCBASED_CTLS) ||
		(msr == IA32_VMX_EXIT_CTLS) ||
		(msr == IA32_VMX_ENTRY_CTLS) ||
		(msr == IA32_VMX_TRUE_PINBASED_CTLS) ||
		(msr == IA32_VMX_TRUE_PROCBASED_CTLS) ||
		(msr == IA32_VMX_TRUE_EXIT_CTLS) ||
		(msr == IA32_VMX_TRUE_ENTRY_CTLS) ||
		(msr == IA32_VMX_PROCBASED_CTLS2));

	capabilities.flags = __readmsr(msr);
	effective_value = (uint32_t)req_val;

	effective_value |= capabilities.allowed_0_settings;
	effective_value &= capabilities.allowed_1_settings;

	if ((effective_value & req_val) != req_val)
	{
		DbgPrint("not all of the requested vm ctrl fields are enabled\n");
		DbgPrint("msr: [0x%p]\nrequested: [%08x]\neffective: [%08x]\n", msr, req_val, effective_value);
	}
	return effective_value;
}

auto paradigm::vmcs::adjust_pinbased_reg(ia32_vmx_pinbased_ctls_register* req_ctls, ia32_vmx_basic_register basic_msr) -> void
{
	USHORT capability_msr = 0;
	capability_msr = basic_msr.vmx_controls ? IA32_VMX_TRUE_PINBASED_CTLS : IA32_VMX_PINBASED_CTLS;
	req_ctls->flags = adjust_ctrl_value(capability_msr, req_ctls->flags);;
}

auto paradigm::vmcs::adjust_entry_ctrls(ia32_vmx_entry_ctls_register* req_ctls, ia32_vmx_basic_register basic_msr) -> void
{
	USHORT capability_msr = 0;
	capability_msr = basic_msr.vmx_controls ? IA32_VMX_TRUE_ENTRY_CTLS : IA32_VMX_ENTRY_CTLS;
	req_ctls->flags = adjust_ctrl_value(capability_msr, req_ctls->flags);;
}

auto paradigm::vmcs::adjust_exit_ctrls(ia32_vmx_exit_ctls_register* req_ctls, ia32_vmx_basic_register basic_msr) -> void
{
	USHORT capability_msr = 0;
	capability_msr = basic_msr.vmx_controls ? IA32_VMX_TRUE_EXIT_CTLS : IA32_VMX_EXIT_CTLS;
	req_ctls->flags = adjust_ctrl_value(capability_msr, req_ctls->flags);;
}

auto paradigm::vmcs::adjust_cpu_primary_based_ctrls(ia32_vmx_procbased_ctls_register* req_ctls, ia32_vmx_basic_register basic_msr) -> void
{
	USHORT capability_msr = 0;
	capability_msr = basic_msr.vmx_controls ? IA32_VMX_TRUE_PROCBASED_CTLS : IA32_VMX_PROCBASED_CTLS;
	req_ctls->flags = adjust_ctrl_value(capability_msr, req_ctls->flags);;
}

auto paradigm::vmcs::adjust_cpu_secondary_based_ctrls(ia32_vmx_procbased_ctls2_register* req_ctls) -> void
{
	USHORT capability_msr = 0;
	capability_msr = IA32_VMX_PROCBASED_CTLS2;
	req_ctls->flags = adjust_ctrl_value(capability_msr, req_ctls->flags);;
}

auto paradigm::vmcs::setup_vmcs(vcpu* vcpu) -> bool
{
	if (!clear_vmcs(vcpu)) return false;
	if (!load_vmcs(vcpu)) return false;

	ia32_vmx_basic_register basic_msr{};
	basic_msr.flags = __readmsr(IA32_VMX_BASIC);

	ia32_vmx_pinbased_ctls_register pin_based_ctrl{};
	adjust_pinbased_reg(&pin_based_ctrl, basic_msr);

	ia32_vmx_entry_ctls_register entry_ctls{};
	entry_ctls.ia32e_mode_guest = 1;
	adjust_entry_ctrls(&entry_ctls, basic_msr);

	ia32_vmx_procbased_ctls_register cpu_primary_ctrls{};
	cpu_primary_ctrls.hlt_exiting = 1;
	adjust_cpu_primary_based_ctrls(&cpu_primary_ctrls, basic_msr);

	ia32_vmx_procbased_ctls2_register cpu_secondary_ctrls{};
	adjust_cpu_secondary_based_ctrls(&cpu_secondary_ctrls);

	ia32_vmx_exit_ctls_register exit_ctrls{};
	exit_ctrls.host_address_space_size = 1;
	adjust_exit_ctrls(&exit_ctrls, basic_msr);

	uint64_t cr0 = __readcr0();
	uint64_t cr4 = __readcr4();

	uint64_t cr0_fixed0 = __readmsr(IA32_VMX_CR0_FIXED0);
	uint64_t cr0_fixed1 = __readmsr(IA32_VMX_CR0_FIXED1);
	uint64_t cr4_fixed0 = __readmsr(IA32_VMX_CR4_FIXED0);
	uint64_t cr4_fixed1 = __readmsr(IA32_VMX_CR4_FIXED1);

	cr0 |= cr0_fixed0;
	cr0 &= cr0_fixed1;
	cr4 |= cr4_fixed0;
	cr4 &= cr4_fixed1;

	__vmx_vmwrite(VMCS_HOST_CR0, cr0);
	__vmx_vmwrite(VMCS_HOST_CR3, __readcr3());
	__vmx_vmwrite(VMCS_HOST_CR4, cr4);

	__vmx_vmwrite(VMCS_HOST_RSP, vcpu->vmm_stack + vmm_sz);
	// temporary, will write vm exit stub l8r
	__vmx_vmwrite(VMCS_HOST_RIP, 0);

	__vmx_vmwrite(VMCS_HOST_CS_SELECTOR, get_cs().flags & 0xf8);
	__vmx_vmwrite(VMCS_HOST_SS_SELECTOR, get_ss().flags & 0xf8);
	__vmx_vmwrite(VMCS_HOST_DS_SELECTOR, get_ds().flags & 0xf8);
	__vmx_vmwrite(VMCS_HOST_ES_SELECTOR, get_es().flags & 0xf8);
	__vmx_vmwrite(VMCS_HOST_FS_SELECTOR, get_fs().flags & 0xf8);
	__vmx_vmwrite(VMCS_HOST_GS_SELECTOR, get_gs().flags & 0xf8);
	__vmx_vmwrite(VMCS_HOST_TR_SELECTOR, get_tr().flags & 0xf8);

	segment_descriptor_register_64 gdtr, idtr;
	_sgdt(&gdtr);
	__sidt(&idtr);

	__vmx_vmwrite(VMCS_HOST_FS_BASE, __readmsr(IA32_FS_BASE));
	__vmx_vmwrite(VMCS_HOST_GS_BASE, __readmsr(IA32_GS_BASE));
	__vmx_vmwrite(VMCS_HOST_TR_BASE, util->segment_base(gdtr, get_tr()));

	__vmx_vmwrite(VMCS_HOST_GDTR_BASE, gdtr.base_address);
	__vmx_vmwrite(VMCS_HOST_IDTR_BASE, idtr.base_address);

	__vmx_vmwrite(VMCS_HOST_SYSENTER_CS, __readmsr(IA32_SYSENTER_CS));
	__vmx_vmwrite(VMCS_HOST_SYSENTER_ESP, __readmsr(IA32_SYSENTER_ESP));
	__vmx_vmwrite(VMCS_HOST_SYSENTER_EIP, __readmsr(IA32_SYSENTER_EIP));

	DbgPrint("done writing host vmcs fields..\n");

	__vmx_vmwrite(VMCS_GUEST_CR0, __readcr0());
	__vmx_vmwrite(VMCS_GUEST_CR3, __readcr3());
	__vmx_vmwrite(VMCS_GUEST_CR4, __readcr4());

	__vmx_vmwrite(VMCS_GUEST_DR7, __readdr(7));

	__vmx_vmwrite(VMCS_GUEST_RSP, 0);
	__vmx_vmwrite(VMCS_GUEST_RIP, 0);
	__vmx_vmwrite(VMCS_GUEST_RFLAGS, __readeflags());
	
	__vmx_vmwrite(VMCS_GUEST_ES_SELECTOR, get_es().flags);
	__vmx_vmwrite(VMCS_GUEST_CS_SELECTOR, get_cs().flags);
	__vmx_vmwrite(VMCS_GUEST_SS_SELECTOR, get_ss().flags);
	__vmx_vmwrite(VMCS_GUEST_DS_SELECTOR, get_ds().flags);
	__vmx_vmwrite(VMCS_GUEST_FS_SELECTOR, get_fs().flags);
	__vmx_vmwrite(VMCS_GUEST_GS_SELECTOR, get_gs().flags);
	__vmx_vmwrite(VMCS_GUEST_LDTR_SELECTOR, get_ldtr().flags);
	__vmx_vmwrite(VMCS_GUEST_TR_SELECTOR, get_tr().flags);

	__vmx_vmwrite(VMCS_GUEST_ES_LIMIT, __segmentlimit(get_es().flags));
	__vmx_vmwrite(VMCS_GUEST_CS_LIMIT, __segmentlimit(get_cs().flags));
	__vmx_vmwrite(VMCS_GUEST_SS_LIMIT, __segmentlimit(get_ss().flags));
	__vmx_vmwrite(VMCS_GUEST_DS_LIMIT, __segmentlimit(get_ds().flags));
	__vmx_vmwrite(VMCS_GUEST_FS_LIMIT, __segmentlimit(get_fs().flags));
	__vmx_vmwrite(VMCS_GUEST_GS_LIMIT, __segmentlimit(get_gs().flags));
	__vmx_vmwrite(VMCS_GUEST_LDTR_LIMIT, __segmentlimit(get_ldtr().flags));
	__vmx_vmwrite(VMCS_GUEST_TR_LIMIT, __segmentlimit(get_tr().flags));
	__vmx_vmwrite(VMCS_GUEST_GDTR_LIMIT, gdtr.limit);
	__vmx_vmwrite(VMCS_GUEST_IDTR_LIMIT, idtr.limit);

	__vmx_vmwrite(VMCS_GUEST_ES_ACCESS_RIGHTS, util->access_right(get_es().flags));
	__vmx_vmwrite(VMCS_GUEST_CS_ACCESS_RIGHTS, util->access_right(get_cs().flags));
	__vmx_vmwrite(VMCS_GUEST_SS_ACCESS_RIGHTS, util->access_right(get_ss().flags));
	__vmx_vmwrite(VMCS_GUEST_DS_ACCESS_RIGHTS, util->access_right(get_ds().flags));
	__vmx_vmwrite(VMCS_GUEST_FS_ACCESS_RIGHTS, util->access_right(get_fs().flags));
	__vmx_vmwrite(VMCS_GUEST_GS_ACCESS_RIGHTS, util->access_right(get_gs().flags));
	__vmx_vmwrite(VMCS_GUEST_LDTR_ACCESS_RIGHTS, util->access_right(get_ldtr().flags));
	__vmx_vmwrite(VMCS_GUEST_TR_ACCESS_RIGHTS, util->access_right(get_tr().flags));

	__vmx_vmwrite(VMCS_GUEST_ES_BASE, util->segment_base(gdtr, get_es()));
	__vmx_vmwrite(VMCS_GUEST_CS_BASE, util->segment_base(gdtr, get_cs()));
	__vmx_vmwrite(VMCS_GUEST_SS_BASE, util->segment_base(gdtr, get_ss()));
	__vmx_vmwrite(VMCS_GUEST_DS_BASE, util->segment_base(gdtr, get_ds()));
	__vmx_vmwrite(VMCS_GUEST_FS_BASE, __readmsr(IA32_FS_BASE));
	__vmx_vmwrite(VMCS_GUEST_GS_BASE, __readmsr(IA32_GS_BASE));
	__vmx_vmwrite(VMCS_GUEST_LDTR_BASE, util->segment_base(gdtr, get_ldtr()));
	__vmx_vmwrite(VMCS_GUEST_TR_BASE, util->segment_base(gdtr, get_tr()));
	__vmx_vmwrite(VMCS_GUEST_GDTR_BASE, gdtr.base_address);
	__vmx_vmwrite(VMCS_GUEST_IDTR_BASE, idtr.base_address);

	__vmx_vmwrite(VMCS_GUEST_SYSENTER_CS, __readmsr(IA32_SYSENTER_CS));
	__vmx_vmwrite(VMCS_GUEST_SYSENTER_ESP, __readmsr(IA32_SYSENTER_ESP));
	__vmx_vmwrite(VMCS_GUEST_SYSENTER_EIP, __readmsr(IA32_SYSENTER_EIP));

	__vmx_vmwrite(VMCS_GUEST_DEBUGCTL, __readmsr(IA32_DEBUGCTL));
	__vmx_vmwrite(VMCS_GUEST_VMCS_LINK_POINTER, MAXULONG64);

	__vmx_vmwrite(VMCS_CTRL_PIN_BASED_VM_EXECUTION_CONTROLS, pin_based_ctrl.flags);
	__vmx_vmwrite(VMCS_CTRL_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, cpu_primary_ctrls.flags);
	__vmx_vmwrite(VMCS_CTRL_SECONDARY_PROCESSOR_BASED_VM_EXECUTION_CONTROLS, cpu_secondary_ctrls.flags);
	__vmx_vmwrite(VMCS_CTRL_VMENTRY_CONTROLS, entry_ctls.flags);
	__vmx_vmwrite(0x0000400C, exit_ctrls.flags);    
 DbgPrint("done writing guest vmcs fields..\n");
	DbgPrint("done configuring vmcs, executing vmlaunch\n");
	return true;
}
