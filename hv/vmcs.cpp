#include "vmcs.h"

paradigm::vmcs* vmcs;

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
