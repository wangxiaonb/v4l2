##################################################################
# TARGET = ${notdir $(CURDIR)}
# TARGET = v4l2.cpython-37m-arm-linux-gnueabihf.so
# TARGET = v4l2_test
# TARGET = v4l2_grab
# TARGET = flash_led
# TARGET = brenner
# TARGET = uvc
# TARGET = uvc2
# TARGET = uvc3
# TARGET = render
TARGET = mmal
# TARGET = dma


SRC_DIR = .
SRC_SUBDIR += .
OBJ_DIR = build

INCLUDES = \
-I/usr/include/ \
-I/usr/include/python3.7m \
-I/usr/local/include/

CC = g++
C_FLAGS = -g -O0 -Wall
LD = $(CC)
LD_FLAGS := -lpthread `pkg-config --cflags --libs opencv`
INCLUDE_SRCS := $(TARGET).cpp v4l2.cpp

ifeq ($(CC), g++)
	TYPE = cpp
else
	TYPE = c
endif

ifeq ($(TARGET), v4l2.cpython-37m-arm-linux-gnueabihf.so)
	LD_FLAGS += -shared
	INCLUDE_SRCS := pyv4l2.cpp v4l2.cpp
else ifneq ($(findstring $(TARGET),  mmal render),)
	C_FLAGS += -I/opt/vc/include -pipe -W -Wextra
	LD_LIBS = -L/opt/vc/lib -lrt -lbcm_host -lvcos -lvchiq_arm -pthread -lmmal_core -lmmal_util -lmmal_vc_client -lvcsm
else ifeq ($(TARGET), v4l2_grab)
	INCLUDE_SRCS := $(TARGET).cpp flash_led.cpp
else
endif

SRCS = $(filter-out $(EXCLUDE_SRCS),$(INCLUDE_SRCS))

OBJS += ${foreach src, $(notdir $(SRCS)), ${patsubst %.$(TYPE), $(OBJ_DIR)/%.o, $(src)}}

vpath %.$(TYPE) $(sort $(dir $(SRCS)))

ifneq ($(TARGET),$(subst .cpython-37m-arm-linux-gnueabihf.so,,$(TARGET)))
all:
	python3 setup.py build_ext --inplace
	mv $(TARGET) python
else
all : $(TARGET)
	@echo "Builded target:" $^
	@echo "Done"
	cp $(TARGET) exec

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

endif

.PHONY : clean cleanobj
clean : cleanobj
	@echo "Remove all executable files"
	rm -f $(TARGET)
cleanobj :
	@echo "Remove object files"
	rm -rf $(OBJ_DIR)/*.o
	rm $(TARGET)
	# rm -rf build/*
