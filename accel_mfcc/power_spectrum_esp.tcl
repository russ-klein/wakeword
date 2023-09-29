options set Input/CppStandard c++11
solution file add ./testbench.cpp
solution file add ./tb_functions.cpp
solution file add ../cpp/cat_access.cpp
solution file add ../cpp/power_spectrum.cpp

solution options set /Input/CompilerFlags { -I ../cpp -D HOST -D WEIGHT_MEMORY -D FIXED_POINT -D PAR_IN=1 -D ESP_VERSION }

go analyze
solution design set power_spectrum_hw -top
go compile
solution library add nangate-45nm_beh -- -rtlsyntool DesignCompiler -vendor Nangate -technology 045nm
solution library add ccs_sample_mem
solution library add ccs_sample_rom
go libraries
directive set -CLOCKS {clk {-CLOCK_PERIOD 10.0 -CLOCK_EDGE rising -CLOCK_UNCERTAINTY 0.0 -CLOCK_HIGH_TIME 5.0 -RESET_SYNC_NAME rst -RESET_ASYNC_NAME arst_n -RESET_KIND sync -RESET_SYNC_ACTIVE high -RESET_ASYNC_ACTIVE low -ENABLE_ACTIVE high}}
go assembly
directive set /power_spectrum_hw/core/ac_math::ac_sqrt<64,16,AC_RND,AC_SAT,32,4,AC_RND,AC_SAT>:for -PIPELINE_INIT_INTERVAL 1

directive set /power_spectrum_hw/core/frames_memory:rsc -INTERLEAVE 2
directive set /power_spectrum_hw/core/power_spectrum_dot_product:for -UNROLL 2
directive set /power_spectrum_hw/core/power_spectrum_dot_product:for -PIPELINE_INIT_INTERVAL 1
directive set /power_spectrum_hw/core/for -PIPELINE_INIT_INTERVAL 1

go architect
go extract


