# # TARGET = gpio_control_user
# TARGET = v4l2

# C_INCLUDES =  \
# -I/usr/include/python3.7m

# # CFLAGS = -O0 -g
# CPPFLAGS = -O0 -g

# CPPFLAGS += ${C_INCLUDES}

# CC = g++
# # CC += ${CFLAGS}

# # OBJS = test.o add.o
# OBJS = ${TARGET}.o

# LIBS = -lm

# ${TARGET}: ${OBJS}
# 	${CC} -o $@ ${OBJS} ${LIBS} ${CFLAGS}

# .PHONY : clean
# clean: 
# 	rm -f ${TARGET} ${OBJS}

##################################################################
# TARGET = ${notdir $(CURDIR)}
TARGET = v4l2

SRC_DIR = .
SRC_SUBDIR += . 
INCLUDE_DIR += /usr/include/python3.7m
OBJ_DIR = .

CC = g++
C_FLAGS = -g -Wall
LD = $(CC)
INCLUDES += -I$(INCLUDE_DIR)
LD_FLAFS += 
LD_LIBS =

# LD += -shared

ifeq ($(CC), g++)
	TYPE = cpp
else
	TYPE = c
endif

SRCS += ${foreach subdir, $(SRC_SUBDIR), ${wildcard $(SRC_DIR)/$(subdir)/*.$(TYPE)}}
OBJS += ${foreach src, $(notdir $(SRCS)), ${patsubst %.$(TYPE), $(OBJ_DIR)/%.o, $(src)}}

vpath %.$(TYPE) $(sort $(dir $(SRCS)))

all : $(TARGET)
	@echo "Builded target:" $^
	@echo "Done"

$(TARGET) : $(OBJS)
	@mkdir -p $(@D)
	@echo "Linking" $@ "from" $^ "..."
	$(LD) -o $@ $^ $(LD_FLAGS) $(LD_LIBS)
	@echo "Link finished\n"

$(OBJS) : $(OBJ_DIR)/%.o:%.$(TYPE)
	@mkdir -p $(@D)
	@echo "Compiling" $@ "from" $< "..."
	$(CC) -c -o $@ $< $(C_FLAGS) $(INCLUDES)
	@echo "Compile finished\n"

.PHONY : clean cleanobj
clean : cleanobj
	@echo "Remove all executable files"
	rm -f $(TARGET)
cleanobj :
	@echo "Remove object files"
	rm -rf $(OBJ_DIR)/*.o