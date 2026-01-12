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

typedef union __segment_selector_t
{
    struct
    {
        unsigned __int16 rpl : 2;
        unsigned __int16 table : 1;
        unsigned __int16 index : 13;
    };
    unsigned __int16 flags;
} segment_selector;

typedef union __segment_access_rights_t
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
} segment_access_rights;

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


extern hypervisor* hv;
