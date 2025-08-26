NAME = CK
VERSION = 0.1.0

CC = gcc
AR = ar

CFLAGS =
DEBUGFLAGS = -g
RELEASEFLAGS = -O3

GLEWDIR = libs/external/glew-2.1.0
GLEWFLAGS = -I$(GLEWDIR)/include -L$(GLEWDIR)/lib/Release/x64

GLFWDIR = libs/external/glfw-3.4.bin.WIN64
GLFWFLAGS = -I$(GLFWDIR)/include -L$(GLFWDIR)/lib-mingw-w64

FREETYPE_DIR = libs/external/freetype
FREETYPE_FLAGS = -I$(FREETYPE_DIR) -L$(FREETYPE_DIR)

LIBFLAGS =  $(GLFWFLAGS) $(GLEWFLAGS) $(FREETYPE_FLAGS) $(ECTERNAL_FLAGS)

LIBS = -lglew32 -lglfw3 -lopengl32 -lgdi32 -luser32 -lshell32 -lfreetype

SRCDIR = src
SRCS = $(wildcard $(SRCDIR)/*.c)

HEADERS = $(wildcard libs/*.h)

BUILD_DIR = build
DEBUG_DIR = $(BUILD_DIR)/Debug
RELEASE_DIR = $(BUILD_DIR)/Release

TARGET_DEBUG = $(DEBUG_DIR)/$(NAME).exe
TARGET_RELEASE = $(RELEASE_DIR)/$(NAME).exe

# Static library outputs
STATIC_DEBUG = $(DEBUG_DIR)/lib$(NAME).a
STATIC_RELEASE = $(RELEASE_DIR)/lib$(NAME).a

DEBUG_OBJS = $(patsubst $(SRCDIR)/%, $(DEBUG_DIR)/%, $(SRCS:.c=.obj))
RELEASE_OBJS = $(patsubst $(SRCDIR)/%, $(RELEASE_DIR)/%, $(SRCS:.c=.obj))

# Exclude test program objects from the static libraries (if present)
LIB_EXCLUDE = $(SRCDIR)/test.c
LIB_SRCS = $(filter-out $(LIB_EXCLUDE),$(SRCS))
LIB_DEBUG_OBJS = $(patsubst $(SRCDIR)/%, $(DEBUG_DIR)/%, $(LIB_SRCS:.c=.obj))
LIB_RELEASE_OBJS = $(patsubst $(SRCDIR)/%, $(RELEASE_DIR)/%, $(LIB_SRCS:.c=.obj))

all: debug

debug: CFLAGS += $(DEBUGFLAGS) $(LIBFLAGS)
debug: $(TARGET_DEBUG)

release: CFLAGS += $(RELEASEFLAGS) $(LIBFLAGS)
release: $(TARGET_RELEASE)

# Build static libraries
static: static-debug

static-debug: CFLAGS += $(DEBUGFLAGS) $(LIBFLAGS)
static-debug: $(STATIC_DEBUG)

static-release: CFLAGS += $(RELEASEFLAGS) $(LIBFLAGS)
static-release: $(STATIC_RELEASE)

$(TARGET_DEBUG): $(DEBUG_OBJS)
	mkdir -p $(DEBUG_DIR)
	$(CC) $(CFLAGS)  -o $(TARGET_DEBUG) $(DEBUG_OBJS) $(LDFLAGS) $(LIBS)

$(TARGET_RELEASE): $(RELEASE_OBJS)
	mkdir -p $(RELEASE_DIR)
	$(CC) $(CFLAGS) -o $(TARGET_RELEASE) $(RELEASE_OBJS) $(LDFLAGS) $(LIBS)

$(STATIC_DEBUG): $(LIB_DEBUG_OBJS)
	mkdir -p $(DEBUG_DIR)
	$(AR) rcs $@ $(LIB_DEBUG_OBJS)

$(STATIC_RELEASE): $(LIB_RELEASE_OBJS)
	mkdir -p $(RELEASE_DIR)
	$(AR) rcs $@ $(LIB_RELEASE_OBJS)

$(DEBUG_DIR)/%.obj: $(SRCDIR)/%.c $(HEADERS)
	mkdir -p $(DEBUG_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(RELEASE_DIR)/%.obj: $(SRCDIR)/%.c $(HEADERS)
	mkdir -p $(RELEASE_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.pdb
	rm -f $(DEBUG_DIR)/*.pdb
	rm -f $(RELEASE_DIR)/*.pdb
	rm -f $(DEBUG_OBJS) $(RELEASE_OBJS)
	rm -f $(STATIC_DEBUG) $(STATIC_RELEASE)

fclean: clean
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET_DEBUG) $(TARGET_RELEASE)

re: fclean all

.PHONY: all debug release static static-debug static-release clean fclean re
