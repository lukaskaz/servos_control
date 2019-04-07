#Output files
PROJ_NAME=servo_ctrl

#======================================================================#
#Directories
SRC_DIR=src/
INC_DIR=inc/
OBJ_DIR=obj/
MAP_DIR=out/
LIB_DIR=lib/
EXEC_DIR=out/

#======================================================================#
#Special symbols
COMMA=,
ASSIGN==

#======================================================================#
#Cross Compiler
CCPREFIX=/home/lukasz/Desktop/projects/rpi/toolchain/linaro/gcc-linaro-7.4.1-2019.02-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
CC=$(CCPREFIX)g++
READELF=$(CCPREFIX)readelf

#======================================================================#
#Dependencies
DEPS+=rgb-if
#DEPS+=qt-if
#DEPS+=web-if
#DEPS+=console-if

DEPS_REF=$(addprefix ../,$(DEPS))

DEPS_INC_DIR=$(addsuffix /inc/,$(DEPS_REF))
DEPS_SRC_DIR=$(addsuffix /src/,$(DEPS_REF))
DEPS_LIB_DIR=$(addsuffix /lib/,$(DEPS_REF))

DEPS_LIB:=$(addsuffix *.so,$(DEPS_LIB_DIR))
DEPS_LIB:=$(wildcard $(DEPS_LIB))

DEPS_SRC:=$(addsuffix *.cpp,$(DEPS_SRC_DIR))
DEPS_SRC:=$(wildcard $(DEPS_SRC))

DEPS_INC:=$(addsuffix *.hpp,$(DEPS_INC_DIR))
DEPS_INC:=$(wildcard $(DEPS_INC))

DEPS_CMD=$(subst deps-,,$(MAKECMDGOALS))
DEPS_RULE=deps-$(DEPS_CMD)

#======================================================================#
#Libraries
LIBS+=pthread
LIBS+=$(DEPS)

LIBS:=$(addprefix -l,$(LIBS)) $(addprefix -L,$(DEPS_LIB_DIR))

#======================================================================#
#Add sources
SRC+=main.cpp
SRC+=helper.cpp
SRC+=i2c.cpp
SRC+=adcs.cpp
SRC+=servos.cpp
SRC+=logs.cpp

#Prepare objects
OBJ=$(SRC:.cpp=.o)

#======================================================================#
#Prepare groups
INC_GROUP=$(addprefix -I,$(INC_DIR) $(DEPS_INC_DIR))
SRC_GROUP=$(addprefix $(SRC_DIR),$(SRC))
OBJ_GROUP=$(addprefix $(OBJ_DIR),$(OBJ))

#prepare outputs
EXEC_FILE=$(EXEC_DIR)$(PROJ_NAME)
MAP_FILE=$(MAP_DIR)$(PROJ_NAME).map

#======================================================================#
#Flags
CFLAGS+=$(INC_GROUP)
CFLAGS+=-Wall -std=c++14
LFLAGS+= $(LIBS)
LDFLAGS+=$(addprefix -Wl$(COMMA)-rpath$(ASSIGN),$(DEPS_LIB_DIR))

#ELF interpreter options
RELFFLAGS=-aW

#======================================================================#
#Prepare rules
MAKEFLAGS=--no-print-directory

#Makefile rules
build:
#	@$(MAKE) deps-build
	@$(MAKE) deps-install
	@$(MAKE) exec-build

rebuild:
	@$(MAKE) clean
	@$(MAKE) build

release:
	@$(MAKE) deps-release
	@$(MAKE) exec-release

debug:
	@$(MAKE) deps-debug
	@$(MAKE) exec-debug


exec-build: CFLAGS+=-O2
exec-build: $(MAP_FILE)

exec-release: CFLAGS+=-O3
exec-release: $(MAP_FILE)

exec-debug: CFLAGS+=-O0 -g3
exec-debug: $(MAP_FILE)

$(DEPS_RULE): DEP_MAKE=$(MAKE) --directory=$@ $(DEPS_CMD)
#$(DEPS_RULE): MAKEFLAGS=--print-directory
$(DEPS_RULE): $(DEPS_REF)


$(DEPS_REF):
	@echo ">> Checking dependencies for '$(@F)'"
	@+$(DEP_MAKE)

$(MAP_FILE): $(EXEC_FILE)
	@echo "Creating map file '$(@F)'"
	$(READELF) $(RELFFLAGS) $^ > $@

$(EXEC_FILE): $(OBJ_GROUP)
	@echo "Linking project '$(@F)'"
	$(CC) $^ -o $@ $(CFLAGS) $(LFLAGS) $(LDFLAGS)

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp $(INC_DIR)%.hpp $(DEPS_LIB)
	@echo "Creating object '$(@F)'"
	$(CC) -c $< -o $@ $(CFLAGS)


#Make clean
clean: deps-clean
	@echo "Cleaning the project '$(PROJ_NAME)'"
	rm -rf $(MAP_DIR)* $(EXEC_DIR)* $(OBJ_DIR)*

#======================================================================
.PHONY: rebuild release debug clean $(DEPS_REF)

