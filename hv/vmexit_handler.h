#pragma once
#include "ia32.hpp"
#include "commons.hpp"
#include "vmx.h"
#include "vmwrappers.h"

extern "C" auto exit_handler(pguest_registers trap_frame) -> uint64_t;

namespace paradigm
{
	class vmexits : vmx
	{
	public:
		inline auto handle_hlt() -> void
		{
			DbgPrint("guest attempted to execute hlt at: [0x%p]", vmread(VMCS_GUEST_RIP));
			increment_guest_ip();
		}
	};
}

extern paradigm::vmexits* vmexit;
