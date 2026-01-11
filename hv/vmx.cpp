auto paradigm::vmx::detect_vmx_support() -> bool
{
	int regs[4];
	ia32_feature_control_register feature_ctrl{};

	__cpuid(regs, 1);

	const bool data = (regs[2] & (1 << 5)) != 0;
	if (!data)
		return false;

	if ((__readcr4() & (1ull << 13)) != 0)
		return false;

	feature_ctrl.flags = __readmsr(IA32_FEATURE_CONTROL);

	if (!feature_ctrl.lock_bit)
	{
		feature_ctrl.lock_bit = TRUE;
		feature_ctrl.enable_vmx_outside_smx = TRUE;
		__writemsr(IA32_FEATURE_CONTROL, feature_ctrl.flags);
	}

	return feature_ctrl.enable_vmx_outside_smx != 0;
}
