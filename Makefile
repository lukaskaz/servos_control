#Output files
APP_NAME=servos_ctrl

#======================================================================#
#Directories
SRC_DIR=src/
INC_DIR=inc/
OBJ_DIR=obj/
MAP_DIR=out/
EXEC_DIR=out/

#======================================================================#
#Cross Compiler
CC=g++
READELF=readelf

#======================================================================#
#Flags

CFLAGS=-I $(INC_DIR) -Wall -std=c++14

LFLAGS=

#ELF interpreter options
RELFFLAGS=-aW
#======================================================================#
#Libraries

LIBS=-lpthread
#======================================================================#
#add sources
SRC+=main.cpp
SRC+=helper.cpp
SRC+=i2c.cpp
SRC+=adcs.cpp
SRC+=servos.cpp
SRC+=logs.cpp

#prepare objects
OBJ=$(SRC:.cpp=.o)

#prepare groups
SRC_GROUP=$(addprefix $(SRC_DIR), $(SRC))
OBJ_GROUP=$(addprefix $(OBJ_DIR), $(OBJ))

#prepare outputs
EXEC_FILE=$(EXEC_DIR)$(APP_NAME)
MAP_FILE=$(MAP_DIR)$(APP_NAME).map
#======================================================================#
#Prepare rules
release: CFLAGS+=-O3
build: CFLAGS+=-O2
debug: CFLAGS+=-O0 -g3

#Make rules
build: $(MAP_FILE)
rebuild: clean build
release: clean $(MAP_FILE)
debug: clean $(MAP_FILE)


$(MAP_FILE): $(EXEC_FILE)
	@echo "Creating map file '$(@F)'"
	$(READELF) $(RELFFLAGS) $^ > $@

$(EXEC_FILE): $(OBJ_GROUP)
	@echo "Linking project '$(@F)'"
	$(CC) $^ -o $@ $(CFLAGS) $(LIBS)

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp $(INC_DIR)%.hpp
	@echo "Creating object '$(@F)'"
	$(CC) -c $< -o $@ $(CFLAGS)


#Make clean
clean:
	@echo "Cleaning the project..."
	rm -rf $(EXEC_FILE) $(MAP_FILE) $(OBJ_DIR)*

#======================================================================
.PHONY: build rebuild release debug clean

