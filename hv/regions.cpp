#include "regions.h"

auto paradigm::regs::allocate_vmx_reg(vcpu* vcpu) -> bool
{
	ia32_vmx_basic_register vmx_basic{};
	PHYSICAL_ADDRESS max{};
	max.QuadPart = MAXULONG64;

	auto alloc = [&]() -> bool
	{
		return (vcpu->vmxon_reg = reinterpret_cast<uint64_t>(MmAllocateContiguousMemory(0x1000, max)))
			? (RtlZeroMemory(reinterpret_cast<void*>(vcpu->vmxon_reg), 0x1000),
				vmx_basic.flags = __readmsr(IA32_VMX_BASIC),
				*reinterpret_cast<uint32_t*>(vcpu->vmxon_reg) = vmx_basic.vmcs_revision_id,
				vcpu->phys_vmxon_reg = util->virt_to_phys(reinterpret_cast<void*>(vcpu->vmxon_reg)),
				true)
			: false;
	};

	return alloc();
}


auto paradigm::regs::allocate_vmcs_reg(vcpu* vcpu) -> bool
{
	ia32_vmx_basic_register vmx_basic{};
	PHYSICAL_ADDRESS max{};
	max.QuadPart = MAXULONG64;

	auto alloc = [&]() -> bool
	{
		return (vcpu->vmcs_reg = reinterpret_cast<uint64_t>(MmAllocateContiguousMemory(0x1000, max)))
			? (RtlZeroMemory(reinterpret_cast<void*>(vcpu->vmcs_reg), 0x1000),
				vmx_basic.flags = __readmsr(IA32_VMX_BASIC),
				*reinterpret_cast<uint32_t*>(vcpu->vmcs_reg) = vmx_basic.vmcs_revision_id,
				vcpu->phys_vmcs_reg = util->virt_to_phys(reinterpret_cast<void*>(vcpu->vmcs_reg)),
				true)
			: false;
	};

	return alloc();
}

auto paradigm::regs::allocate_regions(vcpu* vcpu) -> bool
{
	return !allocate_vmx_reg(vcpu) || !allocate_vmcs_reg(vcpu) ? false : true;
}
