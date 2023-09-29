
CC               = gcc
LD               = gcc

CCFLAGS          = -c -I ../cpp -D SAMPLE_SIZE=16000
LDFLAGS          =

all: make_float_weights make_fixed_weights make_float_wavedata

make_float_weights: make_float_weights.o
	@echo "LD      -o $@ "
	@echo "$(LD) $(LDFLAGS) -o $< make_float_weights.o " >> make.out
	@$(LD) -o $@ make_float_weights.o >> make.out

make_fixed_weights: make_fixed_weights.o	
	@echo "LD      -o $@ "
	@echo "$(LD) $(LDFLAGS) -o $< make_fixed_weights.o " >> make.out
	@$(LD) -o $@ make_fixed_weights.o >> make.out

make_float_wavedata: make_float_wavedata.o read_wave.o ../cpp/read_wave.h
	@echo "LD      -o $@ "
	@echo "$(LD) $(LDFLAGS) -o $< make_float_wavedata.o read_wave.o " >> make.out
	@$(LD) -o $@ make_float_wavedata.o read_wave.o >> make.out

make_fixed_wavedata: make_fixed_wavedata.o read_wave.o ../cpp/read_wave.h
	@echo "LD      -o $@ "
	@echo "$(LD) $(LDFLAGS) -o $< make_fixed_wavedata.o read_wave.o " >> make.out
	@$(LD) -o $@ make_fixed_wavedata.o read_wave.o >> make.out

#read_wave.o: ../cpp/read_wave.cpp ../cpp/read_wave.h
#	@echo "CC      $< "
#	@echo "$(CC) $(CCFLAGS) -o $@ $< " >> make.out
#	@$(CC) $(CCFLAGS) -o $@ $< >> make.out
#
%.o: %.c
	@echo "CC      $< "
	@echo "$(CC) $(CCFLAGS) -o $@ $< " >> make.out
	@$(CC) $(CCFLAGS) -o $@ $< >> make.out

clean:
	@echo "Removing created files and cruft"
	@rm -rf make_float_weights make_float_weights.o \
	       make_fixed_weights make_fixed_weights.o \
               make_float_wavedata make_float_wavedata.o \
               make_fixed_wavedata make_fixed_wavedata.o \
               read_wave.o \
               make.out
