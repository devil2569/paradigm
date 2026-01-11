#include <ntddk.h>
#include "hv/vmx.h"

hypervisor* hv = { 0 };

auto driver_unload() -> void
{
	DbgPrint("unloaded\n");
	if (hv)
	{
		if (hv->vcpus) ExFreePool(hv->vcpus);
		ExFreePool(hv);
	}
}

auto driver_entry( PDRIVER_OBJECT drv, PUNICODE_STRING ) -> NTSTATUS
{
	DbgPrint("entered paradigm\n");
	drv->DriverUnload = reinterpret_cast<PDRIVER_UNLOAD>(driver_unload);

	hv = reinterpret_cast<hypervisor*>(ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(hypervisor), 'hv'));
	if (!hv) return STATUS_INSUFFICIENT_RESOURCES;

	RtlZeroMemory(hv, sizeof(hypervisor));

	hv->vcpu_count = KeQueryActiveProcessorCount(0);

	auto const array_sz = sizeof(vcpu) * hv->vcpu_count;
	hv->vcpus = reinterpret_cast<vcpu*>(ExAllocatePool2(POOL_FLAG_NON_PAGED, array_sz, 'hv'));
	if (!hv->vcpus) return STATUS_INSUFFICIENT_RESOURCES;

	RtlZeroMemory(hv->vcpus, array_sz);

	KAFFINITY affinity_mask;

	for (uint64_t i = 0; i < hv->vcpu_count; i++)
	{
		affinity_mask = 1ull << i;
		KeSetSystemAffinityThread(affinity_mask);

		vmx->start(&hv->vcpus[i]);
		DbgPrint("launched on: [%d]\n", (int)i);
		DbgPrint("vmx reg: [0x%p]\n", (PVOID64)hv->vcpus[i].vmxon_reg);
		DbgPrint("vmcs reg: [0x%p]\n", (PVOID64)hv->vcpus[i].vmcs_reg);
	}

	return STATUS_SUCCESS;
}
