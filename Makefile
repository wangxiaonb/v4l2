# CXX       = g++
# CFLAGS    = -Wall 
# LDFLAGS   = `pkg-config --cflags --libs opencv`

# DBG_ENABLE=0

# ifeq (1, ${DBG_ENABLE})
# 	CFLAGS += -D_DEBUG -O0 -g -DDEBUG=1
# endif

# # SRCS = $(wildcard *.cpp)
# SRCS = test.cpp
# TARGETS = $(patsubst %.cpp, %,$(SRCS))

# all:build

# build:$(TARGETS)

# $(TARGETS):%:%.cpp
# 	$(CXX) $< -o $@ $(CFLAGS) $(LDFLAGS)
	
##################################################################
# TARGET = ${notdir $(CURDIR)}
# TARGET = v4l2.cpython-37m-arm-linux-gnueabihf.so
# TARGET = v4l2
# TARGET = v4l2_grab
# TARGET = flash_led
# TARGET = ov9281
# TARGET = uvc
# TARGET = uvc2
# TARGET = uvc3
TARGET = render


SRC_DIR = .
SRC_SUBDIR += .
OBJ_DIR = .

INCLUDES = \
-I/usr/include/ \
-I/usr/include/python3.7m \
-I/usr/local/include/

# -I/usr/local/include/opencv4/


CC = g++
C_FLAGS = -g -O0 -Wall
LD = $(CC)
LD_FLAGS += -lpthread

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
# INCLUDE_SRCS=$(wildcard *.$(TYPE))

# EXCLUDE_SRCS = ${foreach subdir, $(SRC_SUBDIR), ${wildcard $(SRC_DIR)/$(subdir)/pyv4l2.$(TYPE)}}

ifeq ($(TARGET), v4l2.cpython-37m-arm-linux-gnueabihf.so)
	LD_FLAGS += -shared
	LD_FLAGS += `pkg-config --cflags --libs opencv`
	INCLUDE_SRCS = pyv4l2.cpp v4l2.cpp
else ifeq ($(TARGET), ov9281)
	LD_FLAGS  += `pkg-config --cflags --libs opencv`
	INCLUDE_SRCS = $(TARGET).cpp v4l2.cpp
else ifeq ($(TARGET), v4l2)
	LD_FLAGS  += `pkg-config --cflags --libs opencv`
	INCLUDE_SRCS = v4l2.cpp
else ifeq ($(TARGET), uvc)
	LD_FLAGS  += `pkg-config --cflags --libs opencv`
	INCLUDE_SRCS = $(TARGET).cpp v4l2.cpp
else ifeq ($(TARGET), v4l2_grab)
	LD_FLAGS  += `pkg-config --cflags --libs opencv`
	INCLUDE_SRCS = $(TARGET).cpp flash_led.cpp
else ifeq ($(TARGET), render)
	LD_FLAGS  += `pkg-config --cflags --libs opencv`
	C_FLAGS += -I/opt/vc/include -pipe -W -Wextra
	LD_LIBS = -L/opt/vc/lib -lrt -lbcm_host -lvcos -lvchiq_arm -pthread -lmmal_core -lmmal_util -lmmal_vc_client -lvcsm
	INCLUDE_SRCS = $(TARGET).cpp v4l2.cpp
else
	EXCLUDE_SRCS = pyv4l2.cpp
	EXCLUDE_SRCS += v4l2.cpp
# EXCLUDE_SRCS += v4l2_grab.cpp
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