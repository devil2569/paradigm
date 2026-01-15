#pragma once
#include "ia32.hpp"
#include "commons.hpp"

inline auto vmread(uint64_t const field) -> uint64_t
{
	uint64_t value;
	__vmx_vmread(field, &value);
	return value;
}

inline auto vmwrite(uint64_t const field, uint64_t const value) -> uint64_t
{
	__vmx_vmwrite(field, (size_t)&value);
	return value;
}

inline auto vmptrld(uint64_t vmcs_addr) -> bool
{
	return __vmx_vmptrld(&vmcs_addr) == 0;
}

inline auto vmxon(uint64_t vmxon_addr) -> bool
{
	return __vmx_on(&vmxon_addr) == 0;
}

inline auto vmxoff() -> void
{
	__vmx_off();
}

inline auto vmclear(uint64_t vmcs_addr) -> bool
{
	return __vmx_vmclear(&vmcs_addr) == 0;
}
