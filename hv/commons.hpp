#pragma once
#include <ntddk.h>
#include <intrin.h>
#include <cstdint>

constexpr auto vmxon_sz = 0x1000;
constexpr auto vmm_sz = 0x6000;

typedef struct vcpu
{
    uint64_t vmxon_reg;
    uint64_t phys_vmxon_reg;
    uint64_t vmcs_reg;
    uint64_t phys_vmcs_reg;
    uint64_t vmm_stack;
} vcpu;

typedef struct hypervisor
{
    uint64_t vcpu_count;
    vcpu* vcpus;
} hypervisor;

typedef union hv_segment_selector
{
    struct
    {
        unsigned __int16 rpl : 2;
        unsigned __int16 table : 1;
        unsigned __int16 index : 13;
    };
    unsigned __int16 flags;
} hv_segment_selector;

typedef union hv_segment_access_rights
{
    struct
    {
        unsigned __int32 type : 4;
        unsigned __int32 descriptor_type : 1;
        unsigned __int32 dpl : 2;
        unsigned __int32 present : 1;
        unsigned __int32 reserved0 : 4;
        unsigned __int32 available : 1;
        unsigned __int32 long_mode : 1;
        unsigned __int32 default_big : 1;
        unsigned __int32 granularity : 1;
        unsigned __int32 unusable : 1;
        unsigned __int32 reserved1 : 15;
    };
    unsigned __int32 flags;
} hv_segment_access_rights;


typedef struct _guest_registers
{
    __m128 xmm0;
    __m128 xmm1;
    __m128 xmm2;
    __m128 xmm3;
    __m128 xmm4;
    __m128 xmm5;
    __m128 xmm6;
    __m128 xmm7;
    __m128 xmm8;
    __m128 xmm9;
    __m128 xmm10;
    __m128 xmm11;
    __m128 xmm12;
    __m128 xmm13;
    __m128 xmm14;
    __m128 xmm15;

    uint64_t padding_8b;

    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
} guest_registers, * pguest_registers;


extern "C" unsigned __int32 __load_ar(unsigned __int16 segment_selector);
extern "C" segment_selector get_cs(void);
extern "C" segment_selector get_ds(void);
extern "C" segment_selector get_es(void);
extern "C" segment_selector get_ss(void);
extern "C" segment_selector get_fs(void);
extern "C" segment_selector get_gs(void);
extern "C" segment_selector get_ldtr(void);
extern "C" segment_selector get_tr(void);

extern "C" void _sgdt(segment_descriptor_register_64* gdtr);

extern "C" void asm_vmexit_stub(void);
extern "C" bool vmlaunch_asm(void);

extern hypervisor* hv;
