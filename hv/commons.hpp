#include <ntddk.h>
#include <intrin.h>
#include <cstdint>

typedef struct vcpu
{
	uint64_t vmxon_reg;
	uint64_t phys_vmxon_reg; 

	uint64_t vmcs_reg; 
	uint64_t phys_vmcs_reg; 
};

typedef struct hypervisor
{
	uint64_t vcpu_count;
	vcpu* vcpus;
};

extern hypervisor* hv;
