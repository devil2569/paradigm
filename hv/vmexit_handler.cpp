#include "vmexit_handler.h"

extern "C" auto exit_handler(pguest_registers trap_frame) -> uint64_t
{
	auto exit_reason = vmread(VMCS_EXIT_REASON);
	switch (exit_reason & 0xffff)
	{
	case VMX_EXIT_REASON_EXECUTE_HLT:
		vmexit->handle_hlt();
	}
}
