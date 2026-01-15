#include "vmx.h"
#include "vmwrappers.h"

paradigm::vmx* vmx = nullptr;

auto paradigm::vmx::detect_vmx_support() -> bool
{
	int regs[4];
	ia32_feature_control_register feature_ctrl{};

	__cpuid(regs, 1);

	const bool data = (regs[2] & (1 << 5)) != 0;
	if (!data)
		return false;

	if ((__readcr4() & (1ull << 13)) == 0)
		__writecr4(__readcr4() | (1ull << 13));

	feature_ctrl.flags = __readmsr(IA32_FEATURE_CONTROL);

	if (!feature_ctrl.lock_bit)
	{
		feature_ctrl.lock_bit = TRUE;
		feature_ctrl.enable_vmx_outside_smx = TRUE;
		__writemsr(IA32_FEATURE_CONTROL, feature_ctrl.flags);
	}

	return feature_ctrl.enable_vmx_outside_smx != 0;
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

auto paradigm::vmx::start(vcpu* vcpu) -> bool
{
	if (!detect_vmx_support())
		return false;

	adjust_cr0();
	adjust_cr4();

	if (!regions->allocate_regions(vcpu))
		return false;

	if (vmxon(vcpu->phys_vmxon_reg))
		return false;

	if (vmptrld(vcpu->phys_vmcs_reg))
		return false;

	return true;
}

auto paradigm::vmx::increment_guest_ip() -> void
{
	auto const old_rip = vmread(VMCS_GUEST_RIP);
	auto new_rip = old_rip + vmread(VMCS_VMEXIT_INSTRUCTION_LENGTH);

	if (old_rip < (1ull << 32) && new_rip >= (1ull << 32))
	{
		vmx_segment_access_rights cs_access_rights;

		cs_access_rights.flags = static_cast<uint32_t>(vmread(VMCS_GUEST_CS_ACCESS_RIGHTS));

		// checking for long mode bcs guest will sometimes unintentionally disable it so this check makes it so that we increment EIP properly.
		if (!cs_access_rights.long_mode) new_rip &= 0xFFFF'FFFF;
	}
	vmwrite(VMCS_GUEST_RIP, new_rip);
}
