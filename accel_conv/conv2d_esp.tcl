
solution file add ./testbench.cpp
solution file add ../cpp/cat_access.cpp
solution file add ../cpp/conv_2d.cpp

solution options set /Input/CompilerFlags { -I ../cpp -D HOST -D WEIGHT_MEMORY -D FIXED_POINT -D PAR_IN=1 -D ESP_VERSION }

go analyze
go compile

solution library add nangate-45nm_beh -- -rtlsyntool DesignCompiler -vendor Nangate -technology 045nm
solution library add ccs_sample_mem

go libraries

directive set -CLOCKS {clk {-CLOCK_PERIOD 40 -CLOCK_EDGE rising -CLOCK_HIGH_TIME 20 -CLOCK_OFFSET 0.000000 -CLOCK_UNCERTAINTY 0.0 -RESET_KIND sync -RESET_SYNC_NAME rst -RESET_SYNC_ACTIVE high -RESET_ASYNC_NAME arst_n -RESET_ASYNC_ACTIVE low -ENABLE_NAME {} -ENABLE_ACTIVE high}}

go assembly

directive set /conv2d_esp_hw/core/convolve:coefficient_regs:rsc -MAP_TO_MODULE {[Register]}
directive set /conv2d_esp_hw/core/convolve:image_slice:rsc -MAP_TO_MODULE {[Register]}
directive set /conv2d_esp_hw/core -DESIGN_GOAL Latency

go architect
go allocate
go extract
