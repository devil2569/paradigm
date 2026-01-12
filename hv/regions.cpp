#include "regions.h"

paradigm::regs* regions = nullptr;

auto paradigm::regs::allocate_vmx_reg(vcpu* vcpu) -> bool
{
	ia32_vmx_basic_register vmx_basic{};
	PHYSICAL_ADDRESS max{};
	max.QuadPart = MAXULONG64;

	void* va = MmAllocateContiguousMemory(vmxon_sz, max);
	if (!va)
		return false;

	RtlZeroMemory(va, vmxon_sz);

	vmx_basic.flags = __readmsr(IA32_VMX_BASIC);
	*reinterpret_cast<uint32_t*>(va) = vmx_basic.vmcs_revision_id;

	vcpu->vmxon_reg = reinterpret_cast<uint64_t>(va);
	vcpu->phys_vmxon_reg = util->virt_to_phys(va);

	return vcpu->phys_vmxon_reg != 0;
}

auto paradigm::regs::allocate_vmcs_reg(vcpu* vcpu) -> bool
{
	ia32_vmx_basic_register vmx_basic{};
	PHYSICAL_ADDRESS max{};
	max.QuadPart = MAXULONG64;

	void* va = MmAllocateContiguousMemory(vmxon_sz, max);
	if (!va)
		return false;

	RtlZeroMemory(va, vmxon_sz);

	vmx_basic.flags = __readmsr(IA32_VMX_BASIC);
	*reinterpret_cast<uint32_t*>(va) = vmx_basic.vmcs_revision_id;

	vcpu->vmcs_reg = reinterpret_cast<uint64_t>(va);
	vcpu->phys_vmcs_reg = util->virt_to_phys(va);

	return vcpu->phys_vmcs_reg != 0;
}

auto paradigm::regs::allocate_vmm_stack(vcpu* vcpu)->bool
{
	vcpu->vmm_stack = reinterpret_cast<uint64_t>(ExAllocatePool2(POOL_FLAG_NON_PAGED, vmm_sz, (ULONG)'hv'));
	if (!vcpu->vmm_stack) return false;

	RtlZeroMemory(reinterpret_cast<PVOID>(vcpu->vmm_stack), vmm_sz);
	return true;
}

auto paradigm::regs::allocate_regions(vcpu* vcpu) -> bool
{
	if (!allocate_vmx_reg(vcpu))
		return false;

	if (!allocate_vmcs_reg(vcpu))
		return false;

	if (!allocate_vmm_stack(vcpu))
		return false;

	return true;
}
