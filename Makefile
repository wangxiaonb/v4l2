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
# TARGET = v4l2
TARGET = v4l2.cpython-37m-arm-linux-gnueabihf.so
# TARGET = test

SRC_DIR = .
SRC_SUBDIR += .
INCLUDE_DIR += /usr/include/python3.7m
OBJ_DIR = .

CC = g++
C_FLAGS = -g -Wall
LD = $(CC)
INCLUDES += -I$(INCLUDE_DIR)
LD_FLAFG += 
LD_LIBS =

# C_FLAGS += -pthread -DNDEBUG -g -fwrapv -O2 -Wall -fstack-protector-strong -Wformat -Werror=format-security -Wdate-time -D_FORTIFY_SOURCE=2 -fPIC

# 
# LD_FLAGS += -pthread -shared -Wl,-O1 -Wl,-Bsymbolic-functions -Wl,-z,relro -g -fstack-protector-strong -Wformat -Werror=format-security -Wdate-time -D_FORTIFY_SOURCE=2
# -o /home/pi/v4l2/v4l2.cpython-37m-arm-linux-gnueabihf.so

ifeq ($(CC), g++)
	TYPE = cpp
else
	TYPE = c
endif


# INCLUDE_SRCS = ${foreach subdir, $(SRC_SUBDIR), ${wildcard $(SRC_DIR)/$(subdir)/*.$(TYPE)}}
INCLUDE_SRCS=$(wildcard *.$(TYPE))

# EXCLUDE_SRCS = ${foreach subdir, $(SRC_SUBDIR), ${wildcard $(SRC_DIR)/$(subdir)/pyv4l2.$(TYPE)}}

ifeq ($(TARGET), v4l2.cpython-37m-arm-linux-gnueabihf.so)
	LD_FLAGS += -shared
	EXCLUDE_SRCS = test.cpp
else
	EXCLUDE_SRCS = pyv4l2.cpp
endif

SRCS = $(filter-out $(EXCLUDE_SRCS),$(INCLUDE_SRCS))

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
# @echo "Compile finished\n"

.PHONY : clean cleanobj
clean : cleanobj
	@echo "Remove all executable files"
	rm -f $(TARGET)
cleanobj :
	@echo "Remove object files"
	rm -rf $(OBJ_DIR)/*.o