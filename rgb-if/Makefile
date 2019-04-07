#Output files
#LIB_NAME=libfcgi.a
DEP_NAME=rgb-if
LIB_NAME=lib$(DEP_NAME).so
LIB_VER1=1
LIB_VER2=0
LIB_VER3=0

LIB_FULLNAME=$(LIB_NAME).$(LIB_VER1).$(LIB_VER2).$(LIB_VER3)

#======================================================================#
#Directories
SRC_DIR=src/
INC_DIR=inc/
OBJ_DIR=obj/
MAP_DIR=out/
LIB_DIR=lib/

#======================================================================#
#Links
LINK+=$(LIB_NAME)
LINK+=$(LIB_NAME).$(LIB_VER1)
LINK+=$(LIB_NAME).$(LIB_VER1).$(LIB_VER2)

#======================================================================#
#Cross Compiler
CC=g++
AR=ar
LD=g++
READELF=readelf

#======================================================================#
#Dependencies
DEPS+=

DEPS_PATH=$(addprefix ../,$(DEPS))
INC_DEPS=$(addsuffix /inc/,$(DEPS_PATH))

#======================================================================#
#Libraries
LIBS+=
LIBS:=$(addprefix -l,$(LIBS))

#======================================================================#
#Add sources
SRC+=rgb_if.cpp

#Prepare objects
OBJ=$(SRC:.cpp=.o)

#======================================================================#
#Prepare groups
INC_GROUP=$(addprefix -I,$(INC_DIR) $(INC_DEPS))
#SRC_GROUP=$(addprefix $(SRC_DIR), $(SRC))
OBJ_GROUP=$(addprefix $(OBJ_DIR), $(OBJ))
LINK_GROUP=$(addprefix $(LIB_DIR), $(LINK))

#Prepare outputs
LIB_FILE=$(LIB_DIR)$(LIB_FULLNAME)
MAP_FILE=$(subst .so,.map,$(MAP_DIR)$(LIB_NAME))

#======================================================================#
#Flags
CFLAGS=$(INC_GROUP)
CFLAGS+=-Wall -std=c++14
CFLAGS+=-fPIC
ARFLAGS=-cr
LFLAGS=$(LIBS)
LDFLAGS=-shared

#ELF interpreter options
RELFFLAGS=-aW

#======================================================================#
#Prepare rules
MAKEFLAGS=--no-print-directory

release: PARAMS=-O3
build: PARAMS=-O2
debug: PARAMS=-O0 -g3

#Makefile rules
build: $(MAP_FILE)

rebuild:
	@$(MAKE) clean
	@$(MAKE) build

release: $(MAP_FILE)
debug: $(MAP_FILE)
#install: $(LIB_NAME)
reinstall:
	@$(MAKE) clean
	@$(MAKE) install

install:
	@if [ ! -f $(LIB_FILE) ]; then \
		. ./setenv; \
		scp $$RPI_USER@$$RPI_NODE:/home/$$RPI_USER/Desktop/work/rgb_led/rgb_control_with_lib/rgb-if/lib/librgb-if.so* ./lib/; \
		scp $$RPI_USER@$$RPI_NODE:/home/$$RPI_USER/Desktop/work/rgb_led/rgb_control_with_lib/rgb-if/inc/rgb_if.hpp ./inc/; \
	fi 

$(MAP_FILE): $(LINK_GROUP)
	@echo "Creating map file '$(@F)'"
	$(READELF) $(RELFFLAGS) $< > $@

$(LINK_GROUP): $(LIB_FILE)
	@echo "Creating link '$(@F)' for library '$(<F)'"
	@cd $(LIB_DIR) && ln -s $(<F) $(@F)

$(LIB_FILE): $(OBJ_GROUP)
	@echo "Creating library '$(@F)'"
	$(LD) $(LDFLAGS) $^ -o $@  $(PARAMS) $(CFLAGS) $(LFLAGS) 
#	$(AR) $(ARFLAGS) $@ $^
	@find $(LIB_DIR) -type l -delete

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp $(INC_DIR)%.hpp
	@echo "Creating object '$(@F)'"
	$(CC) -c $< -o $@ $(PARAMS) $(CFLAGS)


#Make clean
clean:
	@echo "Cleaning this dependency '$(DEP_NAME)'"
	rm -rf $(MAP_DIR)* $(LIB_DIR)* $(OBJ_DIR)*

#======================================================================
.PHONY: build rebuild release debug install reinstall clean

