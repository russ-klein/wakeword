
#CATAPULT_VERSION ?= 10.5c
CATAPULT_VERSION ?= 2021.1

CATAPULT_HOME    ?= /wv/hlsb/CATAPULT/$(CATAPULT_VERSION)/PRODUCTION/aol/Mgc_home

#==============================================

FUNCTION         ?= conv_par_in
ACCEL_NAME       ?= cat_accel
PAR              ?= 1

SOURCES          = testbench.cpp ../cpp/dense.cpp ../cpp/cat_access.cpp
#SPEC_FILE        = $(FUNCTION).spec
HEADERS          = ../cpp/cat_access.h ../cpp/dense.h ../cpp/cat_access.h

ACCELERATOR      = ./hw/dense.v
BUS_IF           = ./hw/$(ACCEL_NAME).v
#HEADER           = ./sw/$(FUNCTION).h 

HOST_EXE         = testbench

CRUFT            = Catapult* catapult.log make.out design_checker_constraints.tcl design_checker_pre_build.tcl catapult.pinfo

CATAPULT         = $(CATAPULT_HOME)/bin/catapult
#CATAPULT_FLAGS   = -shell -file master_par$(PAR).tcl
CATAPULT_FLAGS   = -shell -file dense_esp.tcl

ifgen            = ../../../if_gen/if_gen_axi

INCLUDES         = -I $(CATAPULT_HOME)/shared/include -I ../include -I . -I ../cpp
#INCLUDES         = -I ~/catapult/include -I ../../../include -I .

OPTS             += -D FIXED_POINT -D WEIGHT_MEMORY -D MASTER -D A53
#OPTS             += -D ALGORITHMIC_VERSION
OPTS             += -D ARCHITECTURE_VERSION

export WEIGHT_PATH = ../../../data
export PAR

.PHONY: all run_host clean

all: $(ACCELERATOR) $(HEADER) $(BUS_IF) $(HOST_EXE)

run_host: $(HOST_EXE)
	@echo "Running host testbench"
	@./$(HOST_EXE)

$(ACCELERATOR): $(SOURCES) $(HEADERS) dense_esp.tcl
	@echo "catapult testbench.cpp -o $(ACCELERATOR)"
	echo PAR=$(PAR)
	@rm -rf $(CRUFT)
	@$(CATAPULT) $(CATAPULT_FLAGS) >> make.out
	@cp Catapult/*/concat_rtl.v $@
	@rm -rf $(CRUFT)

$(HOST_EXE): $(HEADERS) $(SOURCES) 
	@echo "gcc $(SOURCES) -o $@"
	@echo PAR=$(PAR)
	@g++ -g -o $@ $(INCLUDES) -D HOST -D PAR_IN=$(PAR) -std=c++11 $(OPTS) $(SOURCES) >> make.out
#
#$(BUS_IF): $(SPEC_FILE) $(SOURCES)
#	@echo ifgen $(FUNCTION) $(SPEC_FILE) $(ACCEL_NAME) > $@
#	@$(ifgen) $(FUNCTION) $(SPEC_FILE) $(ACCEL_NAME)
#	mv $(ACCEL_NAME).v $(BUS_IF)
#	#mv $(FUNCTION).h $(HEADER)
#
#
#$(HEADER): $(SPEC_FILE) $(SOURCES)
#	@echo if_gen $(SPEC_FILE) > $@
#	@$(ifgen) $(FUNCTION) $(SPEC_FILE) $(ACCEL_NAME)
#	mv $(ACCEL_NAME).v $(BUS_IF)
#	mv $(FUNCTION).h $(HEADER)
#

clean:
	rm -rf $(CRUFT) $(ACCELERATOR) $(HOST_EXE) $(BUS_IF) $(HEADER)


