#include "vmx.h"

paradigm::vmx* vmx = nullptr;

auto paradigm::vmx::detect_vmx_support() -> bool
{
	int regs[4];
	ia32_feature_control_register feature_ctrl{};

	__cpuid(regs, 1);

	const bool vmxe = (regs[2] >> 5) & 1;

	feature_ctrl.flags = __readmsr(IA32_FEATURE_CONTROL);

	feature_ctrl.lock_bit == 0
		? (feature_ctrl.lock_bit = TRUE, feature_ctrl.enable_vmx_outside_smx = TRUE, __writemsr(IA32_FEATURE_CONTROL, feature_ctrl.flags))
		: void();

	return feature_ctrl.enable_vmx_outside_smx ? vmxe : false;
}

auto paradigm::vmx::adjust_cr0() -> cr0
{
	cr0 new_cr0{};
	cr0 fixed0_cr0{};
	cr0 fixed1_cr0{};

	new_cr0.flags = __readcr0();

	fixed0_cr0.flags = __readmsr(IA32_VMX_CR0_FIXED0);
	fixed1_cr0.flags = __readmsr(IA32_VMX_CR0_FIXED1);

	new_cr0.flags = (new_cr0.flags & fixed1_cr0.flags) | fixed0_cr0.flags;

	__writecr0(new_cr0.flags);
	return new_cr0;
}

auto paradigm::vmx::adjust_cr4() -> cr4
{
	cr4 new_cr4{};
	cr4 fixed0_cr4{};
	cr4 fixed1_cr4{};

	new_cr4.flags = __readcr4();

	fixed0_cr4.flags = __readmsr(IA32_VMX_CR4_FIXED0);
	fixed1_cr4.flags = __readmsr(IA32_VMX_CR4_FIXED1);

	new_cr4.flags = (new_cr4.flags & fixed1_cr4.flags) | fixed0_cr4.flags;

	__writecr4(new_cr4.flags);
	return new_cr4;
}

auto paradigm::vmx::start(vcpu* vcpu) -> void
{
	!detect_vmx_support() ? false : true;

	adjust_cr0();
	adjust_cr4();

	!regions->allocate_regions(vcpu) ? false : true;
	__vmx_on(&vcpu->phys_vmxon_reg);
	__vmx_vmptrld(&vcpu->vmcs_reg);
}
