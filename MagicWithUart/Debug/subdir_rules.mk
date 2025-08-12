################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C6000 Compiler'
	"C:/ti/ti-cgt-c6000_8.5.0.LTS/bin/cl6x" -mv6600 --include_path="C:/Users/eeric/workspace_v9/dual_core/MagicWithUart" --include_path="C:/ti/ti-cgt-c6000_8.5.0.LTS/include" --include_path="C:/ti/pdk_c665x_2_0_16/packages" --include_path="C:/ti/pdk_c665x_2_0_16/packages/ti/drv/uart" --include_path="C:/ti/pdk_C6657_1_1_2_6/packages/ti/platform" --define=__TI_NO_PARALLEL_LOADS=1 --define=SOC_C6657 --define=OSAL_NONOS --define=BUILD_OSAL=NonOS --define=evmc6657l -g --diag_warning=225 --diag_wrap=off --display_error_number --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


